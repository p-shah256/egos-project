#include <stdlib.h>
#include "egos.h"
#include "macb.h"
#include "string.h"

/*
 * These buffer sizes must be power of 2 and divisible
 * by RX_BUFFER_MULTIPLE
 */
#define MACB_RX_BUFFER_SIZE		128
#define GEM_RX_BUFFER_SIZE		2048
#define RX_BUFFER_MULTIPLE		64

#define MACB_RX_RING_SIZE		32
#define MACB_TX_RING_SIZE		16

#define MACB_TX_TIMEOUT		1000
#define MACB_AUTONEG_TIMEOUT	5000000


struct macb_dma_desc {
	m_uint32 addr;
	m_uint32 ctrl;
};

#define ARCH_DMA_MINALIGN   64

#define DMA_DESC_SIZE		16 // Check these dma desc size is 8 bytes only
#define DMA_DESC_BYTES(n)	((n) * DMA_DESC_SIZE)
#define MACB_TX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_TX_RING_SIZE))
#define MACB_RX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_RX_RING_SIZE))

#define DESC_PER_CACHELINE_32	(ARCH_DMA_MINALIGN/sizeof(struct macb_dma_desc))
#define DESC_PER_CACHELINE_64	(ARCH_DMA_MINALIGN/DMA_DESC_SIZE)

#define RXBUF_FRMLEN_MASK	0x00000fff
#define TXBUF_FRMLEN_MASK	0x000007ff

#define BUSY_LOOP 10000

#define CACHE_CONTROLLER_BASE 0x2010000
#define CACHE_FLUSH_64 0x200
#define CACHE_FLUSH_32 0x240

struct macb_device {
	void			*base;

	const struct macb_config *config;

	unsigned int		rx_tail;
	unsigned int		tx_head;
	unsigned int		tx_tail;
	unsigned int		next_rx_tail;
	int			wrapped;

	void			*rx_buffer;
	void			*tx_buffer;
	struct macb_dma_desc	*rx_ring;
	struct macb_dma_desc	*tx_ring;
	int			rx_buffer_size;

	m_uint32	    rx_buffer_dma[2];
	m_uint32		rx_ring_dma[2];
	m_uint32		tx_ring_dma[2];
    
    unsigned long		pclk_rate;
    unsigned short		phy_addr;
};

typedef struct macb_device macb_device;

static macb_device macb;

m_uint32 macb_mdc_clk_div(unsigned long macb_hz) {
	m_uint32 config;

	if (macb_hz < 20000000)
		config = MACB_BF(CLK, MACB_CLK_DIV8);
	else if (macb_hz < 40000000)
		config = MACB_BF(CLK, MACB_CLK_DIV16);
	else if (macb_hz < 80000000)
		config = MACB_BF(CLK, MACB_CLK_DIV32);
	else
		config = MACB_BF(CLK, MACB_CLK_DIV64);

	return config;
}

void* alloc_aligned(m_uint32 size, m_uint32* handle) {
    int offset = ARCH_DMA_MINALIGN - 1;
    void* alloc_start;
    if ((alloc_start = (void*)malloc(size + offset)) == NULL) {
       FATAL("Not enough memory");
    }
    *handle = (m_uint32)alloc_start;

    return (void*)(((size_t)(alloc_start) + offset) & ~(ARCH_DMA_MINALIGN - 1));
}

void macb_probe() {
    // Base of register offsets
	macb.base = (void *)(0x10090000);
	macb.pclk_rate = QUANTUM; //  Configure based on the inputs

	macb.rx_buffer_size = MACB_RX_BUFFER_SIZE;
	// Malloc with DMA aligned sync it with D-cache line size ?
	macb.rx_buffer = alloc_aligned(macb.rx_buffer_size * MACB_RX_RING_SIZE, &macb.rx_buffer_dma[1]);
    macb.rx_buffer_dma[0] = (m_uint32)macb.rx_buffer;

    macb.rx_ring = alloc_aligned(MACB_RX_DMA_DESC_SIZE, &macb.rx_ring_dma[1]);
    macb.rx_ring_dma[0] = (m_uint32)macb.rx_ring;
	
    macb.tx_ring = alloc_aligned(MACB_TX_DMA_DESC_SIZE, &macb.tx_ring_dma[1]);
    macb.tx_ring_dma[0] = (m_uint32)macb.tx_ring;
    
	m_uint32 ncfgr = macb_mdc_clk_div(macb.pclk_rate);
	macb_writel(macb, NCFGR, ncfgr);
}

m_uint16 macb_mdio_read(m_uint8 phy_adr, m_uint8 reg) {
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 2)
		 | MACB_BF(PHYA, phy_adr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2));
	macb_writel(macb, MAN, frame);

	do {
		netstat = macb_readl(macb, NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	frame = macb_readl(macb, MAN);

	netctl = macb_readl(macb, NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	return MACB_BFEXT(DATA, frame);
}

/**
 * mii_nway_result
 * @negotiated: value of MII ANAR and'd with ANLPAR
 *
 * Given a set of MII abilities, check each bit and returns the
 * currently supported media, in the priority order defined by
 * IEEE 802.3u.  We use LPA_xxx constants but note this is not the
 * value of LPA solely, as described above.
 *
 * The one exception to IEEE 802.3u is that 100baseT4 is placed
 * between 100T-full and 100T-half.  If your phy does not support
 * 100T4 this is fine. If your phy places 100T4 elsewhere in the
 * priority order, you will need to roll your own function.
 */
unsigned int mii_nway_result (unsigned int negotiated) {
	unsigned int ret;

	if (negotiated & LPA_100FULL)
		ret = LPA_100FULL;
	else if (negotiated & LPA_100BASE4)
		ret = LPA_100BASE4;
	else if (negotiated & LPA_100HALF)
		ret = LPA_100HALF;
	else if (negotiated & LPA_10FULL)
		ret = LPA_10FULL;
	else
		ret = LPA_10HALF;

	return ret;
}

void barrier() {
    asm volatile("fence");
}

int macb_phy_init()
{
	m_uint32 ncfgr;
	m_uint16 phy_id, status, adv, lpa;
	int media, speed, duplex;

    macb.phy_addr = 0; // Could check for all the PHYs from 0 to 32
    phy_id = macb_mdio_read(macb.phy_addr, MII_PHYSID1);
    if (phy_id == 0xffff) {
        FATAL("No PHY present\n");
        return -1;
    }

    status = macb_mdio_read(macb.phy_addr, MII_BMSR);
    if (!(status & BMSR_LSTATUS)) {
        FATAL("Link down (status: 0x%04x)\n", status);
        return -1;
    }

    adv = macb_mdio_read(macb.phy_addr, MII_ADVERTISE);
    lpa = macb_mdio_read(macb.phy_addr, MII_LPA);
    media = mii_nway_result(lpa & adv);
    speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF) ? 1 : 0);
    duplex = (media & ADVERTISE_FULL) ? 1 : 0;

    CRITICAL("PHY Link up, %sMbps %s-duplex (lpa: 0x%04x)", speed ? "100" : "10", duplex ? "full" : "half", lpa);

	ncfgr = macb_readl(macb, NCFGR);
	ncfgr &= ~(MACB_BIT(SPD) | MACB_BIT(FD) | GEM_BIT(GBE));
	if (speed) {
		ncfgr |= MACB_BIT(SPD);
	}
	if (duplex)
		ncfgr |= MACB_BIT(FD);
	
    macb_writel(macb, NCFGR, ncfgr);

	return 0;
}

void flush_dcache_range(unsigned long start_addr, unsigned long size) {
    unsigned long end_addr = start_addr + size;
    
    // Align to 32-byte or 64-byte boundary depending on which you need
    // For 64-byte alignment (matching cache line size):
    start_addr &= ~(64UL - 1);
    
    for (unsigned long addr = start_addr; addr < end_addr; addr += 64) {
        REGW(CACHE_CONTROLLER_BASE, CACHE_FLUSH_64) = addr;

        // Memory barrier to ensure completion
        barrier();
    }
}

void macb_start() {
	unsigned int val = 0;
	unsigned long paddr;
	int i;

	/* initialize DMA descriptors */
	paddr = macb.rx_buffer_dma[0];
	for (i = 0; i < MACB_RX_RING_SIZE; i++) {
		if (i == (MACB_RX_RING_SIZE - 1))
			paddr |= MACB_BIT(RX_WRAP);
		macb.rx_ring[i].ctrl = 0;
		macb.rx_ring[i].addr = paddr;
		paddr += macb.rx_buffer_size;
	}
	flush_dcache_range(macb.rx_ring_dma[0], MACB_RX_DMA_DESC_SIZE);
	flush_dcache_range((m_uint32)macb.rx_buffer, macb.rx_buffer_size * MACB_RX_RING_SIZE);
    // INFO("Size of dma_desc: %d", sizeof(struct macb_dma_desc));
	for (i = 0; i < MACB_TX_RING_SIZE; i++) {
        // INFO("i = %d", i);
		macb.tx_ring[i].addr = 0;
		if (i == (MACB_TX_RING_SIZE - 1))
			macb.tx_ring[i].ctrl = MACB_BIT(TX_USED) |
				MACB_BIT(TX_WRAP);
		else
			macb.tx_ring[i].ctrl = MACB_BIT(TX_USED);
	}
	flush_dcache_range(macb.rx_ring_dma[0], MACB_RX_DMA_DESC_SIZE);

	macb.rx_tail = 0;
	macb.tx_head = 0;
	macb.tx_tail = 0;
	macb.next_rx_tail = 0;

	macb_writel(macb, RBQP, macb.rx_ring_dma[0]);
	macb_writel(macb, TBQP, macb.tx_ring_dma[0]);

    macb_writel(macb, USRIO, MACB_BIT(MII));

	macb_phy_init();
	
	/* Enable TX and RX */
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE));
}

void invalidate_cache(m_uint32 start_addr, m_uint32 size) {
    // unsigned long end_addr = start_addr + size;

    // // Invalidate each cache line in the range
    // for (unsigned long addr = start_addr; addr < end_addr; addr += 64) {
    //     // CBO.INVAL invalidates without writing back
    //     asm volatile("cbo.inval (%0)" :: "r"(addr));
    // }
    
    // Ensure invalidation is complete
    asm volatile("fence");
}

void delay(int itrs) {
    for (int i = 0;i < itrs;++i);
}

void macb_send(int length, void *packet) {
    unsigned long ctrl;
    unsigned int tx_head = macb.tx_head;
    int i;

    ctrl = length & TXBUF_FRMLEN_MASK;
    ctrl |= MACB_BIT(TX_LAST);
    if (tx_head == (MACB_TX_RING_SIZE - 1)) {
        ctrl |= MACB_BIT(TX_WRAP);
        macb.tx_head = 0;
    } else {
        macb.tx_head++;
    }

    macb.tx_ring[tx_head].ctrl = ctrl;
    macb.tx_ring[tx_head].addr = (m_uint32)packet;

    barrier();
    flush_dcache_range(macb.tx_ring_dma[0], MACB_RX_DMA_DESC_SIZE);
    macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE) | MACB_BIT(TSTART));

    INFO("Is Used: %u", (ctrl & MACB_BIT(TX_USED)));
    /*
    * I guess this is necessary because the networking core may
    * re-use the transmit buffer as soon as we return...
    */
    for (i = 0; i <= MACB_TX_TIMEOUT; i++) {
        barrier();
	INFO("Testing %d", i);
        // invalidate_cache(macb.tx_ring_dma[0], MACB_TX_DMA_DESC_SIZE);
        ctrl = macb.tx_ring[tx_head].ctrl;
    INFO("Test Is Used: %u", (ctrl & MACB_BIT(TX_USED)));
        if (ctrl & MACB_BIT(TX_USED))
		INFO("Used %d", i);
            break;
        delay(BUSY_LOOP);
    }

    if (i <= MACB_TX_TIMEOUT) {
        if (ctrl & MACB_BIT(TX_UNDERRUN))
            CRITICAL("TX underrun");
        if (ctrl & MACB_BIT(TX_BUF_EXHAUSTED))
            CRITICAL("TX buffers exhausted in mid frame");
    } else {
        CRITICAL("TX timeout");
    }
}

static void reclaim_rx_buffer(unsigned int idx) {
    unsigned int mask;
    unsigned int shift;
    unsigned int i;

    /*
    * There may be multiple descriptors per CPU cacheline,
    * so a cache flush would flush the whole line, meaning the content of other descriptors
    * in the cacheline would also flush. If one of the other descriptors had been
    * written to by the controller, the flush would cause those changes to be lost.
    *
    * To circumvent this issue, we do the actual freeing only when we need to free
    * the last descriptor in the current cacheline. When the current descriptor is the
    * last in the cacheline, we free all the descriptors that belong to that cacheline.
    */
    mask = DESC_PER_CACHELINE_32 - 1;
    shift = 0;

    /* we exit without freeing if idx is not the last descriptor in the cacheline */
    if ((idx & mask) != mask)
        return;

    for (i = idx & (~mask); i <= idx; i++)
        macb.rx_ring[i << shift].addr &= ~MACB_BIT(RX_USED);
}

void reclaim_rx_buffers(unsigned int new_tail) {
    unsigned int i;
    i = macb.rx_tail;

    // macb_invalidate_ring_desc(macb, RX);
    while (i > new_tail) {
        reclaim_rx_buffer(i);
        i++;
        if (i >= MACB_RX_RING_SIZE)
            i = 0;
    }

    while (i < new_tail) {
        reclaim_rx_buffer(i);
        i++;
    }

    barrier();
    flush_dcache_range(macb.rx_ring_dma[0], MACB_RX_DMA_DESC_SIZE);
    macb.rx_tail = new_tail;
}


int _macb_recv(unsigned char **packetp) {
    macb.next_rx_tail = macb.rx_tail;
	macb.wrapped = 0;
    unsigned int next_rx_tail = macb.next_rx_tail;
    void *buffer;
    int length;
    m_uint32 status;
    m_uint8 flag = 0;

    macb.wrapped = 0;
    for (;;) {
        // macb_invalidate_ring_desc(macb, RX);

        if (!(macb.rx_ring[next_rx_tail].addr & MACB_BIT(RX_USED)))
            return -1;

        status = macb.rx_ring[next_rx_tail].ctrl;
        if (status & MACB_BIT(RX_SOF)) {
            if (next_rx_tail != macb.rx_tail)
                reclaim_rx_buffers(next_rx_tail);
            macb.wrapped = 0;
        }

        if (status & MACB_BIT(RX_EOF)) {
            buffer = macb.rx_buffer + macb.rx_buffer_size * macb.rx_tail;
            length = status & RXBUF_FRMLEN_MASK;

            // macb_invalidate_rx_buffer(macb);
            if (macb.wrapped) {
		    printf("Wrapped! len: %d", length);
                unsigned int headlen, taillen;

                headlen = macb.rx_buffer_size * (MACB_RX_RING_SIZE - macb.rx_tail);
                taillen = length - headlen;
                memcpy((void *)*packetp, buffer, headlen);
                memcpy((void *)*packetp + headlen, macb.rx_buffer, taillen);
            } else {
                memcpy((void *)*packetp, buffer, length);
            }

            if (++next_rx_tail >= MACB_RX_RING_SIZE)
                next_rx_tail = 0;
            macb.next_rx_tail = next_rx_tail;
            return length;
        } else {
            if (++next_rx_tail >= MACB_RX_RING_SIZE) {
                macb.wrapped = 1;
                next_rx_tail = 0;
            }
        }
        barrier();
    }
}

void macb_test() {
	m_uint8 dummy_frame[60] = {
	    // Destination MAC (Broadcast)
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	    // Source MAC (your device's MAC)
	    0x02, 0x00, 0x00, 0x00, 0x00, 0x01,

	    // Ethertype (0x88B5 = Dummy/Experimental)
	    0x88, 0xB5,

	    // Payload (46 bytes of arbitrary data)
	    'H', 'E', 'L', 'L', 'O', '-', 'F', 'R', 'O', 'M', '-', 'G', 'E', 'M', '-', 'D',
	    'U', 'M', 'M', 'Y', '-', 'P', 'A', 'C', 'K', 'E', 'T', '-', 'T', 'E', 'S', 'T',
	    '-', 'Y', 'A', 'Y', '!', ' ', 'i', 't', 'r', ':', ' ', 0x00, 0x00,
	};
	int itr = 5;
	for (int i = 0;i < 5;++i) {
		dummy_frame[58] = ('1' + i);
		macb_send(60, &dummy_frame);
	}

	int num = 0;
	unsigned char buffer[2000];
	unsigned char* recv_buffer = buffer;
	while(1) {
		num = _macb_recv(&recv_buffer);
		reclaim_rx_buffers(macb.next_rx_tail);
		if (num > 0) {
			printf("Received: %d bytes\n", num);
			for (int i = 0;i < num;++i) {
				printf("%c", buffer[i]);
			}
			printf("\n");
		}
		delay(1e5);
	}
}

int macb_recv(void* buffer) {
	unsigned char* buf = (unsigned char*)buffer;
	int len = _macb_recv(&buf);
	reclaim_rx_buffers(macb.next_rx_tail);

	return len;
}

void macb_init() {

    #ifdef NETON
        macb_probe();
        macb_start();
//	macb_test();
//	SUCCESS("Finished TEST successfully");
    #endif
	
	earth->net_send = macb_send;
	earth->net_recv = macb_recv;

}

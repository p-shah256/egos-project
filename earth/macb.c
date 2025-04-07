#include <stdlib.h>
#include "egos.h"
#include "macb.h"

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

#define ARCH_DMA_MINALIGN   16
#define DMA_DESC_SIZE		16
#define DMA_DESC_BYTES(n)	((n) * DMA_DESC_SIZE)
#define MACB_TX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_TX_RING_SIZE))
#define MACB_RX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_RX_RING_SIZE))
#define MACB_TX_DUMMY_DMA_DESC_SIZE	(DMA_DESC_BYTES(1))

// #define DESC_PER_CACHELINE_32	(ARCH_DMA_MINALIGN/sizeof(struct macb_dma_desc))
#define DESC_PER_CACHELINE_64	(ARCH_DMA_MINALIGN/DMA_DESC_SIZE)

#define RXBUF_FRMLEN_MASK	0x00000fff
#define TXBUF_FRMLEN_MASK	0x000007ff

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

static void* alloc_aligned(m_uint32 size, void* handle) {
    int offset = ARCH_DMA_MINALIGN - 1 + sizeof(void*);
    if ((handle = (void*)malloc(size + offset)) == NULL) {
       FATAL("Not enough memory");
    }
    return (void**)(((size_t)(handle) + offset) & ~(ARCH_DMA_MINALIGN - 1));
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
	// macb_flush_ring_desc(macb, RX);
	// macb_flush_rx_buffer(macb);

	for (i = 0; i < MACB_TX_RING_SIZE; i++) {
		macb.tx_ring[i].addr = 0;
		if (i == (MACB_TX_RING_SIZE - 1))
			macb.tx_ring[i].ctrl = MACB_BIT(TX_USED) |
				MACB_BIT(TX_WRAP);
		else
			macb.tx_ring[i].ctrl = MACB_BIT(TX_USED);
	}
	// macb_flush_ring_desc(macb, TX);

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


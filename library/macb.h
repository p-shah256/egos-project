/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005-2006 Atmel Corporation
 */
#ifndef __DRIVERS_MACB_H__
#define __DRIVERS_MACB_H__

#define MACB_GREGS_NBR     16
#define MACB_GREGS_VERSION 2
#define MACB_MAX_QUEUES    8

/* Generic MII registers. */
#define MII_BMCR        0x00 /* Basic mode control register */
#define MII_BMSR        0x01 /* Basic mode status register  */
#define MII_PHYSID1     0x02 /* PHYS ID 1                   */
#define MII_PHYSID2     0x03 /* PHYS ID 2                   */
#define MII_ADVERTISE   0x04 /* Advertisement control reg   */
#define MII_LPA         0x05 /* Link partner ability reg    */
#define MII_EXPANSION   0x06 /* Expansion register          */
#define MII_CTRL1000    0x09 /* 1000BASE-T control          */
#define MII_STAT1000    0x0a /* 1000BASE-T status           */
#define MII_MMD_CTRL    0x0d /* MMD Access Control Register */
#define MII_MMD_DATA    0x0e /* MMD Access Data Register */
#define MII_ESTATUS     0x0f /* Extended Status             */
#define MII_DCOUNTER    0x12 /* Disconnect counter          */
#define MII_FCSCOUNTER  0x13 /* False carrier counter       */
#define MII_NWAYTEST    0x14 /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER 0x15 /* Receive error counter       */
#define MII_SREVISION   0x16 /* Silicon revision            */
#define MII_RESV1       0x17 /* Reserved...                 */
#define MII_LBRERROR    0x18 /* Lpback, rx, bypass error    */
#define MII_PHYADDR     0x19 /* PHY address                 */
#define MII_RESV2       0x1a /* Reserved...                 */
#define MII_TPISTATUS   0x1b /* TPI status for 10mbps       */
#define MII_NCONFIG     0x1c /* Network interface config    */

/* Link Registers */
#define BMSR_LSTATUS      0x0004 /* Link status                 */
#define ADVERTISE_CSMA    0x0001 /* Only selector supported     */
#define ADVERTISE_100FULL 0x0100 /* Try for 100mbps full-duplex */
#define ADVERTISE_100HALF 0x0080 /* Try for 100mbps half-duplex */
#define ADVERTISE_100FULL 0x0100 /* Try for 100mbps full-duplex */
#define ADVERTISE_10FULL  0x0040 /* Try for 10mbps full-duplex  */
#define ADVERTISE_FULL    (ADVERTISE_100FULL | ADVERTISE_10FULL | ADVERTISE_CSMA)

/* Link partner ability register. */
#define LPA_SLCT            0x001f /* Same as advertise selector  */
#define LPA_10HALF          0x0020 /* Can do 10mbps half-duplex   */
#define LPA_1000XFULL       0x0020 /* Can do 1000BASE-X full-duplex */
#define LPA_10FULL          0x0040 /* Can do 10mbps full-duplex   */
#define LPA_1000XHALF       0x0040 /* Can do 1000BASE-X half-duplex */
#define LPA_100HALF         0x0080 /* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE      0x0080 /* Can do 1000BASE-X pause     */
#define LPA_100FULL         0x0100 /* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM 0x0100 /* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4        0x0200 /* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP       0x0400 /* Can pause                   */
#define LPA_PAUSE_ASYM      0x0800 /* Can pause asymetrically     */
#define LPA_RESV            0x1000 /* Unused...                   */
#define LPA_RFAULT          0x2000 /* Link partner faulted        */
#define LPA_LPACK           0x4000 /* Link partner acked us       */
#define LPA_NPAGE           0x8000 /* Next page bit               */

/* MACB register offsets */
#define MACB_NCR   0x0000 /* Network Control */
#define MACB_NCFGR 0x0004 /* Network Config */
#define MACB_NSR   0x0008 /* Network Status */
#define MACB_TAR   0x000c /* AT91RM9200 only */
#define MACB_TCR   0x0010 /* AT91RM9200 only */
#define MACB_TSR   0x0014 /* Transmit Status */
#define MACB_RBQP  0x0018 /* RX Q Base Address */
#define MACB_TBQP  0x001c /* TX Q Base Address */
#define MACB_RSR   0x0020 /* Receive Status */
#define MACB_ISR   0x0024 /* Interrupt Status */
#define MACB_IER   0x0028 /* Interrupt Enable */
#define MACB_IDR   0x002c /* Interrupt Disable */
#define MACB_IMR   0x0030 /* Interrupt Mask */
#define MACB_MAN   0x0034 /* PHY Maintenance */
#define MACB_PTR   0x0038
#define MACB_PFR   0x003c
#define MACB_FTO   0x0040
#define MACB_SCF   0x0044
#define MACB_MCF   0x0048
#define MACB_FRO   0x004c
#define MACB_FCSE  0x0050
#define MACB_ALE   0x0054
#define MACB_DTF   0x0058
#define MACB_LCOL  0x005c
#define MACB_EXCOL 0x0060
#define MACB_TUND  0x0064
#define MACB_CSE   0x0068
#define MACB_RRE   0x006c
#define MACB_ROVR  0x0070
#define MACB_RSE   0x0074
#define MACB_ELE   0x0078
#define MACB_RJA   0x007c
#define MACB_USF   0x0080
#define MACB_STE   0x0084
#define MACB_RLE   0x0088
#define MACB_TPF   0x008c
#define MACB_HRB   0x0090
#define MACB_HRT   0x0094
#define MACB_SA1B  0x0098
#define MACB_SA1T  0x009c
#define MACB_SA2B  0x00a0
#define MACB_SA2T  0x00a4
#define MACB_SA3B  0x00a8
#define MACB_SA3T  0x00ac
#define MACB_SA4B  0x00b0
#define MACB_SA4T  0x00b4
#define MACB_TID   0x00b8
#define MACB_TPQ   0x00bc
#define MACB_USRIO 0x00c0
#define MACB_WOL   0x00c4
#define MACB_MID   0x00fc
#define MACB_TBQPH 0x04C8
#define MACB_RBQPH 0x04D4

/* Bitfields in NCR */
#define MACB_LB_OFFSET      0 /* reserved */
#define MACB_LB_SIZE        1
#define MACB_LLB_OFFSET     1 /* Loop back local */
#define MACB_LLB_SIZE       1
#define MACB_RE_OFFSET      2 /* Receive enable */
#define MACB_RE_SIZE        1
#define MACB_TE_OFFSET      3 /* Transmit enable */
#define MACB_TE_SIZE        1
#define MACB_MPE_OFFSET     4 /* Management port enable */
#define MACB_MPE_SIZE       1
#define MACB_CLRSTAT_OFFSET 5 /* Clear stats regs */
#define MACB_CLRSTAT_SIZE   1
#define MACB_INCSTAT_OFFSET 6 /* Incremental stats regs */
#define MACB_INCSTAT_SIZE   1
#define MACB_WESTAT_OFFSET  7 /* Write enable stats regs */
#define MACB_WESTAT_SIZE    1
#define MACB_BP_OFFSET      8 /* Back pressure */
#define MACB_BP_SIZE        1
#define MACB_TSTART_OFFSET  9 /* Start transmission */
#define MACB_TSTART_SIZE    1
#define MACB_THALT_OFFSET   10 /* Transmit halt */
#define MACB_THALT_SIZE     1
#define MACB_NCR_TPF_OFFSET 11 /* Transmit pause frame */
#define MACB_NCR_TPF_SIZE   1
#define MACB_TZQ_OFFSET     12 /* Transmit zero quantum pause frame */
#define MACB_TZQ_SIZE       1
#define MACB_SRTSM_OFFSET   15
#define MACB_OSSMODE_OFFSET 24 /* Enable One Step Synchro Mode */
#define MACB_OSSMODE_SIZE   1

/* Bitfields in NCFGR */
#define MACB_SPD_OFFSET         0 /* Speed */
#define MACB_SPD_SIZE           1
#define MACB_FD_OFFSET          1 /* Full duplex */
#define MACB_FD_SIZE            1
#define MACB_BIT_RATE_OFFSET    2 /* Discard non-VLAN frames */
#define MACB_BIT_RATE_SIZE      1
#define MACB_JFRAME_OFFSET      3 /* reserved */
#define MACB_JFRAME_SIZE        1
#define MACB_CAF_OFFSET         4 /* Copy all frames */
#define MACB_CAF_SIZE           1
#define MACB_NBC_OFFSET         5 /* No broadcast */
#define MACB_NBC_SIZE           1
#define MACB_NCFGR_MTI_OFFSET   6 /* Multicast hash enable */
#define MACB_NCFGR_MTI_SIZE     1
#define MACB_UNI_OFFSET         7 /* Unicast hash enable */
#define MACB_UNI_SIZE           1
#define MACB_BIG_OFFSET         8 /* Receive 1536 byte frames */
#define MACB_BIG_SIZE           1
#define MACB_EAE_OFFSET         9 /* External address match enable */
#define MACB_EAE_SIZE           1
#define MACB_CLK_OFFSET         10
#define MACB_CLK_SIZE           2
#define MACB_RTY_OFFSET         12 /* Retry test */
#define MACB_RTY_SIZE           1
#define MACB_PAE_OFFSET         13 /* Pause enable */
#define MACB_PAE_SIZE           1
#define MACB_RM9200_RMII_OFFSET 13 /* AT91RM9200 only */
#define MACB_RM9200_RMII_SIZE   1  /* AT91RM9200 only */
#define MACB_RBOF_OFFSET        14 /* Receive buffer offset */
#define MACB_RBOF_SIZE          2
#define MACB_RLCE_OFFSET        16 /* Length field error frame discard */
#define MACB_RLCE_SIZE          1
#define MACB_DRFCS_OFFSET       17 /* FCS remove */
#define MACB_DRFCS_SIZE         1
#define MACB_EFRHD_OFFSET       18
#define MACB_EFRHD_SIZE         1
#define MACB_IRXFCS_OFFSET      19
#define MACB_IRXFCS_SIZE        1

/* Bitfields in NSR */
#define MACB_NSR_LINK_OFFSET 0 /* pcs_link_state */
#define MACB_NSR_LINK_SIZE   1
#define MACB_MDIO_OFFSET     1 /* status of the mdio_in pin */
#define MACB_MDIO_SIZE       1
#define MACB_IDLE_OFFSET     2 /* The PHY management logic is idle */
#define MACB_IDLE_SIZE       1

/* Bitfields in TSR */
#define MACB_UBR_OFFSET        0 /* Used bit read */
#define MACB_UBR_SIZE          1
#define MACB_COL_OFFSET        1 /* Collision occurred */
#define MACB_COL_SIZE          1
#define MACB_TSR_RLE_OFFSET    2 /* Retry limit exceeded */
#define MACB_TSR_RLE_SIZE      1
#define MACB_TGO_OFFSET        3 /* Transmit go */
#define MACB_TGO_SIZE          1
#define MACB_BEX_OFFSET        4 /* TX frame corruption due to AHB error */
#define MACB_BEX_SIZE          1
#define MACB_RM9200_BNQ_OFFSET 4 /* AT91RM9200 only */
#define MACB_RM9200_BNQ_SIZE   1 /* AT91RM9200 only */
#define MACB_COMP_OFFSET       5 /* Trnasmit complete */
#define MACB_COMP_SIZE         1
#define MACB_UND_OFFSET        6 /* Trnasmit under run */
#define MACB_UND_SIZE          1

/* Bitfields in RSR */
#define MACB_BNA_OFFSET 0 /* Buffer not available */
#define MACB_BNA_SIZE   1
#define MACB_REC_OFFSET 1 /* Frame received */
#define MACB_REC_SIZE   1
#define MACB_OVR_OFFSET 2 /* Receive overrun */
#define MACB_OVR_SIZE   1

/* Bitfields in ISR/IER/IDR/IMR */
#define MACB_MFD_OFFSET      0 /* Management frame sent */
#define MACB_MFD_SIZE        1
#define MACB_RCOMP_OFFSET    1 /* Receive complete */
#define MACB_RCOMP_SIZE      1
#define MACB_RXUBR_OFFSET    2 /* RX used bit read */
#define MACB_RXUBR_SIZE      1
#define MACB_TXUBR_OFFSET    3 /* TX used bit read */
#define MACB_TXUBR_SIZE      1
#define MACB_ISR_TUND_OFFSET 4 /* Enable TX buffer under run interrupt */
#define MACB_ISR_TUND_SIZE   1
#define MACB_ISR_RLE_OFFSET  5 /* EN retry exceeded/late coll interrupt */
#define MACB_ISR_RLE_SIZE    1
#define MACB_TXERR_OFFSET    6 /* EN TX frame corrupt from error interrupt */
#define MACB_TXERR_SIZE      1
#define MACB_TCOMP_OFFSET    7 /* Enable transmit complete interrupt */
#define MACB_TCOMP_SIZE      1
#define MACB_ISR_LINK_OFFSET 9 /* Enable link change interrupt */
#define MACB_ISR_LINK_SIZE   1
#define MACB_ISR_ROVR_OFFSET 10 /* Enable receive overrun interrupt */
#define MACB_ISR_ROVR_SIZE   1
#define MACB_HRESP_OFFSET    11 /* Enable hrsep not OK interrupt */
#define MACB_HRESP_SIZE      1
#define MACB_PFR_OFFSET      12 /* Enable pause frame w/ quantum interrupt */
#define MACB_PFR_SIZE        1
#define MACB_PTZ_OFFSET      13 /* Enable pause time zero interrupt */
#define MACB_PTZ_SIZE        1
#define MACB_WOL_OFFSET      14 /* Enable wake-on-lan interrupt */
#define MACB_WOL_SIZE        1
#define MACB_DRQFR_OFFSET    18 /* PTP Delay Request Frame Received */
#define MACB_DRQFR_SIZE      1
#define MACB_SFR_OFFSET      19 /* PTP Sync Frame Received */
#define MACB_SFR_SIZE        1
#define MACB_DRQFT_OFFSET    20 /* PTP Delay Request Frame Transmitted */
#define MACB_DRQFT_SIZE      1
#define MACB_SFT_OFFSET      21 /* PTP Sync Frame Transmitted */
#define MACB_SFT_SIZE        1
#define MACB_PDRQFR_OFFSET   22 /* PDelay Request Frame Received */
#define MACB_PDRQFR_SIZE     1
#define MACB_PDRSFR_OFFSET   23 /* PDelay Response Frame Received */
#define MACB_PDRSFR_SIZE     1
#define MACB_PDRQFT_OFFSET   24 /* PDelay Request Frame Transmitted */
#define MACB_PDRQFT_SIZE     1
#define MACB_PDRSFT_OFFSET   25 /* PDelay Response Frame Transmitted */
#define MACB_PDRSFT_SIZE     1
#define MACB_SRI_OFFSET      26 /* TSU Seconds Register Increment */
#define MACB_SRI_SIZE        1

/* Timer increment fields */
#define MACB_TI_CNS_OFFSET  0
#define MACB_TI_CNS_SIZE    8
#define MACB_TI_ACNS_OFFSET 8
#define MACB_TI_ACNS_SIZE   8
#define MACB_TI_NIT_OFFSET  16
#define MACB_TI_NIT_SIZE    8

/* Bitfields in MAN */
#define MACB_DATA_OFFSET 0 /* data */
#define MACB_DATA_SIZE   16
#define MACB_CODE_OFFSET 16 /* Must be written to 10 */
#define MACB_CODE_SIZE   2
#define MACB_REGA_OFFSET 18 /* Register address */
#define MACB_REGA_SIZE   5
#define MACB_PHYA_OFFSET 23 /* PHY address */
#define MACB_PHYA_SIZE   5
#define MACB_RW_OFFSET   28 /* Operation. 10 is read. 01 is write. */
#define MACB_RW_SIZE     2
#define MACB_SOF_OFFSET  30 /* Must be written to 1 for Clause 22 */
#define MACB_SOF_SIZE    2

/* Bitfields in USRIO (AVR32) */
#define MACB_MII_OFFSET           0
#define MACB_MII_SIZE             1
#define MACB_EAM_OFFSET           1
#define MACB_EAM_SIZE             1
#define MACB_TX_PAUSE_OFFSET      2
#define MACB_TX_PAUSE_SIZE        1
#define MACB_TX_PAUSE_ZERO_OFFSET 3
#define MACB_TX_PAUSE_ZERO_SIZE   1

/* Bitfields in USRIO (AT91) */
#define MACB_RMII_OFFSET  0
#define MACB_RMII_SIZE    1
#define GEM_RGMII_OFFSET  0 /* GEM gigabit mode */
#define GEM_RGMII_SIZE    1
#define MACB_CLKEN_OFFSET 1
#define MACB_CLKEN_SIZE   1

/* Bitfields in WOL */
#define MACB_IP_OFFSET      0
#define MACB_IP_SIZE        16
#define MACB_MAG_OFFSET     16
#define MACB_MAG_SIZE       1
#define MACB_ARP_OFFSET     17
#define MACB_ARP_SIZE       1
#define MACB_SA1_OFFSET     18
#define MACB_SA1_SIZE       1
#define MACB_WOL_MTI_OFFSET 19
#define MACB_WOL_MTI_SIZE   1

/* Bitfields in MID */
#define MACB_IDNUM_OFFSET 16
#define MACB_IDNUM_SIZE   12
#define MACB_REV_OFFSET   0
#define MACB_REV_SIZE     16

/* offset from EtherType to IP address */
#define ETYPE_SRCIP_OFFSET 12
#define ETYPE_DSTIP_OFFSET 16

/* offset from IP header to port */
#define IPHDR_SRCPORT_OFFSET 0
#define IPHDR_DSTPORT_OFFSET 2

/* Constants for CLK */
#define MACB_CLK_DIV8  0
#define MACB_CLK_DIV16 1
#define MACB_CLK_DIV32 2
#define MACB_CLK_DIV64 3

/* Constants for MAN register */
#define MACB_MAN_SOF   1
#define MACB_MAN_WRITE 1
#define MACB_MAN_READ  2
#define MACB_MAN_CODE  2

/* Capability mask bits */
#define MACB_CAPS_ISR_CLEAR_ON_WRITE        0x00000001
#define MACB_CAPS_USRIO_HAS_CLKEN           0x00000002
#define MACB_CAPS_USRIO_DEFAULT_IS_MII_GMII 0x00000004
#define MACB_CAPS_NO_GIGABIT_HALF           0x00000008
#define MACB_CAPS_USRIO_DISABLED            0x00000010
#define MACB_CAPS_JUMBO                     0x00000020
#define MACB_CAPS_GEM_HAS_PTP               0x00000040
#define MACB_CAPS_BD_RD_PREFETCH            0x00000080
#define MACB_CAPS_NEEDS_RSTONUBR            0x00000100
#define MACB_CAPS_FIFO_MODE                 0x10000000
#define MACB_CAPS_GIGABIT_MODE_AVAILABLE    0x20000000
#define MACB_CAPS_SG_DISABLED               0x40000000
#define MACB_CAPS_MACB_IS_GEM               0x80000000

/* LSO settings */
#define MACB_LSO_UFO_ENABLE 0x01
#define MACB_LSO_TSO_ENABLE 0x02

/* Bit manipulation macros */
#define MACB_BIT(name)          (1 << MACB_##name##_OFFSET)
#define MACB_BF(name, value)    (((value) & ((1 << MACB_##name##_SIZE) - 1)) << MACB_##name##_OFFSET)
#define MACB_BFEXT(name, value) (((value) >> MACB_##name##_OFFSET) & ((1 << MACB_##name##_SIZE) - 1))
#define MACB_BFINS(name, value, old)                                                                                   \
	(((old) & ~(((1 << MACB_##name##_SIZE) - 1) << MACB_##name##_OFFSET)) | MACB_BF(name, value))

/* Register access macros */
#define macb_readl(port, reg)         REGW((port).base, MACB_##reg)
#define macb_writel(port, reg, value) REGW((port).base, MACB_##reg) = value

/* DMA descriptor bitfields */
#define MACB_RX_USED_OFFSET  0
#define MACB_RX_USED_SIZE    1
#define MACB_RX_WRAP_OFFSET  1
#define MACB_RX_WRAP_SIZE    1
#define MACB_RX_WADDR_OFFSET 2
#define MACB_RX_WADDR_SIZE   30

#define MACB_RX_FRMLEN_OFFSET       0
#define MACB_RX_FRMLEN_SIZE         12
#define MACB_RX_OFFSET_OFFSET       12
#define MACB_RX_OFFSET_SIZE         2
#define MACB_RX_SOF_OFFSET          14
#define MACB_RX_SOF_SIZE            1
#define MACB_RX_EOF_OFFSET          15
#define MACB_RX_EOF_SIZE            1
#define MACB_RX_CFI_OFFSET          16
#define MACB_RX_CFI_SIZE            1
#define MACB_RX_VLAN_PRI_OFFSET     17
#define MACB_RX_VLAN_PRI_SIZE       3
#define MACB_RX_PRI_TAG_OFFSET      20
#define MACB_RX_PRI_TAG_SIZE        1
#define MACB_RX_VLAN_TAG_OFFSET     21
#define MACB_RX_VLAN_TAG_SIZE       1
#define MACB_RX_TYPEID_MATCH_OFFSET 22
#define MACB_RX_TYPEID_MATCH_SIZE   1
#define MACB_RX_SA4_MATCH_OFFSET    23
#define MACB_RX_SA4_MATCH_SIZE      1
#define MACB_RX_SA3_MATCH_OFFSET    24
#define MACB_RX_SA3_MATCH_SIZE      1
#define MACB_RX_SA2_MATCH_OFFSET    25
#define MACB_RX_SA2_MATCH_SIZE      1
#define MACB_RX_SA1_MATCH_OFFSET    26
#define MACB_RX_SA1_MATCH_SIZE      1
#define MACB_RX_EXT_MATCH_OFFSET    28
#define MACB_RX_EXT_MATCH_SIZE      1
#define MACB_RX_UHASH_MATCH_OFFSET  29
#define MACB_RX_UHASH_MATCH_SIZE    1
#define MACB_RX_MHASH_MATCH_OFFSET  30
#define MACB_RX_MHASH_MATCH_SIZE    1
#define MACB_RX_BROADCAST_OFFSET    31
#define MACB_RX_BROADCAST_SIZE      1

#define MACB_RX_FRMLEN_MASK  0xFFF
#define MACB_RX_JFRMLEN_MASK 0x3FFF

#define MACB_TX_FRMLEN_OFFSET        0
#define MACB_TX_FRMLEN_SIZE          11
#define MACB_TX_LAST_OFFSET          15
#define MACB_TX_LAST_SIZE            1
#define MACB_TX_NOCRC_OFFSET         16
#define MACB_TX_NOCRC_SIZE           1
#define MACB_MSS_MFS_OFFSET          16
#define MACB_MSS_MFS_SIZE            14
#define MACB_TX_LSO_OFFSET           17
#define MACB_TX_LSO_SIZE             2
#define MACB_TX_TCP_SEQ_SRC_OFFSET   19
#define MACB_TX_TCP_SEQ_SRC_SIZE     1
#define MACB_TX_BUF_EXHAUSTED_OFFSET 27
#define MACB_TX_BUF_EXHAUSTED_SIZE   1
#define MACB_TX_UNDERRUN_OFFSET      28
#define MACB_TX_UNDERRUN_SIZE        1
#define MACB_TX_ERROR_OFFSET         29
#define MACB_TX_ERROR_SIZE           1
#define MACB_TX_WRAP_OFFSET          30
#define MACB_TX_WRAP_SIZE            1
#define MACB_TX_USED_OFFSET          31
#define MACB_TX_USED_SIZE            1

#endif /* __DRIVERS_MACB_H__ */

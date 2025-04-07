## Core Parts MACB

<img width="706" alt="GEMGXL Management TX Clock Select Register" src="https://github.com/user-attachments/assets/bfe7e84b-006b-464e-b810-b08e99108700" />

<img width="682" alt="Table 123 GEMGXL Management Control Status Speed Mode Register" src="https://github.com/user-attachments/assets/ea787c82-fa31-4d33-b557-b1b54dd51526" />

### GEMGXL Control Registers (0x1009_0000 – 0x1009_1FFF)

Reference: https://github.com/u-boot/u-boot/blob/master/doc/develop/driver-model/ethernet.rst

Doc: https://pix-server-sorel.luoss.fr/Manual/Pi/GigabitEthernetMAC%28GEM%29-TechnicalDataSheet-Cadence.pdf

SiFive: https://naizhengtan.github.io/25spring/docs/sifive-fu540-v1p4.pdf

Register Offsets: https://github.com/u-boot/u-boot/blob/master/drivers/net/macb.h

- [ ] Probe
- Finalize on PHY interface (MII/GMII)
- Enable/Setup clk
- Allocate buffers and DMA desc

- [ ] Start
- Initialize DMA desc
- Write RX and TX desc in required regs RBQP and TBQP
- Setup PHY and update ETH controller
- Enable TX and RX.

For the initial implementation we will focus on implementing send and receive as synchronous operations rather than async with interrupts. If time permits will move on to PLIC.

- [ ] Send
- Setup of ctrl and desc addr in TX desc to indicate the buffer to use.
- Wait for a while till the desc is used with some timeout
- Based on the result log success/failure

- [ ] Recv
- Clean up of buffers if required
- Find SOF and EOF
- Calculate length and copy data from buffers

- [ ] Free Packet
- reclaim RX buffers

- [ ] Stop
- HALT the controller and wait for pending TXs
- Disable TX and RX

- [ ] write_hwaddr (Optional)
- Write MAC address into ethernet controller using SA1B and SA1T regs.

## TCP/IP

uIP vs lwIP ?

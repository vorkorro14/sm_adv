#include "ethmod.h"


/*
 * SPI
 */

uint8_t ethmod_current_bank = 0;
uint16_t ethmod_rxrdpt = 0;

#define ethmod_select() ETHMOD_SPI_PORT &= ~ETHMOD_SPI_CS
#define ethmod_release() ETHMOD_SPI_PORT |= ETHMOD_SPI_CS

uint8_t ethmod_rxtx(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF)))
		;
	return SPDR;
}

#define ethmod_rx() ethmod_rxtx(0xff)
#define ethmod_tx(data) ethmod_rxtx(data)

// Generic SPI read command
uint8_t ethmod_read_op(uint8_t cmd, uint8_t adr)
{
	uint8_t data;

	ethmod_select();
	ethmod_tx(cmd | (adr & ETHMOD_ADDR_MASK));
	if(adr & 0x80) // throw out dummy byte 
		ethmod_rx(); // when reading MII/MAC register
	data = ethmod_rx();
	ethmod_release();
	return data;
}

// Generic SPI write command
void ethmod_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
	ethmod_select();
	ethmod_tx(cmd | (adr & ETHMOD_ADDR_MASK));
	ethmod_tx(data);
	ethmod_release();
}

// Initiate software reset
void ethmod_soft_reset()
{
	ethmod_select();
	ethmod_tx(ETHMOD_SPI_SC);
	ethmod_release();
	
	ethmod_current_bank = 0;
	_delay_ms(1); // Wait until device initializes
}


/*
 * Memory access
 */

// Set register bank
void ethmod_set_bank(uint8_t adr)
{
	uint8_t bank;

	if( (adr & ETHMOD_ADDR_MASK) < ETHMOD_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != ethmod_current_bank)
		{
			ethmod_write_op(ETHMOD_SPI_BFC, ECON1, 0x03);
			ethmod_write_op(ETHMOD_SPI_BFS, ECON1, bank);
			ethmod_current_bank = bank;
		}
	}
}

// Read register
uint8_t ethmod_rcr(uint8_t adr)
{
	ethmod_set_bank(adr);
	return ethmod_read_op(ETHMOD_SPI_RCR, adr);
}

// Read register pair
uint16_t ethmod_rcr16(uint8_t adr)
{
	ethmod_set_bank(adr);
	return ethmod_read_op(ETHMOD_SPI_RCR, adr) |
		(ethmod_read_op(ETHMOD_SPI_RCR, adr+1) << 8);
}

// Write register
void ethmod_wcr(uint8_t adr, uint8_t arg)
{
	ethmod_set_bank(adr);
	ethmod_write_op(ETHMOD_SPI_WCR, adr, arg);
}

// Write register pair
void ethmod_wcr16(uint8_t adr, uint16_t arg)
{
	ethmod_set_bank(adr);
	ethmod_write_op(ETHMOD_SPI_WCR, adr, arg);
	ethmod_write_op(ETHMOD_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void ethmod_bfc(uint8_t adr, uint8_t mask)
{
	ethmod_set_bank(adr);
	ethmod_write_op(ETHMOD_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void ethmod_bfs(uint8_t adr, uint8_t mask)
{
	ethmod_set_bank(adr);
	ethmod_write_op(ETHMOD_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void ethmod_read_buffer(uint8_t *buf, uint16_t len)
{
	ethmod_select();
	ethmod_tx(ETHMOD_SPI_RBM);
	while(len--)
		*(buf++) = ethmod_rx();
	ethmod_release();
}

// Write Rx/Tx buffer (at EWRPT)
void ethmod_write_buffer(uint8_t *buf, uint16_t len)
{
	ethmod_select();
	ethmod_tx(ETHMOD_SPI_WBM);
	while(len--)
		ethmod_tx(*(buf++));
	ethmod_release();
}

// Read PHY register
uint16_t ethmod_read_phy(uint8_t adr)
{
	ethmod_wcr(MIREGADR, adr);
	ethmod_bfs(MICMD, MICMD_MIIRD);
	while(ethmod_rcr(MISTAT) & MISTAT_BUSY)
		;
	ethmod_bfc(MICMD, MICMD_MIIRD);
	return ethmod_rcr16(MIRD);
}

// Write PHY register
void ethmod_write_phy(uint8_t adr, uint16_t data)
{
	ethmod_wcr(MIREGADR, adr);
	ethmod_wcr16(MIWR, data);
	while(ethmod_rcr(MISTAT) & MISTAT_BUSY)
		;
}


/*
 * Init & packet Rx/Tx
 */

void ethmod_init(uint8_t *macadr)
{
	// Initialize SPI
	ETHMOD_SPI_DDR |= ETHMOD_SPI_CS|ETHMOD_SPI_MOSI|ETHMOD_SPI_SCK;
	ETHMOD_SPI_DDR &= ~ETHMOD_SPI_MISO;
	ethmod_release();

	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR |= (1<<SPI2X); // Maximum speed

	// Reset ETHMOD
	ethmod_soft_reset();

	// Setup Rx/Tx buffer
	ethmod_wcr16(ERXST, ETHMOD_RXSTART);
	ethmod_wcr16(ERXRDPT, ETHMOD_RXSTART);
	ethmod_wcr16(ERXND, ETHMOD_RXEND);
	ethmod_rxrdpt = ETHMOD_RXSTART;

	// Setup MAC
	ethmod_wcr(MACON1, MACON1_TXPAUS| // Enable flow control
		MACON1_RXPAUS|MACON1_MARXEN); // Enable MAC Rx
	ethmod_wcr(MACON2, 0); // Clear reset
	ethmod_wcr(MACON3, MACON3_PADCFG0| // Enable padding,
		MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX); // Enable crc & frame len chk
	ethmod_wcr16(MAMXFL, ETHMOD_MAXFRAME);
	ethmod_wcr(MABBIPG, 0x15); // Set inter-frame gap
	ethmod_wcr(MAIPGL, 0x12);
	ethmod_wcr(MAIPGH, 0x0c);
	ethmod_wcr(MAADR5, macadr[0]); // Set MAC address
	ethmod_wcr(MAADR4, macadr[1]);
	ethmod_wcr(MAADR3, macadr[2]);
	ethmod_wcr(MAADR2, macadr[3]);
	ethmod_wcr(MAADR1, macadr[4]);
	ethmod_wcr(MAADR0, macadr[5]);

	// Setup PHY
	ethmod_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
	ethmod_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback
	ethmod_write_phy(PHLCON, PHLCON_LACFG2| // Configure LED ctrl
		PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|
		PHLCON_LFRQ0|PHLCON_STRCH);

	// Enable Rx packets
	ethmod_bfs(ECON1, ECON1_RXEN);
}

void ethmod_send_packet(uint8_t *data, uint16_t len)
{
	while(ethmod_rcr(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ETHMOD bug. We must reset
		// transmit logic in cause of Tx error
		if(ethmod_rcr(EIR) & EIR_TXERIF)
		{
			ethmod_bfs(ECON1, ECON1_TXRST);
			ethmod_bfc(ECON1, ECON1_TXRST);
		}
	}

	ethmod_wcr16(EWRPT, ETHMOD_TXSTART);
	ethmod_write_buffer((uint8_t*)"\x00", 1);
	ethmod_write_buffer(data, len);

	ethmod_wcr16(ETXST, ETHMOD_TXSTART);
	ethmod_wcr16(ETXND, ETHMOD_TXSTART + len);

	ethmod_bfs(ECON1, ECON1_TXRTS); // Request packet send
}

uint16_t ethmod_recv_packet(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0, rxlen, status, temp;

	if(ethmod_rcr(EPKTCNT))
	{
		ethmod_wcr16(ERDPT, ethmod_rxrdpt);

		ethmod_read_buffer((void*)&ethmod_rxrdpt, sizeof(ethmod_rxrdpt));
		ethmod_read_buffer((void*)&rxlen, sizeof(rxlen));
		ethmod_read_buffer((void*)&status, sizeof(status));

		if(status & 0x80) //success
		{
			len = rxlen - 4; //throw out crc
			if(len > buflen) len = buflen;
			ethmod_read_buffer(buf, len);	
		}

		// Set Rx read pointer to next packet
		temp = (ethmod_rxrdpt - 1) & ETHMOD_BUFEND;
		ethmod_wcr16(ERXRDPT, temp);

		// Decrement packet counter
		ethmod_bfs(ECON2, ECON2_PKTDEC);
	}

	return len;
}

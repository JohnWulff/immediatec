static const char mcp23s17_c[] =
"$Id: mcp23s17.c,v 1.3 2015/04/17 00:29:37 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 2014  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	mcp23s17.c
 *	communication via SPI interface of the MCP23S17 on a PiFace
 *
 *******************************************************************/

#include	<unistd.h>
#include	<sys/ioctl.h>
#include	<fcntl.h>
#include	<string.h>
#include	<assert.h>
#include	"mcp23s17.h"
#include	"pifacecad.h"

uint8_t	readData[8][MCP_MAX];

/********************************************************************
 *
 * The SPI bus parameters
 *	Variables as they need to be passed as pointers later on
 *
 *******************************************************************/

static const uint8_t	spi_mode = 0;
static const uint8_t	spi_bpw = 8;		// bits per word
static const uint32_t	spi_speed = 10000000;	// 10MHz
static const uint16_t	spi_delay = 0;
static const char *	spidev[4] = {
    "/dev/spidev0.0",
    "/dev/spidev0.1",
    "/dev/spidev1.0",
    "/dev/spidev1.1",
};

/********************************************************************
 *
 * setupSPI
 *	Setup the SPI interface on the Raspberry Pi
 *	pfce	0 = BUS0 CE0 opens /dev/spidev0.0
 *		1 = BUS0 CE1 opens /dev/spidev0.1	can be selected on PiRack
 *		2 = BUS1 CE0 opens /dev/spidev1.0	BUS1 added by Thomas Preston
 *		3 = BUS2 CE1 opens /dev/spidev1.1	may be needed for RPi 2
 *
 *******************************************************************/

int
setupSPI(int pfce)
{
    int			spiFd;
    assert((pfce & ~0x03) == 0);
    if (iC_debug & 0200) fprintf(iC_outFP, "*** pfce = %d spiDevice = %s\n", pfce,  spidev[pfce]);
    // open
    if ((spiFd = open(spidev[pfce], O_RDWR)) < 0) {
        fprintf(iC_errFP,
                "setupSPI: ERROR Could not open SPI device (%s).\n",
                spidev[pfce]);
        return -1;
    }
    // initialise
    if (ioctl(spiFd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
        fprintf(iC_errFP, "setupSPI: ERROR Could not set SPI mode.\n");
	close(spiFd);
        return -1;
    }
    if (ioctl(spiFd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0) {
        fprintf(iC_errFP,
                "setupSPI: ERROR Could not set SPI bits per word.\n");
	close(spiFd);
        return -1;
    }
    if (ioctl(spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
        fprintf(iC_errFP, "setupSPI: ERROR Could not set SPI speed.\n");
	close(spiFd);
        return -1;
    }
    return spiFd;
} /* setupSPI */

/********************************************************************
 *
 * setupMCP23S17
 *	Setup and initialise the MCP23S17 chip on a particular PiFace
 *	pfa	0-7	the PiFace address selected with JP1/JP2 and CE
 *	odr	0	INTB active (no pull-up resistor on GPIO 25)
 *	odr   IOCON_ODR	open drain with a pull-up resistor on GPIO 25
 *	inten	0x00	interrupts disabled for GPB0 - GPB7
 *	inten	0xff	interrupts enabled for GPB0 - GPB7
 *
 *******************************************************************/

int
setupMCP23S17(int spiFd, int pfa, int odr, int inten, uint8_t intf)
{
    uint8_t	c1, c2;
    int		iocon_init = (IOCON_SEQOP | IOCON_HAEN | odr);

    assert((pfa & ~0x07) == 0);
    writeByte(spiFd, pfa, IOCON, iocon_init);
    if (readByte(spiFd, pfa, IOCON) != iocon_init) {
	if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d not present\n", pfa);
	return -1;
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d present\n", pfa);

    if (inten) {
	if (pfa != 4 || intf != INTFA) {	/* PiFace */
	    writeByte(spiFd, pfa, IODIRA, 0x00);	/* Port A -> Outputs */
	    writeByte(spiFd, pfa, IODIRB, 0xff);	/* Port B -> Inputs */

	    writeByte(spiFd, pfa, GPIOA, 0x00);		/* Set all outputs off */
	    writeByte(spiFd, pfa, GPPUB, 0xff);		/* Enable all pull-ups on port B */
	    writeByte(spiFd, pfa, INTCONB, 0x00);	/* compare changes in input to previous value */
	    writeByte(spiFd, pfa, GPINTENB, inten);	/* Disable/Enable all pins for interrupt on port B */

	    c1 = readByte(spiFd, pfa, INTCAPB);		/* clear a possible MCP23S17 interrupt */
	    c2 = readByte(spiFd, pfa, GPIOB);		/* clear a possible MCP23S17 interrupt */
	    if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d present reading 0x%02x 0x%02x\n", pfa, (unsigned int)c1, (unsigned int)c2);
	} else {				/* PiFaceCAD */	
	    writeByte(spiFd, pfa, IODIRB, 0x00);	/* Port B -> Outputs */
	    writeByte(spiFd, pfa, IODIRA, 0xff);	/* Port A -> Inputs */

	    writeByte(spiFd, pfa, GPIOB, 0x00);		/* Set all outputs off */
	    writeByte(spiFd, pfa, GPPUA, 0xff);		/* Enable all pull-ups on port B */
	    writeByte(spiFd, pfa, INTCONA, 0x00);	/* compare changes in input to previous value */
	    writeByte(spiFd, pfa, GPINTENA, inten);	/* Disable/Enable all pins for interrupt on port B */

	    c1 = readByte(spiFd, pfa, INTCAPA);		/* clear a possible MCP23S17 interrupt */
	    c2 = readByte(spiFd, pfa, GPIOA);		/* clear a possible MCP23S17 interrupt */

	    if (iC_debug & 0100) fprintf(iC_outFP, "PiFaceCAD %d lcd_init\n", pfa);
	    pifacecad_lcd_init(spiFd, pfa);

	    if (iC_debug & 0200) fprintf(iC_outFP, "PiFaceCAD %d present reading 0x%02x 0x%02x\n", pfa, (unsigned int)c1, (unsigned int)c2);
	}
    }

    return spiFd;
} /* setupMCP23S17 */

/********************************************************************
 *
 * writeByte:
 *	Write a byte to a register on the MCP23S17 on the SPI bus.
 *	This is using the synchronous access mechanism.
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to write to (example: IODIRA, GPIOA).
 *	data	The data to write.
 *
 * Maintain uint8_t readData[8][0x16] to store data for subsequent bit
 * manipulation.	JW 2014/04/30
 *	indexed by pfa	0-7
 *	indexed by reg	0x00-0x15
 *
 *******************************************************************/

void
writeByte(int spiFd, int pfa, uint8_t reg, uint8_t data)
{
    uint8_t spiBufTx [3];
    uint8_t spiBufRx [3];
    struct spi_ioc_transfer spi;
    memset (&spi, 0, sizeof(spi));	/* Bug fix Thomas Preston Feb 2015 */

    spiBufTx [0] = CMD_WRITE | (pfa & 0x03) << 1;
    spiBufTx [1] = reg;
    spiBufTx [2] = data;

    spi.tx_buf        = (unsigned long)spiBufTx;
    spi.rx_buf        = (unsigned long)spiBufRx;
    spi.len           = 3;
    spi.delay_usecs   = spi_delay;
    spi.speed_hz      = spi_speed;
    spi.delay_usecs   = spi_delay;
    spi.bits_per_word = spi_bpw;

    ioctl (spiFd, SPI_IOC_MESSAGE(1), &spi);
    readData[pfa][reg] = data;
} /* writeByte */

/********************************************************************
 *
 * readByte:
 *	Read a byte from a register on the MCP23S17 on the SPI bus.
 *	This is the synchronous access mechanism.
 *	What appears to happen is that the data returned is at
 *	the same offset as the number of bytes written to the device. So if we
 *	write 2 bytes (e.g. command then register number), then the data returned
 *	will by at the 3rd byte...
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to write to (example: IODIRA, GPIOA).
 *
 *******************************************************************/

uint8_t
readByte(int spiFd, int pfa, uint8_t reg)
{
    uint8_t tx [4];
    uint8_t rx [4];
    struct spi_ioc_transfer spi;
    memset (&spi, 0, sizeof(spi));	/* Bug fix Thomas Preston Feb 2015 */

    tx [0] = CMD_READ | (pfa & 0x03) << 1;
    tx [1] = reg;
    tx [2] = 0;

    spi.tx_buf        = (unsigned long)tx;
    spi.rx_buf        = (unsigned long)rx;
    spi.len           = 3;
    spi.delay_usecs   = spi_delay;
    spi.speed_hz      = spi_speed;
    spi.delay_usecs   = spi_delay;
    spi.bits_per_word = spi_bpw;

    ioctl (spiFd, SPI_IOC_MESSAGE(1), &spi);

    return rx[2];
} /* readByte */

/********************************************************************
 *
 * Writes a single bit to the register specified. Must also specify
 * which hardware address and file descriptor to use.
 *
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to write to (example: IODIRA, GPIOA).
 *	bit	The bit number to write to.
 *	data	The data to write.
 *
 *******************************************************************/

void
writeBit(int spiFd, int pfa, uint8_t reg, uint8_t bit, uint8_t data)
{
    uint8_t reg_data;

    reg_data = readData[pfa][reg];
    if (data) {
        reg_data |= 1 << bit;		/* set bit */
    } else {
        reg_data &= ~(1 << bit);	/* clear bit */
    }
    writeByte(spiFd, pfa, reg, reg_data);
}

/********************************************************************
 *
 * Reads a single bit from the register specified. Must also specify
 * which hardware address and file descriptor to use.
 *
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to read from (example: IODIRA, GPIOA).
 *	bit	The bit number to read.
 *
 *******************************************************************/

uint8_t
readBit(int spiFd, int pfa, uint8_t reg, uint8_t bit)
{
    return (readByte(spiFd, pfa, reg) >> bit) & 1;
}

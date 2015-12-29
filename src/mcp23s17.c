static const char mcp23s17_c[] =
"$Id: mcp23s17.c,v 1.4 2015/12/23 23:52:15 jw Exp $";
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
 *  The SPI bus parameters
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
static int		spiFdA[4] = { -1, -1, -1, -1 };

/********************************************************************
 *
 *  setupSPI
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
    spiFdA[pfce] = spiFd;			/* determine address range 8 or 4 */		
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
 *  setupMCP23S17
 *	Setup and initialise the MCP23S17 chip on a particular PiFace
 *	pfa	0-7	the PiFace address selected with JP1/JP2 and CE or JP3
 *	odr	0	INTB active (no pull-up resistor on GPIO 25)
 *	odr   IOCON_ODR	open drain with a pull-up resistor on GPIO 25
 *	inputA	0x00	all inputs on Port A blocked for piFace
 *	inputA	0xff	all inputs on Port A enabled for piFaceCAD
 *	inputB	varies	some input bits may be blocked for PiFace Relay+
 *
 *  NOTE: IODIRA and IODIRB are set for output (0) unless inputA or inputB
 *	  select some input bits. The bmask for outputs only mask what is
 *	  written to OLATA or OLATB in a writeByte() call, although this
 *	  is not strictly necessary because selected input bits are high
 *	  impedance and never output a value in OLAT.
 *
 *******************************************************************/

int
setupMCP23S17(int spiFd, int pfa, uint8_t odr, uint8_t inten, uint8_t inputA, uint8_t inputB)
{
    uint8_t	c1;
    uint8_t	iocon_init = (IOCON_SEQOP | IOCON_HAEN | odr);

    assert((pfa & ~0x07) == 0);
    writeByte(spiFd, pfa, IOCON, iocon_init);
    if (readByte(spiFd, pfa, IOCON) != iocon_init) {
	if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d not present\n", pfa);
	return -1;
    }

    if (inten) {
	writeByte(spiFd, pfa, IODIRA, inputA);		/* Port A -> Outputs for PiFace */
	writeByte(spiFd, pfa, OLATA, 0x00);		/* Set all A outputs off */
	writeByte(spiFd, pfa, GPPUA, inputA);		/* Enable some pull-ups on port A */
	writeByte(spiFd, pfa, INTCONA, 0x00);		/* compare changes in input to previous value */
	writeByte(spiFd, pfa, GPINTENA, inputA);	/* Disable/Enable some pins for interrupt on port A */

	writeByte(spiFd, pfa, IODIRB, inputB);		/* Port B -> Input/Output */
	writeByte(spiFd, pfa, OLATB, 0x00);		/* Set all B outputs off */
	writeByte(spiFd, pfa, GPPUB, inputB);		/* Enable some pull-ups on port B */
	writeByte(spiFd, pfa, INTCONB, 0x00);		/* compare changes in input to previous value */
	writeByte(spiFd, pfa, GPINTENB, inputB)	;	/* Disable/Enable some pins for interrupt on port B */

	if (inputA) {					/* piFaceCAD */
	    c1 = readByte(spiFd, pfa, INTCAPA);		/* clear a possible MCP23S17 interrupt */
	    if (iC_debug & 0200) fprintf(iC_outFP, "PiFaceCAD %d present reading 0x%02x - lcd_init\n", pfa, (unsigned int)c1);
	    pifacecad_lcd_init(spiFd, pfa);
	}
	if (inputB) {					/* PiFace */
	    c1 = readByte(spiFd, pfa, INTCAPB);		/* clear a possible MCP23S17 interrupt */
	    if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d present reading 0x%02x\n", pfa, (unsigned int)c1);
	}
    }

    return spiFd;
} /* setupMCP23S17 */

/********************************************************************
 *
 *  writeByte:
 *	Write a byte to a register on the MCP23S17 on the SPI bus.
 *	This is using the synchronous access mechanism.
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to write to (example: IODIRA, OLATA).
 *	data	The data to write.
 *
 *  Maintain uint8_t readData[8][0x16] to store data for subsequent bit
 *  manipulation.	JW 2014/04/30
 *	indexed by pfa	0-7
 *	indexed by reg	0x00-0x15
 *
 *******************************************************************/

void
writeByte(int spiFd, int pfa, uint8_t reg, uint8_t data)
{
    uint8_t spiBufTx [3];
    uint8_t spiBufRx [3];
    int     adrMask = spiFd == spiFdA[0] || spiFd == spiFdA[2] ? 0x07 : 0x03;
    struct spi_ioc_transfer spi;
    memset (&spi, 0, sizeof(spi));	/* Bug fix Thomas Preston Feb 2015 */

    spiBufTx [0] = CMD_WRITE | (pfa & adrMask) << 1;
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
 *  readByte:
 *	Read a byte from a register on the MCP23S17 on the SPI bus.
 *	This is the synchronous access mechanism.
 *	What appears to happen is that the data returned is at
 *	the same offset as the number of bytes written to the device. So if we
 *	write 2 bytes (e.g. command then register number), then the data returned
 *	will by at the 3rd byte...
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to read to (example: INTCAPA, GPIOB).
 *
 *******************************************************************/

uint8_t
readByte(int spiFd, int pfa, uint8_t reg)
{
    uint8_t tx [4];
    uint8_t rx [4];
    int     adrMask = spiFd == spiFdA[0] || spiFd == spiFdA[2] ? 0x07 : 0x03;
    struct spi_ioc_transfer spi;
    memset (&spi, 0, sizeof(spi));	/* Bug fix Thomas Preston Feb 2015 */

    tx [0] = CMD_READ | (pfa & adrMask) << 1;
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
 *  Writes a single bit to the register specified. Must also specify
 *  which hardware address and file descriptor to use.
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
 *  Reads a single bit from the register specified. Must also specify
 *  which hardware address and file descriptor to use.
 *
 *	spiFd	The file descriptor returned from setupSPI().
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to read from (example: IODIRA, GPIOA).
 *	bit	The bit number to read.
 *
 *  not used for iC drivers so far
 *
 *******************************************************************/

uint8_t
readBit(int spiFd, int pfa, uint8_t reg, uint8_t bit)
{
    return (readByte(spiFd, pfa, reg) >> bit) & 1;
}

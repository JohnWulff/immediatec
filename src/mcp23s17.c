static const char mcp23s17_c[] =
"$Id: mcp23s17.c,v 1.2 2014/08/07 00:16:13 jw Exp $";
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

static uint8_t	spiMode   = 0;
static uint8_t	spiBPW    = 8;
static uint16_t	spiDelay  = 0;
static uint32_t	spiSpeed  = 5000000;

/********************************************************************
 *
 * setupSPI
 *	Setup the SPI interface on the Raspberry Pi
 *	pfce	0 = CE0 opens /dev/spidev0.0
 *		1 = CE1 opens /dev/spidev0.1	can be selected on PiRack
 *
 *******************************************************************/

int
setupSPI(int pfce)
{
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*ssetupSPI(pfce = %d)", id, id, sp, pfce), id+=2;
    int			spiFd;
    unsigned int	i;
    char		spiDevice[BUFSZ];

    snprintf(spiDevice, BUFSZ, "/dev/spidev0.%d", pfce);	/* dynamic selection of CE=0 or CE=1 */
    if (iC_debug & 0200) fprintf(iC_outFP, "*** pfce = %d spiDevice = %s\n", pfce,  spiDevice);
    if ((spiFd = open(spiDevice, O_RDWR)) < 0)	return -1;

    /********************************************************************
     * Set SPI parameters
     *	Thomas Preston does not initialise read
     *
     *	Gordon Henderson follows each write with the corresponding read
     *	  if (ioctl(spiFd, SPI_IOC_RD_MODE,          &spiMode)  < 0) return -1;
     *	  if (ioctl(spiFd, SPI_IOC_RD_BITS_PER_WORD, &spiBPW)   < 0) return -1;
     *	  if (ioctl(spiFd, SPI_IOC_RD_MAX_SPEED_HZ,  &spiSpeed) < 0) return -1;
     *	Doing this makes the transfer of 4 bit data to the HD4478U on the PiFaceCAD
     *	fail - the characters are garbled - go figure... -JW- 2014/04/29
     *
     *	Works with the PiFace - but PiFace also works without read... -JW-
     *
     *	Why are we doing a read after write?
     *	I don't know - just blindliy copying an example elsewhere... -GH-
     *******************************************************************/

    if (ioctl(spiFd, SPI_IOC_WR_MODE,          &spiMode)  < 0) return -1;
    if (ioctl(spiFd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW)   < 0) return -1;
    if (ioctl(spiFd, SPI_IOC_WR_MAX_SPEED_HZ,  &spiSpeed) < 0) return -1;

    /* Clear any initial pending interrupt */
    read(spiFd, &i, 1);
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*sMCPopen(%u)\n"
	"%*.*s  %08x WR_MODE  = %u\n"
	"%*.*s  %08x WR_BPW   = %u\n"
	"%*.*s  %08x WR_SPEED = %u\n"
	"%*.*sMCPopen END",
	id, id, sp, (unsigned)spiFd,
	id, id, sp, (unsigned)SPI_IOC_WR_MODE,		(unsigned)spiMode,
	id, id, sp, (unsigned)SPI_IOC_WR_BITS_PER_WORD,	(unsigned)spiBPW,
	id, id, sp, (unsigned)SPI_IOC_WR_MAX_SPEED_HZ,	(unsigned)spiSpeed,
	id, id, sp);

    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*ssetupSPI END\n", id, id, sp);
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
    if (iC_debug & 04) fprintf(iC_outFP,
	"\n%*.*ssetupMCP23S17(spiFd = %d, pfa = %d, odr = 0x%02x, inten = 0x%02x)", id, id, sp,
	spiFd, pfa, odr, inten), id+=2;
    uint8_t	c1, c2;
    int		iocon_init = (IOCON_SEQOP | IOCON_HAEN | odr);

    assert((pfa & ~0x07) == 0);
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*sMCPsetup(%d:%u odr=%02x inten=%02x)", id, id, sp,
	spiFd, (unsigned)pfa, (unsigned)odr, (unsigned)inten);
    writeByte(spiFd, pfa, IOCON, iocon_init);
    if (readByte(spiFd, pfa, IOCON) != iocon_init) {
	if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d not present\n", pfa);
	if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*ssetupMCP23S17 END - PiFace %d not present", id, id, sp, pfa);
	if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*s", id, id, sp);
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

    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*ssetupMCP23S17 END - PiFace %d present", id, id, sp, pfa);
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*s", id, id, sp);
    return spiFd;
} /* setupMCP23S17 */

/********************************************************************
 *
 * writeByte:
 *	Write a byte to a register on the MCP23S17 on the SPI bus.
 *	This is using the synchronous access mechanism.
 *	spiFd	The file descriptor returned from <mcp23s17_open>"()".
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

    spiBufTx [0] = CMD_WRITE | (pfa & 0x03) << 1;
    spiBufTx [1] = reg;
    spiBufTx [2] = data;

    spi.tx_buf        = (unsigned long)spiBufTx;
    spi.rx_buf        = (unsigned long)spiBufRx;
    spi.len           = 3;
    spi.delay_usecs   = spiDelay;
    spi.speed_hz      = spiSpeed;
    spi.delay_usecs   = spiDelay;
    spi.bits_per_word = spiBPW;

    ioctl (spiFd, SPI_IOC_MESSAGE(1), &spi);
    readData[pfa][reg] = data;
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*sMCPwrite(%u:%02x) = %02x", id, id, sp,
	(unsigned)pfa, (unsigned)reg, (unsigned)data);
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
 *	spiFd	The file descriptor returned from <mcp23s17_open>"()".
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

    tx [0] = CMD_READ | (pfa & 0x03) << 1;
    tx [1] = reg;
    tx [2] = 0;

    spi.tx_buf        = (unsigned long)tx;
    spi.rx_buf        = (unsigned long)rx;
    spi.len           = 3;
    spi.delay_usecs   = spiDelay;
    spi.speed_hz      = spiSpeed;
    spi.delay_usecs   = spiDelay;
    spi.bits_per_word = spiBPW;

    ioctl (spiFd, SPI_IOC_MESSAGE(1), &spi);
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*sMCPread (%u:%02x) = %02x", id, id, sp,
	(unsigned)pfa, (unsigned)reg, (unsigned)rx[2]);

    return rx[2];
} /* readByte */

/********************************************************************
 *
 * Writes a single bit to the register specified. Must also specify
 * which hardware address and file descriptor to use.
 *
 *	spiFd	The file descriptor returned from <mcp23s17_open>"()".
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

    if (iC_debug & 04) fprintf(iC_outFP,
	"\n%*.*swriteBit(spiFd = %d, pfa = %d, reg = 0x%02x, bit = %u, data = %u)", id, id, sp,
	spiFd, pfa, (unsigned)reg, (unsigned)bit, (unsigned)data), id+=2;
    reg_data = readData[pfa][reg];
    if (data) {
        reg_data |= 1 << bit;		/* set bit */
    } else {
        reg_data &= ~(1 << bit);	/* clear bit */
    }
    writeByte(spiFd, pfa, reg, reg_data);
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*swriteBit END", id, id, sp);
}

/********************************************************************
 *
 * Reads a single bit from the register specified. Must also specify
 * which hardware address and file descriptor to use.
 *
 *	spiFd	The file descriptor returned from <mcp23s17_open>"()".
 *	pfa	The hardware address of the MCP23S17.
 *	reg	The register to read from (example: IODIRA, GPIOA).
 *	bit	The bit number to read.
 *
 *******************************************************************/

uint8_t
readBit(int spiFd, int pfa, uint8_t reg, uint8_t bit)
{
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*sreadBit(spiFd = %d, pfa = %d, reg = 0x%02x, bit = %u)", id, id, sp,
	spiFd, pfa, (unsigned)reg, (unsigned)bit), id+=2;
    if (iC_debug & 04) {
	uint8_t	data = (readByte(spiFd, pfa, reg) >> bit) & 1;
	id-=2, fprintf(iC_outFP, "\n%*.*sreadBit <== %u END", id, id, sp, (unsigned)data);
	return data;
    }
    return (readByte(spiFd, pfa, reg) >> bit) & 1;
}

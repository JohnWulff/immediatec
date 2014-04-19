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
 *	mcp23s17.h
 *	communication via SPI interface of the MCP23S17 on a PiFace
 *
 *******************************************************************/

#ifndef MCP23S17_H
#define MCP23S17_H
static const char mcp23s17_h[] =
"$Id: mcp23s17.h,v 1.1 2014/04/19 03:46:11 pi Exp $";

#include	<stdio.h>
#include	<stdint.h>
//#include	<stdlib.h>
//#include	<unistd.h>
//#include	<signal.h>
//#include	<assert.h>
//#include	<errno.h>
//#include	<sys/ioctl.h>
//#include	<fcntl.h>
//#include	<string.h>
#include	<linux/spi/spidev.h>

#define BUFSZ		32

// MCP23S17 Registers

#define	IOCON		0x0A

#define	IODIRA		0x00
#define	IPOLA		0x02
#define	GPINTENA	0x04
#define	DEFVALA		0x06
#define	INTCONA		0x08
#define	GPPUA		0x0C
#define	INTFA		0x0E
#define	INTCAPA		0x10
#define	GPIOA		0x12
#define	OLATA		0x14

#define	IODIRB		0x01
#define	IPOLB		0x03
#define	GPINTENB	0x05
#define	DEFVALB		0x07
#define	INTCONB		0x09
#define	GPPUB		0x0D
#define	INTFB		0x0F
#define	INTCAPB		0x11
#define	GPIOB		0x13
#define	OLATB		0x15

// Bits in the IOCON register

#define	IOCON_BANK_MODE	0x80
#define	IOCON_MIRROR	0x40
#define	IOCON_SEQOP	0x20
#define	IOCON_DISSLW	0x10
#define	IOCON_HAEN	0x08
#define	IOCON_ODR	0x04
#define	IOCON_INTPOL	0x02
#define	IOCON_UNUSED	0x01

// Command codes

#define CMD_WRITE	0x40
#define CMD_READ	0x41

extern FILE *		iC_outFP;			/* listing file pointer */
extern FILE *		iC_errFP;			/* error file pointer */
extern short		iC_debug;

extern int		setupSPI(int pfce);
extern int		setupMCP23S17(int spiFd, int pfa, int odr, int inten);
extern void		writeByte(int spiFd, int pfa, uint8_t reg, uint8_t data);
extern uint8_t		readByte(int spiFd, int pfa, uint8_t reg);

#endif	/* MCP23S17_H */

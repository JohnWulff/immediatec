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
 *	mcp23017.h
 *	communication via I2C multiplexed by a PCA9548A to MCP23017
 *	16-Bit I/O expanders
 *
 *******************************************************************/

#ifndef MCP23017_H
#define MCP23017_H
static const char mcp23017_h[] =
"$Id: mcp23017.h 1.1 $";

#include	<stdio.h>
#include	<stdint.h>

#define BUFSZ		32

// MCP23017 Registers

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

enum	conf		{ detect, data, concentrate };

extern FILE *		iC_outFP;			/* listing file pointer */
extern FILE *		iC_errFP;			/* error file pointer */
extern short		iC_debug;
#ifdef	TRACE 

extern int	id;		/* used to indent traces if (iC_debug & 04) */
extern char	sp[];
#endif	/* TRACE */

extern int		setupI2C(int i2cSel);
extern int		setupMCP23017(int spiFd, enum conf config, uint8_t devId, uint8_t iocBits, uint8_t inputA, uint8_t inputB);
extern void		writeByte(int spiFd, int devId, uint8_t reg, uint8_t data);
extern uint8_t		readByte(int spiFd, int devId, uint8_t reg);

#endif	/* MCP23017_H */

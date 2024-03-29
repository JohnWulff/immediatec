static const char mcp23017_c[] =
"$Id: mcp23017.c 1.4 $";
/********************************************************************
 *
 *	Copyright (C) 2023  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  Some code has been modified from the program 'smbus.c' from i2c-tools,
 *  which is under the GNU Lesser General Public License - gratefuly acknowledged.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	mcp23017.c
 *	communication via I2C multiplexed by a PCA9548A to MCP23017
 *	16-Bit I/O expanders
 *
 *******************************************************************/

#include	<unistd.h>
#include	<sys/ioctl.h>
#include	<fcntl.h>
#include	<string.h>
#include	<assert.h>
#include	<errno.h>
#include	<linux/i2c.h>
#include	<linux/i2c-dev.h>
#include	"mcp23017.h"

/********************************************************************
 *
 *  The I2C bus parameters
 *
 *******************************************************************/

static const char *	i2cdev[10] = {
    "/dev/i2c-0",				// direct I2C filename bypassing PCA9548A
    "/dev/i2c-11",				// 8 multiplexed I2C filenames
    "/dev/i2c-12",
    "/dev/i2c-13",
    "/dev/i2c-14",
    "/dev/i2c-15",
    "/dev/i2c-16",
    "/dev/i2c-17",
    "/dev/i2c-18",
    "/dev/i2c-1",				// direct I2C filename bypassing PCA9548A
};						// never touch /dev/i2c-2 EEPROMS

/********************************************************************
 *
 *  setupI2C
 *	Setup the I2C interface on the Raspberry Pi
 *	i2cCh	0     = /dev/i2c-0 direct I2C channel bypassing PCA9548A
 *	i2cCh	1 - 8 = /dev/i2c-11 to /dev/i2c-18 multiplexed I2C channels
 *	i2cCh	9     = /dev/i2c-1 direct I2C channel bypassing PCA9548A
 *	return	i2cFd or -1 if I2C channel not active
 *
 *******************************************************************/

int
setupI2C(int i2cCh)
{
    int			i2cFd;
    unsigned long	funcs;

    assert(i2cCh >= 0 && i2cCh < 10);
    // open
    if ((i2cFd = open(i2cdev[i2cCh], O_RDWR)) < 0) {
        return -1;
    }
    // check functionality of the I2C channel - will ignore multiplexed devices if PCA9548A is not present
    if (ioctl(i2cFd, I2C_FUNCS, &funcs) < 0) {
	fprintf(stderr, "setupI2C: ERROR: Could not get the adapter functionality matrix for I2C channel %s: %s\n",
		i2cdev[i2cCh], strerror(errno));
	close(i2cFd);
        return -1;
    }
    if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE) &&
	!(funcs & I2C_FUNC_SMBUS_READ_BYTE)) {
	fprintf(stderr, "setupI2C: ERROR: Could not write or read a byte on I2C device %s: %s\n",
		i2cdev[i2cCh], strerror(errno));
	close(i2cFd);
        return -1;
    }
    return i2cFd;				// leave the file descriptor open until program quits
} /* setupI2C */

/********************************************************************
 *
 *  setupMCP23017
 *	Setup and initialise an MCP23017 chip on a particular I2C channel
 *	i2cFd		file descriptor for a channel /dev/i2c-11 - /dev/i2c-18
 *	config	0/1/2	detect:		test if MCP23017 is present
 *			data:		configure a data MCP23017
 *			concentrate:	configure a concentrator MCP23017
 *	devId	0-7	the MCP23017 address selected by pins A0 to A2
 *	iocBits	0	INTA and INTB active output (active-lo by default)
 *	   IOCON_INTPOL	INTA and INTB pins are active-hi unles IOCON_ODR is set
 *	   IOCON_ODR	open drain with a pull-up resistor required at receiver
 *	   IOCON_MIRROR	INTA and INTB pins are internally connected
 *	inputA		0 bits configure output, 1 bits configure input for GPIOA
 *	inputB		0 bits configure output, 1 bits configure input for GPIOB
 *	invA		0 bits configure active HI, 1 bits configure active LO for IPOLA
 *	invB		0 bits configure active HI, 1 bits configure active LO for IPOLB
 *	return	i2cFd
 *
 *  NOTE: IODIRA and IODIRB are set for output (0x00) unless inputA or inputB
 *	  select some input bits. The bit-mask for outputs only mask what is
 *	  written to OLATA or OLATB in a writeByte() call, although this
 *	  is not strictly necessary because selected input bits are high
 *	  impedance and never output a value in OLAT.
 *
 *******************************************************************/

int
setupMCP23017(int i2cFd, enum conf config, uint8_t devId, uint8_t iocBits,
	      uint8_t inputA, uint8_t inputB, uint8_t invA, uint8_t invB)
{
    uint8_t	iocon_init = (IOCON_SEQOP | iocBits);	/* Sequential operation disabled, Address always enabled */
							/* IOCON_ODR sets the INTA and INTB pins to open drain */
							/* if IOCON_ODR is not set INTPOL is 0 for INT active-low */
    assert((devId & ~0x07) == 0x20);
    writeByte(i2cFd, devId, IOCON, iocon_init);
    if (readByte(i2cFd, devId, IOCON) != iocon_init) {
        return -1;
    }

    if (config != detect) {
        writeByte(i2cFd, devId, IODIRA, inputA);	/* Port A -> Outputs or Inputs */
        if (inputA) {
            writeByte(i2cFd, devId, GPINTENA, inputA);	/* Enable interrupts for inputs */
            if (config == data) {
                writeByte(i2cFd, devId, INTCONA, 0x00);	/* compare changes in input to previous value */
                writeByte(i2cFd, devId, IPOLA, invA);	/* input bit polarity */
            } else {	/* concentrate */
                writeByte(i2cFd, devId, DEFVALA, inputA);	/* concentrator MCP23017 */
                writeByte(i2cFd, devId, INTCONA, inputA);	/* compare changes in input to DEFVALA */
            }
            writeByte(i2cFd, devId, GPPUA, inputA);	/* Enable pull-ups for inputs */
            readByte(i2cFd, devId, INTCAPA);		/* clear a possible INTA interrupt */
	} else {
            writeByte(i2cFd, devId, GPINTENA, 0x00);	/* disable interrupts for inputs */
	    writeByte(i2cFd, devId, INTCONA,  0x00);	/* compare changes in input to previous value */
	    writeByte(i2cFd, devId, IPOLA,    0x00);	/* reset input bit polarity */
	    writeByte(i2cFd, devId, DEFVALA,  0x00);	/* clear  */
            writeByte(i2cFd, devId, GPPUA,    0x00);	/* disable pull-ups for inputs */
        }
        writeByte(i2cFd, devId, OLATA, 0x00);		/* Set all actual outputs off */

        writeByte(i2cFd, devId, IODIRB, inputB);	/* Port B -> Outputs or Inputs */
        if (inputB) {
            writeByte(i2cFd, devId, GPINTENB, inputB);	/* Enable interrupts for inputs */
            if (config == data) {
                writeByte(i2cFd, devId, INTCONB, 0x00);	/* compare changes in input to previous value */
                writeByte(i2cFd, devId, IPOLB, invB);	/* input bit polarity */
            } else {	/* concentrate */
                writeByte(i2cFd, devId, DEFVALB, inputB);	/* concentrator MCP23017 */
                writeByte(i2cFd, devId, INTCONB, inputB);	/* compare changes in input to DEFVALB */
            }
            writeByte(i2cFd, devId, GPPUB, inputB);	/* Enable pull-ups for inputs */
            readByte(i2cFd, devId, INTCAPB);		/* clear a possible INTB interrupt */
	} else {
            writeByte(i2cFd, devId, GPINTENB, 0x00);	/* disable interrupts for inputs */
	    writeByte(i2cFd, devId, INTCONB,  0x00);	/* compare changes in input to previous value */
	    writeByte(i2cFd, devId, IPOLB,    0x00);	/* reset input bit polarity */
	    writeByte(i2cFd, devId, DEFVALB,  0x00);	/* clear  */
            writeByte(i2cFd, devId, GPPUB,    0x00);	/* disable pull-ups for inputs */
        }
        writeByte(i2cFd, devId, OLATB, 0x00);		/* Set all actual outputs off */
    }

    return i2cFd;
} /* setupMCP23017 */

/********************************************************************
 *
 *	Support routine for debug tracing
 *
 *******************************************************************/

const char *
getI2cDevice(int i2cCh) {
    assert((i2cCh >= 0) && (i2cCh < 10));
    return i2cdev[i2cCh];
}

/********************************************************************
 *
 *  i2c_smbus_access
 *	General access routine for I2C transfers - from i2c-tools
 *
 *******************************************************************/

static int32_t i2c_smbus_access(int file, char read_write, uint8_t command,
		       int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;
    int32_t err;

    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;

    err = ioctl(file, I2C_SMBUS, &args);
    if (err == -1) {
	err = -errno;
    }
    return err;
} /* i2c_smbus_access */

/********************************************************************
 *
 *  writeByte:
 *	Write a byte to a register on the MCP23017 on an I2C bus.
 *
 *	i2cFd	The file descriptor returned from setupI2C() for a
 *		specific i2cbus.
 *	devId	The hardware address of the MCP23017.
 *	reg	The register to write to (example: IODIRA, OLATA).
 *	data	The data to write.
 *
 *******************************************************************/

void
writeByte(int i2cFd, int devId, uint8_t reg, uint8_t value)
{
    union i2c_smbus_data data;

    ioctl (i2cFd, I2C_SLAVE, devId);		/* select I2C device */
    /* I2C_SMBUS_BYTE_DATA */
    data.byte = value;
    i2c_smbus_access(i2cFd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, &data);
} /* writeByte */

/********************************************************************
 *
 *  readByte:
 *	i2cFd	The file descriptor returned from setupI2C().
 *	devId	The hardware address of the MCP23017.
 *	reg	The register to read from (example: INTCAPA, GPIOB).
 *
 *******************************************************************/

uint8_t
readByte(int i2cFd, int devId, uint8_t reg)
{
    union i2c_smbus_data data;

    ioctl (i2cFd, I2C_SLAVE, devId);		/* select I2C device */
    /* I2C_SMBUS_BYTE_DATA */
    data.byte = 0;
    i2c_smbus_access(i2cFd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data);
    return 0x0ff & data.byte;
} /* readByte */

static const char ic2busses_h[] =
"$Id: i2cbusses.h 1.1 $";
/********************************************************************
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  This code has been modified from the program 'i2cdetect' from i2c-tools,
 *  which is under the GNU Lesser General Public License - gratefuly acknowledged.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	i2cbusses.c
 *	Identify I2C bus names multiplexed by a PCA9548A multiplexor
 *
 *******************************************************************/

/*
    i2cbusses.h - Part of the i2c-tools package

    Copyright (C) 2004-2010  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#ifndef _I2CBUSSES_H
#define _I2CBUSSES_H

#include <unistd.h>

struct i2c_adap {
	int nr;
	char *name;
	const char *funcs;
	const char *algo;
};

extern const char *	i2cdev[10];

struct i2c_adap *gather_i2c_busses(void);
void free_adapters(struct i2c_adap *adapters);
int open_i2c_dev(int i2cbus, char *filename, size_t size, int quiet);

void generate_i2c_busses(void);

#endif

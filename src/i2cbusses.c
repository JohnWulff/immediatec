static const char ic2busses_c[] =
"$Id: i2cbusses.c 1.1 $";
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
    i2cbusses: Print the installed i2c busses for both 2.4 and 2.6 kernels.
               Part of user-space programs to access for I2C
               devices.
    Copyright (c) 1999-2003  Frodo Looijaard <frodol@dds.nl> and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2008-2012  Jean Delvare <jdelvare@suse.de>

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

/* For strdup and snprintf */
#define _BSD_SOURCE 1 /* for glibc <= 2.19 */
#define _DEFAULT_SOURCE 1 /* for glibc >= 2.19 */

#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "i2cbusses.h"
#include "icc.h"

enum adt { adt_dummy, adt_isa, adt_i2c, adt_smbus, adt_unknown };

struct adap_type {
	const char *funcs;
	const char* algo;
};

static struct adap_type adap_types[5] = {
	{ .funcs	= "dummy",
	  .algo		= "Dummy bus", },
	{ .funcs	= "isa",
	  .algo		= "ISA bus", },
	{ .funcs	= "i2c",
	  .algo		= "I2C adapter", },
	{ .funcs	= "smbus",
	  .algo		= "SMBus adapter", },
	{ .funcs	= "unknown",
	  .algo		= "N/A", },
};

static enum adt i2c_get_funcs(int i2cbus)
{
	unsigned long funcs;
	int file;
	char filename[20];
	enum adt ret;

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 1);
	if (file < 0)
		return adt_unknown;

	if (ioctl(file, I2C_FUNCS, &funcs) < 0)
		ret = adt_unknown;
	else if (funcs & I2C_FUNC_I2C)
		ret = adt_i2c;
	else if (funcs & (I2C_FUNC_SMBUS_BYTE |
			  I2C_FUNC_SMBUS_BYTE_DATA |
			  I2C_FUNC_SMBUS_WORD_DATA))
		ret = adt_smbus;
	else
		ret = adt_dummy;

	close(file);
	return ret;
}

/* Remove trailing spaces from a string
   Return the new string length including the trailing NUL */
static int rtrim(char *s)
{
	int i;

	for (i = strlen(s) - 1; i >= 0 && (s[i] == ' ' || s[i] == '\n'); i--)
		s[i] = '\0';
	return i + 2;
}

void free_adapters(struct i2c_adap *adapters)
{
	int i;

	for (i = 0; adapters[i].name; i++)
		free(adapters[i].name);
	free(adapters);
}

/* We allocate space for the adapters in bunches. The last item is a
   terminator, so here we start with room for 7 adapters, which should
   be enough in most cases. If not, we allocate more later as needed. */
#define BUNCH	8

/* n must match the size of adapters at calling time */
static struct i2c_adap *more_adapters(struct i2c_adap *adapters, int n)
{
	struct i2c_adap *new_adapters;

	new_adapters = realloc(adapters, (n + BUNCH) * sizeof(struct i2c_adap));
	if (!new_adapters) {
		free_adapters(adapters);
		return NULL;
	}
	memset(new_adapters + n, 0, BUNCH * sizeof(struct i2c_adap));

	return new_adapters;
}

struct i2c_adap *gather_i2c_busses(void)
{
	char s[120];
	struct dirent *de, *dde;
	DIR *dir, *ddir;
	FILE *f;
	char fstype[NAME_MAX], sysfs[NAME_MAX], n[NAME_MAX];
	int foundsysfs = 0;
	int len, count = 0;
	struct i2c_adap *adapters;

	adapters = calloc(BUNCH, sizeof(struct i2c_adap));
	if (!adapters)
		return NULL;

	/* look in /proc/bus/i2c */
	if ((f = fopen("/proc/bus/i2c", "r"))) {
		while (fgets(s, 120, f)) {
			char *algo, *name, *type, *all;
			int len_algo, len_name, len_type;
			int i2cbus;

			algo = strrchr(s, '\t');
			*(algo++) = '\0';
			len_algo = rtrim(algo);

			name = strrchr(s, '\t');
			*(name++) = '\0';
			len_name = rtrim(name);

			type = strrchr(s, '\t');
			*(type++) = '\0';
			len_type = rtrim(type);

			sscanf(s, "i2c-%d", &i2cbus);

			if ((count + 1) % BUNCH == 0) {
				/* We need more space */
				adapters = more_adapters(adapters, count + 1);
				if (!adapters)
					return NULL;
			}

			all = malloc(len_name + len_type + len_algo);
			if (all == NULL) {
				free_adapters(adapters);
				return NULL;
			}
			adapters[count].nr = i2cbus;
			adapters[count].name = strcpy(all, name);
			adapters[count].funcs = strcpy(all + len_name, type);
			adapters[count].algo = strcpy(all + len_name + len_type,
						      algo);
			count++;
		}
		fclose(f);
		goto done;
	}

	/* look in sysfs */
	/* First figure out where sysfs was mounted */
	if ((f = fopen("/proc/mounts", "r")) == NULL) {
		goto done;
	}
	while (fgets(n, NAME_MAX, f)) {
		sscanf(n, "%*[^ ] %[^ ] %[^ ] %*s\n", sysfs, fstype);
		if (strcasecmp(fstype, "sysfs") == 0) {
			foundsysfs++;
			break;
		}
	}
	fclose(f);
	if (! foundsysfs) {
		goto done;
	}

	/* Bus numbers in i2c-adapter don't necessarily match those in
	   i2c-dev and what we really care about are the i2c-dev numbers.
	   Unfortunately the names are harder to get in i2c-dev */
	strcat(sysfs, "/class/i2c-dev");
	if(!(dir = opendir(sysfs)))
		goto done;
	/* go through the busses */
	while ((de = readdir(dir)) != NULL) {
		if (!strcmp(de->d_name, "."))
			continue;
		if (!strcmp(de->d_name, ".."))
			continue;

		/* this should work for kernels 2.6.5 or higher and */
		/* is preferred because is unambiguous */
		len = snprintf(n, NAME_MAX, "%s/%s/name", sysfs, de->d_name);
		if (len >= NAME_MAX) {
			fprintf(stderr, "%s: path truncated\n", n);
			continue;
		}
		f = fopen(n, "r");
		/* this seems to work for ISA */
		if(f == NULL) {
			len = snprintf(n, NAME_MAX, "%s/%s/device/name", sysfs,
				       de->d_name);
			if (len >= NAME_MAX) {
				fprintf(stderr, "%s: path truncated\n", n);
				continue;
			}
			f = fopen(n, "r");
		}
		/* non-ISA is much harder */
		/* and this won't find the correct bus name if a driver
		   has more than one bus */
		if(f == NULL) {
			len = snprintf(n, NAME_MAX, "%s/%s/device", sysfs,
				       de->d_name);
			if (len >= NAME_MAX) {
				fprintf(stderr, "%s: path truncated\n", n);
				continue;
			}
			if(!(ddir = opendir(n)))
				continue;
			while ((dde = readdir(ddir)) != NULL) {
				if (!strcmp(dde->d_name, "."))
					continue;
				if (!strcmp(dde->d_name, ".."))
					continue;
				if ((!strncmp(dde->d_name, "i2c-", 4))) {
					len = snprintf(n, NAME_MAX,
						       "%s/%s/device/%s/name",
						       sysfs, de->d_name,
						       dde->d_name);
					if (len >= NAME_MAX) {
						fprintf(stderr,
							"%s: path truncated\n",
							n);
						continue;
					}
					if((f = fopen(n, "r")))
						goto found;
				}
			}
		}

found:
		if (f != NULL) {
			int i2cbus;
			enum adt type;
			char *px;

			px = fgets(s, 120, f);
			fclose(f);
			if (!px) {
				fprintf(stderr, "%s: read error\n", n);
				continue;
			}
			if ((px = strchr(s, '\n')) != NULL)
				*px = 0;
			if (!sscanf(de->d_name, "i2c-%d", &i2cbus))
				continue;
			if (!strncmp(s, "ISA ", 4)) {
				type = adt_isa;
			} else {
				/* Attempt to probe for adapter capabilities */
				type = i2c_get_funcs(i2cbus);
			}

			if ((count + 1) % BUNCH == 0) {
				/* We need more space */
				adapters = more_adapters(adapters, count + 1);
				if (!adapters)
					return NULL;
			}

			adapters[count].nr = i2cbus;
			adapters[count].name = strdup(s);
			if (adapters[count].name == NULL) {
				free_adapters(adapters);
				return NULL;
			}
			adapters[count].funcs = adap_types[type].funcs;
			adapters[count].algo = adap_types[type].algo;
//			printf("%d:	i2c-%d	%s\n", count, adapters[count].nr, adapters[count].name);
			count++;
		}
	}
	closedir(dir);

done:

	return adapters;	/* don't bother sorting */
}

int open_i2c_dev(int i2cbus, char *filename, size_t size, int quiet)
{
	int file, len;

	if (len >= (int)size) {
		fprintf(stderr, "%s: path truncated\n", filename);
		return -EOVERFLOW;
	}
	file = open(filename, O_RDWR);

	if (file < 0 && (errno == ENOENT || errno == ENOTDIR)) {
		len = snprintf(filename, size, "/dev/i2c-%d", i2cbus);
		if (len >= (int)size) {
			fprintf(stderr, "%s: path truncated\n", filename);
			return -EOVERFLOW;
		}
		file = open(filename, O_RDWR);
	}

	if (file < 0 && !quiet) {
		if (errno == ENOENT) {
			fprintf(stderr, "Error: Could not open file "
				"`/dev/i2c-%d' or `/dev/i2c/%d': %s\n",
				i2cbus, i2cbus, strerror(ENOENT));
		} else {
			fprintf(stderr, "Error: Could not open file "
				"`%s': %s\n", filename, strerror(errno));
			if (errno == EACCES)
				fprintf(stderr, "Run as root?\n");
		}
	}

	return file;
}

const char *	i2cdev[10];

/********************************************************************
 *	Generate tearray i2cdev[] for use in setupI2C() in mcp23017.c
 *	The i2cdev[] values forthe 8 mux outputs are /dev/i2c-11 -
 *	/dev/i2c-18 fot RPi 3B or less.
 *	The values are /dev/i2c-22 - /dev/i2c-29 for RPi 4B
 *******************************************************************/

void generate_i2c_busses(void)
{
	struct i2c_adap *adapters;
	int count;
	int chan_id;
	char buffer[15];

	adapters = gather_i2c_busses();
	if (adapters == NULL) {
		fprintf(stderr, "Error: Out of memory!\n");
		return;
	}

	i2cdev[0] = "/dev/i2c-0";			/* dummy entry for very old RPis */
	for (count = 0; adapters[count].name; count++) {
		if (strncmp(adapters[count].name, "bcm2835", 7) == 0 &&
			adapters[count].nr == 1) {
			if (iC_debug & 0200) printf("i2cbusses: i2c-%d\t%-10s\t%-32s\t%s	chan_id = 9\n",
				adapters[count].nr, adapters[count].funcs,
				adapters[count].name, adapters[count].algo);
			i2cdev[9] = "/dev/i2c-1";	/* entry if no mux found */
			
		} else
		if (sscanf(adapters[count].name, "i2c-1-mux (chan_id %d)", &chan_id) == 1) {
			if (iC_debug & 0200) printf("i2cbusses: i2c-%d\t%-10s\t%-32s\t%s	chan_id = %d\n",
				adapters[count].nr, adapters[count].funcs,
				adapters[count].name, adapters[count].algo,
				chan_id);
			snprintf(buffer, sizeof buffer, "/dev/i2c-%d", adapters[count].nr);
			i2cdev[chan_id+1] = strdup(buffer);	/* offset by 1 for extra /dev/i2c-0 entry */
		}
	}

	free_adapters(adapters);
}

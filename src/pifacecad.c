static const char pifacecad_c[] =
"$Id: pifacecad.c 1.5 $";
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
 *	pifacecad.h
 *	A simple static library for controlling PiFace Control and Display.
 *
 *  The following routines were partly extracted from pifacecad.c pifacecad.h
 *  This version has been modified  by John E. Wulff	2014/04/25
 *  for the immediate C system to interface with a PiFaceCAD in
 *  accordance with the terms of the GNU General Public License
 *
 * Copyright (C) 2013 Thomas Preston <thomas.preston@openlx.org.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  The commands pifacecad_lcd_clear() and pifacecad_lcd_home() require
 *  a delay of 2.6 ms to bridge the execution time used internally for
 *  clearing the display buffer.  For the first command no execution
 *  time is specified in the HITACHI specification - the second one
 *  is specified at 1.52 ms. Actual measurement using the Busy Flag
 *  determined that the execution time of pifacecad_lcd_clear() is
 *  between 1.75 and 2.6 ms. Without a delay of 2.6 ms after each of
 *  these commands a following pifacecad_lcd_write() would frequently
 *  garble the text. A pifacecad_lcd_display_off() or _on() does not
 *  execute at all unless there is a delay of 2.6 ms after a prior
 *  piface_lcd_clear(). Alle the other commands are followed by a
 *  delay of 40 us, which is in line with the HITACHI spec.
 *
 *******************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <icg.h>
#include "mcp23s17.h"
#include "tcpc.h"
#include "pifacecad.h"

#ifdef	RASPBERRYPI
#define LCD_PORT	GPIOB
#ifdef	TRACE 

int	id   = 0;		/* used to indent traces if (iC_debug & 04) */
char	sp[] = "                                        ";
#endif	/* TRACE */

/* PiFace Control and Display is usually at /dev/spidev0.1, pfa = 4 */
int	iC_CAD_FD = -1;		/* MCP23S17 SPI file descriptor */
int	iC_CADpfa = 0;

/* current lcd state */
/* static int curcol = 0, currow = 0; */
static uint8_t cur_address = 0;
static uint8_t cur_entry_mode = 0;
static uint8_t cur_function_set = 0;
static uint8_t cur_display_control = 0;


/* static function definitions */
static void sleep_ns(long nanoseconds);


void pifacecad_lcd_init(int fd, uint8_t pfa)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_init(fd = %d, pfa = %u)", id, id, sp, fd, (unsigned)pfa), id+=2;
#endif	/* TRACE */
    iC_CAD_FD = fd;
    iC_CADpfa = pfa;

    /* setup sequence */
    sleep_ns(DELAY_SETUP_0_NS);				/* 15 ms */
    writeByte(iC_CAD_FD, iC_CADpfa, LCD_PORT, 0x03);
    pifacecad_lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_1_NS);				/* 5 ms */
    writeByte(iC_CAD_FD, iC_CADpfa, LCD_PORT, 0x03);
    pifacecad_lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_2_NS);				/* 1 ms */
    writeByte(iC_CAD_FD, iC_CADpfa, LCD_PORT, 0x03);
    pifacecad_lcd_pulse_enable();

    writeByte(iC_CAD_FD, iC_CADpfa, LCD_PORT, 0x02);
    pifacecad_lcd_pulse_enable();

    cur_function_set |= LCD_4BITMODE | LCD_2LINE | LCD_5X8DOTS;
    pifacecad_lcd_send_command(LCD_FUNCTIONSET | cur_function_set);

    cur_display_control |= LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);

    pifacecad_lcd_clear();

    cur_entry_mode |= LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_entry_mode);
#ifdef	TRACE 

    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_init END", id, id, sp);
#endif	/* TRACE */
}


uint8_t pifacecad_lcd_write(const char * message)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_write(\"%s\")", id, id, sp, message), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_send_command(LCD_SETDDRAMADDR | cur_address);

    /* for each character in the message */
    while (*message) {
        if (*message == '\n') {
            pifacecad_lcd_set_cursor(0, 1);
        } else {
            pifacecad_lcd_send_data(*message);
            cur_address++;
        }
        message++;
#ifdef	TRACE 
	if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*scur_address = %d", id, id, sp, cur_address);
#endif	/* TRACE */
    }
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_write END", id, id, sp);
#endif	/* TRACE */
    return cur_address;
}

uint8_t pifacecad_lcd_set_cursor(uint8_t col, uint8_t row)
{
    col = max(0, min(col, (LCD_RAM_WIDTH / 2) - 1));
    row = max(0, min(row, LCD_MAX_LINES - 1));
    pifacecad_lcd_set_cursor_address(colrow2address(col, row));
    return cur_address;
}

void pifacecad_lcd_set_cursor_address(uint8_t address)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_cursor_address(address = %u)", id, id, sp, (unsigned)address), id+=2;
#endif	/* TRACE */
    cur_address = address % LCD_RAM_WIDTH;
    pifacecad_lcd_send_command(LCD_SETDDRAMADDR | cur_address);
#ifdef	TRACE 
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*scur_address = %d", id, id, sp, cur_address);
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_cursor_address END", id, id, sp);
#endif	/* TRACE */
}

uint8_t pifacecad_lcd_get_cursor_address(void)
{
    return cur_address;
}


void pifacecad_lcd_clear(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_clear()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_send_command(LCD_CLEARDISPLAY);
    sleep_ns(DELAY_CLEAR_NS);				/* 2.6 ms (not specified in data sheet */
    cur_address = 0;
#ifdef	TRACE 
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*scur_address = %d", id, id, sp, cur_address);
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_clear END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_home(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_home()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_send_command(LCD_RETURNHOME);
    sleep_ns(DELAY_CLEAR_NS);				/* 2.6 ms  - added JW 2014/05/14 */
    cur_address = 0;
#ifdef	TRACE 
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*scur_address = %d", id, id, sp, cur_address);
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_home END", id, id, sp);
#endif	/* TRACE */
}


void pifacecad_lcd_display_on(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_display_on()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control |= LCD_DISPLAYON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_display_on END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_display_off(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_display_off()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control &= ~LCD_DISPLAYON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_display_off END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_blink_on(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_blink_on()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control |= LCD_BLINKON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_blink_on END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_blink_off(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_blink_off()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control &= ~LCD_BLINKON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_blink_off END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_cursor_on(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_cursor_on()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control |= LCD_CURSORON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_cursor_on END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_cursor_off(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_cursor_off()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control &= ~LCD_CURSORON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_cursor_off END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_backlight_on(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_backlight_on()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_set_backlight(1);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_backlight_on END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_backlight_off(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_backlight_off()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_set_backlight(0);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_backlight_off END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_move_left(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_move_left()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_send_command(LCD_CURSORSHIFT | \
                               LCD_DISPLAYMOVE | \
                               LCD_MOVELEFT);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_move_left END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_move_right(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_move_right()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_send_command(LCD_CURSORSHIFT | \
                               LCD_DISPLAYMOVE | \
                               LCD_MOVERIGHT);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_move_right END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_left_to_right(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_left_to_right()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_entry_mode |= LCD_ENTRYLEFT;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_entry_mode);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_left_to_right END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_right_to_left(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_right_to_left()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_entry_mode &= ~LCD_ENTRYLEFT;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_entry_mode);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_right_to_left END", id, id, sp);
#endif	/* TRACE */
}

/* This will 'right justify' text from the cursor */
void pifacecad_lcd_autoscroll_on(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_autoscroll_on()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control |= LCD_ENTRYSHIFTINCREMENT;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_autoscroll_on END", id, id, sp);
#endif	/* TRACE */
}

/* This will 'left justify' text from the cursor */
void pifacecad_lcd_autoscroll_off(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_auto_scroll_off()", id, id, sp), id+=2;
#endif	/* TRACE */
    cur_display_control &= ~LCD_ENTRYSHIFTINCREMENT;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_display_control);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_auto_scroll_off END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_write_custom_bitmap(uint8_t location)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_write_custom_bit_map(location = %u)", id, id, sp, (unsigned)location), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_send_command(LCD_SETDDRAMADDR | cur_address);
    pifacecad_lcd_send_data(location);
    cur_address++;
#ifdef	TRACE 
    if (iC_debug & 040) fprintf(iC_outFP, "\n%*.*scur_address = %d", id, id, sp, cur_address);
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_write_custom_bit_map END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_store_custom_bitmap(uint8_t location, uint8_t bitmap[])
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_store_custom_bit_map(location = %u, %p)", id, id, sp, (unsigned)location, bitmap), id+=2;
#endif	/* TRACE */
    location &= 0x07; /* we only have 8 locations 0-7 */
    pifacecad_lcd_send_command(LCD_SETCGRAMADDR | (location << 3));
    int i;
    for (i = 0; i < 8; i++) {
        pifacecad_lcd_send_data(bitmap[i]);
    }
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_store_custom_bit_map END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_send_command(uint8_t command)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_send_command(command = 0x%02x)", id, id, sp, (unsigned)command), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_set_rs(0);
    pifacecad_lcd_send_byte(command);
    sleep_ns(DELAY_SETTLE_NS);				/* 40 us */
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_send_command END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_send_data(uint8_t data)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_send_data(data = 0x%02x)", id, id, sp, (unsigned)data), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_set_rs(1);
    pifacecad_lcd_send_byte(data);
    sleep_ns(DELAY_SETTLE_NS);				/* 40 us */
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_send_data END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_send_byte(uint8_t b)
{
    uint8_t current_state;
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_send_byte(b = 0x%02x)", id, id, sp, (unsigned)b), id+=2;
#endif	/* TRACE */
    /* get current lcd port state and clear the data bits */
    current_state = readData[iC_CADpfa][LCD_PORT];
    current_state &= 0xf0; /* clear the data bits */
#ifdef	TRACE 
    if (iC_debug & 040) {
	fprintf(iC_outFP, "\n%*.*s%02x", id, id, sp, current_state);
	if ((b & 0x7f) < 0x40 || (b & 0x7f) == 0x7f) {
	    fprintf(iC_outFP, " %02x	\\%o", b, b);
	} else {
	    fprintf(iC_outFP, " %02x	%c", b, b);
	}
    }
#endif	/* TRACE */

    /* send first nibble (0bXXXX0000) */
    uint8_t new_byte = current_state | ((b >> 4) & 0x0f);  /* set nibble */
    writeByte(iC_CAD_FD, iC_CADpfa, LCD_PORT, new_byte);
    pifacecad_lcd_pulse_enable();

    /* send second nibble (0b0000XXXX) */
    new_byte = current_state | (b & 0x0f);  /* set nibble */
    writeByte(iC_CAD_FD, iC_CADpfa, LCD_PORT, new_byte);
    pifacecad_lcd_pulse_enable();
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_send_byte END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_set_rs(uint8_t state)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_rs(state = %u)", id, id, sp, (unsigned)state), id+=2;
#endif	/* TRACE */
    writeBit(iC_CAD_FD, iC_CADpfa, LCD_PORT, PIN_RS, state);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_rs END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_set_rw(uint8_t state)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_ser_rw(state = %u)", id, id, sp, (unsigned)state), id+=2;
#endif	/* TRACE */
    writeBit(iC_CAD_FD, iC_CADpfa, LCD_PORT, PIN_RW, state);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_ser_rw END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_set_enable(uint8_t state)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_enable(state = %u)", id, id, sp, (unsigned)state), id+=2;
#endif	/* TRACE */
    writeBit(iC_CAD_FD, iC_CADpfa, LCD_PORT, PIN_ENABLE, state);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_enable END", id, id, sp);
#endif	/* TRACE */
}

void pifacecad_lcd_set_backlight(uint8_t state)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_backlight(state = %u)", id, id, sp, (unsigned)state), id+=2;
#endif	/* TRACE */
    writeBit(iC_CAD_FD, iC_CADpfa, LCD_PORT, PIN_BACKLIGHT, state);
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_set_backlight END", id, id, sp);
#endif	/* TRACE */
}

/* pulse the enable pin */
void pifacecad_lcd_pulse_enable(void)
{
#ifdef	TRACE 
    if (iC_debug & 04) fprintf(iC_outFP, "\n%*.*spifacecad_lcd_pulse_enable()", id, id, sp), id+=2;
#endif	/* TRACE */
    pifacecad_lcd_set_enable(1);
    sleep_ns(DELAY_PULSE_NS);					/* 1 us */
    pifacecad_lcd_set_enable(0);
    sleep_ns(DELAY_PULSE_NS);					/* 1 us */
#ifdef	TRACE 
    if (iC_debug & 04) id-=2, fprintf(iC_outFP, "\n%*.*spifacecad_lcd_pulse_enable END", id, id, sp);
#endif	/* TRACE */
}

uint8_t colrow2address(uint8_t col, uint8_t row)
{
    return col + ROW_OFFSETS[row];
}

uint8_t address2col(uint8_t address)
{
    return address % ROW_OFFSETS[1];
}

uint8_t address2row(uint8_t address)
{
    return address > ROW_OFFSETS[1] ? 1 : 0;
}

static void sleep_ns(long nanoseconds)
{
    struct timespec time0, time1;
    time0.tv_sec = 0;
    time0.tv_nsec = nanoseconds;
    nanosleep(&time0 , &time1);
#ifdef	TRACE 
    if (iC_debug & 040) fprintf(iC_outFP, "\tzz %ld ns", nanoseconds);
#endif	/* TRACE */
}
#endif	/* RASPBERRYPI */

/********************************************************************
 *
 *	Special code to support PiFace Control and Display.
 *	Display a string either directly on a PiFaceCAD
 *	or send it via TCP/IP on PFCAD4 to be displayed
 *	  displayString	formatted for display on a PiFaceCAD
 *	  channel	0 = direct display or > 0 && < 0xfff0 iCserver channel
 *
 *******************************************************************/

void
writePiFaceCAD(const char * displayString, unsigned short channel)
{
    char *	cp;
    char	buf[101];

    if (channel == 0) {
#ifdef	RASPBERRYPI
	pifacecad_lcd_clear();
	pifacecad_lcd_write(displayString);	/* write direct to PiFaceCAD */
#else	/* RASPBERRYPI */
	fprintf(stderr, "ERROR: cannot write directly to PiFaceCAD\n");
	iC_quit(SIGUSR1);
#endif	/* RASPBERRYPI */
    } else
    if (channel < 0xfff0) {
	cp = buf;
	if (snprintf(buf, 100, "%hu:%s", channel, displayString) > 100) {
	    buf[100] = '\0';			/* terminate in case of overflow (unlikely) */
	}
#if	YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0200) {
	    fprintf(iC_outFP, "writePiFaceCAD: '%s'\n", buf);	/* terminate with a CR in case last line has none */
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	while ((cp = strchr(cp+1 , ',')) != NULL) {	/* no commas in channel: */
	    *cp = '\036';			/* replace every comma by ASCII RS */
	}
	assert(iC_sockFN > 0);
	iC_send_msg_to_server(iC_sockFN, buf);
    } else {
	fprintf(stderr, "ERROR: unable to register as PFCAD4 sender - is PiFaceCAD input correct ?\n");
	iC_quit(SIGUSR1);
    }
} /* writePiFaceCAD */

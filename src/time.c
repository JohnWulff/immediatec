static const char time_c[] =
"@(#)$Id: time.c,v 1.1 1996/07/30 16:18:20 john Exp $";
/********************************************************************
 *
 * 	Timer 0 wird als Zeitbasis der PPLC benutzt (1ms). 
 *	8.10.94 / mg
 *
 *******************************************************************/
/* "time.c	0.02	95/02/07" */

#include <dos.h>
#include "time.h"
#include "ibsd.h"

#define INT 0x8					/* Timerinterrupt */

void interrupt (*oldint8)(void);		/* old vector */

/********************************************************************
 *
 * 	Subroutine zur Initialisierung des Timers
 *
 *******************************************************************/
void tinit(void)
{
						/* Timer0-Interrupt: */
    oldint8 = getvect(INT);			/* alten Vektor retten */
    setvect(INT, isrtimer);			/* Pointer verbiegen */

						/* 8253, counter 0: */
    outportb(CT_CMD, COUNTER0+SQWAVE);		/* Squarewave fuer Interrupt */
    outportb(CT_0, VALUE0_LO);
    outportb(CT_0, VALUE0_HI);

    outportb(0x21, inportb(0x21) & 0xfe);	/* Int-maske IRQ0 enable */

} /* tinit */

/********************************************************************
 *
 * 	Subroutine zum Rücksetzen der alten Werte
 *
 *******************************************************************/

void trestore(void)
{

//  outportb(0x21, inportb(0x21) | 1);		/* Int-maske IRQ0 disable */
//  !!! Nicht benutzen, Turbo-Debugger hängt sich auf !!!

						/* 8253, counter 0: */
    outportb(CT_CMD, COUNTER0+SQWAVE);		/* Squarewave fuer Interrupt */
    outportb(CT_0, 0xff);			/* 1.19381 MHz % 65536 */
    outportb(CT_0, 0xff);

    setvect(INT, oldint8);			/* alten Vektor restaurieren */

} /* trestore */

static const char ibsd_h[] =
"@(#)$Id: ibsd.h,v 1.3 2001/03/02 12:56:32 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john.wulff@inka.de>
 *
 *	ibsd.h
 *	parallel plc - runtime support for Phoenix Contact COP card
 *		       connected to InterBus-S field bus I/O system
 *
 * 	Headerfile zur Treibersoftware fuer den IBS-COP
 *	20.9.94 / mg
 *
 *******************************************************************/

/* "@(#)ibsd.h		0.11	95/05/12" */

#ifndef __cplusplus
#define TRACE0(format) printf(format)
#define TRACE1(format,v1) printf(format,v1)
#define TRACE2(format,v1,v2) printf(format,v1,v2)
#define TRACE3(format,v1,v2,v3) printf(format,v1,v2,v3)
#define ASSERT(v) if (!(v))\
    printf("ASSERTION failed in %s: line %d\n", __FILE__,__LINE__)
#else
extern "C" {
#endif

/*----------------------------------------------------------------- */
/* Definitionen */
/*----------------------------------------------------------------- */
#define TRUE		1
#define FALSE		0
#define MAXBIN		2048 / 16	/* Binaerdaten werden als Worte */
					/* im Speicher abgelegt */
#define MAXWORD		2048		/* Anzahl Worte zum Master */
#define MAXMAIL		256		/* Groesse der Master Mailbox */

/*----------------------------------------------------------------- */
/* Definitionen fuer COP / Host-PC Kommunikation */
/*----------------------------------------------------------------- */
#define HOSTMAIL	256		/* Groesse der Host Mailbox */
#define MSG_ERROR		0x00
#define GET_SYMBOL_TABLE	0x01
#define GET_NEXT		0x02
#define ACKNOWLEDGE		0x8003
#define GET_END			0x04
#define PUT_LIVE_CMD	0x05
#define GET_LIVE_DATA	0x06

/*----------------------------------------------------------------- */
/* Macros zum Datenaustausch mit der PLC */
/*----------------------------------------------------------------- */

/* Definition of the control structure for message buffers */

typedef struct MsgBuf {
    short		cnt;	/* number of bytes in the buffer */
    unsigned char *	ptr;	/* current buffer pointer */
    unsigned char *	base;	/* &HostInMailX[4], X is In or OUT */
} MsgBuf;

/********************************************************************
 *
 *	buffered message input
 *
 *	modelled on getc() from stdio.h
 *
 *	fillMsg is only executed if buffer is empty
 *
 *	return value is	next character from message buffer (success)
 *	             or EOF (failure)
 *
 *******************************************************************/

#define getM(m)		( (--((m)->cnt) >= 0) \
			? (unsigned char)(*(m)->ptr++) : fillMsg (m) )

#define getMsg()	getM(inMsg)

/********************************************************************
 *
 *	buffered message output
 *
 *	putM() operation:
 *	if cnt > 0 (not empty) || flushMsg() TRUE
 *	    store character, value is cnt-- which is > 0 (no error)
 *	else value is 0 (error)
 *
 *	flushMsg is only executed if buffer is empty (action of ||)
 *
 *	this scheme allows a flushMsg() which does not itself
 *	provide a character on completion of a transfer
 *
 *	return value is	> 0 if putM() was successful
 *	                  0 if putM() failed
 *
 *******************************************************************/

#define putM(c,m)	( (m)->cnt > 0 || flushMsg(m) \
			? (*(m)->ptr++ = (unsigned char)(c), (m)->cnt--) \
			: 0 )

#define putMsg(c)	putM(c,outMsg)

extern int	fillMsg (MsgBuf * inM);	/* used by getM() macro */
extern int	flushMsg(MsgBuf *outM);	/* used by putM() macro */

extern MsgBuf *	inMsg;
extern MsgBuf *	outMsg;

/*----------------------------------------------------------------- */
/* Strukturen zum Datenaustausch mit der PLC */
/*----------------------------------------------------------------- */
extern unsigned short DataIn[MAXBIN];		/* IBS binaere Eingaben */
extern unsigned short DataOut[MAXBIN];		/* IBS binaere Ausgaben */

extern unsigned short HostDataIn[MAXBIN];	/* Host in Daten */
extern unsigned short HostDataOut[MAXBIN];	/* Host out Daten */

extern unsigned char HostInMail[HOSTMAIL];	/* Host in Mailbox */
extern unsigned char HostOutMail[HOSTMAIL];	/* Host out Mailbox */

/*----------------------------------------------------------------- */
/* Funktionsdeklarationen */
/*----------------------------------------------------------------- */
extern short InitDataLink(void);
extern short CloseDataLink(void);
extern short StartIBS(void);
extern short StopIBS(void);
extern short IBSsendData(void);		/* IBS binaere Ausgabe */
extern short IBSrecvData(void);		/* IBS binaere Eingabe */

extern short sendData(void);		/* Host Ausgabe */
extern short recvData(void);		/* Host Eingabe */
extern short sendMsg(unsigned int length);
extern short recvMsg(void);
extern void recvLiveCmd(void);
extern void scanLiveCmd(void);

/*----------------------------------------------------------------- */
/* Funktionsdeklarationen nur fuer COP */
/*----------------------------------------------------------------- */
extern int ibsdrv(void);
extern void scanIn(void);

/*----------------------------------------------------------------- */
/* Funktionsdeklarationen nur fuer PC */
/*----------------------------------------------------------------- */
extern unsigned short	getSymbolTable(void);
extern unsigned short	lookupST(char * name);
extern char *		sTname(unsigned short sTx);
extern char		sTtype(unsigned short sTx);
extern void		setCmdBit(unsigned short n);
extern void		clearCmdBit(unsigned short n);
extern unsigned short	testDataBit(unsigned short n);
extern void		clearLiveCmd(void);
extern void		sendLiveCmd(void);

#ifdef __cplusplus
}
#endif

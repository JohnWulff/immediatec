static const char ibsd.c[] =
"@(#)$Id: ibsd.c,v 1.4 2001/03/02 12:56:32 jw Exp $";
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
 *	ibsd.c
 *	parallel plc - runtime support for Phoenix Contact COP card
 *		       connected to InterBus-S field bus I/O system
 *
 * 	Treibersoftware fuer den IBS-COP
 *	15.8.94 / mg
 *
 *	Das Programm wird ueber die Download-Funktion von TDOS
 *	auf den COP gebracht und dort gestartet.
 *	Bei der Portierung der PPLC sind die Programmteile in
 *	das Laufzeitsystem einzubinden.
 * 
 *******************************************************************/

/* "@(#)ibsd.c	0.12	95/06/18" */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>

#define P_8086		/* fuer Include-Dateien notwendig */
#define C_TURBO_C	/* Syntax fuer stdtypes.h, s.S. 4-45 Handbuch */

#include <ibs_dos.h>
#include <ddi_lib.h>

#include "ibsd.h"
#include "icc.h"

/*----------------------------------------------------------------- */
/* Arrays zum Datenhandling zwischen PLC und IBS */
/*----------------------------------------------------------------- */
unsigned short DataIn[MAXBIN];		/* IBS binaere Eingaben */
unsigned short DataOut[MAXBIN];		/* IBS binaere Ausgaben */
unsigned short WordInPCP[MAXWORD];	/* PCP Worteingaben */
unsigned short WordOutPCP[MAXWORD];	/* PCP Wortausgaben */

static USIGN8 InMail[MAXMAIL]; 		/* Empfangspuffer Nachrichten */
static USIGN8 OutMail[MAXMAIL];		/* Sendepuffer Nachrichten */

unsigned short HostDataIn[MAXBIN];	/* Host in Daten */
unsigned short HostDataOut[MAXBIN];	/* Host out Daten */

USIGN8 HostInMail[HOSTMAIL];		/* Host in Mailbox */
USIGN8 HostOutMail[HOSTMAIL];		/* Host out Mailbox */

MsgBuf	inMsgCtrl;			/* Host in Mailbox Control */
MsgBuf	outMsgCtrl;			/* Host out Mailbox Control */
MsgBuf*	inMsg;				/* Host in Mailbox Control pointer */
MsgBuf*	outMsg;				/* Host out Mailbox Control pointer */

/********************************************************************
 *
 *	live data
 *
 *******************************************************************/

static unsigned char *	liveCmd;/* dynamic array for commands to COP */
static size_t		lClen;	/* length of liveCmd and bits in liveData */
static unsigned char *	liveData;/* dynamic array for data from COP */
static size_t		lDlen;	/* total length of liveData */
static unsigned short	wordCnt;/* number of words at end of liveData */
				/* ZZZ wordCnt is 0 for now */
static unsigned short	blockCommand = GET_NEXT;/* command for next fillMsg */

/*----------------------------------------------------------------- */
/* Node-Handles, werden mit der Funktion DDI_DevOpenNode ermittelt */
/*----------------------------------------------------------------- */
static INT16   MastDtiNH;    /* Node-Handle fuer Master Data-Interface */
static INT16   MastMxiNH;    /* Node-Handle fuer Master Mailbox-Interface */
static INT16   HostDtiNH;    /* Node-Handle fuer Host Data-Interface */
static INT16   HostMxiNH;    /* Node-Handle fuer Host Mailbox-Interface */

/********************************************************************
 *
 * 	Execute Interbus command
 *
 *******************************************************************/

short
execIBS(INT16 nodeHd, INT16 command, INT16 message)
{
    T_DDI_MXI_ACCESS	mxiAcc;
    INT16		i;

    IB_SetCmdCode(OutMail, command); 

    mxiAcc.msgType   = 0;	/* Message-Typ = 0, momentan ohne Bedeutung */
    mxiAcc.DDIUserID = 0;	/* DDIUser-ID = 0, momentan ohne Bedeutung */
    mxiAcc.msgLength = 2; 	/* max. Laenge des Kommandoblocks in Bytes */
    mxiAcc.msgBlk    = OutMail; /* Zeiger auf Puffer mit dem Kommandoblock */
	
    if ((DDI_MXI_SndMessage(nodeHd, &mxiAcc)) != ERR_OK) {
	ReleaseDDI();
	return FALSE;
    }

    /*----------------------------------------------------------------- */
    /* Bestaetigung des Kommandos abwarten */
    /* !!!ACHTUNG!!! Muss durch Timer begrenzt werden! */
    /*----------------------------------------------------------------- */

    mxiAcc.msgType   = 0;	/* Message-Typ = 0, momentan ohne Bedeutung */
    mxiAcc.DDIUserID = 0;	/* DDIUser-ID = 0, momentan ohne Bedeutung */
    mxiAcc.msgLength = MAXMAIL;	/* max. Laenge des Messageblocks in Bytes */
    mxiAcc.msgBlk    = InMail;	/* Zeiger auf Puffer fuer den Messageblock */

    i = 1; while (DDI_MXI_RcvMessage(nodeHd, &mxiAcc) == ERR_NO_MSG && ++i);
    if (i == 0 || IB_GetMsgCode(InMail) != message) {
	ReleaseDDI();
	return FALSE;
    }
    return TRUE;
} /* execIBS */

/********************************************************************
 *
 * 	DDI-Treiber und Datenkanaele initialisieren
 *
 *******************************************************************/

short
InitDataLink(void)
{
    /*----------------------------------------------------------------- */
    /* Initialisierung der Treibersoftware fuer den COP */
    /*  - Boardnumber = 1 */
    /*  - IO-Address  = 120(hex) */
    /*  - MPM-Address = D000(hex) */
    /*  - Interrupt   = IRQ5 */
    /*----------------------------------------------------------------- */

    if (InitDDI(1, 0x120, 0xD000, 5) != ERR_OK) {
	return FALSE;
    }

    /*----------------------------------------------------------------- */
    /* open mailbox controls */
    /*----------------------------------------------------------------- */

    inMsgCtrl.cnt = outMsgCtrl.cnt = HOSTMAIL - 4;
    inMsgCtrl.ptr = (inMsgCtrl.base = HostInMail) + 4;
    outMsgCtrl.ptr = (outMsgCtrl.base = HostOutMail) + 4;
    inMsg = &inMsgCtrl;
    outMsg = &outMsgCtrl;

/********************************************************************
 *
 *	allocate live data blocks
 *
 *******************************************************************/

    lClen = ((sTend - sTable) + 7) >> 3;	/* length of liveCmd[] */
    liveCmd = (unsigned char *) calloc(lClen, sizeof(unsigned char));
    lDlen = lClen + wordCnt * 2;		/* length of liveData[] */
    liveData = (unsigned char *) calloc(lDlen, sizeof(unsigned char));

    /*----------------------------------------------------------------- */
    /* Mailbox- und Data-Interface Kanaele (MXI und DTI) */
    /* zum IBS-Master (Node 1) oeffnen */
    /*  - Device-Names:     IBB1N1_M -> B1=Board1, N1=Node1, M=Mailbox */
    /*  		    IBB1N1_D -> B1=Board1, N1=Node1, D=Data */
    /* zum Host-PC (Node 0) oeffnen */
    /*  - Device-Names:     IBB1N0_M -> B1=Board1, N0=Node0, M=Mailbox */
    /*  		    IBB1N0_D -> B1=Board1, N0=Node0, D=Data */
    /*  - Zugriffsrechte:   DDI_RW (Lesen und Schreiben)  */
    /*  - Zeiger auf Variable fuer Node-Handle */
    /*----------------------------------------------------------------- */

    return
    DDI_DevOpenNode("IBB1N1_M", DDI_RW, &MastMxiNH) == ERR_OK &&
    DDI_DevOpenNode("IBB1N1_D", DDI_RW, &MastDtiNH) == ERR_OK &&
    DDI_DevOpenNode("IBB1N0_M", DDI_RW, &HostMxiNH) == ERR_OK &&
    DDI_DevOpenNode("IBB1N0_D", DDI_RW, &HostDtiNH) == ERR_OK;
} /* InitDataLink */

/********************************************************************
 *
 * 	InterBus starten
 *
 *******************************************************************/

short
StartIBS(void)
{
    return
    /*----------------------------------------------------------------- */
    /* Clear display request */
    /*----------------------------------------------------------------- */
    execIBS(MastMxiNH, IBSM_CLR_DSPLY_REQ, IBSM_CLR_DSPLY_CNF) &&
    /*----------------------------------------------------------------- */
    /* Configure INTERBUS */
    /*----------------------------------------------------------------- */
    execIBS(MastMxiNH, IBSM_CNFG_BUS_REQ, IBSM_CNFG_BUS_CNF) &&
    /*----------------------------------------------------------------- */
    /* Start INTERBUS-S */
    /*----------------------------------------------------------------- */
    execIBS(MastMxiNH, IBSM_STRT_BUS_REQ, IBSM_STRT_BUS_CNF);
} /* StartIBS */

/********************************************************************
 *
 * 	InterBus stoppen
 *
 *******************************************************************/

short
StopIBS(void)
{
    return
    /*----------------------------------------------------------------- */
    /* Stop INTERBUS-S */
    /* Zum Abschalten der Ausgaben wird das Kommando AlarmStopRequest */
    /* verwendet, IBSM_STOP_BUS_REQ haelt nur den Bus an!!! */
    /*----------------------------------------------------------------- */
    execIBS(MastMxiNH, IBSM_ALRM_STOP_REQ, IBSM_ALRM_STOP_CNF);
} /* StopIBS */

/********************************************************************
 *
 * 	Verbindungen zu den Interfaces (Data & Mailbox) schliessen
 *
 *******************************************************************/

short
CloseDataLink(void)
{
    if (DDI_DevCloseNode(MastMxiNH) != ERR_OK ||
	DDI_DevCloseNode(MastDtiNH) != ERR_OK ||
	DDI_DevCloseNode(HostMxiNH) != ERR_OK ||
	DDI_DevCloseNode(HostDtiNH) != ERR_OK) return FALSE;
    /*----------------------------------------------------------------- */
    /* Treibersoftware entfernen und Programm beenden */
    /*----------------------------------------------------------------- */
    ReleaseDDI();		/* setzt Interrupt Vektoren zurueck */
    return TRUE;
} /* CloseDataLink */

/********************************************************************
 *
 * 	Ausgabe der Prozessdaten
 *
 *******************************************************************/

short
IBSsendData(void)
{
    T_DDI_DTI_ACCESS  dtiAcc;

    dtiAcc.address  = DTA_OFFST_IBS_MASTER;	/* Offset-Adresse */
    dtiAcc.length   = MAXBIN * 2; 		/* Groesse Datenbereich */
    dtiAcc.dataCons = DTI_DATA_WORD; 		/* Datenkonsistenz: Word */
    dtiAcc.data     = (USIGN8 FAR *)DataOut; 	/* Adr. OUT-Data-Puffer */

    if (DDI_DTI_WriteData(MastDtiNH, &dtiAcc) != ERR_OK) return FALSE;
    return TRUE;
} /* IBSsendData */

/********************************************************************
 *
 * 	Eingabe der Prozessdaten
 *
 *******************************************************************/

short
IBSrecvData(void)
{
    T_DDI_DTI_ACCESS  dtiAcc;

    dtiAcc.address  = DTA_OFFST_IBS_MASTER;	/* Offset-Adresse */
    dtiAcc.length   = MAXBIN * 2; 		/* Groesse Datenbereich */
    dtiAcc.dataCons = DTI_DATA_WORD; 		/* Datenkonsistenz: Word */
    dtiAcc.data     = (USIGN8 FAR *)DataIn; 	/* Adr. IN-Data-Puffer */

    if (DDI_DTI_ReadData(MastDtiNH, &dtiAcc) != ERR_OK) return FALSE;
    return TRUE;
} /* IBSrecvData */

/********************************************************************
 *
 * 	Ausgabe der Hostdaten - lebendes Bild
 *
 *******************************************************************/

short
sendData(void)
{
    T_DDI_DTI_ACCESS  dtiAcc;

    dtiAcc.address  = DTA_OFFST_COP_TO_PC;	/* Offset-Adresse */
    dtiAcc.length   = MAXBIN * 2; 		/* Groesse Datenbereich */
    dtiAcc.dataCons = DTI_DATA_WORD; 		/* Datenkonsistenz: Word */
    dtiAcc.data     = (USIGN8 FAR *)HostDataOut;/* Adr. OUT-Data-Puffer */

    if (DDI_DTI_WriteData(HostDtiNH, &dtiAcc) != ERR_OK) return FALSE;
    return TRUE;
} /* sendData */

/********************************************************************
 *
 * 	Eingabe der Hostdaten - not used at present
 *
 *******************************************************************/

short
recvData(void)
{
    T_DDI_DTI_ACCESS  dtiAcc;

    dtiAcc.address  = DTA_OFFST_COP_TO_PC;	/* Offset-Adresse */
    dtiAcc.length   = MAXBIN * 2; 		/* Groesse Datenbereich */
    dtiAcc.dataCons = DTI_DATA_WORD; 		/* Datenkonsistenz: Word */
    dtiAcc.data     = (USIGN8 FAR *)HostDataIn;	/* Adr. IN-Data-Puffer */

    if (DDI_DTI_ReadData(HostDtiNH, &dtiAcc) != ERR_OK) return FALSE;
    return TRUE;
} /* recvData */

/********************************************************************
 *
 *	Nachricht zum Host-Pc senden
 *
 *	Rueckgabewert:	Laenge zu gross oder Fehler FALSE
 *	sonst		TRUE
 *
 *******************************************************************/

short
sendMsg(unsigned int length)
{
    T_DDI_MXI_ACCESS	mxiAcc;

    if (length > HOSTMAIL) return FALSE;
    mxiAcc.msgType   = 0;    		/* Message-Typ = 0, (n.u.) */
    mxiAcc.DDIUserID = 0;		/* DDIUser-ID = 0, (n.u.) */
    mxiAcc.msgLength = length;		/* Laenge in Bytes */
    mxiAcc.msgBlk    = HostOutMail;	/* Puffer Messageblock */
    if (DDI_MXI_SndMessage(HostMxiNH, &mxiAcc) != ERR_OK) return FALSE;
    return TRUE;
} /* sendMsg */

/********************************************************************
 *
 *	Nachricht vom Host-Pc empfangen
 *
 *	Rueckgabewert:	Keine Nachricht oder Fehler 0 (FALSE)
 *	oder		Laenge der empfangenen Nachricht (TRUE)
 *
 *******************************************************************/

short
recvMsg(void)
{
    T_DDI_MXI_ACCESS	mxiAcc;

    mxiAcc.msgType   = 0;    		/* Message-Typ = 0, (n.u.) */
    mxiAcc.DDIUserID = 0;		/* DDIUser-ID = 0, (n.u.) */
    mxiAcc.msgLength = HOSTMAIL; 	/* Laenge in Bytes */
    mxiAcc.msgBlk    = HostInMail;	/* Puffer Messageblock */

    if (DDI_MXI_RcvMessage(HostMxiNH, &mxiAcc) != ERR_OK) {
	return 0;			/* ERR_NO_MSG oder anderer Fehler */
    }
    return mxiAcc.msgLength;		/* muss mindestens 1 Byte sein */
} /* recvMsg */

/********************************************************************
 *
 *	Fill the 'getMsg' buffer and return when either the buffer
 *	is full or no connection
 *
 *	returns the first character in the buffer or EOF on failure
 *
 *******************************************************************/

int
fillMsg(MsgBuf * m)
{
    int		ln;
    int		i;

    IB_SetCmdCode(outMsg->base, blockCommand);
    ASSERT(sendMsg(2) == TRUE);
    blockCommand = GET_NEXT;	/* next block from COP */
    i = 16384; do scanIn(); while (!(ln = recvMsg()) && --i);
    ASSERT(i != 0);			/* timeout error */
    TRACE3("fillMsg(): ln = %d %04x %d\n", ln,
	    IB_GetMsgCode(m->base), IB_GetMsgCode(m->base+2));
    ASSERT((USIGN16)IB_GetMsgCode(m->base) == ACKNOWLEDGE);
    m->ptr = m->base + 4;		/* start with new buffer */
    ASSERT((m->cnt = ln - 5) >= 0);	/* and output counter */

    return *m->ptr++;			/* first character in new buffer */
} /* fillMsg */

/********************************************************************
 *
 *	Flush the current 'putM' buffer by sending it to another
 *	platform by calling 'sendMsg' and receiving a GET_ACK
 *
 *	return value is	> 0 if flushMsg() was successful
 *	                  0 if flushMsg() failed, no rcvMsg(ACK)
 *
 *******************************************************************/

int
flushMsg(MsgBuf * m)
{
    int		wcnt;		/* words in buffer */
    int		ln;
    int		i;

    wcnt = (HOSTMAIL - 3 - m->cnt) >> 1;
    IB_SetParaCnt(m->base, wcnt);	/* number of words in message */
    printf("message word length = %d\n", wcnt);
    sendMsg(m->ptr - m->base);	/* at least 1 byte in message */
    i = 16384; do scanIn(); while (!(ln = recvMsg()) && --i);
    ASSERT(i != 0);			/* timeout error */
    TRACE3("flushMsg(): ln = %d %04x %d\n", ln,
	    IB_GetMsgCode(inMsg->base), IB_GetMsgCode(inMsg->base+2));
    m->ptr = m->base + 4;		/* start with new buffer */
    m->cnt = HOSTMAIL - 4;		/* and output counter */
    if (IB_GetMsgCode(inMsg->base) != GET_NEXT) return 0;
    printf("GET_NEXT received\n");
    IB_SetCmdCode(m->base, ACKNOWLEDGE);	/* ZZZ */

    return 1;				/* putM() can continue collecting */
} /* flushMsg */

/********************************************************************
 *
 *	receive liveCmd[] from PC
 *
 *	liveData[] in the PC are cleared and in the COP also
 *
 *	the length is sTlen / 8  this is known to the COP
 *
 *	the COP immediately sends updated liveData[] and retransmits
 *	whenever any changes in the current liveData has occurred.
 *
 *	if all the bits in the liveCmd are 0, retransmission stops
 *	automatically becaus liveData is then also 0.
 *
 *******************************************************************/

void
recvLiveCmd(void)
{
    unsigned char *	lcp;
    unsigned char *	ep;

    TRACE0("recvLiveCmd(): PUT_LIVE_CMD\n");
    blockCommand = GET_NEXT;		/* request from PC */
    inMsg->cnt = 0;			/* force filbuf() ZZZ hide in open */
    for (ep = (lcp = liveCmd) + lClen; lcp < ep; lcp++) {
	*lcp = getMsg();
	TRACE1("%02x ", *lcp);
    }
    TRACE1(" -> cnt = %d\n", inMsg->cnt);
    ASSERT(inMsg->cnt >> 1 == 0);		/* check last buffer empty */
    TRACE0(" -> sendMsg GET_END\n");
    IB_SetCmdCode(outMsg->base, GET_END);/* terminate block transfer */
    ASSERT(sendMsg(2) == TRUE);
    blockCommand = MSG_ERROR;		/* must start with new block */
} /* recvLiveCmd */

/********************************************************************
 *
 *	scan live command
 *
 *	scan liveCmd[] and generate liveData[]
 *	if (liveData[] != liveCopy[]) {
 *	    copy liveData[] to liveCopy[]
 *	    send liveData[] to PC
 *	}
 *
 *	ASSERT(Sizeof(symbol table) == Bitsizeof(liveCmd));
 *
 *	TEST LIVE DATA with Data node not messages
 *	to do this call sendData() when data has changed
 *
 *	NOTE: ***** must test not more than 2048 entries in symbol table
 *		unlikely but DO IT
 *
    for (lcp = liveCmd, ldp = liveData, opp = sTable; opp < sTend; opp += 8) {
 *
 *
 *******************************************************************/

void
scanLiveCmd(void)		/* generate live data */
{
    unsigned char *	lcp;
    unsigned char *	ldp;
    unsigned char *	ep;
    Gate **		opp;		/* symbol table in COP */
    int			changeFlag = 0;

    for (lcp = liveCmd, ldp = (unsigned char *)HostDataOut, opp = sTable;
	opp < sTend; opp += 8) {
	unsigned char	cmd;
	unsigned char	data = 0;

	if ((cmd = *lcp++) != 0) {
	    register Gate **	ip = opp;
	    register Gate *	gp;
	    int			mask = 1;
	    int			inversion;

	    do {			/* assert cmd != 0 */
		if (cmd & 1) {
		    gp = *ip;
		    inversion = 0;
		    while (gp->gt_ini == -ALIAS) {
			inversion ^= gp->gt_mark;
			gp = (Gate*)gp->gt_rlist;
		    }
		    if ((gp->gt_val & 0x80) ^ (inversion << 7)) {
			data |= mask;
		    }
		}
		ip++;
		mask <<= 1;
	    } while (cmd >>= 1);
	    scanIn();
	}
	if (*ldp != data) {		/* check if data has changed */
	    *ldp = data;		/* yes - store new data */
	    changeFlag = 1;		/* mark change */
	}
	ldp++;
    }

    if (changeFlag) {
	sendData();		/* thats all */
    }
} /* scanLiveCmd */

static const char pccop_c[] =
"@(#)$Id: pccop.c,v 1.4 2001/03/30 17:31:20 jw Exp $";
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
 *	pccop.c
 *	parallel plc - runtime test with Phoenix Contact COP card
 *		       connected to InterBus-S field bus I/O system
 *
 * 	Datenaustausch PC --> COP
 *	16.1.95 / mg
 *
 *	!! A C H T U N G !!
 *	Vor dem Start dieses Programmes muss auf dem PC das TSR-
 *	Programm ibspccb.exe gestartet werden!
 *
 *******************************************************************/

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

/*----------------------------------------------------------------- */
/* Funktionsdeklarationen */
/*----------------------------------------------------------------- */
short InitDataLink(void);
short CloseDataLink(void);
short recvData(void);
short sendData(void);
short recvMsg(void);
short sendMsg(unsigned int length);
unsigned short	getSymbolTable(void);

/*----------------------------------------------------------------- */
/* Arrays zum Datenhandling zwischen PC und COP */
/*----------------------------------------------------------------- */

unsigned short HostDataIn[MAXBIN];	/* Host in Daten */
unsigned short HostDataOut[MAXBIN];	/* Host out Daten */

unsigned char HostInMail[HOSTMAIL];	/* Host in Mailbox */
unsigned char HostOutMail[HOSTMAIL];	/* Host out Mailbox */

/*----------------------------------------------------------------- */
/* Node-Handles, werden mit der Funktion DDI_DevOpenNode ermittelt */
/*----------------------------------------------------------------- */
static INT16   HostDtiNH;    /* Node-Handle fuer Host Data-Interface */
static INT16   HostMxiNH;    /* Node-Handle fuer Host Mailbox-Interface */

typedef struct sT {
	char *		name;		/* variable name */
	unsigned short	use;		/* use count */
	unsigned char	type;		/* Symbol type */
} sT;

static long	sTstrLen;
static sT *	sTable;	/* dynamic array for symbol table from COP */
static sT *	sTend;
static int	ec = 0;

/********************************************************************
 *
 * 	Main
 *
 *******************************************************************/

int
main(void)
{
    sT *	sTp;

    if (InitDataLink() == FALSE) {
	fprintf(stderr, "InitDataLink failed\n");
	exit(1);
    }

    recvMsg();	/* take out possible message - fails first time */

    if (getSymbolTable()) {
	for (sTp = sTable; sTp < sTend; sTp++) {
	    printf("%3d %-12s %3d\n", sTp - sTable, sTp->name, (char)sTp->type);
	}
    }

    if (CloseDataLink() == FALSE) {
	fprintf(stderr, "CloseDataLink failed\n");
	ec = 2;
    }
    if (ec) fprintf(stderr, "\nError code: %02X\n", ec);
    return ec;
} /* main */

/********************************************************************
 *
 * 	DDI-Treiber und Datenkanaele initialisieren
 *	(Initialisierung der Treibersoftware in ibspccb.exe)
 *
 *******************************************************************/

short
InitDataLink(void)
{
/*----------------------------------------------------------------- */
/* Kanaele zum Mailbox- und Data-Interface (MXI und DTI) oeffnen */
/* Parameter : */
/*  - Device-Names: 	IBB1N2_M -> B1=Board1, N2=Node2, M=Mailbox */
/*  			IBB1N2_D -> B1=Board1, N2=Node2, D=Data */
/*  - Zugriffsrechte :  DDI_RW (Lesen und Schreiben) */
/*  - Zeiger auf Variable fuer Node-Handle */
/*----------------------------------------------------------------- */

    if ((ec = DDI_DevOpenNode("IBB1N2_M", DDI_RW, &HostMxiNH)) != ERR_OK) {
	fprintf(stderr, "DDI_DevOpenNode IBB1N2_M\n");
	return FALSE;
    }
#ifdef TEST
    printf("HostMxiNH = %d\n", HostMxiNH);
#endif

    if ((ec = DDI_DevOpenNode("IBB1N2_D", DDI_RW, &HostDtiNH)) != ERR_OK) {
	fprintf(stderr, "DDI_DevOpenNode IBB1N2_D\n");
	return FALSE;
    }
#ifdef TEST
    printf("HostDtiNH = %d\n", HostDtiNH);
#endif
    return TRUE;
} /* InitDataLink */

/********************************************************************
 *
 * 	Verbindungen zu den Interfaces (Data & Mailbox) schliessen
 *
 *******************************************************************/

short
CloseDataLink(void)
{
    if (DDI_DevCloseNode(HostMxiNH) != ERR_OK) return FALSE;
    if (DDI_DevCloseNode(HostDtiNH) != ERR_OK) return FALSE;
    return TRUE;
} /* CloseDataLink */

/********************************************************************
 *
 * 	Ausgabe der Prozessdaten
 *
 *******************************************************************/

short
sendData(void)
{
    T_DDI_DTI_ACCESS  dtiAcc;

    dtiAcc.address  = DTA_OFFST_PC_TO_COP;	/* Offset-Adresse */
    dtiAcc.length   = MAXBIN * 2; 		/* Groesse Datenbereich */
    dtiAcc.dataCons = DTI_DATA_WORD; 		/* Datenkonsistenz: Word */
    dtiAcc.data     = (USIGN8 FAR *)HostDataOut;/* Adr. OUT-Data-Puffer */

    if (DDI_DTI_WriteData(HostDtiNH, &dtiAcc) != ERR_OK) return FALSE;
    return TRUE;
} /* sendData */

/********************************************************************
 *
 * 	Eingabe der Prozessdaten
 *
 *******************************************************************/

short
recvData(void)
{
    T_DDI_DTI_ACCESS  dtiAcc;

    dtiAcc.address  = DTA_OFFST_PC_TO_COP;	/* Offset-Adresse */
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
    if ((ec = DDI_MXI_SndMessage(HostMxiNH, &mxiAcc)) != ERR_OK) return FALSE;
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
 *	Get the symbol table from a PPLC application from the
 *	COP processor via the mailbox channel.
 *
 *******************************************************************/

unsigned short
getSymbolTable(void)
{
    char *		mp;
    char *		cp;
    unsigned short	sTlength;
    sT *		sTp;
    int			ln;
    int			i;
    int			n;

#ifdef TEST
    printf("sendMsg GET_SYMBOL_TABLE\n");
#endif
    IB_SetCmdCode(HostOutMail, GET_SYMBOL_TABLE);
    if (sendMsg(2) == FALSE) {
	fprintf(stderr, "sendMsg GET_SYMBOL_TABLE failed\n");
	return 0;
    }
    for (i = 16384; (ln = recvMsg()) == 0 && --i; );
    if (i == 0) { ec = 0x9B; return 0; }	/* timeout error */
#ifdef TEST
    printf("ln = %d %04x %d\n", ln,
	IB_GetMsgCode(HostInMail), IB_GetMsgCode(HostInMail+2));
#endif
    if ((USIGN16)IB_GetMsgCode(HostInMail) != ACKNOWLEDGE) return 0;
    mp = &HostInMail[4];			/* mailbox buffer */
    sTlength = *((unsigned *)mp)++;		/* length of ST */
    sTstrLen = *((long *)mp)++;			/* length of text */
#ifdef TEST
    printf("sTlength = %d sTstrLen = %ld\n", sTlength, sTstrLen);
#endif

    sTable = (sT *) calloc(sTlength, sizeof(sT));
    sTend = sTable + sTlength;			/* allocate symbol table */
    cp = (char *) calloc(sTstrLen, sizeof(char));

    for (sTp = sTable; sTp < sTend; sTp++) {
	sTp->name = cp;				/* point to string copy */
	n = 0;
	do {
	    if (n && sTstrLen-- <= 0) {
		fprintf(stderr, "error in string space. %d strings left\n",
		    sTend-sTp);
		return 0;
	    }
	    if (mp >= &HostInMail[HOSTMAIL]) {
#ifdef TEST
		printf("sendMsg GET_NEXT\n");
#endif
		IB_SetCmdCode(HostOutMail, GET_NEXT);
		if (sendMsg(2) == FALSE) {
		    fprintf(stderr, "sendMsg GET_NEXT failed\n");
		    return 0;
		}
		for (i = 16384; (ln = recvMsg()) == 0 && --i; );
		if (i == 0) { ec = 0x9B; return 0; }	/* timeout error */
#ifdef TEST
		printf("ln = %d %04x %d\n", ln,
		    IB_GetMsgCode(HostInMail), IB_GetMsgCode(HostInMail+2));
#endif
		if ((USIGN16)IB_GetMsgCode(HostInMail) != ACKNOWLEDGE) return 0;
		mp = &HostInMail[4];		/* mailbox buffer */
	    }
	} while ((n || ((sTp->type = *mp++), 1)) &&	/* always true */
	    ++n == 0 || (*cp++ = *mp++) != 0);	/* copy string including null */
    }
#ifdef TEST
    printf("sendMsg GET_END\n");
#endif
    IB_SetCmdCode(HostOutMail, GET_END);
    if (sendMsg(2) == FALSE) {
	fprintf(stderr, "sendMsg GET_END failed\n");
	return 0;
    }
    /* no ACKNOWLEDGE expected */
    return sTlength;
} /* getSymbolTable */

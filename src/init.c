static const char init_c[] =
"@(#)$Id: init.c,v 1.1 1996/07/30 16:18:20 john Exp $";
/*
 *	"init.c"
 *
 *	"init.c	3.28	95/01/20"
 */

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#include	"pplc.h"
#include	"comp.h"
#include	"y_tab.h"

Symbol		*clk;		/* default clock */

/********************************************************************
 *
 *	Initialise Symbol table
 *
 *		reserved words:		type KEYW
 *					u.val = compiler_token
 *					ftype used in compilation
 *
 *		 built-in symbols:	type for built-in
 *					ftype for built-in
 *
 *******************************************************************/

static struct {
    char	*name;
    uchar	type;
    int		u_val;		/* yacc token for type KEYW */
    uchar	ftype;
} builtins[] = {
    "D",	KEYW,	BLTIN1,	D_FF,
    "SH",	KEYW,	BLTIN1,	D_SH,
    "CHANGE",	KEYW,	BLTIN1,	CH_BIT,
    "RISE",	KEYW,	BLTIN1,	RI_BIT,
    "SR",	KEYW,	BLTIN2,	S_FF,
    "on",	KEYW,	BLTINF,	F_CF,
    "C",	KEYW,	CBLTIN,	CLCK,
    "CLOCK",	KEYW,	CBLTIN,	CLCK,
    "T",	KEYW,	TBLTIN,	TIMR,
    "TIMER",	KEYW,	TBLTIN,	TIMR,	/* there is no default timer */
    "F",	KEYW,	BFORCE,	0,
    "FORCE",	KEYW,	BFORCE,	0,
    "L",	KEYW,	BLATCH,	0,
    "LATCH",	KEYW,	BLATCH,	0,
    "DL",	KEYW,	DLATCH,	D_FF,
    "extern",	KEYW,	EXTERN,	0,
    "imm",	KEYW,	IMM,	0,
    "int",	KEYW,	TYPE,	ARITH,
    "bit",	KEYW,	TYPE,	GATE,
    "static",	KEYW,	STATIC,	0,
    "switch",	KEYW,	CKEYW,	0,
    "if",	KEYW,	CKEYW,	0,
    "iClock",	CLK,	0,	CLCKL,	/* must be last non-zero entry */
    0,		0,	0,	0,
};

void
init(void)		/* install constants and built-ins */
{
    int		io;
    char	temp[8];

    for (io = 0; builtins[io].name; io++) {
	clk = install(builtins[io].name, builtins[io].type, builtins[io].ftype);
	clk->u.val = builtins[io].u_val;
    }
    /* 'clk' with name "iClock" is default clock when loop finishes */
}

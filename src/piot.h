static const char piot_h[] =
"@(#)$Id: piot.h,v 1.1 1996/07/30 16:18:20 john Exp $";
/********************************************************************
 *
 *	definitionen der hardwareperipherie zur buskarte
 *
 *	"piot.h	1.00	92/05/12"
 *
 *******************************************************************/

#define BASEADR     0x200	    // basisadresse des hex-schalters

/********************************************************************
 *
 *	PIO 8255A
 *
 *******************************************************************/
// genutzt werden port a und port c im mode2 des 8255A, der port b bleibt
// wegen seiner überflüssigkeit frei

// adressen des 8255A
#define PIO_PA	    BASEADR+0	    // port a
#define PIO_PB	    BASEADR+1	    // port b
#define PIO_PC	    BASEADR+2	    // port c
#define PIO_CMD     BASEADR+3	    // control-byte, write only

// mode des 8255A
#define PIOMODE2    0xC2	    // bit7,6: mode2
				    // bit5...3: n.u.
				    // bit2: port b mode0
				    // bit1: port b input
				    // bit0: port c0..c2 output

// 8255A: belegung des ports c im mode 2
struct port_c {
    unsigned	eop:	1;	// enable output
    unsigned	mode:	1;	// mode des seriellen busses (low = kennung)
    unsigned	rck:	1;	// registerclock des seriellen busses
    unsigned	intra:	1;	// n.u. (-> interrupt inbuffer voll, high aktiv)
    unsigned	stba:	1;	// hardware (-> strobe kanal a, low aktiv)
    unsigned	ibfa:	1;	// input buffer voll, high aktiv
    unsigned	acka:	1;	// hardware (-> send data port a, low aktiv)
    unsigned	obfa:	1;	// output buffer voll, low aktiv (/ack->passiv)
} portc;

/********************************************************************
 *
 *	    8253
 *
 *******************************************************************/

// adressen des 8253
#define CT_1	    BASEADR+4	    // zähler kanal 1
#define CT_2	    BASEADR+5	    // zähler kanal 2
#define CT_3	    BASEADR+6	    // zähler kanal 3
#define CT_CMD	    BASEADR+7	    // control-byte, write only

// control-byte
struct cmdbyte {
    unsigned	select:     2;	    // select kanal
    unsigned	read_load:  2;	    // read/load counter
    unsigned	ct_mode:    3;	    // betriebsart der zähler
    unsigned	bcd:	    1;	    // bcd- oder 16bit- zähler
};

// control word format
// nur 16-bit zähler realisiert (LSB first, MSB last), binary mode
// das eigentliche steuerwort ergibt sich nach addition der werte für
// select und der ct_mode-definition
// select:
#define COUNTER1    0x30
#define COUNTER2    0x70
#define COUNTER3    0xB0
// ct_mode (binär):
#define TCOUNT	    0x00	// mode0: interrupt on terminal count
#define ONESHOT     0x02	// mode1: programmable one shot
#define DIVBYN	    0x04	// mode2: divide by n counter (rate generator)
#define SQWAVE	    0x06	// mode3: quare wave rate generator
#define STSTROBE    0x08	// mode4: software triggered strobe
#define HTSTROBE    0x0a	// mode5: hardware triggered strobe

// zählwerte
//#define VALUE1_LO 0x08	// zähler 1: (1ms -> 208hex bei 8,33MHz/16)
//#define VALUE1_HI 0x02	// interrupt jede millisekunde
#define VALUE1_LO   0xf4	// zähler 1: (1ms -> 1f4hex bei 8,00MHz/16)
#define VALUE1_HI   0x01	// interrupt jede millisekunde
#define VALUE2_LO   0x09	// zähler 2: (520 kHz)
#define VALUE2_HI   0x00	// datenübernahme mit neuntem takt zähler 3
#define VALUE3_LO   0x08	// zähler 3: (520 kHz)
#define VALUE3_HI   0x00	// tor für 8 takte

# @(#)$Id: Makefile,v 1.1 1996/07/30 16:18:20 john Exp $
############################################################
#
#	makefile for parallel plc
#	J.E. Wulff	14-Mar-87
#	pplc.m	3.31	95/03/02
#
#	use Borland C++ (default)
#	or Microsoft C++ (MSC defined)
#	or Microsoft C++  and Windows (WIN defined)
#
############################################################

SRC =	main.c comp.y genr.c init.c symb.c outp.c cexe.c \
	pplc.c pplt.c link.c rsff.c scan.c load.c
SRC2 =	pcop.c ibsd.c time.c
COBJ =	comp.o genr.o init.o symb.o outp.o cexe.o
POBJ =	pplc.o link.o rsff.o scan.o scid.o
TOBJ =	pplt.o link.o rsff.o scan.o scid.o

.SUFFIXES:	.y .yac .lib .v

#### Standalone versions ################################### !!! do last !!!

pplc:	Makefile $(POBJ) $(COBJ) main.o
	cc $(LFLAG) -o $* $(POBJ) $(COBJ) main.o

pplt:	Makefile $(TOBJ) $(COBJ) main.o
	cc $(LFLAG) -o $* $(TOBJ) $(COBJ) main.o

#### Compile objects #######################################
CFLAG = -g

.yac.o:
	mv $*.yac $*.c
	cc -c $(CFLAG) $*.c
	mv $*.c $*.yac

.c.o:
	cc -c $(CFLAG) $*.c

.y.yac:
	chmod +w y.tab.h
	cp -m y.tab.h y.tab.t
	yacc -d -v $*.y
	awk -f c:/usr/lib/yt.awk y.tab.h > y.token
	sed -f c:/usr/lib/yg.sed y.output > y.tab.s
	awk -f c:/usr/lib/yg.awk y.tab.s > y.tab.a
	sort +0.2n y.tab.a > y.tab.s
	uniq y.tab.s > y.gramm
	cat y.token y.gramm y.tab.c > $*.yac
#	sh cmp_cp y.tab.h y.tab.t
	chmod -w y.tab.h
	rm y.tab.c y.tab.a y.tab.s y.tab.t

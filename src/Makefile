# @(#)$Id: Makefile,v 1.6 1999/08/06 21:13:31 jw Exp $
############################################################
#
#	makefile for parallel plc
#	J.E. Wulff	14-Mar-87
#	pplc.m	3.31	95/03/02
#
#	use GNU make	28-Jul-96
#
############################################################

YSRCS =	comp.y
SRC1 =	comp.c main.c genr.c init.c symb.c outp.c cexe.c
SRC2 =	pplc.c pplt.c link.c rsff.c scan.c
#SRC3 =	pcop.c ibsd.c time.c load.c
CSRCS =	$(SRC1) $(SRC2) $(SRC3)

COBJ =	comp.o genr.o init.o symb.o outp.o cexe.o
POBJ =	link.o rsff.o scan.o

CFLAGS = -g -DYYDEBUG $(OPT)
YFLAGS = -d -v
LDFLAGS =

#### Standalone versions ################################### !!! do last !!!

pplc:	Makefile main.d main.o scid.o pplc.d pplc.o $(COBJ:.o=.d) $(COBJ) $(POBJ:.o=.d) $(POBJ)
	cc $(LDFLAG) -o pplc main.o scid.o pplc.o $(COBJ) $(POBJ)

pplt:	Makefile main.d main.o scid.o pplt.d pplt.o $(COBJ:.o=.d) $(COBJ) $(POBJ:.o=.d) $(POBJ)
	cc $(LDFLAG) -o pplt main.o scid.o pplt.o $(COBJ) $(POBJ)

load.a:	Makefile load.d load.o scid.o pplc.d pplc.o $(POBJ:.o=.d) $(POBJ)
	ar -rv load.a load.o scid.o pplc.o $(POBJ)

#### Compile objects #######################################

%.c: %.y
	mv -f y.tab.h y.tab.t
	rm -f y.tab.c $@
	$(YACC.y) $< 
	cp y.tab.c $@
	$(SHELL) -ec 'if cmp -s y.tab.h y.tab.t; then mv y.tab.t y.tab.h; fi'
	rm -f y.tab.t
	chmod -w y.tab.h y.tab.c $@

%.d: %.c
	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $< | sed '\''s/$*.o[ :]*/$@ &/g'\'' > $@'

scid.c:	pplc.v Makefile pawk.a
	rm -f scid.c; ident pplc.v Makefile | grep '$Id' | awk -f pawk.a > scid.c; chmod -w scid.c

comp.c:	comp.y

include $(CSRCS:.c=.d)

# @(#)$Id: Makefile,v 1.14 2001/02/21 16:20:29 jw Exp $
############################################################
#
#	makefile for parallel plc
#	J.E. Wulff	14-Mar-87
#	icc.m	3.31	95/03/02
#
#	use GNU make	28-Jul-96
#
#	to make ict.a, remove all objects and make with export OPT=-DLOAD
#
############################################################

YSRCS =	comp.y
SRC1 =	comp.c main.c genr.c init.c symb.c outp.c cexe.c
SRC2 =	icc.c ict.c icl.c link.c rsff.c scan.c
SRC3 =	load.c
CSRCS =	$(SRC1) $(SRC2) $(SRC3)

COBJ =	comp.o genr.o init.o symb.o outp.o cexe.o
POBJ =	link.o rsff.o scan.o

CFLAGS = -g -DYYDEBUG $(OPT)
YFLAGS = -d -v
LDFLAGS =

#### Standalone versions ################################### !!! do last !!!

icc:	Makefile main.d main.o scid.o icc.d icc.o $(COBJ:.o=.d) $(COBJ) $(POBJ:.o=.d) $(POBJ)
	cc $(LDFLAG) -o icc main.o scid.o icc.o $(COBJ) $(POBJ)

ict:	Makefile main.d main.o scid.o ict.d ict.o tcpc.d tcpc.o $(COBJ:.o=.d) $(COBJ) $(POBJ:.o=.d) $(POBJ)
	cc $(LDFLAG) -o ict main.o scid.o ict.o tcpc.o $(COBJ) $(POBJ)

icl:	Makefile main.d main.o scid.o icl.d icl.o $(COBJ:.o=.d) $(COBJ) $(POBJ:.o=.d) $(POBJ)
	cc $(LDFLAG) -o icl main.o scid.o icl.o $(COBJ) $(POBJ)

ict.a:	Makefile load.d load.o scid.o ict.d ict.o tcpc.d tcpc.o $(POBJ:.o=.d) $(POBJ)
	ar -rv ict.a load.o scid.o ict.o tcpc.o $(POBJ)

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

scid.c:	icc.v Makefile pawk.a
	rm -f scid.c; ident icc.v Makefile | grep '$Id' | awk -f pawk.a > scid.c; chmod -w scid.c

cexe.c:	cexe.h
	icc -c /dev/null	# generate cexe.c with no functions
	cp -f cexe.c cexe.c_empty

comp.c:	comp.y

include $(CSRCS:.c=.d)

$Revision icc_3.5 $
@(#)$Id: icc.v 3.5 $
ARTISTIC        1.1     2001/03/02 16:43:55 jw Exp
COPYING         1.1     2001/03/02 16:43:55 jw Exp
Makefile.in     1.76    024be37 2020-04-06 14:57:33 +1000 John E Wulff
README          1.33    05a8d46 2020-04-14 12:17:41 +1000 John E Wulff
README.RPi      1.4     7516017 2017-02-14 20:38:57 +1100 John Wulff
bar.sav         1.1     2013/02/03 07:14:30 jw Exp
bar.vcd         1.1     2013/02/03 07:14:30 jw Exp
barx.vcd        1.1     2013/02/01 23:41:01 jw Exp
bcm2835.c       1.22    2015/03/29 08:14:44 jw Exp
bcm2835.h       1.20    024be37 2020-04-06 14:57:33 +1000 John E Wulff
cexe.h          1.32    2e80732 2018-10-18 12:38:48 +1100 John E Wulff
cexe_t.c        1.2     7516017 2017-02-14 20:38:57 +1100 John Wulff
comp.h          1.79    ba348c5 2018-10-03 14:00:19 +1000 John E Wulff
comp.y          1.133   024be37 2020-04-06 14:57:33 +1000 John E Wulff
configure       1.9     7516017 2017-02-14 20:38:57 +1100 John Wulff
configure.in    1.4     2005/02/01 16:37:44 jw Exp
cons.y          1.5     8163733 2018-02-10 19:03:14 +1100 John E Wulff
const.c         1.1     5055c6f 2017-02-19 17:19:37 +1100 John Wulff
genr.c          1.95    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
gram.y          1.43    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCbox           1.64    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCgpioPUD.c     1.3     024be37 2020-04-06 14:57:33 +1000 John E Wulff
iClift          1.20    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iClive          1.84    05a8d46 2020-04-14 12:17:41 +1000 John E Wulff
iCmake          1.54    9a658e9 2019-10-13 23:00:47 +1100 John E Wulff
iCman           1.29    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCmsg.pm        1.7     66463bf 2018-06-15 17:28:02 +1000 John E Wulff
iCpiFace.c      1.12    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCpiPWM.c       1.5     d5085f2 2017-07-22 21:11:58 +1000 John Wulff
iCrfid          1.3     024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCserver        1.61    c574ad2 2020-02-10 18:02:44 +1100 John E Wulff
iCstop          1.13    263b257 2016-04-27 16:28:42 +1000 John E Wulff
iCtherm         1.7     024be37 2020-04-06 14:57:33 +1000 John E Wulff
icbegin.c       1.9     024be37 2020-04-06 14:57:33 +1000 John E Wulff
icc.c           1.86    c574ad2 2020-02-10 18:02:44 +1100 John E Wulff
icc.h           1.87    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
icend.c         1.5     d5085f2 2017-07-22 21:11:58 +1000 John Wulff
icg.pl          1.5     2015/10/16 12:33:47 jw Exp
icr.c           1.47    333193e 2018-05-24 16:47:56 +1000 John E Wulff
ict.c           1.80    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
immac           1.31    024be37 2020-04-06 14:57:33 +1000 John E Wulff
immax           1.4     2012/12/06 22:28:03 jw Exp
init.c          1.48    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
init_t.ic       1.5     2754268 2017-09-27 19:16:57 +1000 John E Wulff
init_t.pl       1.5     2754268 2017-09-27 19:16:57 +1000 John E Wulff
jwLift.ic       1.2     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
jwLift.is       1.1     df9ff3a 2017-12-31 17:43:01 +1100 John E Wulff
kbLift.ic       1.14    9a658e9 2019-10-13 23:00:47 +1100 John E Wulff
kbLift.is       1.5     51a0f98 2016-09-06 11:56:28 +1000 John E Wulff
lexc.l          1.40    024be37 2020-04-06 14:57:33 +1000 John E Wulff
lifts.ica       1.6     938d078 2017-03-14 19:00:12 +1100 John Wulff
link.c          1.36    333193e 2018-05-24 16:47:56 +1000 John E Wulff
lmain.c         1.20    5055c6f 2017-02-19 17:19:37 +1100 John Wulff
load.c          1.76    05a8d46 2020-04-14 12:17:41 +1000 John E Wulff
m               1.22    b1b62ef 2018-04-20 14:36:49 +1000 John E Wulff
makeAll         1.44    b1b62ef 2018-04-20 14:36:49 +1000 John E Wulff
manifest        1.31    9a658e9 2019-10-13 23:00:47 +1100 John E Wulff
mcp23s17.c      1.4     2015/12/23 23:52:15 jw Exp
mcp23s17.h      1.4     2015/12/18 07:02:24 jw Exp
misc.c          1.19    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
ml              1.20    37d7c8e 2017-03-31 23:42:38 +1100 John Wulff
outp.c          1.113   024be37 2020-04-06 14:57:33 +1000 John E Wulff
p_channel.c     1.3     d5085f2 2017-07-22 21:11:58 +1000 John Wulff
pawk.pl         1.9     8163733 2018-02-10 19:03:14 +1100 John E Wulff
perlinstall     1.11    2009/08/21 06:08:40 jw Exp
pfcad.ic        1.4     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
pifacecad.c     1.3     2015/03/18 01:30:36 jw Exp
pifacecad.h     1.4     2015/10/16 12:33:47 jw Exp
pod2man         1.10    2004/04/03 01:11:21 jw Exp
pplstfix        1.28    ba348c5 2018-10-03 14:00:19 +1000 John E Wulff
rfid.ic         1.6     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
rpi_gpio.c      1.3     2015/04/08 04:33:54 jw Exp
rpi_gpio.h      1.3     2015/03/05 06:10:16 jw Exp
rpi_rev.c       1.5     7a436bd 2018-11-10 18:16:38 +1100 John E Wulff
rpi_rev.h       1.4     7a436bd 2018-11-10 18:16:38 +1100 John E Wulff
rsff.c          1.67    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
scan.c          1.46    ba348c5 2018-10-03 14:00:19 +1000 John E Wulff
simLift.ic      1.6     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
sortm28_1.sav   1.1     2013/02/27 21:30:43 jw Exp
sortm28_1.vcd   1.1     2013/02/28 00:36:03 jw Exp
symb.c          1.25    d5085f2 2017-07-22 21:11:58 +1000 John Wulff
tcpc.c          1.30    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
tcpc.h          1.23    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
therm.ic        1.4     df9ff3a 2017-12-31 17:43:01 +1100 John E Wulff
vcd2sav         1.2     024be37 2020-04-06 14:57:33 +1000 John E Wulff

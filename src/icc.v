$Revision icc_3.8 $
@(#)$Id: icc.v 3.8 $
ARTISTIC        1.1     2001/03/02 16:43:55 jw Exp
COPYING         1.1     2001/03/02 16:43:55 jw Exp
Makefile.in     1.86    bc49d7e 2023-11-13 21:22:15 +1100 John E Wulff
README          1.34    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
README.RPi      1.4     7516017 2017-02-14 20:38:57 +1100 John Wulff
bar.sav         1.1     2013/02/03 07:14:30 jw Exp
bar.vcd         1.1     2013/02/03 07:14:30 jw Exp
barx.vcd        1.1     2013/02/01 23:41:01 jw Exp
bcm2835.c       1.22    2015/03/29 08:14:44 jw Exp
bcm2835.h       1.20    024be37 2020-04-06 14:57:33 +1000 John E Wulff
cexe.h          1.32    2e80732 2018-10-18 12:38:48 +1100 John E Wulff
cexe_t.c        1.2     7516017 2017-02-14 20:38:57 +1100 John Wulff
comp.h          1.80    e8c295b 2023-11-07 17:36:50 +1100 John E Wulff
comp.y          1.137   c1146c5 2023-11-10 17:59:37 +1100 John E Wulff
configure       1.10    f76619a 2021-11-18 00:16:39 +1100 John E Wulff
configure.in    1.4     2005/02/01 16:37:44 jw Exp
cons.y          1.8     8ec2301 2023-08-05 21:36:40 +1000 John E Wulff
const.c         1.1     5055c6f 2017-02-19 17:19:37 +1100 John Wulff
genr.c          1.98    e8c295b 2023-11-07 17:36:50 +1100 John E Wulff
goserver.go     1.10    c1146c5 2023-11-10 17:59:37 +1100 John E Wulff
gram.y          1.43    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCbox           1.68    87a9bd2 2023-11-14 21:58:00 +1100 John E Wulff
iCgpioPUD.c     1.3     024be37 2020-04-06 14:57:33 +1000 John E Wulff
iClift          1.22    46b9091 2021-12-03 13:38:21 +1100 John E Wulff
iClive          1.98    bc49d7e 2023-11-13 21:22:15 +1100 John E Wulff
iCmake          1.56    5bc4bd0 2023-02-11 21:58:31 +1100 John E Wulff
iCman           1.29    024be37 2020-04-06 14:57:33 +1000 John E Wulff
iCmqtt          1.1     d2e1160 2023-08-29 12:17:33 +0200 John E Wulff
iCmsg.pm        1.7     66463bf 2018-06-15 17:28:02 +1000 John E Wulff
iCmsgM.pm       1.8     d2e1160 2023-08-29 12:17:33 +0200 John E Wulff
iCpiFace.c      1.18    a2bfd76 2023-08-01 19:16:44 +1000 John E Wulff
iCpiI2C.c       1.9     d2e1160 2023-08-29 12:17:33 +0200 John E Wulff
iCpiPWM.c       1.7     f76619a 2021-11-18 00:16:39 +1100 John E Wulff
iCrfid          1.4     f76619a 2021-11-18 00:16:39 +1100 John E Wulff
iCserver        1.71    c1146c5 2023-11-10 17:59:37 +1100 John E Wulff
iCstop          1.13    263b257 2016-04-27 16:28:42 +1000 John E Wulff
iCtherm         1.9     f76619a 2021-11-18 00:16:39 +1100 John E Wulff
icbegin.c       1.9     024be37 2020-04-06 14:57:33 +1000 John E Wulff
icc.c           1.88    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
icc.h           1.87    d45d510 2019-11-21 16:48:51 +1100 John E Wulff
icend.c         1.5     d5085f2 2017-07-22 21:11:58 +1000 John Wulff
icg.pl          1.5     2015/10/16 12:33:47 jw Exp
icr.c           1.48    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
ict.c           1.88    bc49d7e 2023-11-13 21:22:15 +1100 John E Wulff
immac           1.35    7f51315 2023-11-16 11:42:00 +1100 John E Wulff
immag.go        1.4     7f51315 2023-11-16 11:42:00 +1100 John E Wulff
immax           1.4     2012/12/06 22:28:03 jw Exp
init.c          1.49    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
init_t.ic       1.5     2754268 2017-09-27 19:16:57 +1000 John E Wulff
init_t.pl       1.6     3372184 2023-11-12 15:43:54 +1100 John E Wulff
jwLift.ic       1.4     46b9091 2021-12-03 13:38:21 +1100 John E Wulff
jwLift.is       1.5     46b9091 2021-12-03 13:38:21 +1100 John E Wulff
kbLift.ic       1.15    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
kbLift.is       1.5     51a0f98 2016-09-06 11:56:28 +1000 John E Wulff
lexc.l          1.40    024be37 2020-04-06 14:57:33 +1000 John E Wulff
lifts.ica       1.7     46b9091 2021-12-03 13:38:21 +1100 John E Wulff
link.c          1.36    333193e 2018-05-24 16:47:56 +1000 John E Wulff
lmain.c         1.21    8ec2301 2023-08-05 21:36:40 +1000 John E Wulff
load.c          1.81    c1146c5 2023-11-10 17:59:37 +1100 John E Wulff
m               1.28    3372184 2023-11-12 15:43:54 +1100 John E Wulff
makeAll         1.50    8ec2301 2023-08-05 21:36:40 +1000 John E Wulff
mcp23017.c      1.4     a2bfd76 2023-08-01 19:16:44 +1000 John E Wulff
mcp23017.h      1.2     3ea87c7 2023-07-13 22:24:36 +1000 John E Wulff
mcp23s17.c      1.4     2015/12/23 23:52:15 jw Exp
mcp23s17.h      1.4     2015/12/18 07:02:24 jw Exp
misc.c          1.23    bc49d7e 2023-11-13 21:22:15 +1100 John E Wulff
ml              1.21    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
outp.c          1.115   e8c295b 2023-11-07 17:36:50 +1100 John E Wulff
p_channel.c     1.3     d5085f2 2017-07-22 21:11:58 +1000 John Wulff
pawk.pl         1.10    0dade93 2021-03-18 13:29:32 +1100 John E Wulff
perlinstall     1.12    5bc4bd0 2023-02-11 21:58:31 +1100 John E Wulff
pfcad.ic        1.4     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
pifacecad.c     1.5     8ec2301 2023-08-05 21:36:40 +1000 John E Wulff
pifacecad.h     1.4     2015/10/16 12:33:47 jw Exp
pod2man         1.10    2004/04/03 01:11:21 jw Exp
pplstfix        1.28    ba348c5 2018-10-03 14:00:19 +1000 John E Wulff
rfid.ic         1.6     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
rpi_gpio.c      1.3     2015/04/08 04:33:54 jw Exp
rpi_gpio.h      1.3     2015/03/05 06:10:16 jw Exp
rpi_rev.c       1.5     7a436bd 2018-11-10 18:16:38 +1100 John E Wulff
rpi_rev.h       1.4     7a436bd 2018-11-10 18:16:38 +1100 John E Wulff
rsff.c          1.69    55175aa 2021-07-20 13:33:37 +1000 John E Wulff
scan.c          1.47    ae5729a 2021-04-21 17:18:01 +1000 John E Wulff
simLift.ic      1.6     5086f34 2020-01-27 16:12:34 +1100 John E Wulff
sortm28.ica     1.2     6ff51f4 2022-09-18 22:04:36 +1000 John E Wulff
sortm28_1.sav   1.1     2013/02/27 21:30:43 jw Exp
sortm28_1.vcd   1.1     2013/02/28 00:36:03 jw Exp
symb.c          1.25    d5085f2 2017-07-22 21:11:58 +1000 John Wulff
tcpc.c          1.35    3372184 2023-11-12 15:43:54 +1100 John E Wulff
tcpc.h          1.25    c6bba4d 2021-11-20 18:41:25 +1100 John E Wulff
therm.ic        1.4     df9ff3a 2017-12-31 17:43:01 +1100 John E Wulff
vcd2sav         1.2     024be37 2020-04-06 14:57:33 +1000 John E Wulff

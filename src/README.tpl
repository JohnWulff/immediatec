#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)
########################################################################
#   Self extracting README which adjusts to the current version of
#   icc.v, which contains a parts list of the whole of the iC project.
#   icc.v is maintained under source control and the version number
#   of icc.v is used as the version number of the iC release.
#   Call: 	README.tpl icc.v > ../README
#   or    	cat icc.v | README.tpl > ../README
#   No command line checking is done, since only called from Makefile.
#   NOTE: the 'ident' filter is no longer used, since it is part of
#         the 'RCS' package and may not be installed by some users.
#         The power of Perl regular expressions is used instead.
#   $Id: README.tpl,v 1.23 2013/02/28 06:31:52 jw Exp $
########################################################################

use strict;

my ($ID, $NAME, $REV, $DATE, $YEAR, $MONTH, $DAY);
my $id = "Id";

while (<>) {
    if (/(\$$id:[^\$]*\$)/o) {
	my $ident = $1;
	($ID, $NAME, $REV, $DATE) = split " ", $ident if length > 20;	# ident string
	last;
    }
}
($YEAR, $MONTH, $DAY) = split "/", $DATE;

########################################################################
print <<EOF;

			 immediate C, iC rev $REV

		    Copyright (C) 1985-$YEAR, John E. Wulff
			    All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of either:

	a)  the GNU General Public License as published by the Free
	    Software Foundation; either version 2, or (at your option)
	    any later version,
	 OR

	b)  the "Artistic License" which comes with this Kit.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See either the
    GNU General Public License or the Artistic License for more details.

    You should have received a copy of the Artistic License with this
    Kit, in the file named "Artistic". If not, I will be glad to provide one.

    You should also have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

    For those of you that choose to use the GNU General Public License,
    my interpretation of the GNU General Public License is that no iC
    application falls under the terms of the GPL unless you explicitly
    put said application source under the terms of the GPL yourself.
    Furthermore, any object code linked with iC does not automatically
    fall under the terms of the GPL, provided such object code only adds
    definitions of subroutines and variables, and does not otherwise impair
    the resulting interpreter from executing any standard iC source.
    I consider linking in C subroutines in this manner to be the moral
    equivalent of defining subroutines in the iC language itself.  You may
    sell such an object file as proprietary provided that you provide or
    offer to provide the immediate C source, as specified by the GNU General
    Public License.  (This is merely an alternate way of specifying input
    to the program.)  You may also sell a binary produced by compiling an
    iC source that belongs to you with the iC compiler and linking it with
    the iC runtime library, provided that you provide or offer to provide
    the immediate C source as specified by the GPL. (The fact that the iC
    runtime library and your code are in the same binary file is, in this
    case, a form of mere aggregation.) This is my interpretation of the GPL.
    If you still have concerns or difficulties understanding my intent,
    feel free to contact me (see my email at the end of this README)

    Of course, the Artistic License spells all this out for your protection,
    so you may prefer to use that.

    Acknowledgements to Larry Wall, whose README I used as a template.
				    and for Perl - which is just GREAT.

    Acknowledgements to Nick Ing-Simmons for Perl/Tk - which is SMOOTH.

    Acknowledgements to the developers of GTKWave - which is POWERFUL.

    Acknowledgements to Linus Torvalds and the Open-Software community
				    for Linux(R) - which is SOMETHING ELSE.

--------------------------------------------------------------------------

    Notes for the installation of iC rev $REV

    1)  Pre-requisites. You need the following on your system:

	    C compiler			     # tested with gcc, MSC
	    Perl, Perl/Tk and Time::HiRes    # to build iC applications

    1)  Unpack the iC-archive in a suitable working directory with:

	    tar -xvzf icc_$REV.tgz
	    cd icc_$REV/src

    2)  Excute the following:

	    configure     OR  ./configure    # if super user (depricated)
	To make a Debug version do
	    makeAll -gcl  OR  makeAll -qgcl  # to supress intermediate output
	OR to make a Release version do
	    make          OR  make quiet     # to supress intermediate output

	this should build the files
	    immcc			     # the iC to C compiler
	    libict.a			     # the run-time library
	without any errors

    3)  To compile and compare the test iC files in Test0 execute:

	    make test

    4)  To use the Perl support programs, it is mandatory that you install the
	Perl packages Tk804.029 or later and Time::HiRes unless they are already
	installed on your system.  Both are included with this distribution.
	This can be checked by executing the following at this point:

	    iClive -h

	Skip to point 8) if you get a help output and no error message.
	The last line tells you which version of Perl/Tk you are using.

    5)  Unpack build and install the Time::HiRes archive in a suitable
	working directory with:

	    tar -xvzf Time-HiRes-01.20.tar.gz
	    cd Time-HiRes-01.20; perl Makefile.PL; make; make test
	    su		     ### Password  ###
	    make install
	    exit	     ### IMPORTANT ###
	    cd ..; rm -rf Time-HiRes-01.20   # unless you want to keep it

    6)  Perl/Tk is usually contained in Linux distributions and will
	be installed automatically when the package is selected.
	If not, unpack, build and install Tk-804.029.tar.gz (or later).
	Follow the instructions in the README and INSTALL files.
	For Cygwin under WinXP a special binary distribution of Tk800.023
	is included, which works fine.

    7)  Return to the immediate C installtion

	    cd icc_$REV/src		     # or the correct iC src directory

    8)  To install the iC-compiler, library and scripts execute the
	following as super user:

	    su		     ### Password  ###
	    make install
	    exit	     ### IMPORTANT ###

	this copies the essential executables to /usr/local/bin
	it also copies the include file icg.h to /usr/local/include
	libict.a to /usr/local/lib and Msg.pm to /usr/lib/perl5/site...

	    (make uninstall as su will remove all these files)

    9)  To build and run the very simple iC application "hello.ic" do

	    iClive hello.ic		     # starts the IDE with hell0.ic
	    press Build / Build executable   # displays 'hello' successfully built
	    press Run			     # opens an iCbox with 1 button IX0.0
	    press button IX0.0 in iCbox	     # button turns HI (input is green)
		# 'Hello! world' is output in the window iClive was started from
	    press Live
		# The word IX0.0 (the only immediate variable in hello.ic)
		# is coloured yellow/red, because IX0.0 is HI.
		# When IX0.0 is pressed again to LO, the colour in the live
		# display changes to green/black, indicating LO.
	    press File / Quit		     # 'hello' and iCbox are terminated

    10) A slightly bigger application is "simple.ic". Build and run it with iClive.
	An iCbox with 16 inputs and 8 outputs is started automatically.
	Explore the logic of the statements by changing inputs and following
	the outputs in iCbox and the live display in iClive.

    11) The application "bar.ic" uses flip flops to produce a bar of running lights.
	The application also explores the use of programmable time delays, giving
	some idea of the scope of the iC language.

	Running 'iClive bar.ic' as a separate process, while 'bar' is running,
	will display the source listing (in the edit window), connects to iCServer
	as an auxiliary client to receive updates of all variables from the running
	iC program (bar). These updates will change the colours of all words,
	which are immediate variables.  (green/black = 0, yellow/red = 1)
	This "live display" shows the current state of logical relationships in
	visible statements of the iC program. Arithmetic variables are displayed
	in a balloon, when the cursor rests on a variable.
	(Arithmetic variables have a blue background).

	In 'Live' mode, when a "live display" is shown, the text is read only.
	When the 'Edit' button is pressed 'iClive' is a full featured editor.
	The edit facilities of this program are described in the iClive man page
	under the Heading 'KEYBOARD BINDINGS' (press Help button in 'iClive').

	'iClive' can use the Tk::TextUndo package, an extension of Tk::Text. This
	allows undoing changes with the Ctrl-u key. (Control-u is <<Undo>>)
	This is achieved by starting iClive with the -u option. Use this option
	only for editing. In 'Live' mode the display is very jerky with -u active.

    12) Applications can of course be run without iClive. They do need iCserver
	though, which is a hub server for the TCP/IP packets exchanged between
	iC applications, I/O apllications - currently only iCbox, iClift and
	optionally iClive.

	    iCserver &			     # server runs on the background
	    iCbox IX0 &			     # start IX0 manually
	    hello			     # start application

	    iCstop iCserver		     # kill iCserver, hello and iCbox

	A better way is to start iCserver with the -a (auto-vivify) option,
	which will start simulated I/O iCbox, every time an iC application
	is started. Otherwise these must be started manually, which can be
	tedious for larger applications.

	    iCserver -a &		     # auto-vivify iCbox for application
	    simple			     # iCbox with 3 sets of I/O starts

	If iClive is started first, it does all this automatically. It then kills
	iCserver automatically when it quits. When iCserver quits it kills all
	registered applications and I/O's.

    13) Another powerful debugging tool is the GTKWave Wave Analyzer.

	GTKWave (an open-source program) is an analysis tool originally
	intended to perform debugging on Verilog or VHDL simulation
	models. With the exception of interactive VCD viewing, it is not
	intended to run interactively with simulation, but instead relies on
	a post-mortem approach through the use of dumpfiles. Various dumpfile
	formats are supported: VCD: Value Change Dump. This is an industry
	standard file format generated by most Verilog simulators and is
	specified in IEEE-1364.  (Extract from the GTKWave 3.3 Wave Analyzer
	User's Guide) Use in the iC system is hereby gratefully acknowledged.

	gtkwave-3.1.10-4.33.i586.rpm is distributed with immediate C.
	To install gtkwave on OpenSUSE Linux do:

	    sudo rpm -i gtkwave-3.1.10-4.33.i586.rpm

	For other Linux distributions download and follow the instructions
	in the internet to install GTKWave for those sytems.

	Since version 1.131 of the iC run-time a VCD file and an associated
	SAV file is generated by an iC application when run with the -v
	option. This allows the interactive or post-mortem viewing of both
	logic and analog traces against a virtual time scale. This time scale
	was chosen so that each sequential change in any signal is 1 virtual
	microsecond. iClock cycles take up 2 virtual microseconds. Each new
	external input (Ixn.m, IBn, IWn ILn or TX0.m) occurs on the next 10
	virtual microsecond boundary. To test this do:

	    iCserver -a &		     # auto-vivify iCbox for application
	    simple -v simple.vcd	     # writes simple.vcd and simple.sav
	    # press IX0.0 to IX0.7 in iCbox one after the other to turn them on
	    # then press IX0.0 to IX0.7 one after the other to turn them off again
	    iCstop iCserver		     # kill iCserver, simple and iCbox
	    gtkwave simple.vcd simple.sav    # display all non-extended variables

	A more detailed example uses the running of 'bar -v bar.vcd' shown in the
	iC manual iC.pdf and barx.sav, which is in the distribution and which shows
	extra extended gates and markers used in the description in the manual.
	barx.sav was generated by first running gtkwave bar.vcd bar.sav - inserting
	extended variables from modules, setting markers and then saving as barx.sav.

	    gtkwave bar.vcd barx.sav	    # display some extended variables

    14) A fairly varied example is sortm28.ica, which is described fully under
	7.3 'Parcel Sorter' in the iC.odt handbook. It produces good GTKwave traces.

    15) I have included a script called 'iCstop' from my private toolkit.
	It can be used effectively to kill iCserver when it is executing
	in the background, which is appropriate for a server.

	    iCserver &
	    .....
	    iCstop iCserver		     # local copy of 'iCstop'

	I have tried to use 'kill' with named processes as described in
	the 'kill' manpage, but it does not seem to work, even called as
	'command kill iCserver'.
	You will have to install 'iCstop' manually in a PATH directory to
	use it anywhere in your system. (see 'iCstop -h' for help)

    16) To make executable applications from iC sources, use the script iCmake.
	iCmake is a shell script to compile iC sources into C sources using the
	'immcc' compiler.  These in turn are compiled and linked into an
	executable iC applications (currently using gcc - this can be changed).
	Various options allow partial compilation and generation of listings.

	    iCmake -h  OR  iCman iCmake	     # gives a lot of help

    17) The LibreOffice 3.3.1 document doc/iC.odt (or doc/iC.pdf, doc/iC.html)
	is the handbook for the iC Programming Language. It opens the way to
	use "immediate C" fully.

    18) There is a generous help output for every tool in the 'iC Project'
	initiated with the -h option. Each generated iC application also
	has a help output:

	    hello -h			     # list available options

	These options allow connecting to iCserver on another computer in a
	LAN - or with a differnt port number. Very detailed debugging output,
	showing the change of state of every event in the system is available
	for the Debug version of the iC system. (Supressed for Release version)

    19) There are 'man' pages for all the tools used in the 'iC Project'.
	These can be viewed with the normal 'man' command under Linux or with
	'iCman'.  The man page viewer 'iCman' has some nifty web-browser
	features to view and search man pages - try it with 'iCman iCman'.


    Lots of success

    John E. Wulff   $YEAR.$MONTH.$DAY     <immediateC\@gmail.com>


    I have now switched to openSUSE 11.4, which brings along Tk804.29,
    which provides fast live updates in iClive again. They seem to be as
    fast as with Tk800.24. This was only judged by observation - at least
    the performance is now subjectively good and I suggest you get Tk804.29.

    A Test with Knoppix and Tk804.25, which is still available for Debian
    also provided good performance with fast live updates in iClive.

    No recent tests with MAC-OSX or Windows have been done - this will be
    the next thing to do.
EOF
########################################################################

#!/usr/bin/perl
########################################################################
#   Self extracting README which adjusts to the current version of
#   icc.v, which contains a parts list of the whole of the iC project.
#   icc.v is maintained under source control and the version number
#   of icc.v is used as the version number of the iC release.
#   Call: 	ident icc.v | README.tpl > ../README
#   $Id: README.tpl,v 1.12 2003/10/12 20:57:47 jw Exp $
########################################################################

while (<>) {
    ($ID, $NAME, $REV, $DATE) = split if length > 20;	# ident string
}
($YEAR, $MONTH, $DAY) = split "/", $DATE;

########################################################################
print <<EOF;

                         immediate C, iC rev $REV

                    Copyright (C) 1985-$YEAR, John E. Wulff
                            All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of either:

        a) the GNU General Public License as published by the Free
        Software Foundation; either version 2, or (at your option) any
        later version, or

        b) the "Artistic License" which comes with this Kit.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See either the
    GNU General Public License or the Artistic License for more details.

    You should have received a copy of the Artistic License with this
    Kit, in the file named "Artistic". If not, I'll be glad to provide one.

    You should also have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

    For those of you that choose to use the GNU General Public License,
    my interpretation of the GNU General Public License is that no iC
    source falls under the terms of the GPL unless you explicitly put
    said source under the terms of the GPL yourself.  Furthermore, any
    object code linked with iC does not automatically fall under the
    terms of the GPL, provided such object code only adds definitions
    of subroutines and variables, and does not otherwise impair the
    resulting interpreter from executing any standard iC source.  I
    consider linking in C subroutines in this manner to be the moral
    equivalent of defining subroutines in the iC language itself.  You
    may sell such an object file as proprietary provided that you provide
    or offer to provide the iC source, as specified by the GNU General
    Public License.  (This is merely an alternate way of specifying input
    to the program.)  You may also sell a binary produced by compiling an
    iC source that belongs to you with the iC compiler and linking it with
    the iC runtime library, provided that you provide or offer to provide
    the iC source as specified by the GPL. (The fact that the iC runtime
    library and your code are in the same binary file is, in this case,
    a form of mere aggregation.) This is my interpretation of the GPL.
    If you still have concerns or difficulties understanding my intent,
    feel free to contact me at <john\@je-wulff.de>.

    Of course, the Artistic License spells all this out for your protection,
    so you may prefer to use that.

    Acknowledgements to Larry Wall, whose README I used as a template.
                                    and for Perl - which is just great.

    Acknowledgements to Nick Ing-Simmons for Perl-Tk - which is smooth.

    Acknowledgements to Linus Torvalds and the Open-Software community
                                    for Linux(R) - which is something else. 

--------------------------------------------------------------------------

        Notes for the installation of icc rev $REV

        1)  Unpack the archive in a suitable working directory with:

                tar -xvzf icc_$REV.tgz
                cd icc_$REV/src

        2)  Excute the following:

                configure  OR    ./configure # if super user (depricated)
                make       OR    make quiet  # to supress intermediate output

            this should build the files
                icc
                ict
                libict.a
            without any errors

        3)  To test compile the iC files in Test execute:

		make test

	4)  It is recomended that you install the Perl package Time::HiRes 
	    included with this distribution unless it is already installed.
	    This can be checked by executing the following at this point:

	        iCserver -h

	    Skip to point 8) if you get a help output and no error message.

	    You can work without Time::HiRes - the scripts iCbox, iCserver will
	    not support the -m microsecond timing option and 'iClive' will not
	    display short signals by flashing colours. If you subsequently
	    install Time::HiRes, you have to execute make uninstall followed
	    by another make install as su in the icc/src directory, since the
	    Perl scripts are adjusted by make install to work with/without
	    Time::HiRes depending on its presence.

        6)  Unpack build and install the Time::HiRes archive in a suitable
	    working directory with:

                tar -xvzf Time-HiRes-01.20.tar.gz
                cd Time-HiRes-01.20
		perl Makefile.PL
		make
		make test
		su               ### Password  ###
		make install
                exit             ### IMPORTANT ###
		cd ..
		rm -rf Time-HiRes-01.20	# unless you want to keep it

        7)  Return to the immediate C installtion

		cd icc_$REV/src	# or the correct icc src directory

        8)  To install the compiler, library and scripts execute the following
	    as super user:

                su               ### Password  ###
                make install
                exit             ### IMPORTANT ###

            this copies the essential executables to /usr/local/bin
            it also copies the include file icg.h to /usr/local/include
            libict.a to /usr/local/lib and Msg.pm to /usr/lib/perl5/site...
	    
	    (make uninstall as su will remove all these files)

        9)  Change to directory Test and compile a0.ic with iCmake

                cd Test
                iCmake a0.ic

            This should make a0.lst a0.c a0 and a0.ini
                a0.lst  is a listing
                a0.c    is the C source
                a0      is the executable
                a0.ini  is used for testing. It is the initialisation
                        output when executing a0. (Not required later)

        10) The support programs iCbox, which provides simulated I/O and
	    iClive, which provides program entry and debugging facilities
	    require Perl-Tk800.022 (see note below).

	    If you do not have Perl-Tk, install it from your Linux distribution
	    CD's or the copy supplied with this distribution at this point.

	    When Perl/Tk is installed on the system the following starts
	    a very simple demonstration:

                iCserver 'iCbox X0' a0

            An "iCbox" with 8 inputs and 8 outputs is displayed.
            Input IX0.0 can be turned on and off.
            Output QX0.0 follows the input, since program a0.ic simply
	    assigns immediate inputs to corresponding outputs:
                QX0.0 = IX0.0; ...
	    stop with ctrl-C

	11) Make all iC executable in this directory by running

		iCmake *.ic

	    The script 'a012.sh' starts 'iCserver', 3 x 'iCbox' and 'a012'
	    which sets all outputs from inputs like a0 above.

	12) The scripts 'bar3.sh' start demos showing running lights moving
	    every 100 ms. The lights should move smoothly on a 100 MHz or
	    faster computer. 'bar1.sh' and 'bar2.sh' require a faster computer.
	    'demo.sh' shows some logic processing.

	13) Running 'iClive bar3.ic' as a seperate process, while 'bar3' is
	    running, will display the source listing (in an edit window),
	    connect to iCServer as an auxiliary I/O and receive updates
	    of all variables from the running iC program (bar3). These
	    updates will change the colours of all words, which are
	    immediate variables.  (green/black = 0, yellow/red = 1)
	    This "live display" shows the current state of logical
	    relationships in statements of the iC program. Arithmetic
	    variables are displayed in a balloon, when the cursor rests
	    on a variable. (Arithmetic variables have a blue background).

	    The program 'iClive' uses the Tk::TextUndo widget, an extension
	    of Tk::Text. In 'Live' mode when a "live display" is shown the
	    text is read only. When the 'Edit' button is pressed 'iClive' is
	    a full featured editor. The edit facilities of this program are
	    described in the Tk::Text man page in the section 'BINDINGS'
	    near the end. (Control-u is <<Undo>>)

	14) I have included a script called 'stop' from my private toolkit.
	    It can be used effectively to kill iCserver when it is
	    executing in the background, which is appropriate for
	    a server.

		iCserver &
		.....
		../stop iCserver	# local copy of 'stop'

	    I have tried to use 'kill' with named processes as described in
	    the 'kill' manpage, but it does not seem to work, even called as
	    'command kill iCserver'.
	    You will have to install 'stop' manually in a PATH directory to
	    use it anywhere in your system. (see 'stop -h' for help)
	    'stop' is not a command on any Unix or Linux systems I know.
	    If it does interfere I suggest renaming it to 'Stop'.

        15) The StarOffice Word document doc/iC.sdw (or doc/iC.html)
            provide an introduction to the Programming Language iC.

            There are currently no man pages (on my TODO list) but
            the option -h gives help for all executables and scripts
            including applications generated by icc, eg: a0 -h

        
        Lots of success

        John E. Wulff   $YEAR.$MONTH.$DAY     <john\@je-wulff.de>

	NOTE for SuSE Linux 7.1:
	The program 'iCbox' requires Tk800.022 if you have Perl 5.6.0.
	SuSE Linux 7.1 has Tk800.014 in combination with Perl 5.6.0.
	With this constellation, iCbox produces 20 lines of warnings every
	time a virtual button is pressed, which executes \$tcp_conn->send_now()
	from a Tk callback function. Tk800.014 has 'require fileevent', which
	appears to go awry. (should use 'use'!!). Strangely the transmissions
	work - the voluminous 'croak' outputs are only warnings.
	
	If you download Tk800-022 from CPAN and install it, iCbox
	works fine. If you have an older distribution of Perl (5.005)
	this is not necessary.

	SuSE Linux 7.2 distribution has Tk800.022 and works fine.
EOF
########################################################################

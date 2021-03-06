#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

########################################################################
#
#	Copyright (C) 2000-2009  John E. Wulff
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the README file.
#
#   For more information about this program, or for information on how
#   to contact the author, see the README file
#
#	icg.pl
#
#	perl script to add #defines to /usr/local/include/icg.h
#
########################################################################

use strict;

my $named = $0;
$named =~ s#.*[/\\]##;		# delete directory names in path
format STDERR =
Usage:	@<<<<<<< [-h] [-D<def> ...] [<file> ...]
	$named
	-D<def>	zero or more -D<def> options may be supplied
		each will be output to stdout in 3 lines as
		    #ifndef <def>
		    #define <def>
		    #endif
		if <def> is in the form xxx=yyy, '=' is replaced by tab
		    #ifndef xxx
		    #define xxx  yyy
		    #endif
	<file>	finally <file> is copied from the line following one containing
		'STARTFILE' to one before one containing 'ENDFILE' to stdout
		(could be several files)
		if no <file> argument, stdin is copied to stdout
	-h	help, ouput this Usage text only
$Id: icg.pl,v 1.5 2015/10/16 12:33:47 jw Exp $
.

use vars qw($opt_h);
my @opt_D;
use Getopt::Long qw(:config no_ignore_case bundling);
## getopts('D:h');
GetOptions (
    'D=s' => \@opt_D,
    'h'   => \$opt_h);

if ($opt_h) {
    write STDERR; exit 0;	# -h, ouput Usage only
}

my ($option, $macro);
my $outFlag = 0;

while ($option = $macro = shift @opt_D) {
    if ($macro =~ s/(\w+)=/$1\t/) {
	$option = $1;
    }
    print "#ifndef	$option\n".
	  "#define	$macro\n".
	  "#endif\n";
}

while (<>) {
    if (/STARTFILE/) {
	$outFlag = 1;
	next;
    }
    if (/ENDFILE/) {
	$outFlag = 0;
	next;
    }
    print if $outFlag;
}

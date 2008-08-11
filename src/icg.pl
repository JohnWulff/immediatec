#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

########################################################################
#
#	Copyright (C) 2000-2008  John E. Wulff
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the README file.
#
#   For more information about this program, or for information on how
#   to contact the author, see the README file or <ic@je-wulff.de>
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
		each will be output to stdout in one line as
		#define <def>
		if <def> is in the form xxx=yyy, '=' is replaced by tabs
		#define xxx  yyy
	<file>	finally <file> is copied from the line following one containing
		'STARTFILE' to one before one containing 'ENDFILE' to stdout
		(could be several files)
		if no <file> argument, stdin is copied to stdout
	-h	help, ouput this Usage text only
$Id: icg.pl,v 1.2 2008/06/25 21:44:39 jw Exp $
.

use vars qw($opt_D $opt_h);
require "getopts.pl";
&Getopts('D:h');

if ($opt_h) {
    write STDERR; exit 0;	# -h, ouput Usage only
}

my @Doptions = split(' ', $opt_D);
my $option;
my $outFlag = 0;

while ($option = shift @Doptions) {
    $option =~ s/=/\t\t/;
    print "#define	$option\n";
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

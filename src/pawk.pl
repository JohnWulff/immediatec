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
#	pawk.pl
#
#	perl script generated from awk script pawk.a
#
#   NOTE: the 'ident' filter is no longer used, since it is part of
#         the 'RCS' package and may not be installed by some users.
#         The power of Perl regular expressions is used instead.
#
########################################################################

use strict;

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
			# process any FOO=bar switches
my $rev = '@(#)     $Id: pawk.pl,v 1.6 2008/06/25 21:47:44 jw Exp $';
my $name = 'iC_ID';
my $X = '';
my $mod = '';
my $id = "Id";

while (<>) {
    chomp;	# strip record separator
    if (/(\$$id:[^\$]*\$)/o) {
	printf "%sconst char %s%s[] =\n", $mod, $name, $X;
	$X++;
	$mod = 'static ';
	printf "\t\"@%s     %s\";\n", '(#)', $1;
	if ($X <= 1) {
	    printf (("#ifndef LOAD\n"));
	}
    }
}

printf (("#endif\n"));
printf "%sconst char %s%s[] =\n", $mod, $name, $X;
printf "\t\"%s\";\n", $rev;


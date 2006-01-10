#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

########################################################################
#
#	Copyright (C) 2000-2005  John E. Wulff
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the README file.
#
#   For more information about this program, or for information on how
#   to contact the author, see the README file or <john@je-wulff.de>
#
#	pawk.pl
#
#	perl script generated from awk script pawk.a
#
########################################################################

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
			# process any FOO=bar switches
$rev = '@(#)     $Id: pawk.pl,v 1.5 2006/01/10 22:00:55 jw Exp $';
$name = 'iC_ID';
$X = '';
$mod = '';

while (<>) {
    chomp;	# strip record separator
    if (/\$Id/) {
	printf "%sconst char %s%s[] =\n", $mod, $name, $X;
	$X++;
	$mod = 'static ';
	printf "\t\"@%s%s\";\n", '(#)', $_;
	if ($X <= 1) {
	    printf (("#ifndef LOAD\n"));
	}
    }
}

printf (("#endif\n"));
printf "%sconst char %s%s[] =\n", $mod, $name, $X;
printf "\t\"%s\";\n", $rev;


#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

########################################################################
#
#	Copyright (C) 2000-2016  John E. Wulff
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the README file.
#
#   For more information about this program, or for information on how
#   to contact the author, see the README file
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
my $rev = '@(#)     $Id: pawk.pl 1.10 $';
my $name = 'iC_ID';
my $X = '';
my $mod = '';
my $ID = "Id:";
my $describe = '';
my $patch = '';
my $RV = "Revision:";

use vars qw(@opt_O);
use Getopt::Long;
GetOptions (
    'O=s' => \@opt_O,
);

while (<>) {
    chomp;	# strip record separator
    if (/(\$$ID[^\$]*\$)/o) {
	printf "%sconst char %s%s[] =\n", $mod, $name, $X;
	if ($mod) {
	    printf "\t\"@%s     %s\";\n", '(#)', $1;
	} else {
	    if ($describe = qx(git describe --always --dirty 2> /dev/null)) {
		chomp $describe;
		print "\t\"\$$RV $describe \$\";\n";
		if ($patch = qx(git diff --diff-filter=d HEAD)) {
		    $patch =~ s/\\/\\\\/g;
		    $patch =~ s/"/\\"/g;
		    $patch =~ s/\n/\\n"\n"/g;
		}
	    } else {
		printf "\t\"@%s     %s\";\n", '(#)', $1;
	    }
	    print "const char iC_PATCH[] =\n\t\"$patch\";\n";
	    print "#ifndef LOAD\n";
	}
	$X++;
	$mod = 'static ';
    }
}

print "#endif\n";
printf "%sconst char %s%s[] =\n", $mod, $name, $X;
printf "\t\"%s\";\n", $rev;
printf "const char iC_OPT[] =\n";
printf "\t\"%s\";\n", "@opt_O";

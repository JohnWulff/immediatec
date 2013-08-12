#!/usr/bin/perl

########################################################################
#   Generate the pre-compiled functions for including permanently in
#   init.c from the iC source file ddd.ic
#
#   This scheme only works for for simple functions with one level of
#   master gates for each input and individual clocks for each input if
#   the builtin function is clocked (all except FORCE() are clocked)
#
#   execute the following after making a compiler with -D BOOT_COMPILE
#	immcc -d20000 ddd.ic | ddd.pl > ddd.out
#	vi ddd.out init.c	# insert the new code appropriately
#	# ddd.ic has the definitions of the buil-in functions with
#	# mangled names in an appropriate alphabetical order
#	# ddd.pl translates the mangled names to their proper names
#	# and moves the block of static Symbols ahead of the link
#	# definitions, so that addresses of static Symbols can be used
#	# in some links.
#	# use the modified init.c to make a new immcc compiler.
#	# (the original was produced manually and was used to check
#	# that this generated code produces the right results - now
#	# the automatic generator can be used to vary the built-ins)
#
# $Id: init_t.pl,v 1.1 2013/08/12 06:37:58 jw Exp $
########################################################################

use strict;

my $key;
my $save = 0;
my $line;
my @block = ();
my %xlate = (
    Aorce => "FORCE",
    B => "D",
    Cr_ => "SR_",
    Dr_ => "DR_",
    Err_ => "SRR_",
    Fsr_ => "DSR_",
    Gh => "SH",
    Hhr_ => "SHR_",
    Ihsr_ => "SHSR_",
    Jise => "RISE",
    Khange => "CHANGE",
    Llock => "CLOCK",
    Mimer => "TIMER",
    Nimer1 => "TIMER1",
    Olock => "CLOCK",
    Pimer => "TIMER",
    Qimer1 => "TIMER1",
);

while (<>) {
    if (/^static List_e\s+l\[\] = {$/) {
	$save = 1;
    }
    if (/^static BuiltIn\s+b\[\] = {$/) {
	foreach $line (@block) {
	    print $line;
	}
	print "\n";
    }
    foreach $key (sort keys %xlate) {
	s/\b$key(2)?(a_[1-3])?\b/$xlate{$key}$1$2/g;
    }
    if ($save) {
	push(@block, $_);
    } else {
	print;
    }
    if ($save and /^};$/) {
	$save = 0;
    }
}

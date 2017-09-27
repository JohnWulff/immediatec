#!/usr/bin/perl

########################################################################
#
#	Copyright (C) 2000-2013  John E. Wulff
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the README file.
#
#   For more information about this program, or for information on how
#   to contact the author, see the README file
#
#	init_t.pl
#
#   Generate the pre-compiled functions for including permanently in
#   init.c from the iC source file init_t.ic
#
#   This scheme only works for simple functions with one level of
#   master gates for each input and individual clocks for each input if
#   the builtin function is clocked (all except FORCE() are clocked)
#
#   execute the following after making a compiler with -D BOOT_COMPILE
#	immcc -d20000 init_t.ic | init_t.pl > init_t.out
#	vi init_t.out init.c	# insert the new code appropriately
#	# init_t.ic has the definitions of the buil-in functions with
#	# mangled names in an appropriate alphabetical order
#	# init_t.pl translates the mangled names to their proper names
#	# and moves the block of static Symbols ahead of the link
#	# definitions, so that addresses of static Symbols can be used
#	# in some links.
#	# use the modified init.c to make a new immcc compiler.
#	# (the original was produced manually and was used to check
#	# that this generated code produces the right results - now
#	# the automatic generator can be used to vary the built-ins)
#   to check the output init_t.out against init.c with the changes
#   from v 1.3 to 1.4 do the following:
#   perl -e 'while (<>) { s/0xc([12])/0x8$1/g; s/TIMER02/TIMER2/g; print; }' init_t.out | diff -wB - init.c | less
#
# $Id: init_t.pl 1.5 $
########################################################################

use strict;

my $key;
my $save = 0;
my $line;
my @block = ();
my %xlate = (
# the following mangled names must be in sorted order and the correct length
# low level built-in functions - functions which cannot be compiled as function blocks
    Aorce	=> "FORCE",
    B		=> "D",
    Bs_		=> "DS_",
    Cr_		=> "DR_",
    Csr_	=> "DSR_",
    Dr_		=> "SR_",
    Drr_	=> "SRR_",
    Gh		=> "SH",
    Hhr_	=> "SHR_",
    Ihsr_	=> "SHSR_",
    Jise	=> "RISE",
    Khange	=> "CHANGE",
    Llock	=> "CLOCK",
    Mimer	=> "TIMER",
# extended built-in functions - previously compiled as system function blocks at start up
    Ns		=> "DS",
    Or		=> "DR",
    Osr		=> "DSR",
    Pr		=> "SR",
    Prr		=> "SRR",
    Prx		=> "SRX",
    Qk		=> "JK",
    Uhr		=> "SHR",
    Uhsr	=> "SHSR",
    Vt		=> "ST",
    Wrt		=> "SRT",
    Xall	=> "FALL",
    Yatch	=> "LATCH",
    Zlatch	=> "DLATCH",
# end built-in functions
);

while (<>) {
    if (/^#define FO	FUN_OFFSET$/) {
	$save = 1;
    }
    if (@block and /^#e\w+	\/\* BOOT_COMPILE \*\/$/) {
	foreach $line (@block) {
	    print $line;
	}
	@block = ();
    }
    foreach $key (sort keys %xlate) {
	s/\b$key([012]*)(a_[1-9])?\b/$xlate{$key}$1$2/g;
    }
    if ($save) {
	push(@block, $_);
    } else {
	print;
    }
    if ($save and /^};/) {
	$save = 0;
    }
}

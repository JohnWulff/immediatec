#! /usr/bin/perl

#################################################################
#
#   Fix pplc listing with output inversions
#
#   This is necessary because of one pass compiler limitations.
#
#   Author: John E. Wulff  2.1.2001
#   Written entirely in the car between Siegen and Eisingen
#
#################################################################

use strict;

my $named = $0;
$named =~ s#.*[/\\]##;		# delete directory names in path
format STDERR =
Usage:	@<<<<<<< [file ...]
	$named
	-h	help, ouput this Usage text only
$Id: pplstfix.pl,v 1.2 2001/01/02 19:42:44 jw Exp $
.

use vars qw($opt_h);
require "getopts.pl";
&Getopts('h');			# sets $opt_h if -h etc

if ($opt_h) {
    write STDERR; exit 0;	# -h, ouput Usage only
}

my (%inv);
%inv = ("~", " ", " ", "~");	# inversion symbols in listing

while ($ARGV = shift @ARGV) {
    unless (open (IN, $ARGV)) {
	warn "can't open input $ARGV";
	next;
    }
    processFile();
    close(IN);
}

####################################################################
#
#	process one file
#
####################################################################

sub processFile {
    my ($line, $bsp, $fsp, %symbols);
    %symbols = ();

    # Pass 1: scan file for key sentences

    while (<IN>) {
	if (/^\tprevious inputs \'(Q[BWX][\d._]+)\' must be inverted$/) {
	    $line = $.;
	    $bsp = $1;
	}
	if (/^\texcept input \'(Q[BWX][\d._]+)\' to own output \'(Q[BWX][\d._]+)\'$/) {
	    if ($bsp eq $1) {
		$fsp = $2;
		$symbols{$bsp} = [ $line, $fsp ];
	    } else {
		warn("Error: $named: $.: $_");
	    }
	}
    }

    # Pass 2: correct inversion errors

    seek(IN, 0, 0);
    $. = 0;
  line:
    while (<IN>) {
	if (/(Q[BWX][\d._]+)/ and defined($symbols{$1})) {
	    # assume only one relevant Qvariable per line
	    my ($ar);
	    $bsp = $1;
	    $ar = $symbols{$bsp};
	    $line = $ar->[0];
	    $fsp = $ar->[1];
	    if ($line == $.) {
		delete $symbols{$bsp};
		$_ = <IN>;
		$_ = <IN>;			# ignore 3 inserted lines
		next line;
	    }
	    if (! /\b$fsp\b/) {			# don't modify own output
		s/([ ~]) ---/$inv{$1} ---/;	# swap ' ' and '~'
	    }
	}
	print $_;
    }
} # processFile

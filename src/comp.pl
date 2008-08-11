#!/usr/bin/perl -w
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
  if 0;

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
#	comp.pl
#
#	perl script generated from sed script comp.sed
#
########################################################################
$0 =~ s/^.*?(\w+)[\.\w+]*$/$1/;

use strict;
use Symbol;
use vars qw{ $isEOF $Hold %wFiles @Q $CondReg
	     $doAutoPrint $doOpenWrite $doPrint };
$doAutoPrint = 1;
$doOpenWrite = 1;
# prototypes
sub openARGV();
sub getsARGV(;\$);
sub eofARGV();
sub printQ();

# Run: the sed loop reading input and applying the script
#
sub Run(){
    my( $h, $icnt, $s, $n );
    # hack (not unbreakable :-/) to avoid // matching an empty string
    my $z = "\000"; $z =~ /$z/;
    # Initialize.
    openARGV();
    $Hold    = '';
    $CondReg = 0;
    $doPrint = $doAutoPrint;
CYCLE:
    while( getsARGV() ){
	chomp();
	$CondReg = 0;   # cleared on t
BOS:;
# # $Id: comp.pl,v 1.5 2008/06/25 21:38:16 jw Exp $
# s/yyparse/iCparse/g
{ $s = s /yyparse/iCparse/sg;
  $CondReg ||= $s;
}
# s/yylex/iClex/g
{ $s = s /yylex/iClex/sg;
  $CondReg ||= $s;
}
# s/yyleng/iCleng/g
{ $s = s /yyleng/iCleng/sg;
  $CondReg ||= $s;
}
# s/yytext/iCtext/g
{ $s = s /yytext/iCtext/sg;
  $CondReg ||= $s;
}
# s/yybuf/iCbuf/g
{ $s = s /yybuf/iCbuf/sg;
  $CondReg ||= $s;
}
# s/yychar/iCchar/g
{ $s = s /yychar/iCchar/sg;
  $CondReg ||= $s;
}
# s/yyval/iCval/g
{ $s = s /yyval/iCval/sg;
  $CondReg ||= $s;
}
# s/yylval/iClval/g
{ $s = s /yylval/iClval/sg;
  $CondReg ||= $s;
}
# s/yydebug/iCdebug/g
{ $s = s /yydebug/iCdebug/sg;
  $CondReg ||= $s;
}
# s/yynerrs/iCnerrs/g
{ $s = s /yynerrs/iCnerrs/sg;
  $CondReg ||= $s;
}
# s/yyerrflag/iCerrflag/g
{ $s = s /yyerrflag/iCerrflag/sg;
  $CondReg ||= $s;
}
# s/yyss/iCss/g
{ $s = s /yyss/iCss/sg;
  $CondReg ||= $s;
}
# s/yyssp/iCssp/g
{ $s = s /yyssp/iCssp/sg;
  $CondReg ||= $s;
}
# s/yyvs/iCvs/g
{ $s = s /yyvs/iCvs/sg;
  $CondReg ||= $s;
}
# s/yyvsp/iCvsp/g
{ $s = s /yyvsp/iCvsp/sg;
  $CondReg ||= $s;
}
# s/yylhs/iClhs/g
{ $s = s /yylhs/iClhs/sg;
  $CondReg ||= $s;
}
# s/yylen/iClen/g
{ $s = s /yylen/iClen/sg;
  $CondReg ||= $s;
}
# s/yydefred/iCdefred/g
{ $s = s /yydefred/iCdefred/sg;
  $CondReg ||= $s;
}
# s/yydgoto/iCdgoto/g
{ $s = s /yydgoto/iCdgoto/sg;
  $CondReg ||= $s;
}
# s/yysindex/iCsindex/g
{ $s = s /yysindex/iCsindex/sg;
  $CondReg ||= $s;
}
# s/yyrindex/iCrindex/g
{ $s = s /yyrindex/iCrindex/sg;
  $CondReg ||= $s;
}
# s/yygindex/iCgindex/g
{ $s = s /yygindex/iCgindex/sg;
  $CondReg ||= $s;
}
# s/yytable/iCtable/g
{ $s = s /yytable/iCtable/sg;
  $CondReg ||= $s;
}
# s/yycheck/iCcheck/g
{ $s = s /yycheck/iCcheck/sg;
  $CondReg ||= $s;
}
# s/yyname/iCname/g
{ $s = s /yyname/iCname/sg;
  $CondReg ||= $s;
}
# s/yyrule/iCrule/g
{ $s = s /yyrule/iCrule/sg;
  $CondReg ||= $s;
}
# s/#define YYPREFIX "yy"/#define YYPREFIX "iC"/g
{ $s = s /#define YYPREFIX "yy"/#define YYPREFIX "iC"/sg;
  $CondReg ||= $s;
}
# s/YYSTYPE/iCSTYPE/g
{ $s = s /YYSTYPE/iCSTYPE/sg;
  $CondReg ||= $s;
}
# s/y\.tab\.c/comp.c/g
{ $s = s /y\.tab\.c/comp.c/sg;
  $CondReg ||= $s;
}
EOS:    if( $doPrint ){
            print $_, "\n";
        } else {
	    $doPrint = $doAutoPrint;
	}
        printQ() if @Q;
    }

    exit( 0 );
}
Run();

# openARGV: open 1st input file
#
sub openARGV(){
    unshift( @ARGV, '-' ) unless @ARGV;
    my $file = shift( @ARGV );
    open( ARG, "<$file" )
    || die( "$0: can't open $file for reading ($!)\n" );
    $isEOF = 0;
}

# getsARGV: Read another input line into argument (default: $_).
#           Move on to next input file, and reset EOF flag $isEOF.
sub getsARGV(;\$){
    my $argref = @_ ? shift() : \$_;
    while( $isEOF || ! defined( $$argref = <ARG> ) ){
	close( ARG );
	return 0 unless @ARGV;
	my $file = shift( @ARGV );
	open( ARG, "<$file" )
	|| die( "$0: can't open $file for reading ($!)\n" );
	$isEOF = 0;
    }
    1;
}

# eofARGV: end-of-file test
#
sub eofARGV(){
    return @ARGV == 0 && ( $isEOF = eof( ARG ) );
}

# makeHandle: Generates another file handle for some file (given by its path)
#             to be written due to a w command or an s command's w flag.
sub makeHandle($){
    my( $path ) = @_;
    my $handle;
    if( ! exists( $wFiles{$path} ) || $wFiles{$path} eq '' ){
        $handle = $wFiles{$path} = gensym();
	if( $doOpenWrite ){
	    if( ! open( $handle, ">$path" ) ){
		die( "$0: can't open $path for writing: ($!)\n" );
	    }
	}
    } else {
        $handle = $wFiles{$path};
    }
    return $handle;
}

# printQ: Print queued output which is either a string or a reference
#         to a pathname.
sub printQ(){
    for my $q ( @Q ){
	if( ref( $q ) ){
            # flush open w files so that reading this file gets it all
	    if( exists( $wFiles{$$q} ) && $wFiles{$$q} ne '' ){
		open( $wFiles{$$q}, ">>$$q" );
	    }
            # copy file to stdout: slow, but safe
	    if( open( RF, "<$$q" ) ){
		while( defined( my $line = <RF> ) ){
		    print $line;
		}
		close( RF );
	    }
	} else {
	    print $q;
	}
    }
    undef( @Q );
}


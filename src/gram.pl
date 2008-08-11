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
#	gram.pl
#
#	perl script generated from sed script gram.sed
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
# # $Id: gram.pl,v 1.4 2008/06/25 21:39:18 jw Exp $
# s/yyparse/c_parse/g
{ $s = s /yyparse/c_parse/sg;
  $CondReg ||= $s;
}
# s/yylex/c_lex/g
{ $s = s /yylex/c_lex/sg;
  $CondReg ||= $s;
}
# s/yyleng/c_leng/g
{ $s = s /yyleng/c_leng/sg;
  $CondReg ||= $s;
}
# s/yytext/c_text/g
{ $s = s /yytext/c_text/sg;
  $CondReg ||= $s;
}
# s/yybuf/c_buf/g
{ $s = s /yybuf/c_buf/sg;
  $CondReg ||= $s;
}
# s/yychar/c_char/g
{ $s = s /yychar/c_char/sg;
  $CondReg ||= $s;
}
# s/yyval/c_val/g
{ $s = s /yyval/c_val/sg;
  $CondReg ||= $s;
}
# s/yylval/c_lval/g
{ $s = s /yylval/c_lval/sg;
  $CondReg ||= $s;
}
# s/yydebug/c_debug/g
{ $s = s /yydebug/c_debug/sg;
  $CondReg ||= $s;
}
# s/yynerrs/c_nerrs/g
{ $s = s /yynerrs/c_nerrs/sg;
  $CondReg ||= $s;
}
# s/yyerrflag/c_errflag/g
{ $s = s /yyerrflag/c_errflag/sg;
  $CondReg ||= $s;
}
# s/yyss/c_ss/g
{ $s = s /yyss/c_ss/sg;
  $CondReg ||= $s;
}
# s/yyssp/c_ssp/g
{ $s = s /yyssp/c_ssp/sg;
  $CondReg ||= $s;
}
# s/yyvs/c_vs/g
{ $s = s /yyvs/c_vs/sg;
  $CondReg ||= $s;
}
# s/yyvsp/c_vsp/g
{ $s = s /yyvsp/c_vsp/sg;
  $CondReg ||= $s;
}
# s/yylhs/c_lhs/g
{ $s = s /yylhs/c_lhs/sg;
  $CondReg ||= $s;
}
# s/yylen/c_len/g
{ $s = s /yylen/c_len/sg;
  $CondReg ||= $s;
}
# s/yydefred/c_defred/g
{ $s = s /yydefred/c_defred/sg;
  $CondReg ||= $s;
}
# s/yydgoto/c_dgoto/g
{ $s = s /yydgoto/c_dgoto/sg;
  $CondReg ||= $s;
}
# s/yysindex/c_sindex/g
{ $s = s /yysindex/c_sindex/sg;
  $CondReg ||= $s;
}
# s/yyrindex/c_rindex/g
{ $s = s /yyrindex/c_rindex/sg;
  $CondReg ||= $s;
}
# s/yygindex/c_gindex/g
{ $s = s /yygindex/c_gindex/sg;
  $CondReg ||= $s;
}
# s/yytable/c_table/g
{ $s = s /yytable/c_table/sg;
  $CondReg ||= $s;
}
# s/yycheck/c_check/g
{ $s = s /yycheck/c_check/sg;
  $CondReg ||= $s;
}
# s/yyname/c_name/g
{ $s = s /yyname/c_name/sg;
  $CondReg ||= $s;
}
# s/yyrule/c_rule/g
{ $s = s /yyrule/c_rule/sg;
  $CondReg ||= $s;
}
# s/#define YYPREFIX "yy"/#define YYPREFIX "c_"/g
{ $s = s /#define YYPREFIX "yy"/#define YYPREFIX "c_"/sg;
  $CondReg ||= $s;
}
# s/YYSTYPE/c_STYPE/g
{ $s = s /YYSTYPE/c_STYPE/sg;
  $CondReg ||= $s;
}
# s/y\.tab\.c/gram.c/g
{ $s = s /y\.tab\.c/gram.c/sg;
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

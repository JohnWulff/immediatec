/***********************************************************************
 *
 *      Copyright (C) 2020-2022 John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 ***********************************************************************
 *
 *  This program uses the 'govaluate' library available from
	"github.com/Knetic/govaluate" under the following license:
 *  The MIT License (MIT)
 *  Copyright (c) 2014-2016 George Lester
 *  The use of this library is gratefully acknowledged.
 *
 ***********************************************************************
 *
 *      immag.go ==> immag; copied to /usr/local/bin/immac for execution
 *
 *  Pre-compiler to convert "immediate C with arrays" (iCa) source files.
 *  Based on the PERL version of immac, which has beem migrated to GO.
 *
 *  Expands FOR loops and handles IF ELSE statements as well as object
 *  like or function like macros with pre-compile prefix %% into straight
 *  "immediate C" (iC).
 *      "immac" handles %%define %%if etc for a file with .ica extension
 *      "immac" handles %define %if etc for a file with .ic extension
 *      without FOR expansion etc, which is the same as calling "immac -M"
 *
 *  Also several options to handle macros only (no FOR expansion etc):
 *      "immac -M" handles %define %if etc only for straight iC code
 *      "immac -m" handles #define #if etc only to replace cpp for C code
 *      "immac -Y" handles %if etc only (no %define) for conditionals in
 *                 yacc code (yacc and/or bison use %define themselves).
 *
 *  This version of immac is for the iC language version 3 and up.
 *
 *  The following prefixes have been changed from iC version 2:
 *      "pre-compiler prefix in"    vers 2        vers 3
 *       iC code in .ic and .ica       #             %
 *       iCa code in .ica              %             %%
 *       C code in literal blocks      %#            #
 *  These are the only changes in the iC language version 3 (Feb 2017).
 *
 *  This means that #define #if etc macros in C literal blocks will be
 *  written just like in C and passed unchanged to the generated C code.
 *
 *  Several constructs available in the PERL version of immac prior to Jan 2022
 *  have been dropped:
 *      -S and -N flags. immac is now always strict requiring twin braces.
 *      'use strict' and 'no strict' were a mistake in the PERL version because
 *      they are part of the iC language and must be copied to the iC file.
 *      -L option. All lines go to the generated GO file, which is log file.
 *      ELSIF   // is very Perlish and can easily be replaced by ELSE IF.
 *      FOR S ("aa" .. "ad") {{ // is very difficult to implement in GO and
 *                              // was never used in thousands of my examples.
 *      End of line Perl comment started with '#' is no longer supported.
 *
 *  The standard C style FOR loop to generate integer lists is:
 *      FOR (I = 0; I < 8; I++) {{      // list of integers (type of I is int)
 *  The only specifically PERL constructs being retained are:
 *      FOR S ("aa", "ab", "12", -3) {{ // list of parenthesised strings
 *      FOR I (1, 2, -3, +4) {{         // list of integers with optional sign
 *      FOR I (1 .. 4) {{               // list of integers using .. operator

 *  These are very useful and easy to implement in GO. List elements in
 *  double quotes are type 'string'. List elements containing only numeric
 *  characters with or without a leading sign are type 'int' unless part of
 *  a list also containing strings.
 *  List of bareword strings is no longer supported.
 *
 *  The optional type-specifier 'int' before a FOR loop control variable
 *  is ignored. This usage is deprecated because the contol variable may
 *  also be type 'string'. A Warning is now issued.
 *
 *  Integer control variables may only be used in integer arithmetic expressions.
 *  Type 'string' control variables may only be used with the concatination
 *  operator '.' in expressions (borrowed from Perl - not '+' as in GO).
 *  NOTE: String variables and concatenation are only used in the iCa language,
 *        never in iC.
 *
 *  Input and output filnames must be restricted to the following
 *  POSIX Portable Filename Character Set consisting of:
 *      A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
 *      a b c d e f g h i j k l m n o p q r s t u v w x y z
 *      0 1 2 3 4 5 6 7 8 9 . _ -
 *  Directory separator for Linux/Unix is:  /
 *  Directory separator for Windows is:     \
 *
 ***********************************************************************/

package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"errors"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"github.com/Knetic/govaluate"
)

const ID_immag_go = "$Id: immag.go 1.3 $"
const DEF  = true
const EVAL = true

/***********************************************************************
 *
 *  Coding of iC, iCa and C keyword map values for possible future use
 *              track with BuiltIn b[] in init.c
 *      0x001   imm or immC iC type modifier
 *      0x002   iC type
 *      0x004   iC pragma
 *      0x008   iC keyword
 *      0x010   iC clocked built-in function
 *      0x020   iC unclocked built-in bit function
 *      0x030   iC clocked built-in bit function
 *      0x050   iC clocked built-in arithmetic function
 *      0x070   iC clocked built-in bit or arithmetic function
 *      0x090   iC clocked built-in clock function
 *      0x0b0   iC clocked built-in timer function
 *      0x100   iCa keyword FOR IF ELSE	(value used)
 *      0x200   C keyword
 *      0x400   iC keyword if else switch - to enter C code (value used)
 *
 ***********************************************************************/

var keywords = map[string]int {
	"FORCE":    0x020,
	"D":        0x030, // D flip-flop
	"DR":       0x030, // D flip-flop with reset
	"DR_":      0x030, // D flip-flop with simple reset
	"DSR":      0x030, // D flip-flop with set/reset
	"DSR_":     0x030, // D flip-flop with simple set/reset
	"SR":       0x030, // R_FF for reset master
	"SR_":      0x030, // R_FF for reset master
	"SRR":      0x030, // R_FF for reset master
	"SRR_":     0x030, // R_FF for reset master
	"SRX":      0x030, // R_FF for reset master
	"JK":       0x030, // R_FF for reset master
	"ST":       0x030, // monoflop with timed reset
	"SRT":      0x030, // monoflop with reset and timed reset
	"SH":       0x050, // sample and hold
	"SHR":      0x070, // sample and hold with reset
	"SHR_":     0x070, // sample and hold with simple reset
	"SHSR":     0x070, // sample and hold with set/reset
	"SHSR_":    0x070, // sample and hold with simple set/reset
	"LATCH":    0x020,
	"DLATCH":   0x030,
	"RISE":     0x030, // pulse on digital rising edge
	"CHANGE":   0x070, // pulse on anlog or digital change
	"CHANGE2":  0x050, // alternate CHANGE on anlog change
	"CLOCK":    0x090, // CLOCK with 1 or 2 inputs
	"CLOCK2":   0x090, // alternate CLOCK with 2 inputs
	"TIMER":    0x0b0, // TIMER with preset off 0
	"TIMER2":   0x0b0, // alternate TIMER with 2 inputs
	"TIMER1":   0x0b0, // TIMER1 with preset off 1
	"TIMER12":  0x0b0, // alternate TIMER1 with 2 inputs
	"if":       0x608,
	"else":     0x608,
	"switch":   0x608,
	"extern":   0x208,
	"assign":   0x008,
	"return":   0x208,
	"no":       0x004, // turn off use
	"use":      0x004, // turn on use
	"alias":    0x004,
	"list":     0x004,
	"strict":   0x004,
	"imm":      0x001,
	"immC":     0x001,
	"void":     0x202,
	"bit":      0x002,
	"int":      0x202,
	"clock":    0x002,
	"timer":    0x002,
	"this":     0x008, // only used in function block definitions
	"auto":     0x200, // C keywords to cause syntax errors if used in iC
	"break":    0x200,
	"case":     0x200,
	"char":     0x200,
	"const":    0x200,
	"continue": 0x200,
	"default":  0x200,
	"do":       0x200,
	"double":   0x200,
	"enum":     0x200,
	"float":    0x200,
	"for":      0x200,
	"goto":     0x200,
	"long":     0x200,
	"register": 0x200,
	"short":    0x200,
	"signed":   0x200,
	"sizeof":   0x200,
	"static":   0x200,
	"struct":   0x200,
	"typedef":  0x200,
	"union":    0x200,
	"unsigned": 0x200,
	"volatile": 0x200,
	"while":    0x200,
	"fortran":  0x200,
	"asm":      0x200,
	"FOR":      0x100, // keyword used in immac FOR loops
	"IF":       0x100, // keyword used in immac IF statements
	"ELSE":     0x100, // keyword used in immac ELSE statements
	"..":       0x0,   // pseudo keyword around which spaces are kept
	"...":      0x0,   // pseudo keyword around which spaces are kept
}

type multStringFlag []string // flag can be used multiple times returning []string

func (m *multStringFlag) String() string {
	return "" // satisfies interface - not used ??
}

func (m *multStringFlag) Set(value string) error {
	*m = append(*m, strings.TrimSpace(value))
	return nil
}

type numNum struct {
	nm1 int
	nm2 int
}

type strNum struct {
	num int					// number of macro parameters
	str string				// macro replacement string
}

type fileStk struct {
	name string
	line int
	si   int
	file *os.File
	scan *bufio.Scanner
}

type SrtQmkColEndFlg struct {
	sta int
	qmk int
	col int
	end int
	flg bool
}

var named string
var flag1 *flag.FlagSet
var opt_I, opt_P, opt_D, opt_U multStringFlag
var opt_o, opt_l, opt_e *string
var opt_t, opt_m, opt_M, opt_N, opt_Y, opt_a, opt_n, opt_S, opt_T, opt_X, opt_Z *bool
var opt_m_M_Y, iCmode, litBlock bool
var defs   = map[string]*strNum {}
var clDefs = map[string]*strNum {}
var siStk = make([]int, 0)
var argIn string
var genFile *os.File
var lnErr = make([]string, 0)
var lineNo  int
var hashLineFlag bool
var lineText string
var FORline string
var w int
var ret int
var blankLines int
var Used = make(map[string]string)
var FORctrlVar   = make(map[string]string)
var FORctrlIndex = make(map[string]int)
var path string
var iesBrace     = make([]bool, 5)
var genSlice     = make([]string, 0, 5)
var LFflag bool
var SQdotSlice   = make([]SrtQmkColEndFlg, 5)
var SQprCnt int
var SQxFlag bool
var genFlag bool
var reCommEnd    *regexp.Regexp
var reQuoteSlash *regexp.Regexp
var reWord       *regexp.Regexp
var reDefined    *regexp.Regexp
var reNum        *regexp.Regexp
var reSignNum    *regexp.Regexp
var reConstExpr  *regexp.Regexp
var reEvalIf     *regexp.Regexp
var reLdTrSps    *regexp.Regexp
var reTrSpaces   *regexp.Regexp
var reSpaces     *regexp.Regexp
var reTxSpaces   *regexp.Regexp
var reAllSpaces  *regexp.Regexp
var reCcomment   *regexp.Regexp
var reCcomTail   *regexp.Regexp
var reMacro      *regexp.Regexp
var reComma      *regexp.Regexp
var reHashHash   *regexp.Regexp
var reHashSlash  *regexp.Regexp
var reHash       *regexp.Regexp
var reTranslate  *regexp.Regexp
var reResolve    *regexp.Regexp
var reTrue       *regexp.Regexp
var reFalse      *regexp.Regexp
var reFORin1     *regexp.Regexp
var reFORin2     *regexp.Regexp
var rePercent    *regexp.Regexp
var reDivInt     *regexp.Regexp

/********************************************************************
 *
 *	Convert from Posix bundled flag interpretation to Go type flags.
 *
 *	Modify command line bundled flags to interpret -abc as -a -b -c,
 *	which is the way flag.Parse must be presented with Bool flags.
 *	flag.Parse interprets -abc as the flag named "abc".
 *
 *	Mofify Value flags to interpret -xVal as -x Val. Can also have
 *	-abcx Val or -abcxVal, which are both converted to -a -b -c -x Val.
 *	Value flags could be identified by the flag initialisers above,
 *	but it is simpler to define a map of Value flag identifiers.
 *
 *	Command line flags starting with -- are not touched, which is the
 *	Posix way of mixed flags starting with - or --.
 *	https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html
 *
 *******************************************************************/

var args = make([]string, 0, len(os.Args))
var argsP *[]string
var commandLineFlags = map[string]int{
	"M": 0, // Bool flag
	"N": 0,
	"S": 0,
	"T": 0,
	"X": 0,
	"Y": 0,
	"Z": 0,
	"a": 0,
	"m": 0,
	"n": 0,
	"t": 0,
	"h": 0,
	"D": 1, // value flag
	"I": 1,
	"P": 1,
	"U": 1,
	"e": 1,
	"l": 1,
	"o": 1,
}

func convertFlags(argp *[]string, offset int) {
	for _, arg := range (*argp)[offset:] {
		if len(arg) == 0 {
			continue
		}
		Sflag := true
		if arg != "-" && arg[0] == '-' && arg[1] != '-' {
			Sflag = false
			for i, sf := range strings.Split(arg[1:], "") {
				clf, ok := 0, false
				if clf, ok = commandLineFlags[sf]; !ok {
					arg = "-" + arg[i+1:]	// not our flag - nevertheless retain '-'
					Sflag = true
					break
				}
				*argsP = append(*argsP, "-"+sf)
				if clf == 1 && i+2 < len(arg) {
					arg = arg[i+2:]			// value flag followed directly by value
					Sflag = true
					break
				}
			}
		}
		if Sflag {
			*argsP = append(*argsP, arg)	// append a value after a value flag
		}
	}
} // convertFlags

/***********************************************************************
 *
 *  Push FOR loop control variable and state2 on to the ctrlStateStack
 *
 ***********************************************************************/

type ctrlState struct {
	ctrl string				// name of FOR loopcontrol variable or "IF or "ELSE"
	stat rune				// save state2
	ifStat rune				// state2 of initial IF (not changed by ELSE IF)
}

var ctrlStateStack = make([]ctrlState, 5)
var CtrlStateStkp int

func PushCtrlState(Fctrl string, stat2 rune, ct string) {
	if CtrlStateStkp >= cap(ctrlStateStack) {
		ctrlStateStack = append(ctrlStateStack, make([]ctrlState, CtrlStateStkp)...)	// double capacity
		if *opt_Z { fmt.Fprintf(genFile, "//#        cap(ctrlStateStack) = %d in PushCtrlState\n", cap(ctrlStateStack)) }
	}
	ctrlStateStack[CtrlStateStkp].ctrl = Fctrl
	ctrlStateStack[CtrlStateStkp].stat = stat2
	if *opt_Z { fmt.Fprintf(genFile, "//# Push %d %q '%c' %q\n", CtrlStateStkp, ctrlStateStack[CtrlStateStkp].ctrl, ctrlStateStack[CtrlStateStkp].stat, ct) }
	CtrlStateStkp++
} // PushCtrlState

/***********************************************************************
 *
 *  Pop FOR loop control variable and state2 from the ctrlStateStack
 *
 ***********************************************************************/

func PopCtrlState(ct string) (string, rune) {
	CtrlStateStkp--
	if CtrlStateStkp < 0 {
		pushError(&ret, "//* Error: Too many Pop's from ctrlStateStack\n")
		CtrlStateStkp = 0
	}
	if *opt_Z { fmt.Fprintf(genFile, "//# Pop  %d %q '%c' %q\n", CtrlStateStkp, ctrlStateStack[CtrlStateStkp].ctrl, ctrlStateStack[CtrlStateStkp].stat, ct) }
	return ctrlStateStack[CtrlStateStkp].ctrl, ctrlStateStack[CtrlStateStkp].stat
} // PopCtrlState

/***********************************************************************
 *
 *  Save state2 for initial IF
 *
 ***********************************************************************/

func Save_ifStat(stat2 rune, ct string) {
	if CtrlStateStkp >= cap(ctrlStateStack) {
		ctrlStateStack = append(ctrlStateStack, make([]ctrlState, CtrlStateStkp)...)	// double capacity
		if *opt_Z { fmt.Fprintf(genFile, "//#        cap(ctrlStateStack) = %d in Save_ifStat\n", cap(ctrlStateStack)) }
	}
	ctrlStateStack[CtrlStateStkp].ifStat = stat2
	if *opt_Z { fmt.Fprintf(genFile, "//# Save %d ifStat '%c' %q\n", CtrlStateStkp, ctrlStateStack[CtrlStateStkp].ifStat, ct) }
} // Save_ifStat

/***********************************************************************
 *
 *  Read save2 saved from initial IF
 *
 ***********************************************************************/

func Read_ifStat(ct string) (rune) {
	if *opt_Z { fmt.Fprintf(genFile, "//# Read %d ifStat '%c' %q\n", CtrlStateStkp, ctrlStateStack[CtrlStateStkp].ifStat, ct) }
	return ctrlStateStack[CtrlStateStkp].ifStat
} // Read_ifStat

/***********************************************************************
 *
 *  Select FOR or IF atom in state2 machine
 *
 ***********************************************************************/

func FORorIF(nextState rune, SQflag bool, atom string, pos, pose, prevPose int,
			offsetP, iNextP, FORstartP, dBraceCntP, braceCntP, posNextP *int, state2P *rune,
			textP, textResP, FORctrlP *string, ForIfFlgP, iesFlagP, SQvarP, SQcvarP *bool) bool {
	var newStat rune
	if atom == "}}" || atom == "FOR" || atom == "IF" {
		if atom == "}}" {
			if len(genSlice) > 0 {
				if m := reTxSpaces.FindStringSubmatch(genSlice[0]); len(m) == 3 {
					genSlice[0] = "\""+m[2]+"\""	// reduce leading spaces to 0 or 1
					if *opt_S { fmt.Fprintf(genFile, "//*=a= %d: genSlice = %q m = %q\n", lineNo, genSlice, m) }
					genLine(m[1], "")				// output extra spaces before for statement
				}
			}
		}
		p := prevPose
		if atom == "FOR" && !reAllSpaces.MatchString((*textResP)[:pos]) {
			p = pos
		}
		if p > 0 {
			genText := (*textResP)[:p]
			genText  = reQuoteSlash.ReplaceAllString(genText, "\\$1")	// precede " and \ by extra \
			genSliceAppend("\""+genText+"\"")	// text string before FOR IF or closing braces
			if *opt_S { fmt.Fprintf(genFile, "//*=4= %d: genSlice = %v atom = %q\n", lineNo, genSlice, atom) }
			*textResP = substr(*textResP, 0, p, "")
			*textP = *textResP			// remove text just processed
			*offsetP = 0
			*iNextP = 0;
			return true
		}
		if len(genSlice) > 0 {
			if *opt_Z { fmt.Fprintf(genFile, "//= genSliceOut(%q) '%c' in FORorIF\n", genSlice, *state2P) }
			genSliceOut("")			// output text before "FOR" "IF" "}}"  "}} ELSE"
		}
		*FORstartP = pos
		forLine := (*textResP)[*FORstartP:]
		if *opt_S { fmt.Fprintf(genFile, "//*FOr* %d: '%c' %q ForIfFlg = %v FORctrlVar = %q\n", lineNo, *state2P, forLine, *ForIfFlgP, FORctrlVar) }
		if atom == "}}" {			// not else if because of else below
			if nextState != 'X' {
				/********************************************************************
				 *  Closing double braces "}}" after "FOR (...) {{"
				 *  output directly
				 *******************************************************************/
				(*dBraceCntP)--
				if *dBraceCntP < 0 {
					pushError(&ret, "//* Error: too many closing double braces\n")
					*dBraceCntP = 0
					return false
				} else {
					var fc string
					if FORline != "" {
						fmt.Fprint(genFile, FORline)
						FORline = ""
					}
					if *state2P != 'J' {
						if *opt_a && genFlag && *state2P != 'F' {
							genLine("", "\\n")
						}
						fmt.Fprint(genFile, "  }\n")
						opt_aComment("}}", "")
					}
					fc, *state2P = PopCtrlState(fmt.Sprintf("FORorIF }} '%c'", *state2P))
					if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", *state2P, atom) }
					if *opt_S { fmt.Fprintf(genFile, "//*BrC* %d: '%c' L %v %q\n", lineNo, *state2P, LFflag, (*textResP)[prevPose:pose]) }
					if *opt_X { fmt.Printf("}} dBraceCnt = %d %q\n", *dBraceCntP, *textResP) }
					delete(FORctrlVar, fc)		// ok to delete non-existing "" map entry
					if nextState == 'D' {
						*state2P = 'D'		// only statement to enter state2 = 'D'
						if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q in FORorIF\n", *state2P, atom) }
					}
					if *opt_S { fmt.Fprintf(genFile, "//*Bs2* %d: '%c' L %v %q atom = %q pose = %d\n", lineNo, *state2P, LFflag, *textResP, atom, pose) }
					*textResP = substr(*textResP, 0, pose, "")	// pose must be > 0
					if *opt_S { fmt.Fprintf(genFile, "//*Be2* %d: '%c' %q\n", lineNo, *state2P, *textResP) }
					*textP = *textResP		// remove text just output
					*offsetP = 0
					*iNextP = 0;
					return true
				}
			}
			pushError(&ret, "//* Error: double closing braces }} not after FOR IF or ELSE\n")
		} else {							// FOR or IF
			if *state2P == 'D' {
				if *opt_a && genFlag {
					genLine("", "\\n")
				}
				fmt.Fprint(genFile, "  }\n")
				opt_aComment("}}", "")
				*state2P = Read_ifStat(fmt.Sprintf("FORorIF 'D' with atom = %s", atom))
				if *state2P == 0 { panic("FORorIF 'D'") }
			}
			if atom == "FOR" {
				/********************************************************************
				 *  FOR start
				 *  FORctrl is name of the FOR loop control variable discovered early,
				 *  which is used in map FORctrlVar[FORctrl] to indicate the loop
				 *  control variable is valid until the matching closing braces, when
				 *  the map entry is deleted.
				 *
				 *  The word 'int' just before the FOR loop control variable is ignored.
				 *  The optional usage of 'int' before the FOR loop control variable
				 *  is deprecated and a Warning is issued unless -n.
				 *
				 *  The FORctrlVar[FORctrl] value is type "int" or "string" of the
				 *  loop control variable for use in the loop (initially "int").
				 *  FORctrl, which is set to control variable with FOR,
				 *  to "IF" with IF and "ELSE" with ELSE is pushed to ctrlStateStack
				 *  with previous state2.
				 *
				 *  reFORin1    `^FOR\s*\(\s*((int)\s+)?([a-zA-Z]\w*)\s*=`
				 *  reFORin2    `^FOR\s+((int)\s+)?([a-zA-Z]\w*)\s*\(`
				 *******************************************************************/
				var m []string
				*FORctrlP = ""
				if m = reFORin1.FindStringSubmatch(forLine); len(m) == 4 && m[3] != "" {
					*FORctrlP = m[3]		// FOR (N =
				} else if m = reFORin2.FindStringSubmatch(forLine); len(m) == 4 && m[3] != "" {
					*FORctrlP = m[3]		// FOR N (
				} else {
					pushError(&ret, "//* Error: FOR line syntax error\n")
					return false
				}
				if !*opt_n && m[2] == "int" {
					pushError(&w, fmt.Sprintf("//* Warning: type specifier 'int' before loop control variable '%s' is deprecated\n", *FORctrlP))
				}
				if _, ok := keywords[*FORctrlP]; ok {	// control variable exists as a keyword
					pushError(&ret, "//* Error: FOR line control variable is a C/iC/iCa keyword - not allowed\n")
					return false
				}
				if *FORctrlP != "" && !*ForIfFlgP {
					if _, ok := FORctrlVar[*FORctrlP]; ok {	// exists
						pushError(&ret, "//* Error: FOR control variable already set in another loop\n")
						return false
					}
					FORctrlVar[*FORctrlP] = "int"	// must be set in ForIfFlg and SQflag variables
					*ForIfFlgP = true				// allows Rescan2 without exists check
				}
				newStat = 'B'
			} else if atom == "IF" {
				/********************************************************************
				 *  IF start
				 *  FORctrl is set "IF", because it cannot be a control variable in
				 *  the iCa language (is keyword)
				 *******************************************************************/
				*FORctrlP = "IF"
				if *opt_S { fmt.Fprintf(genFile, "//*If** %d: '%c' %q\n", lineNo, *state2P, (*textResP)[*FORstartP:]) }
				*ForIfFlgP = true
				Save_ifStat(*state2P, "FORorIF IF")		// save state2 for initial IF - not modified by other atoms especially ELSE IF
				newStat = 'G'
			}
			if *state2P == 'D' {
				*state2P = 'A'		// previous IF has terminated with "}}" and not ELSE
			}
			PushCtrlState(*FORctrlP, *state2P, fmt.Sprintf("FORorIF %s", atom))
			*state2P = newStat
			if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", *state2P, atom) }
		}
	} else if atom == "ELSE" {
		pushError(&ret, "//* Error: ELSE not after IF\n")
	} else if atom == "{{" {
		pushError(&ret, "//* Error: double opening braces \"{{\" not after FOR IF or ELSE\n")
	} else if atom == "{" {
		if *braceCntP >= cap(iesBrace) {
			iesBrace = append(iesBrace, make([]bool, *braceCntP)...)	// double capacity
		}
		if *iesFlagP {
			iCmode = false			// C code
		}
		iesBrace[*braceCntP] = *iesFlagP
		if *opt_X { fmt.Printf("{{ braceCnt = %d iesBrace = %v %q\n", *braceCntP, iesBrace, *textResP) }
		*iesFlagP = false
		*braceCntP++
	} else if atom == "}" {
		*braceCntP--
			if *braceCntP < 0 {
				pushError(&ret, "//* Error: too many closing single braces\n")
				*braceCntP = 0
			} else if iesBrace[*braceCntP] {
				iCmode   = true		// iC code
			}
	} else if reWord.MatchString(atom) {
		if iCmode {
			if (keywords[atom] & 0x400) != 0 {	// if else switch
				*iesFlagP = true
			} else if (keywords[atom] & 0x20f) == 0x200 {
				pushError(&ret, fmt.Sprintf("//* Error: C keyword %q not valid in iC code\n", atom))
			}
		}
		if SQflag {
			if _, ok := FORctrlVar[atom]; ok {
				*SQvarP = true		// square bracket contains at least one valid control variable - evaluate
				*SQcvarP = true		// check for a valid control variable in C code
			} else if iCmode {
				pushError(&w, fmt.Sprintf("//* Warning: %q not a valid control variable in square brackets in iC code\n", atom))
			}
		}
		*posNextP = 0
	} else if reNum.MatchString(atom) {
		if SQflag {
			*SQvarP = true		// square bracket contains at least one number - evaluate
		}
	} else if atom == "%{" {
		if iCmode {
			litBlock = true
			iCmode   = false		// C code
		} else {
			pushError(&ret, "//* Error: cannot open literal block while in C code\n")
		}
	} else if atom == "%}" {
		if litBlock {
			litBlock = false
			iCmode   = true			// iC code
		} else {
			pushError(&ret, "//* Error: cannot close literal block when not opened\n")
		}
	}
	return false
} // FORorIF

/********************************************************************
 *
 *      Main program
 *
 *******************************************************************/

func main() {
	rePath := regexp.MustCompile(fmt.Sprintf(".*%c", os.PathSeparator))
	named = rePath.ReplaceAllLiteralString(os.Args[0], "")
	flag1 = flag.NewFlagSet(named, flag.ExitOnError)
	argIn = "stdin" // default for STDIN
	var genName string
	log.SetFlags(log.Lshortfile)
	opt_o = flag1.String("o", "", "name of generated output file (default stdout)")
	opt_l = flag1.String("l", "", "name of generated intermediate GO log file (normally deleted)")
	opt_e = flag1.String("e", "", "name of generated error file (default stderr)")
	opt_t = flag1.Bool("t", false, fmt.Sprintf(`test if source is an iCa language file - return 1 if it is

<in.ica>   iCa source file with array definitions (default: stdin)
		   Only processes %%%%include %%%%define %%%%if etc directives when
		   called without -m, -M or -Y options
<in.ic> iC source file - only %%include %%define %%if etc directives are
		   processed unless called with -t option (same as -M option)
		   No iCa language translation (#directives are left untouched)

Pre-compiler to convert "immediate C" source files containing arrays
and object like or function like macros into straight "immediate C".

Copyright (C) 2022	John E. Wulff	 <immediateC@gmail.com>
%q`, ID_immag_go))
	opt_m = flag1.Bool("m", false, fmt.Sprintf(`process only #include #define #if etc directives for C files
No iCa language translation (%s -m replaces cpp)
Warning if %%include %%define %%if etc directives are used`, named))
	opt_M = flag1.Bool("M", false, `process only %include %define %if etc directives for iC files.
No iCa language translation (#directives are left untouched)`)
	opt_Y = flag1.Bool("Y", false, `process only %if directives - enables conditional compiling
for yacc, bison or flex; %define directives are left untouched`)
	opt_a = flag1.Bool("a", false, "output FOR IF ELSE and %%define etc lines as comments")
	opt_n = flag1.Bool("n", false, "no warnings")
	opt_N = flag1.Bool("N", false, "allow 'int' before FOR loop control variable without Warning")
	flag1.Var(&opt_I, "I", `scan %include and #include "files" on another path
May be used multiple times`)
	flag1.Var(&opt_P, "P", `predefine a <macro> for iCa translation
<macro> is usually defined with a numeric value eg. -P m=8
NOTE: command line definitions have precedence over
	 '%%define m 4' in the code (opposite to cpp)
May be used multiple times`)
	flag1.Var(&opt_D, "D", `predefine a <macro> evaluated for C, iC or yacc/bison translation
with -m, -M, -Y or <in.ic>. May be used multiple times`)
	flag1.Var(&opt_U, "U", `undefine <macro> evaluated only with -m, -M, -Y or <in.ic>
May be used multiple times`)
	opt_S = flag1.Bool("S", false, "trace iCa State Machine")
	opt_T = flag1.Bool("T", false, "extra Trace output")
	opt_X = flag1.Bool("X", false, "EXTRA Trace output")
	opt_Z = flag1.Bool("Z", false, "EXTRA Trace output for pre-processor states")

	argsP = &args             // intially fill []args
	convertFlags(&os.Args, 1) // convert POSIX flags to individual flags
	flag1.Parse(args)
	path = os.Getenv("INCLUDE")
	if path == "" {
		path = "/usr/local/include"
	}
	for _, p := range opt_I {
		path += ":" + p // include path options from -I option
	}
	path += ":." // finally add current directory

	var sFlag bool
	if !*opt_N {
		sFlag = true			// default is strict option - no longer any way to change that in iCa code
	}

	var inFile  *os.File
	var inFileC *os.File
	var scanner *bufio.Scanner
	var inFileStk = make([]*fileStk, 0)
	if len(flag1.Args()) == 0 {
		inFile  = os.Stdin
	} else if len(flag1.Args()) == 1 {
		var err error
		argIn = flag1.Arg(0)
		if inFile , err = os.Open(argIn); err != nil {
			log.Print(err); os.Exit(3)
		}
		defer inFile .Close()
	} else {
		log.Println("Too many input file arguments (only one allowed)", flag1.Args()); os.Exit(3)
	}
	inFileC = inFile		// current input file
	reFileName := regexp.MustCompile(fmt.Sprintf("^((([.A-Z_a-z][\\w-]*|\\.*)%c)*)([.A-Z_a-z][\\w-]*)(\\.(\\w*))+$", os.PathSeparator))
	m := reFileName.FindStringSubmatch(argIn)
	if len(m) != 7 {
		log.Printf("badly formed file name %q\n", argIn); os.Exit(3) // unlikely after succesful open
	}
	pathIn := m[1]
	baseIn := m[4]
	extnIn := m[6]
	if *opt_T {
		fmt.Printf(" pathIn = %q baseIn = %q extnIn = %q\n", pathIn, baseIn, extnIn)
	}
	statIn, err := os.Stat(argIn)
	if argIn != "stdin" && err != nil {
		log.Print(err); os.Exit(3) // does not apply to os.Stat("stdin")
	}
	*opt_n = true			// temporarily block warnings
	e := 3					// error exit for normal call
	iCmode = true			// -M or none (iCa) iC mode

	if *opt_t {
		*opt_o = ""
		*opt_e = ""
		*opt_l = ""
		*opt_m = false
		*opt_M = false
		*opt_Y = false
		e      = 0			// error exit for -t call
	} else {
		if *opt_m {
			*opt_M = false	// opt_m has precedence over opt_M and opt_Y
			*opt_Y = false
			iCmode = false	// -m C mode
		} else if *opt_M {
			*opt_Y = false	// opt_M has precedence over opt_Y
		} else if *opt_Y {
			iCmode = false	// -Y C mode
		}
		genName = fmt.Sprintf("%s.go", baseIn)
		statGen, err := os.Stat(genName)
		if err == nil {
			if os.SameFile(statIn, statGen) {
				log.Printf("output file %q should not be the same as generated GO file %q\n", genName, argIn); os.Exit(3)
			}
			if err := os.Chmod(genName, 0644); err != nil {
				log.Print(err); os.Exit(3)
			}
		}
		genFile, err = os.Create(genName)
		if err != nil {
			log.Print(err); os.Exit(3)
		}
		if *opt_l != genName && *opt_l != "" {
			if _, err := os.Stat(*opt_l); !errors.Is(err, os.ErrNotExist) {
				err = os.Remove(*opt_l)		// remove existing log file (required by Link())
				if err != nil {
					log.Print(err); os.Exit(3)
				}
			}
			err = os.Link(genName, *opt_l)	// hard link log file to generated GO file
			if err != nil {					// log file remains when GO file is deleted
				log.Print(err); os.Exit(3)
			}
		}
		fmt.Fprint(genFile,
`package main
import "fmt"
var IF bool
var IFprev bool
var IFblock int
var IFstack = make([]bool, 0, 4)
func main() {
 IF = true
 IFstack = append(IFstack, IF)
 IFprev = true
 if true {
  _ = fmt.Sprint()
`)						// balance } }
		if *opt_o != "" { // keep iC output file if Log file is in error
			statOut, err := os.Stat(*opt_o)
			if err == nil {
				if os.SameFile(statIn, statOut) {
					log.Printf("output file %q should not be the same as input file %q\n", *opt_o, argIn); os.Exit(3)
				}
				if err := os.Chmod(*opt_o, 0644); err != nil {
					log.Print(err); os.Exit(3)
				}
			}
			outFile, err := os.Create(*opt_o)
			if err != nil {
				log.Print(err); os.Exit(3)
			}
			defer outFile.Close()
			os.Stdout = outFile
		}
		if *opt_e != "" { // must be last so STDERR remains open
			statErr, err := os.Stat(*opt_e)
			if err == nil {
				if os.SameFile(statIn, statErr) {
					log.Printf("error file %q should not be the same as input file %q\n", *opt_e, argIn); os.Exit(3)
				}
				if err := os.Chmod(*opt_e, 0644); err != nil {
					log.Print(err); os.Exit(3)
				}
			}
			errFile, err := os.Create(*opt_e)
			if err != nil {
				log.Print(err); os.Exit(3)
			}
			defer errFile.Close()
			os.Stderr = errFile
		}
	}
	opt_m_M_Y = *opt_m || *opt_M || *opt_Y
	if *opt_T && *opt_l == "" {
		fmt.Println(" *opt_m =", *opt_m, "*opt_M =", *opt_M, "*opt_Y =", *opt_Y, "opt_m_M_Y =", opt_m_M_Y)
	}

	// balance { {
	reState     := regexp.MustCompile(`(%?\{\{?|%?}}?|%?#|%%?|/[/*]|\*+/?|\w+|(\\\\)+|\\n|\\t|\\?"|\\?'|\\?\[|\\?\]|\S)`)
	reCommEnd    = regexp.MustCompile(`\*+/`)
	reCommaSemi := regexp.MustCompile(`,(\s*);`)
	reQuoteSlash = regexp.MustCompile(`(["\\])`)
	var reCdirective *regexp.Regexp
	if *opt_m { // both %%directives and %#directives are excluded - #define and %define (warning)
		reCdirective = regexp.MustCompile(`^\s*%[%#]\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
	} else if *opt_M { // %%directives, #directives and %#directives are excluded - %define %if only
		reCdirective = regexp.MustCompile(`^\s*(%%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
	} else if *opt_Y { // additionally %define %undef %include are excluded - %define only (no %if)
		reCdirective = regexp.MustCompile(`^\s*((%%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)|%(define|undef|include))\b`)
	} else { // %directives, #directives and %# directives are excluded - only %%directives
		reCdirective = regexp.MustCompile(`^\s*(%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
	}
	reLdQuote   := regexp.MustCompile(`^\s*'?`)
	reTrQuote   := regexp.MustCompile(`'?\s*$`)
	reWord       = regexp.MustCompile(`^[A-Z_a-z]\w*$`)
	reNumWord   := regexp.MustCompile(`^\w+$`)
	reDefined    = regexp.MustCompile(`(!\s*)?defined\s*`)
	reNum        = regexp.MustCompile(`^(0|[1-9]\d*)$`)
	reSignNum    = regexp.MustCompile(`^[+-]?\s*(0|[1-9]\d*)$`)
	reFmtSprint := regexp.MustCompile(`^fmt\.Sprint\(`)
	reStrInd    := regexp.MustCompile(`\\"([^\\]*), "(\\")?`)
	reConstExpr  = regexp.MustCompile(`(^\s*\d+\s*$|[^\s()\d*\/%+-])`)
	reEvalIf     = regexp.MustCompile(`(!\s*defined|[a-z_A-Z]\w*|([+-]\s*)?\d+|&&|\|\||[\?:\(\)]|[=!<>]=|<<?|>>?)`)
	reLdSpaces  := regexp.MustCompile(`^\s*`)
	reLdTrSps    = regexp.MustCompile(`^\s+|\s+$`)
	reTrSpaces   = regexp.MustCompile(`\s+$`)
	reTrSpaceSl := regexp.MustCompile(`\s+\\$`)
	reSpaces     = regexp.MustCompile(`(\s+)`)
	reTxSpaces   = regexp.MustCompile(`^\"(\s+)(\s.*)\"$`)
	reAllSpaces  = regexp.MustCompile(`^\s*$`)
	reSpacexdot := regexp.MustCompile(`^\s*([x.])\s*$`)
	reWordSpace := regexp.MustCompile(`([IQ]X\d\.|[A-Z_a-z]\w*|^)\s+$`)
	reWordNum   := regexp.MustCompile(`([A-Z_a-z]\w*\d)\s*$`)
	reCcomment   = regexp.MustCompile(`\s*/\*.*`)
	reCcomTail   = regexp.MustCompile(`\*/\s*`)
	reMacro      = regexp.MustCompile(`^([A-Z_a-z]\w*)(\(\s*(([A-Z_a-z]\w*)(\s*,\s*([A-Z_a-z]\w*))*)*\s*\))?(\s+(.+))?$`)
	reTrComma   := regexp.MustCompile(`\s*,\s*$`)
	reComma      = regexp.MustCompile(`\s*,\s*`)
	reHashHash   = regexp.MustCompile(`\s*##\s*`)
	reHashSlash  = regexp.MustCompile(`#\\`)
	reHash       = regexp.MustCompile(`#([^\d\\]|$)`)
	reTranslate  = regexp.MustCompile(`([A-Z_a-z]\w*|\/[*\/]|\*\/|\\?["'])`)
	reResolve    = regexp.MustCompile(`(\(|[A-Z_a-z]\w*|,|\)|\\?["'])`)
	reTrue       = regexp.MustCompile(`<< >>`)
	reFalse      = regexp.MustCompile(`>> <<`)
	reInclude   := regexp.MustCompile(`^["<](([A-Za-z]:)?([/\\]?)[A-Z_a-z.][\w/\\.-]*)[">]$`)
	reErrWarn   := regexp.MustCompile(`^(error|warning)$`)
	reLine      := regexp.MustCompile(`^\s*(\d+)\s+"([A-Z_a-z][\w.]*)"`)
	reFORin1     = regexp.MustCompile(`^FOR\s*\(\s*((int)\s+)?([a-zA-Z]\w*)\s*=`)
	reFORin2     = regexp.MustCompile(`^FOR\s+((int)\s+)?([a-zA-Z]\w*)\s*\(`)
	reFORstd    := regexp.MustCompile(`^FOR\s*\(\s*((int)\s+)?([a-zA-Z]\w*)\s*=(.*)\)\s*{{$`)		// }}
	reFORdot    := regexp.MustCompile(`^FOR\s+((int)\s+)?([a-zA-Z]\w*)\s*\(\s*(\w+(\s*([*%/&^|()+-]|<<|>>)\s*\w+)*)\s*\.\.\s*(\w+(\s*([*%/&^|()+-]|<<|>>)\s*\w+)*)\s*\)\s*{{$`)	// }}
	reFORlist   := regexp.MustCompile(`^FOR\s+((int)\s+)?([a-zA-Z]\w*)\s*\(\s*(.*\S)\s*\)\s*{{$`)	// }}
	reIFstd     := regexp.MustCompile(`^(ELSE\s+)?IF\s*\(\s*(.*\S)\s*\)\s*{{$`)			// }}
	reString    := regexp.MustCompile(`^".*"$`)
	reBareWd    := regexp.MustCompile(`^\S*$`)
	rePercent    = regexp.MustCompile(`%`)
	reDivInt     = regexp.MustCompile(`((\([^)]+\)|\d+)\s*/\s*(\([^)]+\)|\d+))`)
	Cdirective  := false
	state       := 'A'		// starting state
	si          := 1		// initial value of sense indicator; output %else or %elif not allowed
	prefixes := map[string]bool {
		"%%":	true,
		"%":	true,
		"#":	true,
		"%#":	true,
	}
	direcves := map[string]bool {
		"define":	true,
		"undef":	true,
		"include":	true,
		"ifdef":	true,
		"ifndef":	true,
		"if":		true,
		"elif":		true,
		"else":		true,
		"endif":	true,
		"error":	true,
		"warning":	true,
		"line":		true,
	}
	FORctrlVar["int"] = "int"
	direcve    := ""
	macro      := ""
	macroStart := 0
	mWarn      := false
	clFlag     := false
	comStart   := 0
	expFlag    := false
	ternFlag   := false
	if len(opt_P) > 0 {
		for _, p := range opt_P {
			p = reLdQuote.ReplaceAllLiteralString(p, "")	// delete leading blanks and "'" inserted by iCmake
			p = reTrQuote.ReplaceAllLiteralString(p, "")	// delete trailing "'" inserted by iCmake and blanks
			s := regexp.MustCompile(`\s*=\s*`).Split(p, 2)	// space separated macro is also accepted
			macro := s[0]
			if len(s) > 1 {
				macro += " " + s[1]							 // when translation is missing just %%define X
			}
			df, val := scan_define("-P", macro)				// scan and save -P macro in  map defs
			if len(lnErr) > 0 {
				outputError("-P "+p+"\n")
			} else {
				if df == val {
					pushError(&w, "//* Warning: '-P macro' does not change anything ???\n")
					outputError("-P "+p+"\n")
					delete(defs, df)
				} else if _, ok := defs[df]; ok {			// should never fail if scan_define() correct
					clDefs[df] = defs[df]
					if *opt_l != "" {
						fmt.Fprintf(genFile, "//# COMMAND LINE -P %s\n", p)
					}
					if *opt_a {
						fmt.Fprintf(os.Stdout, "//***** -P %s\n", p)
						clFlag = true
					}
				}
			}
			if ret > 0 {
				goto CloseFiles // found Error  # proceed with Warning
			}
		}
	}
	if opt_m_M_Y { // -D -U only processed for <in>.ic iC files or if -m -M or -Y
		if len(opt_D) > 0 {
			for _, p := range opt_D {
				p = reLdQuote.ReplaceAllLiteralString(p, "")	// delete leading blanks and "'" inserted by iCmake
				p = reTrQuote.ReplaceAllLiteralString(p, "")	// delete trailing "'" inserted by iCmake and blanks
				s := regexp.MustCompile(`\s*=\s*`).Split(p, 2)	// space separated macro is also accepted
				macro := s[0]
				if len(s) > 1 {
					macro += " " + s[1] // when translation is missing just %%define X
				}
				df, val := scan_define("-D", macro) // scan and save -D macro in  map defs
				if len(lnErr) > 0 {
					outputError("-D "+p+"\n")
				} else {
					if df == val {
						pushError(&w, "//* Warning: '-D macro' does not change anything ???\n")
						outputError("-D "+p+"\n")
						delete(defs, df)
					} else if _, ok := defs[df]; ok {			// should never fail if scan_define() correct
						clDefs[df] = defs[df]
						if *opt_l != "" {
							fmt.Fprintf(genFile, "//# COMMAND LINE -D %s\n", p)
						}
						if *opt_a {
							fmt.Fprintf(os.Stdout, "//***** -D %s\n", p)
							clFlag = true
						}
					}
				}
				if ret > 0 {
					goto CloseFiles // found Error  # proceed with Warning
				}
			}
		}
		if len(opt_U) > 0 { // for compatibility with cpp for generated command lines
			for _, df := range opt_U {
				df = reLdQuote.ReplaceAllLiteralString(df, "")	// delete leading blanks and "'" inserted by iCmake
				df = reTrQuote.ReplaceAllLiteralString(df, "")	// delete trailing "'" inserted by iCmake and blanks
				//				$df =~ s/'?\s*$//;		// delete trailing "'" inserted by iCmake and blanks
				if reWord.MatchString(df) {
					if *opt_l != "" {
						fmt.Fprintf(genFile, "//# COMMAND LINE -U %s\n", df)
					}
					if *opt_a {
						fmt.Fprintf(os.Stdout, "//***** -U %s\n", df)
						clFlag = true
					}
					delete(defs, df)		// safe to delete even if hash entry does not exist
					delete(clDefs, df)
				} else {
					pushError(&w, "//* Warning: '-U macro' has bad characters - ignored\n")
					outputError("-U "+df+"\n")
				}
			}
		}
	}
	if clFlag {
		fmt.Printf("# 1 %q\n", argIn)
	}

	/***********************************************************************
	 *
	 *  Process the file
	 *
	 ***********************************************************************/

	if *opt_l != "" {
		fmt.Fprintln(genFile, "//###", named, argIn)
	}
	/***********************************************************************
	 *  Read all lines in the input file
	 ***********************************************************************/
	scanner = bufio.NewScanner(inFileC)
	scanner.Split(bufio.ScanLines)
  Level:
	for {
		offset   := 0
		iNext    := 0
		comiNext := 0
		ForIfFlg := false
		FORcomm  := false
		FORend   := 0
		SQstSv   := make([]rune, 0, 5)
		SQbrCnt  := 0
		SQbrFlag := false
		SQbrInr  := make([]int, 0, 5)
		cat      := 0				// 0 no; 0x1 yes with offset 1; 0x2 yes with offset 0; 0x4 comment with offset 0
		prefix   := ""
		text     := ""
		textRes  := ""
		braceCnt := 0
		iesFlag  := false
		dBraceCnt := 0
		state2   := 'A'				// start scan 2 with same state as scan 1
		genTail  := ""
		if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c'\n", state2) }
	  Line:
		for scanner.Scan() {
			LFflag = false
			if cat == 0 {
				iNext = 0
			}
			if iNext == 0 && len(lnErr) > 0 && !*opt_t {
				outputError(lineText)			// possible error in previous line
			}
			if hashLineFlag {
				genLine(fmt.Sprintf("# %d %q 2", lineNo+1, argIn), "\\n")	// # 8 "file" 2
				hashLineFlag = false
			}
			prevText := text
			text = scanner.Text()								// `\r?\n` is stripped from returned text
			text = reTrSpaces.ReplaceAllLiteralString(text, "")	// remove trailing white space
			lineNo++											// first line is lineNo 1
			if cat != 0 {
				ofs := cat & 0x01
				tx := text[reLdSpaces.FindStringIndex(text)[1]:]	// loc[1] is 0 if no leading spaces in text
				if !reTrSpaceSl.MatchString(textRes) {
					tx = " "+tx					// 1 space between concatenated lines
				}
				textRes = textRes[:len(textRes)-ofs] + tx		// concatenate text to previous textRes less \ if 1
				lineText = lineText[:len(lineText)-ofs] + tx	// insert one space
				text = prevText[:len(prevText)-ofs] + tx		// concatenate text to previous textRes less \ if 1
				if ofs != 0 {
					cat &^= 0x1					// stop cat == 0x1 --> 0; retain cat == 0x2 or 0x4
				}
			} else {
				textRes  = text
				lineText = text					// unmodified input text for error messages
				offset   = 0
				ForIfFlg = false
				FORcomm  = false
				FORend   = 0
				SQstSv   = make([]rune, 0, 5)	// reset square bracket stack at start of a new line
				SQbrCnt  = 0
				SQbrFlag = false
				SQbrInr  = make([]int, 0, 5)
				SQprCnt  = 0
			}
			if *opt_X { fmt.Printf(">1 %d:\t%q\n", lineNo, text) }
			l    := len(text)
			last := byte(0)
			if l > 0 {
				last  = text[l-1]				// could be trailing backslash
			}
			var pos, pose, prevPos, prevPose, expPos int
			var prevAtom, expAtom string
			if iNext == 0 {
				prefix   = ""
			}
			atom     := ""
			posNext  := 0
			if *opt_S { fmt.Fprintf(genFile, "//***** %d: '%c' %q\n", lineNo, state, text) }
		  Rescan:
			for {
				if *opt_S { fmt.Fprintf(genFile, "//*Rs1* %d: '%c' %q\n", lineNo, state, text) }
				pose     = 0					// balance {{
				/***********************************************************************
				 *  Process lines with a state machine, which recognises comments,
				 *  parenthesised strings, quoted characters as well as relevant atoms.
				 *
				 *  States are:
				 *  'A' inside iCa, iC or C code
				 *  'C' inside a C comment
				 *  'P' inside a C++ comment
				 *  'S' inside a parenthesised string
				 *  'H' inside a single quoted character constant
				 *
				 *  text     is the unmodified text that is scanned by the state machine
				 *  textRes  is the text modified by resolved macros and other changes
				 *  lineText is the really unmodified text for error messages
				 *
				 *  This processing is done before recognising pre-processor lines #if etc,
				 *  because some system headers have pre-processor lines in a C comment block
				 *  eg: #if in /usr/include/sys/cdefs.h line 84 and line 137
				 *      #include in /usr/include/limits.h line 40
				 *
				 *  Collect lines for the first part of a FOR control statement up to the
				 *  opening braces and present the complete line for further processing.
				 *  This needs to be done here to remove comments between FOR and opening
				 *  double braces and to present a clean FOR control line for output as a
				 *  comment.
				 *  If there is a second FOR, after the opening braces on the same line,
				 *  stay in this loop and collect lines up to the next opening braces also.
				 *  Comments in the FOR control line and just after any double braces are
				 *  deleted. A FOR in a comment or string is ignored (this is very important).
				 *
				 *  The same applies for IF ELSE control statements.
				 *
				 *  NOTE: no brace counting is necessary - only look for next opening double
				 *  brace after a FOR IF ELSE or ELSE IF.
				 ***********************************************************************/
				//  reState:`(%?\{\{?|%?}}?|%?#|%%?|/[/*]|\*+/?|\w+|(\\\\)+|\\n|\\t|\\?"|\\?'|\\?\[|\\?\]|\S)`
				for i, posPair := range reState.FindAllStringIndex(text, -1) {
					//	(							balance {{
					//		%?\{\{?	|	# { {{ %{ - not used %{{
					//		%?}}?	|	# } }} %} - not used %}}
					//		%?#		|	# directive prefix # %#
					//		%%?		|	# directive prefix % %%
					//		/[/*]	|	# /* // start of C or C++ comment
					//		\*+/?	|	# */ **/ etc   end of C comment or * **
					//		\w+		|	# word identifier or number
					//		(\\\\)+	|	# remove pairs of backslashes
					//		\\n		|	# char \n NL in string
					//		\\t		|	# char \t TAB in string
					//		\\?"	|	# " or \"
					//		\\?'	|	# ' or \'
					//		\\?\[	|	# [ or \[   - used in iCa analysis only
					//		\\?\]	|	# ] or \]   - used in iCa analysis only
					//		\S			# any single non space character
					//	)
					prevPose = pose						// start of white space after previous atom
					pose  = posPair[1]
					if i < iNext {
						continue						// skip tokens in text already scanned
					}
					iNext = i							// save token count in case concatenated
					pos = posPair[0]
					posOff := pos + offset
					atom = text[ pos : pose ]
					if *opt_X { fmt.Printf("%2d\t%3d %3d\t%q\n", i, pos, pose, atom) }
					if last == '\\' {			// line with trailing backslash \
						cat |= 0x1				// concatenate next line to current line less \
						if *opt_X { fmt.Printf("*** skip with cat = %#x\n", cat) }
						blankLines++
						continue Line			// build up full line early
					}
					/********************************************************************
					 *  Scan 1 square bracket handling in iCa mode
					 *
					 *  A square bracket in a FOR IF ELSE control line (while ForIfFlg is true)
					 *  is an error. (This allows cat = 0x2 to be used for unmached brackets)
					 *
					 *  In iCa mode inner square brackets are evaluated in generated code.
					 *  This is done even if a square bracket expression is contained in
					 *  a comment, string or character constant.
					 *
					 *  Push state on SQstSv stack for each opening square bracket.
					 *  The state inside the square bracket is changed to 'A'.
					 *  Pop state from SQstSv stack for each closing square bracket.
					 *  Stack handling is necessary, because because states must be
					 *  restored in the inverserse order they are stored.
					 *  This will allow nesting to any depth, which is really over-kill.
					 *  The last matching square bracket restores the previous state.
					 *  The square bracket stack SQstSv is reset for every new line
					 *  after extensions of extra lines have been added by concatenation.
					 *
					 *  A C or C++ comment inside any square bracket pair is removed,
					 *  even if they are nested.
					 *
					 *  If there are unmatched square brackets missing at the end of a line
					 *  the line is extended until square brackets are matched. (cat = 0x2).
					 *  This may lead to very long (erroneous) lines if brackes are missing.
					 *
					 *  An added benefit is that FOR loop control variables modified by
					 *  a macro will be translated correctly in comments.
					 *******************************************************************/
					if !opt_m_M_Y {
						if atom == "[" {
							if ForIfFlg && state != 'S' {
								pushError(&ret, "//* Error: square bracket \"[\" not allowed in FOR control line\n")
							}
							SQstSv = append(SQstSv, state)	// push current state
							state = 'A'
							SQbrCnt++
							SQbrFlag = true
							cat = 0x2					// concatenate next line unless terminated with matching brackets
							if *opt_X { fmt.Printf("%d: SqSt1 cat = %#x SQbrCnt = %d state = %c SQstSv = %c %q\n", lineNo, cat, SQbrCnt, state, SQstSv, textRes) }
						} else if atom == "]" {
							var l int
							SQbrCnt--
							if SQbrCnt < 0 {
								SQbrCnt = 0
								pushError(&ret, "//* Error: too many closing square brackets\n")
							} else {
								if SQbrFlag {
									SQbrInr = append(SQbrInr, SQbrCnt)	// marks an inner nestesd square bracket pair
									SQbrFlag = false
								}
								if SQbrCnt == 0 {
									cat &^= 0x2				// final closing square bracket found
								}
								l = len(SQstSv) - 1
								if l >= 0 {
									state = SQstSv[l]		// pop saved state
									SQstSv = SQstSv[:l]		// TODO check if stack is needed
								}
								if ForIfFlg && state != 'S' {
									pushError(&ret, "//* Error: square bracket \"]\" not allowed in FOR control line\n")
								}
							}
							if *opt_X { fmt.Printf("%d: SqEn1 cat = %#x SQbrCnt = %d state = %c SQstSv = %c l = %d %q\n", lineNo, cat, SQbrCnt, state, SQstSv, l, textRes) }
						}
					}
					if state == 'A' {					// program - not comment, string, char constant
						if i == 0 {
							/***********************************************************************
							 *  If -m then both %%directives and %#directives are excluded - #define and %define (warning)
							 *      reCdirective = regexp.MustCompile(`^\s*%[%#]\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
							 *  Else if -M %%directives, #directives and %#directives are excluded - %define %if only
							 *      reCdirective = regexp.MustCompile(`^\s*(%%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
							 *  Else if -Y additionally %define %undef %include are excluded - %define only (no %if)
							 *      reCdirective = regexp.MustCompile(`^\s*((%%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)|%(define|undef|include))\b`)
							 *  Else if none of the above %directives, #directives and %# directives are excluded - only %%directives
							 *      reCdirective = regexp.MustCompile(`^\s*(%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
							 *
							 *  These regular expressions exclude directives which are not resolved for the chosen language
							 *  Output these directives completely unchanged with possible trailing \
							 *  except if blocked by #if #else or #elif
							 *
							 *  NOTE: %{ #include <stdio.h> %} may only occur in iC or iCa code (option -M or none).
							 *        Thus the C code #include <stdio.h> or other # directives are passed unchanged
							 *        to the output and do not need to be at the start of the line.
							 *        %{ and %} are processed by the immcc compiler.
							 ***********************************************************************/
							if Cdirective || reCdirective.MatchString(text) {	// match text because of -Y
								if last == '\\' {		// trailing backslash
									Cdirective = true	// #define or %define etc output as is, up to line
								} else {
									Cdirective = false	// without trailing backslash
								}
								if !*opt_t {
									if si <= 0 {
										blankLines++	// count lines not output as blank lines
										continue Line	// do not output lines in false blocks
									}
									if opt_m_M_Y && blankLines > 0 {
										blanks()		// print blank lines now
									}
									genLine(text, "\\n")
								}
								continue Line			// must not modify because of comments
							}
							/***********************************************************************
							 *  All directives not belonging to the current option -m, -M, -Y or none
							 *  have been excluded in the previous section.
							 *  From now on treat %%directives, %directives and #directives equivalently
							 *  except warn if %directive used in straight iC file or in immac -m call
							 *  The prefix and the directive should only be the first two atoms in a line
							 *  After the directive continue scanning for comments.
							 *  The macro with a possible parenthesised argument list starts with the next
							 *  atom, which  has all comments and space before comments deleted.
							 ***********************************************************************/
							if prefixes[atom] {			// %% % # or %#
								prefix = atom
								if *opt_m && prefix == "%" {
									mWarn = true		// output Warning later for -m only
								}
							}
						} else if i == 1 && prefix != "" && direcves[atom] {
							direcve = atom				// directive found
							macroStart = -1				// must get next atom to obtain a macro
							textRes = reSpaces.ReplaceAllLiteralString(textRes, " ")	// compress spaces to 1 space
							textRes = reLdTrSps.ReplaceAllLiteralString(textRes, "")	// remove leading and trailing spaces
							text = textRes
							offset = 0
							iNext = 2					// go directly to i == 2 (avoids infinite loop) 
							continue Rescan
						} else if i == 2 && macroStart == -1 {
							macroStart = posOff			// start of macro - do not resolve this atom (offset should be 0)
						}
						if !opt_m_M_Y {
							if (keywords[atom] & 0x100) != 0 {	// FOR IF ELSE (test before reWord.MatchString)
								if *opt_t {
									os.Exit(1)			// iAc code found
								}
								ForIfFlg = true			// If "ELSE IF" ForIfFlg stays true
								cat |= 0x2				// concatenate next line unless terminated with double braces
								if *opt_X { fmt.Printf("%d: FOR  %q\n", lineNo, textRes) }
								continue
							}
							if atom == "{{" {			// ignore old style single brace
								ForIfFlg = false		// end of FOR IF ELSE control line
								FORend  = 1				// remove comment if it starts with next atom
								cat &^= 0x2				// no need to concatenate since terminated with double braces
								continue
							}
							if atom == "}}" {			// ignore old style single brace
								FORend  = 1				// remove comment if it starts with next atom
								cat &^= 0x2				// no need to concatenate since terminated with double braces
								continue
							}
						}
						if reWord.MatchString(atom) {
							if macroStart == 0 {
								/********************************************************************
								 *  atom   word to be resolved (directives already dealt with)
								 *  pos    starting position of atom in text with offset
								 *
								 *  ok     true if atom is in map defs and was resolved
								 *         else false - do nothing with this word
								 *  rs     true if Rescan required after resolve
								 *
								 *  Do not resolve words in #if etc and other directives
								 *  Words in #if #ifdef etc must remain because of 'defined word'.
								 *  Include file path in #include should certainly not be modified.
								 *******************************************************************/
								if _, _, ok, rs := resolve(atom, pos, &posNext, &offset, 0, &text, &textRes); ok {
									if rs {
										if *opt_X { fmt.Printf("*** %d skip after '... %q ==> %q ...' with posOff = %d posNext = %d\n", lineNo, atom, textRes[posOff:posNext], posOff, posNext) }
										continue Rescan		// re-start scan of same text with token at current position
									}						// which may not be a word
								}
								posNext = 0
							}
						} else if atom == "/*" {
							state = 'C'					// start of C comment found
							comStart = prevPose			// include leading spaces
							comiNext = iNext
							if FORend > 0 {
								FORcomm = true
							}
							if *opt_X { fmt.Printf("state = '%c' comStart = %d offset = %d\n", state, comStart, offset) }
						} else if atom == "//" {
							state = 'P'					// start of C++ comment found
							comStart = prevPose			// include leading spaces
							comiNext = iNext
							if FORend > 0 {
								FORcomm = true
							}
						} else if atom == "\"" {		// leading backslash will not compare
							state = 'S'					// start of string found
							if *opt_X { fmt.Printf("%d: StSt %q	state = %c ForIfFlg = %v\n", lineNo, textRes, state, ForIfFlg) }
						} else if atom == "'" {			// leading backslash will not compare
							state = 'H'					// start of character constant found
						}
					} else if state == 'S' {			// string constant
						if atom == "\"" {				// leading backslash will not compare
							state = 'A'					// end of string constant
							if *opt_X { fmt.Printf("%d: StEn %q	state = %c ForIfFlg = %v\n", lineNo, textRes, state, ForIfFlg) }
						}
					} else if state == 'H' {			// character constant
						if atom == "'" {				// leading backslash will not compare
							state = 'A'					// end of character constant
						}
					} else if state == 'C' {			// C comment
						if reCommEnd.MatchString(atom) {		// == "*/" or "**/ etc
							state = 'A'					// end of C comment
							if direcve != "" || ForIfFlg || FORcomm || SQbrCnt > 0 {
								if FORcomm {
									cat &^= 0x4			// end of multiple line FOR comment concatenation
									FORcomm = false
								}
								al := pose - comStart;	// length of C comment
								textRes = substr(textRes, comStart + offset, al, " ")
								textRes = reTrSpaces.ReplaceAllLiteralString(textRes, "")	// remove trailing white space
								text = textRes			// replace C comment with leading spaces by one space
								offset = 0
								iNext = comiNext
								if *opt_X { fmt.Printf("comStart = %d iNext = %d offset = %d al = %d textRes = %q\n", comStart, iNext, offset, al, textRes) }
								continue Rescan
							}
						} else if FORcomm {
							cat |= 0x4					// finish useless FOR comment block
							continue					// concatenate next line if necessary
						} else if !opt_m_M_Y && macroStart == 0 && reWord.MatchString(atom) {
							/********************************************************************
							 *  Resolve word in comment for iCa translation only
							 *  atom   word to be resolved (directives already dealt with)
							 *  pos    starting position of atom in text with offset
							 *
							 *  ok     true if atom is in map defs and was resolved
							 *         else false - do nothing with this word
							 *  rs     true if Rescan required after resolve
							 *******************************************************************/
							if _, _, ok, rs := resolve(atom, pos, &posNext, &offset, 0, &text, &textRes); ok {
								if rs {
									if *opt_X { fmt.Printf("*** %d skip after '... %q ==> %q ...' with posOff = %d posNext = %d\n", lineNo, atom, textRes[posOff:posNext], posOff, posNext) }
									continue Rescan		// re-start scan of same text with token at current position
								}						// which may not be a word
							}
							posNext = 0
						}
					}
					if FORend > 0 {
						FORend--
					}
				}
				break Rescan
			}	// end of state machine scan of one (possibly concatenated) line
			if state == 'P' {						// C++ comment
				state = 'A'							// end of C++ comment
				if direcve != "" || ForIfFlg || FORcomm || SQbrCnt > 0 {
					FORcomm = false
					al := pose - comStart;			// length of C++ comment
					textRes = substr(textRes, comStart + offset, al, "")
					text = textRes					// remove C++ comment with leading spaces
					offset = 0
					iNext = comiNext
				}
			}
			if atom == "[" {
				iNext++								// skip "[" after concatenation for correct SQbrCnt and SQstSv
			}
			if *opt_S { fmt.Fprintf(genFile, "//*Es1* %d: '%c' %q\n", lineNo, state, text) }
			if *opt_X { fmt.Printf("%d: END  cat = %#x SQbrCnt = %d state = %c atom = %q %q\n", lineNo, cat, SQbrCnt, state, atom, textRes) }
			/***********************************************************************
			 *  End of a line processed by the state machine
			 *  state may be 'A' normal code
			 *            or 'C' C comment
			 *            or 'S' string constant
			 *            or 'H' character constant
			 *  state may never be 'P' C++ comment here
			 *
			 *  If iCa line is scanned again start with state2 and
			 *  check state2 == state at end of scan 2 for that line
			 ***********************************************************************/
			if direcve != "" {
				blankLines++
				/***********************************************************************
				 *  Directive found
				 *  analyse possible macro and store a #define macro in map defs[]
				 *  macrostart cannot be 0 for a directive line
				 ***********************************************************************/
				if macroStart > 0 {
					if macroStart <= len(textRes) {
						macro = textRes[ macroStart : ]
						macro = reLdTrSps.ReplaceAllLiteralString(macro, "")	// remove leading and trailing spaces
					} else {
						macro = ""
					}
					if *opt_X { fmt.Printf("%s%s %s; si = %d\n", prefix, direcve, macro, si) }
				}
				macroStart = 0
				/***********************************************************************
				 *  Scan %define or %undef macro definition as well as other directives
				 ***********************************************************************/
				if mWarn {
					pushError(&w, "//* Warning: %directive in iC file or immac -m - expanded\n")
					mWarn = false
				}
				if *opt_T { fmt.Printf("*0* si = %d %q\n", si, text) }
				if direcve[:2] == "if" {		// %%if, %if or #if...
					/***********************************************************************
					 *  si =  +1  initial value of sense indicator      output    %else or %elif not allowed
					 *  si == +2  #ifdef defined #ifndef !defined #if 1 output    %else or %elif allowed
					 *  si == -1  #ifdef !defined #ifndef defined #if 0 no output %else or %elif allowed
					 *  si == +2  #elif 1 after false   (si < 0)        output    %else or %elif allowed
					 *  si == -1  #elif 0 after false   (si < 0)        no output %else or %elif allowed
					 *  si == -2  #elif X after true    (si > 0)        no output %else or %elif allowed
					 *  si ==  0  #else (no inversion)  if si == -2     no output %else or %elif not allowed
					 *  si == +1  #else after false     if si != -2     output    %else or %elif not allowed
					 *  si ==  0  #else after true                      no output %else or %elif not allowed
					 ***********************************************************************/
					siStk = append(siStk, si)				// push current sense indicator
					if si > 0 {								// actually test sense of previous level
						if *opt_T { fmt.Printf("*1* si = %d direcve = %q macro = %q\n", si, direcve, macro) }
						if direcve == "ifdef" {					// ifdef NAME
							si, _ = evalIf("defined "+macro, DEF, EVAL)	// +2 if NAME defined else -1
							if *opt_X { fmt.Printf("%sifdef %s; si = %d\n", prefix, macro, si) }
						} else if direcve == "ifndef" {			// ifndef NAME
							si, _ = evalIf("!defined "+macro, DEF, EVAL)	// +2 if NAME not defined else -1
							if *opt_X { fmt.Printf("%sifndef %s; si = %d\n", prefix, macro, si) }
						} else if direcve == "if" {				// if EXPR
							si, _ = evalIf(macro, !DEF, EVAL)		// +2 if EXPR true else -1
							if *opt_X { fmt.Printf("%sif %s; si = %d\n", prefix, macro, si) }
						} else {
							pushError(&w, fmt.Sprintf("//* Warning: bad if directive %s%s - ignored\n", prefix, direcve))
						}
						opt_aComment(text, "")
					} else {
						si = -1								// %ifxx seen - allow %else or %elif again at this level
					}
				} else if direcve == "elif" {					// elif EXP
					l := len(siStk) - 1
					if l >= 0 && (si & ^0x1) != 0 {			// eval if previous false 0 or -1 else lasting false -2
						s := siStk[l]
						if s <= 0 || si > 0 || si == -2 {
							si = -2
						} else {
							si, _ = evalIf(macro, !DEF, EVAL)		// +2 elif EXPR true else -1
						}
						if *opt_X { fmt.Printf("%selif %s; si = %d l = %d\n", prefix, macro, si, l) }
						if s > 0 {			// print elif unless in a previous false block
							opt_aComment(text, "")
						}
					} else {
						pushError(&w, fmt.Sprintf("//* Warning: extra %selif after %selse - ignored\n", prefix, prefix))
					}
				} else if direcve == "else" {					// else
					l := len(siStk) - 1
					if l >= 0 && (si & ^0x1) != 0 {			// invert sense indicator unless -2 marking %else
						s := siStk[l]
						if s <= 0 || si > 0 || si == -2 {
							si = 0
						} else {
							si = 1
						}
						if *opt_X { fmt.Printf("%selse   si = %d l = %d s = %d\n", prefix, si, l, s) }
						if s > 0 {			// print else unless in a previous false block
							opt_aComment(text, "")
						}
					} else {
						pushError(&w, fmt.Sprintf("//* Warning: extra %selse - ignored\n", prefix))
					}
				} else if direcve == "endif" {					// endif
					l := len(siStk) - 1
					if *opt_X { fmt.Printf("%sendif  si = %d l = %d\n", prefix, si, l) }
					if l >= 0 {
						si = siStk[l]; siStk = siStk[:l]	// pop stacked sense indicator
						if si > 0 {			// print endif unless in a previous false block
							opt_aComment(text, "")
						}
					} else {
						pushError(&w, fmt.Sprintf("//* Warning: extra %sendif - ignored\n", prefix))
					}
				} else if si > 0 {
					if direcve == "define" {
						if *opt_X { fmt.Printf("%sdefine %s; si = %d\n", prefix, macro, si) }
						if *opt_l != "" { fmt.Fprintf(genFile, "//# %s\n", text) }
						opt_aComment(text, "")
						scan_define(direcve, macro)		// scan and save macro in %defs
					} else if direcve == "undef" {
						if reWord.MatchString(macro) {
							if *opt_l != "" { fmt.Fprintf(genFile, "//# %s\n", text) }
							opt_aComment(text, "")
							delete(defs, macro)		// safe to delete even if hash entry does not exist
							delete(clDefs, macro)
						} else {
							pushError(&w, fmt.Sprintf("//* Warning: %s%s '%s' has bad characters - ignored\n", prefix, direcve, macro))
						}
					} else if direcve == "include" {
						var includeFile, f string
						var fileInfo os.FileInfo
						macro = reSpaces.ReplaceAllLiteralString(macro, "")	// take out any white space
						if m := reInclude.FindStringSubmatch(macro); m != nil {
							//	^["<]
							//	(						# $1 full include file name
							//		([A-Za-z]:)?		# $2 optional Windows disc selector
							//		([/\\]?)			# $3 optional Linux or Windows path separator
							//		[A-Z_a-z.][\w/\\.-]*
							//	)
							//	[">]$/x
						  Include:
							for {
								f = m[1]					// a correct Linux, Unix or Windows path name
								if m[3] != "" {
									includeFile = f
									if fileInfo, err = os.Stat(includeFile); err == nil &&
										fileInfo.Mode().IsRegular()	{
										break Include			// absolute path name
									}
								} else {
									for _, p := range strings.Split(path, ":") { // include path
										includeFile = fmt.Sprintf("%s%c%s", p, os.PathSeparator, f)	// p/f
										if fileInfo, err = os.Stat(includeFile); err == nil &&
											fileInfo.Mode().IsRegular()	{
											break Include		// relative path name
										}
									}
								}
								pushError(&w, fmt.Sprintf("//* Warning: '%s' not in include path '%s' - ignored\n", includeFile, path))
								includeFile = ""
								break Include
							}
						} else {
							pushError(&w, fmt.Sprintf("//* Warning: %s%s '%s' has bad characters - ignored\n", prefix, direcve, macro))
						}
						if includeFile != "" {
							fi := &fileStk{ name: argIn, line: lineNo, si: si, file: inFileC, scan: scanner }
							inFileStk = append(inFileStk, fi)	// push current file info
							argIn = f							// use original "file" for error messages
							if inFileC , err = os.Open(includeFile); err == nil {
								scanner = bufio.NewScanner(inFileC)
								scanner.Split(bufio.ScanLines)
								lineNo = 0
								si = 1							// start sense indication again for a new include file
								if *opt_l != "" { fmt.Fprintf(genFile, "//# %s\n", text) }
								opt_aComment(text, "")
								if opt_m_M_Y {
									blanks()					// print blank lines now
									genLine(fmt.Sprintf("# 1 %q 1", argIn), "\\n")	// # 1 "file" 1
								}
								direcve = ""
								macro   = ""
								continue Level
							} else {
								pushError(&ret, "//* Error: Could not open file\n")
								l := len(inFileStk) - 1
								fi := inFileStk[l]
								inFileStk = inFileStk[:l]		// pop current file info
								argIn   = fi.name
								lineNo  = fi.line
								si      = fi.si
								inFileC = fi.file
								scanner = fi.scan
							}
						}
					} else if er_wa := reErrWarn.FindString(direcve); er_wa != "" {	// `^(error|warning)$`
						ecP := &w
						if er_wa == "error" {
							ecP = &ret
						}
						fmt.Fprintf(os.Stderr, "%s:%d:2: %s: %s%s %s\n", argIn, lineNo, er_wa, prefix, direcve, macro)
						*ecP++
						opt_aComment(text, "")
						if er_wa == "error" {
							si = 1						// restore sense indicator (ignore missing directives)
							siStk = siStk[0:0]			// clear sense indicator stack
							break Level					// deliberate error - emulate cpp
						}
					} else if direcve == "line" {
						if m = reLine.FindStringSubmatch(macro); len(m) == 3 {
							lineNo, _ = strconv.Atoi(m[1])	// err unlikely because of \d+ in reLine := `^\s*(\d+)\s+"([A-Z_a-z][\w.]*)"`
							lineNo--						// TODO take acount of m[3] file name when it does not equal argIn
							opt_aComment(text, "")
						} else {
							pushError(&ret, fmt.Sprintf("//* Error: %s%s '%s' malformed - ignored\n", prefix, direcve, macro))
						}
					} else {
						pushError(&w, fmt.Sprintf("//* Warning: %s%s '%s' unknown iCa directive - ignored\n", prefix, direcve, macro))
					}
				}	// end of skipped directive because of #if etc
				if *opt_T { fmt.Printf("*9* si = %d\n", si) }
				direcve = ""
				macro   = ""
				continue Line						// do not output %define etc directives (already counted as blank)
			}	// end of pre-processor directive handling

			if si <= 0 {
				blankLines++							// count lines in false conditional blocks as blank lines
				continue Line							// do not process lines in false blocks
			} else if opt_m_M_Y {
				/***********************************************************************
				 *  If -m -M or -Y then 8 or more blank are ignored
				 *  # $. "file" is output instead (same as cpp)
				 ***********************************************************************/
				if text == "" {
					blankLines++
					continue Line
				} else if blankLines > 0 {
					blanks()						// print blank lines now
				}
			}
			if cat != 0 {
				continue Line
			}
			if opt_m_M_Y {
				genLine(textRes, "\\n")				// just output macro processed non-iCa or error line
				outputError(lineText)				// possible error or warning in previous line
				continue Line						// end of Line for -m -M or -Y options
			}
			if *opt_S { fmt.Fprintf(genFile, "//*Ed1* %d: '%c' %q\n", lineNo, state, text) }
			/***********************************************************************
			 *  Analysis and generation of an iCa code line
			 *
			 *  In the previous scan of one or more text lines all comments have been
			 *  removed from FOR IF ELSE control lines and inner square barackets
			 *
			 *  Square bracket handling
			 *
			 *  FOR (N = 5; ...             x is a bare word; presumably a C variable
			 *
			 *  iCa:  []    [2]     [N]     [N+2]     [x]   [2+x]   [N+x]   [N+2+x]
			 *  eval: []    @{[2]}  @{[N]}  @{[N+2]}  [x]   [2+x]   [N+x]   [N+2+x]
			 *  iC:   []*   2       5       7         [x]*  [2+x]*  [5+x]*  [5+2+x]*
			 *  C:    []    2       5       7         [x]   [2+x]   [5+x]   [5+2+x]
			 *  * iC error message (will also produce error if compiled by immcc)
			 *
			 *  There is no difference in expanding square bracket expressions in iC or C.
			 *  The only difference is the output of error messages for iC code in 3 cases:
			 *  1)      an empty square bracket pair
			 *  2)      a bare word encountered in a square bracket pair
			 *  3)      a square bracket inside an embedded square bracket pair
			 *  These 3 cases are OK in C code and produce good C.
			 *
			 *  iCa:  [[]]  [[2]]   [[N]]   [[N+2]] [[N]+x] [[N+2]+x] [[N]+2+x] [[N]+2+[N+1]]
			 *  iC:   [[]]* [2]*    [5]*    [7]*    [5+x]*  [7+x]*    [5+2+x]*  [5+2+6]*
			 *  C:    [[]]* [2]     [5]     [7]     [5+x]   [7+x]     [5+2+x]   [5+2+6]
			 *
			 *  The inside of an embedded square bracket pair is treated like iC code
			 *  ie. empty, bare words and more square brackets are not allowed.
			 *
			 *  iCa:   [[[]]]   [[x]]   [[2+x]]     [[N+x]]  [[N+2+x]]  [[[N]+2]+x]
			 *  iC/C:  [[[]]]*  [[x]]*  [[2+x]]*    [[5+x]]* [[5+2+x]]* [[5+2]+x]*
			 *
			 *  Appropriate error messages will be output
			 *
			 *  The design goal is, that the generated Perl code will eval correctly
			 *  in all cases - error messages will give a good indication what is wrong
			 *  and immac exit status will prevent immcc compilation if iCmake is used.
			 *  Of course if immcc compilation is tried it will fail, but embedded iCa
			 *  error messages will still indicate the source of the error in iC listing.
			 *
			 *  sqNest
			 *  += 1    every opening square bracket
			 *  -= 1    every closing square bracket until 0 (error if more)
			 *  square bit encoding which is used when square brackets close
			 *  0       outside square brackets
			 *  |= 1    number or translated loop variable or finishing string
			 *  |= 2    completed and expanded 2nd level nested square brackets
			 *  &=~2    this bit is cleared when another nested [ is encountered
			 *  |= 4    bare word unless state == 'S' - string is type 1 constant
			 *  |= 8    after closing square bracket if sqNest != 0
			 *   = 0    after closing square bracket if sqNest == 0
			 *
			 *  The innermost square bracket pair is expanded to @{[expression]}
			 *  if square == 1 when the closing square bracket is encountered.
			 *  In other words this block may only contain numbers or translated loop
			 *  variables or finishing strings. Perl evaluates "expression" in an eval.
			 *  Syntactic errors in "expression" will cause a Perl eval error- this
			 *  should be rare, since index expressions are usually not complicated.
			 *  The Perl eval error message is easy to interpret to fix the iCa code.
			 *
			 *  Save state in sqState on opening square bracket and set state to 'A'.
			 *  Restore state with matching closing square bracket. This allows
			 *  correct handling of strings in square brackets in comments and strings.
			 *  iC Example: ["abc".N] generates abc0 abc1 ... (meaningless for C indices).
			 *  This example is coded better as abc[N], but string may be conditional,
			 *  in which case it must be handled as an expression in square brackets.
			 *
			 *
			 *  Brace handling
			 *
			 *  Braces are used both in C and iC to identify blocks of code. They
			 *  are also used in iCa to mark blocks of code controlled by a FOR loop.
			 *  There are rare cases, where braces in C are deliberately not matched,
			 *  which throws out the matching of the correct closing FOR brace if
			 *  single braces for FOR were used.
			 *
			 *  To overcome this problem iCa uses double braces:
			 *  FOR (N = 0; N < 4; N++) {{
			 *     a[N] ...
			 *  }}
			 *  Independent brace counting of the double braces is easy and leads to
			 *  clean translation code. An added advantage is, that visually the
			 *  boundaries of iCa FOR blocks are obvious and double braces have the
			 *  flavour of capital letters in the FOR keyword. It is also suggested,
			 *  that all FOR loop control variables be made capitals - similar to
			 *  the idea of capitalizing macros for the C pre-processor.
			 *
			 *  Literal blocks starting with '%{' mark the start of embedded C code.
			 *  A closing '%}' always marks the return to iC code independent of the
			 *  brace nesting of the C code in the literal block. This means that
			 *  %{ and %} are counted separately and error messages are output if any
			 *  nesting is attempted. Literal block (fragments) may be nested in a
			 *  FOR block (even only the start or the end). It would be pretty daft
			 *  to do so, but go figure - (quoting Larry Wall). In any case there
			 *  must always be alternate %{ and %} braces - even if only some of them
			 *  are in a FOR block.
			 *
			 *  %{
			 *      FOR CARRAY_NAME ("aa", "bb", "cc") {{
			 *  int     [CARRAY_NAME][[10]]     // expanding C array declaration
			 *  %}
			 *  %{
			 *      }}  // correct FOR loop end
			 *  %}
			 *
			 *  Brace counting of single braces must still be done to determine the
			 *  boundaries between iC and C code. The start of C code, embedded in iC
			 *  code, are blocks starting with the keywords "if", "else" or "switch".
			 *  The end of embedded C code of if/else/switch blocks can only be determined
			 *  by single brace counting and may turn out to be wrong in rare cases.
			 *  The only other use of braces in iC is for the body of function blocks.
			 *
			 *  if IX0.2 {
			 *      if IB0 != aa0[[0]] {        // start of C code
			 *          aa0[[0]] = IB0
			 *          printf("aa0[[0]] = %d\n", aa0[[0]])
			 *  FOR N (1 .. 2) {{               // unmatched FOR loop
			 *      }
			 *      else if IB[N] != aa[N][[0]] {
			 *          aa[N][[0]] = IB[N]
			 *          printf("aa[N][[0]] = %d\n", aa[N][[0]])
			 *  }}                              // FOR loop end
			 *      }                           // end of C code
			 *  }
			 *
			 *  Both the literal block example and the C example would be much more
			 *  sensibly coded by keeping the first closing brace above the FOR loop
			 *  and the final closing brace in the FOR loop. Either way the literal
			 *  block braces and in most cases the C braces must be in matching pairs.
			 *  Under all circumstances FOR loop double braces must match, and are
			 *  independent of both the literal block and C code brace matching.
			 *
			 *  Here is a rather obscure example of (apparently) unmatched braces in
			 *  C code when expanding the initialisation of a C array in a FOR loop.
			 *
			 *  int aa[] = {\
			 *      FOR (N = 0; N <= 3; N++) {{
			 *   [N][N < 3 ? ", " : " };"]\
			 *      }}
			 *
			 *  This expands to:    int aa[] = { 0, 1, 2, 3 };
			 *
			 *  Alternatively first line can be written thus:
			 *
			 *  int aa[] = ["{"]\
			 *
			 *  The first alternative does not work with the old single FOR brace dialect.
			 *  The second alternative of enclosing the constant string "{" in square
			 *  brackets, which evaluates simply to { and matches both braces in
			 *  double quotes, worked with the old iCa dialect, when string constants
			 *  in square bracket index expressions were introduced. It was this case,
			 *  which forced the re-design of the immac pre-processor.        - }}
			 *
			 *  Expansion of square bracket expressions is identical for C and iC -
			 *  only the error messages differ in iC. Thus nesting errors for single
			 *  braces will only cause wrong error messages for square bracket
			 *  expressions containing bare words (not allowed in iC). The output will
			 *  in most cases be nearly correct and errors should be easy to interpret
			 *  at the iC compile stage. (immcc will complain loudly about remaining
			 *  square brackets which do not belong to an immC array variable).
			 ***********************************************************************/
			text     = textRes							// should not be necessary TODO
			iNext    = 0
			FORstart := -1
			FORctrl  := ""
			SQstSv   = make([]rune, 0, 5)		// reset square bracket stack at start of a new analysis
			SQbrCnt  = 0
			SQflag  := false
			SQvar   := false
			SQcvar  := false
			SQstart := -1
			SQend   := -1
			SQbrIx  := 0
			SQdotIndex := 0
			SQcolPrev := -1
			if *opt_X { fmt.Printf(">2 %d:\t%q\n", lineNo, text) }
			if *opt_S { fmt.Fprintf(genFile, "//*Sc2* %d: '%c' %q\n", lineNo, state2, text) }
		  Rescan2:
			for {
				if *opt_S { fmt.Fprintf(genFile, "//*Rs2* %d: '%c' L %v atom = %q %q\n", lineNo, state2, LFflag, atom, text) }
				atom     = ""
				expAtom  = ""
				expPos   = -1
				pose     = 0					// balance {{
				//  reState:`(%?\{\{?|%?}}?|%?#|%%?|/[/*]|\*+/?|\w+|(\\\\)+|\\n|\\t|\\?"|\\?'|\\?\[|\\?\]|\S)`
				for i, posPair := range reState.FindAllStringIndex(text, -1) {
					//	(							balance {{
					//		%?\{\{?	|	# { {{ %{ - not used %{{
					//		%?}}?	|	# } }} %} - not used %}}
					//		%?#		|	# directive prefix # %#
					//		%%?		|	# directive prefix % %%
					//		/[/*]	|	# /* // start of C or C++ comment
					//		\*+/?	|	# */ **/ etc   end of C comment or * **
					//		\w+		|	# word identifier or number
					//		(\\\\)+	|	# remove pairs of backslashes
					//		\\n		|	# char \n NL in string
					//		\\t		|	# char \t TAB in string
					//		\\?"	|	# " or \"
					//		\\?'	|	# ' or \'
					//		\\?\[	|	# [ or \[   - used in iCa analysis only
					//		\\?\]	|	# ] or \]   - used in iCa analysis only
					//		\S			# any single non space character
					//	)
					prevAtom = atom
					prevPos  = pos						// start of previous atom
					prevPose = pose						// start of white space after previous atom
					pose  = posPair[1]
					if i < iNext {
						continue						// skip tokens in text already scanned
					}
					iNext = i							// save token count in case concatenated
					pos = posPair[0]
					atom = text[ pos : pose ]
					if *opt_X { fmt.Printf("%2d\t%3d %3d\t%q\n", i, pos, pose, atom) }
					/********************************************************************
					 *  Scan 2 "**" exponentiation (done in all states except comments and strings)
					 *  Exponentiation expression [expr1 ** expr3] is implemented as follows
					 *  replace with "pow(expr1, expr2)
					 *******************************************************************/
					if atom == "**" && state2 != 'C' && state2 != 'P' && state2 != 'S' {
						if reNumWord.MatchString(prevAtom) {
							expAtom = prevAtom			// 1st operand
							expPos  = prevPos
						} else {
							pushError(&ret, "//* Error: syntax error before '**' operator\n")
						}
					} else if expPos >= 0 {
						if reNumWord.MatchString(atom) {// 2nd operand must be next atom after "**"
							iCexpr := fmt.Sprintf("pow(%s, %s)", expAtom, atom)
							al := pose - expPos
							textRes = substr(textRes, expPos + offset, al, iCexpr)
							text = textRes
							offset = 0
							expFlag = true
							if SQflag {
								SQprCnt++			// count opening parentheses of pow() call
							}
							continue Rescan2
						} else {
							pushError(&ret, "//* Error: syntax error after '**' operator\n")
						}
						expPos  = -1
					}
					/********************************************************************
					 *  Scan 2 square bracket handling (done in all states)
					 *******************************************************************/
					if SQflag && state2 != 'S' {		// operators in a string are ignored
						if atom == "." {				// Perl type string concatenation operator
							if SQdotIndex >= cap(SQdotSlice) - 2 {	// allow extra capacity for "?", ":" and final "]"
								SQdotSlice = append(SQdotSlice, make([]SrtQmkColEndFlg, SQdotIndex)...)	// double capacity
							}
							SQdotSlice[SQdotIndex].end = pos + offset	// concatenate operator "." in a square bracket expression
							SQdotIndex++
							SQdotSlice[SQdotIndex].sta = pos + offset + 1	// start of text after "."
							if SQprCnt != 0 {
								pushError(&ret, "//* Error: unmatched parentheses before '.' in square bracket expression\n")
							}
						} else if atom == "(" {
							SQprCnt++
						} else if atom == "?" {			// ternary operator
							if SQprCnt == 0 {
								if SQcolPrev >= 0 {
									SQdotSlice[SQdotIndex].flg = true
									SQdotSlice[SQdotIndex+1].sta = SQdotSlice[SQdotIndex].sta
									SQdotSlice[SQdotIndex+1].qmk = SQdotSlice[SQdotIndex].qmk
									SQdotSlice[SQdotIndex+1].col = SQdotSlice[SQdotIndex].col
									SQdotSlice[SQdotIndex+1].end = pos + offset
									SQdotSlice[SQdotIndex].sta = SQcolPrev + 1
								} else {
									SQdotSlice[SQdotIndex].flg = false
								}
								SQdotSlice[SQdotIndex].qmk = pos + offset	// ternary operator "?" in a square bracket expression
								SQcolPrev = -1
								SQxFlag = false
							} else {
								pushError(&ret, "//* Error: ternary operator '?' in parenthes is not allowed in iCa\n")
							}
						} else if atom == ":" {			// ternary operator
							if SQprCnt == 0 {
								SQdotSlice[SQdotIndex].col = pos + offset	// ternary operator ":" in a square bracket expression
								SQcolPrev = pos + offset					// start of possible repeat ternary
								SQxFlag = false
							} else {
								pushError(&ret, "//* Error: ternary operator ':' in parenthes is not allowed in iCa\n")
							}
						} else if atom == ")" {
							SQprCnt--
						}
					} else if atom == "[" {
						if SQend >= 0 {
							SQxFlag = true
							SQend = -1
						} else {
							SQxFlag = false
						}
						if SQbrIx < len(SQbrInr) && SQbrInr[SQbrIx] == SQbrCnt {
							SQflag = true				// inner square bracket start
							SQcolPrev = -1
							SQbrIx++
							SQstSv = append(SQstSv, state2)	// push current state2
							state2 = 'A'
							if *opt_Z { fmt.Fprintf(genFile, "//[   state2 = '%c'\n", state2) }
							SQprCnt = 0					// start counting parentheses in an inner square bracket
							SQdotIndex = 0				// start collecting "." operators for inner square brackets
							SQdotSlice = make([]SrtQmkColEndFlg, cap(SQdotSlice))	// zero all elements
							SQstart = pos + offset
							SQdotSlice[0].sta = SQstart + 1	// start of inner square bracket expression
						}
						SQbrCnt++
						if *opt_X { fmt.Printf("%d: SqSt2 SQbrCnt = %d state2 = %c SQstSv = %c %q\n", lineNo, SQbrCnt, state2, SQstSv, textRes) }
					} else {
						SQend = -1						// not adjacent "["
					}
					if atom == "]" {
						var l int
						SQend = pose + offset
						SQbrCnt--						// checks have been done in previous pass
						l = len(SQstSv) - 1				// process inner square bracket
						if l >= 0 {
							state2 = SQstSv[l]			// pop saved state2
							SQstSv = SQstSv[:l]			// TODO do not need stack here
							if *opt_Z { fmt.Fprintf(genFile, "//]   state2 = '%c'\n", state2) }
						}
						if SQflag && SQvar && (iCmode || SQcvar) {	// must have some variable or number in iC mode
							SQflag = false							// or a valid control variable im C mode
							SQvar = false
							SQcvar = false
							if SQprCnt == 0 {			// parentheses must be matched in a square bracket expression
								if SQxFlag {
									/********************************************************************
									 *  Add x between index pairs of a multidimensional array
									 *   m[N][M]   ==> m0x0
									 *  Spaces are allowed
									 *   m [N] [M] ==> m0x0
									 *  Any space between the index pairs is removed at ## below
									 *******************************************************************/
									genSliceAppend("\"x\"")	// adjoining square bracket expressions are separated by "x"
									if *opt_S { fmt.Fprintf(genFile, "//*=0= %d: genSlice = %v\n", lineNo, genSlice) }
								}											// unless the expression contains a "string"
								genText := textRes[:SQstart]			// text preceding opening square bracket
								if len(genText) > 0 {
									if len(genSlice) > 0 || !reAllSpaces.MatchString(genText) {	// leading spaces is genText
										/********************************************************************
										 *  Precede " and \ by extra \
										 *******************************************************************/
										genText = reQuoteSlash.ReplaceAllString(genText, "\\$1")
										if iCmode && state2 != 'S' && reWordSpace.MatchString(genText) ||
											SQxFlag {
											/********************************************************************
											 *  Remove trailing white space if in iC mode and not in a string and
											 *                              following a word, IX\d. or QX\d.
											 *  ## or possible spaces after an insered x
											 *
											 *  For N == 0 in all cases
											 *   m [N] ==> m0     // space between array variable and index is removed
											 *   IX0. [N] ==> IX0.0	// same between digital I/O variale snd index
											 *
											 *  Spaces before index expressions in C code and strings are not removed 
											 *   case [N]: ==> case 0:
											 *   "Hello Nr [N]\n" ==> "Hello Nr 0\n"
											 *  If you must use indexed iC array variables in C code and strings
											 *  avoid extra spaces
											 *
											 *  Spaces before an index expression following one of the iC type keywords
											 *  bit, int, clock, timer or void are also not removed
											 *******************************************************************/
											m := reWordSpace. FindStringSubmatch(genText)
											if len(m) == 2 && (keywords[m[1]] & 0x2) == 0 { // not bit int clock timer void
												genText = reTrSpaces.ReplaceAllLiteralString(genText, "")
											}
										}
										/********************************************************************
										 *  Remove spaces around lone x or . for
										 *  array [N] x [M]		 // x would normally be left out - same output
										 *  QX [N] . [M]		 // C spec alows spaces before index operator [ 
										 *******************************************************************/
										genText = reSpacexdot.ReplaceAllString(genText, "$1")
										/********************************************************************
										 *  Add y to array name finishing with a numeral
										 *  aaa1 [N] translates to aaa1y0 for N == 0
										 *  in iC or C mode and in any state2; also remove spaces
										 *******************************************************************/
										genText = reWordNum.ReplaceAllString(genText, "${1}y")
									}
									if len(genText) > 0 {
										genSliceAppend("\""+genText+"\"")	// text string before inner opening square bracket
									}
									if *opt_S { fmt.Fprintf(genFile, "//*=1= %d: genSlice = %v\n", lineNo, genSlice) }
								}
								SQxFlag = false
								SQstart++
								SQdotSlice[SQdotIndex].end = pos + offset	// final "]" in a square bracket expression
								SQdotIndex++							// cap(SQdotSlice) is made large enough at "." handling
								expr3 := ""
								for i := 0; i < SQdotIndex; i++ {
									sta := SQdotSlice[i].sta
									end := SQdotSlice[i].end
									genText = textRes[sta:end]	// text string to next "." operator or final "]"
									qmk := SQdotSlice[i].qmk
									col := SQdotSlice[i].col
									if qmk > sta+1 && col > qmk+1 && col < end {
										/********************************************************************
										 *  ternary expression [expr1 ? expr2 : expr3] is implemented as follows
										 *  replace with "tern(expr1, expr2, expr3)
										 *******************************************************************/
										ternFlag = true
										_, expr1 := evalIf(textRes[sta:qmk], !DEF, !EVAL)	// add != 0 if expr1 is 'int'
										expr1  = reLdTrSps.ReplaceAllLiteralString(expr1, "")	// remove leading and trailing spaces
										expr2 := reLdTrSps.ReplaceAllLiteralString(textRes[qmk+1:col], "")
										if expr3 == "" {
											expr3 = reLdTrSps.ReplaceAllLiteralString(textRes[col+1:end], "")
										}
										expr3 = fmt.Sprintf("tern(%s, %s, %s)", expr1, expr2, expr3)
										if SQdotSlice[i].flg {
											SQdotIndex++
										} else {
											genText = expr3
											expr3 = ""
										}
									}
									if !SQdotSlice[i].flg {
										genSliceAppend(genText)	// text string to next "." operator
										if *opt_S { fmt.Fprintf(genFile, "//*=2= %d: genSlice = %v\n", lineNo, genSlice) }
									}
								}
								textRes = substr(textRes, 0, SQend, "")	// remove text + inner square bracket pair
								text = textRes
								offset  = 0
								iNext = 0;
								SQstart = -1			// no square bracket expression evaluation until next "["
								continue Rescan2
							}
							pushError(&ret, "//* Error: unmatched parentheses in square bracket expression\n")
						}
						SQflag = false
						if *opt_X { fmt.Printf("%d: SqEn2 SQbrCnt = %d state = %c SQstSv = %c l = %d %q\n", lineNo, SQbrCnt, state, SQstSv, l, textRes) }
					} else if state2 == 'S' {			// string constant
						if atom == "\"" {				// leading backslash will not compare
							_, state2 = PopCtrlState("\" 'S'")	// end of string constant
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if *opt_X { fmt.Printf("%d: StEn %q	state2 = %c\n", lineNo, textRes, state2) }
						}
					} else if state2 == 'H' {			// character constant
						if atom == "'" {				// leading backslash will not compare
							_, state2 = PopCtrlState("' 'H'")	// end of character constant
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if *opt_X { fmt.Printf("%d: CcEn %q	state2 = %c\n", lineNo, textRes, state2) }
						}
					} else if state2 == 'C' {			// C comment
						if reCommEnd.MatchString(atom) {		// == "*/" or "**/ etc
							_, state2 = PopCtrlState("*/ 'C'")	// end of C comment
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if *opt_X { fmt.Printf("%d: CoEn %q	state2 = %c\n", lineNo, textRes, state2) }
						}
					} else if atom == "/*" {
						PushCtrlState(atom, state2, "/*")
						state2 = 'C'					// start of C comment found
						if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
					} else if atom == "//" {
						if state2 != 'P' {				// ignore "//" inside C++ comment
							PushCtrlState(atom, state2, "//")
							state2 = 'P'				// start of C++ comment found
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
						}
					} else if atom == "\"" {			// leading backslash will not compare
						if SQflag {
							SQvar = true				// square bracket contains at least one string - evaluate
						}
						PushCtrlState(atom, state2, "\"")
						state2 = 'S'					// start of string found
						if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
						if *opt_X { fmt.Printf("%d: StSt %q	state2 = %c\n", lineNo, textRes, state2) }
					} else if atom == "'" {				// leading backslash will not compare
						PushCtrlState(atom, state2, "'")
						state2 = 'H'					// start of character constant found
						if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
					} else if state2 == 'A' {			// program - not comment, string, char constant
						if FORorIF(0, SQflag, atom, pos, pose, prevPose,
							&offset, &iNext, &FORstart, &dBraceCnt, &braceCnt, &posNext, &state2,
							&text, &textRes, &FORctrl, &ForIfFlg, &iesFlag, &SQvar, &SQcvar) {
							continue Rescan2
						}
					} else if state2 == 'B' {
						if atom == "{{" {
							/********************************************************************
							 *  Opening double braces after "FOR"
							 *******************************************************************/
							var forLine string
							ForIfFlg = false			// end of FOR control line
							if FORstart < 0 { panic("FORstart error") }	// should not happen
							if len(genSlice) > 0 {
								/********************************************************************
								 *  Special FOR line with string list containing square bracket index
								 *  FOR CARRAY_NAME ("aa[N]", "bb[N]") {{												// }}
								 *  converts to:
								 *  forLine = 'FOR CARRAY_NAME (\"aa", N, "\", \"bb", N, ") {{'							// }}
								 *  modified here to:
								 *  forLine = 'FOR CARRAY_NAME (fmt.Sprint("aa", N), fmt.Sprint("bb", N)) {{'			// }}
								 *  final genFile line:
								 *  for _, CARRAY_NAME := range []string{fmt.Sprint("aa", N), fmt.Sprint("bb", N)} {	// }
								 *******************************************************************/
								genSlice = append(genSlice, textRes)
								tx := strings.Join(genSlice, ", ")
								genSlice = make([]string, 0, cap(genSlice))
								FORstart++
								forLine = tx[FORstart:]
								forLine = reStrInd.ReplaceAllString(forLine, "fmt.Sprint(\"$1)")
							} else {
								forLine = textRes[FORstart:pose]
							}
							FORstart = -1
							if FORline != "" {
								fmt.Fprint(genFile, FORline)
								FORline = ""			// IF, ELSE IF and error are output immediately
							}
							if *opt_S { fmt.Fprintf(genFile, "//*BrO* %d: '%c' %q\n", lineNo, state2, forLine) }
							if m := reFORstd.FindStringSubmatch(forLine); len(m) == 5 {
								/********************************************************************
								 *  Standard C FOR statement: FOR (N = 0; N < 10; N++) {{	  		// }}
								 *  `^FOR\s*\(\s*((int)\s+)?([a-zA-Z]\w*)\s*=(.*)\)\s*{{$`				  		// }}
								 *******************************************************************/
								FORline = fmt.Sprintf("  for %s :=%s {\n", m[3], m[4])
								if sFlag && m[2] == "int" {
									pushError(&w, "//* Warning: type specifier 'int' is deprecated - ignored\n")
								}
								if FORctrl != m[3] { panic("FOR loop compile error") }	// should not happen
							} else if m := reFORdot.FindStringSubmatch(forLine); len(m) == 10 {
								/********************************************************************
								 *   Perl .. FOR statement: FOR N ( 0 .. 9 ) {{						// }}
								 *  `^FOR\s+((int)\s+)?([a-zA-Z]\w*)\s*\(\s*(\w+(\s*([*%/&^|()+-]|<<|>>)\s*\w+)*)\s*\.\.\s*(\w+(\s*([*%/&^|()+-]|<<|>>)\s*\w+)*)\s*\)\s*{{$`	// }}
								 *******************************************************************/
								FORline = fmt.Sprintf("  for %[1]s := %[2]s; %[1]s <= %[3]s; %[1]s++ {\n", m[3], m[4], m[7])
								if sFlag && m[2] == "int" {
									pushError(&w, "//* Warning: type specifier 'int' is deprecated - ignored\n")
								}
								if FORctrl != m[3] { panic("FOR loop compile error") }	// should not happen
							} else if m := reFORlist.FindStringSubmatch(forLine); len(m) == 5 {
								/********************************************************************
								 *  Perl string list FOR statement: FOR N ("aa", "ab", "ac") {{		// }}
								 *  or Perl int list FOR statement: FOR N (4, -3, +2) {{		 	// }}
								 *  Loop control variables may occur in either list.
								 *  If a list contains one string or string type loop control variable,
								 *  the whole list is type string - else it is type int.
								 *  `^FOR\s+((int)\s+)?([a-zA-Z]\w*)\s*\(\s*(.*\S)\s*\)\s*{{$`			  		// }}
								 *******************************************************************/
								numbers := make([]string, 0)
								members := make([]string, 0)
								strFlag := false
								m[4] = reTrComma.ReplaceAllLiteralString(m[4], "")	// remove trailing comma of a list
								prevMember := ""
								for _, member := range reComma.Split(m[4], -1) {
									if prevMember != "" {
										members = append(members, prevMember + "," + member)
										strFlag = true
										prevMember = ""
									} else if reSignNum.MatchString(member) {
										member = reSpaces.ReplaceAllLiteralString(member, "")	// take out any white space
										numbers = append(numbers, member)
										members = append(members, "\""+member+"\"")
									} else if reString.MatchString(member) {
										members = append(members, member)
										strFlag = true
									} else if reFmtSprint.MatchString(member) {
										prevMember = member						// special FOR line with indexed strings
										continue
									} else if reBareWd.MatchString(member) {
										if ts, ok := FORctrlVar[member]; ok {	// exists
											if ts == "string" {
												strFlag = true
											}
											numbers = append(numbers, member)
											members = append(members, member)
										} else {
											pushError(&ret, fmt.Sprintf("//* Error: bare word %q not allowed in string list\n", member))
										}
									} else {
										pushError(&ret, fmt.Sprintf("//* Error: bare words %q not allowed in string list\n", member))
									}
								}
								typeSpec := "int"
								list     := ""
								if strFlag {
									list = strings.Join(members, ", ")
									typeSpec = "string"
									FORctrlVar[FORctrl] = "string"
								} else {
									list = strings.Join(numbers, ", ")
								}
								FORline = fmt.Sprintf("  for _, %s := range []%s{%s} {\n", m[3], typeSpec, list)
								if sFlag && m[2] == "int" {
									pushError(&w, "//* Warning: type specifier 'int' is deprecated - ignored\n")
								}
								if FORctrl != m[3] { panic("FOR loop compile error") }	// should not happen
							} else {
								/********************************************************************
								 *  Syntax error in FOR control statement
								 *******************************************************************/
								if *opt_S { fmt.Fprintf(genFile, "//*ERR* %d: '%c' %q\n", lineNo, state2, text) }
								pushError(&ret, "//* Error: syntax error in FOR control statement - state2 'B'\n")
								break;
							}
							opt_aComment(forLine, opt_aEdit(&forLine, FORctrl))
							LFflag = false
							if *opt_Z { fmt.Fprint(genFile, "//= LFflag = false at 'B' \"{{\"\n") }
							state2 = 'F'
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if *opt_X { fmt.Printf("{{ dBraceCnt = %d %q\n", dBraceCnt, textRes) }
							FORctrl = ""
							dBraceCnt++
							textRes = substr(textRes, 0, pose, "")
							text = textRes			// remove text just output
							if *opt_S { fmt.Fprintf(genFile, "//*BrF* %d: '%c' %q\n", lineNo, state2, text) }
							offset = 0
							if textRes != "" {
								iNext = 0;
								continue Rescan2
							}
						} else if atom == "}}" {
							pushError(&ret, "//* Error: syntax error closing double brace - state2 'B'\n")
						} else if _, ok := FORctrlVar[atom]; !ok && reWord.MatchString(atom) {
							pushError(&ret, fmt.Sprintf("//* Error: %q not a valid control variable or not in scope\n", atom))
						}
					} else if state2 == 'F' {
						if FORorIF(0, SQflag, atom, pos, pose, prevPose,
							&offset, &iNext, &FORstart, &dBraceCnt, &braceCnt, &posNext, &state2,
							&text, &textRes, &FORctrl, &ForIfFlg, &iesFlag, &SQvar, &SQcvar) {
							continue Rescan2
						}
					} else if state2 == 'G' {
						if atom == "{{" {
							/********************************************************************
							 *  Opening double braces after "IF" or "ELSE IF"
							 *******************************************************************/
							ForIfFlg = false			// end of FOR IF ELSE control line
							if FORstart < 0 { panic("FORstart error") }	// should not happen
							forLine := textRes[FORstart:pose]
							FORstart = -1
							if FORline != "" {
								fmt.Fprint(genFile, FORline)
								FORline = ""			// IF, ELSE IF and error are output immediately
							}
							if *opt_S { fmt.Fprintf(genFile, "//*BrO* %d: '%c' %q\n", lineNo, state2, forLine) }
							if m := reIFstd.FindStringSubmatch(forLine); len(m) == 3 {
								/********************************************************************
								 *  Standard C IF statement: IF (N%2 != 2) {{						// }}
								 *  or Standard C ELSE IF statement: ELSE IF (N%2 == 2) {{			// }}
								 *  `^(ELSE\s+)?IF\s*\(\s*(.*\S)\s*\)\s*{{$`						// }}
								 *******************************************************************/
								elsif := "if"
								if m[1] != "" {
									elsif = "} else if"
									forLine = "}} "+forLine
								}
								params := opt_aEdit(&forLine, "")
								opt_aComment(forLine, params)
								_, expr := evalIf(m[2], !DEF, !EVAL)	// add != 0 if m[2] is 'int'
								fmt.Fprint(genFile, fmt.Sprintf("  %s %s {\n", elsif, expr))
								if m[1] != "" {
									opt_aComment(forLine, params)
								}
							} else {
								/********************************************************************
								 *  Syntax error in IF or ELSE IF control statement
								 *******************************************************************/
								if *opt_S { fmt.Fprintf(genFile, "//*ERR* %d: '%c' %q\n", lineNo, state2, text) }
								pushError(&ret, "//* Error: syntax error in IF or ELSE IF control statement - state2 'G'\n")
							}
							LFflag = false
							if *opt_Z { fmt.Fprint(genFile, "//= LFflag = false at 'G' \"{{\"\n") }
							state2 = 'J'
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if *opt_X { fmt.Printf("{{ dBraceCnt = %d %q\n", dBraceCnt, textRes) }
							FORctrl = ""
							dBraceCnt++
							textRes = substr(textRes, 0, pose, "")
							text = textRes			// remove text just output
							if *opt_S { fmt.Fprintf(genFile, "//*BrI* %d: '%c' %q\n", lineNo, state2, text) }
							offset = 0
							if textRes != "" {
								iNext = 0;
								continue Rescan2
							}
						} else if atom == "}}" {
							pushError(&ret, "//* Error: syntax error closing double brace - state2 'G'\n")
						} else if _, ok := FORctrlVar[atom]; !ok && reWord.MatchString(atom) {
							pushError(&ret, fmt.Sprintf("//* Error: %q not a valid control variable or not in scope\n", atom))
						}
					} else if state2 == 'J' {
						if FORorIF('D', SQflag, atom, pos, pose, prevPose,
							&offset, &iNext, &FORstart, &dBraceCnt, &braceCnt, &posNext, &state2,
							&text, &textRes, &FORctrl, &ForIfFlg, &iesFlag, &SQvar, &SQcvar) {
							continue Rescan2
						}
					} else if state2 == 'D' {
						if atom == "ELSE" {
							FORstart = pos
							PushCtrlState("ELSE", Read_ifStat("ELSE at 'D'"), "ELSE at 'D'")	// use state2 of initial IF; ignore state2 of current ELSE
							state2 = 'E'				// ELSE
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
						} else if atom == "FOR" || atom == "IF" {
							if FORorIF('X', SQflag, atom, pos, pose, prevPose,	// block double braces
								&offset, &iNext, &FORstart, &dBraceCnt, &braceCnt, &posNext, &state2,
								&text, &textRes, &FORctrl, &ForIfFlg, &iesFlag, &SQvar, &SQcvar) {
								continue Rescan2
							}
						} else if atom == "{{" {
							pushError(&ret, "//* Error: opening double braces directly follow closing double braces\n")
							state2 = 'A'
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
						} else {
							if FORline != "" {
								fmt.Fprint(genFile, FORline)
								FORline = ""
							}
							if *opt_a && genFlag {
								genLine("", "\\n")
							}
							fmt.Fprint(genFile, "  }\n")	// IF {{ ... }} x
							opt_aComment("}}", "")
							if genTail != "" {
								if *opt_Z { fmt.Fprintf(genFile, "//= genSliceOut(%q, %s) 'D' deferred output\n", genSlice, genTail) }
								genSliceOut(genTail)	// deferrred output for 'D'
								genTail = ""
							}
							state2 = 'A'				// any other atom ignore further FOR or IF
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if *opt_S { fmt.Fprintf(genFile, "//*Ds2* %d: '%c' L %v atom = %q %q prevPose = %d\n", lineNo, state2, LFflag, atom, text, prevPose) }
							if prevPose > 0 {
								textRes = substr(textRes, 0, prevPose, "")
								if *opt_S { fmt.Fprintf(genFile, "//*De2* %d: '%c' %q\n", lineNo, state2, textRes) }
								text = textRes			// remove text just output
								offset = 0
								iNext = 0;
								continue Rescan2
							}
							if atom == "}}" {			// is next atom another closing double braces?
								continue Rescan2		// yes - scan this closing double braces again
							}
						}
					} else if state2 == 'E' {
						if atom == "IF" {
							state2 = 'G'				// ELSE IF
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
						} else if atom == "{{" {
							/********************************************************************
							 *  Opening double braces after "ELSE"
							 *******************************************************************/
							if *opt_X { fmt.Printf("{{ dBraceCnt = %d %q\n", dBraceCnt, textRes) }
							FORctrl = ""
							dBraceCnt++
							opt_aComment("}} ELSE {{", "")
							fmt.Fprint(genFile, "  } else {\n")
							opt_aComment("}} ELSE {{", "")
							LFflag = false
							if *opt_Z { fmt.Fprint(genFile, "//= LFflag = false at 'E' \"{{\"\n") }
							state2 = 'L'
							if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
							if textRes != "" {
								textRes = substr(textRes, 0, pose, "")
								text = textRes			// remove text just output
								if *opt_S { fmt.Fprintf(genFile, "//*BrI* %d: '%c' %q\n", lineNo, state2, text) }
								offset = 0
								iNext = 0;
								continue Rescan2
							}
						} else {
							pushError(&ret, "//* Error: syntax error after ELSE - state2 'E'\n")
						}
					} else if state2 == 'L' {
						if FORorIF(0, SQflag, atom, pos, pose, prevPose,
							&offset, &iNext, &FORstart, &dBraceCnt, &braceCnt, &posNext, &state2,
							&text, &textRes, &FORctrl, &ForIfFlg, &iesFlag, &SQvar, &SQcvar) {
							continue Rescan2
						}
					}
				}
				break Rescan2
			}	// end of state2 machine scan 2 of one iCa line
			if state2 == 'P' {							// C++ comment
				_, state2 = PopCtrlState("end of line 'P'")	// end of C++ comment
				if *opt_Z { fmt.Fprintf(genFile, "//#   state2 = '%c' atom = %q\n", state2, atom) }
			}
			if *opt_S { fmt.Fprintf(genFile, "//*Es2* %d: '%c' L %v %q\n", lineNo, state2, LFflag, text) }
			if state2 == 'A' || state2 == 'F'  || state2 == 'J'  || state2 == 'D' || state2 == 'L' {
				if !reAllSpaces.MatchString(text) {
					genText := text
					genText  = reQuoteSlash.ReplaceAllString(genText, "\\$1")	// precede " and \ by extra \
					genSliceAppend("\""+genText+"\"")
					LFflag = true
					if *opt_Z { fmt.Fprintf(genFile, "//= LFflag = true at '%c' at end of line\n", state2) }
					if *opt_S { fmt.Fprintf(genFile, "//*=5= %d: genSlice = %v\n", lineNo, genSlice) }
				} else if reAllSpaces.MatchString(lineText) {
					if *opt_Z { fmt.Fprint(genFile, "//= empty line\n") }
					LFflag = true
				}
				if LFflag {
					if len(genSlice) > 0 {
						genTail = ", \"\\n\""
					} else {
						if *opt_Z { fmt.Fprint(genFile, "//= empty genSlice\n") }
						genTail = "\"\\n\""
					}
					if state2 != 'D' {
						if *opt_Z { fmt.Fprintf(genFile, "//= genSliceOut(%q, %s) '%c' at end of line\n", genSlice, genTail, state2) }
						genSliceOut(genTail)	// output now for 'A' 'F' or 'L'
						genTail = ""			// defer output for 'D'
					}
				}
			} else if state2 == 'C' {
				if len(genSlice) > 0 {
					if *opt_Z { fmt.Fprintf(genFile, "//= genSliceOut(%q) 'C' square bracket expressions\n", genSlice) }
					genSliceOut("")				// output square bracket expressions in comment
				}
				genLine(textRes, "\\n")			// output line of unfinished C comment
			} else {
				pushError(&ret, fmt.Sprintf("//* Error: syntax error at end of line - state2 '%c'\n", state2))
			}
			/***********************************************************************
			 *  End of analysis and generation of an iCa code line
			 ***********************************************************************/
		}	// end of Scan Line
		if err := scanner.Err(); err != nil {
			log.Print(err); os.Exit(3)
		}
		/***********************************************************************
		 *  End of one file - synchronise brace and square bracket counts TODO
		 ***********************************************************************/
		if state2 == 'D' {
			if *opt_a && genFlag {
				genLine("", "\\n")
			}
			fmt.Fprint(genFile, "  }\n")		// dangling closing double braces after an incomplet IF block
			opt_aComment("}}", "")
		}										// unlikely to finish with an IF block - go figure
		if state != 'A' {
			pushError(&ret, fmt.Sprintf("//* Error: syntax error at end of file %q - state '%c'\n", argIn, state))
		}
		outputError(lineText)					// possible error or warning in previous line
		if l := len(inFileStk) - 1; l >= 0 {
			inFileC.Close()						// EOF of an include file reached
			fi := inFileStk[l]
			inFileStk = inFileStk[:l]			// pop current file info
			argIn   = fi.name
			lineNo  = fi.line
			si      = fi.si
			inFileC = fi.file
			scanner = fi.scan
			if opt_m_M_Y {
				hashLineFlag = true
			}
			continue Level
		}
		if hashLineFlag {
			genLine(fmt.Sprintf("# %d %q 2", lineNo, argIn), "\\n")	// # 8 "file" 2
		}
		if ForIfFlg {
			pushError(&ret, "//* Error: at EOF - FOR line not complete\n")
		}
		if dBraceCnt != 0 {
			pushError(&ret, "//* Error: at EOF - FOR line twin braces do not match\n")
		}
		break Level
	} // exhaust all nested include files
	/***********************************************************************
	 *  All lines in the iCa file have been read
	 ***********************************************************************/
	if len(siStk) > 0 || si != 1 {
		pushError(&w, "//* Warning: at EOF - missing %endif - ignored\n")
		outputError(lineText)
	}

	/***********************************************************************
	 *  Show all defined macros
	 ***********************************************************************/
	if *opt_T || *opt_X {
		clPrefix := "P"
		prefix   := "%%"
		if *opt_m {
			clPrefix = "D"
			prefix   = "#"
		} else if *opt_M || *opt_Y {
			clPrefix = "D"
			prefix   = "%"
		}
		if len(clDefs) > 0 || len(defs) > 0 {
			fmt.Println("*******************************************************")
		}
		if len(clDefs) > 0 {
			fmt.Println("map clDefs [macro]        {pCnt} {translate}")
			macros := make([]string, 0, len(clDefs))
			for macro, _ := range clDefs {
				macros = append(macros, macro)
			}
			sort.Strings(macros)
			for _, macro := range macros {
				fmt.Printf("-%s         %-15q %2d    '%s'\n", clPrefix, macro, clDefs[macro].num, clDefs[macro].str)
			}
		}
		if len(defs) > 0 {
			fmt.Println("map defs   [macro]        {pCnt} {translate}")
			macros := make([]string, 0, len(defs))
			for macro, _ := range defs {
				macros = append(macros, macro)
			}
			sort.Strings(macros)
			for _, macro := range macros {
				fmt.Printf("%2sdefine   %-15q %2d    '%s'\n", prefix, macro, defs[macro].num, defs[macro].str)
			}
		}
		if len(clDefs) > 0 || len(defs) > 0 {
			fmt.Println("*******************************************************")
		}
	}

  CloseFiles:
	if !*opt_t {
		fmt.Fprint(genFile,		// balance { {
` }
}
`		)

		if expFlag {
			fmt.Fprint(genFile,
`
func pow(op1, op2 int) int {
	if op2 > 63 {
		panic("pow: op2 > 63")
	}
	ret := 1
	for i := 0; i < op2; i++ {
		ret *= op1
	}
	return ret
}
`			)
		}

		if ternFlag {
			fmt.Fprint(genFile,
`
func tern(cond bool, ifVal, elseVal interface{}) interface{} {
	if cond {
		return ifVal
	}
	return elseVal
}
`			)
		}

		genFile.Close() // close generated file
		if ret == 0 && !*opt_t {
			/********************************************************************
			 *  Execute the generated GO file and read its output
			 *******************************************************************/
			genOut, err := exec.Command("go", "run", genName).CombinedOutput()
			if err != nil {
				fmt.Print(string(genOut)) // error message from generated file
				log.Print(err)
				ret++
			} else {
				outLines := strings.SplitAfter(string(genOut), "\n")
				for _, outLine := range outLines {
					if len(outLine) > 0 {
						if !opt_m_M_Y {
							outLine = reCommaSemi.ReplaceAllString(outLine, ";")	// remove comma from ,; in generated iCa code
						}
						fmt.Print(outLine) // final output to stdout or -o output
					}
				}
			}
		}
		if *opt_l != "" && *opt_l != genName {
			err = os.Remove(genName)	// remove generated file unless log file
			if err != nil {
				log.Print(err); os.Exit(e)
			}
		}
		if *opt_l != "" {
			err := os.Chmod(*opt_l, 0444)
			if err != nil {
				log.Print(err); os.Exit(e)
			}
		}
	}
	if *opt_o != "" {
		if err := os.Chmod(*opt_o, 0444); err != nil {
			log.Print(err); os.Exit(e)
		}
	}
	if w > 0 {
		fmt.Fprintf(os.Stderr, "//*** %d Warnings reported\n", w)
	}
	if ret > 0 {
		fmt.Fprintf(os.Stderr, "//*** %d Errors reported\n", ret)
		ret = 2
	}
	if *opt_e != "" {
		if err := os.Chmod(*opt_e, 0444); err != nil {
			log.Print(err); os.Exit(e)
		}
	}
	os.Exit(ret)
} // main

/***********************************************************************
 *
 *      Scan %define or -P macro and save it in map defs[macro]
 *
 *      argument count is               .num   -1 for object macro,
 *                                              0 - n for function macro
 *      the actual macro translation is .str
 *
 *          Object macros
 *      SIZE    512       will be converted to { -1 , 512       }
 *      DEF               will be converted to { -1 , 1         }
 *
 *          Function macros
 *      X()     32        will be converted to {  0 , 32        }
 *      Y(len)  (len * 8) will be converted to {  1 , (#1 * 8)  }
 *      Z(a, b) (a * b)   will be converted to {  2 , (#1 * #2) }
 *
 *      Called for -P or -D command line macro definitions and %define
 *
 *      If no macros at all are defined defs == nil.
 *
 ***********************************************************************/

func scan_define(direcve, macro string) (string, string) {
	if *opt_T { fmt.Printf("//* %d:	%s %s\n", lineNo, direcve, macro) }
	if m := reMacro.FindStringSubmatch(macro); m != nil {
		//	m/^
		//	([A-Z_a-z]\w*)					# $1 identifier (not optional)
		//	(								# $2 arguments in parentheses
		//	  \(							#	 no space allowed between identifier and opening parentheses
		//		\s*							#	 space before arguments is optional
		//		(							# $3 arguments
		//		  ([A-Z_a-z]\w*)			# $4 1st argument
		//		  (							# $5
		//			\s*,\s*([A-Z_a-z]\w*)	# $6 2nd or further comma separated arguments
		//		  )*						#	 further arguments are optional
		//		)*							#	 arguments in paraenthese are optional (cpp)
		//		\s*							#	 space after arguments is optional
		//	  \)							#	 closing parentheses
		//	)?								#	 arguments in parentheses are optional
		//	(								# $7
		//	  \s+							#	 space between identifier and translate mandatory
		//	  (.+)							# $8 translate
		//	)?								#	 whole of translate is optional (default is 1)
		//	$/x
		identifier := m[1]
		parens     := m[2]					// optional arguments in parentheses  - "" is object macro
		arguments  := m[3]					// has no leading and trailing spaces - "" is 0 parameter function macro
		translate  := m[8]
		if translate == "" {
			translate = "1"					// default 1 for translate
		}
		val := ""
		if *opt_T { fmt.Printf("//* %d: identifier = %q parens = %q arguments = %q translate = %q\n", lineNo, identifier, parens, arguments, translate) }
		translate = reHashHash.ReplaceAllLiteralString(translate, " #\\# ")	// ## must be embedded in spaces which will disappear
		translate = reHash.ReplaceAllString(translate, "#\\$1")				// protect previous #'s in translate except parameters
		/********************************************************************
		 *  Collect individual parameters from argument list of a function macro
		 *******************************************************************/
		parameters := make(map[string]*numNum)
		pCnt := 0										// required outside for loop
		if parens != "" && arguments != "" {			// if none, arguments is guaranteed to be ""
			for _, parameter := range reComma.Split(arguments, -1) {
				pCnt++ // no need to test parameters for being valid C words - extraction above ensures this
				parameters[parameter] = &numNum{ nm1: pCnt, nm2: 0 }	// start with positional index 1
			}
		} else if parens == "" {
			pCnt = -1										// object macro
		}													// else pCnt = 0 - function macro with 0 arguments
		/********************************************************************
		 *  Replace parameters in the translation with #1 #2 etc
		 *  Handle C and C++ comments and "xyz" strings in macro translation
		 *  as well as 'x' character constants, taking care if \ escapes
		 *******************************************************************/
		state   := 'A'									// outside of string, character constant or comment
		offset := 0
		cPos    := 0
		tr      := translate							// do not modify translate while scanning it
		lo      := 0
		posNext := 0
		//	reTranslate: ([A-Z_a-z]\w*|\/[*\/]|\*\/|\\?["'])
	  Rescan:
		for _, posPair := range reTranslate.FindAllStringIndex(translate, -1) {
			//	m/
			//	(							# $1
			//		[A-Z_a-z]\w*	|		#	 word in translate
			//		\/[*\/]			|		#	 start of C or C++ comment
			//		\*\/			|		#	 end of C comment
			//		\\?["']					#	 string or character constant delimiters
			//	)
			//	/xg
			pos  := posPair[0] + lo
			pose := posPair[1] + lo
			atom := translate[ pos : pose ]
			if state == 'A' {
				if reWord.MatchString(atom) {	// TODO should be `^[A-Z_a-z]`  // avoid parameter replacement in string and character constants
					if p, ok := parameters[atom]; ok {	// map parameters is empty for object like macro
						al := len(atom)
						r  := "#"+strconv.Itoa(p.nm1)	// change parameter to '#1' etc
						tr = substr(tr, pos + offset, al, r)
						offset -= al - len(r)
						p.nm2++							// record that this parameter has occured in the translation
					} else if ald, _, ok, rs := resolve(atom, pos, &posNext, &offset, 0, &translate, &tr); ok {
						if rs {
							lo = ald
							continue Rescan		// re-start scan of same text with token at current position
						}						// which may not be a word
						lo = 0
					}
					posNext = 0
				} else if atom == "/*" {
					state = 'C'							// start of C comment */
					cPos  = pos
				} else if atom == "//" {
					state = 'P'							// C++ comment
					tr = tr[:pos+offset]				// truncate
					break
				} else if atom == "\"" {				// '\"' will not compare
					state = 'S'							// start of string
				} else if atom == "'" {					// "\'" will not compare
					state = 'H'							// start of character constant
				}
			} else if state == 'C' {					// C comment in translate
				if reCommEnd.MatchString(atom) {		// == "*/" or "**/ etc
					al := pose - cPos
					tr = substr(tr, cPos + offset, al, " ")
					offset -= al - 1					// replace C comment by one space
					state = 'A'							// end of C comment
				}
			} else if state == 'S' {					// in string constant
				if atom == "\"" {						// leading backslash will not compare
					state = 'A'							// end of string constant
				}
			} else if state == 'H' {					// in character constant
				if atom == "'" {						// leading backslash will not compare
					state = 'A'							// end of character constant
				}
			}
		}
		translate = tr
		translate = reSpaces.ReplaceAllLiteralString(translate, " ")	// compress spaces to 1 (no leading spaces in translate)
		translate = reTrSpaces.ReplaceAllLiteralString(translate, "")	// remove possible space from trailing C comment
		if *opt_X { fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s` pCnt = %d\n", lineNo, identifier, arguments, translate, pCnt) }
		if parens != "" {								// if none, arguments is guaranteed to be ""
			val = translate+":"+strconv.Itoa(pCnt)
		} else {
			val = translate
		}
		if ref, ok := clDefs[identifier]; ok {			 // exists
			val1 := ref.str
			if ref.num >= 0 {
				val1 += ":"+strconv.Itoa(ref.num)
			}
			if *opt_l != "" { fmt.Fprintf(genFile, "//# command line definition -P %s=%s has precedence over \"%%define %s %s\"\n", identifier, val1, identifier, val) }
			if *opt_a { fmt.Printf("//***** -P %s=%s has precedence over \"%%define %s %s\"\n", identifier, val1, identifier, val) }
		} else if ref, ok := defs[identifier]; ok {		 // exists
			val1 := ref.str
			if ref.num >= 0 {
				val1 += ":"+strconv.Itoa(ref.num)
			}
			if val != val1 {
				pushError(&w, fmt.Sprintf("//* Warning: re-definition '%s=%s' to '%s' - ignored\n", identifier, val1, val))
			}
		} else {
			for k, p := range parameters {
				if p.nm2 == 0 {
					pushError(&w, fmt.Sprintf("//* Warning: parameter %q (%s) missing in %q\n", k, "#"+strconv.Itoa(p.nm1), translate))
				}
			}
			if !opt_m_M_Y && !reConstExpr.MatchString(translate) {	// `(^\s*\d+\s*$|[^\s()\d*\/%+-])`
				/********************************************************************
				 *  If 'translate' is a constant arithmetic expression in iCa processing
				 *  use 'govaluate' to compile and evaluate the arithmetic expression after
				 *  modifying any division sub-expressions by adding '|0' to them.
				 *  The OR operation causes govaluate to cast the result of any division
				 *  from float64 to int64. This trunctes any fractional part of a division.
				 *  ORing with '0' does not change the result, which is cast back to float64.
				 *  CAVEAT: the regular expression 'reDivInt' only recognises plain numbers
				 *  and singly nested expressions in parentheses as operands of a division.
				 *  More deeply nested parenthesised expressions as operands will cause errors.
				 *******************************************************************/
				tr := reDivInt.ReplaceAllString(translate, "($1|0)")
				if expression, errExp := govaluate.NewEvaluableExpression(tr); errExp == nil {
					if result, errEval := expression.Evaluate(nil); errEval == nil {
						if *opt_X { fmt.Printf("go-tran> %v\n", result) }
						if result, ok := result.(float64); ok {		// should be float64
							translate = strconv.Itoa(int(result))
						}
					}	// ignore evaluation or ! float64 result error - unlikely - just keep translate
					lineText += " ==> "+translate					// extra info for error messages TODO ??
				} else {
					pushError(&ret, fmt.Sprintf("//* Error: compile error in constant expression %q in #define etc\n", translate))
				}
			}
			/***********************************************************************
			 *  %define X(a,b) (a * b) is   map defs[X] = { 2, "(#1 * #2)" }
			 ***********************************************************************/
			defs[identifier] = &strNum{ num: pCnt, str: translate }
		}
		return identifier, val
	}
	pushError(&w, fmt.Sprintf("//* Warning: scan_define: %s '%s' has bad characters - ignored\n", direcve, macro))
	return "", ""
} // scan_define

/***********************************************************************
 *
 *  Resolve %%define, %define and #define macros
 *
 *  Two types of macros are resolved:
 *
 *      Object like macros which look like a C style word.
 *
 *      Function like macros where the word is followd by comma separated
 *      arguments in parentheses.
 *
 ***********************************************************************/

func resolve(word string, pos int, posNextP, offsetP *int, alo int, textP, textResP *string) (_, _ int, ex bool, _ bool) {
	var ref *strNum
	var macroArgs    = make([]string, 5)
	var macroExps    = make([]string, 5)
	if ref, ex = defs[word]; ex {		// exists
		ald  := 0
		if *opt_Z { fmt.Printf("Resolve start: ald = %d alo = %d ex = %v\n", ald, alo, ex) }
		al   := len(word)
		num  := ref.num
		tran := ref.str					// "15"
		posOff := pos + *offsetP
		if num >= 0 {
			if *opt_Z { fmt.Printf("### resolve function macro %q\n    word = %q pos = %d *offsetP = %d num = %d tran = %q\n", *textP, word, pos, *offsetP, num, tran) }
			/***********************************************************************
			 *  Resolve one function macro call.
			 *  1) extract comma separated function macro arguments in parentheses.
			 *  2) each argument may contain embedded expressions in parentheses
			 *     - parentheses in each argument must balance.
			 *  3) a comma inside parentheses does not end the argument.
			 *  4) there is no requirement for square brackets and braces to balance
			 *     in an argument.
			 *  5) all arguments are completely macro-expanded before they are
			 *     subbstituted into the macro body except stringified resoltions,
			 *     which substitute the original argument in double quotes.
			 *     This means the original argument must be saved before the argument
			 *     is macro-expanded.
			 *  6) after substitution the complete text is scanned again, including
			 *     the arguments. No substitution is carried out in quoted strings
			 *     included strings generated by stringification.
			 ***********************************************************************/
			var lPos, lPose, parenCnt int
			iNext     := 0
			atom      := ""
			state     := 'A'						// outside of string, character constant or comment
			prevState := state
			argStart  := 0
			wordStart := 0
			expStart  := 0
			aCnt      := 0
			lPosNext  := 0
			lOffset   := 0
			lText     := (*textP)[pos:]				// local text slice starting with word to be resolved
			lTextRes  := (*textResP)[posOff:]		// resolved text slice starting with word to be resolved
			arg       := ""
		  Rescan:
			for {
				lPose     = 0
				//	reResolve: (\(|[A-Z_a-z]\w*|,|\)|\\?["'])
				for i, lPosPair := range reResolve.FindAllStringIndex(lText, -1) {
					//	m/
					//	(						# $1
					//		\(				|	#    opening parentheses
					//		[A-Z_a-z]\w*	|	#	 word in argument
					//		,				|	#    comma
					//		\)				|	#    closing parentheses
					//		\\?["']				#    string or character constant delimiters
					//	)
					//	/xg
					lPose  = lPosPair[1]
					if i < iNext {
						continue					// skip tokens in text already scanned
					}
					lPos  = lPosPair[0]
					lPosOff := lPos + lOffset
					atom = lText[lPos : lPose]
					macroCall := ""
					if *opt_Z { fmt.Printf("lPosPair = %v lPos = %d atom = `%s`\n", lPosPair, lPos, atom) }
					if i == 0 {
						if atom == word {			// is first atom function macro name?
							continue				// yes - look for arguments in parentheses
						} else {
							panic("resolve: call")	// no - should not happen if resolve called correctly
						}
					}
					if i == 1 && atom != "(" {
						break Rescan				// function macro called as object
					}
					if state == 'A' {
						if atom == "(" {
							parenCnt++
							if parenCnt == 1 {
								argStart  = lPose
								wordStart = lPose
								expStart  = lPose + lOffset
								aCnt = 0
							}
						} else if atom == "," || atom == ")" {
							if atom == ")" {
								parenCnt--
								if parenCnt == 0 {
									al = lPose			// length of function macro call
									macroCall = lText[0:lPose]		// for error message
									if *opt_Z { fmt.Printf("lPose = %d al = %d macroCall = %q\n  lText    = %q\n *textP    = %q\n  lTextRes = %q\n *textResP = %q\n", lPose, al, macroCall, lText, *textP, lTextRes, *textResP) }
									state = 'R'		// start argument replacement in macro body
								}
							}
							if parenCnt == 1 && atom == "," || parenCnt == 0 && atom == ")" {
								aCnt++				// first argument is #1
								if aCnt >= cap(macroArgs) {
									macroArgs = append(macroArgs, make([]string, aCnt)...)	// double capacity
									macroExps = append(macroExps, make([]string, aCnt)...)
								}
								arg += lText[wordStart:lPos]	// trailing text after last word in this argument
								if *opt_Z { fmt.Printf("argument end:   atom = %q arg = %q lText = %q wordStart = %d lPos = %d lPose = %d\n", atom, arg, lText, wordStart, lPos, lPose) }
								arg = reLdTrSps.ReplaceAllLiteralString(arg, "")	// remove leading and trailing spaces
								arg = reSpaces.ReplaceAllLiteralString(arg, " ")	// compress spaces to 1 space
								macroArgs[aCnt] = arg	// store unexpanded argument for stringizing and concatenating
								if *opt_X { fmt.Printf("+++%d+ arg  = %q\n",aCnt , arg) }
								exp := lTextRes[expStart:lPosOff]
								exp = reLdTrSps.ReplaceAllLiteralString(exp, "")	// remove leading and trailing spaces
								exp = reSpaces.ReplaceAllLiteralString(exp, " ")	// compress spaces to 1 space
								macroExps[aCnt] = exp	// store expanded argument for normal replacement
								if *opt_X { fmt.Printf("+++%d+ exp  = %q\n",aCnt , macroExps[aCnt]) }
								if *opt_Z { fmt.Printf("[%d] argStart = %d lPos    = %d lText    = %q arg = %q\n*   expStart = %d lPosOff = %d lTextRes = %q exp = %q\n", aCnt, argStart, lPos, lText, arg, expStart, lPosOff, lTextRes, exp) }
								arg = ""							// ready for next argument
								argStart  = lPose		// start of next argument if ',' else start of next text
								wordStart = lPose		// start of next word in original argument with preceding text
								expStart  = lPose + lOffset
							}
						} else if reWord.MatchString(atom) {
							if lPos >= wordStart {
								arg += lText[wordStart:lPose]	// next word from an original argument
								wordStart = lPose
							}
							if *opt_Z { fmt.Printf("before resolve: atom = %q arg = %q lText = %q lPosNext = %d wordStart = %d lPos = %d lPose = %d lOffset = %d\n", atom, arg, lText, lPosNext, wordStart, lPos, lPose, lOffset) }
							/********************************************************************
							 *  atom   word to be resolved
							 *  lPos starting position of atom in text
							 *
							 *  ok     true if atom is in map defs and was resolved
							 *         else false - do nothing with this word
							 *  rs     true if Rescan required after resolve
							 *
							 *  words in strings and character constants are not re-scanned
							 *  comments have been removed previously
							 *******************************************************************/
							var ok, rs bool
							if ald, alo, ok, rs = resolve(atom, lPos, &lPosNext, &lOffset, alo, &lText, &lTextRes); ok {
								if *opt_Z { fmt.Printf("after resolve: atom = %q arg = %q lText = %q lPosNext = %d wordStart = %d lPos = %d lPose = %d lOffset = %d\n", atom, arg, lText, lPosNext, wordStart, lPos, lPose, lOffset) }
								if lPosNext > wordStart {
									wordStart = lPosNext	// start of further words in this argument if changed
								} else if lPos < wordStart {
									wordStart += ald		// modify wordStart to account for change in previous replacement
								}
								if *opt_Z { fmt.Printf("after resolve: atom = %q arg = %q lText = %q lPosNext = %d wordStart = %d lPos = %d lPose = %d lOffset = %d\n", atom, arg, lText, lPosNext, wordStart, lPos, lPose, lOffset) }
								if rs {
									iNext = i		// skip over tokens already scanned
									if *opt_X { fmt.Printf("*** resolve %d skip after '... %q ==> %q ...' with iNext = %d lPosOff = %d lPosNext = %d\n", lineNo, atom, lTextRes[lPosOff:lPosNext], iNext, lPosOff, lPosNext) }
									continue Rescan	// re-start scan of same text with token at current position
								}					// which may not be a word
							}
						} else if atom == "\"" {	// leading backslash will not compare
							prevState = state
							state = 'S'				// start of string found
						} else if atom == "'" {		// leading backslash will not compare
							prevState = state
							state = 'H'				// start of character constant found
						}
					} else if state == 'R' {		// resolve macro
						if num != aCnt {
							pushError(&ret, fmt.Sprintf("//* Error: function macro call %q has %d arguments - should be %d\n", macroCall, aCnt, num))
							return ald, alo, false, false		// no change to macro call TODO check
						}
						if *opt_Z { fmt.Printf("resolve: tran before replacement = %q\n", tran) }
						if *opt_X { fmt.Printf(">>>>> tran = %q\n", tran) }
						for i := 1; i <= aCnt; i++ {
							is := strconv.Itoa(i)
							arg := macroArgs[i]					// unexpanded argument
							// replace stringified argument in translate
							reStringify := regexp.MustCompile(`#\\#`+is)
							if reStringify.MatchString(tran) {	// is this argument stringified ?
								arg = reQuoteSlash.ReplaceAllString(arg, "\\$1")	// yes - precede " and \ by extra \
								if *opt_X { fmt.Printf("===%d= arg  = %q\n",i , arg) }
								tran = reStringify.ReplaceAllLiteralString(tran, "\""+arg+"\"")
								if *opt_X { fmt.Printf("===%d= tran = %q\n",i , tran) }
							}
							reReplace := regexp.MustCompile("#"+is)
							reConcatenate := regexp.MustCompile(`(#\\#\\\s+#`+is+`|#`+is+`\s+#\\#\\)`)
							if reConcatenate.MatchString(tran) {	// is this argument part of a concatanation ?
								tran = reReplace.ReplaceAllLiteralString(tran, arg)	// yes - use unexpanded argument
							} else {
								// replace normal argument in translate
								tran = reReplace.ReplaceAllLiteralString(tran, macroExps[i])
							}
							if *opt_X { fmt.Printf("<<<<< tran = %q\n", tran) }
							if *opt_Z { fmt.Printf("resolve: tran after replacement  = %q macroExps[%d] = %q\n", tran, i, macroExps[i]) }
						}
						tran = reHashSlash.ReplaceAllLiteralString(tran, "#")	// re-constitute protected #'s including ##
						tran = reHashHash.ReplaceAllLiteralString(tran, "")		// concatenete parameters separated by ' ## '
						break Rescan				// function macro expansion complete
					} else if state == 'S' {		// string constant
						if atom == "\"" {			// leading backslash will not compare
							state = prevState		// end of string constant
						}
					} else if state == 'H' {		// character constant
						if atom == "'" {			// leading backslash will not compare
							state = prevState		// end of character constant
						}
					}
				}
			}
			al += alo
		}
		if posOff >= *posNextP {
			Used = map[string]string{}	// clear map Used
		}
		if t, u := Used[word]; !u || t != tran {
			if !u {
				Used[word] = tran
			}
			if *opt_Z { fmt.Printf("word = %q posOff = %d *posNextP = %d alo = %d al = %d ex = %v tran = %q\n", word, posOff, *posNextP, alo, al, ex, tran) }
			if *opt_Z { fmt.Printf("*textResP before substr = %q\n", *textResP) }
			*textResP = substr(*textResP, posOff, al, tran)
			if *opt_Z { fmt.Printf("*textResP after substr  = %q\n", *textResP) }
			*textP = *textResP
			*offsetP = 0
			lt      := len(tran)
			if num >= 0 {
				alo = 0
			}
			ald = lt - al
			alo -= ald
			*posNextP = posOff + lt
			if *opt_X { fmt.Printf("word = %q posOff = %d *posNextP = %d alo = %d al = %d ex = %v tran = %q\n", word, posOff, *posNextP, alo, al, ex, tran) }
			if *opt_Z { fmt.Printf("Resolve end re-start: ald = %d alo = %d ex = %v tran = %q used = %v\n", 0, alo, ex, tran, Used) }
			return ald, alo, ex, true		// re-start scan of same text with token at current position
		}								// which may not be a word
		if *opt_Z { fmt.Printf("Resolve end: ald = %d alo = %d ex = %v tran = %q used = %v\n", 0, alo, ex, tran, Used) }
		if *opt_X { fmt.Printf("word = %q posOff = %d tran = %q al = %d ex = %v\n", word, posOff, tran, al, ex) }
	}
	return 0, alo, ex, false				// do not rescan
} // resolve

/***********************************************************************
 *
 *  Push errors and warnings as they occur
 *
 ***********************************************************************/

func pushError(r *int, errorMsg string) {
	if !*opt_t {
		lnErr = append(lnErr, errorMsg)		// push error or warning message
		*r++								// count error or warning
	}
} // pushError

/***********************************************************************
 *
 *  Output errors and warnings after completing analysis of one line.
 *
 ***********************************************************************/

func outputError(listLine string) {
	listLine = fmt.Sprintf("//* %s:%d: %q\n", argIn, lineNo, listLine)	// precede error message(s) by one listing line
	listFlag  := true
	wListFlag := true
	for _, e := range lnErr {
		if listFlag {
			fmt.Fprint(os.Stderr, listLine)
		}
		fmt.Fprint(os.Stderr, e)		// output errors and warnings to terminal or error file
		if *opt_l != "" {
			if listFlag {
				fmt.Fprint(genFile, listLine)
			}
			fmt.Fprint(genFile, e)		// and to generated GO file for log
		}
		listFlag = false
		if (*opt_o != "" || *opt_e != "") {
			if wListFlag {
				fmt.Print(listLine)
				wListFlag = false
			}
			fmt.Print(e)				// and error messages to output file
		}
	}
	lnErr = []string{}
} // outputError

/***********************************************************************
 *
 *  Auxiliary function for evalIf() to convert and replace a word or
 *  a number in *exprP
 *  Ignore if not eval
 *
 ***********************************************************************/

func replaceAtom(atom string, pos int, posNextP *int, macroP, exprP *string, offsetP *int, i int, iNextP *int, eval bool) (rs bool) {
	if eval && !reNum.MatchString(atom) {
		var ok, rs bool
		if *opt_X { fmt.Printf("beforeReplace: atom = %q pos = %d *exprP = %q *offsetP = %d, rs = %v\n", atom, pos, *exprP, *offsetP, rs) }
		posOff := pos + *offsetP
		if _, _, ok, rs = resolve(atom, pos, posNextP, offsetP, 0, macroP, exprP); ok {
			if rs {
				*iNextP = i			// skip over tokens already scanned
				if *opt_Z { fmt.Printf("*** %d skip after '... %q ==> %q ...' with *iNextP = %d posOff = %d *posNextP = %d\n", lineNo, atom, (*exprP)[posOff:*posNextP], *iNextP, posOff, *posNextP) }
				return true			// re-start scan of same text with token at current position
			}						// which may not be a word
		} else {
			al := len(atom)
			if *opt_X { fmt.Printf("replaceDefault: atom = %q posOff = %d *exprP = %q *offsetP = %d, rs = %v\n", atom, posOff, *exprP, *offsetP, rs) }
			*exprP = substr(*exprP, posOff, al, "0")	// "0" is default for undefined macro value
			*posNextP = posOff + 1
		}
		if *opt_X { fmt.Printf("replaceAtom: atom = %q posOff = %d *exprP = %q *offsetP = %d, rs = %v\n", atom, posOff, *exprP, *offsetP, rs) }
	}										// number does not need to be replaced
	return false
} // replaceAtom

/***********************************************************************
 *
 *  Auxiliary function for evalIf() to replace an arithmetic expression
 *  in *exprP
 *
 ***********************************************************************/

func replaceExpr(posStart int, posEnd int, arHead, arTail string, exprP *string, offsetP *int) {
	al := posEnd - posStart
	arCond := arHead + (*exprP)[posStart:posEnd] + arTail
	bl := len(arCond)
	*exprP = substr(*exprP, posStart, al, arCond)
	*offsetP -= al - bl
	if *opt_X { fmt.Printf("replaceExpr: posStart = %d posEnd = %d al = %d arCond = %q bl = %d *exprP = %q *offsetP = %d\n", posStart, posEnd, al, arCond, bl, *exprP, *offsetP) }
} // replaceExpr

/***********************************************************************
 *
 *  Generate a boolean expression for the conditional directives #if #ifdef
 *  #ifndef as well as #elif. Compile the code with 'govaluate' and eval it.
 *
 *  #ifdef and #ifndef are handled by concatenating "defined " or "!defined "
 *  with 'macro' and setting 'def' = true in the call to evalIf().
 *  In that case 'macro' must be a single directive name which is either
 *  defined or not - if macro is an expression or a number an Error is reported.
 *
 *  Syntax errors with logical operators && and || as well as conditional
 *  operators == != <= etc are reported early before 'govaluate' compilation.
 *
 *  Syntax errors with all other arithmetic operators, parentheses or
 *  other symbols are ignored here. They are reported by 'govaluate'.
 *
 *  Alternatively just modify 'int' expression to 'bool' expression for
 *    IF (expr) {{              ==> IF (expr != 0) {{
 *  and
 *    [ expr1 ? expr2 : expr3 ] ==> [ expr1 != 0 ? expr2 : expr3 ]
 *
 ***********************************************************************/

func evalIf(macro string, def bool, eval bool) (int, string) {
	expr      := macro
	macroLen  := len(macro)
	if *opt_T { fmt.Printf("//1 %d: evalIf('%s' %v)\n", lineNo, macro, eval) }
	remove_comment(&macro)						// take out individual C comments TODO check if necessary
	if *opt_T { fmt.Printf("//2 %d: evalIf('%s' %v)\n", lineNo, macro, eval) }
	if *opt_X { fmt.Printf("//2 %d	%s length = %d, %v\n", lineNo, macro, macroLen, eval) }
	var pos, pose, prevPose, offset int
	posEstart := 0
	posTstart := 0
	iNext     := 0
	iOffset   := 0
	posNext   := 0
	atom      := ""
	atomPrev  := ""
	state     := 'L'						// initial state
	ternFlg   := false
	ternCnt   := 0
  Rescan:
	for {
		pose     = 0
		//	reEvalIf: (!\s*defined|[a-z_A-Z]\w*|([+-]\s*)?\d+|&&|\|\||[\?:\(\)]|[=!<>]=|<<?|>>?)
		for i, posPair := range reEvalIf.FindAllStringIndex(macro, -1) {
			//	(
			//		!\s*defined		|	# special word defined with logical !
			//		[a-z_A-Z]\w*	|	# word identifier (includes defined without !)
			//		([+-]\s*)?\d+	|	# integer number with optional sign
			//		&&				|	# && logical and
			//		\|\|			|	# || logical or
			//		[\?:\(\)]		|	# ternary operators ? : and parentheses
			//		[=!<>]=			|	# == != <= or >= conditionals
			//		<<?				|	# < conditional or << operator
			//		>>?					# > conditional or >> operator
			//	)
			prevPose = pose						// start of white space after previous atom
			pose  = posPair[1]
			if i < iNext {
				continue						// skip tokens in text already scanned
			}
			atomPrev = atom
			pos      = posPair[0]
			atom     = macro[ pos : pose ]
			wo      := false
			nu      := false
			wordNum := false
			logicOp := false
			ternOp  := false
			condOp  := false
			if *opt_Z { fmt.Printf("'%c' %2d  atom = %-5s posPair = %-3v macro = %q iN = %d iO = %d ternCnt = %d ternFlg = %v\n", state, i, atom, posPair, macro, iNext, iOffset, ternCnt, ternFlg) }
			/***********************************************************************
			 *  State machine to parse and evaluate a boolean expression for #if etc.
			 *  {{{{{{{
			 *                                                /--\ "defined"  \
			 *                                           +---+   | number cond } error
			 *                                        /->| D |<--/ && || ? :  /
			 *                                       /   +---+
			 *                                      /      |
			 *                            "defined"/       | word
			 *                                    /        |
			 *                   cond && || ? :  /         |
			 *                      } error     /          |
			 *                        /--\     /  &&       V  /--\ word   \
			 *                        |   +---+   ||     +---+   | number  } error
			 *                        \-->| L |<---------| F |<--/ cond : /
			 *       START--------------->+---+<--\      +---+-----------------\
			 *                              | ^    \                            \
			 *                       word   |  \    \-----------\                \
			 *                       number |   \                \               |
			 *        /---------------\     |   | &&              \ &&           |
			 *       /  /-------\ ?    \ ?  |   | ||              | ||           |
			 *       | /         \   /--\---|---|------\ cond     |              |
			 *       V V number  |  /    \  V  /       V   number |  /--\        |
			 *    />+---+ word +---+  :   +---+ cond +---+ word +---+   | cond : |
			 *   /  | G |----->| H |----->| E |----->| M |----->| N |<--/} error |
			 *  / />+---+      +---+      +---+      +---+      +---+---\        |
			 *  | | ^    \     ^    \ &&  ^    \     ^    \     ^    \   \       |
			 *  | | |    |     |    | ||  |    |     |    |     |    |   |       |
			 *  | | \----/     \----/ }er \----/     \----/     \----/   | ?     | ?
			 *  | |cond && ||   word       : word  cond && ||    word    |       |
			 *  \ \? :} error   number     number  ? :} error    number  /       /
			 *   \ \----------------------------------------------------/       /
			 *    \------------------------------------------------------------/
			 *
			 *    *=========================================================*
			 *    |  atom \ state   |  L |  D |  F |  E |  G |  H |  M |  N |
			 *    *=========================================================*
			 *    |  (!\s*)?defined |  D | er |  - |  - |  - |  - |  - |  - |
			 *    |  word   wo      |  E |  F | er |  E |  H |  H |  N |  N |
			 *    |  number nu      |  E | er | er |  E |  H |  H |  N |  N |
			 *    +-----------------+----+----+----+----+----+----+----+----+
			 *    |  &&     logicOp | er | er |  L |  L | er | er | er |  L |
			 *    |  ||             | er | er |  L |  L | er | er | er |  L |
			 *    +-----------------+----+----+----+----+----+----+----+----+
			 *    |  ?      ternOp  | er | er |  G |  G | er | er | er |  G |
			 *    |  :              | er | er | er |  E | er |  E | er | er |
			 *    +-----------------+----+----+----+----+----+----+----+----+
			 *    |  < <= ..condOp  | er | er | er |  M | er |  M | er | er |
			 *    *=========================================================*
			 *
			 *  All defined variables should have a numerical string value or return
			 *  the constant bool string "true" if preceded by the word "defined".
			 *  A defined variable whose value has not been assigned returns "1".
			 *  All undefined variables return the arithmetic string "0" or "false" if
			 *  preceded by "defined".
			 *    bool expr       expression
			 *  true            defined A   // if A is defined with -D or -P or #define
			 *  false           defined B   // if B has not been defined
			 *  !false          !defined B
			 *  2 != 0          X           // if X has been defined with -D X=2
			 *  2 + 3 != 0      X + Y       // if Y has been defined with -D Y=3
			 *  2 <= 4          X <= Z      // if Z has been defined with -D Z=4
			 *  2 != 0 && 3 != 0        X && Y
			 *  2 < 4 || 3 > 4          X < Z || Y > Z
			 *  !(2 < ~4 || 3 > 4)      !(X < ~Z || Y > Z)
			 *  (X < Y ? X : Y) != 0    X < Y ? X : Y
			 *  ((X < Y ? X : Y)) > 2   (X < Y ? X : Y) > 2
			 *  Spaces, all operators and parentheses - in fact all symbols except
			 *  words, which are replaced by their defined value, are copied exactly.
			 *  Parenthese are placed around a ternary expression, because ternary
			 *  expressions have lower precedence than conditionals.
			 *  Care is taken to make leading unary operators ! and - part of the
			 *  atoms following them. These can only be !defined and -123 (a number).
			 ***********************************************************************/
			if reWord.MatchString(atom) {
				wo      = true
				wordNum = true
			} else if reNum.MatchString(atom) {
				nu      = true
				wordNum = true
			} else if atom == "&&" || atom == "||" {
				logicOp = true
			} else if atom == "?" || atom == ":" {
				ternOp = true
			} else if atom == "==" || atom == "!=" || atom == "<" || atom == "<=" || atom == ">" || atom == ">=" {
				condOp = true
			}
			if state == 'L' {
				if loc := reDefined.FindStringIndex(expr); loc != nil {
					if expr[loc[0]] == '!' {
						loc[0]++							// retain leading !
					}
					dl := loc[1] - loc[0]
					expr = substr(expr, loc[0], dl, "")		// remove first "\s*defined/s*"
					offset -= dl
					state = 'D'
				} else if wordNum {
					posEstart = prevPose + offset			// start of an arithmetic expression with current offset
					posTstart = posEstart
					if wo {
						if replaceAtom(atom, pos, &posNext, &macro, &expr, &offset, i + iOffset, &iNext, eval) {
							continue Rescan
						}
					}
					state = 'E'
				} else if logicOp || condOp || ternOp {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q directly after %q in #if %s\n", atom, atomPrev, macro))
				}
			} else if state == 'D' {
				if atom == "defined" {						// must test before word check
					pushError(&ret, fmt.Sprintf("//* Error: \"#ifdef defined\" or \"#if defined defined\" in #if %s\n", macro))
				} else if wo {
					var boolString string
					al := len(atom)
					if _, ok := defs[atom]; ok {
						boolString = "<< >>"				// true  - must be 2 tokens for rescan
					} else {
						boolString = ">> <<"				// false - to replace 'defined Word'
					}
					bl := len(boolString)
					expr = substr(expr, pos + offset, al, boolString)
					offset -= al - bl
					state = 'F'
				} else if nu || logicOp || condOp || ternOp {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q in \"defined %s\" in #if %s\n", atom, atom, macro))
				}
			} else if state == 'F' {
				if def {
					pushError(&ret, fmt.Sprintf("//* Error: expression token %q illegal in #ifdef or #ifndef %s\n", atom, macro))
				} else if logicOp {
					state = 'L'
				} else if atom == "?" {
					ternFlg = true
					ternCnt++
					state = 'G'
				} else if condOp || wordNum || atom == ":" {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q after \"defined %s\" in #if %s\n", atom, atomPrev, macro))
				}
			} else if state == 'E' {
				iOffset = 0
				if logicOp && ternCnt == 0 {
					if ternFlg {
						replaceExpr(posTstart, prevPose + offset, "(", ") != 0", &expr, &offset)
						iOffset = 4
						ternFlg = false
					} else {
						replaceExpr(posEstart, prevPose + offset, "", " != 0", &expr, &offset)
						iOffset = 2
					}
					state = 'L'
				} else if atom == "?" {
					replaceExpr(posEstart, prevPose + offset, "", " != 0", &expr, &offset)
					iOffset = 2
					ternFlg = true
					ternCnt++
					state = 'G'
				} else if atom == ":" {
					if ternCnt > 0 {
						ternCnt--						// remain in state 'E' for ":"
					} else {
						pushError(&ret, fmt.Sprintf("//* Error: too many \":\" tokens in #if %s\n", macro))
					}
				} else if condOp {
					state = 'M'
				} else if wo {							// remain in state 'E' for word or number
					if replaceAtom(atom, pos, &posNext, &macro, &expr, &offset, i + iOffset, &iNext, eval) {
						continue Rescan
					}
				}
			} else if state == 'G' {
				if wordNum {
					posEstart = prevPose + offset		// start of an arithmetic expression with current offset
					if wo {
						if replaceAtom(atom, pos, &posNext, &macro, &expr, &offset, i + iOffset, &iNext, eval) {
							continue Rescan
						}
					}
					state = 'H'
				} else if logicOp || condOp || ternOp {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q directly after %q in #if %s\n", atom, atomPrev, macro))
				}
			} else if state == 'H' {
				if atom == ":" {
					if ternCnt > 0 {
						ternCnt--
					} else {
						pushError(&ret, fmt.Sprintf("//* Erron: too many \":\" tokens in #if %s\n", macro))
					}
					state = 'E'
				} else if atom == "?" {
					replaceExpr(posEstart, prevPose + offset, "", " != 0", &expr, &offset)
					iOffset = 2
					ternFlg = true
					ternCnt++
					state = 'G'
				} else if condOp {
					state = 'M'
				} else if wo {							// remain in state 'H' for word or number
					if replaceAtom(atom, pos, &posNext, &macro, &expr, &offset, i + iOffset, &iNext, eval) {
						continue Rescan
					}
				} else if logicOp {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q directly after %q in #if %s\n", atom, atomPrev, macro))
				}
			} else if state == 'M' {
				if wordNum {
					if wo {
						if replaceAtom(atom, pos, &posNext, &macro, &expr, &offset, i + iOffset, &iNext, eval) {
							continue Rescan
						}
					}
					state = 'N'
				} else if logicOp || condOp || ternOp {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q directly after %q in #if %s\n", atom, atomPrev, macro))
				}
			} else if state == 'N' {
				if logicOp && ternCnt == 0 {
					state = 'L'
				} else if atom == "?" {
					ternFlg = true
					ternCnt++
					state = 'G'
				} else if wo {							// remain in state 'N' for word or number
					if replaceAtom(atom, pos, &posNext, &macro, &expr, &offset, i + iOffset, &iNext, eval) {
						continue Rescan
					}
				} else if condOp || atom == ":" {
					pushError(&ret, fmt.Sprintf("//* Error: wrong token %q after conditional expression in #if %s\n", atom, macro))
				}
			}
			/***********************************************************************
			 *  atom "<<" and ">>" as well as all other symbols not found by this
			 *  scan are not modified. If any of those symbols form an incorrect
			 *  boolean expression it is reported by the 'govaluate' compile stage.
			 ***********************************************************************/
			if *opt_X { fmt.Printf("evalIf:	'%c' [ %d : %d ]	offset = %d	%q\n\t%s\n", state, pos, pose, offset, atom, expr) }
			if *opt_Z { fmt.Printf("                                         expr  = %q\n", expr) }
		}
		break Rescan
	}
	if *opt_Z { fmt.Printf("'%c'                                      macro = %q iN = %d iO = %d ternCnt = %d ternFlg = %v\n", state, macro, iNext, iOffset, ternCnt, ternFlg) }
	if state == 'E' {
		if ternFlg {
			replaceExpr(posTstart, pose + offset, "(", ") != 0", &expr, &offset)
			ternFlg = false
		} else {
			replaceExpr(posEstart, pose + offset, "", " != 0", &expr, &offset)
		}
	} else if state == 'L' || state == 'D' || state == 'M' {
		pushError(&ret, "//* Error: incomplete boolean expression in #if etc\n")
	}		// can also terminate correctly in state 'N' and state 'F' with a boolean constant
	if *opt_T { fmt.Printf("//3 %d: evalIf(%s) ==> %q\n", lineNo, macro, expr) }
	/********************************************************************
	 *  << >> is changed to true  - this allows 'true' to be used as a variable
	 *  >> << is changed to false - this allows 'false' to be used as a variable
	 *******************************************************************/
	expr = reTrue.ReplaceAllLiteralString(expr, "true")
	expr = reFalse.ReplaceAllLiteralString(expr, "false")
	if *opt_Z { fmt.Printf("                                         expr  = %q\n", expr) }
	if *opt_X { fmt.Printf("//3 %d:	%s\n\t%s\n", lineNo, macro, expr) }
	if ret == 0 && eval {
		/********************************************************************
		 *  if no errors in the above analysis
		 *  use 'govaluate' to compile and evaluate the boolean expression
		 *******************************************************************/
		if expression, errExp := govaluate.NewEvaluableExpression(expr); errExp == nil {
			if result, errEval := expression.Evaluate(nil); errEval == nil {
				if *opt_Z { fmt.Printf("go-repl> %v\n", result) }
				if result.(bool) {				// above analysis ensures bool - else panic
					return 2, expr				// true
				} else {
					return -1, expr				// false
				}
			} else {
				pushError(&ret, fmt.Sprintf("//* Error: goevaluate error %q in boolean expression %q in #if etc\n", errEval, expr))
			}
		} else {
			pushError(&ret, fmt.Sprintf("//* Error: compile error %q in boolean expression %q in #if etc\n", errExp, expr))
		}
	}
	return -1, expr									// false on error or not eval
} // evalIf

/***********************************************************************
 *
 *  Take out individual C comments out of ref
 *
 ***********************************************************************/

func remove_comment(ref *string) {
	var r = make([]string, 0)
	for _, x := range reCcomTail.Split(*ref, -1) {
		x = reCcomment.ReplaceAllLiteralString(x, "")	// take out individual C comments
		r = append(r, x)
	}
	*ref = strings.Join(r, " ")
} // remove_comment

/***********************************************************************
 *
 *  Generate a line to file 'genName' for execution as a GO file
 *
 ***********************************************************************/

func genLine(tx, tl string) {
	if opt_m_M_Y && blankLines > 0 { // blankLines is at least 1 because it is a directive
		blanks()				// print blank lines now
	}
	if *opt_T { fmt.Printf("//* %3d: %q %q\n", lineNo , tx, tl) }
	tx = reQuoteSlash.ReplaceAllString(tx, "\\$1")	// precede " and \ by extra \
	fmt.Fprintf(genFile, "\tfmt.Print(\"%s%s\")\n", tx, tl)	// tl is either "\\n" or "" for unfinished line
	genFlag = tl == ""
} // genLine

/***********************************************************************
 *
 *  Append text to genSlice and set LFflag
 *
 ***********************************************************************/

func genSliceAppend(tx string) {
	genSlice = append(genSlice, tx)
	LFflag = true
	if *opt_Z { fmt.Fprintf(genFile, "//= LFflag = true at in genSliceAppend(%q)\n", tx) }
} // genSliceAppend

/***********************************************************************
 *
 *  Generate a line containing expressions to file 'genName' for execution as a GO file
 *
 ***********************************************************************/

func genSliceOut(tl string) {
	if opt_m_M_Y && blankLines > 0 { // blankLines is at least 1 because it is a directive
		blanks()				// print blank lines now
	}
	if FORline != "" {
		fmt.Fprint(genFile, FORline)
		FORline = ""
	}
	if *opt_T { fmt.Printf("//* %3d: %q %q\n", lineNo , genSlice, tl) }
	tx := strings.Join(genSlice, ", ")
	fmt.Fprintf(genFile, "\tfmt.Print(%s%s)\n", tx, tl)	// tl is either ", \"\\n\"" or "" for unfinished line
	genSlice = make([]string, 0, cap(genSlice))
	genFlag = tl == ""
} // genSliceOut

/***********************************************************************
 *
 *  Print a group of blank lines (only when *opt_m *opt_M or *opt_Y)
 *
 ***********************************************************************/

func blanks() {
	if *opt_T { fmt.Println("//*", lineNo, ":	blankLines =", blankLines) }
	if blankLines < 8 || *opt_Y {		// yacc does not understand # line_no //
		for n := 0; n < blankLines; n++ {
			fmt.Fprint(genFile, "\tfmt.Print(\"\\n\")\n") // output block of 7 or less blank lines as is
		}
	} else {
		tx := fmt.Sprintf("# %d \\\"%s\\\"", lineNo, argIn) // # 23 "file"
		fmt.Fprintf(genFile, "\tfmt.Print(\"%s\\n\")\n", tx) // block of 8 or more blank lines
	}
	blankLines = 0
} // blanks

/***********************************************************************
 *
 *  Expand a comment text with the value of the control variable cv
 *
 ***********************************************************************/

func expandCtrlVar(cv string) string {
	cvType, ok := FORctrlVar[cv]
	if !ok {
		return cv								// cv not found - no change
	}
	if cvType == "string" {
		return fmt.Sprintf("(%s=%%[%d]q)", cv, FORctrlIndex[cv])
	}
	return fmt.Sprintf("(%s=%%[%d]d)", cv, FORctrlIndex[cv])
} // expandCtrlVar

/***********************************************************************
 *
 *  Edit a forLine comment for opt_a
 *
 ***********************************************************************/

func opt_aEdit(commentP *string, reject string) string {
	var ctrlVarParams string
	if *opt_a {
		FORctrlIndex = make(map[string]int)
		i := 1
		ctrlVars := make([]string, 0, len(FORctrlVar))
		for cv, _ := range FORctrlVar {
			if cv != "int" && cv != reject {
				ctrlVars = append(ctrlVars, cv)
				ctrlVarParams += ", "+cv		// build comma separated parameter list
				FORctrlIndex[cv] = i			// in the same order as index i
				i++
			}
		}
		if len(ctrlVars) > 0 {
			*commentP = rePercent.ReplaceAllLiteralString(*commentP, "%%")
			s := strings.Join(ctrlVars, "|")
			reCtrlVar := regexp.MustCompile("\\b("+s+")\\b")
			if reCtrlVar.MatchString(*commentP) {
				*commentP = reCtrlVar.ReplaceAllStringFunc(*commentP, expandCtrlVar)
			} else {
				ctrlVarParams = ""
			}
		}
	}
	return ctrlVarParams
} // opt_aEdit

/***********************************************************************
 *
 *  Print a comment for opt_a
 *
 ***********************************************************************/

func opt_aComment(comment, params string) {
	if *opt_a {
		blankLines--
		if genFlag {
			genLine("", "\\n")	// this clears genFlag = false
		}
		if len(params) > 0 {
			comment = reQuoteSlash.ReplaceAllString(comment, "\\$1")	// precede " and \ by extra \
			fmt.Fprintf(genFile, "\tfmt.Printf(\"//***** %s\\n\"%s)\n", comment, params)
		} else {
			genLine("//***** "+comment, "\\n")
		}
	}
} // opt_aComment

/***********************************************************************
 *
 *  Emulate the Perl substr() function
 *  substr(expr, offset, length, replacement)
 *
 ***********************************************************************/

func substr(expr string, offset, length int, replacement string) string {
	if *opt_X { fmt.Printf("substr: expr   = %q offset = %d length = %d replacement = %q\n", expr, offset, length, replacement) }
	x := []rune(expr)
	x = append(x[:offset], append([]rune(replacement), x[offset+length:]...)...)
	if *opt_X { fmt.Printf("substr: return = %q\n", string(x[:])) }
	return string(x[:])
} // substr

/*********** POD to generate man page *******************************

=encoding utf8

=head1 NAME

immac - the immediate-C array and macro pre-compiler (GO version)

=head1 SYNOPSIS

B<immac> [-tmaSNLTh][ -o <out>][ -l <log>][ -e <err>][ -I <path> ...]
      [ -P <macro>[=<def>] ...][ -D <macro>[=<def>] ...][ -U <macro>]
      [ <in.ica>| <in.ic>]

=head1 OPTIONS

    -o <out>   name of generated output file         (default: stdout)
    -l <log>   name of Perl log file                 (default: none)
    -e <err>   name of error file                    (default: stderr)
    -t         test if source is an iCa language file - return 1 if it is
    -m         process only #include #define #if etc directives (see <in.ic>)
               No iCa language translation (macro pre-processor replaces cpp)
               Warning if %include %define %if etc directives are used
    -M         process only %include %define %if etc directives
               No iCa language translation (#directives are left untouched)
    -Y         process only %if directives - enables optional compiling for
               yacc, bison or flex; %define directives are left untouched
    -I <path>  scan %include and #include "files" on another path
    -P <macro> predefine <macro> evaluated directly by this program
               Usually defined with a numeric value eg. -P m=16
               NOTE: command line definitions have precedence over
                     '%%define m 8' in the code (opposite to cpp)
    -D <macro> predefine <macro> evaluated only with -m, -M or <in.ic>
    -U <macro> undefine <macro> evaluated only with -m, -M or <in.ic>
    -a         output FOR IF ELSE and %define etc lines as comments
    -T         extra Trace output
    -h         this help text
    <in.ica>   iCa source file with array definitions (default: stdin)
    <in.ic>    iC source file - only #include #define #if etc directives are
               processed unless called with -t option (same as -m option)
               Warning if %include %define %if etc directives are used

=head1 DESCRIPTION

Pre-compiler to convert "immediate C" source files containing arrays
and object like or function like macros into straight "immediate C".

B<immac> translates an iC-source with array definitions, called in
iCa language file (extension: .ica) into an expanded iC language file
(extension: .ic) which can be compiled with B<immcc>.

B<immac> reads and translates one source eg file.ica. If no options
are specified, output is to 'stdout' and compilation errors (if
any) are reported on 'stderr'.  Normally output is stored in a file
specified with the -o <output> option. Error messages are included
in the output file.

The B<immac> compiler can also produce an optional log file with
the -l option, which should have the extension .go (for the GO
verson if immac). This can be used with 'go run' to test 'FOR loop'
generation. This produces better error messages.

The B<immac> compiler can be used to test for iCa language constructs
with the -t option.  If the source contains 'FOR loops', IF or ELSE
control statements or B<%%define> macro definitions, the source file
is an iCa language file.  Isolated [index expressions] without at
least one FOR IF ELSE or B<%%define> are not sufficient to classify
a file as an iCa language file.

=head1 ALTERNATIVE MACRO PROCESSING

B<immac> may be called with the options -m, -M and -Y.  All these
alternatives do not translate iCa language constructs.

B<immac -m> is just a macro processor handling B<#define>, B<#undef>,
-D, -U, B<#include>, B<#if> etc, which makes it an alternative for
B<cpp> mainly used to pre-process C code. Every attempt has been made
to make B<immac -m> equivalent to B<cpp>. B<immac -m> is called by
the iC compiler B<immcc> in the internal C compile phase when iC I/O
variables in C code are replaced.

B<immac -M> is an alternative macro processor handling B<%define>,
B<%undef>, -D, -U, B<%include>, B<%if> etc, which is called by the iC
compiler B<immcc> to resolve possible % macro's in iC code (version
3). B<#>directives are left untouched. If B<immac> is called with an iC
(.ic) input file, the -M option is set internally.

A third variation B<immac -Y>  is used to handle conditional grammar
rules in yacc or bison with B<%if> etc. B<%define> lines are left
untouched because they may be interpretd by bison.

For B<immac -m> B<immac -M> and B<immac -Y> macros can be pre-defined
in the command line with the -D option (just like cpp). For B<immac
-Y> this is the only way to define a macro.

    immac -m -D LENGTH=8     vvv.c
    immac -M -D LENGTH=8     www.ic
    immac -Y -D BOOT_COMPILE xxx.y

The -U option is also available in these 3 cases.

=head1 EXIT STATUS

Normally, the exit status is 0 if the iCa file compiles correctly.
If a compile error occurs the exit status is 2.

For the -t option the exit status is 1 if a file is found to be an
iCa file and 0 otherwise.

=head1 SPECIFICATION

The immediate C with arrays language extensions are as follows:

immediate array variables are defined by appending an expression
enclosed in square brackets to the array name eg. array[N+10]

unless the expression in the square brackets is a constant
expression eg. array[4+10], the line containing the expression
must be contained in a FOR block as follows:

    FOR (N = 0; N < 8; N++) {{
        array[N+10],
    }}

B<immac> uses the FOR control statement to repeatedly output iC or
C statements contained in a block bounded by twin braces {{ ... }}.
It was found, that the iCa language was easier to read, if control
variables used in a FOR loop were upper case (recommended) and twin
braces were used (mandatory).  (Version 2 used single braces).

Another variant is to use the following "perlish" syntax using lists
or the Perl .. operator to generate lists, which produces the same
output as the first example:

    FOR N (0 .. 7) {{
        array[N+10],
    }}

An optional type-specifier 'int' before a FOR loop control variable
is ignored. This usage is deprecated because the contol variable may
also be type 'string'. A Warning is issued.

With Perl type lists it is possible to use strings as well as numbers
as values for the FOR loop control variable. In iCa such strings in
a FOR control list must be enclosed in double quotes. These string
values may of course not be used with arithmetic operators in index and
square bracket expressions, but the concatenation operator '.'  can be
used effectively with lists of string variables:

    FOR N ("aa", "ab", "ac", "ad") {{ ["xyz_".N], }}
 will generate
    xyz_aa, xyz_ab, xyz_ac, xyz_ad,

Lists of integers may also be used, in which case the control variable
is an 'int' and may only be used with arithmetic operators on square
bracket expressions:

    FOR N (0, 3, 4, 10, 5) {{ array[N+1], }}
 will generate
    array1, array4, array5, array11, array6,

Lists of strings and integers may not be mixed. Numeric digits in
parenthesis are a string.

In the above examples only the control variable 'N' as well as numeric
values and string constants may be used in index expressions of the
'FOR' block. 'FOR' blocks may be nested.  In that case all the control
variables in enclosing nested blocks may be used.

All immediate C lines in the block may contain square bracketed
expressions, but they do not need to (they will of course not vary).
The lines in a  'FOR' block are repeated a number of times controlled
by the 'FOR' control line.

It must be remembered, that a FOR loop control variable is either
a numeric integer or a string.  Integer division always applies to
integers. In the generated GO code arithmetic expression variables
are type 'int'.  The following is valid for numeric control variables:

    FOR (N = 0; N < 16; N++) {{
       QX[N/8].[N%8] = IB[N];
    }}

produces the following straight immediate C output

    QX0.0 = IB0;
    QX0.1 = IB1;
    ...
    QX1.6 = IB14;
    QX1.7 = IB15;

which is not brilliant code, but shows the pattern.

Both in C and by analogy in immediate C with arrays (iCa), index
expressions surrounded by square brackets may be separated from their
array variables by white space. Because the idea of the iCa compiler
is to generate individual iC variables from index expressions, this
white space is removed. Array variables and their index expressions
must be on the same line - otherwise spaces and tabs are allowed.

Multi-dimensional array indices have an 'x' inserted between indices
    eg: a [1] [2]  is replaced by a1x2
    eg: b [1] [23] is replaced by b1x23 NOT b123
    eg: b [12] [3] is replaced by b12x3 NOT b123 which is ambiguous

If this were not done, the generated iC variables could be
ambiguous. Also the mental conversion from the generated iC
variable back to its array form is made easier this way.

Array names which finish with a numeral will have a 'y' inserted
before the the first index
    eg: b8[0]      is replaced by b8y0

A line terminated by a backslash '\' both inside or outside a
'FOR loop' generates that line without a terminating LF '\n'.
This allows the generation of lists in a single line.  Normally a
LF is inserted on the termination of the 'FOR loop' unless the
final brace is also followed by a backslash '\' - then the line
is not terminated by a LF '\n'.

Back-slash handling is still supported, but it is much easier to
produce the same result by writing a whole FOR statement in one line.

Index expressions which contain a '"' marking a string do not have
x or y inserted before them. This is used as follows
    eg: a [N] [N < MAX ? "," : ";"]
terminates an expression with either a comma or a semi-colon.
This is useful to terminate a single line comma separated list.

    imm int FOR (N = 0; N <= 4; N++) {{ a [N] [N < 4 ? "," : ";"] }}

produces the following statement:

    imm int a0, a1, a2, a3, a4;

The same output can be obtained by the following:

    imm int FOR (N = 0; N <= 4; N++) {{ a [N], }}; // note final ';'

This would normally produce this slighty different statement:

    imm int a0, a1, a2, a3, a4,;

The B<immac> compiler changes ,; to ; to produce correct iC code -
the same as the previous example.

The iC language and the B<immcc> compiler have been changed to
allow all imm extern and extern C declarations to be terminated by ,;
- with the latest change described in the previous paragraph this is
no longer needed.

Sometimes it is necessary to suppress the output of code lines in a
'FOR loop' or to supply one or more alternative output lines depending
on some condition of the existing loop variables. This can be achieved
with an 'IF' or 'IF ELSE' control statement. The syntax and semantics
is identical to  C 'if' or 'if else' statements, except that again the
'IF' and 'ELSE' keywords are upper-case not lower-case. One or more
'ELSE IF' statements may follow an initial 'IF' statement followed
by a final (optional)'ELSE' statement.  'IF' control expressions in
parentheses may only contain existing 'FOR loop' control variables,
macros and constants. No new control variable may be defined. The
statements controlled by an 'IF', 'ELSE IF'  or 'ELSE' statement
must be contained in twin braces (like the 'FOR loop'). The following
produces the same code as the example above:

    imm bit FOR N (0..5) {{ IF (N < 5){{ a[N], }} ELSE {{ a[N]; }} }}

Sometimes a need may exist to write an expression in square brackets,
either as a comment in iC or as an index in iC or C code without being
interpreted.  Back-slashed brackets will do this:
    a\[10+4\]  wil produce a[10+4]
Alternatively nested square brackets can be used. In that case the
inside square bracket will be interpreted:
    a[[10+4]] wil produce a[14]

=head1 MACRO FACILITY

The pre-compiler B<immac> provides a full macro facility very similar
to that provided by the C pre-processor B<cpp>.  Object like macros
without parameters as well as function like macros with parameters
in parentheses are supported.  When B<immac> is called with an iCa
input file (.ica) and without the options -m, -M or -Y, the keyword
to introduce a macro definition is B<%%define> not B<#define>, which
is reserved for B<cpp> or B<immac -m>.  B<immac> without the options
-m, -M or -Y, also handles B<%%undef>, B<%%include>, B<%%ifdef>,
B<%%ifndef>, B<%%if>, B<%%elif>, B<%%endif>, B<%%error>, B<%%warning>
and B<%%line> in the same way that cpp handles their B<#>directive
counterparts.

    %%define LENGTH    4

The same macro term LENGTH could also be pre-defined in the
command line with the -P option:

    immac -P LENGTH=8 uuu.ica

Unlike B<cpp>, the definition in the command line with -P has
precedence over the definition with a B<%%define> line in the
program. This allows iCa programs to define default values for macro
terms, which can be re-defined in the command line. It is an error to
B<%%define> a macro, which has been previously defined (except on the
command line; the new definition is ignored). The command B<%%undef
X> will undefine the macro X, which can then be re-defined. This is
important if an internal definition is to have precedence over a
(possible) command line definition.  Do a B<%%undef> first. It is
not an error to B<%%undef> a non-existing macro.

Macros must be a word starting with a letter or underscore followed
optionally by letters underscores or decimal digits (same as a C or
iC identifier).  It is highly recommended that letters in a macro are
all upper case (same recommendation as for B<cpp>).  Macro replacements
can be any sort of text, which may also include previously defined
macros. For replacement as index values, they should of course reduce
to numeric values or string constants.

    %%define WIDTH     (5+1)
    %%define AREA      (LENGTH * WIDTH) // C++ comment

If a replacement text is longer than one line, each line except the
last must finish with a backslash '\'.  As shown above B<%%define>
lines may be terminated with a C or C++ comment.  Replacement texts may
also contain embedded C comments, which will be replaced by a single
space on expansion.  Multiple spaces will be replaced by one space
(same as B<cpp>).  As with 'FOR loop' control lines, a C comment must
finish on the B<%%define> line. Replacement texts for function like
macros should contain at least one sample of each parameter text. If
not a warning will be issued.

Parameters may be 'stringified' in the replacement by preceding
them with a single B<#>.  Two parameters or indeed any words may be
concatenated by placing B<##> between them.  Every effort has been made
to obtain the same translations for replacement texts as those obtained
by using B<cpp>.

There are some minor differences.  Replacements which resolve to a
constant arithmetic expression involving only the operators + - * /
and % as well as ( ) decimal integers and spaces are evaluated in the
definition. This brings error messages a little closer to the source of
any erroneous constant expression. The final result is the same though.

For the above macro B<immac> tranlates B<%%define> AREA to 48 whereas
B<cpp> and B<immac -m> translates B<#define> AREA to (8 * (5 + 1)).

The B<%%define> lines are not copied to the target except as comment
lines, if the -a option is active for the B<immac> compiler.

Macro replacements may be made in all parts of the iCa code. They
are of course particularly useful to parametrise the termination
of a 'FOR loop' and hence the number of blocks of iC code, which is
generated by the 'FOR loop'.

File inclusion with B<%%include> "file" and conditional compilation
with B<%%ifdef>, B<%%ifndef>, B<%%if>, B<%%elif>, B<%%else>,
B<%%endif>, B<%%error>, B<%%warning> and B<%%line> are also supported
using the same rules as cpp. The word 'defined' in an B<%%if> or
B<%%elif> expression has the usual cpp meaning - it is set to 1 (true)
if defined else 0 (false). Identifiers in such an expression which are
not defined in a previous B<%%define> or -P are also set to 0 (false).

=head1 AUTHOR

John E. Wulff

=head1 BUGS

Email bug reports to B<immediateC@gmail.com> with L<iC Project> in the
subject field.

=head1 SEE ALSO

L<immcc(1)>, L<iCmake(1)>, L<iCserver(1)>, L<iCbox(1)>, L<cpp(1)>

=head1 COPYRIGHT

COPYRIGHT (C) 2000-2017  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

=for ******* End of man page POD ***********************************/

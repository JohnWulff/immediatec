/***********************************************************************
 *
 *		Copyright (C) 2000-2020 John E. Wulff
 *
 *	You may distribute under the terms of either the GNU General Public
 *	License or the Artistic License, as specified in the README file.
 *
 *	For more information about this program, or for information on how
 *	to contact the author, see the README file
 *
 *		immag.go ==> immag; copied to /usr/local/bin/immac for execution
 *
 *	Pre-compiler to convert "immediate C with arrays" (iCa) source files.
 *	Based on the PERL version of immac, which has beem migrated to GO.
 *
 *	Expands FOR loops and handles IF ELSE statements as well as object
 *	like or function like macros with pre-compile prefix %% into straight
 *	"immediate C" (iC).
 *		"immac" handles %%define %%if etc for a file with .ica extension
 *		"immac" handles %define %if etc for a file with .ic extension
 *		without FOR expansion etc, which is the same as calling "immac -M"
 *
 *	Also several options to handle macros only (no FOR expansion etc):
 *		"immac -M" handles %define %if etc only for straight iC code
 *		"immac -m" handles #define #if etc only to replace cpp for C code
 *		"immac -Y" handles %if etc only (no %define) for conditionals in
 *		yacc code (yacc and/or bison use %define themselves).
 *
 *	This version of immac is for the iC language version 3 and up.
 *
 *	The following prefixes have been changed from iC version 2:
 *		"pre-compiler prefix in"	vers 2		  vers 3
 *		 iC code in .ic and .ica	   #			 %
 *		 iCa code in .ica			   %			 %%
 *		 C code in literal blocks	   %#			 #
 *	These are the only changes in the iC language version 3.
 *
 *	This means that #define #if etc macros in C literal blocks will be
 *	written just like in C and passed unchanged to the generated C code.
 *
 *	Several constructs available in the PERL version of immac prior to Jan 2022
 *	have been dropped:
 *		use strict; no strict; as well as its command line counterparts
 *		-S and -N flags. immac is now always strict requiring twin braces.
 *		-L option. All lines go to the generated GO file, which is log file.
 *		ELSIF	// is very Perlish and can easily be replaced by ELSE IF.
 *		FOR S ("aa" .. "ad") {{ // is very difficult to implement in GO and
 *								// was never used in thousands of my examples.
 *
 *	The only specifically PERL constructs being retained are:
 *		FOR S ("aa", "ab", "12", "-3")	// list of parenthesised strings
 *		FOR S (aa, ab, ac, ad)			// list of bare word strings
 *		FOR I (1, -2, 3, +4)			// list of integers
 *		FOR I (1 .. 4)					// list of integers using .. operator
 *	They are very useful and easy to implement in GO.
 *	List elements in parentheses or bare words containing non-numeric
 *	characters or non-leading sign characters are strings of type char*.
 *	List elements containing only numeric characters or a leading sign are
 *	integers of type int.
 *
 *	The type specifier int (or char*) are no longer allowed before the
 *	FOR loop control variable. Type is always derived:
 *		If all elements of a list are integers, the type of the control variable
 *		and all list elements are int, otherwise they are strings (type char*),
 *
 *	The standard C style FOR loop to generate integer lists is:
 *		FOR (I = 1; I <= 4; I++)		// list of integers (type of I is int)
 *	Integer control variables may only be used in integer arithmetic expressions.
 *	String type char* control variables may only be used with the concatination
 *	operator '.' in expressions (borrowed from Perl).
 *	NOTE: String variables and concatenation are only used in the iCa language,
 *		  never in iC.
 *
 *	Input and output filnames must be restricted to the following
 *	POSIX Portable Filename Character Set consisting of:
 *		A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
 *		a b c d e f g h i j k l m n o p q r s t u v w x y z
 *		0 1 2 3 4 5 6 7 8 9 . _ -
 *	Directory seperators for Linux/Unix or Windows are:
 *		/ \
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
	"regexp"
	"sort"
	"strconv"
	"strings"
)

const ID_immag_go = "$Id: immag.go 1.1 $"

/***********************************************************************
 *
 *	iCa keywords with their translation to GO code (and length and control)
 *
 ***********************************************************************/

type for_if struct {
	tran    string
	length  int
	control int
}

var iCaKey = map[string]for_if{
	"FOR":  {"for", 3, 1},  // iCa FOR loop
	"IF":   {"if", 2, 1},   // iCa IF statement
	"ELSE": {"else", 4, 3}, // iCa ELSE statement
}

/***********************************************************************
 *
 *	Coding of iC, iCa and C keyword map values for possible future use
 *				track with builtins[] in init.c
 *		0x001	imm or immC iC type modifier
 *		0x002	iC type
 *		0x004	iC pragma
 *		0x008	iC keyword
 *		0x010	iC clocked built-in function
 *		0x020	iC unclocked built-in bit function
 *		0x030	iC clocked built-in bit function
 *		0x050	iC clocked built-in arithmetic function
 *		0x070	iC clocked built-in bit or arithmetic function
 *		0x090	iC clocked built-in clock function
 *		0x0b0	iC clocked built-in timer function
 *		0x100	iCa keyword
 *		0x200	C keyword
 *	currently only the fact that the map entry is defined is used
 *
 ***********************************************************************/

var keywords = map[string]int{
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
	"SHR":      0x030, // sample and hold with reset
	"SHR_":     0x050, // sample and hold with simple reset
	"SHSR":     0x030, // sample and hold with set/reset
	"SHSR_":    0x050, // sample and hold with simple set/reset
	"LATCH":    0x020,
	"DLATCH":   0x020,
	"RISE":     0x030, // pulse on digital rising edge
	"CHANGE":   0x070, // pulse on anlog or digital change
	"CHANGE2":  0x070, // alternate CHANGE on anlog change
	"CLOCK":    0x090, // CLOCK with 1 or 2 inputs
	"CLOCK2":   0x090, // alternate CLOCK with 2 inputs
	"TIMER":    0x0b0, // TIMER with preset off 0
	"TIMER2":   0x0b0, // alternate TIMER with 2 inputs
	"TIMER1":   0x0b0, // TIMER1 with preset off 1
	"TIMER12":  0x0b0, // alternate TIMER1 with 2 inputs
	"if":       0x208,
	"else":     0x208,
	"switch":   0x208,
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
	"this":     0x208, // only used in function block definitions
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

type selector uint

const (
	ifx selector = 1 << iota
	elif
	elsx
	endif
)

var named string
var opt_I, opt_P, opt_D, opt_U multStringFlag
var opt_o, opt_l, opt_e *string
var opt_t, opt_m, opt_M, opt_Y, opt_a, opt_T *bool
var defs   = map[string]*strNum{}
var clDefs = map[string]*strNum{}
var siStk = make([]int, 0)
var argIn string
var genFile *os.File
var lnErr = make([]string, 0)
var lineNo  int
var blockStart int
var IF bool
var IFs string
var w int
var ret int
var blankLines int
var devIn, inoIn, devOut, inoOut, lastAtom1, lastTranslate, off string
var re string
var used = map[string]string{}
var path string
var block = make([]string, 0)
var reParenSlash *regexp.Regexp
var reWord       *regexp.Regexp
var reDefined    *regexp.Regexp
var reNum        *regexp.Regexp
var reWordNum    *regexp.Regexp
var reEvalIf     *regexp.Regexp
var reTrSpaces   *regexp.Regexp
var reSpaces     *regexp.Regexp
var reCcomment   *regexp.Regexp
var reCcomTail   *regexp.Regexp
var reMacro      *regexp.Regexp
var reComma      *regexp.Regexp
var reHashHash   *regexp.Regexp
var reHash       *regexp.Regexp
var reTranslate  *regexp.Regexp

/********************************************************************
 *
 *		Main program
 *
 *******************************************************************/

func main() {
	rePath := regexp.MustCompile(fmt.Sprintf(".*%c", os.PathSeparator))
	named = rePath.ReplaceAllLiteralString(os.Args[0], "")
	argIn = "stdin" // default for STDIN
	var genName string
	var argL string
	log.SetFlags(log.Lshortfile)
	opt_o = flag.String("o", "", "name of generated output file (default stdout)")
	opt_l = flag.String("l", "", "name of generated intermediate GO log file (normally deleted)")
	opt_e = flag.String("e", "", "name of generated error file (default stderr)")
	opt_t = flag.Bool("t", false, fmt.Sprintf(`test if source is an iCa language file - return 1 if it is

<in.ica>   iCa source file with array definitions (default: stdin)
		   Only processes %%%%include %%%%define %%%%if etc directives when
		   called without -m, -M or -Y options
<in.ic> iC source file - only %%include %%define %%if etc directives are
		   processed unless called with -t option (same as -M option)
		   No iCa language translation (#directives are left untouched)

Pre-compiler to convert "immediate C" source files containing arrays
and object like or function like macros into straight "immediate C".

Copyright (C) 2001	John E. Wulff	 <immediateC@gmail.com>
%q`, ID_immag_go))
	opt_m = flag.Bool("m", false, fmt.Sprintf(`process only #include #define #if etc directives for C files
No iCa language translation (%s -m replaces cpp)
Warning if %%include %%define %%if etc directives are used`, named))
	opt_M = flag.Bool("M", false, `process only %include %define %if etc directives for iC files.
No iCa language translation (#directives are left untouched)`)
	opt_Y = flag.Bool("Y", false, `process only %if directives - enables conditional compiling
for yacc, bison or flex; %define directives are left untouched`)
	opt_a = flag.Bool("a", false, "output FOR IF ELSE and %%define etc lines as comments")
	flag.Var(&opt_I, "I", `scan %include and #include "files" on another path
May be used multiple times`)
	flag.Var(&opt_P, "P", `predefine a <macro> for iCa translation
<macro> is usually defined with a numeric value eg. -P m=8
NOTE: command line definitions have precedence over
	 '%%define m 4' in the code (opposite to cpp)
May be used multiple times`)
	flag.Var(&opt_D, "D", `predefine a <macro> evaluated for C, iC or yacc/bison translation
with -m, -M, -Y or <in.ic>. May be used multiple times`)
	flag.Var(&opt_U, "U", `undefine <macro> evaluated only with -m, -M, -Y or <in.ic>
May be used multiple times`)
	opt_T = flag.Bool("T", false, "extra Trace output")
	flag.Parse()
	path = os.Getenv("INCLUDE")
	if path == "" {
		path = "/usr/local/include"
	}
	for _, p := range opt_I {
		path += ":" + p // include path options from -I option
	}
	path += ":." // finally add current directory
	// fmt.Printf("path = %q\n", path)

	var inFile  *os.File
	var inFileC *os.File
	var scanner *bufio.Scanner
	var inFileStk = make([]*fileStk, 0)
	if len(flag.Args()) == 0 {
		inFile  = os.Stdin
	} else if len(flag.Args()) == 1 {
		var err error
		argIn = flag.Arg(0)
		if inFile , err = os.Open(argIn); err != nil {
			log.Fatal(err)
		}
		defer inFile .Close()
	} else {
		log.Fatalln("Too many input file arguments (only one allowed)", flag.Args())
	}
	inFileC = inFile		// current input file
	reFileName := regexp.MustCompile(fmt.Sprintf("^((([.A-Z_a-z][\\w-]*|\\.*)%c)*)([.A-Z_a-z][\\w-]*)(\\.(\\w*))?$", os.PathSeparator))
	// fmt.Printf("argIn = %q flag.Args() %q\n", argIn, flag.Args())
	m := reFileName.FindStringSubmatch(argIn)
	if len(m) != 7 {
		log.Fatalf("badly formed file name %q\n", argIn) // unlikely after succesful open
	}
	// fmt.Printf("len(m) == %d; argIn = %q; m = %q\n", len(m), argIn, m)
	pathIn := m[1]
	baseIn := m[4]
	extnIn := m[6]
	if *opt_T {
		fmt.Printf(" pathIn = %q baseIn = %q extnIn = %q\n", pathIn, baseIn, extnIn)
	}
	statIn, err := os.Stat(argIn)
	if argIn != "stdin" && err != nil {
		if err != nil {
			log.Fatal(err) // does not apply to os.Stat("stdin")
		}
		if extnIn != "ica" && !*opt_m && !*opt_Y {
			*opt_M = true
			argL = argIn // TODO - looks fishy
		} else if *opt_o != "" {
			argL = *opt_o // name for 1st line of generated file
		}
	}

	if *opt_t {
		*opt_o = ""
		*opt_e = ""
		*opt_l = ""
		*opt_m = false
		*opt_M = false
		*opt_Y = false
	} else {
		if *opt_m {
			*opt_M = false	// opt_m has precedence over opt_M and opt_Y
			*opt_Y = false
		} else if *opt_M {
			*opt_Y = false	// opt_M has precedence over opt_Y
		}
		if *opt_l != "" {
			genName = *opt_l // keep generated GO file as a log file
		} else {
			genName = fmt.Sprintf("%s.go", baseIn)
		}
// fmt.Printf("genName = %q\n", genName)
		statGen, err := os.Stat(genName)
		if err == nil {
			// fmt.Printf("genName = %q exists\n", genName)
			if os.SameFile(statIn, statGen) {
				log.Fatalf("output file %q should not be the same as generated GO file %q\n", genName, argIn)
			}
			if err := os.Chmod(genName, 0644); err != nil {
				log.Fatal(err)
			}
		}
		genFile, err = os.Create(genName)
		if err != nil {
			log.Fatal(err)
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
`)						// balance } }
		blockStart = 1
		if *opt_o != "" { // keep iC output file if Log file is in error
			statOut, err := os.Stat(*opt_o)
			if err == nil {
				if os.SameFile(statIn, statOut) {
					log.Fatalf("output file %q should not be the same as input file %q\n", *opt_o, argIn)
				}
				if err := os.Chmod(*opt_o, 0644); err != nil {
					log.Fatal(err)
				}
			}
			outFile, err := os.Create(*opt_o)
			if err != nil {
				log.Fatal(err)
			}
			defer outFile.Close()
			os.Stdout = outFile
		}
		if *opt_e != "" { // must be last so STDERR remains open
			statErr, err := os.Stat(*opt_e)
			if err == nil {
				if os.SameFile(statIn, statErr) {
					log.Fatalf("error file %q should not be the same as input file %q\n", *opt_e, argIn)
				}
				if err := os.Chmod(*opt_e, 0644); err != nil {
					log.Fatal(err)
				}
			}
			errFile, err := os.Create(*opt_e)
			if err != nil {
				log.Fatal(err)
			}
			defer errFile.Close()
			os.Stderr = errFile
		}
	}
	opt_m_M_Y := *opt_m || *opt_M || *opt_Y
	if *opt_T && *opt_l == "" {
		fmt.Println(" *opt_m =", *opt_m, "*opt_M =", *opt_M, "*opt_Y =", *opt_Y, "opt_m_M_Y =", opt_m_M_Y)
	}

	// balance { { { {
	reState := regexp.MustCompile(`(%?#|%%?|/[/*]|\*/|[\w.]+|%?\{\{?|%?}}?|(\\\\)+|\\n|\\t|\\?"|\\?'|\\?\[|\\?\]|\S|.?$)`)
//	reBackSlash := regexp.MustCompile(`^(.*)\\$`)
	reCommaSemi := regexp.MustCompile(`,\s*;`)
	reParenSlash = regexp.MustCompile(`(["\\])`)
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
	reLdQuote := regexp.MustCompile(`^\s*'?`)
	reTrQuote := regexp.MustCompile(`'?\s*$`)
	reWord     = regexp.MustCompile(`^[A-Z_a-z]\w*$`)
	reDefined  = regexp.MustCompile(`defined\s*`)
	reNum      = regexp.MustCompile(`^\d+$`)
	reWordNum  = regexp.MustCompile(`^\w+$`)
	reEvalIf   = regexp.MustCompile(`(\w+|&&|\|\||[=!<>]=|<<?|>>?|~)`)
//	reComment := regexp.MustCompile(`(\s*(/[/*].*)?$)`)
//	reLdTrSps := regexp.MustCompile(`^\s+|\s+$`)
	reTrSpaces = regexp.MustCompile(`\s+$`)
	reSpaces   = regexp.MustCompile(`(\s+)`)
	reCcomment = regexp.MustCompile(`\s*/\*.*`)
	reCcomTail = regexp.MustCompile(`\*/\s*`)
	reMacro    = regexp.MustCompile(`^([A-Z_a-z]\w*)(\(\s*(([A-Z_a-z]\w*)(\s*,\s*([A-Z_a-z]\w*))*)*\s*\))?(\s+(.+))?$`)
	reComma    = regexp.MustCompile(`\s*,\s*`)
	reHashHash = regexp.MustCompile(`\s*##\s*`)
	reHash     = regexp.MustCompile(`#([^\d\\]|$)`)
	reTranslate = regexp.MustCompile(`([A-Z_a-z]\w*|\/[*\/]|\*\/|\\?["'])`)
	reInclude  := regexp.MustCompile(`^["<](([A-Za-z]:)?([/\\]?)[A-Z_a-z.][\w/\\.-]*)[">]$`)
	reErrWarn  := regexp.MustCompile(`^(error|warning)$`)
// detailed analysis code lines					m/((\\\\)*)(\\?("|'|\[|\]|\n)|%?(\{\{?|}}?)|\/\/|\/\*|\*\/|\w+|\\n|\\t|\S)/g	// {{ balance
	Cdirective := false
	state := 'A'		// starting state
	si    := 1			// initial value of sense indicator; output	%else or %elif not allowed
	prefixes := map[string]bool {
		"%%":	true,
		"%":	true,
		"#":	true,
		"%#":	true,
	}
	directives := map[string]bool {
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
	pragmas := map[string]int {
		"no":		5,
		"use":		5,
		"strict":	6,
	}
	direcve    := ""
	macro      := ""
	macroStart := 0
	mWarn      := false
	clFlag     := false
	comStart   := 0
	FORline    := "X"
	if len(opt_P) > 0 {
		// fmt.Fprintf(os.Stderr, "-P %q\n", opt_P)
		for _, p := range opt_P {
			// fmt.Fprintf(os.Stderr, " %q", p)
			p = reLdQuote.ReplaceAllLiteralString(p, "")	// delete leading blanks and "'" inserted by iCmake
			p = reTrQuote.ReplaceAllLiteralString(p, "")	// delete trailing "'" inserted by iCmake and blanks
			s := regexp.MustCompile(`\s*=\s*`).Split(p, 2)	// space separated macro is also accepted
			macro := s[0]
			if len(s) > 1 {
				macro += " " + s[1]							 // when translation is missing just %%define X
			}
			// fmt.Fprintf(os.Stderr, " %q\n", macro)
			df, val := scan_define("-P", macro)				// scan and save -P macro in  map defs
			// fmt.Fprintf(os.Stderr, " df = %q val = %q ret = %d\n", df, val, ret)
			if len(lnErr) > 0 {
				output_error("-P "+p+"\n")
			} else {
				if df == val {
					lnErr = append(lnErr, "//* Warning: '-P macro' does not change anything ???\n")
					output_error("-P "+p+"\n")
					w++ // warning found
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
				goto CloseFiles // found Error	# proceed with Warning
			}
		}
	}
	if opt_m_M_Y { // -D -U only processed for <in>.ic iC files or if -m -M or -Y
		if len(opt_D) > 0 {
			// fmt.Fprintf(os.Stderr, "-D %q\n", opt_D)
			for _, p := range opt_D {
				// fmt.Fprintf(os.Stderr, " %q", p)
				p = reLdQuote.ReplaceAllLiteralString(p, "")	// delete leading blanks and "'" inserted by iCmake
				p = reTrQuote.ReplaceAllLiteralString(p, "")	// delete trailing "'" inserted by iCmake and blanks
				s := regexp.MustCompile(`\s*=\s*`).Split(p, 2)	// space separated macro is also accepted
				macro := s[0]
				if len(s) > 1 {
					macro += " " + s[1] // when translation is missing just %%define X
				}
				// fmt.Fprintf(os.Stderr, " %q\n", macro)
				df, val := scan_define("-D", macro) // scan and save -D macro in  map defs
				// fmt.Fprintf(os.Stderr, " df = %q val = %q ret = %d\n", df, val, ret)
				if len(lnErr) > 0 {
					output_error("-D "+p+"\n")
				} else {
					if df == val {
						lnErr = append(lnErr, "//* Warning: '-D macro' does not change anything ???\n")
						output_error("-D "+p+"\n")
						w++ // warning found
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
					goto CloseFiles // found Error	# proceed with Warning
				}
			}
		}
		if len(opt_U) > 0 { // for compatibility with cpp for generated command lines
			// fmt.Fprintf(os.Stderr, "-U %q\n", opt_U)
			for _, df := range opt_U {
				// fmt.Fprintf(os.Stderr, " %q\n", df)
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
					lnErr = append(lnErr, "//* Warning: '-U macro' has bad characters - ignored\n")
					output_error("-U "+df+"\n")
					w++ // warning found
				}
			}
		}
	}
	if clFlag {
		fmt.Printf("# 1 %q\n", argL)
	}

	/***********************************************************************
	 *
	 *	Process the file
	 *
	 ***********************************************************************/

	if *opt_l != "" {
		fmt.Fprintln(genFile, "//###", named, argIn)
	}
	/***********************************************************************
	 *	Read all lines in the input file
	 ***********************************************************************/
	scanner = bufio.NewScanner(inFileC)
	scanner.Split(bufio.ScanLines)
Level:
	for {
		for scanner.Scan() {
			text := scanner.Text() // `\r?\n` is stripped from returned text
			text = reTrSpaces.ReplaceAllLiteralString(text, "")	// remove trailing white space
			lineNo++ // first line is lineNo 1 - balance {{
 fmt.Printf("%d:\t%q\n", lineNo, text)
// matches := reState.FindAllStringIndex(text, -1)
// fmt.Println(matches)
			l    := len(text)
			last := byte(0)
			if l > 0 {
				last  = text[l-1]				// could be trailing backslash
			}
			pos      := 0
			pose     := 0
			posePrev := 0
			offset   := 0
			sqNest   := 0
			sqNestSv := 0
			textFOR  := text
			semi     := true					// start of line is syntactically like after a semi-colon
			useNo    := false
			prefix   := ""
			/***********************************************************************
			 *  Process lines with a state machine, which recognises comments,
			 *  parenthesised strings, quoted characters as well a relevant atoms
			 *  'A' inside iCa, iC or C code
			 *  'C' inside a C comment
			 *  'P' inside a C++ comment
			 *  'S' inside a parenthesised string
			 *  'H' inside a single quoted character constant
			 ***********************************************************************/
			//	reState: (%?#|%%?|/[/*]|\*/|[\w.]+|%?\{\{?|%?}}?|(\\\\)+|\\n|\\t|\\?"|\\?'|\\?\[|\\?\]|\S|.?$)
			for i, posPair := range reState.FindAllStringIndex(text, -1) {
				//	(
				//		%?#		|	# directive prefix # %#
				//		%%?		|	# directive prefix % %%
				//		/[/*]	|	# /* // start of C or C++ comment
				//		\*/		|	# */    end of C comment
				//		[\w.]+	|	# word identifier or number - balance {{
				//		%?\{\{?	|	# { {{ %{ - not used %{{
				//		%?}}?	|	# } }} %} - not used %}}
				//		(\\\\)+	|	# remove pairs of backslashes
				//		\\n		|	# char \n NL in string
				//		\\t		|	# char \t TAB in string
				//		\\?"	|	# " or \"
				//		\\?'	|	# ' or \'
				//		\\?\[	|	# [ or \[	- used in iCa analysis only
				//		\\?\]	|	# ] or \]	- used in iCa analysis only
				//		\S			# any single non space character
				//	)
				posePrev = pose						// start of white space after previous atom
				pos   = posPair[0]
				pose  = posPair[1]
				atom := text[ pos : pose ]
 fmt.Printf("%2d\t%3d %3d\t%q\n", i, pos, pose, atom)
				if state == 'A' {				// program - not comment, string, char constant
					if i == 0 {
						/***********************************************************************
						 *	If -m then both %%directives and %#directives are excluded - #define and %define (warning)
						 *		reCdirective = regexp.MustCompile(`^\s*%[%#]\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
						 *	Else if -M %%directives, #directives and %#directives are excluded - %define %if only
						 *		reCdirective = regexp.MustCompile(`^\s*(%%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
						 *	Else if -Y additionally %define %undef %include are excluded - %define only (no %if)
						 *		reCdirective = regexp.MustCompile(`^\s*((%%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)|%(define|undef|include))\b`)
						 *	Else if none of the above %directives, #directives and %# directives are excluded - only %%directives
						 *		reCdirective = regexp.MustCompile(`^\s*(%|%?#)\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error|warning|line)\b`)
						 *
						 *	These regular expressions exclude directives which are not resolved for the chosen language
						 *	Output these directives completely unchanged with possible trailing \
						 *	except if blocked by #if #else or #elif
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
									goto Line		// do not output lines in false blocks
								}
								if opt_m_M_Y && blankLines > 0 {
									blanks()		// print blank lines now
								}
								genLine(text)
							}
							goto Line				// must not modify because of comments
						}
						/***********************************************************************
						 *	All directives not belonging to the current option -m, -M, -Y or none
						 *  have been excluded in the previous section.
						 *	From now on treat %%directives, %directives and #directives equivalently
						 *	except warn if %directive used in straight iC file or in immac -m call
						 *  The prefix and the directive should only be the first two atoms in a line
						 *  After the directive continue scanning for comments.
						 *  The macro with possible parenthesised argument list starts with the next
						 *  atom, which  has all comments and space before comments deleted.
						 ***********************************************************************/
						if prefixes[atom] {			// %% % # or %#
							prefix = atom
							if *opt_m && prefix == "%" {
								mWarn = true		// output Warning later for -m only
							}
						}
					} else if i == 1 && prefix != "" && directives[atom] {
						direcve = atom				// directive found
						macroStart = -1				// must get next atom to obtain a macro 
					} else if i == 2 && macroStart == -1 {
						macroStart = pos			// start of macro
					}
					if atom == "/*" {
						state = 'C'					// start of C comment found
						comStart = posePrev			// include leading spaces
						sqNestSv = sqNest			// save for state 'A'
						sqNest = 0					// sync square brackets
// fmt.Printf("comStart = %d offset = %d\n", comStart, offset)
					} else if atom == "//" {
						state = 'P'					// start of C++ comment found
						comStart = posePrev			// include leading spaces
						sqNestSv = sqNest			// save for state 'A'
						sqNest = 0					// sync square brackets
// fmt.Printf("comStart = %d offset = %d\n", comStart, offset)
					} else if useNo && pragmas[atom] == 6 {	// currently "strict" (could be more)
						/***********************************************************************
						 *	Pragma found
						 *	use or no strict is deprecated - both are now a NOP
						 ***********************************************************************/
						lnErr = append(lnErr, "//* Warning: use or no strict is deprecated - ignore\n")
						w++							// warning found
						useNo = false
					} else if semi && pragmas[atom] == 5 {	// "use" or "no" at start of line or after semi-colon
						useNo = true
					} else if atom == ";" {
						semi = true
					} else {
						semi = false				// any atom except start of comments
						if atom == "[" {			// leading backslash will not compare
							sqNest++				// opening square bracket
						} else if atom == "]" {		// leading backslash will not compare
							sqNest--				// closing square bracket
						} else if atom == "\"" {	// leading backslash will not compare
							state = 'S'				// start of string found
							sqNestSv = sqNest		// save for state 'A'
							sqNest = 0				// sync square brackets
						} else if atom == "'" {		// leading backslash will not compare
							state = 'H'				// start of character constant found
							sqNestSv = sqNest		// save for state 'A'
							sqNest = 0				// sync square brackets
/***********************************************************************
						} else if iCaKey[atom] {	// FOR IF ELSE or ELSE IF
							if aix {
								if atom == "IF" && prevAtom == "ELSE" {
									len = pos - prevPos - 3		// both have offset added, which cancels
									substr(_, prevPos + offset + 3, len) = ""
									offset -= len	// change "ELSE IF" to "ELSIF" for Perl code TODO Fix
								}
							}
							FORline = "_"
						} else if atom == "{{" {	// ignore old style single brace
							FORline = ""
							FORend = sqNest = 0		// sync square brackets
						} else if atom == "}}" {	// ignore old style single brace
							FORend = 0				// special false marker that twin brace in this line
						} else if atom == ";" {
							sqNest = 0				// sync square brackets
 ***********************************************************************/
						}
					}
				} else if state == 'S' {			// string constant
					if atom == "\"" {				// leading backslash will not compare
						sqNest = sqNestSv			// restore for state 'A'
						state = 'A'					// end of string constant
					}
				} else if state == 'H' {			// character constant
					if atom == "'" {				// leading backslash will not compare
						sqNest = sqNestSv			// restore for state 'A'
						state = 'A'					// end of character constant
					}
				} else if state == 'C' {			// C comment
					if atom == "*/" {
						sqNest = sqNestSv			// restore for state 'A'
						state = 'A'					// end of C comment
						if direcve != "" || FORline != "" {
							al := pose - comStart;	// length of C comment
							textFOR = substr(textFOR, comStart + offset, al, "")
							offset -= al
// fmt.Printf("comStart = %d offset = %d al = %d textFOR = %q\n", comStart, offset, al, textFOR)
						}
					}
				}
			}
			if state == 'P' {						// C++ or Perl comment end
				sqNest = sqNestSv					// restore for state 'A'
				state = 'A'							// end of C++ or Perl comment
				if direcve != "" || FORline != "" {
					al := pose - comStart;			// length of C comment
					textFOR = substr(textFOR, comStart + offset, al, "")
					offset -= al
// fmt.Printf("comStart = %d offset = %d al = %d textFOR = %q\n", comStart, offset, al, textFOR)
				}
			}
			/***********************************************************************
			 *	End of a line processed by the state machine
			 *	state may be 'A' normal code
			 *			  or 'C' C comment
			 *			  or 'S' string constant
			 *			  or 'H' character constant
			 *	state may never be 'P' C++ comment here
			 ***********************************************************************/
 fmt.Printf("%c>:\t%q\n", state, textFOR)
			if direcve != "" {
				/***********************************************************************
				 *	Directive found
				 *	analyse possible macro and store in map defs[]
				 ***********************************************************************/
				if macroStart >= 0 {
					if macroStart > 0 {
						if macroStart <= len(textFOR) {
							macro = textFOR[ macroStart : ]
						} else {
							macro = ""
						}
						macroStart = 0
					} else {
						macro += textFOR			// concatenate new part to previous part(s)
					}
	 fmt.Printf("macro = %s %q\n", direcve, macro)
				}
				if macro != "" && last == '\\' {	// must check last in case previous C++ comment
					lm    := len(macro)
					if lm > 0 && macro[lm-1] == '\\' {
						om := l - posePrev
						macro = macro[ : lm-om ]	// remove trailing '\' with preceding spaces
	 fmt.Printf("MACRO = %s %q lm = %d om = %d\n", direcve, macro, lm, om)
					}								// append next line to macro
				} else {
					/***********************************************************************
					 *	Scan %define or %undef macro definition as well as other directives
					 ***********************************************************************/
					if mWarn {
						lnErr = append(lnErr, "//* Warning: %directive in iC file or immac -m - expanded\n")
						w++							// warning found
						mWarn = false
					}
// fmt.Printf("direcve macro: %q %q\n", direcve, macro)
					/***********************************************************************
					 *	Conditional inclusion statements %%if, %if or #if with #else #elif snd #endif
					 *	are executed in the generated GO code.  If there are nesting errors in those
					 *	statements the error will be reported by the GO code.
					 *	Use IF as the control variable, because it is a keyword in the iCa language
					 *	and cannot therefore be used as a macro
					 ***********************************************************************/
					const DEF = true
					if direcve == "if" {				// if EXPRESSION
						genIf(parseIf(macro, !DEF), ifx, text)
					} else if direcve == "ifdef" {		// ifdef NAME
						genIf(parseIf("defined "+macro, DEF), ifx, text)
					} else if direcve == "ifndef" {		// ifndef NAME
						genIf(parseIf("!defined "+macro, DEF), ifx, text)
					} else if direcve == "elif" {		// elif EXPRESSION
						genIf(parseIf(macro, !DEF), elif, text)
					} else if direcve == "else" {		// else
						genIf("", elsx, text)
					} else if direcve == "endif" {		// endif
						genIf("", endif, text)
					} else if si > 0 {
						if direcve == "define" {
	/***********************************************************************
							resolve_line()					// resolve embedded macros
							(direcve, macro) = split " ", _, 2	// split again after resolve_line()
							macro =~ s!\s*(//.*)?$!!				// delete C++ comment and trailing blanks
	 ***********************************************************************/
	// fmt.Printf("#define si = %d macro = '%s'\n", si, macro)
							if *opt_l != "" { fmt.Fprintf(genFile, "//# %s\n", text) }
							if *opt_a {
								blankLines--
								if opt_m_M_Y && blankLines > 0 {	// blankLines is at least 1 because it is a directive
									blanks()		// print blank lines now
								}
								genLine("//***** "+text)
							}
							scan_define(direcve, macro)		// scan and save macro in %defs
						} else if direcve == "undef" {
							if reWord.MatchString(macro) {
								if *opt_l != "" { fmt.Fprintf(genFile, "//# %s\n", text) }
								if *opt_a {
									blankLines--
									if opt_m_M_Y && blankLines > 0 {	// blankLines is at least 1 because it is a directive
										blanks()		// print blank lines now
									}
									genLine("//***** "+text)
								}
								delete(defs, macro)		// safe to delete even if hash entry does not exist
								delete(clDefs, macro)
							} else {
								lnErr = append(lnErr, fmt.Sprintf("//* Warning: %s%s '%s' has bad characters - ignored\n", prefix, direcve, macro))
								w++					// warning found
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
									lnErr = append(lnErr, fmt.Sprintf("//* Warning: '%s' not in include path '%s' - ignored\n", includeFile, path))
									w++						// warning found
									includeFile = ""
									break Include
								}
							} else {
								lnErr = append(lnErr, fmt.Sprintf("//* Warning: %s%s '%s' has bad characters - ignored\n", prefix, direcve, macro))
								w++					// warning found
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
									if *opt_a {
										blankLines--
										if opt_m_M_Y && blankLines > 0 {	// blankLines is at least 1 because it is a directive
											blanks()		// print blank lines now
										}
										genLine("//***** "+text)
	 genLine(fmt.Sprintf("//### start of include %q len(siStk) = %d si = %d", argIn, len(siStk), si))
									}
									if opt_m_M_Y {
										blanks()					// print blank lines now
										genLine(fmt.Sprintf("# 1 %q 1", argIn))	// # 1 "file" 1
									}
									direcve = ""
									macro   = ""
									continue Level
								} else {
									lnErr = append(lnErr, "//* Error: Could not open file\n")
									ret++							// open error
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
						} else if err_wa := reErrWarn.FindString(direcve); err_wa != "" {	// `^(error|warning)$`
	/***********************************************************************
							if !opt_m_M_Y && prefix == "%%" {
								lnErr = append(lnErr, "//* Error: direcve macro\n")
								ret++ if er_wa == "error"			// deliberate error
								if !*opt_t {
									if len(block) > 0 || finBlock || opt_L {
										macro =~ s/"/\\"/g
										block = append(block, "\FOR .= \"%{\\n#er_wa macro\\n%}\\n\";\n")
									} else {
										print "%{\n#er_wa macro\n%}\n"
									}
								}
							} else {						// -m or -M or #error or #warning
								print STDERR "argIn:$.:2: er_wa: direcve macro\n"
								exit 1 if er_wa == "error"	// deliberate error - emulate cpp
							}
	 ***********************************************************************/
						} else if direcve == "line" {
	/***********************************************************************
							if macro =~ m/^\s*(\d+)\s+(".+")(.*)$/ {
								print "prefix $1 $2"
							} else {
								print STDERR "argIn:$.: 'direcve macro' malformed\n"
							}
	 ***********************************************************************/
						} else {
							lnErr = append(lnErr, "//* Warning: direcve 'macro' unknown iCa directive - ignored\n")
							w++						// warning found
						}
					}
					if *opt_T { fmt.Printf("*9* si = %d\n", si) }
					direcve = ""
					macro   = ""
				}
				goto Line							// do not output %define etc directives (already counted as blank)
			}
/***********************************************************************
			^***********************************************************************
			 *	Collect lines for the first part of a FOR control statement up to the
			 *	opening braces and present the complete line for further processing.
			 *	This needs to be done here to remove comments between FOR and opening
			 *	braces and to present a clean FOR control line for output as a comment.
			 *	If there is a second FOR, after the opening braces on the same line,
			 *	stay in this loop and collect lines up to the next opening braces also.
			 *	Comments in the FOR control line and just after the first braces are
			 *	deleted. A FOR in a comment or string is ignored (this is very important).
			 *
			 *	The same applies for IF ELSE control statements.
			 *
			 *	NOTE: no brace counting is necessary - only look for next opening
			 *	brace(s) after a FOR IF ELSE or ELSE IF.					{ balance
			 ***********************************************************************^
			else if (not opt_m_M_Y &&
				(m/(\bFOR\b|\bIF\b|\bELSE\b|}|\[|\])/ || FORline || FORend || SQline)) {
				if FORline {					// speeds up this scan
					s/^\s+//					// delete leading blanks in follow on lines
					_ = "FORline_"				// concatenate new part to previous part(s)
					FORline = ""				// wait till FOR is scanned to set it again
					state = st0Save
				}
				if FORend {						// ignore value '0'
					s/^\s+//					// delete leading blanks in follow on lines
					_ = "FORend_"				// concatenate new part to previous part(s)
					FORend = ""			// wait till end of C comment
					state = st0Save
				}
				if SQline {
					s/^\s+//					// delete leading blanks in follow on lines
					_ = "SQline_"				// concatenate new part to previous part(s)
					SQline = ""			// wait till ']' is scanned
					state = st0Save
				}
				st0Save = state
				if *opt_T && !*opt_l != "" { fmt.Print("$.*FOR*IF*		_") }
				@atoms = ()								// repeat scan if line has been extended   {{ balance
				sqNest = sqNestSv = offset = 0	// sqNest is used independently here and in full analysis
				while (m/((\\\\)*)(\\?("|'|\[|\]|\n)|\/\*|\/\/|\*\/|#|%?\{\{?|%?}}?|\w+|;)/g) {
					atoms = append(atoms, [ length($`)+length($1), $3 ])		// [ pos, atom ]
				}
				for (my aix = 0; aix < @atoms; aix++) {
					( prevPos, prevAtom ) = ( pos, atom )
					my ref = atoms[aix]
					( pos, atom ) = @ref
					if *opt_T && !*opt_l != "" { fmt.Println("$.* atom*		pos = pos, offset = offset, state = state") }
					if state == 'A' {			// iC - Program
						if atom == "[" {
							sqNest++					// opening square bracket
						} else if atom == "]" {
							--sqNest					// closing square bracket
						} else if atom == "\"" { // leading backslash will not compare
							state = 'S'				// start of string found
							sqNestSv = sqNest	// save for state 'A'
							sqNest = 0			// sync square brackets
						} else if atom == "'" { // leading backslash will not compare
							state = 'H'				// start of character constant found
							sqNestSv = sqNest	// save for state 'A'
							sqNest = 0			// sync square brackets
						} else if atom == "/*" {
							state = 'C'				// start of C comment found
							comStart = pos + offset		// will remain if comment finishes on another line
							sqNestSv = sqNest	// save for state 'A'
							sqNest = 0			// sync square brackets
						} else if atom == "//" || atom == "#" {
							state = "CP"				// start of C++ or Perl comment found
							comStart = pos + offset		// C-pre-processor directives are taken out earlier
							sqNestSv = sqNest	// save for state 'A'
							sqNest = 0			// sync square brackets
						} else if iCaKey[atom] {		// FOR IF ELSE or ELSE IF
							if aix {
								if atom == "IF" && prevAtom == "ELSE" {
									len = pos - prevPos - 3		// both have offset added, which cancels
									substr(_, prevPos + offset + 3, len) = ""
									offset -= len		// change "ELSE IF" to "ELSIF" for Perl code TODO Fix
								}
							}
							FORline = "_"
						} else if atom == "{{" {		// ignore old style single brace
							FORline = ""
							FORend = sqNest = 0 // sync square brackets
						} else if atom == "}}" {		// ignore old style single brace
							FORend = 0			// special false marker that twin brace in this line
						} else if atom == ";" {
							sqNest = 0			// sync square brackets
						}
					} else if state == 'S' {			// string constant
						if atom == "\"" {				// leading backslash will not compare
							sqNest = sqNestSv	// restore for state 'A'
							state = 'A'				// end of string constant
						}
					} else if state == 'H' {			// character constant
						if atom == "'" {				// leading backslash will not compare
							sqNest = sqNestSv	// restore for state 'A'
							state = 'A'				// end of character constant
						}
					} else if state == 'C' {			// C comment
						if atom == "*%%%/" {
							sqNest = sqNestSv	// restore for state 'A'
							state = 'A'				// end of C comment
							if FORline {
								len = pos + offset + 2 - comStart; # length of C comment
								substr(_, comStart, len) = ""
								offset -= len
							}
						}
					} else if state == 'P'" {			// C++ or Perl comment
						if atom =~ m/^(\\?\n)$/ {
							sqNest = sqNestSv	// restore for state 'A'
							state = 'A'				// end of C++ or Perl comment
							if FORline {
								len = pos + offset - comStart	// length of C++ or Perl comment
								substr(_, comStart, len) = ""
								offset -= len
							}
						}
					}
				}
				if FORline {
					chomp								// FOR line not terminated by opening brace(s)
					s/\s*$/ /							// 1 space at the end of the line for continuation
					FORline = _
					goto Line								// input continuation line(s)
				}
				if FORend == '0' {
					if state == 'C' {
						chomp							// end of FOR block ends in incomlete C comment
						s/\s*$/ /						// 1 space at the end of the line for continuation
						FORend = _						// twin opening or closing braces have been seen in this line
						goto Line
					}
					FORend = ""
				}
				if sqNest {
					chomp								// square bracket nesting not comlete
					s/\s+$//							// 0 space at the end of the line for continuation
					SQline = _
					goto Line
				}
				if *opt_T && !*opt_l != "" { fmt.Print("$.*END*		_") }
			}
 ***********************************************************************/
			if si <= 0 {
				blankLines++							// count lines in false conditional blocks as blank lines
				goto Line							// do not process lines in false blocks
			} else if opt_m_M_Y {
				/***********************************************************************
				 *	If -m -M or -Y then 8 or more blank are ignored
				 *	# $. "file" is output instead (same as cpp)
				 ***********************************************************************/
				if text == "\n" {
					blankLines++
					goto Line
				} else if blankLines > 0 {
					blanks()						// print blank lines now
				}
			}
//			resolve_line()							// resolve	macros for the rest of the code
			if opt_m_M_Y {
				genLine(text)						// just output macro processed line for <in.ic>
				goto Line							// end of Line for -m -M or -Y options 
			}

			/***********************************************************************
			 *	Analyse an iCa code line
			 *
			 *	Square bracket handling
			 *
			 *	FOR (N = 5; ...			x is a bare word; presumably a C variable
			 *
			 *	iCa:  []		[2]		[N]		[N+2]	  [x]	[2+x]	[N+x]	[N+2+x]
			 *	eval: []		@{[2]}	@{[N]}	@{[N+2]} [x]	[2+x]	[N+x]	[N+2+x]
			 *	iC:		  []*	2		5		7		  [x]*	[2+x]*	[5+x]*	[5+2+x]*
			 *	C:		  []	2		5		7		  [x]	[2+x]	[5+x]	[5+2+x]
			 *	* iC error message (will also produce error if compiled by immcc)
			 *
			 *	There is no difference in expanding square bracket expressions in iC or C.
			 *	The only difference is the output of error messages for iC code in 3 cases:
			 *	1)		an empty square bracket pair
			 *	2)		a bare word encountered in a square bracket pair
			 *	3)		a square bracket inside an embedded square bracket pair
			 *	These 3 cases are OK in C code and produce good C.
			 *
			 *	iCa:  [[]]		 [[2]]	[[N]]	[[N+2]] [[N]+x] [[N+2]+x] [[N]+2+x] [[N]+2+[N+1]]
			 *	iC:		  [[]]* [2]*	[5]*	[7]*	[5+x]*	[7+x]*	  [5+2+x]*	[5+2+6]*
			 *	C:		  [[]]* [2]	[5]		[7]		[5+x]	[7+x]	  [5+2+x]	[5+2+6]
			 *
			 *	The inside of an embedded square bracket pair is treated like iC code
			 *	ie. empty, bare words and more square brackets are not allowed.
			 *
			 *	iCa:   [[[]]]	[[x]]	[[2+x]] [[N+x]]	[[N+2+x]]	[[[N]+2]+x]
			 *	iC/C:  [[[]]]*	[[x]]*	[[2+x]]*  [[5+x]]*	[[5+2+x]]*	[[5+2]+x]*
			 *
			 *	Appropriate error messages will be output
			 *
			 *	The design goal is, that the generated Perl code will eval correctly
			 *	in all cases - error messages will give a good indication what is wrong
			 *	and immac exit status will prevent immcc compilation if iCmake is used.
			 *	Of course if immcc compilation is tried it will fail, but embedded iCa
			 *	error messages will still indicate the source of the error in iC listing.
			 *
			 *	sqNest
			 *	+= 1	every opening square bracket
			 *	-= 1	every closing square bracket until 0 (error if more)
			 *	square bit encoding which is used when square brackets close
			 *	0		outside square brackets
			 *	|= 1	number or translated loop variable or finishing string
			 *	|= 2	completed and expanded 2nd level nested square brackets
			 *	&=~2	this bit is cleared when another nested [ is encountered
			 *	|= 4	bare word unless state == 'S' - string is type 1 constant
			 *	|= 8	after closing square bracket if sqNest != 0
			 *	 = 0	after closing square bracket if sqNest == 0
			 *
			 *	The innermost square bracket pair is expanded to @{[expression]}
			 *	if square == 1 when the closing square bracket is encountered.
			 *	In other words this block may only contain numbers or translated loop
			 *	variables or finishing strings. Perl evaluates "expression" in an eval.
			 *	Syntactic errors in "expression" will cause a Perl eval error- this
			 *	should be rare, since index expressions are usually not complicated.
			 *	The Perl eval error message is easy to interpret to fix the iCa code.
			 *
			 *	Save state in sqState on opening square bracket and set to 'A'.
			 *	Restore state with matching closing square bracket. This allows
			 *	correct handling of strings in square brackets in comments and strings.
			 *	iC Example: ["abc".N] generates abc0 abc1 ... (meaningless for C indices).
			 *	This example is coded better as abc[N], but string may be conditional,
			 *	in which case it must be handled as an expression in square brackets.
			 *
			 *
			 *	Brace handling
			 *
			 *	Braces are used both in C and iC to identify blocks of code. They
			 *	are also used in iCa to mark blocks of code controlled by a FOR loop.
			 *	There are rare cases, where braces in C are deliberately not matched,
			 *	which throws out the matching of the correct closing FOR brace if
			 *	single braces for FOR were used.
			 *
			 *	To overcome this problem iCa uses double braces:
			 *	FOR (N = 0; N < 4; N++) {{
			 *	   a[N] ...
			 *	}}
			 *	Independent brace counting of the double braces is easy and leads to
			 *	clean translation code. An added advantage is, that visually the
			 *	boundaries of iCa FOR blocks are obvious and double braces have the
			 *	flavour of capital letters in the FOR keyword. It is also suggested,
			 *	that all FOR loop control variables be made capitals - similar to
			 *	the idea of capitalizing macros for the C pre-processor.
			 *
			 *	Literal blocks starting with '%{' mark the start of embedded C code.
			 *	A closing '%}' always marks the return to iC code independent of the
			 *	brace nesting of the C code in the literal block. This means that
			 *	%{ and %} are counted separately and error messages are output if any
			 *	nesting is attempted. Literal block (fragments) may be nested in a
			 *	FOR block (even only the start or the end). It would be pretty daft
			 *	to do so, but go figure - (quoting Larry Wall). In any case there
			 *	must always be alternte %{ and %} braces - even if only some of them
			 *	are in a FOR block.
			 *
			 *	%{
			 *		FOR CARRAY_NAME ("aa", "bb", "cc") {{
			 *	int		[CARRAY_NAME][[10]]		// expanding C array declaration
			 *	%}
			 *	%{
			 *		}}	// correct FOR loop end
			 *	%}
			 *
			 *	Brace counting of single braces must still be done to determine the
			 *	boundaries between iC and C code. The start of C code embedded in iC
			 *	are blocks starting with the keywords "if", "else" or "switch". The
			 *	end of embedded C code of if/else/switch blocks can only be determimed
			 *	by single brace counting and may turn out to be wrong in rare cases.
			 *	The only other use of braces in iC is for the body of function blocks.
			 *
			 *	if IX0.2 {
			 *		if IB0 != aa0[[0]] {		// start of C code
			 *			aa0[[0]] = IB0
			 *			printf("aa0[[0]] = %d\n", aa0[[0]])
			 *	FOR N (1 .. 2) {{		// unmatched FOR loop
			 *		}
			 *		else if IB[N] != aa[N][[0]] {
			 *			aa[N][[0]] = IB[N]
			 *			printf("aa[N][[0]] = %d\n", aa[N][[0]])
			 *	}}						// FOR loop end
			 *		}							// end of C code
			 *	}
			 *
			 *	Both the literal block example and the C example would be much more
			 *	sensibly coded by keeping the first closing brace above the FOR loop
			 *	and the final closing brace in the FOR loop. Either way the literal
			 *	block braces and in most cases the C braces must be in matching pairs.
			 *	Under all circumstances FOR loop double braces must match, and are
			 *	independent of bothe literal block and C code brace matching.
			 *
			 *	Here is a rather obscure example of (apparently) unmatched braces in
			 *	C code when expanding the initialisation of a C array in a FOR loop.
			 *
			 *	int aa[] = {\			// can be written thus: int aa[] = ["{"]\
			 *		FOR (N = 0; N <= 3; N++) {{
			 *	 [N][N < 3 ? ", " : " };"]\
			 *		}}
			 *
			 *	This expands to:		int		aa[] = { 0, 1, 2, 3 }
			 *
			 *	The first alternative does not work with the old single FOR brace dialect.
			 *	The second alternative of enclosing the constant string "{" in square
			 *	brackets, which evaluates simply to { and matches both braces in
			 *	parentheses, worked with the old iCa dialect, when string constants
			 *	in square bracket index expressions were introduced. It was this case,
			 *	which forced the re-design of the immac pre-processor.		  - }}
			 *
			 *	Expansion of square bracket expressions is identical for C and iC -
			 *	only the error messages differ in iC. Thus nesting errors for single
			 *	braces will only cause wrong error messages for square bracket
			 *	expressions containing bare words (not allowed in iC). The output will
			 *	in most cases be nearly correct and errors should be easy to interpret
			 *	at the iC compile stage. (immcc will complain loudly about remaining
			 *	square brackets which do not belong to an immC array variable).
			 *
			 ***********************************************************************/
/***********************************************************************
			do {
				if *opt_a {
					if rest && rest =~ m/^\s*(ELSE|ELSIF)/ {
						comment2nd = 1
					} else {
						comment =~ s/\s*(\/[\/*].*)?$//			// {{
						if comment =~ m/^\s*}}$/ && _ =~ m/(ELSE|ELSIF)/ {
							my b = pop(@block)
							if *opt_l != "" { fmt.Fprintf(genFile, "#--pop \_=b") } && *opt_T
							comment .= _
							lfFlag = 1
						} else {
							comment = _
						}
						comment2nd = 0
					}
					if comment =~ m/\b(IF|ELSE|ELSIF)\b/ {
						foreach id (@ids) {
							if id {						// use lookaround asserions to avoid double change
								comment =~ s/(?<!\$)\bid\b(?!=\$)/(id=identifiers[ id ])/g
							}
						}
					}
					if *opt_l != "" { fmt.Fprintf(genFile, "##**** $.: EndP=commentEndPos EndE=commentEndEnd LfF=commentLfFlag 2nd=comment2nd :", comment?comment:"**_") } && *opt_T
				} else {
					comment = _
				}
				rest = ""
				^***********************************************************************
				 *	 Start of detailed analysis of code lines
				 *	 extract atoms to drive state machine - save with position
				 *	 ignore leading backslash pairs
				 *	 include odd backslash to change atom for some lookups - \" != "
				 ***********************************************************************^
				@atoms = ()
				@nlTabs = ()
				control = sqNest = square = offset = 0
				while (
					//	m/
					//		(					# $1 leading backslah pairs
					//			(\\\\)*			# $2
					//		)
					//		(					# $3 found token
					//			\\?				#	 with optional leading backslash
					//			(				# ($4)
					//				"	|		#	 " \"
					//				'	|		#	 ' \'
					//				\[	|		#	 [ \[
					//				\]	|		#	 ] \]
					//				\n			#	 EOL \EOL
					//			)		|
					//			%?				#	 with optional leading %
					//			(				# ($5)
					//				\{\{? |		#	 { {{ %{ (%{{)	  {{ balance
					//				}}?			#	 } }} %} (%}})
					//			)
					//					|		#	 rest with no extra leading characters
					//			\/\/	|		#	 C++ comment
					//			\/\*	|		#	 start of C comment
					//			\*\/	|		#	 end of C comment
					//			\w+		|		#	 word identifier
					//			\\n		|		#	 char LF
					//			\\t		|		#	 char TAB
					//			\S				#	 any other non-space character
					//		)
					//	/xg
				) {
					atoms = append(atoms, [ length($`)+length($1), $3 ])		// [ pos, atom ]
				}
				^***********************************************************************
				 *		\n		end of line to terminate rest of line comments
				 *				if preceded by \ it only affects end of C comment after braces
				 *		"	'	string and character quotes				(may be preceded by \)
				 *		[	]	iCa index expressions - also used in C code		(also \)
				 *		\\n \\t string NL and TAB in final [  ] is replaced by real NL and TAB
				 *		//	#	rest of line comments
				 *		/%%%*  *%%%/	C style comments
				 *		%{	%}	literal blocks
				 *		{	}	iC/C compound statements blocks
				 *		{{	}}	blocks controlled by a FOR IF ELSE or ELSE IF statement
				 *		\w+		words and numbers - any type of program atom
				 *		=	;	statement delimiter to trigger a preceding/follow on in FOR {{
				 *		\S		any other non-space character starts new output after }}
				 *	check atoms in sequence - positions in line may change now
				 *	translate atoms only in square brackets in code, comments and strings
				 *	and in iCa FOR control lines
				 ***********************************************************************^
				for (my aix = 0; aix < @atoms; aix++) {
					my ref = atoms[aix]
					( pos, atom ) = @ref
					if (forEnd &&
						not sqNest &&
						state == 'A' &&
						atom !~ m!^(/[/*]|#|\n)$!) {	// keep any trailing comment with the brace line
						if atom != "\\\n" {
							rest = substr(_, forEnd, 1000, "\n"); # split FOR after braces or C comment
						}
						control = 0x6 if control == 0x2 // no line termination if closing braces
						last							// output up to here as a separate line
					}
					if atom =~ m/^(0|[1-9]\d*)$/ {		// decimal integer constant
						square |= 0x1 if sqNest // numeral in square brackets (iC Ok, no change)
					} else if atom =~ m/^\w+$/ {		// may be a bare word starting with a digit
						if sqNest || forFlag {
							len = length atom	// translate in strings and comments also
							if defined(translate = identifiers[ atom ]) {
								substr(_, pos + offset, len) = translate
								offset += length(translate) - len
								square |= 0x1 if sqNest // translation in square brackets (iC Ok)
							} else if sqNest {			// not in FOR line
								if state != 'S' {
									square |= 0x4		// bare word in square brackets (iC error / C Ok)
									sqE = append(sqE, atom)
								}
							} else if state == 'A' {	// bare word in FOR line - comment or string ignored
								if forFlag == 2 {
									substr(_, pos + offset + len, 0) = "\""
									substr(_, pos + offset, 0) = "\""
									offset += 2 // change bare word to quoted string variable
								} else {
									lnErr = append(lnErr, "//* Error: C variable 'atom' (bare word) in FOR line. File argIn, line $.\n")
									ret++				// error return
								}
							}
						} else if (not defined compound &&		// not sqNest and not forFlag
							state == 'A' &&						// not in comments or strings
							(atom == "if" || atom == "else" || atom == "switch")) {
							iesFlag = 1					// start of iC if else or switch control line
						}
					} else if atom == "\\n" || atom == "\\t" {	// look for innermost embedded string NLs or TABs in square brackets
						nlTabs = append(nlTabs, [ pos + offset, atom ]) // [ pos, atom ]
					}
					if atom == "[" {					// not else if because "FOR" is looked for in state 'A'
						if !sqNest++ {			// opening square bracket pair
							sqSave = state
							state = 'A'			// allows index expressions with strings in comments
							@sqE = ()					// collect new set of bare words
							sqP = pos			// note position of first bracket in a possibly nested block
						}
						sqStart = pos + offset	// save for changes to innermost bracket pair
						square &= ~0x2			// possible start of another embedded bracket pair
					} else if atom == "]" {
						if sqNest {
							sqS = "???"
							sqS = substr(comment, sqP, pos-sqP+1) if comment	// for error messages
							if square == 0x1 {					//	 1					iC/C Ok - expand
								substr(_, pos + offset, 1) = "]}"		// end of bracket pair with expression
								while (my ref = pop @nlTabs) {	// extract last first
									my ( nlP, nlA ) = @ref
									if nlP > sqStart {
										substr(_, nlP, 2) = nlA == "\\n" ? "\n" : "\t"
										offset -= 1				// replace string NL or TAB by real \n or \t
									}							// inside final eval brackets
								}								// in reverse order to keep offsets correct
								substr(_, sqStart, 1) = "\@{["	// start of bracket pair - eval in Perl
								offset += 3						// offset += 2 - 1 + 3 - 1
							}
							if --sqNest {
								if square == 0x1 {
									square |= 0x2				// mark good embedded bracket pair (iC error / C Ok)
								} else {
									square |= 0x8				// stop further eval - allows error message for deep nesting
								}
							} else {
								if *opt_T && !*opt_l != "" { fmt.Print("$.:pos:square	_") }
								if sqSave !~ m/C/ && !*opt_t {
									if square > 0x8 {	// the following are not errors in a comment or -t
										lnErr = append(lnErr, "//* Error: Index expression sqS in C or iC code has invalid nesting. File argIn, line $.\n")
										ret++			// error return
									} else if square == 0x8 {
										lnErr = append(lnErr, "//* Error: Empty nested index expression sqS in iC or C code. File argIn, line $.\n")
										ret++			// error return
									} else if !defined compound {		// empty [] and nested [[3]] now allowed in iC code
										if square & 0x4 {
											lnErr = append(lnErr, "//* Error: Index expression sqS in iC code contains C variable @sqE. File argIn, line $.\n")
											ret++				// error return
										}
									}
								}
								square = 0						// no other change outside nested square bracket
								state = sqSave
								@sqE = ()
							}
						} else if !defined compound {
							lnErr = append(lnErr, "//* Error: lone ']' outside of square brackets\n")
							ret++								// error return
							lnErr = append(lnErr, "//* Trace: argIn line $.: \compound = 'compound', \braceCount = 'braceCount', \twinCount = '@{[twinCount-0x100]}'\n") if *opt_T && !*opt_l != ""
						}
					} else if atom == "\\[" || atom == "\\]" {
						substr(_, pos + offset, 1) = ""
						offset -= 1								// allows square brackets (in comments)
					} else {
						if state == 'A' {						// iC - Program
							if atom == "\"" {					// leading backslash will not compare
								state = 'S'						// start of string found
							} else if atom == "'" {				// leading backslash will not compare
								state = 'H'						// start of character constant found
							} else if atom == "/*" {
								state = 'C'						// start of C comment found
								if sqNest {
									lnErr = append(lnErr, "//* Error: Unmatched square bracket at start of C comment; sqNest ] missing\n")
									ret++						// error return
									sqNest = 0			// sync square brackets
								}
							} else if atom == "*%%%/" {
								if sqNest {
									lnErr = append(lnErr, "//* Warning: Unmatched square bracket at end of C comment; sqNest ] missing\n")
									w++					// warning found
									sqNest = 0			// sync square brackets
								} else {
									lnErr = append(lnErr, "//* Warning: *%%%/ found after end of comment\n")
									w++					// warning found
								}
							} else if atom == "//" || atom == "#" {
								state = "CP"					// start of C++ or Perl comment found
								^***********************************************************************
								 *	 C-pre-processor directives are taken out earlier
								 ***********************************************************************^
							} else if (ref = iCaKey[atom]) {
								if *opt_t { return 1 }			// test has found iCa code FOR IF ELSE IF or ELSE
								if aix {						// iC/C code before braces - output first
									rest = substr(_, pos+offset, 1000, "")
									s/\\?\s*$/\\\n/		// strip trailing spaces from split part - \
									if comment { substr(comment, 0, pos, "") }
									last						// output up to here as a separate line
								}
								( tran, len, control ) = @ref
								substr(_, pos + offset, len) = tran		// "FOR" to Perl "for", "IF" to "if" etc
								if atom == "FOR" {
								  retry:
									ref = atoms[aix + 1]		// next atom in @atoms [int] identifier
									if ref(ref) == "ARRAY" {
										( pos, identifier ) = @ref
										if identifier == "(" {
											aix++				// skip over "("
											goto retry
										}
										if identifier == "int" {
											substr(_, pos + offset, 4) = ""
											offset -= 4 // delete "int " from perl eval code
											aix++				// skip over "int"
											goto retry
										}
										if !forFlag {
											forFlag = 1			// translate atoms till next (twin) brace
											substr(_, pos + offset, 0) = "my "
											offset += 3
											if len(block) == 0 {
												%identifiers = ()		// the first FOR line after eval
												@ids = ()
												el = $.			// start of block to eval for error reporting
											}
											if !defined identifiers[ identifier ] {
												if *opt_a {
													foreach id (@ids) {
														if id { // use lookaround assertions to avoid double change
															comment =~ s/(?<!\$)\bid\b(?!=\$)/(id=identifiers[ id ])/g
														}
													}
												}
												ids = append(ids, identifier)	// each iCa FOR loop only has one identifier
												identifiers[ identifier ] = "\identifier"
												finBlock = ++inBlock
												spOfs = 1
												forFlag = 2				// proper identifier for this FOR line found
											} else {
												lnErr = append(lnErr, "//* Warning: FOR 'identifier' used twice\n")
											w++					// warning found
											}
										} else {
											lnErr = append(lnErr, "//* Warning: another "FOR" used before '{{'\n")
											w++					// warning found
										}
									} else {
										lnErr = append(lnErr, "//* Warning: "FOR" not followed by identifier\n")
										w++								// warning found
									}
								} else {
									^***********************************************************************
									 *	If IF occurs before a FOR, then only the block marked by its double
									 *	braces is eval'd. A following ELSE and its block will be eval'd
									 *	separately, which will cause a Perl error (else without previous if).
									 *	To make sure it executes correctly opt_L is set after seeing IF, which
									 *	will put the rest of the code in one eval, which is OK.
									 ***********************************************************************^
									if atom == "IF" && !inBlock {
										opt_L = 1				// keep IF and ELSE in same eval block
									}
									ids = append(ids, "")				// dummy identifier when IF ELSE IF or ELSE
									finBlock = ++inBlock
									spOfs = 1
									forFlag = 2			// proper control statement
								}
							} else if atom =~ m"^\{\{?$" {
								if *opt_T && !*opt_l != "" { lnErr = append(lnErr, "//# OPEN_BRACE: argIn line $.: atom \compound = 'compound', \braceCount = 'braceCount', \twinCount = '@{[twinCount-0x100]}'\n") }
								if forFlag {
									iesFlag && die "Compiler error: \iesFlag should not be set if \forFlag"
									if forFlag == 1 {
										lnErr = append(lnErr, "//* Warning: FOR line has no control variable ???\n")
										w++						// warning found
									}
									if !control {
										spOfs = 2
										control = 0x8			// free standing brace (cannot happen any more)
									}
									forFlag = 0			// stop translating atoms in FOR control line
									forEnd = pos + offset + 1	// in case FOR line continues
									if atom == "{{" {
										forHash{ twinCount++ } = 1		// expecting twin braces
										substr(_, pos + offset, 1) = "" // change to "{"
										offset -= 1				// applies to next atom, not this one
									} else {
										forHash{ braceCount++ } = 1		// accept old dialect anyway
										if opt_S {
											lnErr = append(lnErr, "//* Error: strict: FOR line requires '{{'\n")
											ret++				// error return
										}
									}
								} else {
									if iesFlag {				// end of if else switch or literal block control line
										compound = braceCount; # start of a C compound statement in iC code
										iesFlag = 0
									}
									braceCount++				// current brace count
									if atom == "{{" {
										substr(_, pos + offset, 1) = ""
										offset -= 1
										lnErr = append(lnErr, "//* Error: iC/C code should not use '{{'\n")
										ret++					// error return
									}
								}
							} else if atom == "}" {
								count = --braceCount
								if *opt_T && !*opt_l != "" { lnErr = append(lnErr, "//# CLOSE_BRACE: argIn line $.: atom \compound = 'compound', \braceCount = 'braceCount', \twinCount = '@{[twinCount-0x100]}'\n") }
								if defined forHash[ braceCount ] {
									if opt_S {
										lnErr = append(lnErr, "//* Error: strict: FOR line requires '}}'\n")
										ret++					// error return
									}
									goto alternate				// support old dialect
								}
							  single:
								if defined compound {
									if braceCount <= compound; { undef compound }		// end of C compound statement
								}
							} else if atom == "}}" {
								count = --twinCount
								if defined forHash[ count ] {
								  alternate:
									if aix {					// iC/C code before braces - output first
										if count < 0xf0 {		// atom == "}"
											braceCount++		// restore for next round
										} else {				// atom == "}}"
											twinCount++ // in either mode
										}
										rest = substr(_, pos+offset, 1000, "")
										s/\\?\s*$/\\\n/ // strip trailing spaces from split part - \
										if comment { substr(comment, 0, pos, "") }
										last					// output up to here as a separate line
									}							// aix == 0 closing braces at start of line
									ref = atoms[aix + 1]		// next atom in @atoms maybe ELSE or ELSE IF
									if ref(ref) == "ARRAY" {
										( pos1, identifier ) = @ref
										if identifier =~ m/^(ELSE|ELSIF)$/ {
											rest = substr(_, pos1+offset, 1000, "")
											s/\\?\s*$/\\\n/		// strip trailing spaces from split part - \
										}
									}
									forEnd = pos + offset + 1	// in case "}}" line continues
									if count >= 0xf0 {	// atom == "}}"
										substr(_, pos + offset, 1) = ""
										offset -= 1				// strip first brace
									}
									inBlock--					// FOR IF ELSE IF ELSE control block finishes with brace
									if m"}\s*\\" {
										control = 0x6			// - { {
										s/}(\s*\\)/}/			// only first instance
										offset -= length $1
										comment =~ s/\\$//
									} else {
										control = 0x2
									}
									spOfs = 0
									delete forHash[ count ]

									if idPop := pop ids; idPop != "" {	// dummy members for IF ELSE ELSE IF keep order
										delete identifiers[ idPop ]; // latest identifier out of scope
									}
									if rest { last }			// output up to here as a separate line
								} else {
									lnErr = append(lnErr, "//* Warning: Unmatched '}}'\n")
									w++					// warning found
									goto single			// keep going - makes output cleaner
								}
							} else if atom == "%{" {
								if iesFlag || defined compound {
									lnErr = append(lnErr, "//* Warning: Attempt to use '%{' in C code - not correct\n")
									w++					// warning found
									iesFlag = 0
								}
								compound = braceCount	// start of a C literal block in iC code
								braceCount++					// current brace count
								if sqNest {
									lnErr = append(lnErr, "//* Error: Unmatched square bracket at start of C literal block; sqNest ] missing\n")
									ret++						// error return
									sqNest = 0			// sync square brackets
								}
							} else if atom == "%}" {
								--braceCount
								if defined compound {
									if braceCount <= compound; { undef compound }		// end of C compound statement
								} else {
									lnErr = append(lnErr, "//* Warning: Unmatched '%}' - not correct immediate C\n")
									w++					// warning found
								}
								if sqNest {
									lnErr = append(lnErr, "//* Error: Unmatched square bracket at end of C literal block; sqNest ] missing\n")
									ret++						// error return
									sqNest = 0			// sync square brackets
								}
							} else if atom == ";" {
								if forFlag == 2 { forFlag = 3 } // expression FOR line
								if sqNest {
									lnErr = append(lnErr, "//* Error: Unmatched square bracket at end of statement; sqNest ] missing\n")
									ret++						// error return
									sqNest = 0			// sync square brackets
								}
							} else if atom == "=" && forFlag == 2 {
								forFlag = 3						// expression FOR line
							}
						} else if state == 'S' {				// string constant
							if atom == "\"" {					// leading backslash will not compare
								state = 'A'
								if sqNest { square |= 0x1 }		// string in square brackets (no change)
							}
						} else if state == 'H' {				// character constant
							if atom == "'" {					// leading backslash will not compare
								state = 'A'
							}
						} else if state == 'C' {				// C comment
							if atom == "*%%%/" {
								state = 'A'						// end of C comment
								if forEnd; { forEnd = pos + offset + 2 }		// split after C comment
							} else if atom == "/*" {
								lnErr = append(lnErr, "//* Warning: /* found during comment\n")
								w++								// warning found
							}
						} else if state == "CP" {				// C++ or Perl comment
							if atom =~ m/^(\\?\n)$/ {
								state = 'A'						// end of C++ or Perl comment
							}
						}
					}
				}
				forEnd = 0										// end of line or split line
				if sqNest && !defined compound {
					lnErr = append(lnErr, "//* Error: Unmatched square bracket at end of line; sqNest ] missing\n")
					ret++				// error return
				}
				^***********************************************************************
				 *	End of line analysis
				 *	Generate Perl code for an eval block
				 ***********************************************************************^
				if finBlock {
					finBlock = inBlock			// { balance
					if *opt_T && !*opt_l != "" { lnErr = append(lnErr, "//# END_ANALYSIS: argIn line $.: \compound = 'compound', \braceCount = 'braceCount', \twinCount = '@{[twinCount-0x100]}'\n") }
					if m/(\@\{\[|\]})/ {
						while (m/([\w.]+)(\s+)\@\{\[/g) {
							if !defined keywords[$1] {
								save = pos(_)
								pos(_) -= length($2) + 3
								s/\G\s+//				// remove spaces after array name and . unless keyword
								pos(_) = save	// .. and ... are declared as keywords - leave spaces
							}
						}								// { balnce
						while (m/\]}(\s+)([\w.]+)/g) {
							if !defined keywords[$2] {
								save = pos(_)
								pos(_) -= length($1) + length($2)
								s/\G\s+//				// and before array continuation and . unless keyword
								pos(_) = save	// .. and ... are declared as keywords - leave spaces
							}
						}								// {{ balance
						if s/\]}\s*\@\{\[/]}x\@{[/g {	// insert x for multi-dimensional arrays { balance
							s/x(\@\{\[[^\]]*"[^\]]*\]})/$1/g;# remove x before [exp?",":""] special
						}								// } balance
						if s/(\d)\@\{\[/${1}y\@{[/g {	// insert y for array name ending in numerals { balance
							s/y(\@\{\[[^\]]*"[^\]]*\]})/$1/g;# remove y before [exp?",":""] special
						}								// } balance
					}
					if control || forFlag {
						if *opt_a {
							comment =~ s/\\/\\\\/g		// protect interpolated \ in string
							comment =~ s/"/\\"/g		// protect interpolated " in string
							chomp comment
							if comment {
								if lfFlag {
									commentOut = "\FOR .= \"\\n//***** comment\\n\";\n"
									commentLfFlag = 1
									lfFlag = 0	// lines terminated by '\' are now terminated by LF in genFile
								} else {
									commentOut = "\FOR .= \"//***** comment\\n\";\n"
									commentLfFlag = 0
								}
							}
						}
						s!(\s*(/[*%%%/]|#).*|[ \t]+)$!! // delete C, C++ or Perl comment and trailing blanks
					} else {
						s/\\/\\\\/g						// protect interpolated \ in string
						s/"/\\"/g						// {{ - protect interpolated " in string
						s/\]}\[/]}"."[/g				// insert something between @{[]}[ - needed for Perl interpretation
						chomp
						if s/\\\\$// {
							if (spOfs &&				// this line is not terminated by a LF
								s/^(\s*)(\s)/ /) {		// repeat with single space unless there were none
								my space = $1 . ($2 == "\t" ? "		  " : "")
								if space {
									splice(@block, @block - spOfs, 0, "\FOR .= \"space\";\n")
									if *opt_l != "" { fmt.Fprintf(genFile, "###### splice: block[@block-spOfs]") } && *opt_T
								}
							}
							_ = "\FOR .= \"_\";\n"		// output line without LF in the FOR loop when eval'd
							lfFlag = 1			// terminate the last line in the block with LF
						} else {
							_ = "\FOR .= \"_\\n\";\n"	// output line with LF in the FOR loop when eval'd
							lfFlag = 0			// line already terminated wih LF
						}
						spOfs = 0						// was set in "FOR line and brace"
					}
					if *opt_a {
						if (control & 0x1) && comment && !comment2nd {
							if commentEndPos < len(block) && comment =~ m/(ELSE|ELSIF)/ {
								if *opt_l != "" { fmt.Fprintf(genFile, "####1# splice @{[scalar @block]} commentEndPos commentEndEnd \commentOut=commentOut") } && *opt_T
								splice(@block, commentEndPos, commentEndEnd, commentOut)
							} else {
								block = append(block, commentOut)		// push the FOR (), IF (), ELSE and ELSE IF () comment line for eval
								if *opt_l != "" { fmt.Fprintf(genFile, "####1# @{[scalar @block]} commentEndPos \commentOut=commentOut") } && *opt_T
							}
							commentEndPos = 65535
							commentEndEnd = 0
						}								// {
						if _ =~ m/^\s*}\s*$/ {
							commentEndPos = scalar @block
						} else {
							commentEndPos = 65535
						}
					}
					block = append(block, _)					// push the modified line for eval
					if *opt_l != "" { fmt.Fprintf(genFile, "###### \control=control scalar \@block=@{[scalar @block]} \lfFlag=lfFlag LfF=commentLfFlag EndP=commentEndPos\n###### \_=_") } && *opt_T
					if control & 0x2 {
						if *opt_a {
							if comment {
								if commentEndPos < len(block) && comment =~ m/(ELSE|ELSIF)/ {
									if *opt_l != "" { fmt.Fprintf(genFile, "####2# splice @{[scalar @block]} commentEndPos \commentOut=commentOut") } && *opt_T
									splice(@block, commentEndPos, 0, commentOut)
									commentEndPos = 65535
									commentEndEnd = 0
								} else {
									if commentEndPos < len(block) && !idPop && commentOut =~ s/= "\\n/= "/ {
										if *opt_l != "" { fmt.Fprintf(genFile, "####2# splice @{[scalar @block]} commentEndPos end='\FOR .= \"\\n\";'\n") } && *opt_T
										splice(@block, commentEndPos, 0, "\FOR .= \"\\n\";\n")	// splice LF in block
										commentEndEnd = 1
									}
									if comment =~ m/(ELSE|ELSIF)/ {
										commentOut =~ s/= "\\n/= "/		// remove leading LF
									}
									block = append(block, commentOut)	// push the FOR (), IF (), ELSE and ELSE IF () comment line for eval
									if *opt_l != "" { fmt.Fprintf(genFile, "####2# @{[scalar @block]} commentEndPos 'idPop' \commentOut=commentOut") } && *opt_T
								}
								lfFlag = 0				// lines terminated by '\' are now terminated by LF
							}
						} else if control == 0x2 && lfFlag {
							endPos = scalar @block
							block = append(block, "\FOR .= \"\\n\";\n")
							if *opt_l != "" { fmt.Fprintf(genFile, "#+++++ \_=\FOR .= \"\\n\";\n") } && *opt_T
							lfFlag = 0			// lines terminated by '\' are now terminated by LF
						} else if control == 0x3 && !lfFlag && endPos {
							my b = splice(@block, endPos, 1)
							undef endPos
							if *opt_l != "" { fmt.Fprintf(genFile, "#--spl \_=b") } && *opt_T
						}
					} else {
						undef endPos
					}
					if !finBlock && len(block) > 0 && !lfFlag && !opt_L {
						eval_block(\@block, "BLOCK @{[++evalBlock]} at ", el)
					}							// { balance
				} else if m/(\@\{\[|\]}|\\$/	// look for iCa index expressions and backslash outside block
					&& !*opt_t					// do not return for [] outside "FOR loop" - could be in comment
				) {								// can select either alternatives above or below
					while (m/([\w.]+)(\s+)\@\{\[/g) {
						if !defined keywords[$1] {
							save = pos(_)
							pos(_) -= length($2) + 3
							s/\G\s+//			// remove spaces after array name and . unless keyword
							pos(_) = save		// .. and ... are declared as keywords - leave spaces
						}
					}							// { balance
					while (m/\]}(\s+)([\w.]+)/g) {
						if !defined keywords[$2] {
							save = pos(_)
							pos(_) -= length($1) + length($2)
							s/\G\s+//			// and before array continuation and . unless keyword
							pos(_) = save		// .. and ... are declared as keywords - leave spaces
						}
					}							// {{{ balance
					s/\]}\s*\@\{\[/]}x\@{[/g	// insert x for multi-dimensional arrays
					s/(\d)\@\{\[/${1}y\@{[/g	// insert y for array name ending in numerals
					s/\\/\\\\/g			// protect interpolated \ in string
					s/"/\\"/g					// protect interpolated " in string
					s/\]}\[/]}"."[/g			// insert something between ]}[ - needed for Perl interpretation
					chomp
					if s/\\\\$//) {				// is line terminated by a backslash ? (remove it
						_ = "\FOR .= \"_\";\n"	// yes - output line without LF in the FOR loop when eval'd
						lfFlag = 1				// terminate the last line in the block with LF
					} else {
						_ = "\FOR .= \"_\\n\";\n"		// no - output line with LF in the FOR loop when eval'd
						lfFlag = 0				// line already terminated wih LF
					}
					block = append(block, _)			// push the modified line for eval
				} else if !*opt_t {
					if len(block) > 0 || lfFlag || opt_L {
						s/'/\\'/g
						if chomp {
							block = append(block, "\FOR .= '_\n';\n")	// push the modified line for eval with LF
							lfFlag = 0
						} else {
							block = append(block, "\FOR .= '_';\n")		// or without LF
						}
					} else {
						print					// no iCa index expressions - faster if direct print
					}
				}
				if !finBlock && len(block) > 0 && !lfFlag && !opt_L {
					eval_block(\@block, "", $.)
				}
			} while ((_ = rest))				// maybe the line was split
			^***********************************************************************
			 *	End of analysis and generation of an iCa code line
			 ***********************************************************************^
 ***********************************************************************/
		  Line:
			if len(lnErr) > 0 && !*opt_t { output_error(text) }
		}	// end of Scan line
		if l := len(inFileStk) - 1; l >= 0 {
			inFileC.Close()						// EOF of an include file reached
 genLine(fmt.Sprintf("//### end of include %q len(siStk) = %d si = %d", argIn, len(siStk), si))
			if len(siStk) > 0 || si != 1 {
//				lnErr = append(lnErr, fmt.Sprintf("//* end of #include %s #if ... #endif not matched - si = %d\n", argIn, si))
			}
			fi := inFileStk[l]
			inFileStk = inFileStk[:l]			// pop current file info
			argIn   = fi.name
			lineNo  = fi.line
			si      = fi.si
			inFileC = fi.file
			scanner = fi.scan
			if opt_m_M_Y {
// TODO				my l = $. + ((eof in) ? 0 : 1)
				genLine(fmt.Sprintf("# %d %q 2", lineNo+1, argIn))	// # 8 "file" 2
			}
			continue Level
		}
		break Level
	} // exhaust all nested include files
/***********************************************************************
	^***********************************************************************
	 *	If -m -M or -Y then output any blank lines at the end of the file
	 ***********************************************************************^
	if opt_m_M_Y && blankLines > 0 {
		blanks()								// print blank lines now
	}
	^***********************************************************************
	 *	All lines in the iCa file have been read
	 ***********************************************************************^
	if finBlock {
		lnErr = append(lnErr, "//* Error: at EOF - probably braces are not matched. File argIn, line $.\n")
		ret++			// error return
	}
	if FORline {
		lnErr = append(lnErr, "//* Error: at EOF - FOR line not complete. File argIn, line $.\n")
		ret++			// error return
	}
	if twinCount != 0x100 {
		lnErr = append(lnErr, "//* Error: at EOF - FOR line twin braces {{ @{[twinCount-0x100]} }} do not match. File argIn, line $.\n")
		ret++			// error return
	}
	if len(siStk) > 0 || si != 1 {
		lnErr = append(lnErr, "//* Warning: at EOF - missing %endif in argIn at line $. - ignored\n")
		w++				// Warning found
	}
	if len(block) > 0 { eval_block(\@block, "LAST BLOCK @{[++evalBlock]} at ", el) }
	if ret > 0 || w > 0 {
		lnErr = append(lnErr, "%{\n")
		if ret > 0 { lnErr = append(lnErr, "#error immac found ret compilation error@{[ret>1?"s":""]} - see comments in iC list file\n") }
		if w > 0 { lnErr = append(lnErr, "#warning immac found w compilation warning@{[w>1?"s":""]} - see comments in iC list file\n") }
		lnErr = append(lnErr, "%}\n")
	}
	if len(lnErr) > 0 && !*opt_t { output_error(text) }
	if braceCount && *opt_T && !*opt_l != "" { fmt.Println("//***** \braceCount = braceCount") }
 ***********************************************************************/

	if *opt_T {
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
				fmt.Printf("-P         %-15q %2d    %q\n", macro, clDefs[macro].num, clDefs[macro].str)
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
				fmt.Printf("%%%%define   %-15q %2d    %q\n", macro, defs[macro].num, defs[macro].str)
			}
		}
		if len(clDefs) > 0 || len(defs) > 0 {
			fmt.Println("*******************************************************")
		}
	}

CloseFiles:
	fmt.Fprint(genFile,
`}
}
`)
	genFile.Close() // close generated file
	/********************************************************************
	 *	Execute the generated GO file and read its output
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
				outLine = reCommaSemi.ReplaceAllString(outLine, ";")	// remove comma from ,; in generated code
				fmt.Print(outLine) // final output to stdout or -o output
			}
		}
	}
	if *opt_l == "" && ret == 0 {
		err = os.Remove(genName) // remove generated file unless log file or error
		if err != nil {
			log.Fatal(err)
		}
	} else if err := os.Chmod(genName, 0444); err != nil {
		log.Fatal(err)
	}
	if *opt_o != "" {
		if err := os.Chmod(*opt_o, 0444); err != nil {
			log.Fatal(err)
		}
	}
	if *opt_e != "" {
		if err := os.Chmod(*opt_e, 0444); err != nil {
			log.Fatal(err)
		}
	}
	if w > 0 {
		fmt.Fprintf(os.Stderr, "*** %d Warnings reported\n", w)
	}
	if ret > 0 {
		fmt.Fprintf(os.Stderr, "*** %d Errors reported\n", ret)
		ret = 2
	}
	os.Exit(ret)
} // main

/***********************************************************************
 *
 *		Scan %define or -P macro and save it in map defs[X] where
 *		X(a, b) (a * b) will be converted to   { (#1 * #2), 2 }
 *		Y(len)	(len * 8) will be converted to { (#1 * 8) , 1 }
 *		Z		32		  will be converted to { 32		  , 0 }
 *		DEF				  will be converted to { 1		  , 0 }
 *
 *		argument count is				.num
 *		the actual macro translation is .str
 *
 *		Called for -P command line macro definitions and %define
 *
 *		If no macros at all are defined defs == nil.
 *
 ***********************************************************************/

func scan_define(direcve, macro string) (string, string) {
	if *opt_T { fmt.Printf("//* %d:	%s %s\n", lineNo, direcve, macro) }
	if m := reMacro.FindStringSubmatch(macro); m != nil {
		//	m/^
		//	([A-Z_a-z]\w*)					# $1 $identifier (not optional)
		//	(								# $2 arguments in parentheses
		//	  \(							#	 no space allowed between identifier and opening parentheses
		//		\s*							#	 space before arguments is optional
		//		(							# $3 $arguments
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
		//	  (.+)							# $8 $translate
		//	)?								#	 whole of translate is optional (default is 1)
		//	$/x
		identifier := m[1]
		arguments  := m[3]					// has no leading and trailing spaces
		translate  := m[8]
		if translate == "" {
			translate = "1"					// default 1 for translate
		}
		val := ""
		if *opt_T { fmt.Printf("//* %d: identifier = %q arguments = %q translate = %q\n", lineNo, identifier, arguments, translate) }
// fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s`\n", lineNo, identifier, arguments, translate)
		translate = reHashHash.ReplaceAllLiteralString(translate, " #\\# ")	// ## must be embedded in spaces which will disappear
// fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s`\n", lineNo, identifier, arguments, translate)
		translate = reHash.ReplaceAllString(translate, "#\\$1")				// protect previous #'s in translate except parameters
// fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s`\n", lineNo, identifier, arguments, translate)
		/********************************************************************
		 *  Collect individual parameters from argument list of a function macro
		 *******************************************************************/
		parameters := make(map[string]*numNum)
		pCnt := 0										// required outside for loop
		if arguments != "" {							// if none, arguments is guaranteed to be ""
			for _, parameter := range reComma.Split(arguments, -1) {
				pCnt++ // no need to test parameters for being valid C words - extraction above ensures this
				parameters[parameter] = &numNum{ nm1: pCnt, nm2: 0 }	// start with positional index 1
			}
		}
		/********************************************************************
		 *  Replace parameters in the translation with #1 #2 etc
		 *  Handle C and C++ comments and "xyz" strings in macro translation
		 *  as well as 'x' character constants, taking care if \ escapes
		 *******************************************************************/
		stat2   := 0									// outside of string, character constant or comment
		offset2 := 0
		cPos    := 0
		tr      := translate							// do not modify translate while scanning it
		for _, posPair := range reTranslate.FindAllStringIndex(translate, -1) {
			//	m/
			//	(							# $1
			//		[A-Z_a-z]\w*	|		#	 word in translate
			//		\/[*\/]			|		#	 start of C or C++ comment
			//		\*\/			|		#	 end of C comment
			//		\\?["']					#	 string or character constant delimiters
			//	)
			//	/xg
			pos2  := posPair[0]
			atom2 := translate[ pos2 : posPair[1] ]
// fmt.Printf("posPair = %v pos2 = %d atom2 = `%s`\n", posPair, pos2, atom2)
			if stat2 >= 0 {
				if reWord.MatchString(atom2) {	// TODO should be `^[A-Z_a-z]`	// avoid parameter replacement in string and character constants
					if p, ok := parameters[atom2]; ok {	// map parameters is empty for object like macro
						al := len(atom2)
						r  := "#"+strconv.Itoa(p.nm1)	// change parameter to '#1' etc
						tr = substr(tr, pos2 + offset2, al, r)
						offset2 -= al - len(r)
						p.nm2++							// record that this parameter has occured in the translation
					}									// ignore missing parameters in translate (info cpp) Warning later
				} else if atom2 == "//" {
					stat2 = -1							// C++ comment
					tr = tr[:pos2+offset2]				// truncate
					break
				} else if atom2 == "/*" {
					stat2 = -2							// start of C comment */
					cPos  = pos2
				} else if atom2 == "\"" {				// '\"' will not compare
					stat2 = -3							// start of string
				} else if atom2 == "'" {				// "\'" will not compare
					stat2 = -4							// start of character constant
				}
			} else if stat2 == -2 {						// C comment in translate
				if atom2 == "*/" {
					al := pos2 - cPos + 2
					tr = substr(tr, cPos + offset2, al, " ")
					offset2 -= al - 1					// replace C comment by one space
					stat2 = 0							// end of C comment
				}
			} else if stat2 == -3 {						// in string constant
				if atom2 == "\"" {						// leading backslash will not compare
					stat2 = 0							// end of string constant
				}
			} else if stat2 == -4 {						// in character constant
				if atom2 == "'" {						// leading backslash will not compare
					stat2 = 0							// end of character constant
				}
			}
		}
		translate = tr
// fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s`\n", lineNo, identifier, arguments, translate)
		translate = reSpaces.ReplaceAllLiteralString(translate, " ")	// compress spaces to 1 (no leading spaces in translate)
// fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s`\n", lineNo, identifier, arguments, translate)
		translate = reTrSpaces.ReplaceAllLiteralString(translate, "")	// remove possible space from trailing C comment
 fmt.Printf("//* %d: identifier = %q arguments = %q translate = `%s` pCnt = %d\n", lineNo, identifier, arguments, translate, pCnt)
		if arguments != "" {							// if none, arguments is guaranteed to be ""
			val = translate+":"+strconv.Itoa(pCnt)
		} else {
			val = translate
		}
// fmt.Printf("identifier = %q val = %q\n", identifier, val)
		if ref, ok := clDefs[identifier]; ok {			 // exists
// fmt.Printf("ref = %v\n", ref)
			val1 := ref.str
			if ref.num > 0 {
				val1 += ":"+strconv.Itoa(ref.num)
			}
			if *opt_l != "" { fmt.Fprintf(genFile, "//# command line definition -P %s=%s has precedence over %define %s %s\n", identifier, val1, identifier, val) }
			if *opt_a { fmt.Printf("//***** -P %s=%s has precedence over %define %s %s\n", identifier, val1, identifier, val) }
		} else if ref, ok := defs[identifier]; ok {		 // exists
			val1 := ref.str
			if ref.num > 0 {
				val1 += ":"+strconv.Itoa(ref.num)
			}
			if val != val1 {
				lnErr = append(lnErr, fmt.Sprintf("//* Warning: re-definition '%s=%s' to '%s' - ignored\n", identifier, val1, val))
				w++										// warning found
			}
		} else {
			for k, p := range parameters {
				if p.nm2 == 0 {
					lnErr = append(lnErr, fmt.Sprintf("//* Warning: parameter %q (%s) missing in %q\n", k, "#"+strconv.Itoa(p.nm1), translate))
					w++					// warning found
				}
			}
/***********************************************************************
			if !opt_m_M_Y && translate !~ m/^\s*\d+\s*$/ && translate !~ m/[^ \t()\d*\/%+-]/ {
				my tran = eval translate		// not -m and not -M and not a constant but a constant expression
				if $@ {
					lnErr = append(lnErr, "//* Error: start of Perl eval File argIn, line $.\n")
					lnErr = append(lnErr, sprintf("// %2d:		%s", 1, _))		// append eval Perl code to iC file
					lnErr = append(lnErr, "//* Error: end of Perl eval\n")
					lnErr = append(lnErr, "/** Error messages from Perl eval:	 **\\\n")		// start of C comment
					lnErr = append(lnErr, split /^/, $@)										// eval error messages
					lnErr = append(lnErr, "\\** End Error messages from Perl eval **%%%/\n")	// end of C comment
					ret++						// eval error return
				} else {
					translate = tran
					chomp _
					_ = "_ ==> translate\n"
				}
			}
 ***********************************************************************/
			// %define X(a,b) (a * b) is   map defs[X] = { 2, "(#1 * #2)" }
			defs[identifier] = &strNum{ num: pCnt, str: translate }
/***********************************************************************
			if direcve != "-P" {
				if *opt_l != "" { fmt.Fprintf(genFile, "//# _") }
				if *opt_a && $. {
					blankLines--
					if opt_m_M_Y && blankLines > 0 {		// blankLines is at least 1 because it is a directive
						blanks()				// print blank lines now
					}
					print "//***** _"
				}
			}
 ***********************************************************************/
		}
		return identifier, val
	}
	lnErr = append(lnErr, fmt.Sprintf("//* Warning: scan_define: %s '%s' has bad characters - ignored\n", direcve, macro))
	w++									// warning found
	return "", ""
} // scan_define

/***********************************************************************
 *
 *	Resolve %%define, %define and #define macros in lines of code
 *	which may occur in the remaining lines after some have been excluded
 *
 ***********************************************************************/

func resolve_line() {
/***********************************************************************
	if %defs {
		lastAtom1 = lastTranslate = ""
		off = 0
		re = 0
		%used = ()
		if *opt_T { fmt.Println("//0 $.: re = re _ '_'") }
		_ = resolve(_)					// resolve macros in line recursively
		if *opt_T { fmt.Println("//5 $.: re = re _ '_'") }
		if *opt_l != "" { fmt.Fprintf(genFile, "//# SUB 'lastAtom1' 'lastTranslate' in $.: _") } && off && lastAtom1
	}
 ***********************************************************************/
} // resolve_line

/***********************************************************************
 *
 *	Resolve %%define, %define and #define macros
 *	Call resolve(translate) recursively
 *
 *	Two types of macros are resolved:
 *
 *		Object like macros which look like a C style word.
 *
 *		Function like macros where the word is followd by comma separated
 *		arguments in parentheses.
 *
 *	Do not resolve macros, which have already been resolved once in a
 *	previous recursion to avoid infinite recursion. (see info cpp)
 *
 ***********************************************************************/

func resolve() {
/***********************************************************************
	my line = shift @_
	my (atom1, atom, pos, translate, openP, @atoms1, prevAtom1, Aref, @realParameters, realPstring)
	my (stat, newS, oldS, iPos, rPos, tail, length, spLen, lPos, prevTranslate, defineFlag)
	re++
	if *opt_T { fmt.Println("//1 $.:	re = re line = 'line'") }
	defineFlag = 0
	prevAtom1 = ""
	undef pos
	undef translate
	while ($line =~						# look for 'word' 'word (' '%%word' '%word' or '#word' in $line
		m/(((%%|[%#])\s*)?[A-Z_a-z]\w*)((\s*)(\())?/g
		//	m/
		//	(								# $1
		//		(							# $2 optional %%, % or #
		//			(%%|[%#])				# $3
		//		\s*)?						#	 followed by optional spaces
		//		[A-Z_a-z]\w*				#	 followed by macro (not optional C style word)
		//	)
		//	(								# $4
		//		(\s*)						# $5 spaces preceding '(' (maybe null string)
		//		(\()						# $6 '(' marking start of function like macro
		//	)?								#	 missing for object like macro
		//	/xg
	) {
		iPos   = length($`)										// position of "word" found
		atom1  = $1												// "word" found
		length = length atom1
		spLen  = defined $5 ? length $5 : 0						// spaces after "word"
		openP  = $6												// '('
		tail   = $'												// string after "word" or "word ("
		lPos   = pos(line)
		if *opt_T { fmt.Println("//2 $.:		re = re atom1 = 'atom1' iPos = iPos lPos = lPos") }
		if !pos && prevAtom1 =~ m/^(%%|[%#])define$/ {
			defineFlag = 1		// suppress translation of redefined macro name (first word after %%, %define or #define)
		} else if defined (Aref = defs[ atom1 ] &&				// expand object like macro defined without parameters
			(Aref->[1] == 0 || openP) &&						// or function like macro defined and called with (
			defined (translate = Aref->[0])) {			// do not expand function like macro called as object
			pos = iPos
			if Aref->[1] > 0 && openP {					// function like macro ( with defined parameters ?
				rPos = pos + length + spLen + 1			//	yes expand parameters - ignore (...) if defined as object
				@atoms1 = ()									// analyse string after "word (" to find closing ")"
				while ($tail =~
					m/(\(|,|\)|\/[*\/]|\*\/|\\?["'])/g
					//	m/
					//	(					# $1
					//		\(		|		#	 opening parentheses
					//		,		|		#	 comma
					//		\)		|		#	 closing parentheses
					//		\/[*\/] |		#	 start of C or C++ comment
					//		\*\/	|		#	 end of C comment
					//		\\?["']			#	 string or character constant delimiters
					//	)
					//	/xg
				) {
					atoms1 = append(atoms1, [ rPos + length($`), $1 ])	// [ pos, atom ]
				}
				stat = newS = oldS = 1					// start of function like macro( found
				@realParameters = ()							// prepare to receive real parameters
				for (my aix = 0 ; aix < @atoms1; aix++) {
					my ref = atoms1[aix]
					( pos, atom ) = @ref
					if *opt_T && !*opt_l != "" { fmt.Println("//* $.: stat=stat		oldS=oldS		aix=aix pos=pos atom=atom") }
					if stat >= 0 {
						if atom == "//" {
							stat = -1							// C++ comment in macro replacement
							last								// should have finished earlier
						} else if atom == "/*" {
							oldS = stat
							newS = -2							// start of C comment
						} else if atom == "\"" {			// '\"' will not compare
							oldS = stat
							newS = -3							// start of string
						} else if atom == "'" {			// "\'" will not compare
							oldS = stat
							newS = -4							// start of character constant
						} else if stat == 1 {
							if atom == "(" {
								newS = 2						// nested parentheses
							} else if atom == "," {
								realPstring = substr line, rPos, pos - rPos
								realParameters = append(realParameters, realPstring)	// real parameters before ','
								rPos = pos + 1
							} else if atom == ")" {
								my n
								realPstring = substr line, rPos, pos - rPos
								realParameters = append(realParameters, realPstring)	// last real parameter before ')'
								^***********************************************************************
								 *	 Distribute real parameters to translate
								 ***********************************************************************^
								if (n = Aref->[1]) != len(realParameters) {
									lnErr = append(lnErr, "//* Error: Macro 'atom1' should have Aref->[1] real parameters. File argIn, line $.\n")
									ret++						// error return
									return line
								} else if n {
									n = 1
									foreach realPstring (@realParameters) {
										realPstring =~ s/^\s+// // remove leading spaces
										realPstring =~ s/\s+$// // remove trailing spaces
										realPstring =~ s/ +/ /g // actual cpp 4.5.1 collapses only spaces to 1 space
												// info cpp says all whitespace and comments should collapse to 1 space
										if translate =~ m/#\\#n/ {		// is this argument stringified ?
											my (string, stat2, pos2, atom2, offset2)	// yes
											var atoms2	= make([]string, 0)
											string = realPstring		// analyse replacement string for embedded strings
											stat2 = offset2 = 0 // outside of string, character constant or comment
											while ($string =~
												m/(\/[*\/]|\*\/|\\?["']|\\)/g
												//	m/
												//	(				# $1
												//		\/[*\/] |	#	 start of C or C++ comment
												//		\*\/	|	#	 end of C comment
												//		\\?["'] |	#	 string or character constant delimiters
												//		\\			#	 backslash
												//	)
												//	/xg
											) {
												atoms2 = append(atoms2, [ length($`), $1 ])		// [ pos2, atom2 ]
											}
											for (my aix2 = 0 ; aix2 < @atoms2; aix2++) {
												my ref = atoms2[aix2]
												( pos2, atom2 ) = @ref
												if stat2 >= 0 {
													if atom2 == "//" {
														stat2 = -1				// C++ comment
														last					// should have finished earlier
													} else if atom2 == "/*" {
														stat2 = -2				// start of C comment
													} else if atom2 == "\"" {	// '\"' will not compare
														stat2 = -3				// start of string
														substr(string, pos2 + offset2, 0) = '\\'
														offset2 += 1			// change bare '"' to '\"'
													} else if atom2 == "'" {	// "\'" will not compare
														stat2 = -4				// start of character constant
													}
												} else if stat2 == -2 { // C comment in stringified parameter
													if atom2 == "*%%%/" {
														stat2 = 0				// end of C comment
													}
												} else if stat2 <= -3 { // in string or character constant
													if (atom2 == "\\" ||		// bare '\' in string or character constant
														atom2 == "\"" || // bare '"' in string or character constant
														atom2 == "\\'") {		// "\'" in string or character constant
														substr(string, pos2 + offset2, 0) = '\\'
														offset2 += 1			// change leading '\' to '\\'
													} else if atom2 == '\\"' {
														substr(string, pos2 + offset2, 0) = '\\\\'
														offset2 += 2			// change '\"' to '\\\\"'
													}
													if stat2 == -3 {			// in string constant
														if atom2 == "\"" {		// leading backslash will not compare
															stat2 = 0			// end of string constant
														}
													} else if atom2 == "'" {	// leading backslash will not compare
														stat2 = 0				// end of character constant
													}
												}
											}
											translate =~ s/#\\#n/\"string\"/g	// stringify all #parameters into translate
										}
										translate =~ s/#n/realPstring/g unless defineFlag		// replace normal parameters in translate
										n++
									}
									translate =~ s/#\\/#/g						// re-constitute protected #'s
									translate =~ s/ ## ?//g						// concatenete parameters separated by ' ## '
								}
								^***********************************************************************
								 *	 Resolve translation with ( parameters )
								 ***********************************************************************^
								length = pos + 1 - iPos
								stat = 0
								last							// for (aix...) macro ( ) complete
							}
						} else if stat >= 2 {
							if atom == "(" {
								newS = stat + 1
							} else if atom == ")" {
								newS = stat - 1
							}
						}
					} else if stat == -2 {						// C comment in macro replacement
						if atom == "*%%%/" {
							newS = oldS					// end of C comment
						}
					} else if stat == -3 {						// string in macro replacement
						if atom == "\"" {						// leading backslash will not compare
							newS = oldS					// end of string
						}
					} else if stat == -4 {						// character constant in macro replacement
						if atom == "'" {						// leading backslash will not compare
							newS = oldS					// end of character constant
						}
					}
					stat = newS
				} // end of for
				if stat != 0 {
					lnErr = append(lnErr, "//* Error: Macro was terminated prematurely. File argIn, line $.\n")
					ret++						// error return
					return line
				}
			} // else object like macro with no parameters defined - ignored if ... after call
			^***********************************************************************
			 *	Resolve all translations
			 ***********************************************************************^
			if !used[ translate ] {
				if *opt_l != "" { fmt.Fprintf(genFile, "//# OLD 'atom1' 'translate' in $.: line") } && off++ == 0
				if *opt_T { fmt.Println("//3 $.:		re = re translate = 'translate' iPos = iPos lPos = lPos length = length line = 'line'") }
				used{ prevTranslate = translate } = 1
				translate = resolve(translate)			// start a new scan of translate to resolve embedded macros
				delete used[ prevTranslate ]
				atom1 = substr(line, iPos, length, translate)	// line is now changed
				lPos += length(translate) - length				// skip word or full translation
				if *opt_T { fmt.Println("//4 $.:		re = re translate = 'translate' iPos = iPos lPos = lPos length = length line = 'line'") }
			}
			lastTranslate = translate					// until no more macros are found
			lastAtom1 = atom1
		} // end of if word atom1 is a valid macro
		prevAtom1 = atom1										// no more embedded macros resolved
		pos(line) = lPos										// skip word or full translation in word scan
	} // end if while look for "word" or "word ("
	re--
	return line
 ***********************************************************************/
} // resolve

/***********************************************************************
 *
 *	Eval of a generated block from iCa code to produce expanded iC code
 *
 ***********************************************************************/

func eval_block() {
/***********************************************************************
	my (Rblock, msg, el) = @_
	my FOR = ""
	if *opt_l != "" { fmt.Fprintf(genFile, "\FOR = "";\n//# eval ${msg}LINE $.\n", Rblock) }
	eval "@Rblock"						// evaluate code block
	if $@ {
		lnErr = append(lnErr, "//* Error: start of Perl eval File argIn, line el\n")
		my line = 0
		for (@Rblock) {
			line++
			lnErr = append(lnErr, sprintf("// %2d:		%s", line, _))	// append eval Perl code to iC file
		}
		lnErr = append(lnErr, "//* Error: end of Perl eval (line lines) File argIn, line $.\n")
		lnErr = append(lnErr, "/** Error messages from Perl eval:	 **\\\n")	// start of C comment
		lnErr = append(lnErr, split /^/, $@)									// eval error messages
		lnErr = append(lnErr, "\\** End Error messages from Perl eval **%%%/\n")		// end of C comment
		ret++							// eval error return
	}
	@Rblock = ()						// end of this eval block
	if FOR != "" {
		FOR =~ s/,(\s*;)/$1/g			// remove comma from ,; in generated code
		print FOR						// actual output of this eval block
	}									// ignore empty output from eval - is algorithmically ok
	if *opt_l != "" { fmt.Fprintf(genFile, "print \FOR if \FOR;\n") }
 ***********************************************************************/
} // eval_block

/***********************************************************************
 *
 *	Output errors and warnings after completing analysis of one line.
 *
 ***********************************************************************/

func output_error(listLine string) {
	lnErr = append([]string{fmt.Sprintf("//* %s:%d: %s\n", argIn, lineNo, listLine)}, lnErr...) // precede error message(s) by one listing line
	for _, e := range lnErr {
		fmt.Fprint(os.Stderr, e) // output errors and warnings to terminal
		if *opt_o != "" || *opt_e != "" {
			fmt.Print(e)
		} // and output file
		if *opt_l != "" {
			fmt.Fprint(genFile, e)
		} // and log file
	}
	lnErr = []string{}
} // output_error

/***********************************************************************
 *
 *	Auxiliary function for parseIf() to convert and replace a word or
 *	a number in *expr
 *
 ***********************************************************************/

func replaceAtom(atom string, pos int, expr *string, offset *int) {
	if !reNum.MatchString(atom) {
		var arString string
		al := len(atom)
		if ref, ok := defs[atom]; ok {		// exists
			arString = ref.str				// "15"
		} else {
			arString = "0"					// "0" is default for undefined macro value
		}
		bl := len(arString)
		*expr = substr(*expr, pos + *offset, al, arString)
		*offset -= al - bl
	}										// number does not need to be replaced
} // replaceAtom

/***********************************************************************
 *
 *	Auxiliary function for parseIf() to replace an arithmetic expression
 *	in *expr
 *
 ***********************************************************************/

func replaceExpr(posStart int, posEnd int, arTail string, expr *string, offset *int) {
	arExpr := (*expr)[posStart:posEnd]
	al := len(arExpr)
	// arCond = resolve(arExpr)			// take care of possible function macros
	arCond := fmt.Sprintf("%s%s", arExpr, arTail)
	bl := len(arCond)
	*expr = substr(*expr, posStart, al, arCond)
	*offset -= al - bl
 fmt.Printf("	posStart = %d posEnd = %d arExpr = %q al = %d arCond = %q bl = %d offset = %d\n", posStart, posEnd, arExpr, al, arCond, bl, *offset)
} // replaceExpr

/***********************************************************************
 *
 *	Generate a precompiler constant expression for #if macro or #elif macro
 *	suitable for input to genIf()
 *
 *	#ifdef and #ifndef are handled by concatenating "defined " or "!defined "
 *	with 'macro' and setting 'def' = true in the call to parseIf().
 *	In that case 'macro' must be a single directive name which is either
 *	defined or not - if macro is an expression or a number an Error is reported.
 *
 *	Syntax errors with logical operators && and || as well as conditional
 *	operators == != <= etc are reported early before GO compilation.
 *
 *	Syntax errors with all other arithmetic operators, parentheses or
 *	other symbols are ignored here. They are reported by the GO compile.
 *	Such errors should be rare because arithmetic is not a common part
 *	of conditinal pre-compiler directives - but they are handled correctly.
 *
 ***********************************************************************/

func parseIf(macro string, def bool) string {
	expr      := macro
	macroLen  := len(macro)
	if *opt_T { fmt.Printf("//1 %d: parseIf(%s)\n", lineNo, macro) }
	remove_comment(&macro)						// take out individual C comments TODO check if necessary
	if *opt_T { fmt.Printf("//2 %d: parseIf(%s)\n", lineNo, macro) }
 fmt.Printf("//2 %d	%s length = %d\n", lineNo, macro, macroLen)
	pos       := 0
	pose      := 0
	posePrev  := 0
//	posLstart := 0
	posEstart := 0
	offset    := 0
	atom      := ""
	atomPrev  := ""
	wordNum := false
	condOp  := false
	logicOp := false
	state     := 'L'						// initial state
	//	reEvalIf: (\w+|&&|\|\||[=!<>]=|<<?|>>?|~)
	for _, posPair := range reEvalIf.FindAllStringIndex(macro, -1) {
		//	(
		//		\w+		|	# word identifier or integer number
		//		&&		|	# && logical and
		//		\|\|	|	# || logical or
		//		[=!<>]=	|	# == != <= or >= conditionals
		//		<<?		|	# < conditional or << operator
		//		>>?		|	# > conditional or >> operator
		//		~			# C complement operator "~" changed to GO "^"
		//	)
		posePrev = pose						// start of white space after previous atom
		atomPrev = atom
		pos      = posPair[0]
		pose     = posPair[1]
		atom     = macro[ pos : pose ]
		wordNum  = false
		condOp   = false
		logicOp  = false
		/***********************************************************************
		 *	State machine to parse boolean expressions for #if etc and to generate
		 *	equivalant GO code. All defined variables should have a numerical string
		 *	value or return the constant bool string "true" if preceded by 'defined'.
		 *	All undefined variables return the arithmetic string "0" or "false" if
		 *	preceded by defined.
		 *	bool GO expr	expression
		 *	true			defined A	// if A is defined with -D or -P or #define
		 *	false			defined B	// if B has not been defined
		 *	!false			!defined B
		 *	2 != 0			X			// if X has been defined with -D X=2
		 *	2 + 3 != 0		X + Y		// if Y has been defined with -D Y=3
		 *	2 <= 4			X <= Z		// if Z has been defined with -D Z=4
		 *	2 != 0 && 3 != 0	X && Y
		 *	2 < 4 || 3 > 4		X < Z || Y > Z
		 *	!(2 < ^4 || 3 > 4)	!(X < ~Z || Y > Z)
		 *	Spaces, all operators and parentheses - in fact all symbols except
		 *	words which are replaced by their defined value and the C operator
		 *	"~", which is replaced by the GO operator "^" - are copied exactly.
		 *	No parentheses are added - that causes problems with unary operators.
		 ***********************************************************************/
		if reWordNum.MatchString(atom) {
			wordNum = true
		} else if atom == "&&" || atom == "||" {
			logicOp = true
		} else if atom == "==" || atom == "!=" || atom == "<" || atom == "<=" || atom == ">" || atom == ">=" {
			condOp = true
		} else if atom == "~" {
			expr = substr(expr, pos + offset, 1, "^")	// change C logical negate to GO code
		}
		if state == 'L' {
			if atom == "defined" {
				loc := reDefined.FindStringIndex(expr)
				dl := loc[1] - loc[0]
				expr = substr(expr, loc[0], dl, "")		// remove first "defined/s*"
				offset -= dl
				state = 'D'
			} else if wordNum {
				posEstart = posePrev + offset			// start of an arithmetic expression with current offset
				replaceAtom(atom, pos, &expr, &offset)
				state = 'E'
			} else if logicOp || condOp {
				lnErr = append(lnErr, fmt.Sprintf("//* Error: wrong token %q directly after %q in #if etc\n", atom, atomPrev))
				ret++
			}
		} else if state == 'D' {
			if atom == "defined" {						// must test before word check
				lnErr = append(lnErr, "//* Error: \"#ifdef defined\" or \"#if defined defined\"\n")
				ret++
			} else if reWord.MatchString(atom) {
				var boolString string
				al := len(atom)
				if _, ok := defs[atom]; ok {
					boolString = "true"
				} else {
					boolString = "false"
				}
				bl := len(boolString)
				expr = substr(expr, pos + offset, al, boolString)
				offset -= al - bl
				state = 'F'
			} else if logicOp || condOp || wordNum {	// number only - word already excluded
				lnErr = append(lnErr, fmt.Sprintf("//* Error: wrong token %q in \"defined %s\" in #if etc\n", atom, atom))
				ret++
			}
		} else if state == 'F' {
			if logicOp && !def {
				state = 'L'
			} else if condOp || wordNum || def {
				lnErr = append(lnErr, fmt.Sprintf("//* Error: wrong token %q after \"defined %s\" in #if etc\n", atom, atomPrev))
				ret++
			}
		} else if state == 'E' {
			if logicOp {
				replaceExpr(posEstart, posePrev, " != 0", &expr, &offset)
				state = 'L'
			} else if condOp {
				state = 'M'
			} else if wordNum {
				replaceAtom(atom, pos, &expr, &offset)
			}
		} else if state == 'M' {
			if wordNum {
				replaceAtom(atom, pos, &expr, &offset)
				state = 'N'
			} else if logicOp || condOp {
				lnErr = append(lnErr, fmt.Sprintf("//* Error: wrong token %q directly after %q in #if etc\n", atom, atomPrev))
				ret++
			}
		} else if state == 'N' {
			if logicOp {
				replaceExpr(posEstart, posePrev, "", &expr, &offset)
				state = 'L'
			} else if wordNum {
				replaceAtom(atom, pos, &expr, &offset)
			} else if condOp {
				lnErr = append(lnErr, fmt.Sprintf("//* Error: wrong token %q after conditional expression in #if etc\n", atom))
				ret++
			}
		}
		/***********************************************************************
		 *	atom "~" changed to "^" as well as "<<" and ">>" are not modified
		 *	like all other symbols not found by this scan. If any of those
		 *	symbols form bad GO code it is reported by the GO compile stage.
		 ***********************************************************************/
 fmt.Printf("parseIf:	'%c' [ %d : %d ]	offset = %d	%q\n\t%s\n", state, pos, pose, offset, atom, expr)
	}
	if state == 'E' {
		replaceExpr(posEstart, pose+offset, " != 0", &expr, &offset)
	} else if state == 'N' {
		replaceExpr(posEstart, pose+offset, "", &expr, &offset)
	} else if state == 'L' || state == 'D' || state == 'M' {
		lnErr = append(lnErr, "//* Error: incomplete boolean expression in #if etc\n")
		ret++
	}
	if *opt_T { fmt.Printf("//3 %d: parseIf(%s) ==> %q\n", lineNo, macro, expr) }
 fmt.Printf("//3 %d:	%s\n\t%s\n", lineNo, macro, expr)
	return string(expr)						// modified to const boolean expression
} // parseIf

/***********************************************************************
 *	Generate GO code for the conditional directives #if #ifdef #ifndef
 *	as well as #elif #else and #endif
 *
 *	Care is taken that nested #if #endif groups are handled correctly
 *
 *	'condition' must be a string containing a GO constant boolean
 *	expression such as:
 *		true					// #ifdef A or #if defined A
 *		!false					// #ifndef A or #if ! defined A
 *		false || 4 != 0			// #if defined X || Y
 *		32767 == 32767 && true	// #elif INT_MAX == 32767 && defined (LONG16)
 *	'condition' is not used for #else and #endif
 *  'sel'       is a bit selector which nomimates ifx elif else or endif
 *	'text'      is the text line ontaining the conditional directive for -a
 *	            comment output
 ***********************************************************************/

func genIf(condition string, sel selector, text string) {
	if *opt_a || *opt_Y {
		fmt.Fprintf(genFile,		//          balance {
`} else {
	for FOR := 0; FOR < %d; FOR++ {
		fmt.Print("\n")
	}
`		, lineNo - blockStart)		//          balance }
	}
	fmt.Fprint(genFile, "}\n")
	if sel & (endif) != 0 {
		fmt.Fprint(genFile, "IFprev = IFstack[len(IFstack)-2]\n")
		fmt.Fprint(genFile, "IF = IFprev\n")
		fmt.Fprint(genFile, "IFstack = IFstack[:len(IFstack)-1]\n")
	}
	if *opt_a {
		if sel & (ifx) != 0 {
			fmt.Fprint(genFile, "if !IF { IFblock++ }\n")
		}
		fmt.Fprintf(genFile, "if IFblock == 0 { fmt.Print(\"//***** %s\\n\") } else { fmt.Print(\"\\n\") }\n", text)
		if sel & (endif) != 0 {
			fmt.Fprint(genFile, "if IFblock > 0 { IFblock-- }\n")
		}
	} else if *opt_Y {
		fmt.Fprint(genFile, "\tfmt.Print(\"\\n\")\n")
	}
	if sel & (ifx) != 0 {
		fmt.Fprint(genFile, "IFprev = IF\n")
	}
	if sel & (elif | elsx) != 0 {
		fmt.Fprint(genFile, "if IF { IFprev = false }\n")
	}
	if sel & (ifx | elif) != 0 {
		fmt.Fprintf(genFile, "IF = IFprev && (%s)\n", condition)
	}
	if sel & (ifx) != 0 {
		fmt.Fprint(genFile, "IFstack = append(IFstack, IF)\n")
	}
	if sel & (elsx) != 0 {
		fmt.Fprint(genFile, "IF = IFprev\n")
	}
	if sel & (elif | elsx) != 0 {
		fmt.Fprint(genFile, "IFstack[len(IFstack)-1] = IF\n")
	}
	if (*opt_T) {
		fmt.Fprint(genFile, "\tfmt.Printf(\"IFprev = %5v IF = %5v IFstack = %v\\n\", IFprev, IF, IFstack)\n")
	}
	fmt.Fprint(genFile, "if IF {\n")
	blockStart = lineNo + 1
} // genIf

/***********************************************************************
 *
 *	Take out individual C comments out of ref
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
 *	Generate a line to file 'genName' for execution as a GO file
 *
 ***********************************************************************/

func genLine(tx string) {
	if *opt_T { fmt.Printf("//* %3d: %s\n", lineNo , tx) }
	tx = reParenSlash.ReplaceAllString(tx, "\\$1")	// precede " and \ by extra \
	fmt.Fprintf(genFile, "\tfmt.Print(\"%s\\n\")\n", tx)
} // genLine

/***********************************************************************
 *
 *	Print a group of blank lines (only when *opt_m *opt_M or *opt_Y)
 *
 ***********************************************************************/

func blanks() {
	if *opt_T { fmt.Println("//*", lineNo, ":	blankLines =", blankLines) }
	if blankLines < 8 || *opt_Y {		// yacc does not understand # line_no //
		for n := 0; n < blankLines; n++ {
			fmt.Fprintf(genFile, "\tfmt.Print(\"\\n\")\n") // output block of 7 or less blank lines as is
		}
	} else {
		tx := fmt.Sprintf("# %d \\\"%s\\\"", lineNo, argIn) // # 23 "file"
		fmt.Fprintf(genFile, "\tfmt.Print(\"%s\\n\")\n", tx) // block of 8 or more blank lines
	}
	blankLines = 0
} // blanks

/***********************************************************************
 *
 *	Emulate the Perl substr() function
 *  substr(expr, offset, length, replacement)
 *
 ***********************************************************************/

func substr(expr string, offset, length int, replacement string) string {
	x := []rune(expr)
	x = append(x[:offset], append([]rune(replacement), x[offset+length:]...)...)
// fmt.Printf("substr: expr = `%s` offset = %d length = %d replacement = %q return = `%s`\n", expr, offset, length, replacement, string(x[:]))
	return string(x[:])
} // substr

/*********** POD to generate man page *******************************

=encoding utf8

=head1 NAME

immac - the immediate-C array and macro pre-compiler

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
the -l option, which can be run as a Perl script to test 'FOR loop'
generation. This produces better error messages.

The B<immac> compiler can be used to test for iCa language constructs
with the -t option.  If the source contains 'FOR loops', IF or ELSE
control statements or B<%%define> macro definitions, the source file
is an iCa language file.  Isolated [index expressions] without at
least one FOR IF ELSE or B<%%define> are not sufficient to classify
a file as an iCa language file.

=head1 EXIT STATUS

Normally, the exit status is 0 if the iCa file compiles correctly.
If a compile error occurs the exit status is 2.

For the -t option the exit status is 1 if a file is found to be an
iCa file and 0 otherwise.

=head1 SPECIFICATION

The immediate C language extension is as follows:

immediate array variables are defined by appending an expression
enclosed in square brackets to the array name eg. array[N+10]

unless the expression in the square brackets is a constant
expression eg. array[4+10], the line containing the expression
must be contained in a FOR block as follows:

    FOR (int N = 0; N < 8; N++) {{
        array[N+10],
    }}

B<immac> uses the FOR control statement to repeatedly output iC or
C statements contained in a block bounded by twin braces {{ ... }}
B<immac> used to use single braces just like blocks defining a function
block in iC or blocks in C. In rare cases, the braces used for iC/C
code would get mixed up with the braces marking an iCa FOR block. To
overcome this, twin braces were introduced.  It was also found, that
the iCa language was easier to read, if control variables used in a FOR
loop were upper case.  This is only a recomendation. the B<immax>
converter will convert old iCa code to the new dialect.

Another variant is to use the following "perlish" syntax using lists
or the Perl .. operator to generate lists, which produces the same
output as the first example:

    FOR int N (0 .. 7) {{
        array[N+10],
    }}

The 'int' type specifier of the control variable is optional - it
may be used in both variants to make the syntax look more natural.

With Perl type lists it is possible to use strings as well as numbers
as values for the loop variable. In iCa such strings in a FOR control
list can be bare words, although they may be enclosed in double quotes
- they are required when strings are used in an expression. These
string values may of course not be used with arithmetic operators
in index expressions, but the Perl concatenation operator '.' can
sometimes be used effectively. Three variants are shown:

    FOR N (aa, ab, ac, ad) {{ xyz_[N], }}
 or alternatively
    FOR N ("aa", "ab", "ac", "ad") {{ ["xyz_".N], }}
 or even
    FOR N ("aa" .. "ad") {{ xyz_[N], }}
 will all generate
    xyz_aa, xyz_ab, xyz_ac, xyz_ad,

    FOR N (0, 3, 4, 10, 5) {{ array[N+1], }}
 will generate
    array1, array4, array5, array11, array6,

In the above instances only the control variable 'N' as well as numeric
values and string constants may be used in index expressions of the
block. 'FOR' blocks may be nested.  In that case all the control
variables in enclosing nested blocks may be used.

All immediate C lines in the block may contain bracketed index
expressions, but they do not need to (they will of course not vary).
The lines in a  'FOR' block are repeated a number of times controlled
by the 'FOR' control line.

It must be remembered, that the control variable is either a numeric
interger or a string.  In the Perl code, 'use integer' has been
called, so integer division always applies - the following is valid
for numeric control variables:

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

A line terminated by a back-slash '\' both inside or outside a
'FOR loop' generates that line without a terminating LF '\n'.
This allows the generation of lists in a single line.  Normally a
LF is inserted on the termination of the 'FOR loop' unless the
final brace is also followed by a back-slash '\' - then the line
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

=head1 ALTERNATIVE MACRO PROCESSING

B<immac -m> is just a macro processor handling B<#define>, B<#undef>,
-D, -U, B<#include>, B<#if> etc, which makes it an alternative for
B<cpp> mainly used to pre-process C code. Every attempt has been made
to make B<immac -m> equivalent to B<cpp>.

B<immac -M> is an alternative macro processor handling B<%define>,
B<%undef>, -D, -U, B<%include>, B<%if> etc, which is called by
the iC compiler B<immcc> to resolve macro's in iC code (version
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

The -U option is also available in these cases.  All these alternatives
do not translate iCa language constructs.

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

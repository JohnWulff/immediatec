/***********************************************************************
 *
 *	Copyright (C) 2021  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	goserver.go; the TCP/IP server for iC clients - both apps and I/O's
 *
 *  based on the PERL version of iCserver, which uses the Messaging Toolkit
 *  from Advanced Perl Programming by Sriram Srinivasan, which communicates
 *  with TCP/IP messages consisting of a 4 byte header containing the
 *  BigEndian length of the data followed by the data.
 *
 *  The Perl program has been migrated to the GO language. The code
 *  implementing the lenth+message format from the Messaging Toolkit
 *  is contained in the GO package 'tcpcomm'.
 *
 ***********************************************************************/

package main

import (
    "tcpcomm"			// own package handling Perl type messages
    "net"
    "os"
    "os/exec"
    "os/signal"
    "bufio"
    "syscall"
    "sync"
    "flag"
    "io"
    "strings"
    "strconv"
    "regexp"
    "sort"
    "time"
    "fmt"
)

const ID_goserver_go = "$Id: goserver.go 1.4 $"
const TCP_PORT = "8778"			// default TCP port for iC system

type eq struct {
    channel	int
    equivP	*map[string]string	// hashed by instance - equivalences for 1 channel
}
type eqB struct {
    base	string
    primCh	int
    secCh	int
}
type altRecv struct {
    altsecCh	int			// secondary channel for these alternate receivers
    altCons	[]net.Conn		// slice of alternate receiver connections
}

var muConn	sync.Mutex		// guard activity associated with one message in handleConnection()
var muEquiv	sync.Mutex		// guard equivalences and equivBase modifications

/********************************************************************
 *  The following slice and map variables are common to all goroutines
 *  handleConnection(). They are indexed by 'conn' or 'channel' or
 *  names, which are not shared by the different goroutines except
 *  when errors occur, which are then reported.
 *******************************************************************/

var clientCons    = map[string]net.Conn{}  // hashed by name - client connection - existence of client
var clientNames   = map[net.Conn]string{}  // hashed by conn - client name for a connection
var channels      = map[string]int{}	   // hashed by name - channel of I/O S<name> and R<name>
var equivalences  = map[string][]*eq{}	   // hashed by base - equivalent names with same or different channel as base
var equivBase     = map[string]*eqB{}	   // hashed by name - base name of equivalenced names (even own name if base)

/********************************************************************
 *  The following 5 slices are all indexed by channel, which is
 *  always <= maxChannel. maxChannel starts at 1 and is only
 *  increased in allocateChannel() where len and cap of all
 *  5 slices is doubled when maxLen >= cap(senderCon)
 *******************************************************************/
var senderCon     = make([]net.Conn, 16)   // indexed by channel ... sender connections      conn
var senderName    = make([]string, 16)	   // indexed by channel ... sender names            name
var senderValue   = make([]string, 16)	   // indexed by channel ... current sender values   value
var receiverCons  = make([][]net.Conn, 16) // indexed by channel ... primary channel receiver connections
					   // [conn1 conn2 conn3 ...]
var receiverNames = make([][]string, 16)   // indexed by channel ... primary channel receiver names
					   // [name1 name2 name3 ...]

var altRecvCons   = map[int][]altRecv{}    // hashed by channel  ... secondary channel and receiver connections
					   //                        for an alternate channel
					   // [{channel1 [conn4 conn5 ...]} {channel2 [conn6 conn7 ...]} ...]
var altRecvNames  = map[int][][]string{}   // hashed by channel  ... receiver names for an alternate channel
					   // [[name4 name5 ...] [name6 name7 ...] ...]

var goCount, numberOfConnections, maxChannel int

/********************************************************************
 *
 *	Allocate a channel to a name
 *
 *	This gives each Sender and Receiver pair an index called 'channel'
 *	in the arrays 'senderCon', 'receiverCons' and map 'altRecvCons'.
 *	Each data transmission starts with this channel number.
 *	Some differing names may use the same channel (equivalences).
 *
 *******************************************************************/

func allocateChannel(name string, channel int) int {
    if ch, ok := channels[name]; ok {
	if channel > 0 {
	    warnD(fmt.Sprintf("attempting to allocate channel %d for %s:%d twice - ignore", channel, name, ch))
	}
	channel = ch			// previously allocated channel for this name
    } else {
	if channel == 0 {
	    maxChannel++
	    channel = maxChannel
	    if maxChannel >= cap(senderCon) {
		/********************************************************************
		 *  Double the capacity of al slices indexed randomly by 'channel'
		 *******************************************************************/
		senderCon     = append(senderCon, make([]net.Conn, maxChannel)...)
		senderName    = append(senderName, make([]string, maxChannel)...)
		senderValue   = append(senderValue, make([]string, maxChannel)...)
		receiverCons  = append(receiverCons, make([][]net.Conn, maxChannel)...)
		receiverNames = append(receiverNames, make([][]string, maxChannel)...)
		if *opt_t { fmt.Printf("/// len(senderCon) = %d cap(senderCon) = %d\n", len(senderCon), cap(senderCon)) }
	    }
	}
	channels[name] = channel
    }
    if *opt_t { fmt.Printf("%s: allocateChannel %q, %d\n", named, name, channel) }
    return channel
} // allocateChannel

/********************************************************************
 *
 *	Allocate equivalences  -  with the -e option in this server
 *	or from another iC app either
 *	    as a single equivalence IX0=IX0-1
 *	    or as a multiple equivalence IX0=IX0-1=IX0-2
 *	    or as group-equivalences IX0=IX2=IX4, which belong to one instance of
 *		an app characterized by having the same connection (this precludes
 *		them from being simple equivalences on the same channel)
 *	    or as a space or comma separated list of the above equivalences
 *
 *	equivalences[]		// Auxiliary structure only used to build equivBase[]
 *	hashed by base - points to equivalent names with same or different channel as base IX0
 *	equivalence group (usually only has one sub-group with the channel of the base name)
 *	        {channel equivP}
 *	[
 *	 IX0=>[&{ch0 [''=>IX0 '-1'=>IX0-1 '-2'=>IX0-2 ...]}	// channel same as base channel
 *	       &{ch1 [''=>IX1 '-1'=>IX1-1 '-2'=>IX1-2 ...]}	// members of another group with a different channel
 *	       &{ch2 [''=>IX2 '-1'=>IX2-1 '-2'=>IX2-2 ...]} ...]
 *	 IX5=> &{ch3 [''=>IX5 '-1'=>IX5-1 '-2'=>IX5-2 ...]} ...]
 *	 ...=> ...
 *	]
 *
 *	equivBase[]		// Used during registration and autovivification
 *	hashed by name - points to base name of equivalenced names (even own name if base)
 *	 name =>&{primaryBase primaryChannel secondaryChannel}
 *	]
 *	 IX0  =>&{IX0 ch0 ch0}
 *	 IX0-1=>&{IX0 ch0 ch0}
 *	 IX0-2=>&{IX0 ch0 ch0}
 *	 ...  => ...
 *	 IX1  =>&{IX0 ch0 ch1}
 *	 IX1-1=>&{IX0 ch0 ch1}
 *	 IX1-2=>&{IX0 ch0 ch1}
 *	 ...  => ...
 *	 IX2  =>&{IX0 ch0 ch2}
 *	 IX2-1=>&{IX0 ch0 ch2}
 *	 IX2-2=>&{IX0 ch0 ch2}
 *	 ...  => ...
 *	 IX5  =>&{IX5 ch3 ch3}
 *	 IX5-1=>&{IX5 ch3 ch3}
 *	 IX5-2=>&{IX5 ch3 ch3}
 *	 ...  => ...
 *	]
 *
 *	If no name in an equivalence is found in equivBase[] start a new
 *	equivalence set and group hashed by either the only output name in
 *	the set or the first input name in the set as primary base.
 *
 *	Or if a name in an equivalence group has already been registered it will
 *	have a secondary channel. Each equivalence set may only have one pre-
 *	registered input name, which will replace the primary base name of the set,
 *	even if a different name has already been allocated in previous equivalences.
 *	This ensures that the wrong input name is not autovivified on that channel
 *	leaving the pre-registered input element as a zombie with no receivers.
 *	(The alternative autovivfied input element actually does act correctly on
 *	the pre-registered input on the secondary channel originally allocated).
 *
 *	Else if simple equivalence add new name to current sub-group -
 *	Else start a new sub-group.
 *
 *	During registration a check is made that all names in a sub-group have
 *	different connections.
 *
 *******************************************************************/

func allocateEquivalences (arg string) {
    muEquiv.Lock()
    defer muEquiv.Unlock()
    errFlag := false
    reCommaSpace := regexp.MustCompile(`[, ]`)
    for _, equiv := range reCommaSpace.Split(string(arg), -1) {
	if equiv == "" { continue }			// ignore extra commas and spaces
	if ! *opt_q { fmt.Printf("Equivalence %q\n", equiv) }
	var equivSet = make([][2]string, 0, 16)
	var dupFlag bool
	var outName, xbwlh, base string
	var primCh, secCh, ch int
	/********************************************************************
	 *  The following are all part of a set of equivalence groups belonging to
	 *  one base IEC name. Scan once to see of this set extends a previous set
	 *  by checking in equivBase[].
	 *******************************************************************/
	for _, nn := range strings.Split(equiv, "=") {
	    if *opt_t { fmt.Printf("--- nn = %q\n", nn) }
	    reIQ_XBWLH := regexp.MustCompile(`^([IQ])([XBWLH])\d+((-\d{1,3})?)$`)
	    if m := reIQ_XBWLH.FindStringSubmatch(nn); len(m) != 5 {	// `^([IQ])([XBWLH])\d+((-\d{1,3})?)$`
		fmt.Fprintf(os.Stderr, "ERROR %s: %q in %q is not an IEC name - will not work!\n", named, nn, equiv)
		errFlag = true
	    } else {
		iq := m[1]
		if iq == "Q" {
		    if outName != "" {
			if outName != nn {
			    fmt.Fprintf(os.Stderr, "ERROR %s: %q in %q is 2nd output IEC name after outName - will not work!\n", named, nn, equiv)
			    errFlag = true
			}
		    } else {
			outName = nn
		    }
		}
		if xbwlh != "" {
		    if xbwlh != m[2] {
			fmt.Fprintf(os.Stderr, "ERROR %s: %q in %q is not the same IEC type - will not work!\n", named, nn, equiv)
			errFlag = true
		    }
		} else {
		    xbwlh = m[2]			// IEC type of first equivalence entry
		}
		inst := m[3]				// instance (may be null string)
		if sl, ok := equivBase[nn]; ok {
		    base   = sl.base
		    primCh = sl.primCh
		    secCh  = sl.secCh
		} else {
		    ch = channels[nn]
		    if *opt_t { fmt.Printf("%%%%%% N nn = %q inst = %q base = %q primCh = %d secCh = %d ch = %d\n", nn, inst, base, primCh, secCh, ch) }
		    if base == "" || iq == "Q" || ch > 0 {
			if ch > 0 && primCh > 0 {
			    /********************************************************************
			     *  Encountered a pre-registered input, whose channel has been allocated
			     *   1) ignore 2nd and further pre-registered inputs per equivalence
			     *   2) move the equivalence set from the previous base to this input name
			     *   3) re-assign the new name and channel as base and primary channel in equivBase[]
			     *   4) rebase map altRecvCons entries already registered to new primary
			     *      channel before any more registrations take place
			     *   5) rebase []receiverCons entries already registered as secondary channel
			     *      entries in map altRecvCons because channels have already been acked
			     *   6) delete Autovivify entry for primary input already registered
			     *******************************************************************/
			    if dupFlag {				// 1)
				fmt.Fprintf(os.Stderr, "Warning %s: cannot have more than 1 pre-registered input %q on ch '%d' in %q - ignore\n", named, nn, ch, equiv)
				continue
			    }
			    dupFlag = true
			    equivalences[nn] = equivalences[base]	// 2) move equivalence set to new base
			    if *opt_t { fmt.Printf("--- nn = %q delete(equivalences, %q)\n", nn, base) }
			    delete(equivalences, base)			//    delete old base entry
			    for _, equivBp := range equivBase {
				if equivBp.primCh == primCh {		// 3) find old primary channel
				    equivBp.base   = nn			//    re-assign base
				    equivBp.primCh = ch			//    re-assign primary channel
				}
			    }
			    if _, ok := altRecvCons[primCh]; ok {	// 4) exists
				altRecvCons[ch] = altRecvCons[primCh]	// TODO make sure altRecvCons[ch] is empty
				delete(altRecvCons, primCh)
				altRecvNames[ch] = altRecvNames[primCh]
				delete(altRecvNames, primCh)
			    }
			    if len(receiverCons[primCh]) > 0 {		// 5)
				altRecvCons[ch] = append([]altRecv{ altRecv{ primCh, receiverCons[primCh] } }, altRecvCons[ch]...)	// unshift
				receiverCons[primCh] = []net.Conn{}
				altRecvNames[ch] = append([][]string{ receiverNames[primCh] }, altRecvNames[ch]...)	// unshift
				receiverNames[primCh] = []string{}
			    }
			    if boxName != "" {				// make sure AutoVivify is started
				autoBoxCh<- "-" + base			// 6) delete AutoVivify entry
				if *opt_t { fmt.Printf("delete Autovivify entry 'AutoVivify[%s]'\n", base) }
			    }
			} else {
			    ch = 0					// no primary channel (or no channel)
			}
			base = nn
			equivSet = append([][2]string{{nn, inst}}, equivSet...)	// unshift output or first input name
		    } else {
			equivSet = append(equivSet, [2]string{nn, inst})	// push later inputs in equivalence set
		    }
		    if *opt_t { fmt.Printf("--- equivSet = %v\n", equivSet) }
		}
	    }
	}
	/********************************************************************
	 *  Now a base IEC name has been determined distribute equivalences
	 *  in equivalences[] - also update equivBase[].
	 *******************************************************************/
	if *opt_t { fmt.Printf("--- base = %q\n", base) }
	for _, ref := range equivSet {
	    if *opt_t { fmt.Printf("--- ref = %v\n", ref) }
	    var refEq *eq
	    ref1 := map[string]string{}
	    nn   := ref[0]
	    inst := ref[1]
	    if *opt_t { fmt.Printf("--- nn = %q inst = %q primCh = %d ch = %d\n", nn, inst, primCh, ch) }
	    if ch > 0 {
		primCh = ch
		secCh  = ch
	    } else {
		for _, refEq0 := range equivalences[base] {
		    if *opt_t { fmt.Printf("--- refEq0 = %v\n", refEq0) }
		    if refEq0 != nil {
			secCh = refEq0.channel
			ref0 := *refEq0.equivP
			if _, ok := ref0[inst]; ! ok {	// not exists
			    ref0[inst] = nn		// extend equivalence group with different instances
			    allocateChannel(nn, secCh)	// allocate same channel for next IEC instance
			    equivBase[nn] = &eqB{ base, primCh, 0 }
			    if *opt_t { fmt.Printf("--- ext secCh = %d nn = %q\n", secCh, nn) }
			    goto EquivStored
			}
		    }
		}
		secCh = allocateChannel(nn, 0)		// allocate new channel for first IEC name in group
	    }
	    ref1[inst] = nn				// new equivalence with instance used in previous group
	    if primCh == 0 {
		primCh = secCh
	    }
	    if *opt_t { fmt.Printf("--- new secCh = %d nn = %q\n", secCh, nn) }
	    refEq = &eq{ secCh, &ref1 }			// secondary channel for equivalence group and first IEC
	    if ch > 0 {					// new equivalence group for pre-registered input
		equivalences[base] = append([]*eq{refEq}, equivalences[base]...)	// unshift
		ch = 0
	    } else {					// new equivalence group on a different channel
		equivalences[base] = append(equivalences[base], refEq)			// push
	    }
	    equivBase[nn] = &eqB{ base, primCh, 0 }
	  EquivStored:
	    if *opt_t { fmt.Printf("--- equivalences = %v\n", equivalences) }
	    if *opt_t { fmt.Printf("--- equivBase    = %v\n", equivBase) }
	}
    }
    if errFlag { shutdownServer(9, "Hard error in equivalence specifications") }
} // allocateEquivalences

/********************************************************************
 *
 *	Auxiliary routine for printTables()
 *	Append registered Receivers
 *
 *******************************************************************/

func printTail(aref []net.Conn, nref []string) {
    var rName string
    var rCons []string
    for i, rcon := range aref {
	if rcon != nil {
	    if rName = clientNames[rcon]; rName == "" {
		rName = "<undef>"
	    }
	    rCons = append(rCons, rName, nref[i])
	}
    }
    if len(rCons) == 0 {
	rName = "<no receivers>"
    } else {
	rName = strings.Join(rCons, "\t")
    }
    fmt.Printf("%s\n", rName)
} // printTail

/********************************************************************
 *
 *	Print tables of clientNames
 *	Print table of equivalences if any are defined
 *	Print table of registered Senders and Receivers
 *
 *******************************************************************/

func printTables() {
    muEquiv.Lock()
    defer muEquiv.Unlock()
    fmt.Printf("%s: Table of clientNames and registered Senders and Receivers\n", named)
    names := make([]string, 0, len(clientNames))
    for _, name := range clientNames {
	names = append(names, name)
    }
    sort.Strings(names)
    for _, name := range names {
	fmt.Printf("%s\n", name)
    }
    if len(equivalences) > 0 {
	fmt.Printf("\nequivalences = {\n")
	bases := make([]string, 0, len(equivalences))
	for base, _ := range equivalences {
	    bases = append(bases, base)
	}
	sort.Sort(iCboxOrder(bases))
	for _, base := range bases {
	    fmt.Printf("%-6s => [\n", base)
	    for _, refEq := range equivalences[base] {
		if refEq != nil {
		    secCh := refEq.channel
		    fmt.Printf("    [ %d, {", secCh)
		    ref1 := *refEq.equivP
		    instances := make([]string, 0, len(ref1))
		    for inst, _ := range ref1 {
			instances = append(instances, inst)
		    }
		    sort.Strings(instances)
		    for _, inst := range instances {
			fmt.Printf(" %q => %s,", inst, ref1[inst])
		    }
		    fmt.Printf(" } ],\n")
		}
	    }
	    fmt.Printf("],\n")
	}
	fmt.Printf("}\n")
    }
    if len(equivBase) > 0 {
	fmt.Printf("\nequivBase	channels\nname	base	primCh	secCh	ch\n")
	names := make([]string, 0, len(equivBase))
	for name, _ := range equivBase {
	    names = append(names, name)
	}
	sort.Sort(iCboxOrder(names))
	for _, name := range names {
	    fmt.Printf("%s	%s	%d	%d	%d\n",
		name, equivBase[name].base, equivBase[name].primCh, equivBase[name].secCh, channels[name])
	}
    }
    fmt.Printf("\nch  Sender          name            value       Recv[0]	name[0]	Recv[1]	name[1]	...\n")
    for channel := 0; channel <= maxChannel; channel++ {
	var chName string
	aref := receiverCons[channel]
	con := senderCon[channel]
	if con != nil || len(aref) > 0 {
	    if chName = clientNames[con]; chName == "" {
		chName = "<no sender>"
	    }
	    fmt.Printf("%-3d %-15s %-15s %-11s ", channel, chName, senderName[channel], senderValue[channel])
	    nref := receiverNames[channel]
	    printTail(aref, nref)
	}
	if _, ok := altRecvCons[channel]; ok {		// exists
	    for i, aref := range altRecvCons[channel] {
		fmt.Printf("    %-43d ", aref.altsecCh)	// secondary channel
		nref := altRecvNames[channel][i]
		printTail(aref.altCons, nref)
	    }
	}
    }
} // printTables

/********************************************************************
 *
 *	Write messages for different connections
 *
 *******************************************************************/

func writeReceiverData(receiverData map[net.Conn][]string, goId int) {
    for con, msgSlice := range receiverData {
	wbuf := append([]byte{ 0, 0, 0, 0 }, strings.Join(msgSlice, ",")...)	// build Perl type message
	if *opt_t { fmt.Printf("%02d: (%d) %s > %q\n", goId, len(wbuf)-4, wbuf[4:], clientNames[con]) }
	err := tcpcomm.Write(con, &wbuf)
	if err != nil {
	    fmt.Fprintf(os.Stderr, "%02d: msg = %q client = %q\n", goId, wbuf[4:], clientNames[con])
	    shutdownServer(4, fmt.Sprint(err))
	}
    }
} // writeReceiverData

/********************************************************************
 *
 *	Disconnect a client
 *
 *	nil 'senderCon[channel]' for all channels at this 'conn'
 *	splice out all 'receiverCons[channel]' entries with this 'conn'
 *	splice out all 'receiverNames[channel]' entries with this 'conn'
 *	as well as similar elements in 'altRecvCons' and 'altRecvNames'
 *
 *******************************************************************/

func disconnect (name string, goId int) {
    if name != "" {
	if ! *opt_q { fmt.Printf("%02d: %s: disconnecting now\n", goId, name) }
	receiverData := map[net.Conn][]string{}	// clear all keys and entries
	for channel := 0; channel <= maxChannel; channel++ {
	    rflag := false
	    con := senderCon[channel]
	    nam := clientNames[con]
	    if nam == name {
		senderCon[channel] = nil	// no need to splice out only array entry
		if *opt_r && senderValue[channel] != "0" {
		    rflag = true
		    senderValue[channel] = "0"	// reset sender value
		}
		if *opt_t { fmt.Printf("%02d: %s: S %s	%d nil\n", goId, name, senderName[channel], channel) }
	    }
	    if rflag {
		// scan once, resetting receivers, before splicing out any array elements
		for _, con := range receiverCons[channel] {
		    if clientNames[con] != name {
			msg := fmt.Sprintf("%d:0", channel)
			receiverData[con] = append(receiverData[con], msg) // reset message to receiver channels
		    }
		}
		if _, ok := altRecvCons[channel]; ok {			// exists
		    for _, aref := range altRecvCons[channel] {
			ch := aref.altsecCh				// alternate channel
			for _, con := range aref.altCons {
			    msg := fmt.Sprintf("%d:0", ch)
			    receiverData[con] = append(receiverData[con], msg)	// reset message to alternate channels
			}
		    }
		}
	    }
	    /********************************************************************
	     *  Do not use 'range' when splicing out entries
	     *******************************************************************/
	    aref := &receiverCons[channel]
	    if len(*aref) != 0 {
		for i := 0; i < len(*aref); i++ {
		    con := (*aref)[i]
		    if clientNames[con] == name {
			nref := &receiverNames[channel]
			nam := (*nref)[i]
			if equivBase[nam] != nil {
			    equivBase[nam].secCh = 0		// clear secondary channel to flag no registration
			}
			if *opt_t { fmt.Printf("%02d: %s: R %s	%d spliced out\n", goId, name, nam, channel) }
			// splice out connection and name
			*aref = append((*aref)[:i], (*aref)[i+1:]...)
			*nref = append((*nref)[:i], (*nref)[i+1:]...)
			/********************************************************************
			 *  Checked that there are no multiple Receiver entries with the same
			 *  'Conn' when registering. Nevertheless scan to the end to keep 'i'
			 *  correct for 'altRecvCons' and 'altRecvNames'
			 *******************************************************************/
			i--				// adjust index of next entry and name to be spliced out
		    }
		}
	    }
	    if _, ok := altRecvCons[channel]; ok {		// exists
		for j := 0; j < len(altRecvCons[channel]); j++ {
		    aref  := &altRecvCons[channel][j]
		    secCh := aref.altsecCh
		    cref  := &aref.altCons
		    for i := 0; i < len(*cref); i++ {
			con := (*cref)[i]
			if clientNames[con] == name {
			    nref := &altRecvNames[channel][j]
			    nam := (*nref)[i]
			    if _, ok := equivBase[nam]; ok {	// exists
				equivBase[nam].secCh = 0	// clear secondary channel to flag no registration
			    }
			    if *opt_t { fmt.Printf("%02d: %s: R %s	%d spliced out\n", goId, name, nam, secCh) }
			    *cref = append((*cref)[:i], (*cref)[i+1:]...)	// skip extra channel entry
			    *nref = append((*nref)[:i], (*nref)[i+1:]...)	// splice out name whose index starts at 0
			    i--				// adjust index of next entry and name to be spliced out
			}
		    }
		    if len(*cref) == 0 {		// group has no connections left, only alternate channel
			if *opt_t { fmt.Printf("%02d: altRecvCons[%d] group %d spliced out\n", goId, channel, j) }
			altRecvCons[channel] =  append(altRecvCons[channel][:j],
			    altRecvCons[channel][j+1:]...)		// whole group disconnected
			altRecvNames[channel] = append(altRecvNames[channel][:j],
			    altRecvNames[channel][j+1:]...)
			j--				// adjust index of next group to be spliced out
		    }
		}
		if len(altRecvCons[channel]) == 0 {
		    delete(altRecvCons, channel)	// delete whole alternate channel set
		    delete(altRecvNames, channel)
		}
	    }
	}
	writeReceiverData(receiverData, goId)		// write data collected for different receivers
	delete(clientNames, clientCons[name])
	delete(clientCons, name)
	if boxName != "" {				// make sure AutoVivify is started
	    autoBoxCh<- "-" + name			// delete AutoVivify entry (ignored if already done)
	    if *opt_t { fmt.Printf("delete Autovivify entry 'AutoVivify[%s]'\n", name) }
	}
	if ! *opt_q { fmt.Printf("%02d: %s: disconnected\n", goId, name) }
	numberOfConnections--
	if numberOfConnections <= 0 {
	    /********************************************************************
	     *  This was last connection - clear all arrays and hashes
	     *  this prevents channel numbers getting higher and higher
	     *******************************************************************/
	    clientCons    = map[string]net.Conn{}
	    channels      = map[string]int{}
	    clientNames   = map[net.Conn]string{}
	    senderCon     = make([]net.Conn, 16)
	    senderName    = make([]string, 16)
	    senderValue   = make([]string, 16)
	    receiverCons  = make([][]net.Conn, 16)
	    receiverNames = make([][]string, 16)
	    altRecvCons   = map[int][]altRecv{}
	    altRecvNames  = map[int][][]string{}
	    maxChannel = 0
	    goCount = 0
	    /********************************************************************
	     *  Re-constitute channels of equivalence arguments.
	     *  All secondary channels in equivBase[] have been set to 0
	     *  so use equivalences[] to get secondary channels instead.
	     *******************************************************************/
	    if len(equivalences) > 0 {
		bases := make([]string, 0, len(equivalences))
		for base, _ := range equivalences {
		    bases = append(bases, base)
		}
		sort.Sort(iCboxOrder(bases))
		for _, base := range bases {
		    for _, refEq := range equivalences[base] {
			if refEq != nil {
			    secCh := refEq.channel
			    ref1 := *refEq.equivP
			    instances := make([]string, 0, len(ref1))
			    for inst, _ := range ref1 {
				instances = append(instances, inst)
			    }
			    sort.Strings(instances)
			    for _, inst := range instances {
				nn := ref1[inst]		// extend equivalence group with different instances
				allocateChannel(nn, secCh)	// allocate same channel for next IEC instance
			    }
			}
		    }
		}
	    }
	    if ! *opt_q { fmt.Printf("%02d: last client has disconnected\n", goId) }
	}

    } else {
	warnD(fmt.Sprintf("the client %q is not currently registered - system error", name))
    }
} // disconnect

/********************************************************************
 *
 *	Warning sent to Debug (iClive) registered on channel 0
 *
 *******************************************************************/

func warnD(warnMsg string) {
    fmt.Fprintf(os.Stderr, "%s: WARNING: %s\n", named, warnMsg)
    receiverData := map[net.Conn][]string{}	// clear all keys and entries
    if aref := receiverCons[0]; aref != nil {	// channel 0 for error messages
	for _, rconn := range aref {
	    if *opt_t { fmt.Fprintf(os.Stderr, "%s: sent warning => %s\n", named, clientNames[rconn]) }
	    receiverData[rconn] = append(receiverData[rconn], "0:1;" + warnMsg)	//  message to channel 0
	    writeReceiverData(receiverData, 0)	// write data collected for different receivers
	}
    }
} // warnD

/********************************************************************
 *
 *	Handle all input communication on a TCP connection in this
 *	dedicated goroutine.
 *	Output communication is usually to other TCP connections except
 *	the ack string for registration messages.
 *
 *******************************************************************/

func handleConnection(conn net.Conn) {
    muConn.Lock()
    defer muConn.Unlock()			// unlock when client disconnects
    goCount++
    goId := goCount				// identify this goroutine
    if ! *opt_q { fmt.Printf("%02d: Connection %s at %s\n", goId, conn.RemoteAddr().String(), named) }
    defer conn.Close()
    var regFlag bool				// initially false
    /********************************************************************
     *  Receive messages from clients
     *
     *  connection 'conn' identifies the sender
     *  message 'rbuf' has the following characteristics:
     *  . zero length		client has signalled shutdown
     *  . comma separated list	break up into individual items 'msg'
     *    each item 'msg' has the following characteristics:
     *    . matches /^\d+:.+$/	  split into channel and value, send
     *				  value to all receivers for the channel
     *    . matches /^[NnSsRrZ]/  registration item
     *    . matches /^E.*$/	  equivalance message from an app
     *    . matches /^[XT]/	  special actions
     *    . equals ''		  ignore empty message before leading comma 
     *    . else eror
     *******************************************************************/
    for {
	receiverData := map[net.Conn][]string{} // hashed by con
					      // { con1-> "ch1:val1,ch2:val2", con2-> "ch3:val3,ch4:val4" ... }
					      // sent at end of Read - must be local to this goroutine
	wbuf := make([]byte, 4, tcpcomm.REPLY+4)	// Perl type length+message buffer 
	iniString := ""
	if *opt_m {
	    t0 = time.Now()
	}
	muConn.Unlock()
	rbuf, err := tcpcomm.Read(conn)		// Read message from client
	muConn.Lock()
	if err != nil {
	    if err == io.EOF {
		break
	    }
	    fmt.Fprintln(os.Stderr, "Error in tcpcomm.Read!", err)
	    break
	}
	sender := clientNames[conn]			// set when name registered for this 'conn'
	if *opt_m {
	    t1 := time.Now()
	    diff := t1.Sub(t0)
	    if *opt_t {
		fmt.Printf("S %f: ", diff.Seconds())
	    } else {
		fmt.Printf("S %f\n", diff.Seconds())
	    }
	}
	if *opt_t { fmt.Printf("%02d: (%d) %s < %q\n", goId, len(rbuf), rbuf, sender) }
	if len(rbuf) == 0 {
	    /********************************************************************
	     *  Client 'conn' has signalled shutdown - disconnect client
	     *  clear all entries for 'conn' in 'senderCon' and 'receiverCons'
	     *******************************************************************/
	    disconnect(sender, goId)
	    break
	}
	for _, msg := range strings.Split(string(rbuf), ",") {
	    if m := reData.FindStringSubmatch(msg); len(m) != 0 {	// "^([0-9]+):(.*)$"
		if ! regFlag {
		    warnD(fmt.Sprintf("data %q during registration - app spec error", msg))
		    continue
		}
		/********************************************************************
		 *  Allow message to be received from any connection, not just
		 *  from con = senderCon[channel]
		 *  This allows apps to re-transmit received messages for debugging
		 *******************************************************************/
		channel, _ := strconv.Atoi(m[1])
		senderValue[channel] = m[2]		// latest value in case lost connection
		sendFlag := false
		/********************************************************************
		 *  Central function of server done at maximum speed.
		 *  Transmit message from a sender for a particular channel
		 *  to each receiver on that channel - most common case.
		 *******************************************************************/
		for _, rconn := range receiverCons[channel] {
		    receiverData[rconn] = append(receiverData[rconn], msg) //  messages to receiver channels
		    sendFlag = true
		}
		/********************************************************************
		 *  Transmit message to each receiver on equivalenced alternate
		 *  channels - this is very rare and only happens if several inputs
		 *  on the same instance of an app are equivalenced. Only one statement
		 *  is added to the execution of the common case.  Map altRecvCons
		 *  is hashed by channel ... receive connections on alternate channel
		 *  [ channel1, [ conn3, conn4 ... ],  channel2, [ conn5, conn6 ... ] ... ]
		 *  NOTE: ReceiverCons[channel] may be empty and AltRecvCons[channel]
		 *  may have entries for the case where an output from a connection is
		 *  equivalenced with an input of the same connection.
		 *******************************************************************/
		if _, ok := altRecvCons[channel]; ok {			// exists
		    for _, aref := range altRecvCons[channel] {
			ch := aref.altsecCh				// alternate channel
			for _, rconn := range aref.altCons {
			    msg = fmt.Sprintf("%d:%s", ch, m[2])	// message value
			    receiverData[rconn] = append(receiverData[rconn], msg) //  message to alternate channels
			    sendFlag = true
			}
		    }
		}
		if ! sendFlag {
		    if *opt_t { fmt.Printf("%02d: %s %s => dummy\n", goId, sender, msg) }
		}
	    } else if m := reNname.FindStringSubmatch(msg); len(m) != 0 {	// "^N(.+)$"
		name := m[1]
		if name[0] == '-' {			// -bar is name of iClive session for program bar
		    for i, con := range receiverCons[0] {
			if con == conn {
			    receiverNames[0][i] = name	// name has changed in old warning registration
			    goto ALREADY_REGISTRERD
			}
		    }
		    receiverCons[0]  = append(receiverCons[0], conn)	// new warning registration
		    receiverNames[0] = append(receiverNames[0], name)
		} else if con  := clientCons[name]; con != nil {
		    /********************************************************************
		     *  a client with the same name was previously registered
		     *******************************************************************/
		    if ! *opt_k {
			/********************************************************************
			 *  kill the new client and report error
			 *******************************************************************/
			warnD(fmt.Sprintf("trying to register client %q twice - aborted", name))
			goCount--
			return				// new client was not registered yet
		    }
		    /********************************************************************
		     *  -k option: unregister and disconnect previous instance
		     *  proceed with new registration
		     *******************************************************************/
		    disconnect(name, goId)	// kills old client process
		    channel   := channels["D"+name]
		    if channel != 0 {
			iniString = fmt.Sprintf("%d:1", channel)	// get S.T. from new client
		    }
		}
	      ALREADY_REGISTRERD:
		clientCons[name]  = conn		// client by this name exists now
		clientNames[conn] = name		// name may change for iClive connections
		sender = name
		numberOfConnections++
		if ! *opt_q { fmt.Printf("%02d: %s: client registering now\n", goId, sender) }
	    } else if m = reSRiec.FindStringSubmatch(msg); len(m) != 0 {	// `^([SR])([\\\/\w]+(-\d*)?)(\((\d+)\))?$`
		/********************************************************************
		 *  regex is permissive allowing any number of instance digits
		 *  clients have to stay in specification 1 to 3 digits
		 *  if not, server still functions correctly
		 *******************************************************************/
		direction := m[1]
		name      := m[2]
		bits      := m[5]
		muEquiv.Lock()				// allocateChannel() also called in allocateEquivalences()
		channel := allocateChannel(name, 0)
		muEquiv.Unlock()
		var base string
		var primCh, secCh int
		if eqBase, ok := equivBase[name]; ok {
		    base   = eqBase.base
		    primCh = eqBase.primCh
		    secCh  = eqBase.secCh
		    if secCh == 0 {
			eqBase.secCh = channel		// assign secondary channel now to flag registration
		    } else if secCh != channel {
			warnD(fmt.Sprintf("%q equivalenced channel %d not allocated channel %d ???", name, secCh, channel))
			disconnect(sender, goId)
			break
		    }					// secCh === channel
		} else {
		    base, primCh, secCh = name, channel, 0
		}
		ack := channel				// used to build acknowledge string
		if *opt_t { fmt.Printf("%02d: register %s %q base = %q primCh = %d secCh = %d channel = %d ack = %d\n", goId, direction, name, base, primCh, secCh, channel, ack) }
		if direction == "S" {
		    if primCh != channel {
			/********************************************************************
			 *  A client is trying to register a sender on a channel for
			 *  an alternate equivalence group TODO find a test case
			 *******************************************************************/
			if *opt_t { fmt.Printf("%02d: register S name = %q base = %q primCh = %d secCh = %d channel = %d\n", goId, name, base, primCh, secCh, channel) }
			if equivBase[base].secCh != 0 {	// secondary channel of base name already registered
			    ack = -channel
			    warnD(fmt.Sprintf("%q trying to register S %q for alternative channel %d/%d - aborted", sender, name, primCh, channel))
			    regFlag = true		// client registeration terminated
			    disconnect(sender, goId)
			    break
			}
			/********************************************************************
			 *  Locate secondary group in map equivalences and move it
			 *  to the primary position - adjust map equivBase
			 *******************************************************************/
			var i int
			var aref *eq
			for i, aref = range equivalences[base] {
			    if aref.channel == channel {
				goto Found
			    }
			}
			goto NotFound
		      Found:
			equivalences[base] =  append(equivalences[base][:i],  equivalences[base][i+1:]...)
			equivalences[base] =  append([]*eq{ aref },  equivalences[base][:]...)
			if *opt_t { fmt.Printf("%02d: S delete equivalences[base = %s]\n", goId, base) }
			delete(equivalences, base)			// delete old base entry
			for _, equivBp := range equivBase {
			    if equivBp.primCh == primCh {		// find old primary channel
				equivBp.base   = name			//    re-assign base
				equivBp.primCh = channel		//    re-assign primary channel
			    }
			}
		      NotFound:
		    }
		    if senderCon[channel] != nil {
			/********************************************************************
			 *  A client is trying to register a sender twice on the same
			 *  channel
			 *******************************************************************/
			ack = -channel
			warnD(fmt.Sprintf("%q trying to register S %q for channel %d twice - aborted", sender, name, channel))
			// has registered name and may already have registered some receivers
			regFlag = true			// client registeration terminated
			disconnect(sender, goId)
			break
		    }
		    /********************************************************************
		     *  Register a new Sender
		     *  sends all registered receivers on the channel a 0 value
		     *  (this also works for iClive - NOP)
		     *******************************************************************/
		    senderCon[channel]   = conn
		    senderName[channel]  = name
		    senderValue[channel] = "0"		// default initial value
		    flag := false
		    for _, rconn := range receiverCons[channel] {
			receiverData[rconn] = append(receiverData[rconn], fmt.Sprintf("%d:0", channel))	//  reset receiver
			flag = true			// sender has at least one receiver
		    }
		    if reQXBWL.FindString(name) != "" {	// `^Q[XBWL]\d+`
			if ! flag {				// no iCbox for this sender if it has receivers
			    iCboxParam := name
			    if bits != "" {
				iCboxParam += "," + bits
			    }
			    if boxName == "" {
				boxName = sender + "-IO"
				autoBoxCh<- boxName	// start AutoVivify iCbox
			    }
			    autoBoxCh<- iCboxParam
			    if *opt_t { fmt.Printf("%02d: Autovivify %s: sender %s\n", goId, boxName, iCboxParam) }
			}
		    } else if boxName != "" && reIXBWL.FindString(name) != "" {	// `^I[XBWL]\d+`
			autoBoxCh<- "-" + name	// delete AutoVivify entry (ignored if already done)
			if *opt_t { fmt.Printf("%02d: Delete autovivify %s: sender %s\n", goId, boxName, name) }
		    }
		    if *opt_t { fmt.Printf("%02d: %q S %q	ack = %d\n", goId, sender, name, ack) }
		} else if direction == "R" {
		    /********************************************************************
		     *  ignore duplicate Receiver entries for this conn
		     *  TODO remove - this is no longer likely to happen with alternate groups
		     *******************************************************************/
		    if primCh == channel {
			for _, rconn := range receiverCons[channel] {
			    if rconn == conn {
				// happens if receiver equivalences in the same client
				ack = -channel		// not an error in iCbox
				warnD(fmt.Sprintf("register R %q for channel %d at %q twice", name, channel, sender))
			    }
			}
		    } else if _, ok := altRecvCons[primCh]; ok {		// exists
			for _, aref := range altRecvCons[channel] {
			    ch := aref.altsecCh					// alternate channel
			    for _, rconn := range aref.altCons {
				if rconn == conn {
				    // happens if receiver equivalences in the same client
				    ack = -ch		// not an error in iCbox
				    warnD(fmt.Sprintf("register R %q for channel %d/%d at %q twice", name, primCh, ch, sender))
				}
			    }
			}
		    }
		    ini:= senderValue[channel]					// current value
		    if ack > 0 {
			if primCh == channel {
			    receiverCons[channel] = append(receiverCons[channel], conn)	// simple R registration
			    receiverNames[channel] = append(receiverNames[channel], name)
			} else {
			    if _, ok := altRecvCons[primCh]; ok {		// exists
				for i, aref := range altRecvCons[primCh] {
				    secCh = aref.altsecCh			// alternate channel
				    if *opt_t { fmt.Printf("%02d: scan   alternate R registration name = %q primCh = %d secCh = %d channel = %d\n    %v\n    %v\n",
					goId, name, primCh, secCh, channel, altRecvCons[primCh], altRecvNames[primCh]) }
				    if secCh == channel {
					// extend alternate R registration group
					altRecvCons[primCh][i].altCons = append(altRecvCons[primCh][i].altCons, conn)
					// extend alternate R names
					altRecvNames[primCh][i] = append(altRecvNames[primCh][i], name)
					if *opt_t { fmt.Printf("%02d: extend alternate R registration name = %q primCh = %d secCh = %d channel = %d\n    %v\n    %v\n",
					    goId, name, primCh, secCh, channel, altRecvCons[primCh], altRecvNames[primCh]) }
					goto AltStored
				    }
				}
			    }
			    // new alternate R registration in group on a different channel
			    altRecvCons[primCh] = append(altRecvCons[primCh], altRecv{ channel, []net.Conn{ conn } })
			    altRecvNames[primCh] = append(altRecvNames[primCh], []string{ name })
			    if *opt_t { fmt.Printf("%02d: new    alternate R registration name = %q primCh = %d channel = %d\n    %v\n    %v\n",
				goId, name, primCh, channel, altRecvCons[primCh], altRecvNames[primCh]) }
			  AltStored:
			}
			if senderCon[primCh] != nil {	// is sender registered on primary channel ?
			    if ini != "0" {					// yes - current value
				/********************************************************************
				 *  sender is registered and has either initial value 0 or other
				 *  current value - send new receiver value different to 0
				 *  if sender is already registered
				 *******************************************************************/
				iniSt := fmt.Sprintf("%d:%s", channel, ini)	// message to all secondary receivers
				if iniString == "" {
				    iniString = iniSt
				} else {
				    iniString += "," + iniSt
				}
			    }
			} else if reIXBWL.FindString(base) != "" {	// `^I[XBWL]\d+`
			    iCboxParam := base
			    if bits != "" {
				iCboxParam += "," + bits
			    }
			    if boxName == "" {
				boxName = sender + "-IO"
				autoBoxCh<- boxName		// start AutoVivify iCbox
			    }
			    autoBoxCh<- iCboxParam
			    if *opt_t { fmt.Printf("%02d: Autovivify %s: receiver %s\n", goId, boxName, iCboxParam) }
			}
			if boxName != "" && reQXBWL.FindString(base) != "" {	// `^Q[XBWL]\d+`
			    autoBoxCh<- "-" + base	// delete AutoVivify entry (ignored if already done)
			    if *opt_t { fmt.Printf("%02d: Delete autovivify %s: receiver %s\n", goId, boxName, base) }
			}
		    }
		    if *opt_t { fmt.Printf("%02d: %s R %s	ack = %d	ini = %q\n", goId, sender, name, ack, ini) }
		}
		if len(wbuf) == 4 {
		    wbuf = append(wbuf, fmt.Sprintf("%d", ack)...)
		} else {
		    wbuf = append(wbuf, fmt.Sprintf(",%d", ack)...)
		}
	    } else if msg == "Z" {
		if len(wbuf) <= 4 {
		    wbuf = append(wbuf, "-0"...)	// send a null acknowledgment
		}
		regFlag = true			// client has registered
	    } else if m := renName.FindStringSubmatch(msg); len(m) != 0 {	// "^n(.+)$"
		if name := m[1]; name != "" {
		    delete(clientNames, conn)
		    delete(clientCons, name)
		    numberOfConnections--
		    if ! *opt_q { fmt.Printf("%02d: %s: client unregistering now\n", goId, name) }
		} else {
		    /********************************************************************
		     *  no client with this name was previously registered
		     *******************************************************************/
		    warnD(fmt.Sprintf("no client with n %q was previously registered", name))
		}
		regFlag = false			// client has deregistered (iClive only)
	    } else if m = resrIec.FindStringSubmatch(msg); len(m) != 0 {	// `^([sr])([\\\/\w]+(-\d*)?)(\((\d+)\))?$`
		direction := m[1]
		name      := m[2]		// ignore bits in m[5]
		channel   := channels[name]
		if channel == 0 {
		    warnD(fmt.Sprintf("%q trying to unregister %q not previously registered - aborted", sender, name))
		}
		if direction == "s" {
		    var con net.Conn
		    if con = senderCon[channel]; con == nil {
			warnD(fmt.Sprintf("%q trying to unregister s '%s; not registered as sender - aborted", sender, name))
		    } else
		    /********************************************************************
		     *  Unregister a previously registered Sender
		     *******************************************************************/
		    if con == conn {
			senderCon[channel] = nil	// no need to splice out only array entry
			if *opt_t { fmt.Printf("%02d: unregister sender   %s\n", goId, name) }
		    } else {
			warnD(fmt.Sprintf("%q trying to unregister s '%s; not registered as sender - aborted", sender, name))
		    }
		    // leave name
		    senderValue[channel] = "0"		// reset initial value
		} else if direction == "r" {
		    /********************************************************************
		     *  check that there are no Receiver entries with this conn
		     *******************************************************************/
		    for i, rconn := range receiverCons[channel] {
			if rconn == conn {
			    if *opt_t { fmt.Printf("%02d: unregister receiver %s\n", goId, name) }
			    // splice out connection and name
			    receiverCons[channel]  = append(receiverCons[channel][:i],  receiverCons[channel][i+1:]...)
			    receiverNames[channel] = append(receiverNames[channel][:i], receiverNames[channel][i+1:]...)
			    /********************************************************************
			     *  checked that there are no multiple Receiver entries
			     *  with the same 'conn' when registering
			     *  only used for iClive which only registers one receiver
			     *  but use full algorithm just in case of future clients
			     *******************************************************************/
			    break
			}
		    }
		}
	    } else if m := reEquiv.FindStringSubmatch(msg); len(m) != 0 {	// "^E(.+)$"
		if *opt_t { fmt.Printf("%02d: Allocate equivs %q\n", goId, m[1]) }
		allocateEquivalences(m[1])
	    } else if msg == "T" {
		if *opt_t { fmt.Printf("%02d: Print Tables\n", goId) }
		printTables()
	    } else if m := reXname.FindStringSubmatch(msg); len(m) != 0 {	// "^X(.*)$"
		if len(m[1]) == 0 {
		    shutdownServer(0, "Stop from parent")
		} else {
		    shutdownServer(0, fmt.Sprintf("Stop from %q - stop %s", m[1], named))
		}
	    } else if msg != "" {
		fmt.Printf("%02d: Warning strange message %s in %s ???\n", goId, msg, rbuf)	// ignore null msg
	    }
	}
	if len(wbuf) > 4 {
	    /********************************************************************
	     *  Send partial or final registration acknowledge string
	     *  Must be sent before any initialization data generated during
	     *  registration
	     *******************************************************************/
	    if *opt_t { fmt.Printf("%02d: ACK: %s => %s\n", goId, wbuf[4:], sender) }
	    err = tcpcomm.Write(conn, &wbuf)	// registration acknowledgement
	    if err != nil {
		shutdownServer(5, fmt.Sprint(err))
	    }
	    if regFlag {			// defer ini and autoVivBox until registration terminated
		if iniString != "" {
		    // must be independent xmission, because different rcv call in client
		    if *opt_t { fmt.Printf("%02d: INI: %s => %s\n", goId, iniString, sender) }	// registration initialization
		    wbuf = []byte{ 0, 0, 0, 0 }			// clear wbuf for this iteration
		    wbuf = append(wbuf, []byte(iniString)...)	// build Perl type message
		    err = tcpcomm.Write(conn, &wbuf)
		    if err != nil {
			shutdownServer(6, fmt.Sprint(err))
		    }
		}
	    }
	} else if regFlag {
	    writeReceiverData(receiverData, goId)		// write data collected for different receivers
	}
    }
} // handleConnection

/********************************************************************
 *
 *	AutoVivify one or more iCbox's  with option -a or -A 'iCbox <flags>'
 *
 *  sort order is instance then IEC names in the following order
 *  IX IB IW IL IH QX QB QW QL QH .. finally number
 *   0  1  2  .. 10 11 12 .. 9999 .. correct for numerical compare
 *******************************************************************/
var sortOrder = map[string]int {	// determines in which order iCboxes ar autovivified
    "IX": 0, "QX": 5,
    "IB": 1, "QB": 6,
    "IW": 2, "QW": 7,
    "IL": 3, "QL": 8,
    "IH": 4, "QH": 9,
}

type iCboxOrder []string
func (p iCboxOrder) Len() int { return len(p) }
func (p iCboxOrder) Less(i, j int) bool {
    var ci, cj int
    mi := make([]string, 5)
    mj := make([]string, 5)
    // "^([A-Z]+)(\d+)(-(\d+))?" to parse "IX0", "IB1-0", "QH99-999" etc 
    if mi = reSort.FindStringSubmatch(p[i]); len(mi) != 5 { panic("illformed IEC string to sort") }
    if mj = reSort.FindStringSubmatch(p[j]); len(mj) != 5 { panic("illformed IEC string to sort") }
    if mi[4] == "" { ci = -1 } else { ci, _ = strconv.Atoi(mi[4]) }
    if mj[4] == "" { cj = -1 } else { cj, _ = strconv.Atoi(mj[4]) }
    if ci != cj {
	return ci < cj			// instance "" => -1, "-0" => 0, ... "-999" => 999
    }
    ci    = sortOrder[mi[1]]
    cj    = sortOrder[mj[1]]
    if ci != cj {
	return ci < cj			// IEC header "IX" => 0, "IB" => 1, ... "QH" => 9
    }
    ci, _ = strconv.Atoi(mi[2])
    cj, _ = strconv.Atoi(mj[2])
    return ci < cj			// IEC number "0" => 0, "1" => 1, ... "99" => 99
}
func (p iCboxOrder) Swap(i, j int) { p[i], p[j] = p[j], p[i] }

/********************************************************************
 *
 *	AutoVivify goroutine autoVivBox()
 *
 *******************************************************************/

var boxName  string			// global variable - reset after 1 iCbox is autovivified
var autoBoxCh  = make(chan string)	// AutoVivify iCbox go channel

func autoVivBox(avArg string) {
    iCboxCmd := strings.Split(avArg, " ")
    if *opt_t { fmt.Printf("=b= iCboxCmd   = %s\n", iCboxCmd[0]) }
    for {
	var ioName, autoMsg string
	var iCboxFlags = make([]string, 0, 20)
	iCboxMap  := map[string]string{}
	var iCboxArgs  = make([]string, 0, 16)
	if len(iCboxCmd) > 1 {
	    if iCboxCmd[1] == "-d" {
		flag1.Set("d", "true")		// *opt_d = true - may be set with *opt_A
	    }
	    iCboxFlags = append(iCboxFlags, iCboxCmd[1:]...)
	}
	if *opt_p != TCP_PORT { iCboxFlags = append(iCboxFlags, "-p", *opt_p) }
	if *opt_t { iCboxFlags = append(iCboxFlags, "-t") }
	if *opt_m { iCboxFlags = append(iCboxFlags, "-m") }
	if *opt_q { iCboxFlags = append(iCboxFlags, "-q") }
	boxName = ""				// global first name of an autovivified iCbox
	ioName   = <-autoBoxCh			// ioName triggers building of iCboxArgs
      loop:
	for {
	    select {
	    case autoMsg = <-autoBoxCh:		// waits for next message string
		if *opt_t { fmt.Println("=b= autoMsg    =", autoMsg) }
		if autoMsg[0] != '-' {
		    iec := strings.SplitN(autoMsg, ",", 2)
		    iCboxMap[iec[0]] = autoMsg	// remove ,bits in key to allow delete
		} else {
		    iec := autoMsg[1:]
		    delete(iCboxMap, iec)
		    if eqb, ok := equivBase[iec]; ok {
			if eqb.base != iec {
			    delete(iCboxMap, eqb.base)
			}
		    }
		}
	    case <-time.After(time.Second):	// delay for other apps to register
		break loop			// out of select and 2nd for
	    }
	}
	if len(iCboxMap) > 0 {
	    for _, am := range iCboxMap {
		iCboxArgs = append(iCboxArgs, am)
	    }
	    sort.Sort(iCboxOrder(iCboxArgs))
	    if *opt_a {
		for clientCons[ioName] != nil {
		    ioName += "x"
		}
		iCboxFlags = append(iCboxFlags, "-n", ioName)
		iCboxFlags = append(iCboxFlags, iCboxArgs...)
		if ! *opt_q { fmt.Printf("$ %s %v\n", iCboxCmd[0], strings.Join(iCboxFlags, " ")) }
		cmd := exec.Command(iCboxCmd[0], iCboxFlags...)
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		err := cmd.Start()			// execute iCbox in another process
		if err != nil {
		    shutdownServer(7, fmt.Sprint(err))
		}
		if *opt_d {
		    flag1.Set("a", "false")	// autovivify iCbox -d only once to stop recursion
		}
	    } else {
		warnD(fmt.Sprintf("%q I/Os are missing", strings.Join(iCboxArgs, " ")))
	    }
	}
    }
} // autoVivBox

/********************************************************************
 *
 *	goroutine keyboardIn()
 *
 *******************************************************************/

func keyboardIn() {
    in := bufio.NewReader(os.Stdin)
  kbLoop:
    for {
	r, _, err := in.ReadRune()
	if err == io.EOF {
	    break kbLoop
	}
	switch r {
	case 'T':
	    printTables()
	case 'm':
	    *opt_m = ! *opt_m			// toggle -m option
	case 't':
	    *opt_t = ! *opt_t			// toggle -t option
	case 'q':
	    break kbLoop
	}
    }
    shutdownServer(0, "Stop from keyboard")	// gracefull shutdown
} // keyboarIn

/********************************************************************
 *
 *	Quit program
 *
 *******************************************************************/

func shutdownServer(ret int, message string) {
    if ret != 0 {
	fmt.Fprintln(os.Stderr, message)
    } else if ! *opt_q {
	fmt.Println(message)
    }
    os.Exit(ret)
} // shutdownServer

/********************************************************************
 *
 *	Convert from Posix bundled flag interpretation to Go type flags.
 *	Extract -f flag and open next argument as an include file
 *	Extract first -R flag and save all further arguments in runArgs
 *
 *	Modify command line bundled flags to interpret -abc as -a -b -c,
 *	which is the way flag.Parse must be presented with Bool flags.
 *	flag.Parse interprets -abc as the flag named "abc".
 *	Mofify Value flags to interpret -xVal as -x Val. Can also have
 *	-abcx Val or -abcxVal, which are both converted to -a -b -c -x Val.
 *	Value flags could be identified by the flag initialisers above,
 *	but it is simpler to define a map of Value flag identifiers.
 *	Command line flags starting with -- are not touched, which is the
 *	Posix way of mixed flags starting with - or --.
 *	https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html
 *
 *	The -f <include file> argument opens the include file, which is
 *	trimmed of extraneous spaces and comments and produces another slice
 *	of command line arguments. A recursive call of convertFlags() moves
 *	these arguments into args and runargs, replacing the f<> argument.
 *
 *	The -R option starts a Bernstein chain. Each -R option in the chain
 *	starts a new iC app. Apps in a Bernstein chain are started sequentially
 *	to run their initialization code, at which point the app starts
 *	the next iC app in the Bernstein chain with its -R option. The
 *	current app keeps running in parallel with the app that started it.
 *	Finally all iC apps in a Bernstein chain run in parallel after their
 *	initialization codes have been run sequentially.
 *	The first iC app in the Bernstein chain must have the -z option, which
 *	suppresses stdin interrupts.
 *
 *	Modifying command line options and interpreting -f is stopped after the
 *	first -R option because any further options are interpreted by apps in the
 *	Bernstein chain and may not be set correctly in the commandLineFlags map.
 *
 *******************************************************************/

var args    = make([]string, 0, len(os.Args))
var runArgs = make([]string, 0, len(os.Args))
var argsP *[]string
var Rflag int
var commandLineFlags = map[string]int {
    "a":0,					// Bool flag
    "d":0,
    "t":0,
    "m":0,
    "q":0,
    "r":0,
    "k":0,
    "z":0,
    "h":0,
    "s":1,					// value flag
    "p":1,
    "e":1,
    "A":1,
    "f":1,
    "R":1,
}

func convertFlags(argp *[]string, offset int) {
    Fflag := false
    for _, arg := range (*argp)[offset:] {
	if len(arg) == 0 { continue}
	Sflag := true
	if arg != "-" && arg[0] == '-' && arg[1] != '-' && (Rflag == 0 || arg[1] == 'R') {
	    Sflag = false
	    for i, sf := range strings.Split(arg[1:], "") {
		clf, ok := 0, false
		if clf, ok = commandLineFlags[sf]; !ok {
		    arg = "-" + arg[i+1:]	// not our flag - nevertheless retain '-'
		    Sflag = true
		    break
		}
		if sf == "R" && Rflag == 0 {
		    Rflag = 1			// first -R not appended to args or runArgs
		    argsP = &runArgs		// now fill arguments for -R option in runArgs
		} else if sf == "f" && Rflag == 0 {
		    Fflag = true
		} else {
		    *argsP = append(*argsP, "-" + sf)
		}
		if clf == 1 && i+2 < len(arg) {
		    arg = arg[i+2:]		// value flag followed directly by value
		    Sflag = true
		    break
		}
	    }
	}
	if Sflag {
	    if ! Fflag {
		*argsP = append(*argsP, arg)	// append a value after a value flag
		if Rflag == 1 {
		    *argsP = append(*argsP, "-z")	// insert -z after first -R <app>
		    Rflag = 2
		}
	    } else {
		var incArgs []string		// process options in an include file
		reComment := regexp.MustCompile(`(^\s+)|(\s*(#.*)?$)`)
		reSpaces  := regexp.MustCompile(`\s*([=:,;.])\s*`)
		Fflag = false
		includeFile, err := os.Open(arg)
		if err != nil {
		    shutdownServer(8, fmt.Sprint(err))
		}
		defer includeFile.Close()
		scanner := bufio.NewScanner(includeFile)
		scanner.Split(bufio.ScanLines)
		for scanner.Scan() {
		    text := scanner.Text()
		    text = reComment.ReplaceAllLiteralString(text, "")	// strip leading space and trailing comment
		    text = reSpaces.ReplaceAllString(text, "$1")	// remove spaces around separators
		    if len(text) > 0 {
			incArgs = append(incArgs, strings.Split(text, " ")...)
		    }
		}
		if len(incArgs) > 0 {
		    convertFlags(&incArgs, 0)	// recursive call to add options in include file
		}
	    }
	}
    }
} // convertFlags

/********************************************************************
 *
 *	Main program
 *
 *	Create server by listening on the specified TCP port and then
 *	accepting connections from different iC clients.
 *	Each accepted client is handled in a different instance of the
 *	goroutine 'handleConnection()'.
 *
 *******************************************************************/

type multStringFlag []string			// flag can be used multiple times returning []string

func (m *multStringFlag) String() string {
    return ""					// satisfies interface - not used ??
}

func (m *multStringFlag) Set(value string) error {
    *m = append(*m, strings.TrimSpace(value))
    return nil
}

var opt_e multStringFlag
var opt_s, opt_p, opt_A *string
var opt_a, opt_d, opt_t, opt_m, opt_q, opt_r, opt_k, opt_z *bool

var reData, reNname, reSRiec, renName, resrIec, reEquiv, reXname, reQXBWL, reIXBWL *regexp.Regexp
var reSort *regexp.Regexp
var named string
var flag1 *flag.FlagSet
var t0 time.Time

func main() {
    re := regexp.MustCompile(`.*[/\\]`)
    named = re.ReplaceAllLiteralString(os.Args[0], "")
    flag1 = flag.NewFlagSet(named, flag.ExitOnError)
    opt_s = flag1.String("s", "0.0.0.0", "host address that clients must specify unless")
    opt_p = flag1.String("p", TCP_PORT, "specify TCP port")
    opt_a = flag1.Bool("a", false, "autovivify \"iCbox\" - provides virtual I/O")
    opt_d = flag1.Bool("d", false, "autovivify \"iCbox -d\" - useful for monitoring real I/O")
    opt_A = flag1.String("A", "", "autovivify alternate iCbox call (default \"iCbox\")")
    flag1.Var(&opt_e, "e", "define equivalences eg 'IX0=IX0-1=IX0-2'\nmay be used multiple times")
    opt_t = flag1.Bool("t", false, "trace messages for debugging")
    opt_m = flag1.Bool("m", false, "display elapsed time in seconds and microseconds")
    opt_q = flag1.Bool("q", false, "quit - do not report clients connecting and disconnecting")
    opt_r = flag1.Bool("r", false, "reset registered receivers when sender disconnects - ie reset\noutputs of an app when it shuts down (default no change)")
    opt_k = flag1.Bool("k", false, "kill previous client when a new client with the same name registers\n(default: do not accept the new client)")
    opt_z = flag1.Bool("z", false, fmt.Sprintf(`block keyboard input - must be used when running in background

        KEYBOARD inputs
        t   toggle -t option - trace messages for debugging.
        m   toggle -m option - display elapsed time.
        T   output %s Client Tables.
        q   stop %s and all registered iC apps.
    The last two actions can also be triggered from the iClive Build
    Menu. The easiest way to stop the iC system is to stop an iCbox
    by clicking its (X) button, which shuts down %s and all
    registered iC apps.
Copyright (C) 2001  John E. Wulff    <immediateC@gmail.com>
%q
%q`, named, named, named, ID_goserver_go, tcpcomm.ID_tcpcomm_go))

    // The -f and -R option are handled directly in convertFlags(). Include here for help info
    _     = flag1.String("f", "", "include file containing options, equivalences and client calls")
    _     = flag1.String("R", "", `run one app followed by -z and its arguments as a
separate process; -R ... must be last arguments`)

    argsP = &args				// intially fill []args
    convertFlags(&os.Args, 1)			// convert POSIX flags to individual flags - process -f -R flags
    flag1.Parse(args)
    if *opt_t { fmt.Printf("args    = %v\nrunArgs = %v\n", args, runArgs) }
    if len(flag1.Args()) > 0 {
	shutdownServer(0, fmt.Sprintf("ERROR: unknown command line arguments %v", flag1.Args()))
    }
    ipAdress := *opt_s + ":" + *opt_p
    if *opt_A != "" {
	flag1.Set("a", "true")
	if *opt_d {
	    rd := regexp.MustCompile(`iCbox`)
	    *opt_A = rd.ReplaceAllLiteralString(os.Args[0], "iCbox -d")
	}
    } else if *opt_d {
	flag1.Set("a", "true")
	flag1.Set("A", "iCbox -d")
    } else {
	flag1.Set("A", "iCbox")
    }
    /********************************************************************
     *  Compile all global regular expressions here only once.
     *  A Regexp is safe for concurrent use by multiple goroutines.
     *******************************************************************/
    reData  = regexp.MustCompile(`^(\d+):(.*)$`)
    reNname = regexp.MustCompile(`^N(.+)$`)
    reSRiec = regexp.MustCompile(`^([SR])([\\\/\w]+(-\d*)?)(\((\d+)\))?$`)
    renName = regexp.MustCompile(`^n(.+)$`)
    resrIec = regexp.MustCompile(`^([sr])([\\\/\w]+(-\d*)?)(\((\d+)\))?$`)
    reEquiv = regexp.MustCompile(`^E(.+)$`)
    reXname = regexp.MustCompile(`^X(.*)$`)
    reQXBWL = regexp.MustCompile(`^Q[XBWL]\d+`)
    reIXBWL = regexp.MustCompile(`^I[XBWL]\d+`)
    reSort  = regexp.MustCompile(`^([A-Z]+)(\d+)(-(\d+))?`)
    /********************************************************************
     *  Prime ctrl-C interrupt and terminate signals
     *******************************************************************/
    sigs := make(chan os.Signal, 1)
    signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
    go func() {
	sig := <-sigs
	shutdownServer(0, fmt.Sprintf("\nStopped by %v signal", sig))
    }()
    /********************************************************************
     *  Optionally allocate equivalences
     *******************************************************************/
    if len(opt_e) != 0 {
	equivs := strings.Join(opt_e, ",")
	if *opt_t { fmt.Printf("Allocate equivs '%v'\n", equivs) }
	allocateEquivalences(equivs)
    }
    /********************************************************************
     *  Optionally start Bernstein chain of iC apps with -R option
     *******************************************************************/
    if len(runArgs) > 0 {
	var runFlags = make([]string, 0, 5)
	if *opt_p != TCP_PORT { runFlags = append(runFlags, "-p", *opt_p) }
	if *opt_t { runFlags = append(runFlags, "-t") }
	if *opt_m { runFlags = append(runFlags, "-m") }
	if *opt_q { runFlags = append(runFlags, "-q") }

	if len(runFlags) > 0 {
	    runArgs = append(runArgs[:1], append(runFlags, runArgs[1:]...)...)	// insert -p <port> etc
	}
	if ! *opt_q { fmt.Printf("$ %s %v\n", runArgs[0], strings.Join(runArgs[1:], " ")) }
	cmd := exec.Command(runArgs[0], runArgs[1:]...)
	cmd.Stdin  = os.Stdin			// lets 'stty size' work correctly in iClive
	cmd.Stdout = os.Stdout			// show output of apps in Bernstein chain on terminal
	cmd.Stderr = os.Stderr
	err := cmd.Start()			// execute first iC app of Bernstein chain
	if err != nil {
	    shutdownServer(7, fmt.Sprint(err))
	}
    }
    /********************************************************************
     *  Start the 'autoVivBox()' goroutine, which waits to be triggered by
     *  a name string on go channel 'autoBoxCh' from various instances of
     *  'handleConnection()' to start an iCbox if option -a, else warning
     *******************************************************************/
    go autoVivBox(*opt_A)
    /********************************************************************
     *  Start the 'keyboardIn()' goroutine unless -z option
     *******************************************************************/
    if ! *opt_z {
	go keyboardIn()
    }
    /********************************************************************
     *  Launch server
     *******************************************************************/
    if ! *opt_q { fmt.Printf("%s created. Waiting for events\n", named) }
    listen, err := net.Listen("tcp4", ipAdress)
    if err != nil {
	shutdownServer(2, fmt.Sprint(err))
    }
    defer listen.Close()
    /********************************************************************
     *  Accept TCP connections forever and handle communication in
     *  dedicated goroutine 'handleConnection()' when connection accepted.
     *******************************************************************/
    for {
	conn, err := listen.Accept()	// Accept a TCP connection
	if err != nil {
	    shutdownServer(3, fmt.Sprint(err))
	}
	go handleConnection(conn)	// Handle communication on one TCP connection
    }
} // main

/*********** POD to generate man page *******************************

=encoding utf8

=head1 NAME

iCserver - the central server for iC clients (Go version)

=head1 SYNOPSIS

  iCserver[ -qadrkztmh][ -s <host>][ -p <port>]
          [ -e <equivalences>][ -A <cmd>][ -f <file>]
          [ -R <aux_app>[ <aux_argument> -R ...]] # must be last arguments
    -s host address of server - clients must specify the same address
            unless (default '0.0.0.0' which accepts any client host)
    -p port service port of server for all clients  (default '8778')
            make sure the Firewall is open for the TCP port used.
    -e equivalences  eg: -e IX0=IX0-1     (can be used more than once)
            or a comma separated list eg: -e IX0=IX0-1,QX0=QX0-1,IB1=IB1-1
    -q      quiet - do not report clients connecting and disconnecting
    -a      autovivify I/O clients 'iCbox' for missing I/O's (any number
            of iCbox's can be autovivified with -a as apps register)
    -d      autovivify I/O client 'iCbox -d' for missing I/O's, which is
            useful to monitor real I/O (only one 'iCbox -d' can be auto-
            vivified to avoid recursive calls for missing senders)
    -A cmd  use <cmd> to autovivify I/O clients (eg -A 'iCbox -Q2 -C19')
    -f file read options, equivalences and client calls from this INI file
    -r      reset registered receivers when sender disconnects - ie reset
            outputs of an app when it shuts down (default no change)
    -k      kill previous client when a new client with the same name
            registers (default: do not accept the new client)
    -z      block keyboard input (required when iCserver is executed as a
            background process)
    -h      help, ouput Usage text only
        DEBUG options
    -t      trace messages for debugging
    -m      display elapsed time in seconds and microseconds
        AUXILIARY app   - start a Bernstein chain
    -R <app ...> run one app followed by -z and its arguments as a
                 separate process; -R ... must be last arguments,
                 which do not need to be quoted.
             eg: -R iCbox X0-X3 X10 -R sorter
        KEYBOARD inputs
        t   toggle -t option - trace messages for debugging.
        m   toggle -m option - display elapsed time.
        T   output iCserver Client Tables.
        q   stop iCserver and all registered iC apps.
    The last two actions can also be triggered from the iClive Build
    Menu. The easiest way to stop the iC system is to stop an iCbox
    by clicking its (X) button, which shuts down iCserver and all
    registered iC apps.

=head1 DESCRIPTION

B<iCserver> acts as a hub for a number of iC clients in a network,
who send data to each other. Each client connects via TCP/IP to
iCserver on a specified port (default 8778).  Normally only one
iCserver on one port runs in a network.  It is possible to run several
iCservers on different ports, but clients registered with one of the
iCservers cannot communicate with clients registered with the others.
Clients can connect either via 'localhost' when they run on the same
processor as iCserver (default for iC clients) or via the host address
of the processor iCserver is running on.  If iCserver is running on
a different host than some iC applications, care must be taken that
the local Firewall of the processor iCserver is running on allows TCP
port 8778 (or the port chosen with the -p option) to make connections.
The Zone is either the External Zone or the Public Zone if it exists.

On connection each client registers itself and all its inputs and
outputs with iCserver. This registration is an iC specific operation
and has nothing to do with the TCP/IP connection process described
above.

Clients for B<iCserver> are I<iC programs>, I<real I/O programs> or
I<virtual I/O programs> such as B<iCbox> and I<debugging programs>
such as B<iClive>. These clients either send or receive data values
from and to named I/O locations or debugging information.  As far
as iCserver is concerned I/O locations could have any name, but
the iC language calls for I/O names or addresses according to the
IEC-1131 standard.  Data values can be 8 bit bytes (eg: IB1 QB9),
16 bit words (IW2 QW10), 32 bit long words (IL4 QL12) or 64 bit huge
words (IH8 QH16).  Bit values like IX0.0 IX0.1 QX8.2 QX8.3 are always
transmitted as bytes - in this case via IX0 and QX8, which are the
names used for registration.  Whenever any bit in the byte changes,
the whole byte is transmitted.

Each client registers the I/O names it requires on connection to
iCserver. Each unique name is stored in a Map variable in iCserver,
whose key is the name and whose value is a channel number, which is
used for all actual data transfers. These are not Go channels, but
simple integers used for indexing iC variables in different iC apps.

The Map is only required for registration. Each channel allows the
identification of one Sender for data on the channel (or I/O name)
and one or more Receivers for the data. A detailed description follows
in the SPECIFICATION below.

Additional functionality in B<iCserver>.

 a) Equivalences
    -e option - interconnection of different I/O addresses in
    iCserver. This option puts two or more entries in the Map and
    assigns them a common channel number. Send or receive entries
    associated with the channel are undefined at this stage. Then when
    registration of one of the equivalence names occurs, the common
    channel number is used. With this option different IEC-1131
    addresses can be assigned to the same channel, thereby making
    them equivalent - in effect interconnecting them.

    This functionality is required when the output of one iC control
    application is to be the input for another iC control application
    (often a different instance of the same application).  Several
    equivalences may be specified. The order of the equivalence is not
    relevant (it is not an assignment). For correct autovivification
    outputs should be named first though.

    Example 1:
        iCserver -e QX7-0=IX7-1,QX7-1=IX7-0

    This connects QX7 of instance 0 with IX7 of instance 1 via a common
    channel and QX7 of instance 1 with IX7 of instance 0 via another
    channel. Equivalencing an output and an input of the same instance
    is possible but rather useless and much slower than using internal
    variables (it may be useful for testing).

    Equivalencing is also required if one external I/O source must be
    connected to the input of more than one iC control application.

    Example 2:
        iCserver -e IX8=IX8-0=IX8-1,IX9=IX9-0=IX9-1

    This sets up common inputs IX8 and IX9 to two instances of the same
    controller. Naming the other inputs with the same base IEC-1131
    address is not necessary but highly advisable for transparency in
    the documentation. The first address is used for autovivifying an
    iCbox if it does not exist already. Autovivification does not take
    place for a channel, until a receiver has been registered for that
    channel and all other registrations have taken place.

    Equivalencing two or more output addresses (Q...) in an iC
    control application is an error. Both output addresses try to
    register as senders - this leads to two or more senders on the
    same channel. When the second or later output sender registers,
    the error will be reported. A similar error will be reported
    if a second external input device in an equivalence chain is
    registered as a second sender. This would happen if both IX8 and
    IX8-0 were started as iCboxes after the equivalence statement in
    Example 2 above. In rare cases two outputs may be legitametely
    equivalenced if an iC application uses a certain output name and
    a real output with a different name must be used to accept that
    output. This practice is highly deprecated, because transparency
    in the documentation is lost.

    Formally equivalences consist of two or more IEC base identifiers,
    optionally followed by '-' and a 1 to 3 digit instance specifier,
    separated by an equal sign '='. Several equivalences may be
    specified in a comma ',' separated list or several -e equivalence
    parameters may be used.

    It is not allowed to equivalence IEC identifiers of different types
    X B W L or H, since the consequences are not what is expected.

*   I<Equivalances can also be defined later by iC programs> for
    input IEC variables in that application before the variables are
    registered using the -e option. The same calling sequence described
    for iCserver above applies.  For control applications for which
    several instances are instantiated, there is an extra -e option:

        -eI # equivalence all IEC input names to the same names-<inst>

    This option for the control application only makes sense if an
    instance is also specified eg: -i1

    The control application bar generated from bar.ic defines 3 IEC
    inputs: IB1, IX0 and IX2. Calling bar as follows:

        bar -i1 -eI

    generates the following 3 equivalences:

        Equivalence 'IB1=IB1-1'
        Equivalence 'IX0=IX0-1'
        Equivalence 'IX2=IX2-1'

    When generating equivalences with the -e option from an iC app IEC I/O
    names which have already been registered by other iC apps may not be
    used. This is an error.

 b) Autovivification
    -a option - automatic startup of one or more iCbox virtual I/O's.
    When a control application registers its I/O's, 'iCserver -a' starts up
    a matching 'iCbox' for all complementary I/O's, which have not already
    been registered. With the -a option, clients must be started in a
    particular order:

    i)  iCserver -a   # always first anyway.

    ii) any manually started I/O's with real I/O or specific ranges etc
        or because of equivalances (optional).

    iii)iC control application(s), which causes iCserver to autovivify
        any missing I/O's as an iCbox with appropriate ranges for each app.

    -d option - automatic startup of one iCbox -d for monitoring real
    I/O's. When a control application registers its I/O's, 'iCserver -d'
    starts up a matching 'iCbox -d' for all complementary I/O's for
    monitoring.  Outputs are the same as without the -d option, but
    inputs will only display their value and cannot be changed. With
    the -d option, clients must be started in a different order:

    i)  iCserver -d   # always first anyway.

    ii) one control application, which causes iCserver to autovivify all
        I/O's (which are all missing their complementary senders and
        receivers at this stage) as an 'iCbox -d' with appropriate ranges.

    iii)any manually started I/O's with real I/O or virtual I/O. Care must
        be taken to ensure that all missing inputs for the app are present,
        since no further missing IEC inputs are autovivified. Missing
        outputs are optional, since they show up in the autovivified iCbox,
        although it is best to keep all inputs and outputs together in one
        controlling iCbox.

    -A <cmd> - automatic startup of one or more <cmd>
               usually <cmd> is iCbox with extra options eg -A 'iCbox -H'
    Startup and calling order is the same as for the -a option unless the
    -d option is also used - in which case the -d option applies.

    Note: with the -a -d and -A option care is taken to only autovivify
    the first member of an equivalence list, which is the sender of that
    list if it is an output or will become the sender if it is an input.

 c) -r option - reset registered receivers when sender disconnects
    ie reset outputs of an app when it shuts down (default no change)

 d) -k option - if a sender registers with the same name as one already
    registered, iCserver kills the previously registerd sender, rather
    than report an error. This allows a re-compiled version of an iC
    application to be started, while an old version is still running -
    the old one will quitly be killed. This should not be done in a
    production system.

    Note: when a control application exits (disconnects from iCserver)
    the I/O's are not disconnected. They can be re-used by a restarted
    similar control application. If the restarted control application
    uses different I/O's a new iCbox is autovivified for any extra
    I/Os. This situation could be confusing and it would be better to
    start again by stopping iCserver with q or ctrl-C. When iCserver
    exits, all connected clients are disconnected and closed.

 e) -f <INI_file> - execute a file with iCserver options at startup.
    For very large equivalence tables a file defining equivalences and
    possibly other iCserver switches and options can be used.

        iCserver -f <INI_file>

    Format of the INI_file:
        -e <equivalence_line>
        ...                       # more equivalences
        <other_options>
        -R <client call>          # run a client in a Bernstein chain
        ...                       # only further -R calls

    Example same as Example 1, 2 and 3 above with extra options:
      -e QX7-0 = IX7-1 QX7-1 = IX7-0 # equivalences joining 2 instances
      -e IX8 = IX8-0 = IX8-1         # input equivalences
      -e IX9 = IX9-0 = IX9-1
      -a -k                          # extra switches for iCserver
      -R iCbox -n sorter-IO IX0 QX0 QX1 # client call with multiple arguments
      -R sorter                      # plain client call

    Individual equivalences and client calls must be written without
    spaces in the command line.  In the INI file white spaces before
    and after an = or , may be used in equivalences.

    Comments in the INI file are started with #

 f) -R <aux_app>[ <aux_app_argument> ...] # must be last arguments
    Start a Bernstein chain of iC application which are each initialised
    serially and then run in parallel - in this case with iCserver.
    Example 3:
        iCserver -R iCbox -n sorter-IO IX0 QX0 QX1 -R sorter

If B<iCserver> has not been started, the first I<iC program> or
I<iClive> will start B<iCserver -a -z> as a parallel process, which
means that for most cases B<iCserver> does not need to be started
separately, unless special options are needed.  Equivalences can
also be specified by I<iC programs> with the -e option as described
in a)* above.

=head1 SPECIFICATION

This specification of B<iCserver> is provided here to give users
information on how communication in the iC network is handled. This
specification was developed 10 jears before JSON was published,
which has similarities. But the iC communication protocol is more
compact than JSON, particularly for bits, so it is being retained.

 1) On connection to iCserver each client register by supplying the
    following registration information:

    a)  The name of the client device:

        <name_op><client_name>[-<instance>]

        <name_op>
        N signifies, that the named client registers with iCserver.
        n signifies, that the named client unregisters from iCserver.

        <client_name>
        The client_name must be unique. For iC applications the control
        program register with the name it is called with. The iClive
        debug program registers with the name of the iC application
        it is connected to preceded by a '-'. The leading - is used
        by iClive to detect live registrations, which may register
        and unregister with different names, to monitor different
        iC applications running at the same time.

        -<instance>
        An optional instance number allows the registrations of several
        instances of the same control application. Its use is explained
        in detail in section 6.

    b)  Registration codes: the type of data the client is sending or
        receiving and the physical address in IEC-1131 format:

        <send_receive_op><address_code>[-<instance>][(<bit_map>)]

        <send_receive_op>
        S signifies, that the client sends data to iCserver.
        R signifies, that the client receives data from iCserver.
        s sender unregisters from iCserver (used by iClive only)
        r receiver unregisters from iCserver (used by iClive only)

        <address_code>
        The <address_code> is made up either of an IEC-1131 base name
        used in iC programs to identify (address) a particular input
        or output or by a special address for communicating with
        the debugging client iClive consisting of an <address_code>
        starting with C or D. These are followed by the name of the
        debugging client.

        -<instance>
        An optional instance number for a particulat input or output
        or debugging client.

        (<bit_map>)
        An optional bit map, which defines the bits implemented for
        a digital input or output. The default value is 255, which
        implements bits 0 to 7.

    c)  Termination code: for the registration string consisting of
        the single letter 'Z'. This allows the registration string
        to be longer than the TCP/IP transmission size of 1400 bytes
        (also maximum buffer size) - it will be sent in more than one
        transmission if it is too long.

    In summary: the registration string consists of a comma separated
    list beginning with 'Nname' followed by one or more individual
    sender and receiver registration codes starting with 'S', 'R',
    'C' or 'D' and terminated with a single 'Z'. The whole is sent
    in one or more TCP/IP transmissions. iCserver acknowledges each
    transmission by sending a comma separated list of channel numbers
    used for each sender or receiver for each partial registration
    string. These acknowledge strings are always shorter than the
    registration strings - hence there is no danger of buffer overflow.

        IEC-1131 base names are as follows:

            <io_op><size_op><address>

            <io_op>
            I data is input to an iC control client.
            Q data is output from an iC control client.

                        - the following are for debugging clients only
            C debug data is output from an iC control client to iClive
            D debug data is input to an iC control client from iClive
                        - more details for C and D in section 5.

            <size_op>
            X signifies 8 bits of single bit data.
                    bit addresses are not used in the registration or
                    transmission of X senders and receivers. 8 bits are
                    always transmitted as 1 byte and the clients insert
                    or extract modified bits as necessary.
            B signifies 8 bits or 1 byte of numerical data.
            W signifies 16 bits or 2 bytes of numerical data.
            L signifies 32 bits or 4 bytes of numerical data.
            H signifies 64 bits or 8 bytes of numerical data.

            <address>
            This number is the numerical IEC-1131 address. Each
            data type QX, QB, QW, QL and QH as well as IX, IB, IW,
            IL and IH has its own address space, which is a sparse
            array.  The numbering of each of these address spaces is
            independent of the others and continuous.

            Some hardware I/O drivers may have a common address
            space which may also enforce word, long word and long
            long word baundaries.  Checking for such boundaries
            and overlap of I/Os with different word length has been
            implemented for iC apps but is normally not switched on.
            This does not affect iCserver either way.

        NOTE: the syntax of correct IEC-1131 base names is only checked
        by iCserver for equivalence declarations. All other IEC_1131
        base names are received from clients where they should have
        been checked.

        Examples:
        SIX0    8 single bits IX0.0 to IX0.7 sent from an external source
        RIX0    8 single bits IX0.0 to IX0.7 received by an iC controller
        SQX9(7) 3 single bits IX9.0 to IX9.2 sent from an iC controller
        RQX9(7) 3 single bits IX9.0 to IX9.2 received by an external sink

        SIB10   single 8 bit byte IB10 sent from an external source
        RIB10   single 8 bit byte IB10 received by an iC controller
        SQB15   single 8 bit byte QB15 sent from an iC controller
        RQB15   single 8 bit byte QB15 received by an external sink

        SIW10   single 16 bit word IW10 sent from an external source
        RIW10   single 16 bit word IW10 received by an iC controller
        SQW15   single 16 bit word QW15 sent from an iC controller
        RQW15   single 16 bit word QW15 received by an external sink

        SIL10   single 32 bit long IL10 sent from an external source
        RIL10   single 32 bit long IL10 received by an iC controller
        SQL15   single 32 bit long QL15 sent from an iC controller
        RQL15   single 32 bit long QL15 received by an external sink

        SIH10   single 64 bit long long IH10 sent from an external source
        RIH10   single 64 bit long long IH10 received by an iC controller
        SQH15   single 64 bit long long QH15 sent from an iC controller
        RQH15   single 64 bit long long QH15 received by an external sink

 2) One client device has one two way TCP/IP connection to iCserver,
    but it can register for any number of send and/or receive
    I/O addresses.  For example the simple iCbox registers as X0
    by default, supplying 8 bits of input data and sinking 8 bits
    of output data. It will register itself as SIX0,RQX0. SIX0 is
    the source of the signals IX0.0 to IX0.7 and RQX0 is the sink
    of QX0.0 to QX0.7.  An iC control application handling those
    two signal groups will register itself as RIX0,SQX0.  RIX0 for
    receiving IX0.0 to IX0.7 and SQX0 for sending QX0.0 to QX0.7.

    This scheme makes I/O devices and controllers identical as far as
    iCserver data handling is concerned and opens the possibility,
    that one iC control application can communicate data to another via
    the iCserver network. This is a definite requirement when multiple
    iC applications are involved in an overall project. A more obscure
    use would be sending data from an external input directly to an
    external output. More realistic would be using an iCbox output to
    monitor the data going to another client doing real output.

 3) To allow monitoring and to allow several iC programs to react to
    input from a single source, iCserver supports the registration
    of the same receive I/O address from several clients.  iCserver
    will then send the same data to each of those clients, when it
    is received from its sender source for distribution.

 4) On the other hand, only one client may register a particular I/O
    address as its sender. If iCserver did not catch this as an error,
    serious malfunction would occur. Several clients could then send
    different data, which would be overwritten inconsistently at the
    destination receivers, thereby breaking the fundemental strategy
    of the single assignment rule of event driven systems.

 5) Communication with debugging devices (iClive) is regulated with two
    alternative <io_op>s; 'C' for controller and 'D' for debugger.
    There can be several instances of iClive running together, but
    only one for each control application. They are addressed by the
    program name, not an IEC-1131 address. A control application named
    'sorter' would connect with iClive as follows:

        iC program                 iClive
         Nsorter,                 N-sorter,
         SCsorter,     ====>      RCsorter,
         RDsorter     <====       SDsorter

    Each unit registers for both send and receive, since the data
    handling for send and receive are handled in separate sections of
    iCserver and a live display needs both. This means that no special
    treatment of live display registration and data parsing is needed.

 6) iCserver allows every client to be started by specifying an
    optional <instance> parameter (see 1), which is appended to
    all registration codes or addresses. The same instance number
    is also appended to the addresses of I/O clients communicating
    with a particular controller instance. Such a set of a controller
    and I/O clients with a common instance number are a set, which
    communicate with each other. The instance parameter is passed to
    each client with the calling parameter -i<instance>.

    Example without instance number:
        sorter     # (using IX0, QX0, QX1) will register with:
            Nsorter,SCsorter,RDsorter,RIX0,SQX0,SQX1,Z
        iCbox      IX0 QX0 QX1           # will register with:
            NIX0,SIX0,RQX0,RQX1,Z
        iClive     sorter                # will register with:
            N-sorter,RCsorter,SDsorter,Z

    Example of a first instance:
        sorter -i0 # (using IX0, QX0, QX1) will register with:
            Nsorter-0,SCsorter-0,RDsorter-0,RIX0-0,SQX0-0,SQX1-0,Z
        iCbox  -i0 IX0 QX0 QX1           # will register with:
            NIX0-0,SIX0-0,RQX0-0,RQX1-0,Z
        iClive -i0 sorter                # will register with:
            N-sorter-0,RCsorter-0,SDsorter-0,Z

    Example of a second instance:
        sorter -i1 # (using IX0, QX0, QX1) will register with:
            Nsorter-1,SCsorter-1,RDsorter-1,RIX0-1,SQX0-1,SQX1-1,Z
        iCbox  -i1 IX0 QX0 QX1           # will register with:
            NIX0-1,SIX0-1,RQX0-1,RQX1-1,Z
        iClive -i1 sorter                # will register with:
            N-sorter-1,RCsorter-1,SDsorter-1,Z

 7) Registration of clients from the view of iCserver

    a)  A client connects to iCserver via TCP/IP, which accepts and
        issues a 'conn' object.

    b)  The client sends a Message with all its name and registration
        codes in a comma separated list to iCserver (see 1).

    c)  In iCserver the registration code <address_code>[-<instance]
        is used as a key in a Map, which associates a unique
        channel number with the key.  The key identifies either an
        IEC-1131 I/O or a debug address. It is the identification or
        name of a communication send/receive pair and is unique. The
        associated channel number is an index in an array in each
        client, which holds run-time details for the communication
        pair or channel in each client app.
        NOTE: iC channel indexes have nothing to do with GO channels.

        iCserver uses the Map of I/O addresses during registration
        to catch attempts to register an I/O address twice incorrectly.

    d)  iCserver acknowledges registration of a client by sending a
        comma separated list of channel numbers in the same order as
        the I/O registration codes. If a particular registration code
        cannot be registered correctly, the acknowledge list contains
        a negative error number for that entry.

 8) Registration from the view of clients

    a)  When a client starts it tries to connect to iCserver. If it
        fails it will start 'iCserver -a' and try to connect again
        a number of times, which should succeed. Once connected it
        sends one or more strings of all the registration codes it
        uses, which are all I/O names if it is an iC program or debug
        names if iClive.

    b)  iCserver responds by sending the acknowledge string of channel
        numbers. When the client receives this positive acknowledgement,
        which will be the first message received, it pairs off the
        registration codes sent and the channel numbers received in the
        acknowledgement. The details depend on the client application.

        For the first example in section 6, the following tables will
        be set up in the different clients:

         client  direction  code    channel
        sorter
                     S     Csorter     1
                     R     Dsorter     2
                     R     IX0         3
                     S     QX0         4
                     S     QX1         5
        iCbox
                     S     IX0         3
                     R     QX0         4
                     R     QX1         5
        iClive
                     R     Csorter     1
                     S     Dsorter     2

 9) When registration is complete, iCserver can determine, that each
    channel has one sender and at least one receiver. If a project
    is run only as a simulation with the -a option, missing senders
    or receivers are autovivified. If a project drives real I/Os a
    warning is issued by iCserver if I/Os are missing.

 10)Formats of data messages between iCserver and clients follow a
    uniform pattern:

    <channel_number>:<data_value> in a comma separated list for a number
    of simultaneous values on different channels to the same client.

    <channel_number> is an unsigned decimal string identifying the
    unique channel allocated by iCserver.

    <data_value> is a decimal string with a - sign for negative
    values in case of the numeric types B, W, L or H, unsigned for
    type X. iClive uses a more complex data format consisting of a
    semi-colon separated list of space separated data pairs.

    Examples:
        4:1,5:2         # set QX0.0 and QX1.1 in the example above
        2:4;2 128;5 48  # a type 4 live data telegram for iClive

 11)Unregistering a client.

    A client is able to unregister itself, without disrupting other
    running processes, particularly if it is only a secondary receiver.
    This will happen when a client closes a TCP/IP connection. When a
    client is disconnected, the Map entries for the channel numbers
    are kept in case it connects again.

=head1 AUTHOR

John E. Wulff

=head1 BUGS

Email bug reports to B<immediateC@gmail.com> with L<iC Project> in the
subject field.

The SPECIFICATION supplements the DESCRIPTION of B<iCserver> and may be
useful in understanding its workings.

=head1 SEE ALSO

L<immcc(1)>, L<iCbox(1)>, L<iClive(1)>

=head1 COPYRIGHT

Copyright (C) 2000-2021  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=for ******* End of man page POD ***********************************/

# $Id: iCmsg.pm 1.7 $

########################################################################
#
#   Originally described in "Advanced Perl Programming" by Sriram Srinivasan
#   p. 203 as "Msg: Messaging Toolkit" and freely available on the internet
#   as Msg.pm - use in the immediate C project is gratefully acknowledged.
#
#   Modification John Wulff 2000.05.27 - event_loop()
#	select time_out is 0 if loop_count is defined
#	else undef - which maintains previous functionality
#
#	if loop_count has a fractional part interpret it as
#	the time_out value and set loop_count to 1 - useful for
#	timed message loops
#
#   Modification John Wulff 2012.10.31 - enable non-blocking
#	The original implemantion from 2000 had:
#	  my $blocking_supported = 0;	# executed after BEGIN block
#	leading to the situation that setting $blocking_supported in
#	the BEGIN block was overwritten with 0 - inhibiting non-blocking.
#
#	Also set_non_blocking and set_blocking requires F_GETFL in the
#	first fcntl() call, which was never executed because of the above.
#
#	According to the Errata for "Advanced Perl Programming"
#	$offset in _rcv() was incorrectly calculated - fixed.
#
#   Modification John Wulff 2012.11.06 - disable Nagle's algorithm
#	From <en.wikipedia.org/wiki/Nagle's_algorithm>
#	Applications that expect real time response can react poorly to
#	Nagle's algorithm. The algorithm purposefully delays transmission,
#	increasing bandwidth efficiency at the expense of latency.
#	For this reason applications with low bandwidth time sensitive
#	transmissions typically use TCP_NODELAY to bypass the Nagle delay.
#
#	This very much applies to network traffic in the immediate C system.
#	Messages are typically 5 to 10 bytes long staggered at intervals
#	which are usually 10's of milliseconds to seconds apart, but can
#	involve a burst of several messages as a result of the same trigger.
#	Nagle's algorithm sends the first if these bursts and delays the
#	rest for up to 500 ms or when an unrelated return message comes,
#	which is much too late.	(Also changed for C code in tcpc.c)
#
#   Modification John Wulff 2012.11.08 - method to inhibit Nagle's algorithm
#	  Msg->inhibit_nagle(1)	# inhibit Nagle's algorithm for real time reponse
#	  Msg->inhibit_nagle(0)	# activate Nagle's algorithm again for throughput
#	Without the first call Nagle's algorthm is active - so Msg.pm
#	functions without change from the original for backwards compatibility.
#
#   Modification John Wulff 2013.04.25 - modify send buffering
#	The syswrite() call in _send() takes a short time to complete. The return
#	from the OS is initiated by an interrupt. If another _send() is executed
#	before the previous syswrite is complete, it causes a 2nd syswrite() of the
#	same message at the head of the queue, because the message was not shifted
#	until the 1st syswrite has completed. In the changed code $conn->{sendBusy} is
#	set before starting to syswrite messages in the buffer. When another _send
#	occurs while busy is set, _send returns immediately. The queued message
#	is sent during the current loop emtying the buffer.
#	By having the $conn->{sendBusy} flag, messages are written in the order
#	they come in.  $conn->{sendBusy} only applies to one $conn. Other
#	connections can barge in and pre-empt a sys_write without ill effects.
#
#	A similar modification has been implemented in _rcv() to buffer received
#	messages when rcvd_notification_proc is interrupted starting another _rcv().
#	This uses the $conn{rcvBusy} flag to return immediately.
#
#   Modification John Wulff 2013.04.28 - added trace arguments to inhibit_nagle()
#	  Msg->inhibit_nagle(0)		# Nagle's algorithm active
#	  Msg->inhibit_nagle(1)		# inhibit Nagle's algorithm - no traces
#	  Msg->inhibit_nagle(1, 1)	# also trace rcv calls only
#	  Msg->inhibit_nagle(1, 0, 1)	# also trace send calls only
#	  Msg->inhibit_nagle(1, 1, 1)	# also trace rcv and send calls
#
#   Modification John Wulff 2015.03.06 - added time out delay to connect() calls
#	Some iC clients, in particular iCbox, are started from applications
#	automatically with a system() call. These same applications usually also
#	start iCserver if it is not already running. This may involve a timing
#	race and the client may not be able to connect to iCserver immediately.
#	To guarantee a smooth startup sequence, connect() has been modified to
#	retry 3 times at 2 second intervals.
#	
#	This action is not appropriate if there is no likelyhood of iCserver
#	being started externally. This applies to iClive, which starts iCserver
#	itself if there is none to connect to. To speed up startup of iClive
#	an extra optional argument '$time_out_inhibit' has been added to connect().
#	  Msg->connect($host, $port, $rcvd_notification_proc, $time_out_inhibit)
#	$time_out_inhibit undef or 0	# retry 3 connects at 2 sec intervals
#	$time_out_inhibit 1		# returns immediately if connect fails
#
#   Modification John Wulff 2018.06.03 - output rcv and send trace messages to STDERR
#       rename package to iCmsg in file iCmsg.pm because of the many changes
#
########################################################################

package iCmsg;
use strict;
use IO::Select;
use IO::Socket;
use Socket qw(IPPROTO_TCP TCP_NODELAY);
use Carp;

use vars qw (%rd_callbacks %wt_callbacks $rd_handles $wt_handles);

%rd_callbacks = ();
%wt_callbacks = ();
$rd_handles   = IO::Select->new();
$wt_handles   = IO::Select->new();
my $blocking_supported;			# executed after BEGIN block
my ($nagle, $trace_rcv, $trace_send);	# global nagle and trace flags

BEGIN {
    # Checks if blocking is supported
    eval {
        require POSIX; POSIX->import(qw (F_GETFL F_SETFL O_NONBLOCK EAGAIN));
    };
    $blocking_supported = ($@) ? 0 : 1;	# executed before my $blocking_supported;
}

#-----------------------------------------------------------------
# Inhibit Nagle's algorithm (see in the first header)
				# $nagle 0 or undef is active, 1 is inhibited
				# $trace_rcv  1 is trace rcv messages
				# $trace_send 1 is trace send messages
sub inhibit_nagle {
    my $pkg;
    ($pkg, $nagle, $trace_rcv, $trace_send) = @_;
}

#-----------------------------------------------------------------
# Send side routines
sub connect {
    my ($pkg, $to_host, $to_port, $rcvd_notification_proc, $time_out_inhibit) = @_;
    my ($sock);
    my $time_out = $time_out_inhibit ? 0 : 3;

    # Create a new internet socket
    until (defined (
        $sock = IO::Socket::INET->new (
                                       PeerAddr => $to_host,
                                       PeerPort => $to_port,
                                       Proto    => 'tcp',
                                       Reuse    => 1)
        )
    ) {
	unless ($time_out--) {
	    return undef;
	}
	sleep(2);			# wait 2 seconds for server to start
    }

    setsockopt($sock, IPPROTO_TCP, TCP_NODELAY, $nagle) if defined $nagle;	# inhibit Nagle's algorithm
    # Create a connection end-point object
    my $conn = bless {			# Reference to $conn{sock} and $conn{rcvd_notification_proc}
        sock                   => $sock,
        rcvd_notification_proc => $rcvd_notification_proc,
    }, $pkg;				# CLASSNAME $pkg

    if ($rcvd_notification_proc) {
        my $callback = sub {_rcv($conn, 0)};
        set_event_handler ($sock, "read" => $callback);
    }
    print STDERR "iCmsg.pm:connect\n" if $trace_rcv or $trace_send;
    return $conn;
}

sub disconnect {
    my $conn = shift;
    my $sock = delete $conn->{sock};
    return unless defined($sock);
    print STDERR "iCmsg.pm:disconnect\n" if $trace_rcv or $trace_send;
    set_event_handler ($sock, "read" => undef, "write" => undef);
    close($sock);
}

sub send_now {
    my ($conn, $msg) = @_;
    print STDERR "iCmsg.pm:send_now    $msg\n" if $trace_send;
    _enqueue ($conn, $msg);
    $conn->_send (1); # 1 ==> $flush
}

sub send_later {
    my ($conn, $msg) = @_;
    print STDERR "iCmsg.pm:send_later  $msg\n" if $trace_send;
    _enqueue($conn, $msg);
    my $sock = $conn->{sock};
    return unless defined($sock);
    set_event_handler ($sock, "write" => sub {$conn->_send(0)});
}

sub _enqueue {
    my ($conn, $msg) = @_;
    # prepend length (encoded as network long)
    my $len = length($msg);
    print STDERR "iCmsg.pm:_enqueue [$len]$msg\n" if $trace_send;
    $msg = pack ('N', $len) . $msg;
    push (@{$conn->{sendQueue}}, $msg);
}

sub _send {
    my ($conn, $flush) = @_;
    my $sock = $conn->{sock};
    return unless defined($sock);
    my $sq = $conn->{sendQueue};

    # If $flush is set, set the socket to blocking, and send all
    # messages in the queue - return only if there's an error
    # If $flush is 0 (deferred mode) make the socket non-blocking, and
    # return to the event loop only after every message, or if it
    # is likely to block in the middle of a message.

    $flush ? $conn->set_blocking() : $conn->set_non_blocking();
    my $offset = (exists $conn->{send_offset}) ? $conn->{send_offset} : 0;

    # If $conn->{sendBusy} is set, syswrite was interrupted by another _send_now
    # or _send_later before syswrite has actually written the previous $msg
    # or all of its parts if the $msg is long. The effect of this in the
    # original implementation was, that later messages are actually
    # written to the socket before early ones, which had drastic
    # consequences for iC applications.
    # By having the $conn->{sendBusy} flag, messages are written in the order
    # they come in.  $conn->{sendBusy} only applies to one $conn. Other
    # connections can barge in and pre-empt a sys_write without ill effects.
    # jw 2013.04.25

    if ($conn->{sendBusy}++) {
	my $msg = $sq->[0];
	my ($l, $m) = unpack "N A*", $msg;
	print STDERR "iCmsg.pm:_send*0  [$l]$m sendBusy $conn->{sendBusy}\n" if $trace_send;
	return 1;			# $msg is queued out of turn and sywrite not complete
    }
    while (@$sq) {
        my $msg            = shift @$sq;
        my $bytes_to_write = length($msg) - $offset;
        my $bytes_written  = 0;
        while ($bytes_to_write) {
	    my ($l, $m) = unpack "N A*", $msg;
	    print STDERR "iCmsg.pm:_send*1  [$l]$m\n" if $trace_send;
            $bytes_written = syswrite ($sock, $msg,
                                       $bytes_to_write, $offset);
	    print STDERR "iCmsg.pm:_send*2  [$l]$m\n" if $trace_send;
            if (!defined($bytes_written)) {
                if (_err_will_block($!)) {
                    # Should happen only in deferred mode. Record how
                    # much we have already sent.
                    $conn->{send_offset} = $offset;
                    # Event handler should already be set, so we will
                    # be called back eventually, and will resume sending
                    return 1;
                } else {    # Uh, oh
                    $conn->handle_send_err($!);
		    push (@$sq, $msg);
                    return 0; # fail. Message is back in queue ..
                }
            }
            $offset         += $bytes_written;
            $bytes_to_write -= $bytes_written;
        }
        delete $conn->{send_offset};
        $offset = 0;
        last unless $flush;	# Go back to select and wait for it to fire again if $flush == 0
    }
    print STDERR "iCmsg.pm:_send*end	sendBusy $conn->{sendBusy}\n" if $trace_send and $conn->{sendBusy} > 1;
    $conn->{sendBusy} = 0;		# early syswrite $msg completed - later queued $msg picked up in loop
    # Call me back if queue has not been drained.
    if (@$sq) {
        set_event_handler ($sock, "write" => sub {$conn->_send(0)});
    } else {
        set_event_handler ($sock, "write" => undef);
    }
    1;  # Success
}

sub _err_will_block {
    if ($blocking_supported) {
        return ($_[0] == EAGAIN());
    }
    return 0;
}
sub set_non_blocking {
    if ($blocking_supported) {
	my ($conn) = @_;
        my $flags = fcntl($conn->{sock}, F_GETFL(), 0);
        fcntl($conn->{sock}, F_SETFL(), $flags | O_NONBLOCK());
    }
}
sub set_blocking {
    if ($blocking_supported) {
	my ($conn) = @_;
        my $flags = fcntl($conn->{sock}, F_GETFL(), 0);
        fcntl($conn->{sock}, F_SETFL(), $flags & ~O_NONBLOCK());
    }
}
sub handle_send_err {
   # For more meaningful handling of send errors, subclass iCmsg and
   # rebless $conn.
   my ($conn, $err_msg) = @_;
   warn "Error while sending: $err_msg\n";
   set_event_handler ($conn->{sock}, "write" => undef);
}

#-----------------------------------------------------------------
# Receive side routines

my ($g_login_proc,$g_pkg);
my $main_socket = 0;
sub new_server {
    @_ == 4 || die "iCmsg->new_server (myhost, myport, login_proc)\n";
    my ($pkg, $my_host, $my_port, $login_proc) = @_;

    $main_socket = IO::Socket::INET->new (
                                          LocalAddr => $my_host,
                                          LocalPort => $my_port,
                                          Listen    => 5,
                                          Proto     => 'tcp',
                                          Reuse     => 1);
    die "Could not create socket: $! \n" unless $main_socket;
    print STDERR "iCmsg.pm:new_server\n" if $trace_rcv or $trace_send;
    set_event_handler ($main_socket, "read" => \&_new_client);
    $g_login_proc = $login_proc; $g_pkg = $pkg;
}

sub rcv_now {
    my ($conn) = @_;
    my ($msg, $err) = _rcv ($conn, 1); # 1 ==> rcv now
    print STDERR "iCmsg.pm:rcv_now     $msg\n" if $trace_rcv;
    return wantarray ? ($msg, $err) : $msg;
}

sub _rcv {                     # Complement to _send
    my ($conn, $rcv_now) = @_; # $rcv_now complement of $flush
    # Find out how much has already been received, if at all
    my ($msg, $offset, $bytes_to_read, $bytes_read);
    my $sock = $conn->{sock};
    return unless defined($sock);
    if (exists $conn->{msg}) {
        $msg           = $conn->{msg};
        $offset        = length($msg);      # sysread appends to it. (- 1 incorrect according to Errata)
        $bytes_to_read = $conn->{bytes_to_read};
        delete $conn->{'msg'};              # have made a copy
	print STDERR "iCmsg.pm:_rcv*0   [$offset]$msg\n" if $trace_rcv;
    } else {
        # The typical case ...
        $msg           = "";                # Otherwise -w complains
        $offset        = 0 ;
        $bytes_to_read = 0 ;                # Will get set soon
    }
    # We want to read the message length in blocking mode. Quite
    # unlikely that we'll get blocked too long reading 4 bytes
    if (!$bytes_to_read)  {                 # Get new length
        my $buf;
        $conn->set_blocking();
        $bytes_read = sysread($sock, $buf, 4);
        if ($! || ($bytes_read != 4)) {
            goto FINISH;
        }
        $bytes_to_read = unpack ('N', $buf);
    }
    $conn->set_non_blocking() unless $rcv_now;
    while ($bytes_to_read) {
	print STDERR "iCmsg.pm:_rcv*1   [$bytes_to_read]$msg\n" if $trace_rcv;
        $bytes_read = sysread ($sock, $msg, $bytes_to_read, $offset);
        if (defined ($bytes_read)) {
            if ($bytes_read == 0) {
                last;
            }
            $bytes_to_read -= $bytes_read;
            $offset        += $bytes_read;
        } else {
            if (_err_will_block($!)) {
                # Should come here only in non-blocking mode
                $conn->{msg}           = $msg;
                $conn->{bytes_to_read} = $bytes_to_read;
		print STDERR "iCmsg.pm:_rcv*x   [$bytes_to_read]$msg\n" if $trace_rcv;
                return ;   # .. _rcv will be called later
                           # when socket is readable again
            } else {
                last;
            }
        }
    }
    print STDERR "iCmsg.pm:_rcv*2   [$offset]$msg\n" if $trace_rcv;

  FINISH:
    if (length($msg) == 0) {
        $conn->disconnect();
    }
    if ($rcv_now) {
        return ($msg, $!);
    }
    push (@{$conn->{rcvQueue}}, $msg);	# queue received message just in case this interrupted rcvd_notification_proc
    my $rq = $conn->{rcvQueue};
    if ($conn->{rcvBusy}++) {
	my $m = $rq->[0];
	print STDERR "iCmsg.pm:_rcv*0       $m	rcvBusy $conn->{rcvBusy}\n" if $trace_rcv;
	return 1;			# rcvd_notification_proc not complete - $msg is queued out of turn
    }
    while (@$rq) {			# process all queued received messages in reception order
	my $msg = shift @$rq;
	print STDERR "iCmsg.pm:_rcv*3       $msg\n" if $trace_rcv;
	&{$conn->{rcvd_notification_proc}}($conn, $msg, $!);
    }
    print STDERR "iCmsg.pm:_rcv*end		rcvBusy $conn->{rcvBusy}\n" if $trace_rcv and $conn->{rcvBusy} > 1;
    $conn->{rcvBusy} = 0;		# later queued $msg picked up in above loop
}

sub _new_client {
    my $sock = $main_socket->accept();
    setsockopt($sock, IPPROTO_TCP, TCP_NODELAY, $nagle) if defined $nagle;	# inhibit Nagle's algorithm
    my $conn = bless {			# Reference to $conn{sock} and $conn{state}
        'sock' =>  $sock,
        'state' => 'connected'
    }, $g_pkg;				# CLASSNAME $g_pkg from new_server
    my $rcvd_notification_proc =
        &$g_login_proc ($conn, $sock->peerhost(), $sock->peerport());
    if ($rcvd_notification_proc) {
        $conn->{rcvd_notification_proc} = $rcvd_notification_proc;
        my $callback = sub {_rcv($conn,0)};
        set_event_handler ($sock, "read" => $callback);
    } else {  # Login failed
        $conn->disconnect();
    }
    print STDERR "iCmsg.pm:_new_client\n" if $trace_rcv or $trace_send;
}

#----------------------------------------------------
# Event loop routines used by both client and server

sub set_event_handler {
    shift unless ref($_[0]); # shift if first arg is package name
    my ($handle, %args) = @_;
    my $callback;
    if (exists $args{'write'}) {
        $callback = $args{'write'};
        if ($callback) {
            $wt_callbacks{$handle} = $callback;
            $wt_handles->add($handle);
        } else {
            delete $wt_callbacks{$handle};
            $wt_handles->remove($handle);
        }
    }
    if (exists $args{'read'}) {
        $callback = $args{'read'};
        if ($callback) {
            $rd_callbacks{$handle} = $callback;
            $rd_handles->add($handle);
        } else {
            delete $rd_callbacks{$handle};
            $rd_handles->remove($handle);
       }
    }
}

########################################################################
#
#	Modification John Wulff 2000.05.27
#	select time_out is 0 if loop_count is defined
#	else undef - which maintains previous functionality
#
#	if loop_count has a fractional part interpret it as
#	the time_out value and set loop_count to 1 - useful for
#	timed message loops
#
########################################################################

sub event_loop {
    my ($pkg, $loop_count) = @_; # event_loop(1) to process events once
    my ($r, $w, $rset, $wset);
    my $time_out = $loop_count;
    if (defined($loop_count)) {
	if ($time_out == int $time_out) {
	    $time_out = 0;
	} else {
	    $loop_count = 1;
	}
    }
    while (1) {
        # Quit the loop if no handles left to process
        last unless ($rd_handles->count() || $wt_handles->count());
        ($rset, $wset) =
            IO::Select->select ($rd_handles, $wt_handles, undef, $time_out);
        foreach $r (@$rset) {
            &{$rd_callbacks{$r}} ($r) if exists $rd_callbacks{$r};
        }
        foreach $w (@$wset) {
            &{$wt_callbacks{$w}}($w) if exists $wt_callbacks{$w};
        }
        if (defined($loop_count)) {
            last unless --$loop_count;
        }
    }
}

1;

__END__


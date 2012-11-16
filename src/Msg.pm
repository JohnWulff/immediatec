# $Id: Msg.pm,v 1.4 2012/11/08 06:48:40 jw Exp $

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
########################################################################

package Msg;
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
my $nagle;

BEGIN {
    # Checks if blocking is supported
    eval {
        require POSIX; POSIX->import(qw (F_GETFL F_SETFL O_NONBLOCK EAGAIN));
    };
    $blocking_supported = ($@) ? 0 : 1;	# executed before my $blocking_supported;
}

#-----------------------------------------------------------------
# Inhibit Nagle's algorithm (see in the first header)
sub inhibit_nagle {
    my ($pkg, $flag) = @_;
    $nagle = $flag;		# 0 is active, 1 is inhibited
}

#-----------------------------------------------------------------
# Send side routines
sub connect {
    my ($pkg, $to_host, $to_port,$rcvd_notification_proc) = @_;

    # Create a new internet socket

    my $sock = IO::Socket::INET->new (
                                      PeerAddr => $to_host,
                                      PeerPort => $to_port,
                                      Proto    => 'tcp',
                                      Reuse    => 1);

    return undef unless $sock;

    setsockopt($sock, IPPROTO_TCP, TCP_NODELAY, $nagle) if defined $nagle;	# inhibit Nagle's algorithm
    # Create a connection end-point object
    my $conn = bless {
        sock                   => $sock,
        rcvd_notification_proc => $rcvd_notification_proc,
    }, $pkg;

    if ($rcvd_notification_proc) {
        my $callback = sub {_rcv($conn, 0)};
        set_event_handler ($sock, "read" => $callback);
    }
    $conn;
}

sub disconnect {
    my $conn = shift;
    my $sock = delete $conn->{sock};
    return unless defined($sock);
    set_event_handler ($sock, "read" => undef, "write" => undef);
    close($sock);
}

sub send_now {
    my ($conn, $msg) = @_;
    _enqueue ($conn, $msg);
    $conn->_send (1); # 1 ==> flush
}

sub send_later {
    my ($conn, $msg) = @_;
    _enqueue($conn, $msg);
    my $sock = $conn->{sock};
    return unless defined($sock);
    set_event_handler ($sock, "write" => sub {$conn->_send(0)});
}

sub _enqueue {
    my ($conn, $msg) = @_;
    # prepend length (encoded as network long)
    my $len = length($msg);
    $msg = pack ('N', $len) . $msg;
    push (@{$conn->{queue}}, $msg);
}

sub _send {
    my ($conn, $flush) = @_;
    my $sock = $conn->{sock};
    return unless defined($sock);
    my ($rq) = $conn->{queue};

    # If $flush is set, set the socket to blocking, and send all
    # messages in the queue - return only if there's an error
    # If $flush is 0 (deferred mode) make the socket non-blocking, and
    # return to the event loop only after every message, or if it
    # is likely to block in the middle of a message.

    $flush ? $conn->set_blocking() : $conn->set_non_blocking();
    my $offset = (exists $conn->{send_offset}) ? $conn->{send_offset} : 0;

    while (@$rq) {
        my $msg            = $rq->[0];
        my $bytes_to_write = length($msg) - $offset;
        my $bytes_written  = 0;
        while ($bytes_to_write) {
            $bytes_written = syswrite ($sock, $msg,
                                       $bytes_to_write, $offset);
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
                    return 0; # fail. Message remains in queue ..
                }
            }
            $offset         += $bytes_written;
            $bytes_to_write -= $bytes_written;
        }
        delete $conn->{send_offset};
        $offset = 0;
        shift @$rq;
        last unless $flush; # Go back to select and wait
                            # for it to fire again.
    }
    # Call me back if queue has not been drained.
    if (@$rq) {
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
   # For more meaningful handling of send errors, subclass Msg and
   # rebless $conn.
   my ($conn, $err_msg) = @_;
   warn "Error while sending: $err_msg \n";
   set_event_handler ($conn->{sock}, "write" => undef);
}

#-----------------------------------------------------------------
# Receive side routines

my ($g_login_proc,$g_pkg);
my $main_socket = 0;
sub new_server {
    @_ == 4 || die "Msg->new_server (myhost, myport, login_proc)\n";
    my ($pkg, $my_host, $my_port, $login_proc) = @_;

    $main_socket = IO::Socket::INET->new (
                                          LocalAddr => $my_host,
                                          LocalPort => $my_port,
                                          Listen    => 5,
                                          Proto     => 'tcp',
                                          Reuse     => 1);
    die "Could not create socket: $! \n" unless $main_socket;
    set_event_handler ($main_socket, "read" => \&_new_client);
    $g_login_proc = $login_proc; $g_pkg = $pkg;
}

sub rcv_now {
    my ($conn) = @_;
    my ($msg, $err) = _rcv ($conn, 1); # 1 ==> rcv now
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
                return ;   # .. _rcv will be called later
                           # when socket is readable again
            } else {
                last;
            }
        }
    }

  FINISH:
    if (length($msg) == 0) {
        $conn->disconnect();
    }
    if ($rcv_now) {
        return ($msg, $!);
    } else {
        &{$conn->{rcvd_notification_proc}}($conn, $msg, $!);
    }
}

sub _new_client {
    my $sock = $main_socket->accept();
    setsockopt($sock, IPPROTO_TCP, TCP_NODELAY, $nagle) if defined $nagle;	# inhibit Nagle's algorithm
    my $conn = bless {
        'sock' =>  $sock,
        'state' => 'connected'
    }, $g_pkg;
    my $rcvd_notification_proc =
        &$g_login_proc ($conn, $sock->peerhost(), $sock->peerport());
    if ($rcvd_notification_proc) {
        $conn->{rcvd_notification_proc} = $rcvd_notification_proc;
        my $callback = sub {_rcv($conn,0)};
        set_event_handler ($sock, "read" => $callback);
    } else {  # Login failed
        $conn->disconnect();
    }
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
    my ($conn, $r, $w, $rset, $wset);
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


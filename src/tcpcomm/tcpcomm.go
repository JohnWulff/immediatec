/********************************************************************
 *
 *	Copyright (C) 2021  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	Package tcpcomm
 *	TCP/IC communication support
 *
 *  Read and Write buffers match the buffers used in Perl module msg.pm
 *  The contents of the buffers consists of a 4 byte length (BigEndian)
 *  followed by the number of bytes specified by that length.
 *  The maximum number of bytes is 1400 for TCP.
 *
 *******************************************************************/

package tcpcomm

import (
	"io"
	"net"
	"encoding/binary"
	"fmt"
)

const ID_tcpcomm_go = "$Id: tcpcomm.go 1.2 $"
const (
    REPLY	= 1400-4		// max size of TCP reply, in bytes - room for 4 byte length
)

/********************************************************************
 *
 *  Read a TCP message into Read buffer rbuf
 *  Local []byte slices lbuf and rbuf are on a different stack for
 *  each call from a different goroutine and hence concurrency safe
 *
 *******************************************************************/

func Read(conn net.Conn) ([]byte, error) {
    var err error
    lbuf := make([]byte, 4)
    if _, err = io.ReadFull(conn, lbuf); err != nil {
	return lbuf, err
    }
    rbuf := make([]byte, int(binary.BigEndian.Uint32(lbuf)))
    _, err = io.ReadFull(conn, rbuf)
    return rbuf, err
}

/********************************************************************
 *
 *  Write a TCP message of a specified length from Write buffer pointed
 *  to by wbufP. The buffer is declared and filled in the calling
 *  goroutine, which makes it concurrency safe
 *
 *******************************************************************/

type lengthError struct{}

func (m *lengthError) Error() string {
    return fmt.Sprintf("tcpcomm.Write: message length is too long (> %d)", REPLY+4)
}

func Write(conn net.Conn, wbufP *[]byte) (error) {
    length := len(*wbufP)
    if length > REPLY+4 {
	return &lengthError{};
    }
    binary.BigEndian.PutUint32((*wbufP)[:4], uint32(length-4))
    _, err := conn.Write(*wbufP)
    return err
}

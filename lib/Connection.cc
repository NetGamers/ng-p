/**
 * Connection.cc
 * Author: Daniel Karrels (dan@karrels.com)
 * Copyright (C) 2002 Daniel Karrels <dan@karrels.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/socket.h>
#include	<netdb.h>

#include	<string>

#include	<cstring>

#include	"Connection.h"
#include	"Buffer.h"

namespace gnuworld
{

// Allocate these static variables in class Connection
const Connection::flagType	Connection::F_PENDING = 0x01 ;
const Connection::flagType	Connection::F_CONNECTED = 0x02 ;
const Connection::flagType	Connection::F_INCOMING = 0x04 ;
const Connection::flagType	Connection::F_LISTEN = 0x08 ;

using std::string ;

// Simply initialize the object
Connection::Connection( const string& _hostname,
	const unsigned short int _remotePort,
	const char _delimiter )
: hostname( _hostname ),
	localPort( 0 ),
	remotePort( _remotePort ),
	inputBuffer( string(), _delimiter ),
	outputBuffer( string(), _delimiter ),
	IP( string() ),
	sockFD( -1 ),
	flags( F_PENDING )
{
memset( &addr, 0, sizeof( struct sockaddr_in ) ) ;
}

Connection::Connection( const char _delimiter )
: localPort( 0 ),
	remotePort( 0 ),
	inputBuffer( string(), _delimiter ),
	outputBuffer( string(), _delimiter ),
	sockFD( -1 ),
	flags( F_PENDING )
{
memset( &addr, 0, sizeof( struct sockaddr_in ) ) ;
}

Connection::~Connection()
{
/* No work to be done, no heap space allocated */
}

} // namespace gnuworld

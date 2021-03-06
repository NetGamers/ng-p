/**
 * iClient.cc
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

#include	<new>
#include	<string>
#include	<iostream>
#include	<ctime>
#include	<map>
#include	<algorithm>

#include	<cassert>

#include	"iClient.h"
#include	"client.h"
#include	"Numeric.h"
#include	"ip.h"

namespace gnuworld
{

using std::string ;
using std::map ;

const iClient::modeType iClient::MODE_OPER        = 0x01 ;
const iClient::modeType iClient::MODE_WALLOPS     = 0x02 ;
const iClient::modeType iClient::MODE_INVISIBLE   = 0x04 ;
const iClient::modeType iClient::MODE_DEAF        = 0x08 ;
const iClient::modeType iClient::MODE_SERVICES    = 0x10 ;
const iClient::modeType iClient::MODE_REGISTERED  = 0x20 ;
const iClient::modeType iClient::MODE_HIDDEN_HOST = 0x40 ;

iClient::iClient( const unsigned int& _uplink,
	const string& _yxx,
	const string& _nickName,
	const string& _userName,
	const string& _hostBase64,
	const string& _insecureHost,
	const string& _realInsecureHost,
	const string& _mode,
	const string& _account,
	const string& _description,
	const time_t& _connectTime )
: intYY( _uplink ),
	nickName( _nickName ),
	userName( _userName ),
	IP( xIP( _hostBase64, true ).GetLongIP() ),
	insecureHost( _insecureHost ),
	realInsecureHost( _realInsecureHost ),
	description( _description),
	connectTime( _connectTime ),
	mode( 0 ),
	account( _account )
{
if( 5 == _yxx.size() )
	{
	// n2k, yyxxx
	intXXX = base64toint( _yxx.c_str() + 2, 3 ) ;
	intYYXXX = base64toint( _yxx.c_str(), 5 ) ;

	charYY[ 0 ] = _yxx[ 0 ] ;
	charYY[ 1 ] = _yxx[ 1 ] ;
	charYY[ 2 ] = 0 ;
	charXXX[ 0 ] = _yxx[ 2 ] ;
	charXXX[ 1 ] = _yxx[ 3 ] ;
	charXXX[ 2 ] = _yxx[ 4 ] ;
	charXXX[ 3 ] = 0 ;
	}
else
	{
	// yxx
	intXXX = base64toint( _yxx.c_str() + 1, 2 ) ;
	intYYXXX = base64toint( _yxx.c_str(), 3 ) ;

	charYY[ 0 ] = _yxx[ 0 ] ;
	charYY[ 1 ] = 0 ;
	charXXX[ 0 ] = _yxx[ 1 ] ;
	charXXX[ 1 ] = _yxx[ 2 ] ;
	charXXX[ 2 ] = 0 ;
	}

setModes( _mode ) ;
customDataMap = 0 ;
}

iClient::~iClient()
{
delete customDataMap ;
customDataMap = 0 ;
}

void iClient::setModes( const string& newModes )
{
for( string::size_type i = 0 ; i < newModes.size() ; i++ )
	{
	switch( newModes[ i ] )
		{
		case 'o':
		case 'O':
			mode |= MODE_OPER ;
			break ;
		case 'i':
		case 'I':
			mode |= MODE_INVISIBLE ;
			break ;
		case 'w':
		case 'W':
			mode |= MODE_WALLOPS ;
			break ;
		case 'k':
		case 'K':
			mode |= MODE_SERVICES ;
			break ;
		case 'd':
		case 'D':
			mode |= MODE_DEAF ;
			break ;
		case 'r':
		case 'R':
			mode |= MODE_REGISTERED ;
			if (isModeR() && isModeX()) setHiddenHost();
			break ;
		case 'x':
		case 'X':
			mode |= MODE_HIDDEN_HOST ;
			if (isModeR() && isModeX()) setHiddenHost();
			break ;
		default:
			// Unknown mode
			break ;
		} // switch
	} // for
} // setModes()

bool iClient::removeChannel( Channel* theChan )
{
// No need to remove all duplicate channel instances, since
// addChannel() guarantees that none exist
for( channelIterator ptr = channelList.begin() ;
	ptr != channelList.end() ; ++ptr )
	{
	if( *ptr == theChan )
		{
		channelList.erase( ptr ) ;
		return true  ;
		}
	}

// Channel not found
return false ;
}

const string iClient::getCharModes() const
{
string retMe( "+" ) ;

if( mode & MODE_OPER )		retMe += 'o' ;
if( mode & MODE_WALLOPS )	retMe += 'w' ;
if( mode & MODE_INVISIBLE )	retMe += 'i' ;
if( mode & MODE_DEAF )		retMe += 'd' ;
if( mode & MODE_SERVICES )	retMe += 'k' ;
if( mode & MODE_REGISTERED )	retMe += 'r' ;
if( mode & MODE_HIDDEN_HOST )	retMe += 'x' ;

return retMe ;
}

bool iClient::setCustomData( xClient* theClient, void* data )
{
assert( theClient != 0 ) ;

// Is the customDataMap empty?
if( NULL == customDataMap )
	{
	// Yes, go ahead and allocate it
	customDataMap = new (std::nothrow) customDataMapType ;
	assert( customDataMap != 0 ) ;
	}

// Is this xClient already using its customDataMap slot?
if( customDataMap->find( theClient ) != customDataMap->end() )
	{
	// Yes, return failure
	return false ;
	}

// Attempt to add this data into the customDataMap.  Return the
// success/failure of the operation
return customDataMap->insert( customDataMapType::value_type(
	theClient, data ) ).second ;
}

void* iClient::getCustomData( xClient* theClient ) const
{
assert( theClient != 0 ) ;

// Has the customDataMap been allocated?
if( NULL == customDataMap )
	{
	// Nope, return NULL
	return 0 ;
	}

// Attempt to find this xClient's data element
customDataMapType::const_iterator ptr =
	customDataMap->find( theClient ) ;

// Did we find the element?
if( ptr == customDataMap->end() )
	{
	// Nope, return NULL
	return 0 ;
	}

// Found it, go ahead and return this xClient's data element
return ptr->second ;
}

void* iClient::removeCustomData( xClient* theClient )
{
assert( theClient != 0 ) ;

// Has the customDataMap been allocated?
if( NULL == customDataMap )
	{
	// No, return NULL
	return 0 ;
	}

// Attempt to find this xClient's data element
customDataMapType::iterator ptr = customDataMap->find( theClient ) ;

// Did we find it?
if( ptr == customDataMap->end() )
	{
	// Nope, return NULL
	return 0 ;
	}

// Yes, we found this xClient's data element.  Remove it from
// the customDataMap
customDataMap->erase( ptr ) ;

// The customDataMap may now be empty, but let it remain allocated.
// Return the data element
return ptr->second ;
}

bool iClient::addChannel( Channel* theChan )
{

// This method is public, make sure the pointer is valid
assert( theChan != 0 ) ;

if( std::find( channelList.begin(), channelList.end(), theChan )
	!= channelList.end() )
	{
	// Channel already found
	return false ;
	}

// Add the channel to the channelList
channelList.push_front( theChan ) ;

return true ;
}

} // namespace gnuworld

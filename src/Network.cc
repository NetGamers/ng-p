/**
 * Network.cc
 * Author: Daniel Karrels dan@karrels.com
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
#include	<iostream>
#include	<string>
#include	<list>
#include	<algorithm>

#include	<cassert>

#include	"Network.h"
#include	"ELog.h"
#include	"iClient.h"
#include	"iServer.h"
#include	"Channel.h"
#include	"client.h"
#include	"misc.h"
#include	"Numeric.h"
#include	"match.h"
#include	"StringTokenizer.h"
#include	"ip.h"
#include	"defs.h"

namespace gnuworld
{

using std::map ;
using std::string ;
using std::endl ;
using std::list ;
using std::unary_function ;

xNetwork::xNetwork()
{}

xNetwork::~xNetwork()
{
// TODO: Establish protocol here about who deletes all clients/server/etc
}

bool xNetwork::addClient( iClient* newClient )
{
assert( NULL != newClient ) ;

if( !numericMap.insert( numericMapType::value_type(
	newClient->getIntYYXXX(), newClient ) ).second )
	{
	elog	<< "xNetwork::addClient> Insert into numericMap failed"
		<< endl ;
	return false ;
	}

//elog	<< "xNetwork::addClient> Added client: "
//	<< *newClient
//	<< endl ;

addNick( newClient ) ;

return true ;
}

// xClients are local to this server, they
// must be handled a little differently.
// This method needs to assign numerics
// to the client -- the client doesn't
// already have a numeric.
bool xNetwork::addClient( xClient* newClient )
{
assert( NULL != newClient ) ;

if( findLocalNick( newClient->getNickName() ) != 0 )
	{
	// Nickname already exists on this serer
	return false ;
	}

// First, find a new numeric for this client
xClientVectorType::size_type pos = 0 ;
for( ; pos < localClients.size() ; pos++ )
	{
	if( NULL == localClients[ pos ] )
		{
		break ;
		}
	}

// Were there any spots available?
if( pos >= localClients.size() )
	{
	// Nope, make a new one
	localClients.push_back( NULL ) ;
	}

// pos is now the index of the next empty
// slot in the localClients vector
localClients[ pos ] = newClient ;

// Give the new client a numeric
newClient->setIntXXX( pos ) ;

return true ;
}

bool xNetwork::addServer( iServer* newServer )
{
assert( newServer != NULL ) ;

if( !serverMap.insert( serverMapType::value_type(
	newServer->getIntYY(), newServer ) ).second )
	{
	elog	<< "xNetwork::addServer> Insert into serverMap failed"
		<< endl ;
	return false ;
	}
return true ;
}

bool xNetwork::addChannel( Channel* theChan )
{
assert( theChan != NULL ) ;

//elog	<< "Adding channel: "
//	<< *theChan
//	<< endl ;

return channelMap.insert(
	channelMapType::value_type( theChan->getName(),
		theChan ) ).second ;
}

iClient* xNetwork::findClient( const unsigned int& intYY,
	const unsigned int& intXXX ) const
{
unsigned int intYYXXX = combinebase64int( intYY, intXXX ) ;

numericMapType::const_iterator ptr = numericMap.find( intYYXXX ) ;
if( ptr == numericMap.end() )
	{
	return 0 ;
	}
return ptr->second ;
}

iClient* xNetwork::findClient( const string& yyxxx ) const
{
unsigned int intYYXXX = base64toint( yyxxx.c_str(), yyxxx.size() ) ;
numericMapType::const_iterator ptr = numericMap.find( intYYXXX ) ;
if( ptr == numericMap.end() )
	{
	return 0 ;
	}
return ptr->second ;
}

iClient* xNetwork::findNick( const string& nick ) const
{
nickMapType::const_iterator ptr = nickMap.find( nick ) ;
if( ptr == nickMap.end() )
	{
	return 0 ;
	}
return ptr->second ;
}

xClient* xNetwork::findLocalClient( const unsigned int& YY,
	const unsigned int& XXX ) const
{
// Nothing fancy for now, just linear iteration
for( xClientVectorType::size_type i = 0 ; i < localClients.size() ; i++ )
	{
	if( NULL == localClients[ i ] )
		{
		continue ;
		}
	if( YY == localClients[ i ]->getIntYY() &&
		XXX == localClients[ i ]->getIntXXX() )
		{
		return localClients[ i ] ;
		}
	}
return 0 ;
}

xClient* xNetwork::findLocalClient( const string& yyxxx ) const
{
unsigned int	yy = 0,
		xxx = 0 ;

if( yyxxx.size() == 5 )
	{
	// n2k
	yy = base64toint( yyxxx.c_str(), 2 ) ;
	xxx = base64toint( yyxxx.c_str() + 2, 3 ) ;
	}
else
	{
	// yxx
	yy = base64toint( yyxxx.c_str(), 1 ) ;
	xxx = base64toint( yyxxx.c_str() + 1, 2 ) ;
	}

return findLocalClient( yy, xxx ) ;
}

xClient* xNetwork::findLocalNick( const string& nickName ) const
{
for( xClientVectorType::size_type i = 0 ; i < localClients.size() ; i++ )
	{
	if( NULL == localClients[ i ] )
		{
		continue ;
		}
	if( !strcasecmp( nickName, localClients[ i ]->getNickName() ) )
		{
		return localClients[ i ] ;
		}
	}
return 0 ;
}

iServer* xNetwork::findServer( const string& YY ) const
{
return findServer( base64toint( YY.c_str(), YY.size() ) ) ;
}

iServer* xNetwork::findServer( const unsigned int& YY ) const
{
serverMapType::const_iterator ptr = serverMap.find( YY ) ;
if( ptr == serverMap.end() )
	{
	return 0 ;
	}
return ptr->second ;
}

iServer* xNetwork::findServerName( const string& name ) const
{
for( serverMapType::const_iterator ptr = serverMap.begin(),
	endPtr = serverMap.end() ; ptr != endPtr ; ++ptr )
	{
	if( !strcasecmp( ptr->second->getName(), name ) )
		{
		return ptr->second ;
		}
	}
return 0 ;
}

iServer* xNetwork::findExpandedServerName( const string& name ) const
{
for( serverMapType::const_iterator ptr = serverMap.begin(),
	endPtr = serverMap.end() ; ptr != endPtr ; ++ptr )
	{
	if( !match( ptr->second->getName(), name ) )
		{
		return ptr->second ;
		}
	}
return 0 ;
}

Channel* xNetwork::findChannel( const string& name ) const
{
channelMapType::const_iterator ptr = channelMap.find( name ) ;
if( ptr == channelMap.end() )
	{
//	elog	<< "xNetwork::findChannel> Failed to find: "
//		<< name << endl ;
	return 0 ;
	}
return ptr->second ;
}

iClient* xNetwork::removeClient( const unsigned int& intYY,
	const unsigned int& intXXX )
{
return removeClient( combinebase64int( intYY, intXXX ) ) ;
}

iClient* xNetwork::removeClient( const string& yyxxx )
{
return removeClient( base64toint( yyxxx.c_str(), 5 ) ) ;
}

iClient* xNetwork::removeClient( const unsigned int& intYYXXX )
{
numericMapType::iterator ptr = numericMap.find( intYYXXX ) ;
if( ptr == numericMap.end() )
	{
	elog	<< "xNetwork::removeClient> Unable to find client "
		<< "numeric: "
		<< intYYXXX
		<< endl ;
	return 0 ;
	}

//elog	<< "xNetwork::removeClient> Removing client: "
//	<< *(ptr->second)
//	<< endl ;

numericMap.erase( ptr ) ;

iClient* retMe = ptr->second ;
removeNick( retMe->getNickName() ) ;

// Remove all associations between client->channel
iClient::channelIterator chanPtr = retMe->channels_begin() ;
while( chanPtr != retMe->channels_end() )
	{
//	elog	<< "xNetwork::removeClient> Removing user "
//		<< retMe->getCharYYXXX()
//		<< " from channel "
//		<< (*chanPtr)->getName()
//		<< endl ;

	ChannelUser* theChanUser = (*chanPtr)->removeUser( retMe ) ;
	if( NULL == theChanUser )
		{
		elog	<< "xNetwork::removeClient> Unable to find "
			<< "ChannelUser in channel "
			<< (*chanPtr)->getName()
			<< ", for iClient: "
			<< *retMe
			<< endl ;
		}
	delete theChanUser ; theChanUser = 0 ;

	if( (*chanPtr)->empty() )
		{
//		elog	<< "xNetwork::removeClient> Removing channel "
//			<< (*chanPtr)->getName()
//			<< endl ;

		delete removeChannel( (*chanPtr)->getName() ) ;
		}
	++chanPtr ;
	}

retMe->clearChannels() ;
return retMe ;
}

iClient* xNetwork::removeClient( iClient* theClient )
{
assert( theClient != 0 ) ;

return removeClient( theClient->getIntYY(), theClient->getIntXXX() ) ;
}

xClient* xNetwork::removeLocalClient( xClient* theClient )
{
assert( theClient != 0 ) ;

for( xClientVectorType::size_type i = 0 ; i < localClients.size() ; ++i )
	{
	if( localClients[ i ] == theClient )
		{
		localClients[ i ] = 0 ;
		return theClient ;
		}
	}
return 0 ;
}

void xNetwork::removeNick( const string& nick )
{
nickMap.erase( nick ) ;
}

/**
 * Remove a server by numeric from the network tables.
 * This will deallocate any clients on that server.
 * A pointer to the server being removed is returned.
 * The memory space for this iServer is NOT deallocated
 * by this method, so it's up to the caller to deallocate
 * any heap memory.
 */
iServer* xNetwork::removeServer( const unsigned int& YY,
	bool postEvent )
{

// Attempt to find the server being removed
serverMapType::iterator serverIterator = serverMap.find( YY ) ;

// Did we find the server?
if( serverIterator == serverMap.end() )
	{
	// Nope, log an error
	elog	<< "xNetwork::removeServer> Failed to find server "
		<< "numeric: "
		<< YY
		<< endl ;

	// Let the caller know that the remove failed
	return 0 ;
	}

// Grab a pointer to the iServer for convenience and readability
iServer* serverPtr = serverIterator->second ;

// Remove the server from the internal table
serverMap.erase( serverIterator ) ;

// Verbose debugging information
//elog	<< "xNetwork::removeServer> Removing server: "
//	<< *serverPtr
//	<< endl ;

// Walk through the numericMap looking for clients which are on
// the server being removed.
// This algorithm is O(N) :(
for( numericMapType::iterator clientIterator = numericMap.begin() ; 
	clientIterator != numericMap.end() ; )
	{
	// Is this client on the server that is being removed?
	if( YY != clientIterator->second->getIntYY() )
		{
		// Nope, move to next client
		++clientIterator ;

		// Skip rest of the loop
		continue ;
		}

	// Let removeClient() handle:
	// - Removing the client<->channel interactions
	// - Removing the channel itself, if empty
	// - Removing the client from the internal tables
	//
	// The removal of an element from a hash_map<> invalidates
	// the iterator (or perhaps all iterators, still searching
	// for documentation on this).  However, hash_map::erase()
	// does not return an updated iterator, so we have to make
	// a little work around here to make sure we keep a valid
	// iterator.

	// nextIterator is the iterator past the current iterator
	numericMapType::iterator nextIterator( clientIterator ) ;
	++nextIterator ;

	// Remove the iClient and perform functions described above.
	// This method calls numericMap.erase(), and so after the
	// call to removeClient(), the clientIterator is no longer
	// valid.
	iClient* theClient = removeClient( clientIterator->second ) ;

	// Point the clientIterator to the nextIterator
	clientIterator = nextIterator ;

	// Should we post this as an EVT_QUIT event?
	// It seems that this may add unneeded complexity in handling
	// requests of the client modules since they can call methods
	// which may attempt to access the server to which this client
	// is/was connected (the server is no longer in the tables)
	if( postEvent )
		{
		// Yes, post the event
		theServer->PostEvent( EVT_QUIT,
			static_cast< void* >( theClient ) ) ;
		}

	// Be sure to deallocate the iClient's allocated heap space
	delete theClient ;
	}

// Return the server being removed
return serverPtr ;
}

iServer* xNetwork::removeServer( const string& YY )
{
return removeServer( base64toint( YY.c_str() ) ) ;
}

iServer* xNetwork::removeServerName( const string& name )
{
iServer* serverPtr = findServerName( name ) ;
if( NULL == serverPtr )
	{
	return NULL ;
	}
return removeServer( serverPtr->getIntYY() ) ;
}

Channel* xNetwork::removeChannel( const string& name )
{
channelMapType::iterator ptr = channelMap.find( name ) ;
if( ptr == channelMap.end() )
	{
	elog	<< "xNetwork::removeChannel> Failed to find channel: "
		<< name
		<< endl ;
	return 0 ;
	}
channelMap.erase( ptr ) ;
return ptr->second ;
}

Channel* xNetwork::removeChannel( const Channel* theChan )
{
assert( theChan != 0 ) ;

return removeChannel( theChan->getName() ) ;
}

void xNetwork::rehashNick( const string& yyxxx,
	const string& newNick )
{
iClient* theClient = findClient( yyxxx ) ;
if( NULL == theClient )
	{
	elog	<< "xNetwork::rehashNick> Unable to find numeric: "
		<< yyxxx
		<< ", new nick: "
		<< newNick
		<< endl ;
	return ;
	}

// No need to keep track of the return value here,
// we already have it.
removeNick( theClient->getNickName() ) ;

// oldNick is used in the event posting
string oldNick = theClient->getNickName() ;

// Change the client's nickname
theClient->setNickName( newNick ) ;

// Add the client back to the nickname table
addNick( theClient ) ;

// Note that the user's numeric never changes,
// so no need to rehash numeric

// Go ahead and post this event
theServer->PostEvent( EVT_CHNICK,
	static_cast< void* >( theClient ),
	static_cast< void* >( &oldNick ) ) ;

// TODO: theServer->PostNickChange() --> OnNickChange()
}

void xNetwork::addNick( iClient* theClient )
{
// This is a protected method, theClient is guaranteed to
// be non-NULL.
if( !nickMap.insert( nickMapType::value_type(
	theClient->getNickName(), theClient ) ).second )
	{
	elog	<< "xNetwork::addNick> Failed to add nick: "
		<< theClient->getNickName()
		<< endl ;
	}
}

/**
 * Handle a netsplit.
 * This method removes and deallocated the server corresponding
 * to the numeric (intYY), and all of its leaf servers (and all
 * of their leaf servers, etc).
 * Our uplink server has its own numeric as its uplinkIntYY.
 * This method was once a disturbingly complicated recursive method.
 * It turned out that the previous implementation was not robust
 * to modifications made to internal data structure types.
 * Therefore, it has been greatly simplified here, to where even
 * someone like me can understand it.
 * The idea here is to use a simple recursive method to gather the
 * server numerics which need to be removed, and then perform the
 * removals in an iterative manner.
 * (intYY) is the server numeric of the server to be removed.
 * The caller of this method is responsible for posting the
 * EVT_NETBREAK event.
 */
void xNetwork::OnSplit( const unsigned int& intYY )
{

// yyVector will be used to hold server numerics of servers
// to be removed.
typedef vector< unsigned int > yyVectorType ;
yyVectorType yyVector ;

// Of course add the top level server to the list of servers
// to be removed.
yyVector.push_back( intYY ) ;

// Recursive method to find all leaf servers of intYY, and all
// of each of those servers' leaf servers.
// This is much simpler than having the entire OnSplit() method
// recursive.
findLeaves( yyVector, intYY ) ;

// yyVector should now have all leaf servers (if any) of intYY,
// each of which must be removed.

// Now iterate through the server numeric vector once more and
// remove the servers from the network data tables.
// Note that this will call removeServer() for each server,
// which will remove that server from the data tables, and also
// deallocate the server pointer.
// removeServer() will also remove all iClient's resident on that
// server (and deallocate them), and also remove any channels
// which are determined to be empty as a result of the netsplit.

for( yyVectorType::const_iterator yyIterator = yyVector.begin() ;
	yyIterator != yyVector.end() ; ++yyIterator )
	{
	// Obtain a pointer to the server in question for convenience
	// and readability
	iServer* removeMe = findServer( *yyIterator ) ;
	assert( removeMe != 0 ) ;

	// Generate some debugging information
//	elog	<< "xNetwork::OnSplit> Removing server: "
//		<< *removeMe
//		<< endl ;

	// Remove the server, its clients, any empty channels,
	// and post events for all of the above.
	delete removeServer( removeMe->getIntYY(), true ) ;
	}
}

/**
 * This is a simple recursive method which traverses the
 * serverMap looking for all leaves to server whose numeric
 * is uplinkIntYY.  Any server which is found to be a leaf
 * to uplinkIntYY is then passed into a recursive call to
 * findLeaves() for that particular leaf server, etc.
 * This method does not modify any data tables, it only
 * walks the serverMap table.
 */
void xNetwork::findLeaves( vector< unsigned int >& yyVector,
	const unsigned int uplinkIntYY ) const
{

// Begin our walk down the serverMap looking for leaf servers
// of uplinkIntYY.
for( serverMapType::const_iterator serverIterator = serverMap.begin() ;
	serverIterator != serverMap.end() ; ++serverIterator )
	{

	// Obtain a pointer to this iServer for convenience and
	// readability
	const iServer* serverPtr = serverIterator->second ;

	// Check to see if this server is our uplink, don't want
	// to remove that one :)
	if( theServer->getUplinkIntYY() == serverPtr->getIntYY() )
		{
		// It's our uplink.  Prevent infinite recursion here
		// by just ignoring this server (base case).

//		elog	<< "xServer::findLeaves> Found uplink"
//			<< endl ;
		}
	else if( serverPtr->getUplinkIntYY() == uplinkIntYY )
		{
		// serverPtr is a leaf to uplinkIntYY

		// Add serverPtr to the list of leaves to server
		// uplinkIntYY
		yyVector.push_back( serverPtr->getIntYY() ) ;

		// Call this method recursively to find leaves
		// of serverPtr.
		findLeaves( yyVector, serverPtr->getIntYY() ) ;

		} // else if()

	} // for()

} // findLeaves()

size_t xNetwork::serverList_size() const
{
return static_cast< size_t >( serverMap.size() ) ;
}

size_t xNetwork::clientList_size() const
{
return static_cast< size_t >( numericMap.size() ) ;
}

void xNetwork::foreach_xClient( xNetwork::fe_xClientBase f )
{
std::for_each( localClients.begin(), localClients.end(), f ) ;
}

size_t xNetwork::countClients( const iServer* serverPtr ) const
{
assert( theServer != 0 ) ;

// Server numeric for which to search
const unsigned int YY = serverPtr->getIntYY() ;

// The number of clients found for the given server
size_t numClients = 0 ;

for( numericMapType::const_iterator ptr = numericMap.begin(),
	endPtr = numericMap.end() ; ptr != endPtr ; ++ptr )
	{
	if( YY == ptr->second->getIntYY() )
		{
		++numClients ;
		}
	}
return numClients ;
}

list< const iClient* > xNetwork::matchHost( const string& wildHost ) const
{
list< const iClient* > retMe ;

for( numericMapType::const_iterator ptr = numericMap.begin(),
	endPtr = numericMap.end() ; ptr != endPtr ; ++ptr )
	{
	const iClient* clientPtr = ptr->second ;
	if( !match( wildHost, clientPtr->getInsecureHost() ) ||
		!match( wildHost,
			xIP( clientPtr->getIP() ).GetNumericIP() ) )
		{
		// Found a match
		retMe.push_back( clientPtr ) ;
		}
	}

return retMe ;
}

list< const iClient* > xNetwork::matchUserHost(
	const string& wildUserHost ) const
{

// Tokenize the wildUserHost into username and hostname
StringTokenizer st( wildUserHost, '@' ) ;

// Make sure there are exactly two tokens
if( st.size() != 2 )
	{
	// Invalid format of wildUserHost, return empty list
	return list< const iClient* >() ;
	}

// Get a list of matching hosts to that of wildUserHost
list< const iClient* > matchingHosts = matchHost( st[ 1 ] ) ;

// Were any found?
if( matchingHosts.empty() )
	{
	// No matching hosts found, return the empty list
	return matchingHosts ;
	}

// Create a list to return to the caller, create matchingHosts.size()
// empty slots to speed up memory allocation
list< const iClient* > retMe ;

// Iterate through the list of matching hostnames
for( list< const iClient* >::const_iterator ptr = matchingHosts.begin() ;
	ptr != matchingHosts.end() ; ++ptr )
	{
	// Does this iClient's username also match that of the
	// wildUserHost username?
	if( !match( st[ 0 ], (*ptr)->getUserName() ) )
		{
		// Found a matching username
		retMe.push_back( *ptr ) ;
		}
	}

// Return the list of matching username and hostnames
return retMe ;
}

size_t xNetwork::countMatchingUserHost( const string& wildUserHost ) const
{
return static_cast< size_t >( matchUserHost( wildUserHost ).size() ) ;
}

list< const iClient* > xNetwork::findHost( const string& hostName ) const
{
list< const iClient* > retMe ;

for( numericMapType::const_iterator ptr = numericMap.begin(),
	endPtr = numericMap.end() ; ptr != endPtr ; ++ptr )
	{
	const iClient* clientPtr = ptr->second ;

	if( !strcasecmp( hostName, clientPtr->getInsecureHost() ) )
		{
		// Found a match
		retMe.push_back( clientPtr ) ;
		}
	}

return retMe ;
}

size_t xNetwork::countMatchingHost( const string& wildHost ) const
{
return static_cast< size_t >( matchHost( wildHost ).size() ) ;
}

size_t xNetwork::countHost( const string& hostName ) const
{
return static_cast< size_t >( findHost( hostName ).size() ) ;
}

list< const iClient* > xNetwork::matchRealName( const string& realName ) const
{
list< const iClient* > retMe ;

for( numericMapType::const_iterator ptr = numericMap.begin(),
	endPtr = numericMap.end() ; ptr != endPtr ; ++ptr )
	{
	const iClient* clientPtr = ptr->second ;

	if( !match( realName, clientPtr->getDescription() ) )
		{
		retMe.push_back( clientPtr ) ;
		}
	}

return retMe;
}

} // namespace gnuworld

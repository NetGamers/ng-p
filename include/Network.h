/**
 * Network.h
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

#ifndef __NETWORK_H
#define __NETWORK_H

#include	<vector>
#include	<string>
#include	<map>
#include	<list>
#include	<algorithm>

#include	<ctime>

#include	"iServer.h"
#include	"iClient.h"
#include	"Channel.h"
#include	"client.h"
#include	"misc.h" // struct noCaseCompare

namespace gnuworld
{

using std::string ;
using std::vector ;
using std::map ;
using std::list ;
using std::unary_function ;

class xServer ;

/**
 * This class is responsible for storing the network's iClient's,
 * iServer's, and local xClient's.  This is done using several
 * tables, some of which are a bit non-trivial to understand at
 * first, but are actually very simple.
 * This is a big container class, mutated primarily by the
 * xServer instance.
 */
class xNetwork
{

private:

	/**
	 * This is the type of vector for storing
	 * local (services) clients.
	 */
	typedef vector< xClient* > xClientVectorType ;

	/**
	 * This is the type used to store the network Channel
	 * instances.
	 */
	typedef map< string, Channel*, noCaseCompare > channelMapType ;

	/**
	 * This is the type used to store the nick name/iClient* pairs.
	 */
	typedef map< string, iClient*, noCaseCompare > nickMapType ;

	/**
	 * The type used to store information about iClient's,
	 * keyed by integer numeric.
	 */
	typedef map< unsigned int, iClient* > numericMapType ;

	/**
	 * The type used to store information about iServer's,
	 * keyed by integer numeric.
	 */
	typedef map< unsigned int, iServer* > serverMapType ;

public:

	/**
	 * The default constructor handles all of the default
	 * allocation and initialization.
	 */
	xNetwork() ;

	/**
	 * The default destructor will deallocate all objects
	 * that are not explicitly removed from this structure.
	 * Therefore, it is highly recommend that the application
	 * programmer iterate through this structure and remove
	 * all items before destroying this structure.
	 */
	virtual ~xNetwork() ;

	/* Addition methods
	 * These methods return true if addition is successful,
	 * false otherwise.
	 */

	/**
	 * Add a new iClient to the network.  This will also handle
	 * any additions into the nickname table, if required.
	 * Returns false if the addition fails.
	 */
	virtual bool		addClient( iClient* ) ;

	/**
	 * Add a services client to the network table.
	 * This is stored in the table of local clients.
	 * Returns false if the addition fails.
	 */
	virtual bool		addClient( xClient* ) ;

	/**
	 * Add a new server to the network table.
	 * Returns false if the addition fails.
	 */
	virtual bool		addServer( iServer* ) ;

	/**
	 * Add a new channel to the network table.
	 * Returns false if the addition fails.
	 */
	virtual bool		addChannel( Channel* ) ;

	/*
	 * All nickname based searches are case insensitive.
	 */

	/**
	 * Find a remote client given the client's integer numeric.
	 * Returns NULL if not found.
	 */
	virtual iClient*	findClient( const unsigned int& YY,
					const unsigned int& XXX ) const ;

	/**
	 * Find a remote client given the client's character numeric.
	 * Returns NULL if not found.
	 */
	virtual iClient*	findClient( const string& yyxxx ) const ;

	/**
	 * Retrieve a pointer to an iClient given its nick name.
	 * Returns NULL if not found.
	 */
	virtual iClient*	findNick( const string& nickName ) const ;

	/**
	 * Find a local (services) client by its numeric.
	 * Returns NULL if not found.
	 */
	virtual xClient*	findLocalClient( const unsigned int& YY,
					const unsigned int& XXX ) const ;

	/**
	 * Find a local (services) client by its character numeric.
	 * Returns NULL if not found.
	 */
	virtual xClient*	findLocalClient( const string& yyxxx ) const ;

	/**
	 * Find a local (services) client by its case-insensitive nickname.
	 * Returns NULL if not found.
	 */
	virtual xClient*	findLocalNick( const string& nickName ) const ;

	/**
	 * Find a remote server by its integer numeric.
	 * Returns NULL if not found.
	 */
	virtual iServer*	findServer( const unsigned int& YY ) const ;

	/**
	 * Find a remote server by its character numeric.
	 * Returns NULL if not found.
	 */
	virtual iServer*	findServer( const string& YY ) const ;

	/**
	 * Find a remote server by its case-insensitive name.
	 * Returns NULL if not found.
	 */
	virtual iServer*	findServerName( const string& name ) const ;

	/**
	 * Find a remote server by a wildmask name.
	 * Returns NULL if not found.
	 */
	virtual iServer*	findExpandedServerName( const string& name )
					const ;

	/**
	 * Find a global channel by case insensitive name.
	 * Returns NULL if not found.
	 */
	virtual Channel*	findChannel( const string& name ) const ;

	/* Removal methods. */

	/**
	 * Remove a remote client by integer numeric and return a
	 * pointer to the iClient.	
	 * Return NULL if not found.
	 */
	virtual iClient*	removeClient(
					const unsigned int& intYYXXX ) ;

	/**
	 * Remove a remote client by integer server and client numerics,
	 * and return a pointer to the iClient.
	 * Returns NULL if not found.
	 */
	virtual iClient*	removeClient( const unsigned int& YY,
					const unsigned int& XXX ) ;

	/**
	 * Remove a remote client by character numeric, and return a
	 * pointer to it.
	 * Returns NULL if not found.
	 */
	virtual iClient*	removeClient( const string& yyxxx ) ;

	/**
	 * Remove a remote client by its pointer.
	 * This will NOT deallocate the iClient instance.
	 */
	virtual iClient*	removeClient( iClient* ) ;

	/**
	 * Remove a local client from the network data tables.
	 */
	virtual xClient*	removeLocalClient( xClient* ) ;

	/**
	 * Remove a nick name from the internal nick name table.
	 * Note that this returns void.  This is because nick names
	 * are cross referenced in the nick name table.  That is,
	 * no heap space is allocated for elements in the nick name
	 * table, they simply point to iClient instances.
	 */
	virtual void		removeNick( const string& nickName ) ;

	/**
	 * Remove a remote server by its integer numeric, and
	 * return a pointer to it.
	 * When a server is removed, all its clients are also
	 * removed.  Each client is deleted, so make sure that
	 * the clients added to the network table (this) are
	 * allocated on the heap, and that there are no
	 * other stale pointers to any clients.
	 * If postEvent is true, then an EVT_QUIT will be generated
	 * for each client removal.
	 * Returns NULL if not found.
	 */
	virtual iServer*	removeServer( const unsigned int& YY,
					bool postEvent = false ) ;

	/**
	 * Remove a remote server by its character numeric, and
	 * return a pointer to it.
	 * When a server is removed, all its clients are also
	 * removed.  Each client is deleted, so make sure that
	 * the clients added to the network table (this) are
	 * allocated on the heap, and that there are no
	 * other stale pointers to any clients.
	 * Returns NULL if not found.
	 */
	virtual iServer*	removeServer( const string& YY ) ;

	/**
	 * Remove a remote server by its case-insensitive name, and
	 * return a pointer to it.
	 * When a server is removed, all its clients are also
	 * removed.  Each client is deleted, so make sure that
	 * the clients added to the network table (this) are
	 * allocated on the heap, and that there are no
	 * other stale pointers to any clients.
	 * Returns NULL if not found.
	 */
	virtual iServer*	removeServerName( const string& name ) ;

	/**
	 * Remove a channel from the network table, keyed by its
	 * case insensitive name.
	 * Returns NULL if not found.
	 */
	virtual Channel*	removeChannel( const string& name ) ;

	/**
	 * Remove a channel from the network table.
	 * Returns the Channel which has been removed, or NULL if the
	 * channel was not found.
	 */
	virtual Channel*	removeChannel( const Channel* theChan ) ;

	/* Utility methods */

	/**
	 * Change a remote client's nickname, given its character
	 * numeric and new nickname.
	 * This method does NOT work for local clients.
	 */
	virtual void		rehashNick( const string& yyxxx,
					const string& newNick ) ;

	/**
	 * This method performs a recursive removal of all servers
	 * which are uplinked by intYY.
	 * The server referenced by intYY is deallocated in this
	 * method.
	 */
	virtual void		OnSplit( const unsigned int& intYY ) ;

	/**
	 * Define a non-const iterator for walking through the
	 * structure of remote servers.
	 */
	typedef serverMapType::iterator serverIterator ;

	/**
	 * Define a const iterator for walking through the structure
	 * of remote servers.
	 */
	typedef serverMapType::const_iterator const_serverIterator ;

	/**
	 * Return a non-const iterator to the beginning of the
	 * remote servers table.
	 */
	inline serverIterator server_begin()
		{ return serverMap.begin() ; }

	/**
	 * Return a non-const iterator to the end of the
	 * remote servers table.
	 */
	inline serverIterator server_end()
		{ return serverMap.end() ; }

	/**
	 * Return a const iterator to the beginning of the
	 * remote servers table.
	 */
	inline const_serverIterator server_begin() const
		{ return serverMap.begin() ; }

	/**
	 * Return a const iterator to the end of the remote
	 * servers table.
	 */
	inline const_serverIterator server_end() const
		{ return serverMap.end() ; }

	/**
	 * Define a non-const iterator for walking through the
	 * structure of local clients (xClients).
	 */
	typedef xClientVectorType::iterator localClientIterator ;

	/**
	 * Define a const iterator for walking through the
	 * structure of local clients (xClients).
	 */
	typedef xClientVectorType::const_iterator const_localClientIterator ;

	/**
	 * Return a non-const iterator to the beginning of the
	 * local clients table.
	 */
	inline localClientIterator localClient_begin()
		{ return localClients.begin() ; }

	/**
	 * Return a non-const iterator to the end of the local
	 * clients table.
	 */
	inline localClientIterator localClient_end()
		{ return localClients.end() ; }

	/**
	 * Return a const iterator the beginning of the
	 * local clients table.
	 */
	inline const_localClientIterator localClient_begin() const
		{ return localClients.begin() ; }

	/**
	 * Return a const iterator to the end of the local
	 * clients table.
	 */
	inline const_localClientIterator localClient_end() const
		{ return localClients.end() ; }

	/**
	 * Define a non-const iterator for walking through the 
	 * channels structure
	 */
	typedef channelMapType::const_iterator	constChannelIterator;
	
	/**
	 * Returns an iterator to the begining of the channels structure
	 */
	inline constChannelIterator		channels_begin() const
		{ return channelMap.begin(); }
	
	/**
	 * Returns an iterator to the end of the channels structure
	 */
	inline constChannelIterator		channels_end() const
		{ return channelMap.end(); }

	/**
	 * Return the number of channels currently stored in the
	 * the channel table.
	 */
	inline size_t		channelList_size() const
		{ return channelMap.size() ; }

	/**
	 * Return the number of servers currently stored in the
	 * the server table.
	 */
	size_t		serverList_size() const ;

	/**
	 * Return the number of network clients currently stored
	 * in the client table.
	 */
	size_t		clientList_size() const ;

	/**
	 * Retrieve the number of clients on the given server.
	 */
	size_t		countClients( const iServer* theServer ) const ;

	/**
	 * A base class unary function used in foreach_xClient.
	 */
	struct fe_xClientBase : public unary_function< xClient*, void >
		{
		virtual void operator() ( xClient* ) {}
		} ;

	/**
	 * Execute a unary function for each xClient.  Keep in mind
	 * that the arguments passed to the operator() of this
	 * functor may be NULL.
	 */
	virtual void	foreach_xClient( fe_xClientBase  ) ;

	/**
	 * This method is used to set the xServer used for
	 * backwards communication.  This is bad, and I would like
	 * very much to get rid of it.
	 */
	virtual void	setServer( xServer* _theServer )
		{ theServer = _theServer ; }

	/**
	 * Attempt to match the hostname, which may include wildcard
	 * characters, with any clients on the network.  Return a
	 * list of pointers to const iClient's which match.
	 */
	virtual list< const iClient* > matchHost( const string& wildHost )
			const ;
	/**
	 * Match the given user@host string, which may include
	 * wildcards, to each client on the network.  Return a
	 * list of pointers to const iClient's which match.
	 */
	virtual list< const iClient* >	matchUserHost( const string& )
			const ;

	/**
	 * Attempt to find hostnames which are equivalent to the given
	 * hostname, found using a case insensitive search.
	 * Return a list of pointers to const iClient's which are
	 * found.
	 */
	virtual list< const iClient* > findHost( const string& hostName )
			const ;

	/**
	 * Perform a similar match as to matchHost(), except return
	 * only the number of matches found.
	 */
	virtual size_t	countMatchingHost( const string& wildHost ) const ;

	/**
	 * Perform a similar match as to matchUserHost(), except return
	 * only the number of matches found.
	 */
	virtual size_t	countMatchingUserHost( const string& wildUserHost )
				const ;
	/**
	 * Perform a similar operation as to findHost(), except return
	 * only the number of hosts found.
	 */
	virtual size_t	countHost( const string& hostName ) const ;

	/**
	 * Match the given client real name string, which may include
	 * wildcards, to each client on the network.  Return a
	 * list of pointers to const iClient's which match.
	 */
	virtual list< const iClient* >	matchRealName( const string& )
			const ;

protected:

	/**
	 * Disable copy constructing, this method is declared
	 * but NOT defined.
	 */
	xNetwork( const xNetwork& ) ;

	/**
	 * Disable copy assignment, this method is declared but
	 * NOT defined.
	 */
	xNetwork operator=( const xNetwork& ) ;

	/**
	 * This method is used internally when a client is added to
	 * the structure.
	 */
	void addNick( iClient* ) ;

	/**
	 * Perform a simple recursive search for all leaves of
	 * the server whose numeric is the second arguments, and
	 * place each of those servers' numerics into the vector.
	 */
	void	findLeaves( vector< unsigned int >& yyVector,
			const unsigned int intYY ) const ;

	/**
	 * The vector of local clients.
	 */
	xClientVectorType		localClients ;

	/**
	 * The structure used to store the network channel
	 * information.
	 */
	channelMapType			channelMap ;

	/**
	 * The structure used to store the nick name->iClient
	 * cross references.
	 */
	nickMapType			nickMap ;

	/**
	 * The container used to store the network iClient's,
	 * keyed by iClient numeric.
	 */
	numericMapType			numericMap ;

	/**
	 * The container used to store the network iServer's,
	 * keyed by iServer numeric.
	 */
	serverMapType			serverMap ;

	/**
	 * This variable is used backwards calls to the main server.
	 */
	xServer				*theServer ;
} ;

/**
 * Declare the single xNetwork instance as global, for now.
 */
extern xNetwork* Network ;

} // namespace gnuworld

#endif // __NETWORK_H

/**
 * msg_C.cc
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

#include	<new>
#include	<string>
#include	<utility>

#include	<cassert>

#include	"server.h"
#include	"Network.h"
#include	"events.h"

#include	"ELog.h"
#include	"Socket.h"
#include	"StringTokenizer.h"
#include	"xparameters.h"
#include	"iClient.h"
#include	"Channel.h"
#include	"ChannelUser.h"
#include	"ServerCommandHandler.h"

namespace gnuworld
{

using std::pair ;
using std::string ;
using std::endl ;

CREATE_HANDLER(msg_C)

/**
 * Someone has just joined an empty channel (create)
 * UAA C #xfactor 957134023
 * zBP C #OaXaCa,#UruApan,#skatos 957207634
 */
bool msg_C::Execute( const xParameters& Param )
{

// Verify that there exist sufficient arguments to successfully
// handle this command
// client_numeric #channel[,#channel2,...] timestamp
if( Param.size() < 3 )
	{
	// Insufficient arguments provided
	elog	<< "msg_C> Invalid number of parameters"
		<< endl ;

	// Return error
	return false ;
	}

// Find the client in question.
iClient* theClient = Network->findClient( Param[ 0 ] ) ;

// Did we find the client?
if( NULL == theClient )
	{
	// Nope, log the error
	elog	<< "msg_C> ("
		<< Param[ 1 ]
		<< ") Unable to find client: "
		<< Param[ 0 ]
		<< endl ;

	// Return error
	return false ;
	}

// Grab the creation time.
time_t creationTime =
	static_cast< time_t >( atoi( Param[ Param.size() - 1 ] ) ) ;

// Tokenize based on ','.  Multiple channels may be put into the
// same C(REATE) command.
StringTokenizer st( Param[ 1 ], ',' ) ;

for( StringTokenizer::const_iterator ptr = st.begin() ; ptr != st.end() ;
	++ptr )
	{

	// Is this a modeless channel?
	if( '+' == (*ptr)[ 0 ] )
		{
		// Modeless channel, ignore it
		continue ;
		}

	// Find the channel in question.
	Channel* theChan = Network->findChannel( *ptr ) ;

	// Did we find the channel?
	if( NULL == theChan )
		{
		// Channel doesn't exist..this transmutes to a create
		theChan = new (std::nothrow)
			Channel( *ptr, creationTime ) ;
		assert( theChan != 0 ) ;

		// Add this channel to the network channel table
		if( !Network->addChannel( theChan ) )
			{
			// Addition failed, log the error
			elog	<< "msg_C> Failed to add channel: "
				<< *theChan
				<< endl ;

			// Prevent memory leaks by removing the unused
			// channel
			delete theChan ; theChan = 0 ;

			// continue to next one *shrug*
			continue ;
			}
		}

	// Add this channel to the client's channel structure.
	if( !theClient->addChannel( theChan ) )
		{
		elog	<< "msg_C> Unable to add channel "
			<< *theChan
			<< " to iClient "
			<< *theClient
			<< endl ;

		continue ;
		}

	// Create a new ChannelUser to represent this iClient's
	// membership in this channel.
	ChannelUser* theUser =
		new (std::nothrow) ChannelUser( theClient ) ;
	assert( theUser != 0 ) ;

	// The user who creates a channel is automatically +o
	theUser->setModeO() ;

	// Build associations

	// Add the ChannelUser to the Channel's information
	if( !theChan->addUser( theUser ) )
		{
		// Addition failed, log the error
		elog	<< "msg_C> Unable to add user "
			<< theUser->getNickName()
			<< " to channel "
			<< theChan->getName()
			<< endl ;

		// Prevent a memory leak by deallocating the unused
		// ChannelUser structure
		delete theUser ; theUser = 0 ;

		// Remove the channel information from the client
		theClient->removeChannel( theChan ) ;

		// Continue to next channel
		continue ;
		}

	// Notify all listening xClients of this event
	theServer->PostChannelEvent( EVT_CREATE, theChan,
		static_cast< void* >( theClient ) ) ;

	} // for()

return true ;
}

} // namespace gnuworld

/**
 * msg_M.cc
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
#include	<vector>
#include	<iostream>

#include	"misc.h"
#include	"events.h"

#include	"server.h"
#include	"iClient.h"
#include	"Channel.h"
#include	"ChannelUser.h"
#include	"Network.h"
#include	"ELog.h"
#include	"StringTokenizer.h"
#include	"ServerCommandHandler.h"

namespace gnuworld
{

using std::string ;
using std::vector ;
using std::endl ;
using std::ends ;

class msg_M : public ServerCommandHandler
{
public:
	msg_M( xServer* theServer )
	 : ServerCommandHandler( theServer )
	{}
	virtual ~msg_M()
	{}

	virtual bool Execute( const xParameters& ) ;

protected:
	bool	onUserModeChange( const xParameters& ) ;

} ;

CREATE_LOADER(msg_M)

// Mode change
// OAD M ripper_ :+owg
//
// i M #3dx +o eAA
// J[K M DEMET_33 :+i
bool msg_M::Execute( const xParameters& Param )
{
if( Param.size() < 3 )
	{
	elog	<< "msg_M> Invalid number of arguments"
		<< endl ;
	return false ;
	}

// This source stuff really isn't used here, but it's here for
// debugging and validation.
iServer* serverSource = 0 ;
iClient* clientSource = 0 ;

// Note that the order of this if/else if/else is important
if( NULL != strchr( Param[ 0 ], '.' ) )
	{
	// Server, by name
	serverSource = Network->findServerName( Param[ 0 ] ) ;
	}
else if( strlen( Param[ 0 ] ) >= 3 )
	{
	// Client numeric
	clientSource = Network->findClient( Param[ 0 ] ) ;
	}
else
	{
	// 1 or 2 char numeric, server
	serverSource = Network->findServer( Param[ 0 ] ) ;
	}

if( (NULL == clientSource) && (NULL == serverSource) )
	{
	elog	<< "msg_M> Unable to find source: "
		<< Param[ 0 ]
		<< endl ;
	// TODO: Why is this commented out?
	// return -1
	}

// Is it a user mode change?
if( '#' != Param[ 1 ][ 0 ] )
	{
	// Yup, process the user's mode change(s)
	return onUserModeChange( Param ) ;
	}

// Find the channel in question
Channel* theChan = Network->findChannel( Param[ 1 ] ) ;
if( NULL == theChan )
	{
	elog	<< "msg_M> Unable to find channel: "
		<< Param[ 1 ]
		<< endl ;
	return false ;
	}

// Find the ChannelUser of the source client
// It is possible that the ChannelUser will be NULL, in the
// case that a server is setting the mode(s)
ChannelUser* theUser = 0 ;
if( clientSource != 0 )
	{
	theUser = theChan->findUser( clientSource ) ;
	if( NULL == theUser )
		{
		elog	<< "msg_M> ("
			<< theChan->getName()
			<< ") Unable to find channel user: "
			<< clientSource->getCharYYXXX()
			<< endl ;
		return false ;
		}
	}

bool polarity = true ;
xParameters::size_type argPos = 3 ;

xServer::opVectorType opVector ;
xServer::voiceVectorType voiceVector ;
xServer::banVectorType banVector ;

for( const char* modePtr = Param[ 2 ] ; *modePtr ; ++modePtr )
	{
	switch( *modePtr )
		{
		case '+':
			polarity = true ;
			break ;
		case '-':
			polarity = false ;
			break ;
		case 'C':
			theServer->OnChannelModeC( theChan, polarity, theUser ) ;
			break;
		case 'S':
			theServer->OnChannelModeS( theChan, polarity, theUser ) ;
			break;
		case 'T':
			theServer->OnChannelModeT( theChan, polarity, theUser ) ;
			break;
		case 'c':
			theServer->OnChannelModec( theChan, polarity, theUser ) ;
			break;
		case 'i':
			theServer->OnChannelModei( theChan, polarity, theUser ) ;
			break ;
		case 'm':
			theServer->OnChannelModem( theChan, polarity, theUser ) ;
			break ;
		case 'n':
			theServer->OnChannelModen( theChan, polarity, theUser ) ;
			break ;
		case 'p':
			theServer->OnChannelModep( theChan, polarity, theUser ) ;
			break ;
		case 'r':
			theServer->OnChannelModer( theChan, polarity, theUser );
			break;
		case 's':
			theServer->OnChannelModes( theChan, polarity, theUser ) ;
			break ;
		case 't':
			theServer->OnChannelModet( theChan, polarity, theUser ) ;
			break ;

		// Channel mode l only has an argument if
		// it is being added, but not removed
		case 'l':
			if( polarity && (argPos >= Param.size()) )
				{
				elog	<< "msg_M> Invalid "
					<< "format for message: missing "
					<< "argument to mode +l"
					<< endl ;
				continue ;
				}

			theServer->OnChannelModel( theChan,
				polarity, theUser,
				polarity ? atoi( Param[ argPos++ ] )
					: 0 ) ;
			break ;

		// Channel mode k always has an argument
		case 'k':
			if( argPos >= Param.size() )
				{
				elog	<< "msg_M> Invalid "
					<< "format for message: missing "
					<< "argument for mode 'k'"
					<< endl ;
				continue ;
				}

			theServer->OnChannelModek( theChan,
				polarity, theUser,
				Param[ argPos++ ] ) ;
			break ;
		case 'o':
			{
			if( argPos >= Param.size() )
				{
				elog	<< "msg_M> Invalid "
					<< "format for message: missing "
					<< "argument for mode 'o'"
					<< endl ;
				continue ;
				}

			iClient* targetClient = Network->findClient(
				Param[ argPos++ ] ) ;
			if( NULL == targetClient )
				{
				elog	<< "msg_M> Unable to "
					<< "find op target client: "
					<< Param[ argPos - 1 ]
					<< endl ;
				break ;
				}
			ChannelUser* targetUser = theChan->findUser(
				targetClient ) ;
			if( NULL == targetUser )
				{
				elog	<< "msg_M> Unable to "
					<< "find op target user: "
					<< Param[ argPos - 1 ]
					<< endl ;
				break ;
				}
			opVector.push_back(
				pair< bool, ChannelUser* >(
				polarity, targetUser ) ) ;

			// If the op mode is +o, remove the ZOMBIE
			// state from this user.
			if( polarity && targetUser->isZombie() )
				{
				targetUser->removeZombie() ;
				elog	<< "msg_M> Removing "
					<< "zombie"
					<< endl ;
				}
			break ;
			}
		case 'v':
			{
			if( argPos >= Param.size() )
				{
				elog	<< "msg_M> Invalid "
					<< "format for message: missing "
					<< "argument for mode 'v'"
					<< endl ;
				continue ;
				}

			iClient* targetClient = Network->findClient(
				Param[ argPos++ ] ) ;
			if( NULL == targetClient )
				{
				elog	<< "msg_M> Unable to "
					<< "find voice target client: "
					<< Param[ argPos - 1 ]
					<< endl ;
				break ;
				}
			ChannelUser* targetUser = theChan->findUser(
				targetClient ) ;
			if( NULL == targetUser )
				{
				elog	<< "msg_M> Unable to "
					<< "find voice target user: "
					<< Param[ argPos - 1 ]
					<< endl ;
				break ;
				}
			voiceVector.push_back(
				pair< bool, ChannelUser* >(
				polarity, targetUser ) ) ;
			break ;
			}
		case 'b':
			{
			if( argPos >= Param.size() )
				{
				elog	<< "msg_M> Invalid "
					<< "format for message: missing "
					<< "argument for mode 'b'"
					<< endl ;
				continue ;
				}

			const char* targetBan = Param[ argPos++ ] ;
			banVector.push_back(
				pair< bool, string >(
				polarity, string( targetBan ) ) ) ;
			break ;
			}

		} // switch()
	} // for()

if( !opVector.empty() )
	{
	theServer->OnChannelModeO( theChan, theUser, opVector ) ;
	}
if( !voiceVector.empty() )
	{
	theServer->OnChannelModeV( theChan, theUser, voiceVector ) ;
	}
if( !banVector.empty() )
	{
	theServer->OnChannelModeB( theChan, theUser, banVector ) ;
	}

return true ;
}

bool msg_M::onUserModeChange( const xParameters& Param )
{

// Since users aren't allowed to change modes for anyone other than
// themselves, there is no need to lookup the second user argument
// For some reason, when a user changes his/her/its modes, it still
// specifies the second argument to be nickname instaed of numeric.
iClient* theClient = Network->findNick( Param[ 1 ] ) ;
if( NULL == theClient )
	{
	elog	<< "msg_M::OnUserModeChange> Unable to find target "
		<< "client: "
		<< Param[ 1 ]
		<< endl ;
	return false ;
	}

// Local channels are not propogated across the network.

// It's important that the mode '+' be default
bool plus = true ;

for( const char* modePtr = Param[ 2 ] ; *modePtr ; ++modePtr )
	{
	switch( *modePtr )
		{
		case '+':
			plus = true ;
			break;
		case '-':
			plus = false ;
			break;
		case 'i':
			if( plus )	theClient->setModeI() ;
			else		theClient->removeModeI() ;
			break ;
		case 'k':
			if( plus )	theClient->setModeK() ;
			else		theClient->removeModeK() ;
			break ;
		case 'd':
			if( plus )	theClient->setModeD() ;
			else		theClient->removeModeD() ;
			break ;
		case 'w':
			if( plus )	theClient->setModeW() ;
			else		theClient->removeModeW() ;
			break ;
		case 'x':
			if( plus )	theClient->setModeX() ;
			else		theClient->removeModeX() ;
			break ;
		case 'o':
		case 'O':
			if( plus )
				{
				theClient->setModeO() ;
				theServer->PostEvent( EVT_OPER,
					static_cast< void* >( theClient ) ) ;
				}
			else
				{
//				elog	<< "msg_M::onUserModeChange> "
//					<< "Caught -o for user: "
//					<< *theClient
//					<< endl ;
				theClient->removeModeO() ;
				}
			break ;
		default:
			break ;
		} // close switch
	} // close for
return true ;
}

} // namespace gnuworld

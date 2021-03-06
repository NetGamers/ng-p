/*
 * client.cc
 */

#include	<new>
#include	<string>
#include	<sstream>
#include	<iostream>

#include	<cstdio>
#include	<cctype>
#include	<cstdarg>
#include	<cstring>
#include	<cstdlib>

#include	"config.h"
#include	"misc.h"
#include	"Numeric.h"
#include	"iClient.h"
#include	"iServer.h"
#include	"Network.h"
#include	"ip.h"

#include	"client.h"
#include	"EConfig.h"
#include	"StringTokenizer.h"

#include	"ELog.h"
#include	"events.h"

namespace gnuworld
{

using std::string ;
using std::stringstream ;
using std::ends ;
using std::endl ;

xClient::xClient()
{
me = 0 ;
MyUplink = NULL ;
Connected = false ;

intYY = intXXX = 0 ;

memset( charYY, 0, sizeof( charYY ) ) ;
memset( charXXX, 0, sizeof( charXXX ) ) ;
}

xClient::xClient( const string& fileName )
 : configFileName( fileName )
{
MyUplink = 0 ;
Connected = false ;

intYY = intXXX = 0 ;

memset( charYY, 0, sizeof( charYY ) ) ;
memset( charXXX, 0, sizeof( charXXX ) ) ;

EConfig conf( fileName ) ;
nickName = conf.Require( "nickname" )->second ;
userName = conf.Require( "username" )->second ;
hostName = conf.Require( "hostname" )->second ;
userDescription = conf.Require( "userdescription" )->second ;

Mode( conf.Require( "mode" )->second ) ;

}

xClient::~xClient()
{}

// This method is called after being added
// to the network tables.
// It has been assigned int XX.
void xClient::ImplementServer( xServer* Server )
{
if( NULL == Server )
	{
	Connected = false ;
	elog	<< "ImplementServer> NULL Uplink"
		<< endl ;
	return ;
	}

MyUplink = Server ;

intYY = MyUplink->getIntYY() ;

inttobase64( charYY, intYY, 2 ) ;
inttobase64( charXXX, intXXX, 3 ) ;

}

int xClient::BurstChannels()
{
return 0 ;
}

int xClient::Connect( int ForceTime )
{
if( !Connected && MyUplink && MyUplink->isConnected() )
	{
	Connected = true ;
	return OnConnect() ;
	}
return -1 ;
}

int xClient::Exit( const string& Message )
{
if( !Connected )
	{
	return -1 ;
	}

stringstream s ;
s	<< getCharYYXXX()
	<< " Q :"
	<< Message
	<< ends ;

MyUplink->Write( s ) ;

Connected = false ;
return OnQuit() ;
}

string xClient::getModes() const
{
string			Mode( "+" ) ;

if( mode & iClient::MODE_DEAF )		Mode += 'd' ;
if( mode & iClient::MODE_SERVICES )	Mode += 'k' ;
if( mode & iClient::MODE_OPER )		Mode += 'o' ;
if( mode & iClient::MODE_WALLOPS )	Mode += 'w' ;
if( mode & iClient::MODE_INVISIBLE )	Mode += 'i' ;

return Mode ;
}

int xClient::Mode( const string& Value )
{

//elog	<< "xClient::Mode> Value: "
//	<< Value
//	<< endl ;

// Set the bot's modes, and output to
// the network if we are connected

// Clear the internal modes
mode = 0 ;

// Iterate through the array and
// set modes appropriately
string::const_iterator ptr = Value.begin(),
	end = Value.end() ;

for( ; ptr != end ; ++ptr )
	{
	switch( *ptr )
		{
		case '+': break ;
		case 'd': mode |= iClient::MODE_DEAF ; break;
		case 'k': mode |= iClient::MODE_SERVICES ; break;
		case 'o': mode |= iClient::MODE_OPER ; break;
		case 'w': mode |= iClient::MODE_WALLOPS ; break;
		case 'i': mode |= iClient::MODE_INVISIBLE ; break;

		default:
			elog	<< "xClient::Mode> Unknown mode: "
				<< *ptr
				<< endl ;
			break ;
		} // switch()
	} // close while

// Output to the network if we are connected
if( (MyUplink != NULL) && MyUplink->isConnected() && !Value.empty() )
	{
	stringstream s ;
	s	<< getCharYYXXX()
		<< " M "
		<< getCharYYXXX()
		<< " "
		<< Value
		<< ends ;

	return MyUplink->Write( s ) ;
	}

return( 1 ) ;
}

int xClient::QuoteAsServer( const string& Message )
{
if( MyUplink )
	{
	return MyUplink->Write( Message ) ;
	}
return -1 ;
}

int xClient::Wallops( const string& Message )
{
return QuoteAsServer( getCharYYXXX() + " WA :" + Message ) ;
}

int xClient::Wallops( const char* Format, ... )
{
if( Connected && MyUplink && Format && Format[ 0 ] != 0 )
	{
	char buffer[ 1024 ] ;
	memset( buffer, 0, 1024 ) ;
	va_list list;

	va_start( list, Format ) ;
	vsnprintf( buffer, 1024, Format, list ) ;
	va_end( list ) ;

	return MyUplink->Write( "%s WA :%s",
		getCharYYXXX().c_str(),
		buffer ) ;
	}
return -1 ;
}

int xClient::WallopsAsServer( const string& buf )
{
if( !Connected || !MyUplink )
	{
	return -1 ;
	}
return MyUplink->Wallops( buf ) ;
}

int xClient::WallopsAsServer( const char* Format, ... )
{
if( Connected && MyUplink && Format && Format[ 0 ] != 0 )
	{
	char buffer[ 1024 ] = { 0 } ;
	va_list list;

	va_start( list, Format ) ;
	vsnprintf( buffer, 1024, Format, list ) ;
	va_end( list ) ;

	return MyUplink->Wallops( buffer ) ;
	}
return -1 ;
}

int xClient::ModeAsServer( const string& Channel, const string& Mode )
{
if( Connected && MyUplink )
	{
	return MyUplink->Write( "%s M #%s %s\r\n",
		MyUplink->getCharYY(),
		(Channel[ 0 ] == '#' ? (Channel.c_str() + 1) :
			Channel.c_str()),
		Mode.c_str() ) ;
	}
return -1 ;
}

int xClient::ModeAsServer( const Channel* theChan, const string& Mode )
{
assert( theChan != 0 ) ;

return ModeAsServer( theChan->getName(), Mode ) ;
}

int xClient::DoCTCP( iClient* Target,
	const string& CTCP,
	const string& Message )
{
if( !Connected && !MyUplink )
	{
	return -1 ;
	}

return MyUplink->Write( "%s O %s :\001%s %s\001\r\n",
	getCharYYXXX().c_str(),
	Target->getCharYYXXX().c_str(),
	CTCP.c_str(),
	Message.c_str() ) ;
}

int xClient::Message( const iClient* Target, const string& Message )
{
if( Connected && MyUplink )
	{
	return MyUplink->Write( "%s P %s :%s\r\n",
		getCharYYXXX().c_str(),
		Target->getCharYYXXX().c_str(),
		Message.c_str() ) ;
	}
return 0 ;
}

int xClient::Message( const iClient* Target, const char* Message, ... )
{
if( Connected && MyUplink && Message && Message[ 0 ] !=0 )
	{
	char buffer[ 1024 ] ;
	memset( buffer, 0, 1024 ) ;
	va_list list;

	va_start( list, Message ) ;
	vsnprintf( buffer, 1024, Message, list ) ;
	va_end( list ) ;

	return MyUplink->Write( "%s P %s :%s\r\n",
		getCharYYXXX().c_str(),
		Target->getCharYYXXX().c_str(),
		buffer ) ;
	}
return 0 ;
}

int xClient::Message( const string& Channel, const char* Message, ... )
{
if( Connected && MyUplink && Message && Message[ 0 ] != 0 )
	{
	char buffer[ 1024 ] = { 0 } ;
	va_list list ;

	va_start( list, Message ) ;
	vsnprintf( buffer, 1024, Message, list ) ;
	va_end( list ) ;

	return MyUplink->Write( "%s P #%s :%s\r\n",
		getCharYYXXX().c_str(),
		(Channel[ 0 ] == '#') ? (Channel.c_str() + 1) :
			Channel.c_str(),
		buffer ) ;
	}
return 0 ;
}

int xClient::Message( const Channel* theChan, const string& Message )
{
assert( theChan != 0 ) ;

if( Connected && MyUplink )
	{
	return MyUplink->Write( "%s P %s :%s",
		getCharYYXXX().c_str(),
		theChan->getName().c_str(),
		Message.c_str() ) ;
	}
return 0 ;
}

int xClient::Notice( const iClient* Target, const string& Message )
{
if( Connected && MyUplink )
	{
	return MyUplink->Write( "%s O %s :%s\r\n",
		getCharYYXXX().c_str(),
		Target->getCharYYXXX().c_str(),
		Message.c_str() ) ;
	}
return 0 ;
}

int xClient::Notice( const iClient* Target, const char* Message, ... )
{
if( Connected && MyUplink && Message && Message[ 0 ] != 0 )
	{
	char buffer[ 1024 ] ;
	memset( buffer, 0, 1024 ) ;
	va_list list;

	va_start(list, Message);
	vsnprintf(buffer, 1024, Message, list);
	va_end(list);

	// O is the token for NOTICE, *shrug*
	return MyUplink->Write("%s O %s :%s\r\n",
		getCharYYXXX().c_str(),
		Target->getCharYYXXX().c_str(),
		buffer ) ;
	}
return 0 ;
}

int xClient::Notice( const string& Channel, const char* Message, ... )
{
if( Connected && MyUplink && Message && Message[ 0 ] != 0 )
	{
	char buffer[ 1024 ] ;
	memset( buffer, 0, 1024 ) ;
	va_list list;

	va_start(list, Message);
	vsnprintf(buffer, 1024, Message, list);
	va_end(list);

	return MyUplink->Write("%s O #%s :%s\r\n",
		getCharYYXXX().c_str(),
		('#' == Channel[ 0 ]) ? (Channel.c_str() + 1) :
			Channel.c_str(),
		buffer ) ;
	}
return 0 ;
}

int xClient::OnCTCP( iClient*, const string&,
	const string&, bool )
{
return 0;
}

int xClient::OnEvent( const eventType&, void*, void*, void*, void* )
{
return 0;
}

int xClient::OnChannelEvent( const channelEventType&, Channel*,
	void*, void*, void*, void* )
{
return 0 ;
}

void xClient::OnNetworkKick( Channel*,
	iClient*, // srcClient, may be NULL
	iClient*, // destClient
	const string&, // kickMessage,
	bool ) // authoritative
{
}

void xClient::OnChannelModeO( Channel*, ChannelUser*,
	const xServer::opVectorType& )
{
}

void xClient::OnChannelModeV( Channel*, ChannelUser*,
	const xServer::voiceVectorType& )
{
}

void xClient::OnChannelModeB( Channel*, ChannelUser*,
	const xServer::banVectorType& )
{
}

int xClient::OnPrivateMessage( iClient*, const string&, bool )
{
return 0;
}

int xClient::OnServerMessage( iServer*, const string&, bool )
{
return 0;
}

int xClient::OnConnect()
{
Connected = true ;
return 0 ;
}

int xClient::OnQuit()
{
Connected = false ;
return 0 ;
}

int xClient::OnKill()
{
Connected = false ;
return 0 ;
}

int xClient::OnWhois( iClient* sourceClient, iClient* targetClient )
{
return 0 ;
}

int xClient::OnInvite( iClient* sourceClient, Channel* theChan )
{
return 0 ;
}

int xClient::Kill( iClient* theClient, const string& reason )
{
assert( theClient != 0 ) ;

// TODO
Write( "%s D %s :%s",
	MyUplink->getCharYY(),
	theClient->getCharYYXXX().c_str(),
	reason.c_str() ) ;

/*
// Do NOT cast away constness
string localReason( reason ) ;

MyUplink->PostEvent( EVT_KILL,
	0,
	static_cast< void* >( theClient ),
	static_cast< void* >( &localReason ) ) ;

// Remove the user
delete Network->removeClient( theClient ) ;
*/

return 0 ;
}

bool xClient::Op( Channel* theChan, iClient* theClient )
{
assert( theChan != NULL ) ;
assert( theClient != NULL) ;

if( !Connected )
	{
	return false ;
	}

ChannelUser* theUser = theChan->findUser( theClient ) ;
if( NULL == theUser )
	{
	elog	<< "xClient::Op> Unable to find ChannelUser: "
		<< *theClient
		<< endl ;
	return false ;
	}

if( theUser->getMode( ChannelUser::MODE_O ) )
	{
	// User is already opped
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Op> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

// Op the user
Write( "%s M %s +o %s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	theClient->getCharYYXXX().c_str() ) ;

// Was the bot on the channel previously?
if( !OnChannel )
	{
	Part( theChan ) ;
	}

xServer::opVectorType opVector ;
opVector.push_back( xServer::opVectorType::value_type(
	true, theUser ) ) ;

MyUplink->OnChannelModeO( theChan, 0, opVector ) ;

return true ;
}

bool xClient::Op( Channel* theChan,
	const vector< iClient* >& clientVector )
{
assert( theChan != NULL ) ;

if( !Connected )
	{
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Op> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

xServer::opVectorType opVector ;

for( vector< iClient* >::const_iterator ptr = clientVector.begin(),
	end = clientVector.end() ; ptr != end ; ++ptr )
	{
	if( NULL == *ptr )
		{
		elog	<< "xClient::Op(vector)> Found NULL "
			<< "iClient!"
			<< endl ;
		continue ;
		}

	ChannelUser* theUser = theChan->findUser( *ptr ) ;
	if( NULL == theUser )
		{
		elog	<< "xClient::Op(vector)> Unable to find "
			<< "client on channel: "
			<< theChan->getName()
			<< endl ;
		continue ;
		}

	if( !theUser->getMode( ChannelUser::MODE_O ) )
		{
		// User is not already opped
		opVector.push_back( xServer::opVectorType::value_type(
			true, theUser ) ) ;
		}
	}

string modeString ;
string args ;

for( xServer::opVectorType::const_iterator ptr = opVector.begin(),
	end = opVector.end() ; ptr != end ; ++ptr )
	{
	modeString += 'o' ;
	args += ptr->second->getCharYYXXX() + ' ' ;

	if( (MAX_CHAN_MODES == modeString.size()) ||
		((ptr + 1) == end) )
		{
		stringstream s ;
		s	<< getCharYYXXX() << " M "
			<< theChan->getName() << ' '
			<< "+" << modeString << ' ' << args
			<< ends ;

		Write( s ) ;

		modeString.erase( modeString.begin(), modeString.end() ) ;
		args.erase( args.begin(), args.end() ) ;

		} // if()

	} // for()

MyUplink->OnChannelModeO( theChan, 0, opVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}
 
return true ;
}

bool xClient::Voice( Channel* theChan,
	const vector< iClient* >& clientVector )
{
assert( theChan != NULL ) ;

if( !Connected )
	{
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Voice> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

xServer::voiceVectorType voiceVector ;

for( vector< iClient* >::const_iterator ptr = clientVector.begin(),
	end = clientVector.end() ; ptr != end ; ++ptr )
	{
	if( NULL == *ptr )
		{
		elog	<< "xClient::Voice(vector)> Found NULL "
			<< "iClient for channel: "
			<< theChan->getName()
			<< endl ;
		continue ;
		}

	ChannelUser* theUser = theChan->findUser( *ptr ) ;
	if( NULL == theUser )
		{
		elog	<< "xClient::Voice(vector)> Unable to find "
			<< "client on channel: "
			<< theChan->getName()
			<< endl ;
		continue ;
		}

	if( !theUser->getMode( ChannelUser::MODE_V ) )
		{
		// User is not already voiced
		voiceVector.push_back(
			xServer::voiceVectorType::value_type(
			true, theUser ) ) ;
		}
	}

string modeString ;
string args ;

for( xServer::voiceVectorType::const_iterator ptr = voiceVector.begin(),
	end = voiceVector.end() ; ptr != end ; ++ptr )
	{
	modeString += 'v' ;
	args += ptr->second->getCharYYXXX() + ' ' ;

	if( (MAX_CHAN_MODES == modeString.size()) ||
		((ptr + 1) == end) )
		{
		stringstream s ;
		s	<< getCharYYXXX() << " M "
			<< theChan->getName() << ' '
			<< "+" << modeString << ' ' << args
			<< ends ;

		Write( s ) ;

		modeString.erase( modeString.begin(), modeString.end() ) ;
		args.erase( args.begin(), args.end() ) ;

		} // if()

	} // for()

MyUplink->OnChannelModeV( theChan, 0, voiceVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}
 
return true ;

}

bool xClient::Voice( Channel* theChan, iClient* theClient )
{
assert( theChan != NULL ) ;
assert( theClient != NULL) ;

if( !Connected )
	{
	return false ;
	}

ChannelUser* theUser = theChan->findUser( theClient ) ;
if( NULL == theUser )
	{
	elog	<< "xClient::Voice> Unable to find ChannelUser: "
		<< *theClient << endl ;
	return false ;
	}

if( theUser->getMode( ChannelUser::MODE_V ) )
	{
	// User is already voiced
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Voice> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

Write( "%s M %s +v %s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	theClient->getCharYYXXX().c_str() ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

xServer::voiceVectorType voiceVector ;
voiceVector.push_back( xServer::voiceVectorType::value_type(
	true, theUser ) ) ;

MyUplink->OnChannelModeV( theChan, 0, voiceVector ) ;

return true ;
} 
 
bool xClient::DeOp( Channel* theChan, iClient* theClient )
{
assert( theChan != NULL ) ;
assert( theClient != NULL ) ;

if( !Connected )
	{
	return false ;
	}

ChannelUser* theUser = theChan->findUser( theClient ) ;
if( NULL == theUser )
	{
	elog	<< "xClient::DeOp> Unable to find ChannelUser: "
		<< *theClient
		<< endl ;
	return false ;
	}

if( !theUser->getMode( ChannelUser::MODE_O ) )
	{
	// User is not opped
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::DeOp> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

Write( "%s M %s -o %s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	theClient->getCharYYXXX().c_str() ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

xServer::opVectorType opVector ;
opVector.push_back( xServer::opVectorType::value_type(
	false, theUser ) ) ;

MyUplink->OnChannelModeO( theChan, 0, opVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::DeOp( Channel* theChan,
	const vector< iClient* >& clientVector )
{
assert( theChan != NULL ) ;

if( !Connected )
	{
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::DeOp> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

xServer::opVectorType opVector ;

for( vector< iClient* >::const_iterator ptr = clientVector.begin(),
	end = clientVector.end() ; ptr != end ; ++ptr )
	{
	if( NULL == *ptr )
		{
		elog	<< "xClient::DeOp(vector)> Found NULL "
			<< "iClient!"
			<< endl ;
		continue ;
		}

	ChannelUser* theUser = theChan->findUser( *ptr ) ;
	if( NULL == theUser )
		{
		elog	<< "xClient::DeOp(vector)> Unable to find "
			<< "client on channel: "
			<< theChan->getName()
			<< endl ;
		continue ;
		}

	if( theUser->getMode( ChannelUser::MODE_O ) )
		{
		// User is opped
		opVector.push_back( xServer::opVectorType::value_type(
			false, theUser ) ) ;
		}
	}

string modeString ;
string args ;

for( xServer::opVectorType::const_iterator ptr = opVector.begin(),
	end = opVector.end() ; ptr != end ; ++ptr )
	{
	modeString += 'o' ;
	args += ptr->second->getCharYYXXX() + ' ' ;

	if( (MAX_CHAN_MODES == modeString.size()) ||
		((ptr + 1) == end) )
		{
		stringstream s ;
		s	<< getCharYYXXX() << " M "
			<< theChan->getName() << ' '
			<< "-" << modeString << ' ' << args
			<< ends ;

		Write( s ) ;

		modeString.erase( modeString.begin(), modeString.end() ) ;
		args.erase( args.begin(), args.end() ) ;

		} // if()

	} // for()

MyUplink->OnChannelModeO( theChan, 0, opVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}
 
return true ;
}
 
bool xClient::DeVoice( Channel* theChan, iClient* theClient )
{
assert( theChan != 0 ) ;
assert( theClient != 0 ) ;

if( !Connected )
	{
	return false ;
	}

ChannelUser* theUser = theChan->findUser( theClient ) ;
if( NULL == theUser )
	{
	elog	<< "xClient::DeVoice> Unable to find ChannelUser: "
		<< *theClient
		<< endl ;
	return false ;
	}

if( !theUser->getMode( ChannelUser::MODE_V ) )
	{
	// User is not voiced
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::DeVoice> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

Write( "%s M %s -v %s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	theClient->getCharYYXXX().c_str() ) ;

xServer::voiceVectorType voiceVector ;
voiceVector.push_back( xServer::voiceVectorType::value_type(
	false, theUser ) ) ;

MyUplink->OnChannelModeV( theChan, 0, voiceVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::DeVoice( Channel* theChan,
	const vector< iClient* >& clientVector )
{
assert( theChan != NULL ) ;

if( !Connected )
	{
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::DeVoic> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

xServer::voiceVectorType voiceVector ;

for( vector< iClient* >::const_iterator ptr = clientVector.begin(),
	end = clientVector.end() ; ptr != end ; ++ptr )
	{
	if( NULL == *ptr )
		{
		elog	<< "xClient::DeVoice(vector)> Found NULL "
			<< "iClient!"
			<< endl ;
		continue ;
		}

	ChannelUser* theUser = theChan->findUser( *ptr ) ;
	if( NULL == theUser )
		{
		elog	<< "xClient::DeVoice(vector)> Unable to find "
			<< "client on channel: "
			<< theChan->getName()
			<< endl ;
		continue ;
		}

	if( theUser->getMode( ChannelUser::MODE_V ) )
		{
		// User is voiced
		voiceVector.push_back(
			xServer::voiceVectorType::value_type(
			false, theUser ) ) ;
		}
	}

string modeString ;
string args ;

for( xServer::voiceVectorType::const_iterator ptr = voiceVector.begin(),
	end = voiceVector.end() ; ptr != end ; ++ptr )
	{
	modeString += 'v' ;
	args += ptr->second->getCharYYXXX() + ' ' ;

	if( (MAX_CHAN_MODES == modeString.size()) ||
		((ptr + 1) == end) )
		{
		stringstream s ;
		s	<< getCharYYXXX() << " M "
			<< theChan->getName() << ' '
			<< "-" << modeString << ' ' << args
			<< ends ;

		Write( s ) ;

		modeString.erase( modeString.begin(), modeString.end() ) ;
		args.erase( args.begin(), args.end() ) ;

		} // if()

	} // for()

MyUplink->OnChannelModeV( theChan, 0, voiceVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}
 
return true ;
}
 
bool xClient::Ban( Channel* theChan, iClient* theClient )
{
assert( theChan != NULL ) ;
assert( theClient != NULL ) ;

if( !Connected )
	{
	return false ;
	}

if( 0 == theChan->findUser( theClient ) )
	{
	// User is not on that channel
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Ban> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

string banMask = Channel::createBan( theClient ) ;

Write( "%s M %s +b :%s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	banMask.c_str() ) ;

// No users are kicked by just setting a ban.

xServer::banVectorType banVector ;
banVector.push_back( xServer::banVectorType::value_type(
	true, banMask ) ) ;

MyUplink->OnChannelModeB( theChan, 0, banVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::UnBan( Channel* theChan, const string& banMask )
{
assert( theChan != 0 ) ;

if( !Connected )
	{
	return false ;
	}

if( !theChan->findBan( banMask ) )
	{
	return true ;
	}

// Ban exists, remove it
bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::UnBan> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

Write( "%s M %s -b %s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	banMask.c_str() ) ;

xServer::banVectorType banVector ;
banVector.push_back( xServer::banVectorType::value_type(
	false, banMask ) ) ;

MyUplink->OnChannelModeB( theChan, 0, banVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::Ban( Channel* theChan,
	const vector< iClient* >& clientVector )
{
assert( theChan != NULL ) ;

if( !Connected )
	{
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Ban> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

xServer::banVectorType banVector ;

for( vector< iClient* >::const_iterator ptr = clientVector.begin(),
	end = clientVector.end() ; ptr != end ; ++ptr )
	{
	if( NULL == *ptr )
		{
		elog	<< "xClient::Ban(vector)> Found NULL "
			<< "iClient!"
			<< endl ;
		continue ;
		}

	ChannelUser* theUser = theChan->findUser( *ptr ) ;
	if( NULL == theUser )
		{
		elog	<< "xClient::Ban(vector)> Unable to find "
			<< "client on channel: "
			<< theChan->getName()
			<< endl ;
		continue ;
		}

	banVector.push_back( xServer::banVectorType::value_type(
		true, Channel::createBan( *ptr ) ) ) ;
	}

string modeString = "+" ;
string args ;

for( xServer::banVectorType::const_iterator ptr = banVector.begin(),
	end = banVector.end() ; ptr != end ; ++ptr )
	{
	modeString += 'b' ;
	args += ptr->second + ' ' ;

	if( ((MAX_CHAN_MODES + 1) == modeString.size()) ||
		((ptr + 1) == end) )
		{
		stringstream s ;
		s	<< getCharYYXXX() << " M "
			<< theChan->getName() << ' '
			<< modeString << ' ' << args
			<< ends ;

		Write( s ) ;

		modeString.erase( modeString.begin(), modeString.end() ) ;
		args.erase( args.begin(), args.end() ) ;

		} // if()

	} // for()

MyUplink->OnChannelModeB( theChan, 0, banVector ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}
 
return true ;
}

bool xClient::BanKick( Channel* theChan, iClient* theClient,
	const string& reason )
{
assert( theChan != 0 ) ;
assert( theClient != 0 ) ;

if( !Connected )
	{
	return false ;
	}

if( 0 == theChan->findUser( theClient ) )
	{
	// User is not on that channel
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::BanKick> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

string banMask = Channel::createBan( theClient ) ;

Write( "%s M %s +b :%s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	banMask.c_str() ) ;

Write( "%s K %s %s :%s",
	getCharYYXXX().c_str(),
	theChan->getName().c_str(),
	theClient->getCharYYXXX().c_str(),
	reason.c_str() ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

// Update the channel's ban list
theChan->setBan( banMask ) ;

return true ;
}

bool xClient::Kick( Channel* theChan, iClient* theClient,
	const string& reason )
{
assert( theChan != NULL ) ;
assert( theClient != NULL ) ;

if( !Connected )
	{
	return false ;
	}

if( NULL == theChan->findUser( theClient ) )
	{
	elog	<< "xClient::Kick> Can't find "
		<< theClient->getNickName()
		<< " on channel "
		<< theChan->getName()
		<< endl ;
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Kick> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

stringstream s ;
s	<< getCharYYXXX() << " K "
	<< theChan->getName() << ' '
	<< theClient->getCharYYXXX() << " :"
	<< reason << ends ;

Write( s ) ;

// TODO: OnPartChannel()
if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::Kick( Channel* theChan, const vector< iClient* >& theClients,
	const string& reason )
{
assert( theChan != NULL ) ;

if( !Connected )
	{
	return false ;
	}

if( theClients.empty() )
	{
	return true ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	// Join, giving ourselves ops
	Join( theChan, string(), 0, true ) ;
	}
else
	{
	// Bot is already on the channel
	ChannelUser* meUser = theChan->findUser( me ) ;
	if( NULL == meUser )
		{
		elog	<< "xClient::Kick> Unable to find myself in "
			<< "channel: "
			<< theChan->getName()
			<< endl ;
		return false ;
		}

	// Make sure we have ops
	if( !meUser->getMode( ChannelUser::MODE_O ) )
		{
		// The bot does NOT have ops
		return false ;
		}

	// The bot has ops
	}

// We will assume that this client is on the channel pointed to by theChan

for( vector< iClient* >::const_iterator ptr = theClients.begin() ;
	ptr != theClients.end() ; ++ptr )
	{

	if( NULL == theChan->findUser( *ptr ) )
		{
		// The client is not on the channel
		continue ;
		}

	stringstream s ;
	s	<< getCharYYXXX() << " K "
		<< theChan->getName() << ' '
		<< (*ptr)->getCharYYXXX() << " :"
		<< reason << ends ;

	Write( s ) ;
	}

// TODO: OnPartChannel()
if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::Join( const string& chanName,
	const string& chanModes,
	const time_t& joinTime,
	bool getOps )
{
if( !Connected )
	{
	return false ;
	}

// Ask the server to join this bot into the given channel.
MyUplink->JoinChannel( this, chanName, chanModes, joinTime, getOps ) ;
return true ;
}

bool xClient::Join( Channel* theChan,
	const string& chanModes,
	const time_t& joinTime,
	bool getOps )
{
assert( theChan != NULL ) ;

return Join( theChan->getName(), string(), joinTime, getOps ) ;
}

bool xClient::Part( const string& chanName, const string& reason )
{

if( !Connected )
	{
	return false ;
	}

// Ask the server to part us from the channel.
MyUplink->PartChannel( this, chanName, reason ) ;

return true ;
}

bool xClient::Part( Channel* theChan )
{
assert( theChan != NULL ) ;

return Part( theChan->getName() ) ;
}

bool xClient::Invite( iClient* theClient, const string& chanName )
{
assert( theClient != NULL ) ;

Channel* theChan = Network->findChannel( chanName ) ;
if( NULL == theChan )
	{
	return false ;
	}

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	Join( theChan ) ;
	}

Write( "%s I %s %s",
	getCharYYXXX().c_str(),
	theClient->getNickName().c_str(),
	theChan->getName().c_str() ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ;
}

bool xClient::Invite( iClient* theClient, Channel* theChan )
{
assert( theClient != 0 ) ;
assert( theChan != 0 ) ;

bool OnChannel = isOnChannel( theChan ) ;
if( !OnChannel )
	{
	Join( theChan ) ;
	}

Write( "%s I %s %s",
	getCharYYXXX().c_str(),
	theClient->getNickName().c_str(),
	theChan->getName().c_str() ) ;

if( !OnChannel )
	{
	Part( theChan ) ;
	}

return true ; 
}

bool xClient::isOnChannel( const string& chanName ) const
{
return false ;
}

bool xClient::isOnChannel( const Channel* theChan ) const
{
assert( theChan != NULL ) ;
return isOnChannel( theChan->getName() ) ;
}

int xClient::Write( const char* format, ... )
{
char buf[ 4096 ] ;
memset( buf, 0, 4096 ) ;
va_list _list ;

va_start( _list, format ) ;
vsnprintf( buf, 4096, format, _list ) ;
va_end( _list ) ;

return Write( string( buf ) ) ;
}

void xClient::OnJoin( Channel* theChan )
{
assert( theChan != 0 ) ;

addChan( theChan ) ;
}

void xClient::OnJoin( const string& chanName )
{
Channel* theChan = Network->findChannel( chanName ) ;
if( NULL == theChan )
	{
	elog	<< "xClient::OnJoin> Failed to find channel: "
		<< chanName
		<< endl ;
	return ;
	}
OnJoin( theChan ) ;
}

void xClient::OnPart( Channel* theChan )
{
assert( theChan != 0 ) ;

removeChan( theChan ) ;
}

void xClient::OnPart( const string& chanName )
{
Channel* theChan = Network->findChannel( chanName ) ;
if( NULL == theChan )
	{
	elog	<< "xClient::OnPart> Failed to find channel: "
		<< chanName
		<< endl ;
	return ;
	}
OnPart( theChan ) ;
}

bool xClient::addChan( Channel* )
{
return true ;
}

bool xClient::removeChan( Channel* )
{
return true ;
}

int xClient::OnTimer( xServer::timerID ID, void* data )
{
return 0 ;
}

int xClient::OnSignal( int whichSig )
{
return 0 ;
}

} // namespace gnuworld


#include	<new>
#include	<string>
#include	<vector>
#include	<iostream>
#include	<algorithm>

#include	<cstring>

#include	"client.h"
#include	"iClient.h"
#include	"EConfig.h"
#include	"events.h"
#include	"StringTokenizer.h"
#include	"misc.h"
#include	"Network.h"
#include	"ELog.h"
#include	"libpq++.h"
#include	"nickserv.h"
#include	"nickservCommands.h"
#include	"nsUser.h"
#include	"server.h"

const char Nickserv_h_rcsId[] = __NICKSERV_H ;
const char Nickserv_cc_rcsId[] = "$Id: nickserv.cc,v 1.7 2002-01-28 22:20:05 jeekay Exp $" ;

// If __NS_DEBUG is defined, no output is ever sent to users
// this also prevents users being killed. It is intended
// to see how the module behaves in a production environment
// without actually screwing anything up if it goes wrong.
#undef __NS_DEBUG

// __NS_DEBUGINFO sends a notice to the console for every action
// taken (warning or kill). Disable this on non-test
// networks to avoid flooding everyone
// 1 - Show actions
// 2 - Show queue processing
#define __NS_DEBUGINFO 1

namespace gnuworld
{

using std::string ;
using std::vector ;
using std::cout ;
using std::endl ; 
using std::count ;

namespace nserv
{

using gnuworld::xServer;


/*
 *  Exported function used by moduleLoader to gain an
 *  instance of this module.
 */

extern "C"
{
  xClient* _gnuwinit(const string& args)
  { 
    return new nickserv( args );
  }

} 
 
nickserv::nickserv( const string& configFileName )
 : xClient( configFileName )
{

// Read the config file
EConfig conf( configFileName ) ;

string sqlHost = conf.Require("sql_host" )->second;
string sqlDb = conf.Require( "sql_db" )->second;
string sqlPort = conf.Require( "sql_port" )->second;
string sqlUser = conf.Require( "sql_user")->second;
debugChan = conf.Require("debug_chan")->second;
timeToLive = atoi((conf.Require("timeToLive")->second).c_str());
adminRefreshTime = atoi((conf.Require("adminRefreshTime")->second).c_str());

string Query = "host=" + sqlHost + " dbname=" + sqlDb + " port=" + sqlPort + " user=" +sqlUser;

elog	<< "nickserv::nickserv> Attempting to connect to "
	<< sqlHost
	<< "; Database: "
	<< sqlDb
	<< endl;
 
SQLDb = new (std::nothrow) cmDatabase( Query.c_str() ) ;
assert( SQLDb != 0 ) ;

//-- Make sure we connected to the SQL database; if
// we didn't we exit entirely.
if (SQLDb->ConnectionBad ())
	{
	elog	<< "nickserv::nickserv> Unable to connect to SQL server."
		<< endl 
		<< "nickserv::nickserv> PostgreSQL error message: "
		<< SQLDb->ErrorMessage()
		<< endl ;

	::exit( 0 ) ;
	}
else
	{
	elog	<< "nickserv::nickserv> Connection established to SQL "
		<< "server. Backend PID: "
		<< SQLDb->getPID()
		<< endl ;
	}


authLen = atoi(conf.Require("authLen")->second.c_str());


// Be sure to use all capital letters for the command name

RegisterCommand(new LOGINCommand( this, "LOGIN", "<password>")) ;
RegisterCommand(new RECOVERCommand( this, "RECOVER", "<username> <password>"));
RegisterCommand(new STATSCommand( this, "STATS", "<stat>"));

// Load all the admin names and levels
refreshAdminAccessLevels();

}


nickserv::~nickserv()
{
// Deallocate each command handler
for( commandMapType::iterator ptr = commandMap.begin() ;
	ptr != commandMap.end() ; ++ptr )
	{
	delete ptr->second ;
	ptr->second = 0 ;
	}
commandMap.clear() ;

}

// Register a command handler
bool nickserv::RegisterCommand( Command* newComm )
{
assert( newComm != NULL ) ;

// Unregister the command handler first; prevent memory leaks
UnRegisterCommand( newComm->getName() ) ;



// Insert the new handler
return commandMap.insert( pairType( newComm->getName(), newComm ) ).second ;
}

bool nickserv::UnRegisterCommand( const string& commName )
{
// Find the command handler
commandMapType::iterator ptr = commandMap.find( commName ) ;

// Was the handler found?
if( ptr == commandMap.end() )
	{
	// Nope
	return false ;
	}

// Deallocate the handler
//delete ptr->second ;

// Remove the handler
commandMap.erase( ptr ) ;

// Return success
return true ;
}

int nickserv::BurstChannels()
{

  //  Join(debugChan);
  MyUplink->JoinChannel(this, debugChan, "+ntsi");

// Don't forget to call the base class BurstChannels() method
return xClient::BurstChannels() ;

}

// I don't really like doing this.
// In order for each of this bot's Command's to have a valid server
// pointer, this method must be overloaded and server must be
// explicitly set for each Command.
void nickserv::ImplementServer( xServer* theServer )
{
for( commandMapType::iterator ptr = commandMap.begin() ;
	ptr != commandMap.end() ; ++ptr )
	{
	ptr->second->setServer( theServer ) ;
	}


// Register the events that we wish to listen for
theServer->RegisterEvent( EVT_KILL, this );
theServer->RegisterEvent( EVT_QUIT, this );
theServer->RegisterEvent( EVT_NICK , this );
theServer->RegisterEvent( EVT_CHNICK, this );
theServer->RegisterEvent( EVT_LOGGEDIN, this );

// Start the counters rolling
processQueueID = theServer->RegisterTimer(::time(NULL) + timeToLive, this, NULL);
refreshAdminID = theServer->RegisterTimer(::time(NULL) + adminRefreshTime, this, NULL);

xClient::ImplementServer( theServer ) ;
}

int nickserv::OnPrivateMessage( iClient* theClient, const string& Message,
	bool secure)
{


// Tokenize the message
StringTokenizer st( Message ) ;

// Make sure there is a command present
if( st.empty() )
	{
	Notice( theClient, "Incomplete command" ) ;
	return 0 ;
	}

// This is no longer necessary, but oh well *shrug*
const string Command = string_upper( st[ 0 ] ) ;

// Abort if someone tries to do something password related
// without doing full nick@server
 if(!secure && ((Command == "LOGIN") || (Command == "RECOVER")))
   {
     Notice(theClient, "To use %s, you must use /msg %s@%s",
	    Command.c_str(), getNickName().c_str(), getUplinkName().c_str());
     return false;
   }

// Attempt to find a handler for this method.
commandIterator commHandler = findCommand( Command ) ;

// Was a handler found?
if( commHandler == command_end() )
	{
	Notice( theClient, "Unknown command" ) ;
	return 0 ; 
	}

commHandler->second->Exec( theClient, Message) ;

return xClient::OnPrivateMessage( theClient, Message ) ;
}

int nickserv::OnCTCP( iClient* theClient, const string& CTCP
		, const string& Message ,bool Secure = false ) 
{

StringTokenizer st( CTCP );

if( st.empty()) { return 0; }

const string Command = string_upper(st[0]);

 if(Command == "PING" || Command == "ECHO")
   {
     xClient::DoCTCP(theClient, CTCP, Message);
   }

return true;
}

int nickserv::OnEvent( const eventType& theEvent,
	void* Data1, void* Data2, void* Data3, void* Data4 )
{

switch( theEvent )
	{
	case EVT_LOGGEDIN:
	  {
	    // Data1 = iClient*, Data2 = string*
	    iClient* theClient = static_cast< iClient* >( Data1);
	    string* authNick = static_cast< string* >( Data2);
	    authUser(theClient, *authNick);
	    if(string_lower(theClient->getNickName()) == string_lower(*authNick))
	      {
		removeFromQueue(theClient);
	      }
	    break;
	  }
	case EVT_QUIT:
	case EVT_KILL:
		{
		iClient* NewUser;
		if(theEvent == EVT_QUIT)
		  { NewUser = static_cast< iClient* >( Data1); }
		else
		  { NewUser = static_cast< iClient* >( Data2); }
		removeFromQueue(NewUser);
		nsUser* tmpUser = static_cast< nsUser* >( NewUser->getCustomData(this));
		delete tmpUser;
		break ;
		} // case EVT_KILL/case EVT_QUIT
	
	case EVT_NETJOIN:
		{
		break;
		}
	case EVT_NETBREAK:
		{
		break;
		}
	case EVT_BURST_CMPLT:
		{
		break;
		}	
	case EVT_NICK:
		{ // New nick being introduced
		iClient* NewUser = static_cast< iClient* >( Data1);
		nsUser* tmpUser = new nsUser(NewUser->getCharYYXXX(), ::time(NULL), NewUser->getNickName());
		NewUser->setCustomData(this, static_cast< void* >( tmpUser ) );
		KillingQueue[NewUser->getCharYYXXX()] = tmpUser;
		break;
		}
	case EVT_CHNICK:
	  { // User changing nick. Remove from queue if present
	  iClient* NewUser = static_cast< iClient* >( Data1);
	  nsUser* tmpUser = static_cast< nsUser* >( NewUser->getCustomData(this));
	  removeFromQueue(NewUser);

	  // Is the new nick the authed nick? If so, dont add to queue
	  if(tmpUser->getLoggedIn() && (tmpUser->getLoggedNick() == NewUser->getNickName())) { return 0; }
	
	  // Create new custom data, readd to queue for verification
	  tmpUser->SetData(NewUser->getCharYYXXX(), ::time(NULL), NewUser->getNickName());
	  KillingQueue[NewUser->getCharYYXXX()] = tmpUser;
    break;
	  } 
	} // switch()

return 0;

return xClient::OnEvent( theEvent, Data1, Data2, Data3, Data4 ) ;
}

int nickserv::OnChannelEvent( const channelEventType& theEvent,
	Channel* theChan,
	void* Data1, void* Data2, void* Data3, void* Data4 )
{

switch( theEvent )
	{
	//case EVT_JOIN:
	}

// Call the base class OnChannelEvent()
return xClient::OnChannelEvent( theEvent, theChan,
	Data1, Data2, Data3, Data4 ) ;
}

int nickserv::OnTimer(xServer::timerID timer_id, void* data)
{ // OnTimer

  if((timer_id == processQueueID) && (MyUplink->IsEndOfBurst()))
	{ // processQueueID
	  strstream strTemp;
	  strTemp << "Processing kill queue - "
	          << KillingQueue.size()
	          << " entr" << ((KillingQueue.size() == 1) ? ("y") : ("ies")) << "." << ends;
	  logDebugMessage(strTemp.str());
		delete[] strTemp.str();
		unsigned int warnings = 0;
		unsigned int kills = 0;
		for(killIterator pos = KillingQueue.begin(); pos != KillingQueue.end(); )
		{ // Iterative loop
			// Quick sanity checking
		  nsUser* tmpNS = static_cast< nsUser* >( pos->second);
			iClient* tmpClient = Network->findClient(tmpNS->getNumeric());
			
			// Sanity checking
			if(!tmpClient || !tmpNS)
			{ // Somehow we have a non-existant numeric
				logDebugMessage(string("Wierd error with numeric ") + tmpNS->getNumeric());
				KillingQueue.erase(pos++);
				continue;
			}
			
#if __NS_DEBUGINFO >= 2
			strstream debugString;
			debugString << "Processing numeric " << tmpNS->getNumeric() << ends;
			logDebugMessage(debugString.str());
			delete[] debugString.str();
#endif
			if(! (tmpNS->getInQueue()) )
			{ // This user is not in the queue
				if(checkUser(tmpNS))
				{ // Nick is registered
					tmpNS->clearFlags();
					tmpNS->setInQueue();
					tmpNS->setCheckTime();
#if __NS_DEBUGINFO >= 1
					logDebugMessage(tmpNS->getNumeric() + " - Warned");
#endif
					++warnings;
#ifndef __NS_DEBUG
					Notice(tmpClient, "Your nickname is registered. Please login or change your nick.");
#endif
				} // Nick is registered
				else
				{ // Nick is not registered
					KillingQueue.erase(pos++);
					continue;
				} // Nick is not registered
				++pos;
	      continue;
			} // User not in queue
	  	else
			{ // User has INQUEUE set
				// Check time
				if((::time(NULL) - (tmpNS->getCheckTime())) >= timeToLive)
				{ // User has expired their time. Kill them.
#if __NS_DEBUGINFO >= 1
					logDebugMessage(tmpClient->getCharYYXXX() + " - Killed");
#endif
					++kills;

#ifndef __NS_DEBUG
					Notice(tmpClient, "You have not logged into NickServ. You will now be autokilled.");
					strstream s;
					s << getCharYY()
					  << " D "
					  << tmpClient->getCharYYXXX()
					  << " :" << getNickName() << " [NickServ] AutoKill" << ends;
					Write(s.str());
					delete[] s.str();
					
					MyUplink->PostEvent(gnuworld::EVT_NSKILL, static_cast<void*>(tmpClient));
					
					// Remove GNUworld data about user
					nsUser *tmpData = static_cast< nsUser* >( tmpClient->getCustomData(this) );
					delete tmpData;
					tmpClient->removeCustomData(this);
#endif
					
					KillingQueue.erase(pos++);
					continue;
				} // User has expired their time.
				++pos;
			} // User has INQUEUE set
		} // Iterative loop

		strstream summaryString;
		summaryString << "Warnings: " << warnings << "; Kills: " << kills << ends;
		logDebugMessage(summaryString.str());
		delete[] summaryString.str();
		processQueueID = MyUplink->RegisterTimer(::time(NULL) + timeToLive, this, NULL);
	} // processQueueID

  if(timer_id == refreshAdminID)
    {
      refreshAdminAccessLevels();
      refreshAdminID = MyUplink->RegisterTimer(::time(NULL) + adminRefreshTime, this, NULL);
    }

return true;
}

// This method does NOT add the channel to any internal tables
bool nickserv::Join( const string& chanName, const string& chanModes,
	time_t joinTime, bool getOps )
{
if( isOnChannel( chanName ) )
	{
	// Already on this channel
	return true ;
	}
bool result = xClient::Join( chanName, chanModes, joinTime, getOps ) ;
if( result )
	{
	MyUplink->RegisterChannelEvent( chanName, this ) ;
	}

return result ;
}

bool nickserv::Part( const string& chanName )
{

bool result = xClient::Part( chanName ) ;
if( result )
	{
	MyUplink->UnRegisterChannelEvent( chanName, this ) ;
	}

return result ;
}

bool nickserv::Kick( Channel* theChan, iClient* theClient,
	const string& reason )
{
assert( theChan != NULL ) ;

if( !isOnChannel( theChan->getName() ) )
	{
	return false ;
	}

return xClient::Kick( theChan, theClient, reason ) ;
}

bool nickserv::checkUser(nsUser* tmpUser)
{
unsigned short int userFlags;

ExecStatusType status;
strstream s;
s << "SELECT id,user_name,flags from Users Where lower(user_name) = '"
	    << string_lower(tmpUser->getNickName()) << "'" << ends;
status = SQLDb->Exec(s.str());

delete[] s.str();

if(PGRES_TUPLES_OK == status)
	{
	if(SQLDb->Tuples() > 0)
		{
		  userFlags = atoi(SQLDb->GetValue(0, 2));
		  return userFlags & F_AUTOKILL;
		}
	return false;
	}
elog << " Query error!" << endl;
return false;
}

nsUser* nickserv::isAuth(iClient* tmpClient)
{
nsUser* tmpUser = static_cast < nsUser* >( tmpClient->getCustomData(this));
if(tmpUser->getLoggedIn())
	return tmpUser;
return NULL;

}

void nickserv::authUser(iClient* tmpClient, const string& authNick)
{
nsUser* tmpUser = static_cast < nsUser* >( tmpClient->getCustomData(this));
tmpUser->setLoggedIn();
tmpUser->setLoggedNick(authNick);
if(int tmpAccess = getAdminAccessLevel(authNick))
  {
    Notice(tmpClient, "Authed with NS admin level: %d", tmpAccess);
  }
}

void nickserv::removeFromQueue(iClient* theClient)
{
  KillingQueue.erase(theClient->getCharYYXXX());
}

bool nickserv::isInQueue(iClient* tmpClient)
{
nsUser* tmpUser = static_cast < nsUser* >( tmpClient->getCustomData(this));
return tmpUser->getInQueue();
}

bool nickserv::logDebugMessage(const string& theMessage)
{
  // Check we live
  Channel* tmpChan = Network->findChannel(debugChan);
  if(!tmpChan)
    {
      elog << "nickserv::logDebugMessage> Unable to find debug channel ";
      elog << debugChan;
      elog << endl;
      return false;
    }

  ChannelUser* tmpChanUser = tmpChan->findUser(this->getInstance());
  if(!tmpChanUser)
    {
      elog << "nickserv::logDebugMessage> We do not appear to be in the debug";
      elog << " channel " << debugChan << endl;
      return false;
    }

  // We are alive and in the right channel. Lets message it!
  this->Message(tmpChan, theMessage + "\n");
  return true;

}

void nickserv::refreshAdminAccessLevels( void )
{
  strstream theQuery;
  theQuery << "SELECT users.user_name,levels.access "
           << "FROM users,levels,channels "
           << "WHERE channels.name='" << debugChan << "' "
           << "AND channels.id=levels.channel_id "
           << "AND users.id=levels.user_id" << ends;

  ExecStatusType status = SQLDb->Exec(theQuery.str());
  delete[] theQuery.str();

  adminList.clear();

  if( PGRES_TUPLES_OK == status )
  {
    for(int i = 0; i < SQLDb->Tuples(); i++)
    {
      // Add new admin information to cache
      string newAdmin = SQLDb->GetValue(i, 0);
      short int newLevel = atoi(SQLDb->GetValue(i,1));
      adminList[string_lower(newAdmin)] = newLevel;
    }
  }

  elog << "nickserv::refreshAdminAccessLevels> Refresh complete" << endl;

  strstream dbgMsg;
  dbgMsg << "ns::rAAL> Admin map has been refreshed - Total entries: " << adminList.size() << ends;

  logDebugMessage(dbgMsg.str());

  delete[] dbgMsg.str();

  return;
}

short int nickserv::getAdminAccessLevel( string theNick )
{
  adminIteratorType pos;
  pos = adminList.find(string_lower(theNick));
  if(pos == adminList.end())
    {
      return 0;
    }
  else
    {
      return pos->second;
    }
}

} // namespace nserv

const string gnuworld::escapeSQLChars(const string& theString)
{
  string retMe ;

  for( string::const_iterator ptr = theString.begin() ;
       ptr != theString.end() ; ++ptr )
    {
      if( *ptr == '\'' )
	{
	  retMe += "\\\047" ;
	}
      else if ( *ptr == '\\' )
	{
	  retMe += "\\\134" ;
	}
        else
	  {
	    retMe += *ptr ;
	  }
    }
  return retMe ;
}


} // namespace gnuworld



#include	<new>
#include	<string>
#include	<vector>
#include	<iostream>
#include	<algorithm>

#include	<cstring>
#include <time.h>

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
const char Nickserv_cc_rcsId[] = "$Id: nickserv.cc,v 1.26 2002-04-01 18:31:28 jeekay Exp $" ;

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

confSqlHost = conf.Require("sql_host" )->second;
confSqlDb = conf.Require( "sql_db" )->second;
confSqlPort = conf.Require( "sql_port" )->second;
confSqlUser = conf.Require( "sql_user" )->second;
confSqlPass = conf.Require( "sql_pass" )->second;

dbConnCheckTime = atoi((conf.Require("dbConnCheckTime")->second).c_str());
dbConnCheckMax = atoi((conf.Require("dbConnCheckMax")->second).c_str());

debugChan = conf.Require("debug_chan")->second;

timeToLive = atoi((conf.Require("timeToLive")->second).c_str());
initialWait = atoi((conf.Require("initialWait")->second).c_str());
checkNickMax = atoi((conf.Require("checkNickMax")->second).c_str());

jupeNumericStart = atoi((conf.Require("jupeNumericStart")->second).c_str());
jupeNumericCount = atoi((conf.Require("jupeNumericCount")->second).c_str());
jupeExpireTime = atoi((conf.Require("jupeExpireTime")->second).c_str());
jupeDefaultLength = atoi((conf.Require("jupeDefaultLength")->second).c_str());

nickCService = conf.Require("cserviceNick")->second;

string Query = "host=" + confSqlHost + " dbname=" + confSqlDb + " port=" + confSqlPort + " user=" +confSqlUser+ " password="+confSqlPass;

elog	<< "nickserv::nickserv> Attempting to connect to "
	<< confSqlHost
	<< "; Database: "
	<< confSqlDb
	<< endl;
 
SQLDb = new (std::nothrow) nsDatabase( Query.c_str() ) ;
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


// Be sure to use all capital letters for the command name

RegisterCommand(new JUPECommand( this, "JUPE", "(ADD|DEL|FORCEADD|INFO) <nick> (duration) (reason)"));
RegisterCommand(new RELEASECommand( this, "RELEASE", "<nick>"));
RegisterCommand(new STATSCommand( this, "STATS", "<stat>"));
RegisterCommand(new SAYCommand( this, "SAY", "<channel> <text>"));

// Initialise list of juped numerics
initialiseJupeNumerics();

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
theServer->RegisterEvent( EVT_FORCEDEAUTH, this );

// Start the counters rolling
processQueueID = theServer->RegisterTimer(::time(NULL) + timeToLive + initialWait, this, NULL);
if(!processQueueID)
	elog << "nickserv::ImplementServer> Unable to register timer for processQueueID" << endl;

dbConnCheckID  = theServer->RegisterTimer(::time(NULL) + dbConnCheckTime, this, NULL);
if(!dbConnCheckID)
	elog << "nickserv::ImplementServer> Unable to register timer for dbConnCheckID" << endl;

jupeExpireID = theServer->RegisterTimer(::time(NULL) + jupeExpireTime, this, NULL);
if(!jupeExpireID)
	elog << "nickserv::ImplementServer> Unable to register timer for jupeExpireID" << endl;

if(!processQueueID || !dbConnCheckID || !jupeExpireID)
	::exit(0);

dbConnRetries = 0;

xClient::ImplementServer( theServer ) ;
}

int nickserv::OnConnect()
{

// Find our CService!

myCService = static_cast< gnuworld::cservice* >(Network->findLocalNick(nickCService));

if(!myCService)
	{
	elog << "nickserv::nickserv> Unable to find an instance of CService running." << endl;
	::exit(0);
	}
else
	{
	elog << "nickserv::nickserv> Found CService at numeric: " << myCService->getCharYYXXX() << endl;
	}

return xClient::OnConnect();
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
	case EVT_FORCEDEAUTH:
		{
		/* Data1 = iClient*
		 * P has forcibly deauthed someone (clone probably)
		 * We need to remove that iClient's current logged nick
		 */
		iClient* theClient = static_cast< iClient* >( Data1);
		nsUser* theUser = static_cast< nsUser* >( theClient->getCustomData(this));
		theUser->clearLoggedNick();
		KillingQueue[theClient->getCharYYXXX()] = theUser;
		break;
		}
	case EVT_LOGGEDIN:
	  {
	  // Data1 = iClient*, Data2 = string*
	  iClient* theClient = static_cast< iClient* >( Data1);
	  string* authNick = static_cast< string* >( Data2);
	  authUser(theClient, *authNick);
	  if(string_lower(theClient->getNickName()) == string_lower(*authNick))
	    {	removeFromQueue(theClient); }
		if(string_lower(theClient->getNickName()) != string_lower(*authNick))
			{ removeJupeNick(*authNick, "User Logged In"); }
	  break;
	  } // case EVT_LOGGEDIN
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
	  if(tmpUser->getLoggedIn() && (string_lower(tmpUser->getLoggedNick()) == string_lower(NewUser->getNickName()))) { return 0; }
	
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
		processKillQueue();
		processQueueID = MyUplink->RegisterTimer(::time(NULL) + timeToLive, this, NULL);
	} // processQueueID

	if(timer_id == dbConnCheckID)
		{
		checkDBConnectionStatus();
		dbConnCheckID = MyUplink->RegisterTimer(::time(NULL) + dbConnCheckTime, this, NULL);
		}
	
	if(timer_id == jupeExpireID)
		{
		checkJupeExpire();
		jupeExpireID = MyUplink->RegisterTimer(::time(NULL) + jupeExpireTime, this, NULL);
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
		  return userFlags & NS_F_AUTOKILL;
		}
	return false;
	}
elog << " Query error!" << endl;
return false;
}

void nickserv::authUser(iClient* tmpClient, const string& authNick)
{
nsUser* tmpUser = static_cast < nsUser* >( tmpClient->getCustomData(this));
tmpUser->setLoggedIn();
tmpUser->setLoggedNick(authNick);
}

void nickserv::removeFromQueue(iClient* theClient)
{
  KillingQueue.erase(theClient->getCharYYXXX());
}

bool nickserv::logDebugMessage(const char* format, ...)
{
char buf[1024] = { 0 };
va_list _list;

va_start(_list, format);
vsnprintf(buf, 1024, format, _list);
va_end(_list);

Channel* tmpChan = Network->findChannel(debugChan);
if(!tmpChan)
  {
    elog << "nickserv::logDebugMessage> Unable to find debug channel "
    	<< debugChan << endl;
    return false;
  }

Message(debugChan, buf);
return true;
}

void nickserv::initialiseJupeNumerics( void )
{
	elog << "nickserv::iJN> Initialising jupedNicks map" << endl;
	char clientNumeric[4];
	for(unsigned int i = jupeNumericStart; i < (jupeNumericStart + jupeNumericCount); i++)
		{
		inttobase64(clientNumeric, i, 3);
		jupedNickList[string(clientNumeric)] = NULL;
		}
}

bool nickserv::jupeNick( string theNick, string hostMask, string theReason, time_t duration )
{
	elog << "nickserv::jupeNick> Juping nick " << theNick << endl;
	
	if(0 == duration) { duration = jupeDefaultLength; }
	
	// Check this user does not already exist here
	for(jupeIteratorType pos = jupedNickList.begin(); pos != jupedNickList.end(); ++pos)
		{
		juUser* theUser = pos->second;
		if(theUser && string_lower(theUser->getNickName()) == string_lower(theNick))
			{ return false; }
		}
	
	for(jupeIteratorType pos = jupedNickList.begin(); pos != jupedNickList.end(); ++pos)
		{
		if(NULL == pos->second)
			{ // We have found a free numeric!
			juUser* theUser = new juUser(theNick, pos->first, ::time(NULL), ::time(NULL)+duration, theReason, hostMask);
			pos->second = theUser;
			
			strstream outNick;
			outNick << charYY << " N " << theNick
							<< " 1 31337 juped " << theNick << ".nick.name +idk B]AAAB "
							<< charYY << pos->first
							<< " :" << theReason << ends;
			Write(outNick.str());
			delete[] outNick.str();
			
			Channel* theChan = Network->findChannel(debugChan);
			if(theChan)
				{
				strstream outJoin;
				outJoin << charYY << pos->first << " J "
								<< debugChan << ends;
				Write(outJoin.str());
				delete[] outJoin.str();
				} // Debugchan exists
			return true;
			} // Empty numeric
		} // Iteration
	return false;
}

bool nickserv::removeJupeNick( string theNick, string theReason = "End Of Jupe" )
{
	elog << "nickserv::removeJupeNick> Removing jupe for " << theNick << endl;
	for(jupeIteratorType pos = jupedNickList.begin(); pos != jupedNickList.end(); ++pos)
		{
		juUser* theUser = pos->second;
		if(theUser && (string_lower(theUser->getNickName()) == string_lower(theNick)))
			{ // This is our nick
			strstream outQuit;
			outQuit << charYY << pos->first << " Q :" << theReason << ends;
			Write(outQuit.str());
			delete[] outQuit.str();
			
			delete theUser;
			pos->second = NULL;
			return true;
			} // Right nick?
		} // Iteration
	return false;
}

juUser* nickserv::findJupeNick( string theNick )
{
	for(jupeIteratorType pos = jupedNickList.begin(); pos != jupedNickList.end(); ++pos)
		{
		juUser* theUser = pos->second;
		if(theUser && (string_lower(theUser->getNickName()) == string_lower(theNick)))
			{ return theUser; }
		}
	return NULL;
}

void nickserv::checkJupeExpire( void )
{
	time_t timeNow = ::time(NULL);
	for(jupeIteratorType pos = jupedNickList.begin(); pos != jupedNickList.end(); ++pos)
		{
		juUser* theUser = pos->second;
		if(theUser && (timeNow > *(theUser->getExpires())))
			{
			strstream outQuit;
			outQuit << charYY << pos->first << " Q :Jupe Expired" << ends;
			Write(outQuit.str());
			delete[] outQuit.str();
			
			delete theUser;
			pos->second = NULL;
			}
		}
}

void nickserv::checkDBConnectionStatus( void )
{
	if(SQLDb->Status() == CONNECTION_BAD)
		{
		logDebugMessage("\002WARNING:\002 Backend database connection has been lost, attempting to reconnect.");
		elog << "nickserv::nickserv> Database connection died. Attempting to reconnect." << endl;
		
		// Remove old SQL object
		delete(SQLDb);
		
		string Query = "host=" + confSqlHost +  " dbname=" + confSqlDb + " port=" + confSqlPort + " user=" + confSqlUser + " password=" + confSqlPass;
		
		SQLDb = new (std::nothrow) nsDatabase( Query.c_str() );
		assert( SQLDb != 0);
		
		if(SQLDb->ConnectionBad())
			{
			elog << "nickserv::nickserv> Unable to connect to SQL server." << endl;
			elog << "nickserv::nickserv> PostgreSQL error message: " << SQLDb->ErrorMessage() << endl;
			
			++dbConnRetries;
			if(dbConnRetries >= dbConnCheckMax)
				{
				logDebugMessage("\002ERROR:\002 Unable to reconnect to database.");
				MyUplink->flushBuffer();
				::exit(0);
				} // Max retries exceeded
			else
				{
				logDebugMessage("\002WARNING:\002 Connection failed. Retrying.");
				} // Retrying
			} // Bad connection still
		else
			{
			logDebugMessage("Successfully reconnected to the database server. Panic over :)");
			dbConnRetries = 0;
			} // Good connection now
		} // Bad connection found
} // nickserv::checkDBConnectionStatus

int nickserv::getAdminAccessLevel( iClient* theClient )
{
// We are guaranteed that iClient is NOT null

// Attempt to get admin access in #ns.console
sqlChannel* csChan = myCService->getChannelRecord(debugChan);
if(!csChan)
	{ // NS debug chan isnt registered
	elog << "nickserv> " << debugChan.c_str()
		<< " isnt registered with CService." << endl;
	return false;
	}

sqlUser* csUser = myCService->isAuthed(theClient, false);
if(!csUser) { return false; }

return myCService->getEffectiveAccessLevel(csUser, csChan, false);
} // nickserv::getAdminAccessLevel

void nickserv::processKillQueue( void )
{
clock_t startTime = ::clock();
clock_t endTime = 0;

logDebugMessage("Processing kill queue - %d entr%s.",
	KillingQueue.size(), (KillingQueue.size() == 1) ? ("y") : ("ies"));

unsigned int warnings = 0;
unsigned int kills = 0;
unsigned int iterations = 0;

for(killIterator pos = KillingQueue.begin(); pos != KillingQueue.end(); )
{ // Iterative loop
	if(iterations >= checkNickMax) break; else ++iterations;
	// Quick sanity checking
	nsUser* tmpNS = static_cast< nsUser* >( pos->second);
	iClient* tmpClient = Network->findClient(tmpNS->getNumeric());
			
	// Sanity checking
	if(!tmpClient || !tmpNS)
	{ // Somehow we have a non-existant numeric
		logDebugMessage("Wierd error with numeric %s", tmpNS->getNumeric().c_str());
		KillingQueue.erase(pos++);
		continue;
	}
			
#if __NS_DEBUGINFO >= 2
	logDebugMessage("Processing numeric %s", tmpNS->getNumeric().c_str());
#endif

	if(! (tmpNS->getInQueue()) )
	{ // This user is not in the queue
		if(checkUser(tmpNS))
		{ // Nick is registered
			tmpNS->clearFlags();
			tmpNS->setInQueue();
			tmpNS->setCheckTime();
#if __NS_DEBUGINFO >= 1
			logDebugMessage("%s - Warned", tmpNS->getNumeric().c_str());
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
			logDebugMessage("%s - Killed", tmpClient->getCharYYXXX().c_str());
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
					
			// Remove GNUworld data about user
			nsUser *tmpData = static_cast< nsUser* >( tmpClient->getCustomData(this) );
			delete tmpData;
			tmpClient->removeCustomData(this);
					
			jupeNick(tmpClient->getNickName(), tmpClient->getNickUserHost(), "AutoKill Juped Nick", 0);

			MyUplink->PostEvent(gnuworld::EVT_NSKILL, static_cast<void*>(tmpClient));
#endif
					
			KillingQueue.erase(pos++);
			continue;
			} // User has expired their time.
		++pos;
		} // User has INQUEUE set
	} // Iterative loop

endTime = ::clock();
logDebugMessage("Processed: %d; Warnings: %d; Kills: %d; Duration: %d ms",
	iterations, warnings, kills, (endTime - startTime) / CLOCKS_PER_SEC);

return;
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


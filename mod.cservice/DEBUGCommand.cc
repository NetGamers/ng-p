/* DEBUGCommand.cc - Various bits of coder debug info
 *
 * (c) Copyright 2002 Rasmus Hansen (GK@panet)
 *
 * Distributed under the GNU Public Licence
 *
 * $Id: DEBUGCommand.cc,v 1.6 2004-05-16 13:08:16 jeekay Exp $
 */

#include	<string>

#include "StringTokenizer.h"
#include "ELog.h"
#include "Network.h"

#include "cservice.h"

const char DEBUGCommand_cc_rcsId[] = "$Id: DEBUGCommand.cc,v 1.6 2004-05-16 13:08:16 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void DEBUGCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.DEBUG");

// DEBUG servers
// DEBUG lock list
// DEBUG lock [add|remove] command reason
StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return ; }

int aLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* debugCommandLevel = bot->getLevelRequired("DEBUG", "ADMIN");

if(aLevel < debugCommandLevel->getLevel())
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
	return ;
	}

string command = string_upper(st[1]);

/* DEBUG servers
 * List all servers, their uplinks and their burst status
 * YY:name->uplinkYY:(0|1)
 */

if("SERVERS" == command)
	{
	//xNetwork::serverVectorType::const_iterator netServers;
	xNetwork::const_serverIterator netServers;
	netServers = Network->server_begin();
	while(netServers != Network->server_end())
		{
		iServer* tempServer = (*netServers).second;
		if(tempServer != 0)
			{
			char uplinkYY[3];
			inttobase64(uplinkYY, tempServer->getUplinkIntYY(), 2);
			bot->Notice(theClient, "%s:%s->%s:%d",
				tempServer->getCharYY(),
				tempServer->getName().c_str(),
				uplinkYY,
				tempServer->isBursting());
			}
		++netServers;
		}
	return ;
	}

if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}

string option = string_upper(st[2]);

if("LOCK" == command && "LIST" == option)
	{
	bot->Notice(theClient, "Locked commands list:");
	cservice::lockedCommandsType::const_iterator myCommands;
	myCommands = bot->lockedCommands.begin();
	while(myCommands != bot->lockedCommands.end())
		{
		bot->Notice(theClient, "%s: %s",
			myCommands->first.c_str(), myCommands->second.c_str());
		++myCommands;
		}
		
	return ;
	}

if ( st.size() < 5 )
	{
	Usage(theClient);
	return ;
	}

string function = string_upper(st[3]);
string data = st.assemble(4);

if("LOCK" == command && "ADD" == option)
	{
	if("DEBUG" == function)
		{
		bot->Notice(theClient, "I don't think that'd be a very good idea.");
		return ;
		}
	
	cservice::lockedCommandsType::const_iterator myCommand;
	myCommand = bot->lockedCommands.find(function);
	if(myCommand != bot->lockedCommands.end())
		{
		bot->Notice(theClient, "%s is already locked", function.c_str());
		return ;
		}
	
	bot->lockedCommands[function] = data;
	bot->Notice(theClient, "Locked %s because %s",
		function.c_str(), data.c_str());
	
	bot->logAdminMessage("%s (%s) - DEBUG LOCK - %s - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		function.c_str(), data.c_str());
	
	return ;
	}

if("LOCK" == command && "REM" == option)
	{
	cservice::lockedCommandsType::const_iterator myCommand;
	myCommand = bot->lockedCommands.find(function);
	if(myCommand == bot->lockedCommands.end())
		{
		bot->Notice(theClient, "%s is not locked", function.c_str());
		return ;
		}
	
	bot->lockedCommands.erase(myCommand->first);
	bot->Notice(theClient, "Unlocked %s because %s",
		function.c_str(), data.c_str());
	
	bot->logAdminMessage("%s (%s) - DEBUG UNLOCK - %s - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		function.c_str(), data.c_str());
	
	return ;
	}

Usage(theClient);

return ;
} // DEBUGCommand::Exec

} // namespace gnuworld.

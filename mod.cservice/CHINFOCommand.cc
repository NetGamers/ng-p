/* CHINFOCommand.cc - Allow admins to change user information
 *
 * (c) Copyright 2002 Rasmus Hansen (GK@panet)
 *
 * Distributed under the GNU Public Licence
 *
 * $Id: CHINFOCommand.cc,v 1.2 2002-03-24 02:00:55 jeekay Exp $
 */

#include	<string>

#include "StringTokenizer.h"
#include "ELog.h"
#include "cservice.h"
#include "levels.h"

const char CHINFOCommand_cc_rcsId[] = "$Id: CHINFOCommand.cc,v 1.2 2002-03-24 02:00:55 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

bool CHINFOCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.CHINFO");

// CHINFO [email|user_name|verification] nick newdata
StringTokenizer st( Message ) ;
if( st.size() != 4 )
	{
	Usage(theClient);
	return true;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return false; }

string option = string_upper(st[1]);
string target = st[2];
string newdata = st[3];

int aLevel = bot->getAdminAccessLevel(theUser);
sqlUser* targetUser = bot->getUserRecord(target);
if(!targetUser)
	{
	bot->Notice(theClient, "%s is not registered with me.", target.c_str());
	return false;
	}

if((aLevel >= level::chinfo::email) && ("EMAIL" == option))
	{
	targetUser->setEmail(newdata);
	targetUser->commit();
	
	bot->Notice(theClient, "Changed %s's %s to %s",
		targetUser->getUserName().c_str(), option.c_str(), newdata.c_str());
	bot->logAdminMessage("%s (%s) changed %s's %s to %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		targetUser->getUserName().c_str(), option.c_str(), newdata.c_str());
	return true;
	}

if((aLevel >= level::chinfo::verification) && ("VERIFICATION" == option))
	{
	targetUser->setVerificationData(newdata);
	targetUser->commit();
	
	bot->Notice(theClient, "Changed %s's %s to %s",
		targetUser->getUserName().c_str(), option.c_str(), newdata.c_str());
	bot->logAdminMessage("%s (%s) changed %s's %s to %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		targetUser->getUserName().c_str(), option.c_str(), newdata.c_str());
	return true;
	}

if((aLevel >= level::chinfo::nick) && ("NICK" == option))
	{
	/* First things first - is the new nick already taken? */
	sqlUser* newUser = bot->getUserRecord(newdata);
	if(newUser)
		{
		bot->Notice(theClient, "Sorry, the nick %s is already in use.",
			newUser->getUserName().c_str());
		return false;
		}
	
	/* Next - check legality of new nick */
	if(newdata[0] == '-' ||
		newdata.size() < 3)
		{
		bot->Notice(theClient, "Sorry, %s is an illegal nickname.",
			newdata.c_str());
		return false;
		}
	
	/* Save copy of current name */
	string userName = targetUser->getUserName();
	
	/* First, we change the database */
	targetUser->setUserName(newdata);
	targetUser->commit();
	
	/* Then we check if the user is logged on */
	iClient* targetClient = targetUser->isAuthed();
	if(targetClient)
		{
		bot->Notice(targetClient, "Your registered nick has been changed from %s to %s by %s (%s)",
			userName.c_str(), newdata.c_str(), theClient->getNickName().c_str(),
			theUser->getUserName().c_str());
		}
	
	/* Then we update the cache */
	cservice::sqlUserHashType::iterator ptr = bot->sqlUserCache.find(target);
	if(ptr != bot->sqlUserCache.end())
		{
		/* We should always get here as we just getUserRecord()'d */
		bot->sqlUserCache[newdata] = bot->sqlUserCache[ptr->first];
		bot->sqlUserCache.erase(ptr);
		}
	
	bot->Notice(theClient, "Changed %s's %s to %s",
		userName.c_str(), option.c_str(), newdata.c_str());
	bot->logAdminMessage("%s (%s) changed %s's %s to %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		userName.c_str(), option.c_str(), newdata.c_str());
	return true;
	}

Usage(theClient);

return true;
} // CHINFOCommand::Exec

} // namespace gnuworld.

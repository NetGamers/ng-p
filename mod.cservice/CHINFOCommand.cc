/* CHINFOCommand.cc - Allow admins to change user information
 *
 * (c) Copyright 2002 Rasmus Hansen (GK@panet)
 *
 * Distributed under the GNU Public Licence
 *
 * $Id: CHINFOCommand.cc,v 1.5 2002-09-24 20:06:17 jeekay Exp $
 */

#include	<string>

#include "StringTokenizer.h"
#include "ELog.h"
#include "cservice.h"
#include "levels.h"

const char CHINFOCommand_cc_rcsId[] = "$Id: CHINFOCommand.cc,v 1.5 2002-09-24 20:06:17 jeekay Exp $" ;

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

int targetLevel = bot->getAdminAccessLevel(targetUser);
if(targetLevel && (aLevel < level::chinfo::admin))
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
	return false;
	}

/****************
 * CHINFO EMAIL *
 ****************/

if((aLevel >= level::chinfo::email) && ("EMAIL" == option))
	{
	targetUser->setEmail(newdata);
	targetUser->commit();
	
	bot->Notice(theClient, "Changed %s's %s to %s",
		targetUser->getUserName().c_str(), option.c_str(), newdata.c_str());
	bot->logAdminMessage("%s (%s) - CHINFO - EMAIL - %s to %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		targetUser->getUserName().c_str(), newdata.c_str());
	return true;
	}

/***********************
 * CHINFO VERIFICATION *
 ***********************/

if((aLevel >= level::chinfo::verification) && ("VERIFICATION" == option))
	{
	targetUser->setVerificationData(newdata);
	targetUser->commit();
	
	bot->Notice(theClient, "Changed %s's %s to %s",
		targetUser->getUserName().c_str(), option.c_str(), newdata.c_str());
	bot->logAdminMessage("%s (%s) - CHINFO - VERIFICATION - %s to %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		targetUser->getUserName().c_str(), newdata.c_str());
	return true;
	}

/***************
 * CHINFO NICK *
 ***************/

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
	if(targetUser->isAuthed())
		{
		bot->noticeAllAuthedClients(targetUser, "Your registered nick has been changed from %s to %s by %s (%s)",
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
	bot->logAdminMessage("%s (%s) - CHINFO - NICK - %s to %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		userName.c_str(), newdata.c_str());
	return true;
	}

Usage(theClient);

return true;
} // CHINFOCommand::Exec

} // namespace gnuworld.

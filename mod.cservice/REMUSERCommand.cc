/*
 * REMUSERCommand.cc
 *
 * 27/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Removes a users access from a particular channel.
 *
 * Caveats: None
 *
 *
 * $Id: REMUSERCommand.cc,v 1.7 2002-10-20 02:12:08 jeekay Exp $
 */

#include	<string>

#include	"ELog.h"
#include	"libpq++.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"

const char REMUSERCommand_cc_rcsId[] = "$Id: REMUSERCommand.cc,v 1.7 2002-10-20 02:12:08 jeekay Exp $" ;

namespace gnuworld
{

using std::ends;

bool REMUSERCommand::Exec( iClient* theClient, const string& Message )
{
	bot->incStat("COMMANDS.REMUSER");

	StringTokenizer st( Message ) ;
	if( st.size() < 3 )
	{
		Usage(theClient);
		return true;
	}

	static const char* queryHeader = "DELETE FROM levels WHERE ";

	stringstream theQuery;
	ExecStatusType status;

	/*
	 *  Fetch the sqlUser record attached to this client. If there isn't one,
	 *  they aren't logged in - tell them they should be.
	 */

	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return false;

 	/*
	 *  First, check the channel is registered.
	 */

	sqlChannel* theChan = bot->getChannelRecord(st[1]);
	if (!theChan) {
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::chan_not_reg,
				string("Sorry, %s isn't registered with me.")).c_str(),
			st[1].c_str());
		return false;
	}

#ifdef FEATURE_FORCELOG
	unsigned short forcedAccess = bot->isForced(theChan, theUser);
	if (forcedAccess <= 900  && forcedAccess > 0)
        	{
	        bot->writeForceLog(theUser, theChan, Message);
        	}

#endif

	/*
	 *  Check the user has sufficient access on this channel.
	 */
	sqlUser* targetUser = bot->getUserRecord(st[2]);

	int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
  sqlCommandLevel* chgAdminLevel = bot->getLevelRequired("CHGADMIN", "ADMIN");

	/*
	 * check if we are removing admins or normal users
	 * cause removinging admins requires high access ;)
	 */

	if (theChan->getName() == "*")
        	{
	        if (level < chgAdminLevel->getLevel())
        	        {
                	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
	                return false;
        	        }
	        }

	if ((level < level::remuser) && ((targetUser) && targetUser != theUser))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::insuf_access,
				string("You have insufficient access to perform that command.")));
		return false;
	}

	/*
	 *  Check the person we're trying to remove actually exists.
	 */


	if (!targetUser)
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::not_registered,
				string("Sorry, I don't know who %s is.")).c_str(),
			st[2].c_str());
		return false;
	}

	/*
	 *  Check this user has access on this channel.
	 */

	sqlLevel* tmpLevel = bot->getLevelRecord(targetUser, theChan);

	if (!tmpLevel)
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::doesnt_have_access,
				string("%s doesn't appear to have access in %s.")).c_str(),
			targetUser->getUserName().c_str(), theChan->getName().c_str());
		return false;
	}

	int targetLevel = tmpLevel->getAccess();

	/*
	 *  Check we aren't trying to remove someone with access higher than ours.
	 *  Unless they are trying to remove themself.. in which case its ok ;)
	 */

	if ((level <= targetLevel) && (targetUser != theUser) || (targetLevel == 499 && !bot->isForced(theChan, theUser)))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::cant_rem_higher,
				string("Cannot remove a user with equal or higher access than your own")));
		return false;
	}


	if ((theChan->getName() == "*") && (targetUser == theUser))
	{
		bot->Notice(theClient,
                        bot->getResponse(theUser,
                                language::cant_rem_higher,
                                string("CSC has your soul! YOU CAN NEVER ESCAPE!")));
                return false;
	}

	if ((targetLevel == 500) && (targetUser == theUser))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::cant_rem_owner_self,
				string("You can't remove yourself from a channel you own")));
		return false;
	}


	/*
	 *  Now, build up the SQL query & execute it!
	 */

	theQuery << queryHeader
	<< "channel_id = " << theChan->getID()
	<< " AND user_id = " << targetUser->getID()
	<< ";" << ends;

#ifdef LOG_SQL
	elog << "sqlQuery> " << theQuery.str().c_str() << endl;
#endif

	if ((status = bot->SQLDb->Exec(theQuery.str().c_str())) == PGRES_COMMAND_OK)
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::removed_user,
				string("Removed user %s from %s")).c_str(),
			targetUser->getUserName().c_str(), theChan->getName().c_str());
	} else {
		bot->dbErrorMessage(theClient);
 	}

	/* Remove tmpLevel from the cache. (It has to be there, we just got it even if it wasnt..) */

	pair<int, int> thePair;
	thePair = std::make_pair(tmpLevel->getUserId(), tmpLevel->getChannelId());
	bot->sqlLevelCache.erase(thePair);
	delete(tmpLevel);

	return true ;
}

} // namespace gnuworld.

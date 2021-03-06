/*
 * REMUSERCommand.cc
 *
 * 27/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Removes a users access from a particular channel.
 *
 * Caveats: None
 */

#include	<string>

#include	"ELog.h"
#include	"libpq++.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{


void REMUSERCommand::Exec( iClient* theClient, const string& Message )
{
	bot->incStat("COMMANDS.REMUSER");

	StringTokenizer st( Message ) ;
	if( st.size() < 3 )
	{
		Usage(theClient);
		return ;
	}

	static const char* queryHeader = "DELETE FROM levels WHERE ";

	stringstream theQuery;
	ExecStatusType status;

	/*
	 *  Fetch the sqlUser record attached to this client. If there isn't one,
	 *  they aren't logged in - tell them they should be.
	 */

	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return ;

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
		return ;
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
	sqlCommandLevel* chgAdminLevel = bot->getLevelRequired("REMUSER", "A2");

	/*
	 * check if we are removing admins or normal users
	 * cause removing admins requires high access ;)
	 */

	if (theChan->getName() == "*")
        	{
	        if (level < chgAdminLevel->getLevel())
        	        {
                	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
	                return ;
        	        }
	        }

	if ((level < level::remuser) && ((targetUser) && targetUser != theUser))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::insuf_access,
				string("You have insufficient access to perform that command.")));
		return ;
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
		return ;
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
		return ;
	}

	int targetLevel = tmpLevel->getAccess();

	/*
	 *  Check we aren't trying to remove someone with access higher than ours.
	 *  Unless they are trying to remove themself.. in which case its ok ;)
	 */

	/* My Karnaugh map for this says:
	 * F - Forced
	 * N - New
	 * S - Supporter
	 *       F
	 *       0 1
	 * NS 00 1 1
	 *    01 1 1
	 *    11 0 1
	 *    10 1 1
	 *
	 * Which solves to F+!N+N!S
	 * or, for the negative, !FNS
	 */

	if(!bot->isForced(theChan, theUser) && bot->isNew(theChan) && targetLevel == 499) {
		bot->Notice(theClient, "Only CService may remove users with 499+ access inside the NEW period.");
		return ;
	}

	if ((level <= targetLevel) && (targetUser != theUser))
	{
		bot->Notice(theClient, "Cannot remove a user with equal or higher access than your own.");
		return ;
	}


	if ((theChan->getName() == "*") && (targetUser == theUser))
	{
		bot->Notice(theClient, "CSC has your soul! YOU CAN NEVER ESCAPE!");
                return ;
	}

	if ((targetLevel == 500) && (targetUser == theUser))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::cant_rem_owner_self,
				string("You can't remove yourself from a channel you own.")));
		return ;
	}

	/* If we have been removed from a channel with STRICTOP or STRICTVOICE
	 * set, we may need to deop or devoice the user.
	 */
	Channel *tmpChan = Network->findChannel(theChan->getName());
	if(tmpChan) {
		/* Iterate over the logged in clients of the user */
		sqlUser::networkClientListType::const_iterator itr =
			targetUser->networkClientList.begin();
		
		for( ; itr != targetUser->networkClientList.end() ; ++itr ) {
			/* Is this client in the channel? */
			ChannelUser *tmpChanUser = tmpChan->findUser(*itr);
			if(tmpChanUser) {
				/* Do we need to deop the user? */
				if( theChan->getFlag(sqlChannel::F_STRICTOP) &&
				    tmpChanUser->isModeO()) {
					bot->Notice(*itr, "You are not"
						" allowed to be opped in %s.",
						theChan->getName().c_str());
					bot->DeOp(tmpChan, *itr);
				}
				
				/* Do we need to devoice the user? */
				if( theChan->getFlag(sqlChannel::F_STRICTVOICE) &&
				    tmpChanUser->isModeV()) {
					bot->Notice(*itr, "You are not"
						" allowed to be voiced in %s.",
						theChan->getName().c_str());
					bot->DeVoice(tmpChan, *itr);
				}
			} /* if(tmpChanUser) */
		} /* iterate over logged in clients */
	} // if(tmpChan)

	/*
	 *  Now, build up the SQL query & execute it!
	 */

	theQuery	<< queryHeader
			<< "channel_id = " << theChan->getID()
			<< " AND user_id = " << targetUser->getID()
			<< ";"
			;

#ifdef LOG_SQL
	elog << "sqlQuery> " << theQuery.str().c_str() << endl;
#endif

	if ( bot->SQLDb->ExecCommandOk(theQuery.str().c_str()) )
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

	return ;
}

} // namespace gnuworld.

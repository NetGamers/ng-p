/*
 * MODINFOCommand.cc
 *
 * 27/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Modifies a user's 'Level' record in a particular channel.
 *
 * Caveats:
 * 1. In the rare case of somebody attempting to MODINFO a forced access
 * that doesn't exist in the database, then the commit() will fail.
 * This is fine, as the modified record doesn't really exist anyway.
 * Shouldn't really happen, as trying to MODINFO a forced access doesn't
 * make sense - adduser and then MODINFO that :)
 */

#include	<string>

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
using std::string ;

void MODINFOCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.MODINFO");

StringTokenizer st( Message ) ;
if( st.size() < 5 )
	{
	Usage(theClient);
	return ;
	}

const string command = string_upper(st[2]);
if ((command != "ACCESS") && (command != "AUTOMODE"))
	{
	Usage(theClient);
	return ;
	}

/*
 *  Fetch the sqlUser record attached to this client. If there isn't one,
 *  they aren't logged in - tell them they should be.
 */

sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser)
	{
	return ;
	}

/*
 *  First, check the channel is registered.
 */

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
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
int level = bot->getEffectiveAccessLevel(theUser, theChan, true);

/*
 * check if we are modifying admins or normal users
 * cause modifying admins requires high access ;)
 */

sqlCommandLevel* chgAdminLevel = bot->getLevelRequired("MODINFO", "A2");

if (theChan->getName() == "*")
        {
        if (level < chgAdminLevel->getLevel())
                {
                bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
                return ;
                }
        }


if (level < level::modinfo)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("Sorry, you have insufficient access to perform that command.")));
	return ;
	}

/*
 *  Check the person we're trying to change actually exists.
 */

sqlUser* targetUser = bot->getUserRecord(st[3]);
if (!targetUser)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::not_registered,
			string("Sorry, I don't know who %s is.")).c_str(), st[3].c_str());
	return ;
	}

/*
 *  Check this user really does have access on this channel.
 */

int targetLevel = bot->getAccessLevel(targetUser, theChan);
if (targetLevel == 0)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::doesnt_have_access,
			string("%s doesn't appear to have access in %s.")).c_str(),
		targetUser->getUserName().c_str(),
		theChan->getName().c_str());
	return ;
	}

/*
 *  Figure out what they're doing - ACCESS or AUTOMODE
 */

if (command == "ACCESS") {
	/*
	 * Check we aren't trying to change someone with access
	 * higher (or equal) than ours.
	 */

	if (level <= targetLevel || targetLevel >= 999)
		{
		/*
		 * Let forced users modify their own user records in channels to
		 * any setting.
		 */
		if (!bot->isForced(theChan, theUser))
			{
			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::mod_access_higher,
					string("Cannot modify a user with equal or higher access than your own.")));
			return ;
			}
		}

	int newAccess = atoi(st[4].c_str());

	if ((newAccess <= 0) || (newAccess > 999)) {
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::inval_access,
				string("Invalid access level.")));
		return ;
	}

	if( (theChan->getName() != "*") &&
	    (newAccess >= 499) &&
	    (!bot->isForced(theChan, theUser))) {
		bot->Notice(theClient, "Only CService may modify users with"
			" 499+ access.");
		return ;
	}

	/*
	 * And finally, check they aren't trying to give someone
	 * higher access than them.
	 */

	if (level <= newAccess)
		{
		bot->Notice(theClient, "Cannot give a user higher or equal"
			" access to your own.");
		return ;
		}

	sqlLevel* aLevel = bot->getLevelRecord(targetUser, theChan);

	/* Check we arent trying to lower a 499 */
	if( theChan->getName() != "*" &&
	    aLevel->getAccess()==499 &&
	    !bot->isForced(theChan, theUser)) {
		bot->Notice(theClient, "Only CService may modify users with"
			" 499+ access.");
		return ;
	}
	
	/* If the new level is below 100 or 25 as appropriate and
	 * STRICTOP or STRICTVOICE is set, deop or devoice.
	 */
	
	/* Does the channel exist on the network? */
	Channel *tmpChan = Network->findChannel(theChan->getName());
	if(tmpChan) {
		/* Iterate over the logged in clients of the user */
		sqlUser::networkClientListType::const_iterator itr =
			targetUser->networkClientList.begin();
		for( ; itr != targetUser->networkClientList.end() ; ++itr) {
			/* Is the user in the channel? */
			ChannelUser *tmpChanUser = tmpChan->findUser(*itr);
			if(tmpChanUser) {
				/* Do we need to deop the user? */
				if( theChan->getFlag(sqlChannel::F_STRICTOP) &&
				    newAccess < level::op &&
				    tmpChanUser->isModeO()) {
				    	bot->Notice(*itr, "You are not"
						" allowed to be opped in %s.",
						theChan->getName().c_str());
					bot->DeOp(tmpChan, *itr);
				}
			
				/* Do we need to devoice the user? */
				if( theChan->getFlag(sqlChannel::F_STRICTVOICE) &&
				    newAccess < level::voice &&
				    tmpChanUser->isModeV()) {
					bot->Notice(*itr, "You are not"
						" allowed to be voiced in %s.",
						theChan->getName().c_str());
					bot->DeVoice(tmpChan, *itr);
				}
			} /* if(tmpChanUser) */
		} // iterate over targetUser's clients
	} /* if(tmpChan) */

	aLevel->setAccess(newAccess);
	aLevel->setLastModif(bot->currentTime());
	aLevel->setLastModifBy( string( "(" + theUser->getUserName() + ") " +
		theClient->getNickUserHost() ) );
	if (newAccess < level::set::autoinvite)
		aLevel->removeFlag(sqlLevel::F_AUTOINVITE);
	aLevel->commit();

	bot->Notice(theClient, "Modified %s's access level on channel %s to %i",
		targetUser->getUserName().c_str(),
		theChan->getName().c_str(),
		newAccess);
} // if( command == "ACCESS" )

if (command == "AUTOMODE")
	{
	/*
	 * Check we aren't trying to change someone with access higher
	 * than ours (or equal).
	 */

	if (level < targetLevel)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::mod_access_higher,
				string("Cannot modify a user with higher access than your own.")));
		return ;
		}

	/*
	 *  Check for "ON" or "OFF" and act accordingly.
	 */

	if (string_upper(st[4]) == "OP")
		{
		sqlLevel* aLevel = bot->getLevelRecord(targetUser, theChan);
		aLevel->removeFlag(sqlLevel::F_AUTOVOICE);
		aLevel->setFlag(sqlLevel::F_AUTOOP);
//		aLevel->setLastModif(bot->currentTime());
//		aLevel->setLastModifBy(theClient->getNickUserHost());
		aLevel->commit();

		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::automode_op,
				string("Set AUTOMODE to OP for %s on channel %s")).c_str(),
			targetUser->getUserName().c_str(),
			theChan->getName().c_str());

		return ;
		}

	if (string_upper(st[4]) == "VOICE")
		{
		sqlLevel* aLevel = bot->getLevelRecord(targetUser, theChan);
		aLevel->removeFlag(sqlLevel::F_AUTOOP);
		aLevel->setFlag(sqlLevel::F_AUTOVOICE);
//		aLevel->setLastModif(bot->currentTime());
//		aLevel->setLastModifBy(theClient->getNickUserHost());
		aLevel->commit();

		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::automode_voice,
				string("Set AUTOMODE to VOICE for %s on channel %s")).c_str(),
			targetUser->getUserName().c_str(),
			theChan->getName().c_str());
		return ;
		}

	if (string_upper(st[4]) == "NONE")
		{
		sqlLevel* aLevel = bot->getLevelRecord(targetUser, theChan);
		aLevel->removeFlag(sqlLevel::F_AUTOOP);
		aLevel->removeFlag(sqlLevel::F_AUTOVOICE);
//		aLevel->setLastModif(bot->currentTime());
//		aLevel->setLastModifBy(theClient->getNickUserHost());
		aLevel->commit();

		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::automode_none,
				string("Set AUTOMODE to NONE for %s on channel %s")).c_str(),
			targetUser->getUserName().c_str(),
			theChan->getName().c_str());
		return ;
		}

	Usage(theClient);
	return ;
	}

return ;
}

} // namespace gnuworld

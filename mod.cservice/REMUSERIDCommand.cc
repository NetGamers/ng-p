/*
 * REMUSERIDCommand.cc - Allow purging of a user nick
 *
 * 20020308 GK@PAnet - Initial Writing
 *
 * $Id: REMUSERIDCommand.cc,v 1.11 2002-09-13 21:30:40 jeekay Exp $
 */

#include	<string>

#include "StringTokenizer.h"
#include "ELog.h"
#include "cservice.h"
#include "levels.h"
#include "networkData.h"

const char REMUSERIDCommand_cc_rcsId[] = "$Id: REMUSERIDCommand.cc,v 1.11 2002-09-13 21:30:40 jeekay Exp $" ;

namespace gnuworld
{

using std::ends ;
using std::string ;

bool REMUSERIDCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.REMUSERID");

// REMUSERID nick reason
StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return true;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser)
	{ return false; }

int aLevel = bot->getAdminAccessLevel(theUser);
if(aLevel < level::remuserid)
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
	return false;
	}

sqlUser* targetUser = bot->getUserRecord(st[1]);
if(!targetUser)
	{
	bot->Notice(theClient, "%s is not registered with me.", st[1].c_str());
	return true;
	}

if(!targetUser->getComment().empty())
  {
  bot->Notice(theClient, "Sorry, this nick is commented and cannot be removed.");
  return false;
  }

int targetALevel = bot->getAdminAccessLevel(targetUser);
if(targetALevel && (aLevel < level::chgadmin))
	{
	bot->Notice(theClient, "Sorry, you cannot purge a user with admin access.");
	return false;
	}

if(targetALevel >= aLevel)
	{
	bot->Notice(theClient, "You cannot purge someone with higher admin access than you.");
	return false;
	}

if(targetUser->getFlag(sqlUser::F_NOPURGE))
	{
	bot->Notice(theClient, "%s has NOPURGE set. Purge not allowed.", targetUser->getUserName().c_str());
	return true;
	}

/* Very first things first - is the user currently logged in? */
iClient* authTestUser = targetUser->isAuthed();
if(authTestUser)
	{
	networkData* tmpData = static_cast< networkData* >( authTestUser->getCustomData(bot) );
	if(!tmpData)
		{
		bot->Notice(theClient, "Wierd internal error.");
		elog << "REMUSERID> tmpData null this user" << endl;
		return false;
		}
	
	tmpData->currentUser = NULL;
	targetUser->networkClient = NULL;
	server->PostEvent(gnuworld::EVT_FORCEDEAUTH, static_cast< void* >( authTestUser));
	}

/* First things first -
 * Does this user own any channels? if so - abort!
 */

ExecStatusType status;

stringstream chanOwner;
chanOwner << "SELECT COUNT(*) AS count FROM levels WHERE user_id = "
	<< targetUser->getID() << " AND access = 500 AND channel_id <> 1"
	<< ends;
#ifdef LOG_SQL
elog << "REMUSERID:cO:SQL> " << chanOwner.str().c_str() << endl;
#endif
status = bot->SQLDb->Exec(chanOwner.str().c_str());

if(PGRES_TUPLES_OK != status)
	{
	elog << "REMUSERID::cO> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

int chansOwned = atoi(bot->SQLDb->GetValue(0,0));
if(chansOwned)
	{
	bot->Notice(theClient, "This user owns %d channels. Please deal with that first.",
		chansOwned);
	return false;
	}

/*
 * Now we know that this user exists and is in the current cache
 * We first need to delete it from the database
 * then delete it from the cache. This prevents the cache
 * reloading the record accidentally after we've tried to delete
 * it. Note this shouldnt be a problem unless we start using
 * threading. Better safe than sorry, though.
 */
 
// First, we need to delete any references to levels this user has

stringstream selectLevelQuery;
selectLevelQuery << "SELECT user_id,channel_id FROM levels WHERE user_id = "
	<< targetUser->getID()
	<< ends;

status = bot->SQLDb->Exec(selectLevelQuery.str().c_str());

if(PGRES_TUPLES_OK != status)
	{
	elog << "REMUSERID::sLQ> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

for(int i = 0; i < bot->SQLDb->Tuples(); i++)
	{
	int userId = atoi(bot->SQLDb->GetValue(i, 0));
	int channelId = atoi(bot->SQLDb->GetValue(i, 1));
	pair<int, int> myLevelPair;
	myLevelPair = std::make_pair(userId, channelId);
	bot->sqlLevelCache.erase(myLevelPair);
	}

stringstream deleteLevelQuery;
deleteLevelQuery << "DELETE FROM levels WHERE user_id = "
	<< targetUser->getID()
	<< ends;

status = bot->SQLDb->Exec(deleteLevelQuery.str().c_str());

if(PGRES_COMMAND_OK != status)
	{
	elog << "REMUSERID::sqlError> "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

/* 
 * Now we need to delete the lastseen record of this user
 */

stringstream deleteLastseenQuery;
deleteLastseenQuery << "DELETE FROM users_lastseen WHERE user_id = "
	<< targetUser->getID()
	<< ends;

status = bot->SQLDb->Exec(deleteLastseenQuery.str().c_str());

if(PGRES_COMMAND_OK != status)
	{
	elog << "REMUSERID::sqlError> "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

/*
 * Delete any memos the user might have
 */

stringstream deleteMemoQuery;
deleteMemoQuery << "DELETE FROM memo WHERE"
	<< " from_id = " << targetUser->getID()
	<< " OR to_id = " << targetUser->getID()
	<< ends;
status = bot->SQLDb->Exec(deleteMemoQuery.str().c_str());

if(PGRES_COMMAND_OK != status)
	{
	elog << "REMUSERID::sqlError> "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

/*
 * Delete any note allow records
 */

stringstream deleteAllowQuery;
deleteAllowQuery << "DELETE FROM note_allow WHERE"
	<< " user_id = " << targetUser->getID()
	<< " OR user_from_id = " << targetUser->getID()
	<< ends;
status = bot->SQLDb->Exec(deleteAllowQuery.str().c_str());

if(PGRES_COMMAND_OK != status)
	{
	elog << "REMUSERID::sqlError> "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

/*
 * Delete any userlog entries
 */

stringstream deleteUserlogQuery;
deleteUserlogQuery << "DELETE FROM userlog WHERE"
	<< " user_id = " << targetUser->getID()
	<< ends;
#ifdef LOG_SQL
elog << "REMUSERID:SQL> " << deleteUserlogQuery.str().c_str() << endl;
#endif
status = bot->SQLDb->Exec(deleteUserlogQuery.str().c_str());

if(PGRES_COMMAND_OK != status)
	{
	elog << "REMUSERID::sqlError> "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

/*
 * Now proceed to delete the actual user record
 */

stringstream delUserQuery;
delUserQuery << "DELETE FROM users WHERE id = "
	<< targetUser->getID()
	<< ends;

#ifdef LOG_SQL
	elog << "REMUSERID::sqlQuery> "
		<< delUserQuery.str().c_str()
		<< endl;
#endif

status = bot->SQLDb->Exec(delUserQuery.str().c_str());

if(PGRES_COMMAND_OK != status)
	{
	elog << "REMUSERID::sqlError> "
		<< bot->SQLDb->ErrorMessage()
		<< endl;
	bot->Notice(theClient, "Unknown SQL error removing user. Please contact a DB administrator.");
	return false;
	}

bot->Notice(theClient, "Successfully removed %s from the database.", st[1].c_str());

// Was the user logged in? Notice them
if(authTestUser)
	bot->Notice(authTestUser, "Your registered nick has been purged. You are no longer authenticated.");


// The user has now been deleted from the database
// We now want to clean the cache of this user

cservice::sqlUserHashType::iterator ptr = bot->sqlUserCache.find(st[1]);
if(ptr != bot->sqlUserCache.end())
	{
	// We should always get to here. Otherwise the user wasnt in the cache
//	string removeKey = ptr->first;
	
	// **NOTE: From here on, targetUser is an invalid pointer
	delete(ptr->second);
	bot->sqlUserCache.erase(ptr);
	
	bot->Notice(theClient, "Successfully removed %s from the user cache.", st[1].c_str());
	}

bot->Notice(theClient, "%s purged.", st[1].c_str());

// Done! Now lets make sure everyone sees it was done
bot->logAdminMessage("%s (%s) - REMUSERID - %s - %s",
	theClient->getNickName().c_str(), theUser->getUserName().c_str(),
	st[1].c_str(), st.assemble(2).c_str());

return true ;
} // REMUSERIDCommand::Exec

} // namespace gnuworld.

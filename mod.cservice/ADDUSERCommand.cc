/*
 * ADDUSERCommand.cc
 *
 * 26/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * 01/03/01 - Daniel Simard <svr@undernet.org>
 * Fixed Language module stuff.
 *
 * Adds a new user to a channel, obeying common sense.
 *
 * Caveats: None
 *
 * $Id: ADDUSERCommand.cc,v 1.7 2002-09-13 21:30:38 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"levels.h"
#include	"libpq++.h"
#include	"responses.h"
#include	"cservice_config.h"

const char ADDUSERCommand_cc_rcsId[] = "$Id: ADDUSERCommand.cc,v 1.7 2002-09-13 21:30:38 jeekay Exp $" ;

namespace gnuworld
{

using std::ends ;
using std::string ;

static const char* queryHeader = "INSERT INTO levels (channel_id, user_id, access, flags, added, added_by, last_modif, last_modif_by, last_updated) ";

bool ADDUSERCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.ADDUSER");

StringTokenizer st( Message ) ;
if( st.size() != 4 )
	{
	Usage(theClient);
	return true;
	}

/*
 *  Fetch the sqlUser record attached to this client. If there isn't one,
 *  they aren't logged in - tell them they should be.
 */

sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser)
	{
	return false;
	}

/*
 *  First, check the channel is registered.
 */

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::chan_not_reg).c_str(),
		st[1].c_str()
	);
	return false;
	}

#ifdef FEATURE_FORCELOG
unsigned short forcedAccess = bot->isForced(theChan, theUser);
if (forcedAccess <= 900 && forcedAccess > 0)
        {
        bot->writeForceLog(theUser, theChan, Message);
        }

#endif

/*
 *  Check the user has sufficient access on this channel.
 */

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);

/* 
 * check if we are adding admins or normal users
 * cause adding admins requires high access ;)
 */

if (theChan->getName() == "*")
	{
	if (level < level::chgadmin)
		{
	        bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
		return false;
		}
	}

if (level < level::adduser)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access).c_str()
	);
	return false;
	}

/*
 *  Check we aren't trying to add someone with access higher than ours.
 */
int targetAccess = atoi(st[3].c_str());

if (level <= targetAccess || (targetAccess == 499 && !bot->isForced(theChan, theUser)))
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::access_higher).c_str()
	);
	return false;
	}

if ((targetAccess <= 0) || (targetAccess > 999))
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::inval_access).c_str()
	);
	return false;
	}

/*
 * Allow multiple comma seperated nicks to be added
 */

char delim=0;
string::size_type pos = st[2].find_first_of(',');
if(string::npos != pos)
	{ // We have a winner
	delim = ',';
	}
else
	{
	delim = ' ';
	}

StringTokenizer st2(st[2], ',');

for(StringTokenizer::size_type counter = 0; counter < st2.size(); ++counter)
	{
	/*
	 *  Check the person we're trying to add is actually registered.
	 */

	sqlUser* targetUser = bot->getUserRecord(st2[counter]);
	if (!targetUser)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::not_registered).c_str(),
			st2[counter].c_str() );
		return false;
		} // if(!targetUser)

	/*
	 *  Check this user doesn't already have access on this channel.
	 *  (Note: If they're forced, this will only be shown in
	 *  getEffectiveAccess, not by looking at level records).
	 */

	sqlLevel* newLevel = bot->getLevelRecord(targetUser, theChan);
	int levelTest = newLevel ? newLevel->getAccess() : 0 ;

	if (levelTest != 0)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::already_in_list).c_str(),
			targetUser->getUserName().c_str(),
			theChan->getName().c_str(),
			levelTest);
		return false;
		} // if(levelTest != 0)

	/*
	 *  Work out the flags this user should default to.
	 */

	unsigned short targetFlags = 0;

	if (theChan->getUserFlags() == 1) targetFlags = sqlLevel::F_AUTOOP;
	if (theChan->getUserFlags() == 2) targetFlags = sqlLevel::F_AUTOVOICE;

	/*
	 *  Now, build up the SQL query & execute it!
	 */

	stringstream theQuery;
	theQuery	<< queryHeader
			<< "VALUES ("
			<< theChan->getID() << ","
			<< targetUser->getID() << ","
			<< targetAccess << ","
			<< targetFlags << ","
			<< bot->currentTime() << ","
			<< "'(" << theUser->getUserName() << ") "
	                   << theClient->getNickUserHost() << "',"
			<< bot->currentTime() << ","
			<< "'(" << theUser->getUserName() << ") "
	                   << theClient->getNickUserHost() << "',"
			<< bot->currentTime()
			<< ");"
			<< ends;

#ifdef LOG_SQL
		elog	<< "ADDUSER::sqlQuery> "
					<< theQuery.str().c_str()
					<< endl;
#endif

	ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str()) ;

	if( PGRES_COMMAND_OK == status )
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::add_success).c_str(),
			targetUser->getUserName().c_str(),
			theChan->getName().c_str(),
			targetAccess);

		/*
		 * Add this new record to the level cache.
		 */

		sqlLevel* newLevel = new (std::nothrow) sqlLevel(bot->SQLDb);
		newLevel->setChannelId(theChan->getID());
		newLevel->setUserId(targetUser->getID());
		newLevel->setAccess(targetAccess);
		newLevel->setFlag(targetFlags);
		newLevel->setAdded(bot->currentTime());
		newLevel->setAddedBy("(" + theUser->getUserName() + ") " + theClient->getNickUserHost());
		newLevel->setLastModif(bot->currentTime());
		newLevel->setLastModifBy("(" + theUser->getUserName() + ") " + theClient->getNickUserHost());

		pair<int, int> thePair( newLevel->getUserId(), newLevel->getChannelId());
		bot->sqlLevelCache.insert(cservice::sqlLevelHashType::value_type(thePair, newLevel));

		/*
		 *  "If they where added to *, set their invisible flag" (Ace).
		 */
		if (theChan->getName() == "*")
			{
			targetUser->setFlag(sqlUser::F_INVIS);
			targetUser->commit();
			} // if(theChan->getName() == "*")
		}
	else
		{
		bot->dbErrorMessage(theClient);
		} // if(PGRES_COMMAND_OK == status)
	} // for() loop around arguments


return true ;
}

} // namespace gnuworld.

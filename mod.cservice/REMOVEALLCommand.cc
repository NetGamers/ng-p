/*
 * REMOVEALLCommand.
 *
 * Quickly clear a channels access list.
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include 	"responses.h"
#include	"levels.h"
#include	"cservice_config.h"

const char REMOVEALLCommand_cc_rcsId[] = "$Id: REMOVEALLCommand.cc,v 1.4 2002-07-01 00:33:06 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

bool REMOVEALLCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.REMOVEALL");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
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
if ((!theChan) || (st[1] == "*"))
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::chan_not_reg,
			string("%s isn't registered with me")).c_str(),
		st[1].c_str());
	return false;
	}

/*
 *  Check the user has sufficient access for this command..
 */

int level = bot->getAdminAccessLevel(theUser);
if (level < level::removeall)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("You have insufficient access to perform that command")));
	return false;
	}


/*
 * We need to ensure this user is forced before they can removeall
 */

int forceLevel = bot->isForced(theChan, theUser);
if(!forceLevel)
	{
	bot->Notice(theClient, "You are required to FORCE the channel prior to using REMOVEALL.");
	bot->logAdminMessage("%s (%s) - REMOVEALL - Failed - %s Not Forced",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		theChan->getName().c_str());
	return false;
	}

/*
 *  Now, perform a fast query on the levels table for this channel.
 */

stringstream clearAllQuery;
clearAllQuery	<< "SELECT user_id FROM levels WHERE"
		<< " channel_id = "
		<< theChan->getID()
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlQuery> "
		<< clearAllQuery.str().c_str()
		<< endl;
#endif

ExecStatusType status = bot->SQLDb->Exec(clearAllQuery.str().c_str()) ;

if( status != PGRES_TUPLES_OK )
	{
	elog	<< "REMOVEALL> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return false ;
	}

/*
 * Iterate over all the results, checking if this access is in the
 * cache.
 * If so, remove it!
 */

int delCounter = bot->SQLDb->Tuples();

for (int i = 0 ; i < bot->SQLDb->Tuples(); i++)
{
	pair<int, int> thePair( atoi(bot->SQLDb->GetValue(i, 0)), theChan->getID() );

	cservice::sqlLevelHashType::iterator ptr = bot->sqlLevelCache.find(thePair);
	if(ptr != bot->sqlLevelCache.end())
		{
		/*
		 *  Found it in the cache, free the memory and
		 *  remove it from the cache.
		 */

		delete(ptr->second);
		bot->sqlLevelCache.erase(thePair);
		}
}

/*
 * Now the cache is clean, execute some SQL to eradicate the records
 * from the database.
 */

stringstream deleteAllQuery;
deleteAllQuery	<< "DELETE FROM levels WHERE"
		<< " channel_id = "
		<< theChan->getID()
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlQuery> "
		<< deleteAllQuery.str().c_str()
		<< endl;
#endif

if (bot->SQLDb->ExecCommandOk(deleteAllQuery.str().c_str()))
	{
		bot->Notice(theClient, "Done. Zapped %i access records from %s",
			delCounter, theChan->getName().c_str());
		bot->writeChannelLog(theChan,
			theClient,
			sqlChannel::EV_REMOVEALL, "" );

		bot->logAdminMessage("%s (%s) - REMOVEALL - %s",
			theClient->getNickName().c_str(), theUser->getUserName().c_str(), theChan->getName().c_str());

	} else
	{
		bot->Notice(theClient, "A database error occured while removing the access records.");
		bot->Notice(theClient, "Please contact a database administrator!");
	}

return true;
}
} // namespace gnuworld.


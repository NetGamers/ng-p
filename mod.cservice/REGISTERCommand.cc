/*
 * REGISTERCommand.cc
 *
 * 26/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Registers a channel.
 *
 * Caveats: None
 *
 * $Id: REGISTERCommand.cc,v 1.3 2002-03-25 01:20:16 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"levels.h"
#include	"libpq++.h"
#include	"Network.h"
#include	"responses.h"

const char REGISTERCommand_cc_rcsId[] = "$Id: REGISTERCommand.cc,v 1.3 2002-03-25 01:20:16 jeekay Exp $" ;

namespace gnuworld
{

using namespace gnuworld;

bool REGISTERCommand::Exec( iClient* theClient, const string& Message )
{
	bot->incStat("COMMANDS.REGISTER");

	StringTokenizer st( Message ) ;
	if( st.size() < 3 )
	{
		Usage(theClient);
		return true;
	}

	/*
	 *  Fetch the sqlUser record attached to this client. If there isn't one,
	 *  they aren't logged in - tell them they should be.
	 */

	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return false;
	
	// Check the channel is not currently in the database either
	strstream chanQuery;
	chanQuery << "SELECT id FROM channels WHERE lower(name) = '"
						<< escapeSQLChars(st[1]) << "'" << ends;
#ifdef LOG_SQL
	elog << "REGISTER::sqlQuery> " << chanQuery.str() << endl;
#endif
	ExecStatusType status = bot->SQLDb->Exec(chanQuery.str());
	delete[] chanQuery.str();
	
	if( PGRES_TUPLES_OK != status )
		{
		elog << "REGISTER::sql> SQL Error: "
				 << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}
	
	if(bot->SQLDb->Tuples() > 0)
		{
		bot->Notice(theClient, "%s is already in the database", st[1].c_str());
		return false;
		}

 	/*
	 *  First, check the channel isn't already registered.
	 */

	sqlChannel* theChan;
	theChan = bot->getChannelRecord(st[1]);
	if (theChan)
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::chan_already_reg,
				string("%s is already registered with me.")).c_str(),
			st[1].c_str());
		return false;
	}

	sqlUser* tmpUser = bot->getUserRecord(st[2]);
	if (!tmpUser)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::not_registered,
				string("The user %s doesn't appear to be registered.")).c_str(),
			st[2].c_str());
		return true;
		}

	/*
	 *  Check the user has sufficient access for this command..
	 */

	int level = bot->getAdminAccessLevel(theUser);
	if (level < level::registercmd)
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::insuf_access,
				string("You have insufficient access to perform that command.")));
		return false;
	}

	string::size_type pos = st[1].find_first_of( ',' ); /* Don't allow comma's in channel names. :) */

	if ( (st[1][0] != '#') || (string::npos != pos))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::inval_chan_name,
				string("Invalid channel name.")));
		return false;
	}

	/*
	 * Create the new channel and insert it into the cache.
	 * If the channel exists on IRC, grab the creation timestamp
	 * and use this as the channel_ts in the Db.
	 */

	unsigned int channel_ts = 0;
	Channel* tmpChan = Network->findChannel(st[1]);
	channel_ts = tmpChan ? tmpChan->getCreationTime() : ::time(NULL);

	sqlChannel* newChan = new (std::nothrow) sqlChannel(bot->SQLDb);
	newChan->setName(st[1]);
	newChan->setChannelTS(channel_ts);
	newChan->setRegisteredTS(bot->currentTime());
	newChan->setChannelMode("+tn");
	newChan->setLastUsed(bot->currentTime());

	bot->sqlChannelCache.insert(cservice::sqlChannelHashType::value_type(newChan->getName(), newChan));
	bot->sqlChannelIDCache.insert(cservice::sqlChannelIDHashType::value_type(newChan->getID(), newChan));

 	/*
	 *  If this channel exists in the database (without a registered_ts set),
	 *  then it is currently unclaimed. This register command will
	 *  update the timestamp, and proceed to adduser.
	 */

	strstream checkQuery;

	checkQuery 	<< "SELECT id FROM channels WHERE "
				<< "registered_ts = 0 AND lower(name) = '"
				<< escapeSQLChars(string_lower(st[1]))
				<< "'"
				<< ends;

	elog << "sqlQuery> " << checkQuery.str() << endl;

	bool isUnclaimed = false;
	if ((status = bot->SQLDb->Exec(checkQuery.str())) == PGRES_TUPLES_OK)
	{
		if (bot->SQLDb->Tuples() > 0) isUnclaimed = true;
	}

	delete[] checkQuery.str();

	if (isUnclaimed)
	{
		/*
		 *  Quick query to set registered_ts back for this chan.
		 */

		strstream reclaimQuery;

		reclaimQuery<< "UPDATE channels SET registered_ts = now()::abstime::int4,"
					<< " last_updated = now()::abstime::int4, "
					<< " flags = 0, description = '', url = '', comment = '', keywords = '', channel_mode = '+tn' "
					<< " WHERE lower(name) = '"
					<< escapeSQLChars(string_lower(st[1]))
					<< "'"
					<< ends;

		elog << "sqlQuery> " << reclaimQuery.str() << endl;

		if ((status = bot->SQLDb->Exec(reclaimQuery.str())) == PGRES_COMMAND_OK)
		{
			bot->logAdminMessage("%s (%s) has registered %s to %s", theClient->getNickName().c_str(),
				theUser->getUserName().c_str(), st[1].c_str(), tmpUser->getUserName().c_str());

			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::regged_chan,
					string("Registered channel %s")).c_str(),
				st[1].c_str());
		} else {
			bot->Notice(theClient, "Unable to update the channel in the database!");
			return false;
		}

		delete[] reclaimQuery.str();

	}
		else /* We perform a normal registration. */
	{
		newChan->insertRecord();

		bot->logAdminMessage("%s (%s) - REGISTER - %s - %s", theClient->getNickName().c_str(),
			theUser->getUserName().c_str(), st[1].c_str(), tmpUser->getUserName().c_str());
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::regged_chan,
				string("Registered channel %s")).c_str(),
			st[1].c_str());
	}

	/*
	 *  Now add the target chap at 500 in the new channel. To do this, we need to know
	 *  the db assigned channel id of the newly created channel :/
	 */
	strstream idQuery;

	idQuery 	<< "SELECT id FROM channels WHERE "
				<< "lower(name) = '"
				<< escapeSQLChars(string_lower(st[1]))
				<< "'"
				<< ends;

	elog << "sqlQuery> " << idQuery.str() << endl;

	unsigned int theId = 0;

	if ((status = bot->SQLDb->Exec(idQuery.str())) == PGRES_TUPLES_OK)
	{
		if (bot->SQLDb->Tuples() > 0)
		{
			theId = atoi(bot->SQLDb->GetValue(0, 0));
			newChan->setID(theId);
		} else
		{
			/*
			 * If we can't find the channel in the db, something has gone
			 * horribly wrong.
			 */
			return false;
		}

	} else
	{
		return false;
	}

	/*
	 *  Finally, commit a channellog entry.
	 */

	bot->writeChannelLog(newChan, theClient, sqlChannel::EV_REGISTER, "to " + tmpUser->getUserName());

	/*
	 * Create the new manager.
	 */

	sqlLevel* newManager = new (std::nothrow) sqlLevel(bot->SQLDb);
	newManager->setChannelId(newChan->getID());
	newManager->setUserId(tmpUser->getID());
	newManager->setAccess(500);
	newManager->setAdded(bot->currentTime());
	newManager->setAddedBy("(" + theUser->getUserName() + ") " + theClient->getNickUserHost());
	newManager->setLastModif(bot->currentTime());
	newManager->setLastModifBy("(" + theUser->getUserName() + ") " + theClient->getNickUserHost());

	if (!newManager->insertRecord())
		{
			bot->Notice(theClient, "Couldn't automatically add the level 500 Manager, check it doesn't already exist.");
			delete(newManager);
			return (false);
		}

	/*
	 * Insert this new 500 into the level cache.
	 */

	pair<int, int> thePair( newManager->getUserId(), newManager->getChannelId());
	bot->sqlLevelCache.insert(cservice::sqlLevelHashType::value_type(thePair, newManager));

	return true;
}

} // namespace gnuworld.

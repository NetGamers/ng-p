/*
 * REGISTERCommand.cc
 *
 * 26/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Registers a channel.
 *
 * Caveats: None
 */

#include	<string>

#include	"ELog.h"
#include	"libpq++.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{


void REGISTERCommand::Exec( iClient* theClient, const string& Message )
{
	bot->incStat("COMMANDS.REGISTER");

	StringTokenizer st( Message ) ;
	if( st.size() < 3 )
	{
		Usage(theClient);
		return ;
	}

	/*
	 *  Fetch the sqlUser record attached to this client. If there isn't one,
	 *  they aren't logged in - tell them they should be.
	 */

	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return ;
	
	/*
	 *  Check the user has sufficient access for this command..
	 */

	int level = bot->getAdminAccessLevel(theUser);
  sqlCommandLevel* registerCommandLevel = bot->getLevelRequired("REGISTER", "ADMIN");
  
	if (level < registerCommandLevel->getLevel())
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::insuf_access,
				string("You have insufficient access to perform that command.")));
		return ;
	}

	// Check the channel is not currently in the database either
	stringstream chanQuery;
	chanQuery << "SELECT id FROM channels WHERE lower(name) = '"
						<< escapeSQLChars(st[1]) << "'" ;
#ifdef LOG_SQL
	elog << "REGISTER::sqlQuery> " << chanQuery.str() << endl;
#endif
	ExecStatusType status = bot->SQLDb->Exec(chanQuery.str().c_str());
	
	if( PGRES_TUPLES_OK != status )
		{
		elog << "REGISTER::sql> SQL Error: "
				 << bot->SQLDb->ErrorMessage() << endl;
		return ;
		}
	
	if(bot->SQLDb->Tuples() > 0)
		{
		bot->Notice(theClient, "%s is already in the database", st[1].c_str());
		return ;
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
		return ;
	}

	sqlUser* tmpUser = bot->getUserRecord(st[2]);
	if (!tmpUser)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::not_registered,
				string("The user %s doesn't appear to be registered.")).c_str(),
			st[2].c_str());
		return ;
		}

	string::size_type pos = st[1].find_first_of( ',' ); /* Don't allow comma's in channel names. :) */

	if ( (st[1][0] != '#') || (string::npos != pos))
	{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::inval_chan_name,
				string("Invalid channel name.")));
		return ;
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

	stringstream checkQuery;

	checkQuery 	<< "SELECT id FROM channels WHERE "
				<< "registered_ts = 0 AND lower(name) = '"
				<< escapeSQLChars(string_lower(st[1]))
				<< "'"
				;

#ifdef LOG_SQL
	elog << "sqlQuery> " << checkQuery.str().c_str() << endl;
#endif

	bool isUnclaimed = false;
	if ((status = bot->SQLDb->Exec(checkQuery.str().c_str())) == PGRES_TUPLES_OK)
	{
		if (bot->SQLDb->Tuples() > 0) isUnclaimed = true;
	}

	if (isUnclaimed)
	{
		/*
		 *  Quick query to set registered_ts back for this chan.
		 */

		stringstream reclaimQuery;

		reclaimQuery	<< "UPDATE channels SET registered_ts = now()::abstime::int4,"
				<< " last_updated = now()::abstime::int4, "
				<< " flags = 0, description = '', url = '', comment = '', keywords = '', channel_mode = '+tn' "
				<< " WHERE lower(name) = '"
				<< escapeSQLChars(string_lower(st[1]))
				<< "'"
				;

#ifdef LOG_SQL
		elog << "sqlQuery> " << reclaimQuery.str().c_str() << endl;
#endif

		if ((status = bot->SQLDb->Exec(reclaimQuery.str().c_str())) == PGRES_COMMAND_OK)
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
			return ;
		}

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
	stringstream idQuery;

	idQuery 	<< "SELECT id FROM channels WHERE "
			<< "lower(name) = '"
			<< escapeSQLChars(string_lower(st[1]))
			<< "'"
			;

#ifdef LOG_SQL
	elog << "sqlQuery> " << idQuery.str().c_str() << endl;
#endif

	unsigned int theId = 0;

	if ((status = bot->SQLDb->Exec(idQuery.str().c_str())) == PGRES_TUPLES_OK)
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
			return ;
		}

	} else
	{
		return ;
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
			return ;
		}

	/*
	 * Insert this new 500 into the level cache.
	 */

	pair<int, int> thePair( newManager->getUserId(), newManager->getChannelId());
	bot->sqlLevelCache.insert(cservice::sqlLevelHashType::value_type(thePair, newManager));

	return ;
}

} // namespace gnuworld.

/*
 * PURGECommand.cc
 *
 * 24/01/2001 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Purges a channel.
 *
 * Caveats: None
 *
 * $Id: PURGECommand.cc,v 1.13 2004-01-17 17:43:31 jeekay Exp $
 */

#include	<string>

#include	"ELog.h"
#include	"Network.h"
#include	"libpq++.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"cservice_config.h"
#include	"responses.h"

const char PURGECommand_cc_rcsId[] = "$Id: PURGECommand.cc,v 1.13 2004-01-17 17:43:31 jeekay Exp $" ;

namespace gnuworld
{

using std::ends;

bool PURGECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.PURGE");

// PURGE #channel reason
// PURGE FORCE #channel reason
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
if (!theUser)
	{
	return false;
	}

/*
 *  Check the user has sufficient access for this command..
 */

int level = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* purgeCommandLevel = bot->getLevelRequired("PURGE", "ADMIN");

if (level < purgeCommandLevel->getLevel())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("You have insufficient access to perform that command")));
	return false;
	}

/*
 * Are we trying to do a force delete?
 */
 
if("FORCE" == string_upper(st[1]))
	{
	/* Do we have enough parameters? */
	if(st.size() < 4)
		{
		Usage(theClient);
		return false;
		}
	
	/* Is this chan registered? */
	sqlChannel* theChan = bot->getChannelRecord(st[2]);
	if(theChan)
		{
		bot->Notice(theClient, "Sorry, %s is currently registered with me. Please PURGE first.",
			theChan->getName().c_str());
		return false;
		}
	
	/* This channel is not currently registered
	 * Now grab the ID as we need to clear channellog too
	 */
	
	string targetChan = string_lower(st[2]);
	
	stringstream queryChan;
	queryChan << "SELECT id FROM channels WHERE lower(name) = '"
		<< escapeSQLChars(targetChan) << "'"
		<< ends;
#ifdef LOG_SQL
	elog << "PURGEFORCE:SQL> " << queryChan.str().c_str() << endl;
#endif
	ExecStatusType statusQueryChan = bot->SQLDb->Exec(queryChan.str().c_str());
	
	if(PGRES_TUPLES_OK != statusQueryChan)
		{
		bot->Notice(theClient, "Internal database error.");
		elog << "PURGEFORCE:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}
	
	/* Did we actually find any entries? */
	
	if(bot->SQLDb->Tuples() != 1)
		{
		bot->Notice(theClient, "That channel is not in the database.");
		return false;
		}
	
	int chanID = atoi(bot->SQLDb->GetValue(0,0));
	
	/* Now clear channellog */
	stringstream delChanLog;
	delChanLog << "DELETE FROM channellog WHERE channelid = " << chanID
		<< ends;
#ifdef LOG_SQL
	elog << "PURGEFORCE:SQL> " << delChanLog.str().c_str() << endl;
#endif
	ExecStatusType statusDelChanLog = bot->SQLDb->Exec(delChanLog.str().c_str());
	
	if(PGRES_COMMAND_OK != statusDelChanLog)
		{
		bot->Notice(theClient, "Internal database error.");
		elog << "PURGEFORCE:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}

	/* Now delete the channel proper */
	
	stringstream delChan;
	delChan << "DELETE FROM channels WHERE id = " << chanID
		<< ends;
#ifdef LOG_SQL
	elog << "PURGEFORCE:SQL> " << delChan.str().c_str() << endl;
#endif
	ExecStatusType statusDelChan = bot->SQLDb->Exec(delChan.str().c_str());
	
	if(PGRES_COMMAND_OK != statusDelChan)
		{
		bot->Notice(theClient, "Internal database error.");
		elog << "PURGEFORCE:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}
	
	bot->logAdminMessage("%s (%s) - PURGE FORCE - %s - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		st[2].c_str(), st.assemble(3).c_str());
	bot->Notice(theClient, "Successfully purged %s", st[2].c_str());
	return true;
	}

/*
 *  First, check the channel isn't already registered.
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
 * Don't purge the channel if NOPURGE is set. 
 */ 

if(theChan->getFlag(sqlChannel::F_NOPURGE)) 
{ 
	bot->Notice(theClient, "%s has NOPURGE set, so I'm not purging it.", theChan->getName().c_str()); 
	return false; 
}

/* Don't purge LOCKED channels */
if(theChan->getFlag(sqlChannel::F_LOCKED)) {
	bot->Notice(theClient, "%s has LOCKED set, so I'm not purging it.",
		theChan->getName().c_str()
		);
	return false;
}

/* Don't purge commented channels. */
if(!theChan->getComment().empty()) {
	bot->Notice(theClient, "%s is commented, so I'm not purging it.",
		theChan->getName().c_str()
		);
	return false;
}

/*
 * Fetch some information about the owner of this channel, so we can
 * 'freeze' it for future investigation in the log.
 */

stringstream managerQuery;
managerQuery	<< "SELECT users.user_name,users.email "
		<< "FROM users,levels "
		<< "WHERE levels.user_id = users.id "
		<< "AND levels.access = 500 "
		<< "AND levels.channel_id = "
		<< theChan->getID()
		<< " LIMIT 1"
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlQuery> "
		<< managerQuery.str().c_str()
		<< endl;
#endif

ExecStatusType status = bot->SQLDb->Exec(managerQuery.str().c_str()) ;

string manager = "No Manager";
string managerEmail = "No Email Address";

if( status != PGRES_TUPLES_OK )
	{
	elog	<< "PURGE> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return false ;
	}
		else
	{
		if (bot->SQLDb->Tuples() != 0)
		{
			manager = bot->SQLDb->GetValue(0,0);
			managerEmail = bot->SQLDb->GetValue(0,1);
		}
	}

/*
 *  Set this channel records registered_ts to 0 (ie: not registered).
 *  The register command, and suitable PHP can re-register this channel.
 */

theChan->clearFlags(); 
theChan->setMassDeopPro(3); 
theChan->setFloodPro(7); 
theChan->setURL(""); 
theChan->setDescription(""); 
theChan->setComment(""); 
theChan->setKeywords(""); 
theChan->setRegisteredTS(0); 
theChan->setChannelMode("+tn"); 
theChan->commit(); 


stringstream theQuery ;
theQuery	<< "DELETE FROM levels WHERE channel_id = "
		<< theChan->getID()
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlQuery> "
		<< theQuery.str().c_str()
		<< endl;
#endif

status = bot->SQLDb->Exec(theQuery.str().c_str()) ;

if( status != PGRES_COMMAND_OK )
	{
	elog	<< "PURGE> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return false ;
	}

/* 
 * Bin 'em all. 
 */ 
    
cservice::sqlLevelHashType::const_iterator ptr = bot->sqlLevelCache.begin(); 
cservice::sqlLevelHashType::key_type thePair; 
    
while(ptr != bot->sqlLevelCache.end()) 
{ 
	sqlLevel* tmpLevel = ptr->second; 
        unsigned int channel_id = ptr->first.second; 
    
        if (channel_id == theChan->getID()) 
        { 
        	thePair = ptr->first; 
                elog << "Purging Level Record for: " << thePair.second << " (UID: " << thePair.first << ")" << endl; 
    
                ++ptr; 
                bot->sqlLevelCache.erase(thePair); 
    
                delete(tmpLevel); 
         } else 
         { 
         	++ptr; 
         } 
    
} 

string reason = st.assemble(2);

bot->logAdminMessage("%s (%s) - PURGE - %s - %s",
	theClient->getNickName().c_str(),
	theUser->getUserName().c_str(),
	theChan->getName().c_str(),
	reason.c_str());

bot->Notice(theClient,
	bot->getResponse(theUser, language::purged_chan,
		string("Purged channel %s")).c_str(),
	st[1].c_str());

bot->writeChannelLog(theChan,
	theClient,
	sqlChannel::EV_PURGE,
	"has purged " + theChan->getName() + " (" + reason + "), " +
	"Manager was " + manager + " (" + managerEmail + ")" );

/* Remove from cache.. part channel. */
bot->sqlChannelCache.erase(theChan->getName());
bot->sqlChannelIDCache.erase(theChan->getID());
bot->getUplink()->UnRegisterChannelEvent( theChan->getName(), bot ) ;
bot->Part(theChan->getName());
bot->joinCount--;

assert(theChan);

delete(theChan);

return true ;
}

} // namespace gnuworld.

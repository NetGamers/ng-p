/* STATUSCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"
#include	"Network.h"
#include	"cservice_config.h"

const char STATUSCommand_cc_rcsId[] = "$Id: STATUSCommand.cc,v 1.22 2004-05-16 13:08:17 jeekay Exp $" ;

namespace gnuworld
{

using std::ends ;
using std::string ;

void STATUSCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.STATUS");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
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
 *  Check the channel is actually registered.
 */

if (st[1] == "*")
	{
	/*
	 *  Special case, display admin stats.
	 */

	/* Don't show if they don't have any admin or coder-com access. */
	if (!bot->getAdminAccessLevel(theUser) && !bot->getCoderAccessLevel(theUser))
		{
		bot->Notice(theClient,
			bot->getResponse(theUser, language::chan_not_reg).c_str(),
			st[1].c_str());
		return ;
		}
	
	/*
	 *  Show some fancy stats.
	 */

	float userTotal = bot->userCacheHits + bot->userHits;
	float userEf = (bot->userCacheHits ?
		((float)bot->userCacheHits / userTotal * 100) : 0);

	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::status_tagline,
			string("CMaster Channel Services internal status:")));

	bot->Notice(theClient,
		"[User Record Stats] \002Cached Entries:\002 %i    \002DB Requests:\002 %i    \002Cache Hits:\002 %i    \002Efficiency:\002 %.2f%%",
		bot->sqlUserCache.size(),
		bot->userHits,
		bot->userCacheHits,
		userEf);

	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::status_data_alloc,
			string("Custom data containers allocated: %i")).c_str(),
			bot->customDataAlloc);

	float joinTotal = ((float)bot->joinCount / (float)Network->channelList_size()) * 100;
	bot->Notice(theClient, "I am in %i channels out of %i on the network. (%.2f%%)",
		bot->joinCount, Network->channelList_size(), joinTotal);
	
  unsigned int mins = (bot->currentTime() - bot->getUplink()->getStartTime()) / 60;
  if(0 == mins) mins = 1; // Prevent silly divide by zero errors
  float cPerMin = (float)bot->totalCommands / (float)mins;
  bot->Notice(theClient, "I've received %i commands since I started (%.2f commands per minute).",
    bot->totalCommands, cPerMin);
  
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::status_uptime,
			string("\002Uptime:\002 %s")).c_str(),
		bot->prettyDuration(bot->getUplink()->getStartTime()
			+ bot->dbTimeOffset).c_str());

	return ;
	} /// Channel query = *

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_not_reg).c_str(),
		st[1].c_str());
	return ;
	}

/*
 *  Check the user has sufficient access on this channel.
 */

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);

// Let authenticated admins view status also.
int admLevel = bot->getAdminAccessLevel(theUser);

if ((level < level::status) && (admLevel <= 0) && !theClient->isOper())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str());
	return ;
	}

/*
 *  Display some fancy info about the channel.
 */
Channel* tmpChan = Network->findChannel(theChan->getName());

if (tmpChan)
	{
	// If the person has access >200, or is a 1+ admin (or and Oper).
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::status_chan_info,
			string("Channel %s has %d users (%i operators)")).c_str(),
		tmpChan->getName().c_str(),
		tmpChan->size(),
		bot->countChanOps(tmpChan) ) ;
	if ((level >= level::status2) || (admLevel >= 1) || theClient->isOper())
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::status_mode,
				string("Mode is: %s")).c_str(),
			tmpChan->getModeString().c_str() ) ;
		}

	/*
	 * Are we on this channel?
	 */

	if (!theChan->getInChan())
		{
			bot->Notice(theClient, "I'm \002not\002 in this channel (%s).",
				theChan->getName().c_str());
		} else
		{
			bot->Notice(theClient, "I'm currently in this channel (%s).",
				theChan->getName().c_str());
		}

	} // if( tmpChan )
	else
	{
		bot->Notice(theClient, "I'm \002not\002 in this channel (%s).",
			theChan->getName().c_str());
	}

if ((level >= level::status2) || (admLevel >= 1) || theClient->isOper())
	bot->Notice(theClient, "Welcome: %s", theChan->getWelcome().c_str());

bot->Notice(theClient, "MassDeopPro: %i",
	    theChan->getMassDeopPro());

string flagsSet;
if (theChan->getFlag(sqlChannel::F_NOPURGE))     flagsSet += "NOPURGE ";
if (theChan->getFlag(sqlChannel::F_SPECIAL))     flagsSet += "SPECIAL ";
if (theChan->getFlag(sqlChannel::F_SUSPEND))     flagsSet += "SUSPEND ";
if (theChan->getFlag(sqlChannel::F_TEMP))        flagsSet += "TEMP ";
if (theChan->getFlag(sqlChannel::F_CAUTION))     flagsSet += "CAUTION ";
if (theChan->getFlag(sqlChannel::F_VACATION))    flagsSet += "VACATION ";
if (theChan->getFlag(sqlChannel::F_STRICTOP))    flagsSet += "STRICTOP ";
if (theChan->getFlag(sqlChannel::F_NOOP))        flagsSet += "NOOP ";
if (theChan->getFlag(sqlChannel::F_AUTOTOPIC))   flagsSet += "AUTOTOPIC ";
if (theChan->getFlag(sqlChannel::F_AUTOJOIN))    flagsSet += "AUTOJOIN ";
if (theChan->getFlag(sqlChannel::F_LOCKED))      flagsSet += "LOCKED ";
if (theChan->getFlag(sqlChannel::F_NOFORCE))     flagsSet += "NOFORCE ";
if (theChan->getFlag(sqlChannel::F_STRICTVOICE)) flagsSet += "STRICTVOICE ";
if (theChan->getFlag(sqlChannel::F_INVISIBLE))   flagsSet += "INVISIBLE ";
if (theChan->getFlag(sqlChannel::F_IDLE))        flagsSet += "IDLE ";
if (!theChan->getComment().empty() && admLevel)  flagsSet += "COMMENT ";
if (theChan->getFlag(sqlChannel::F_FLOATLIM)) 
           { 
           stringstream floatLim; 
           floatLim 
           << "FLOATLIM (" 
           << theChan->getLimitOffset() 
           << ":" 
           << theChan->getLimitPeriod() 
           << ":"
           << theChan->getLimitGrace()
           << ")" 
           << ends; 
           flagsSet += floatLim.str(); 
           } 

bot->Notice(theClient,
	bot->getResponse(theUser, language::status_flags,
		string("Flags set: %s")).c_str(),flagsSet.c_str());

/*
 *  Get a list of authenticated users on this channel.
 */

stringstream authQuery;
authQuery	<< "SELECT users.user_name,levels.access FROM "
		<< "users,levels WHERE users.id = levels.user_id "
		<< "AND levels.channel_id = "
		<< theChan->getID()
		<< " ORDER BY levels.access DESC"
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlQuery> "
		<< authQuery.str().c_str()
		<< endl;
#endif

ExecStatusType status = bot->SQLDb->Exec( authQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "STATUS> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return ;
	}

string authList;
string nextPerson;

bool showNick = false;

for (int i = 0 ; i < bot->SQLDb->Tuples(); i++)
	{
	/*
	 *  Look up this username in the cache.
	 */

	cservice::sqlUserHashType::iterator ptr =
		bot->sqlUserCache.find(bot->SQLDb->GetValue(i, 0));

	if(ptr != bot->sqlUserCache.end())
		{
    sqlUser* currentUser = ptr->second;
		if( !currentUser->isAuthed() ) {
			continue ;
		}

    nextPerson += bot->SQLDb->GetValue(i, 0);
    nextPerson += "/\002";

		/*
		 * Only show the online nickname if that person is in the target
		 * channel.
     *
     * Loop around all people auth'd as this nick and append their nicks
		 */
     
     string authedClients;
     
    for(sqlUser::networkClientListType::iterator ptr = currentUser->networkClientList.begin();
      ptr != currentUser->networkClientList.end(); ++ptr) {
      
      iClient* tmpClient = (*ptr);

		  showNick = false;
		  if (tmpChan) showNick = (tmpChan->findUser(tmpClient) || admLevel);

		  if (showNick)	{
        if(!authedClients.empty()) authedClients += "\002,\002";
			  authedClients += tmpClient->getNickName();
      }
    }

    nextPerson += "(";
    nextPerson += authedClients;
		nextPerson += ")\002[";
		nextPerson += bot->SQLDb->GetValue(i, 1);
		nextPerson += "] ";

		/*
		 *  Will this string overflow our Notice buffer?
		 *  If so, dump it now..
		 */
		if(nextPerson.size() + authList.size() > 400)
			{
			bot->Notice(theClient, "Auth: %s", authList.c_str());
			authList.erase( authList.begin(), authList.end() );
			}

		/*
		 * Add it on to our list.
		 */

		authList += nextPerson;
		nextPerson.erase( nextPerson.begin(), nextPerson.end() );
		}


	} // for()

bot->Notice(theClient, "Auth: %s", authList.c_str());

/*
 *  Finally(!) display a quick list of everyone 'forced' on the
 *  channel.
 */

if (admLevel >= 1)
{
	for(sqlChannel::forceMapType::const_iterator ptr = theChan->forceMap.begin();
		ptr != theChan->forceMap.end(); ++ptr)
		{
			bot->Notice(theClient, "Force: %s (%i)",
				ptr->second.second.c_str(), ptr->second.first);
		}
}

return ;
}

} // namespace gnuworld.

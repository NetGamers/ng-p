/*
 * PARTCommand.cc
 *
 * 10/02/2001 - David Henriksen <david@itwebnet.dk>
 * Initial Version. Written, and finished.
 *
 * Makes cmaster leave a registered channel.
 *
 * Caveats: None
 *
 * $Id: PARTCommand.cc,v 1.4 2004-05-16 13:08:17 jeekay Exp $
 */


#include	<string>

#include	"StringTokenizer.h"
#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"
#include	"Network.h"

const char PARTCommand_cc_rcsId[] = "$Id: PARTCommand.cc,v 1.4 2004-05-16 13:08:17 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void PARTCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.PART");

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

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_not_reg).c_str(),
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

if(theChan->getFlag(sqlChannel::F_LOCKED))
	{
	int admLevel = bot->getAdminAccessLevel(theUser);
	if(admLevel < level::set::locked)
		{
		bot->Notice(theClient, "The channel %s has been locked by a CService Administrator and I cannot be parted from there.", theChan->getName().c_str());
		return ;
		} // if(admLevel < level::set::locked)
	} // if(theChan->getFlag(sqlChannel::F_LOCKED))

/* Check the bot is in the channel. */

if (!theChan->getInChan())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::i_am_not_on_chan,
			string("I'm not in that channel!")));
	return ;
	}

/*
 *  Check the user has sufficient access on this channel.
 */

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
if (level < level::part)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str());
	return ;
	}

theChan->setInChan(false);
bot->getUplink()->UnRegisterChannelEvent(theChan->getName(), bot);
bot->joinCount--;

/* Forced access. */
if (bot->isForced(theChan, theUser))
	{
	bot->Part(theChan->getName(), "At the request of a CService Administrator");
	bot->writeChannelLog(theChan, theClient, sqlChannel::EV_PART, "[CS-ADMIN]");
	return ;
	}

/* Write a log of this event.. */
bot->writeChannelLog(theChan, theClient, sqlChannel::EV_PART, "");

string partReason = "At the request of " + theUser->getUserName();

bot->Part(theChan->getName(), partReason);
return ;
}

} // namespace gnuworld.

/*
 * JOINCommand.cc
 *
 * 10/02/2001 - David Henriksen <david@itwebnet.dk>
 * Initial Version. Written, and finished.
 *
 * Makes cmaster join a registered channel.
 *
 * Caveats: None
 */


#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"
#include	"Network.h"

#include	"sqlChannel.h"
#include	"sqlUser.h"


namespace gnuworld
{
using std::string ;

void JOINCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.JOIN");

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

/*
 *  Check the user has sufficient access on this channel.
 */

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
if (level < level::join)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str());
	return ;
	}

/* Check the bot isn't in the channel. */
if (theChan->getInChan())
	{
	bot->Notice(theClient, bot->getResponse(theUser,
		language::already_on_chan, "I'm already in that channel!"));
	return ;
	}

bot->writeChannelLog(theChan, theClient, sqlChannel::EV_JOIN, "");

theChan->setInChan(true);
theChan->removeFlag(sqlChannel::F_IDLE);
theChan->setFlag(sqlChannel::F_AUTOJOIN);
theChan->commit();
bot->getUplink()->RegisterChannelEvent( theChan->getName(), bot);
bot->Join(theChan->getName(),
	"",
	theChan->getChannelTS(),
	false);
bot->joinCount++;

/* Whack this reop on the Q */
bot->reopQ.insert(cservice::reopQType::value_type(theChan->getName(),
	bot->currentTime() + 15) );

return ;
}

} // namespace gnuworld

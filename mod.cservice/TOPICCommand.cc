/*
 * TOPICCommand.cc
 *
 * 26/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * 30/12/2000 - David Henriksen <david@itwebnet.dk>
 * Wrote TOPIC Command.
 *
 * Sets a topic in the channel.
 *
 * Caveats: None
 *
 * $Id: TOPICCommand.cc,v 1.9 2004-08-25 20:33:13 jeekay Exp $
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


void TOPICCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.TOPIC");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser)
	{
	return ;
	}

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if(!theChan)
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

/* Check the bot is in the channel. */

if (!theChan->getInChan())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::i_am_not_on_chan,
			string("I'm not in that channel!")));
	return ;
	}

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
if(level < level::topic)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str(),
		st[1].c_str());
	return ;
	}

// Cannot set topic, if the bot hasn't joined.

Channel* tmpChan = Network->findChannel(theChan->getName());
if(!tmpChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_is_empty).c_str(),
		st[1].c_str());
	return ;
	}

// Done with the checking.

string topic = st.assemble(2);

// Default ircu TOPICLEN - maxusername?
// TODO: Put into config somewhere
if( topic.size() > 230 )
	{
	bot->Notice(theClient, "ERROR: Topic cannot exceed 230 chars. You attempted to use %u characters.",
		topic.size());
	return ;
        }

stringstream s;
s	<< bot->getCharYYXXX()
	<< " T "
	<< st[1]
	<< " :("
	<< theUser->getUserName()
	<< ") "
	<< topic
	;

bot->Write( s );

return ;
}

} // namespace gnuworld.

/*
 * OPERJOINCommand.cc
 *
 * 10/02/2001 - David Henriksen <david@itwebnet.dk>
 * Initial Version. Written, and finished.
 *
 * Allows an oper to make cmaster join a registered channel.
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

namespace gnuworld
{

using std::string ;

void OPERJOINCommand::Exec( iClient* theClient, const string& Message )
{

bot->incStat("COMMANDS.OPERJOIN");
StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return ;
	}

/*
 *  Check the user is an oper.
 */
sqlUser* theUser = bot->isAuthed(theClient, false);

if(!theClient->isOper())
	{
	bot->Notice(theClient, bot->getResponse(theUser,
		language::ircops_only_cmd,
		"This command is reserved to IRC Operators"));
	return ;
	}

/*
 *  Check the channel is actually registered.
 */

Channel* tmpChan = Network->findChannel(st[1]);
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_not_reg,
		"The channel %s doesn't appear to be registered").c_str(),
		st[1].c_str());
	return ;
	}

/* Check the bot isn't in the channel. */
if (theChan->getInChan())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::already_on_chan,
		"I'm already in that channel!"));
	return ;
	}

// Tell the world.

stringstream s;
s	<< server->getCharYY()
	<< " WA :"
	<< "An IRC Operator is asking me to join channel "
	<< theChan->getName()
	;

bot->Write(s);

bot->logAdminMessage("%s is asking me to join channel %s",
		theClient->getNickUserHost().c_str(),
		theChan->getName().c_str());

bot->writeChannelLog(theChan, theClient, sqlChannel::EV_OPERJOIN, "");

theChan->setInChan(true);
bot->getUplink()->RegisterChannelEvent( theChan->getName(), bot);
bot->Join(theChan->getName(),
	theChan->getChannelMode(),
	theChan->getChannelTS(),
	false);
bot->joinCount++;

/* Whack this reop on the Q */
bot->reopQ.insert(cservice::reopQType::value_type(theChan->getName(),
	bot->currentTime() + 15) );

if (tmpChan)
	{
	if(theChan->getFlag(sqlChannel::F_NOOP))
		{
		bot->deopAllOnChan(tmpChan);
		}
	if(theChan->getFlag(sqlChannel::F_STRICTOP))
		{
		bot->deopAllUnAuthedOnChan(tmpChan);
		}
	}

return ;
}

} // namespace gnuworld.

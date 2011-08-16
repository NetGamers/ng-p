/*
 * INVITECommand.cc
 *
 * 29/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Invites a user to a channel they have access on.
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

void INVITECommand::Exec( iClient* theClient, const string& Message )
{
	bot->incStat("COMMANDS.INVITE");
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
	if (!theUser) {
		return ;
	}

	/*
	 *  Check the channel is actually registered.
	 */

	sqlChannel* theChan = bot->getChannelRecord(st[1]);
	if (!theChan) {
		bot->Notice(theClient, bot->getResponse(theUser, language::chan_not_reg).c_str(),
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

	if (!theChan->getInChan()) {
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
	if (level < level::invite)
	{
		bot->Notice(theClient, bot->getResponse(theUser, language::insuf_access).c_str());
		return ;
	}

	/*
	 *  No parameters, Just invite them to the channel.
	 */

	bot->Invite(theClient, theChan->getName());

	return ;
}

} // namespace gnuworld.

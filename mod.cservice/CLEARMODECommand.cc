/*
 * CLEARMODECommand.cc
 *
 * 16/02/2001 - David Henriksen <david@itwebnet.dk>
 * Initial version.
 *
 * Clears all channel modes.
 *
 * Caveats: None.
 *
 * Todo: Support ircu2.10.11's CLEARMODE feature.
 *
 * $Id: CLEARMODECommand.cc,v 1.7 2004-05-16 15:20:21 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"
#include	"Network.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

const char CLEARMODECommand_cc_rcsId[] = "$Id: CLEARMODECommand.cc,v 1.7 2004-05-16 15:20:21 jeekay Exp $" ;

namespace gnuworld
{

using std::ends;

void CLEARMODECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.CLEARMODE");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
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
	bot->Notice(theClient, bot->getResponse(theUser,
		language::i_am_not_on_chan, "I'm not in that channel!"));
	return ;
	}

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
if(level < level::clearmode)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str(),
		st[1].c_str());
	return ;
	}

// Cannot clear modes, if E hasn't joined.

Channel* tmpChan = Network->findChannel(theChan->getName());
if(!tmpChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_is_empty).c_str(),
		st[1].c_str());
	return ;
	}

stringstream s;
s	<< bot->getCharYYXXX()
	<< " M "
	<< st[1]
	<< " -CScimnprstkl *"
	<< ends;

tmpChan->removeMode( Channel::MODE_C );
tmpChan->removeMode( Channel::MODE_S );
tmpChan->removeMode( Channel::MODE_c );
tmpChan->removeMode( Channel::MODE_i );
tmpChan->removeMode( Channel::MODE_m );
tmpChan->removeMode( Channel::MODE_n );
tmpChan->removeMode( Channel::MODE_p );
tmpChan->removeMode( Channel::MODE_r );
tmpChan->removeMode( Channel::MODE_s );
tmpChan->removeMode( Channel::MODE_t );
tmpChan->removeMode( Channel::MODE_k );
tmpChan->removeMode( Channel::MODE_l );
tmpChan->setLimit( 0 );
tmpChan->setKey( "" );

bot->Write( s );

bot->Notice(theClient, "%s: Cleared channel modes.",
	theChan->getName().c_str());

return ;
}

} // namespace gnuworld.

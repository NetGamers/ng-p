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
 * $Id: CLEARMODECommand.cc,v 1.4 2002-09-13 21:30:38 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"
#include	"Network.h"

const char CLEARMODECommand_cc_rcsId[] = "$Id: CLEARMODECommand.cc,v 1.4 2002-09-13 21:30:38 jeekay Exp $" ;

namespace gnuworld
{

using std::ends;

bool CLEARMODECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.CLEARMODE");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return true;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser)
	{
	return false;
	}

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if(!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_not_reg).c_str(),
		st[1].c_str());
	return false;
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
	return false;
	}

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
if(level < level::clearmode)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str(),
		st[1].c_str());
	return false;
	}

// Cannot clear modes, if E hasn't joined.

Channel* tmpChan = Network->findChannel(theChan->getName());
if(!tmpChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_is_empty).c_str(),
		st[1].c_str());
	return false;
	}

stringstream s;
s	<< bot->getCharYYXXX()
	<< " M "
	<< st[1]
	<< " -mnstipkl *"
	<< ends;

tmpChan->removeMode( Channel::MODE_M );
tmpChan->removeMode( Channel::MODE_N );
tmpChan->removeMode( Channel::MODE_S );
tmpChan->removeMode( Channel::MODE_T );
tmpChan->removeMode( Channel::MODE_P );
tmpChan->removeMode( Channel::MODE_K );
tmpChan->removeMode( Channel::MODE_L );
tmpChan->removeMode( Channel::MODE_I );
tmpChan->setLimit( 0 );
tmpChan->setKey( "" );

bot->Write( s );

bot->Notice(theClient,
    bot->getResponse(theUser,
	    language::modeclear_done,
	    string("%s: Cleared channel modes.")).c_str(),
		theChan->getName().c_str());

return true ;
}

} // namespace gnuworld.

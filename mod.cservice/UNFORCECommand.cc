/* UNFORCECommand.cc */

#include	<string>

#include	"ELog.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

const char UNFORCECommand_cc_rcsId[] = "$Id: UNFORCECommand.cc,v 1.5 2004-05-16 15:20:22 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void UNFORCECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.UNFORCE");

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

sqlChannel* admChan = bot->getChannelRecord("*");

int admLevel = bot->getAccessLevel(theUser, admChan);
sqlCommandLevel* forceLevel = bot->getLevelRequired("FORCE", "ADMIN");

if (admLevel < forceLevel->getLevel())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("Sorry, you have insufficient access to "
				"perform that command.")));
	return ;
	}

/*
 *  First, check the channel is registered.
 */

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::chan_not_reg,
			string("Sorry, %s isn't registered with me.")).c_str(),
		st[1].c_str());
	return ;
	}

/*
 * Check to see if this userID is forced in this channel.
 */

sqlChannel::forceMapType::iterator ptr
	= theChan->forceMap.find(theUser->getID());

/* If we found something, drop it. */
if(ptr != theChan->forceMap.end())
	{
	theChan->forceMap.erase(theUser->getID());
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::rem_temp_access,
				string("Removed your temporary access of %i from channel %s")).c_str(),
			admLevel,
			theChan->getName().c_str());
	bot->logAdminMessage("%s (%s) - UNFORCE - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		theChan->getName().c_str());
	return ;
	}

bot->Notice(theClient,
	bot->getResponse(theUser,
		language::no_forced_access,
		string("You don't appear to have a forced access in "
			"%s, perhaps it expired?")).c_str(),
	theChan->getName().c_str());

return ;
}

} // namespace gnuworld

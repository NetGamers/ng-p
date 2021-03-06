/*
 * LBANLISTCommand.cc
 *
 * 13/01/2001 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Lists internal bot banlist for a channel.
 *
 * Caveats: None.
 */

#include	<string>

#include	<ctime>

#include	"StringTokenizer.h"
#include	"cservice.h"
#include	"levels.h"
#include	"match.h"
#include	"responses.h"
#include	"cservice_config.h"
#include	"time.h"

#include	"sqlChannel.h"
#include	"sqlUser.h"


namespace gnuworld
{
using std::string ;
using namespace level;

void LBANLISTCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.LBANLIST");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}

// Is the channel registered?

sqlUser* theUser = bot->isAuthed(theClient,true);
if(!theUser)
	{
	return ;
	}
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if(!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::chan_not_reg,
			string("Sorry, %s isn't registered with me.")).c_str(),
		st[1].c_str());
	return ;
	}

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
int admLevel = bot->getAdminAccessLevel(theUser);

if( (level < level::banlist) && (admLevel < level::globalbanlist) )
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
	return ;
	}

/* Show all results? */
bool showAll = false;

for( StringTokenizer::const_iterator ptr = st.begin() ; ptr != st.end() ;
	++ptr )
	{
	if (string_lower(*ptr) == "-all")
		{
		sqlUser* tmpUser = bot->isAuthed(theClient, false);
		if ( tmpUser && bot->getAdminAccessLevel(tmpUser) )
			{
			showAll = true;
			}
		continue;
		}
	}

bot->Notice(theClient,
	bot->getResponse(theUser,
		language::lbanlist_for,
		string("\002*** Ban List for channel %s ***\002")).c_str(),
	theChan->getName().c_str());

size_t results = 0;
time_t ban_expires = 0;
time_t ban_expires_d = 0;
time_t ban_expires_f = 0;

for( vector< sqlBan* >::const_iterator ptr = theChan->banList.begin() ; ptr != theChan->banList.end() ; ++ptr )
	{
	sqlBan* theBan = (*ptr);

	/* If its expired.. just don't show it - it'll be removed soon ;) */
	if (theBan->getExpires() >= bot->currentTime())
		{
		if (match(st[2], theBan->getBanMask()) == 0)
			{
			results++;
			ban_expires = theBan->getExpires();
			ban_expires_d = ban_expires - bot->currentTime();
			ban_expires_f = bot->currentTime() - ban_expires_d;

			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::lban_entry,
					string("%s %s Level: %i")).c_str(),
				theChan->getName().c_str(),
				theBan->getBanMask().c_str(),
				theBan->getLevel());
			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::lban_added_by,
					string("ADDED BY: %s (%s)")).c_str(),
				theBan->getSetBy().c_str(),
				theBan->getReason().c_str());
			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::lban_since,
					string("SINCE: %s")).c_str(),
				ctime(&theBan->getSetTS()));
			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::lban_exp,
					string("EXP: %s")).c_str(),
				bot->prettyDuration(ban_expires_f).c_str());
			}
		}
	if( (results >= MAX_LBAN_RESULTS) && !showAll)
		{
		break;
		}
	} // for()

if( (results >= MAX_LBAN_RESULTS) && !showAll)
	{
	bot->Notice(theClient, "There are more than %u matching entries.",
		MAX_LBAN_RESULTS
		);
	bot->Notice(theClient, "Please restrict your query or consult "
		"http://www.netgamers.org/."
		);
	}
else if (results > 0)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::lban_end,
			string("\002*** END ***\002")));
	}
else
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::no_match,
			string("No match!")));
	}

return ;
}

} // namespace gnuworld.


/*
 * VOICECommand.cc
 *
 * 20/12/2000 - Perry Lorier <perry@coders.net>
 * Initial Version.
 *
 * 28/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Added multilingual support.
 *
 * 01/01/2001 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Added duplicate checking to avoid people doing:
 * /msg e voice #coder-com Gte Gte Gte Gte Gte Gte Gte {etc}
 * And flooding the target with notices.
 *
 * 2001-03-21 - Perry Lorier <Isomer@coders.net>
 * Added "on chan" to the message
 *
 * Voice's one or more users on a channel the user as access on.
 *
 *
 * Caveats: None
 */

#include	<string>
#include	<map>
#include	<vector>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"Network.h"
#include	"levels.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlUser.h"

using std::map ;
using std::vector ;


namespace gnuworld
{
using std::map ;
using std::vector ;

void VOICECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.VOICE");

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
		bot->getResponse(theUser,
			language::chan_not_reg).c_str(),
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

/*
 *  Check the user has sufficient access on this channel.
 */

int level = bot->getEffectiveAccessLevel(theUser, theChan, true);
if (level < level::voice)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str());
	return ;
	}

Channel* tmpChan = Network->findChannel(theChan->getName());
if (!tmpChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_is_empty).c_str(),
		theChan->getName().c_str());
	return ;
	}

vector< iClient* > voiceList; // List of clients to Voice.

/*
 *  Loop over the remaining 'nick' parameters, voicing them all.
 */

char delim = 0;
unsigned short counter = 2; // Offset of first nick in list.

string source;
iClient* target = 0;

typedef map < iClient*, int > duplicateMapType;
duplicateMapType duplicateMap;

if( st.size() < 3 )
	{
	// No nicks provided, assume we voice ourself. :)
	voiceList.push_back(theClient);
	source = Message;
	delim = ' ';
	}
else
	{
	string::size_type pos = st[2].find_first_of( ',' ) ;

	// Found a comma?
	if( string::npos != pos )
		{
		// We'll do a comma seperated search then.
		source = st.assemble(2);
		delim = ',';
		counter = 0;
		}
	else
		{
		source = Message;
		delim = ' ';
		}
	}

StringTokenizer st2( source, delim );

while (counter < st2.size())
	{
	target = Network->findNick(st2[counter]);

	if(!target)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::dont_see_them).c_str(),
			st2[counter].c_str());
		counter++;
		continue ;
		}

	ChannelUser* tmpChanUser = tmpChan->findUser(target) ;

	// User isn't on the channel?
	if (!tmpChanUser)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::cant_find_on_chan).c_str(),
			target->getNickName().c_str(),
			theChan->getName().c_str());
		counter++;
		continue ;
		}

	// User is already voiced?
	if(tmpChanUser->getMode(ChannelUser::MODE_V))
		{
		bot->Notice(theClient, bot->getResponse(theUser, language::already_voiced).c_str(),
			target->getNickName().c_str(), theChan->getName().c_str());
		++counter;
		continue ;
		}

	sqlBan* tmpBan = bot->isBannedOnChan(theChan, tmpChanUser->getClient());
	if( tmpBan && (tmpBan->getLevel() >= 25) )
	{
		bot->Notice(theClient, "%s isn't allowed to be voiced in %s",
								tmpChanUser->getClient()->getNickName().c_str(),
								theChan->getName().c_str());
		++counter;
		continue;
	}

	sqlUser* authUser = bot->isAuthed(tmpChanUser->getClient(), false);

#ifdef FEATURE_STRICTVOICE
	if(theChan->getFlag(sqlChannel::F_STRICTVOICE))
		{
		if(!authUser)
			{
			bot->Notice(theClient, "The STRICTVOICE flag is set on %s (and %s is not authenticated)",
									theChan->getName().c_str(), tmpChanUser->getNickName().c_str());
			++counter;
			continue;
			}
		else if(!(bot->getEffectiveAccessLevel(authUser, theChan, false) >= level::voice))
			{
			bot->Notice(theClient, "The STRICTVOICE flag is set on %s (and %s has insufficient access)",
									theChan->getName().c_str(), authUser->getUserName().c_str());
			++counter;
			continue;
			}
		}
#endif

	// Check for duplicates.
	duplicateMapType::iterator ptr = duplicateMap.find(target);
	if(ptr == duplicateMap.end())
		{
		// Not a duplicate.
		voiceList.push_back(target);
		duplicateMap.insert(duplicateMapType::value_type(target, 0));

		// Don't send a notice to the person who issued the command.
		if(target != theClient)
			{
				bot->Notice(target, "You are voiced in %s by %s", theChan->getName().c_str(),
																													theClient->getNickName().c_str());
			} // Don't send to person who issued.
	  } // Not a duplicate.

	counter++;
	}

// Voice them.
bot->Voice(tmpChan, voiceList);
return ;
}

} // namespace gnuworld

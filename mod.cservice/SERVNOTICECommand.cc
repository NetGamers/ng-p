/*
 * SERVNOTICECommand.cc
 *
 * 03/01/2001 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Sends a server notice to a channel.
 *
 * Caveats: None
 *
 * $Id: SERVNOTICECommand.cc,v 1.3 2004-05-16 13:08:17 jeekay Exp $
 */

#include	<string>
#include	<map>

#include	"ELog.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

const char SERVNOTICECommand_cc_rcsId[] = "$Id: SERVNOTICECommand.cc,v 1.3 2004-05-16 13:08:17 jeekay Exp $" ;

namespace gnuworld
{
using std::map ;

void SERVNOTICECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.SERVNOTICE");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
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
 *  Check the user has sufficient admin access to do this.
 */

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* servnoticeLevel = bot->getLevelRequired("SERVNOTICE", "ADMIN");

if (admLevel < servnoticeLevel->getLevel())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access).c_str());
	return ;
	}

Channel* tmpChan = Network->findChannel(st[1]);
if (!tmpChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_is_empty).c_str(),
		st[1].c_str());
	return ;
	}

string theMessage = st.assemble(2);
bot->serverNotice(tmpChan, theMessage.c_str());

return ;
}

} // namespace gnuworld.


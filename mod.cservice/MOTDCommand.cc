/* MOTDCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include 	"responses.h"

const char MOTDCommand_cc_rcsId[] = "$Id: MOTDCommand.cc,v 1.1 2002-01-14 23:14:18 morpheus Exp $" ;

namespace gnuworld
{
using std::string ;

bool MOTDCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.MOTD");

StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return true;
	}

	sqlUser* theUser = bot->isAuthed(theClient, false);

	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::motd,
			string("No MOTD set.")));

return true ;
}

} // namespace gnuworld.

/* MOTDCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include 	"responses.h"

const char MOTDCommand_cc_rcsId[] = "$Id: MOTDCommand.cc,v 1.3 2004-05-16 13:08:16 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void MOTDCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.MOTD");

StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return ;
	}

	sqlUser* theUser = bot->isAuthed(theClient, false);

/*	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::motd,
			string("No MOTD set."))); */
	
	bot->sendMOTD(theClient);

return ;
}

} // namespace gnuworld.

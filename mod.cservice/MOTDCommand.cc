/* MOTDCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include 	"responses.h"

const char MOTDCommand_cc_rcsId[] = "$Id: MOTDCommand.cc,v 1.2 2002-06-09 09:04:44 jeekay Exp $" ;

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

/*	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::motd,
			string("No MOTD set."))); */
	
	bot->sendMOTD(theClient);

return true ;
}

} // namespace gnuworld.

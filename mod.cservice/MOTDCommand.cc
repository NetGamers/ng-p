/* MOTDCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include 	"responses.h"

const char MOTDCommand_cc_rcsId[] = "$Id: MOTDCommand.cc,v 1.4 2004-05-16 15:20:21 jeekay Exp $" ;

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

bot->sendMOTD(theClient);

return ;
}

} // namespace gnuworld.

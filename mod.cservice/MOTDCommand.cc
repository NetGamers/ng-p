/* MOTDCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include 	"responses.h"


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

/* STATSCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"

const char STATSCommand_cc_rcsId[] = "$Id: STATSCommand.cc,v 1.2 2004-05-16 13:08:17 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void STATSCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.STATS");

sqlUser* theUser = bot->isAuthed(theClient, false);
if (!theUser)
	{
	return ;
	}

int admLevel = bot->getAdminAccessLevel(theUser);
int coderLevel = bot->getCoderAccessLevel(theUser);
if (!admLevel && !coderLevel) return ;

bot->Notice(theClient, "P Command/SQL Query Statistics:");
for( cservice::statsMapType::iterator ptr = bot->statsMap.begin() ;
	ptr != bot->statsMap.end() ; ++ptr )
	{
	bot->Notice(theClient, "%s: %i", ptr->first.c_str(), ptr->second);
	}

bot->Notice(theClient, "-- End of STATS");

return ;
}

} // namespace gnuworld.

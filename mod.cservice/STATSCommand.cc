/* STATSCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"

const char STATSCommand_cc_rcsId[] = "$Id: STATSCommand.cc,v 1.1 2002-01-14 23:14:22 morpheus Exp $" ;

namespace gnuworld
{
using std::string ;

bool STATSCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.STATS");

sqlUser* theUser = bot->isAuthed(theClient, false);
if (!theUser)
	{
	return false;
	}

int admLevel = bot->getAdminAccessLevel(theUser);
int coderLevel = bot->getCoderAccessLevel(theUser);
if (!admLevel && !coderLevel) return false;

bot->Notice(theClient, "P Command/SQL Query Statistics:");
for( cservice::statsMapType::iterator ptr = bot->statsMap.begin() ;
	ptr != bot->statsMap.end() ; ++ptr )
	{
	bot->Notice(theClient, "%s: %i", ptr->first.c_str(), ptr->second);
	}

bot->Notice(theClient, "-- End of STATS");

return true ;
}

} // namespace gnuworld.

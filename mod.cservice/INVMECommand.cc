/* INVMECommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 
#include	"cservice.h" 
#include	"levels.h"
#include	"responses.h"

const char INVMECommand_cc_rcsId[] = "$Id: INVMECommand.cc,v 1.2 2002-05-28 18:54:33 jeekay Exp $" ;

namespace gnuworld
{

using namespace gnuworld;
 
bool INVMECommand::Exec( iClient* theClient, const string& Message )
{ 
	bot->incStat("COMMANDS.INVME");
	
	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return false;

	int admLevel = bot->getAdminAccessLevel(theUser);
	if (admLevel < level::invme)
		{
		bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
		return false;
		}

	bot->Invite(theClient, bot->relayChan);

	return true ;
} 

} // namespace gnuworld.


/* INVMECommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 

#include	"cservice.h" 
#include	"responses.h"

const char INVMECommand_cc_rcsId[] = "$Id: INVMECommand.cc,v 1.4 2002-10-20 02:12:07 jeekay Exp $" ;

namespace gnuworld
{

bool INVMECommand::Exec( iClient* theClient, const string& Message )
{ 
	bot->incStat("COMMANDS.INVME");
	
	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return false;

	int admLevel = bot->getAdminAccessLevel(theUser);
  sqlCommandLevel* invMeLevel = bot->getLevelRequired("INVME", "ADMIN");
  
	if (admLevel < invMeLevel->getLevel())
		{
		bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
		return false;
		}

	bot->Invite(theClient, bot->relayChan);

	return true ;
} 

} // namespace gnuworld.


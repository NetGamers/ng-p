/* INVMECommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 

#include	"cservice.h" 
#include	"responses.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{

void INVMECommand::Exec( iClient* theClient, const string& Message )
{ 
	bot->incStat("COMMANDS.INVME");
	
	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser) return ;

	int admLevel = bot->getAdminAccessLevel(theUser);
	sqlCommandLevel* invMeLevel = bot->getLevelRequired("INVME", "ADMIN");
  
	if (admLevel < invMeLevel->getLevel())
		{
		bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
		return ;
		}

	bot->Invite(theClient, bot->relayChan);

	return ;
} 

} // namespace gnuworld.


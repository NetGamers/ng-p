/* INVMECommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 

#include	"cservice.h" 
#include	"responses.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

const char INVMECommand_cc_rcsId[] = "$Id: INVMECommand.cc,v 1.6 2004-05-16 15:20:21 jeekay Exp $" ;

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


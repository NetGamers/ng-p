/* INVMECommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 
#include	"cservice.h" 
#include	"levels.h"
#include	"responses.h"

const char INVMECommand_cc_rcsId[] = "$Id: INVMECommand.cc,v 1.1 2002-01-14 23:14:17 morpheus Exp $" ;

namespace gnuworld
{

using namespace gnuworld;
 
bool INVMECommand::Exec( iClient* theClient, const string& Message )
{ 
	bot->incStat("COMMANDS.INVME");
        sqlUser* theUser = bot->isAuthed(theClient, true);
        if (!theUser) return false;

        sqlChannel* admChan = bot->getChannelRecord("*");

        int admLevel = bot->getAccessLevel(theUser, admChan);
        if (admLevel < level::invme)
	        {
	        bot->Notice(theClient,
	                bot->getResponse(theUser,
	                        language::insuf_access,
	                	string("Sorry, you have insufficient access to perform that command.")));
		return false;
		}

        strstream s;

	s << bot->getCharYYXXX() << " I " << theClient->getNickName() << " "
	  << bot->relayChan << ends;

	bot->Write( s ) ;
	delete[] s.str() ;
		  
	return true ;
} 

} // namespace gnuworld.


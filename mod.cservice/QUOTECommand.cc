/* QUOTECommand.cc */

#include	<string>

#include	"ELog.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{

using std::string ;

void QUOTECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.QUOTE");

StringTokenizer st( Message ) ;
if( st.size() < 1 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser)
	{
	return ;
 	}

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* quoteCommandLevel = bot->getLevelRequired("QUOTE", "ADMIN");

if (admLevel < quoteCommandLevel->getLevel())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("Sorry, you have insufficient access to perform that command.")));
	return ;
	}

bot->Write( st.assemble(1) );

return ;
}

} // namespace gnuworld.

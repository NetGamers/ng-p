/*
 * SHUTDOWNCommand.cc
 *
 * 28/12/2001 - Matthias Crauwels <ultimate_@wol.be>
 * Initial Version.
 *
 * Shuts down the bot and squits the server
 *
 */

#include	<string>

#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

const char SHUTDOWNCommand_cc_rcsId[] = "$Id: SHUTDOWNCommand.cc,v 1.4 2004-05-16 13:08:17 jeekay Exp $" ;
namespace gnuworld
{
using std::string ;

void SHUTDOWNCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.SHUTDOWN");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return ;
	}

/*
 *  Fetch the sqlUser record attached to this client. If there isn't one,
 *  they aren't logged in - tell them they should be.
 */

sqlUser* theUser = bot->isAuthed(theClient, false);
if (!theUser)
	{
	return ;
	}

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* shutdownLevel = bot->getLevelRequired("SHUTDOWN", "ADMIN");

if (admLevel < shutdownLevel->getLevel())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("Sorry, you have insufficient access to perform that command.")));
	return ;
	}

bot->logAdminMessage("%s issued the shutdown command! Bye! xxx!", theClient->getNickName().c_str());
bot->Exit( st.assemble(1).c_str());

char buf[ 512 ] = { 0 } ;

sprintf( buf, "%s SQ %s :%s made me shutdown (%s)\n", bot->getCharYY(), bot->getUplinkName().c_str(),
			theClient->getNickName().c_str(), st.assemble(1).c_str() );

bot->QuoteAsServer( buf );

return ;
}

} // namespace gnuworld.


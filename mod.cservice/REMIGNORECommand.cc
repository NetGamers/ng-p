/* REMIGNORECommand.cc */

#include	<string>

#include	"ELog.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

const char REMIGNORECommand_cc_rcsId[] = "$Id: REMIGNORECommand.cc,v 1.7 2004-05-16 15:20:22 jeekay Exp $" ;

namespace gnuworld
{

using std::ends ;
using std::string ;

void REMIGNORECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.REMIGNORE");

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

sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser)
	{
	return ;
 	}

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* remIgnoreLevel = bot->getLevelRequired("REMIGNORE", "ADMIN");

if (admLevel < remIgnoreLevel->getLevel())
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("Sorry, you have insufficient access to perform that command.")));
	return ;
	}

for( cservice::silenceListType::iterator ptr = bot->silenceList.begin() ;
	ptr != bot->silenceList.end() ; ++ptr )
	{
	if ( string_lower(st[1]) == string_lower(ptr->first.c_str()) )
		{
		stringstream s;
		s	<< bot->getCharYYXXX()
			<< " SILENCE * -"
			<< ptr->first.c_str()
			<< ends;
		bot->Write( s );

		/*
		 * Locate this user by numeric.
		 * If the numeric is still in use, clear the ignored flag.
		 * If someone else has inherited this numeric, no prob,
		 * its cleared anyway.
		 */

		iClient* netClient = Network->findClient(ptr->second.second);
		if (netClient)
			{
			bot->setIgnored(netClient, false);
			}

		bot->silenceList.erase(ptr);
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::unsilenced,
				string("Removed %s from my ignore list")).c_str(),
			st[1].c_str());
		bot->logAdminMessage("%s (%s) - REMIGNORE - %s",
			theClient->getNickName().c_str(),
			theUser->getUserName().c_str(),
			st[1].c_str());
		return ;
		}

	} // for()

bot->Notice(theClient,
	bot->getResponse(theUser,
		language::couldnt_find_silence,
		string("Couldn't find %s in my silence list")).c_str(),
	st[1].c_str());

return ;
}

} // namespace gnuworld

/* JUPECommand.cc */

#include <string>
#include <iomanip.h>

#include "StringTokenizer.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"

const char JUPECommand_cc_rcsId[] = "$Id: JUPECommand.cc,v 1.1 2002-02-04 04:45:34 jeekay Exp $";

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool JUPECommand::Exec( iClient* theClient, const string& Message )
{

nsUser* theUser = static_cast< nsUser* >(theClient->getCustomData(bot));

StringTokenizer st( Message );

if(st.size() != 3)
{
	Usage(theClient);
	return true;
}

string option = string_upper(st[1]);

int adminAccess = bot->getAdminAccessLevel(theUser->getLoggedNick());

if("ADD" == option && (adminAccess >= level::jupeadd))
	{
	iClient* targetClient = Network->findNick(st[2]);
	if(targetClient)
		{ // We cant jupe someone already on the network!
		bot->Notice(theClient, "%s already exists on the network!", st[2].c_str());
		return false;
		}
	
	if(bot->jupeNick(st[2]))
		{
		bot->Notice(theClient, "%s successfully juped.", st[2].c_str());
		}
	else
		{
		bot->Notice(theClient, "Unable to jupe %s.", st[2].c_str());
		}
	return true;
	}

if("DEL" == option && (adminAccess >= level::jupedel))
	{
	if(bot->removeJupeNick(st[2]))
		{
		bot->Notice(theClient, "Jupe for %s successfully deleted.", st[2].c_str());
		}
	else
		{
		bot->Notice(theClient, "Unable to remove jupe for %s.", st[2].c_str());
		}
	return true;
	}

if("FORCEADD" == option && (adminAccess >= level::jupeforce))
	{
	if(bot->jupeNick(st[2]))
		{
		bot->Notice(theClient, "%s successfully juped.", st[2].c_str());
		}
	else
		{
		bot->Notice(theClient, "Unable to jupe %s.", st[2].c_str());
		}
	return true;
	}

return true;

} // JUPECommand::Exec

} // Namespace nserv

} // Namespace gnuworld

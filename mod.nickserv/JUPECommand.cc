/* JUPECommand.cc */

#include <string>
#include <iomanip.h>
#include <time.h>

#include "StringTokenizer.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"

const char JUPECommand_cc_rcsId[] = "$Id: JUPECommand.cc,v 1.2 2002-02-05 02:24:06 jeekay Exp $";

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

if("ADD" == option && (adminAccess >= level::jupe::add))
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

if("DEL" == option && (adminAccess >= level::jupe::del))
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

if("FORCEADD" == option && (adminAccess >= level::jupe::force))
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

if("INFO" == option && (adminAccess >= level::jupe::info))
	{
	juUser* theUser = bot->findJupeNick(st[2]);
	if(!theUser)
		{
		bot->Notice(theClient, "No active jupe found for %s.", st[2].c_str());
		}
	else
		{
		bot->Notice(theClient, "Jupe information for %s:", st[2].c_str());
		bot->Notice(theClient, "Jupe Numeric   : %s%s", bot->getUplinkCharYY(), theUser->getNumeric().c_str());
		bot->Notice(theClient, "Jupe Activated : %s", ctime(theUser->getSet()));
		bot->Notice(theClient, "Jupe Expires   : %s", ctime(theUser->getExpires()));
		bot->Notice(theClient, "Jupe Reason    : %s", theUser->getReason().c_str());
		}
	return true;
	}

return true;

} // JUPECommand::Exec

} // Namespace nserv

} // Namespace gnuworld

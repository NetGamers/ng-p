/* JUPECommand.cc */

#include <string>
#include <iomanip.h>
#include <time.h>

#include "StringTokenizer.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"

const char JUPECommand_cc_rcsId[] = "$Id: JUPECommand.cc,v 1.3 2002-02-05 03:13:45 jeekay Exp $";

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool JUPECommand::Exec( iClient* theClient, const string& Message )
{

nsUser* theUser = static_cast< nsUser* >(theClient->getCustomData(bot));

StringTokenizer st( Message );

// JUPE (add|del|forceadd|info) nick (duration) (reason)

if(st.size() < 3)
{
	Usage(theClient);
	return true;
}

string nick, reason;
unsigned int duration;

nick = st[2];

if(st.size() >= 4)
	{
	if(IsNumeric(st[3]))
		{
		duration = atoi(st[3].c_str());
		if(st.size() > 4) { reason = st.assemble(4); }
		else { reason = "Admin Juped Nick"; }
		}
	else
		{
		duration = 0;
		reason = st.assemble(3);
		}
	}
else
	{
	duration = 0;
	reason = "Admin Juped Nick";
	}

string option = string_upper(st[1]);

int adminAccess = bot->getAdminAccessLevel(theUser->getLoggedNick());

if("ADD" == option && (adminAccess >= level::jupe::add))
	{
	iClient* targetClient = Network->findNick(nick);
	if(targetClient)
		{ // We cant jupe someone already on the network!
		bot->Notice(theClient, "%s already exists on the network!", nick.c_str());
		return false;
		}
	
	if(bot->jupeNick(nick, reason, duration))
		{
		bot->Notice(theClient, "%s successfully juped.", nick.c_str());
		}
	else
		{
		bot->Notice(theClient, "Unable to jupe %s.", nick.c_str());
		}
	return true;
	}

if("DEL" == option && (adminAccess >= level::jupe::del))
	{
	if(bot->removeJupeNick(nick))
		{
		bot->Notice(theClient, "Jupe for %s successfully deleted.", nick.c_str());
		}
	else
		{
		bot->Notice(theClient, "Unable to remove jupe for %s.", nick.c_str());
		}
	return true;
	}

if("FORCEADD" == option && (adminAccess >= level::jupe::force))
	{
	if(bot->jupeNick(nick, reason, duration))
		{
		bot->Notice(theClient, "%s successfully juped.", nick.c_str());
		}
	else
		{
		bot->Notice(theClient, "Unable to jupe %s.", nick.c_str());
		}
	return true;
	}

if("INFO" == option && (adminAccess >= level::jupe::info))
	{
	juUser* theUser = bot->findJupeNick(nick);
	if(!theUser)
		{
		bot->Notice(theClient, "No active jupe found for %s.", nick.c_str());
		}
	else
		{
		bot->Notice(theClient, "Jupe information for %s:", theUser->getNickName().c_str());
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

/* JUPECommand.cc */

#include <string>
#include <time.h>

#include "StringTokenizer.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"

const char JUPECommand_cc_rcsId[] = "$Id: JUPECommand.cc,v 1.7 2002-09-13 21:27:34 jeekay Exp $";

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool JUPECommand::Exec( iClient* theClient, const string& Message )
{

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

iClient* targetClient = Network->findNick(nick);
int adminAccess = bot->getAdminAccessLevel(theClient);

if("ADD" == option && (adminAccess >= level::jupe::add))
	{
	if(targetClient)
		{ // We cant jupe someone already on the network!
		bot->Notice(theClient, "%s already exists on the network!", nick.c_str());
		return false;
		}
	
	if(bot->jupeNick(nick, "unknown", reason, duration*3600))
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
	string hostMask;
	if(targetClient) { hostMask = targetClient->getNickUserHost(); }
	else { hostMask = "unknown"; }
	if(bot->jupeNick(nick, hostMask, reason, duration*3600))
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
		bot->Notice(theClient, "Last Hostmask  : %s", theUser->getHostMask().c_str());
		bot->Notice(theClient, "Jupe Reason    : %s", theUser->getReason().c_str());
		}
	return true;
	}

return true;

} // JUPECommand::Exec

} // Namespace nserv

} // Namespace gnuworld

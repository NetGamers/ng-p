/* STATSCommand.cc */

#include <string>
#include <iomanip.h>

#include "StringTokenizer.h"
#include "ELog.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"
#include "libpq-int.h"

const char STATSCommand_cc_rcsId[] = "$Id: STATSCommand.cc,v 1.3 2002-02-08 23:22:32 jeekay Exp $";

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool STATSCommand::Exec( iClient* theClient, const string& Message )
{

nsUser* theUser = static_cast< nsUser* >( theClient->getCustomData(bot) );
string chanName = bot->getDebugChannel();
Channel* theChannel = Network->findChannel(chanName);

if(!theChannel)
{
	bot->Notice(theClient, "Unable to locate debug channel on the network.");
	return true;
}

StringTokenizer st( Message );
if(st.size() != 2)
{
	Usage(theClient);
	return true;
}

string option = string_upper(st[1]);

if(option == "ALL" && (bot->getAdminAccessLevel(theUser->getLoggedNick()) >= level::stats::all))
{
	bot->Message(chanName, "All stats:");
	bot->Message(chanName, "My Numeric    : %s%s", bot->getCharYY(), bot->getCharXXX());
	bot->Message(chanName, "Total Clients : %d", static_cast< int >(Network->clientList_size()));
	return true;
}

if("PID" == option && (bot->getAdminAccessLevel(theUser->getLoggedNick()) >= level::stats::pid))
{
	const cmDatabase* SQLDb = bot->getSQLDb();
	bot->Notice(theClient, "Current Backend SQL PID: %d", SQLDb->getPID());
	return true;
}

return true;

} // STATSCommand::Exec

} // Namespace nserv

} // Namespace gnuworld

/* STATSCommand.cc */

#include <string>
#include <iomanip.h>

#include "StringTokenizer.h"
#include "ELog.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"

const char STATSCommand_cc_rcsId[] = "$Id: STATSCommand.cc,v 1.1 2002-01-23 01:05:47 jeekay Exp $";

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

if(option == "ALL" && bot->getAdminAccessLevel(theUser->getLoggedNick()))
{
	bot->Message(chanName, "All stats:");
	bot->Message(chanName, "My Numeric    : %s%s", bot->getCharYY(), bot->getCharXXX());
	bot->Message(chanName, "Total Clients : %d", static_cast< int >(Network->clientList_size()));
	return true;
}

return true;

} // STATSCommand::Exec

} // Namespace nserv

} // Namespace gnuworld

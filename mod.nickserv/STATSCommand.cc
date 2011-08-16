/* STATSCommand.cc */

#include <string>

#include "StringTokenizer.h"
#include "ELog.h"
#include "Network.h"
#include "nickserv.h"
#include "levels.h"

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool STATSCommand::Exec( iClient* theClient, const string& Message )
{

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

int adminAccess = bot->getAdminAccessLevel(theClient);

if(option == "ALL" && (adminAccess >= level::stats::all))
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

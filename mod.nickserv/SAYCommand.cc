/*
 * SAYCommand.cc
 *
 * 20020130 - Jeekay - Initial version
 *
 * $Id: SAYCommand.cc,v 1.2 2002-02-04 04:26:36 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"
#include "nickserv.h"
#include "levels.h"
#include "Network.h"

namespace gnuworld
{

namespace nserv
{

using std::string;

bool SAYCommand::Exec( iClient* theClient, const string& Message )
{

StringTokenizer st( Message );
if(st.size() < 3)
	{
	Usage(theClient);
	return true;
	}

nsUser* theUser = static_cast< nsUser* >( theClient->getCustomData(bot) );
int admLevel = bot->getAdminAccessLevel(theUser->getLoggedNick());
if(admLevel < level::say)
	{
	return false;
	} // Not enough access

Channel* theChan = Network->findChannel(st[1]);
if(!theChan)
	{
	bot->Notice(theClient, "Channel not found");
	return false;
	} // Channel doesnt exist

bot->Message(st[1], st.assemble(2).c_str());

return true;

} // SAYCommand::Exec

} // namespace nserv

} // namespace gnuworld

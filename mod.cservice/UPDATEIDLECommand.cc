/* UPDATECommand.cc
 *
 * Author: Jeekay - Originally written for NetGamers
 *
 * Purpose: Parts any channel that has only 1 user in it.
 *
 * Insofar as a single source file can have a licence, this one is GPL'd
 */

#include	<string>

#include	"ELog.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

namespace gnuworld
{
using std::string ;

void UPDATEIDLECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.UPDATEIDLE");

StringTokenizer st( Message ) ;
if( st.size() != 1 ) {
  Usage(theClient);
  return ;
}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return ; }

int aLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* updatedbLevel = bot->getLevelRequired("UPDATEIDLE", "ADMIN");

if(aLevel < updatedbLevel->getLevel()) { return ; }

bot->Notice(theClient, "Starting updateidle routine...");

gnuworld::cservice::sqlChannelIDHashType::const_iterator myChanItr;

int mismatchCount = 0;

for(myChanItr = bot->sqlChannelIDCache.begin(); myChanItr != bot->sqlChannelIDCache.end(); myChanItr++) {
  sqlChannel* sqlChan = myChanItr->second;
  if(!sqlChan) {
    bot->Notice(theClient, "Weird error with channel id %d", myChanItr->first);
    continue;
  }

  if(!sqlChan->getInChan()) { continue; }

  Channel* netChan = Network->findChannel(sqlChan->getName());
  if(!netChan) { continue; }
  
  bool servicesOnly = true;
  Channel::const_userIterator userItr = netChan->userList_begin();
  for( ; userItr != netChan->userList_end() ; ++userItr ) {
  	ChannelUser *theChanUser = userItr->second;
  	/* Check if the user is either a service or special */
	if( theChanUser->getIntYY() != 50 &&
	    theChanUser->getIntYY() != 40 ) {
		servicesOnly = false;
		break;
	}
	
  }
  
  if( ! servicesOnly) { continue; }

	/* We are the only person in this channel */
	bot->Notice(theClient, "Channel %s is idle.",
		netChan->getName().c_str()
		);
	
	bot->partIdleChannel(sqlChan);
	
	mismatchCount++;
}

bot->Notice(theClient, "Total idle channels found: %d", mismatchCount);

bot->logAdminMessage("%s (%s) - UPDATEIDLE - Parted %d channels",
  theClient->getNickName().c_str(), theUser->getUserName().c_str(),
  mismatchCount);

return ;
}

} // namespace gnuworld.

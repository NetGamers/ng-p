/* UPDATECommand.cc
 *
 * Author: Jeekay - Originally written for NetGamers
 *
 * Purpose: Resets any database channel timestamps that dont match the
 *          network timestamps. This is when the DB TS is 0, or if the
 *          DB timestamp has managed to be larger than the channel TS.
 *
 * Insofar as a single source file can have a licence, this one is GPL'd
 */

#include	<string>

#include	"ELog.h"
#include  "Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"

const char UPDATEDBCommand_cc_rcsId[] = "$Id: UPDATEDBCommand.cc,v 1.6 2002-11-09 20:38:01 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

bool UPDATEDBCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.UPDATEDB");

StringTokenizer st( Message ) ;
if( st.size() != 1 ) {
  Usage(theClient);
  return true;
}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return 0; }

int aLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* updatedbLevel = bot->getLevelRequired("UPDATEDB", "ADMIN");

if(aLevel < updatedbLevel->getLevel()) { return 0; }

bot->Notice(theClient, "Starting updatedb routine...");

gnuworld::cservice::sqlChannelIDHashType::const_iterator myChanItr;

int mismatchCount = 0;

for(myChanItr = bot->sqlChannelIDCache.begin(); myChanItr != bot->sqlChannelIDCache.end(); myChanItr++) {
  sqlChannel* myChan = myChanItr->second;
  if(!myChan) {
    bot->Notice(theClient, "Wierd error with channel id %d", myChanItr->first);
    continue;
  }

  Channel* netChan = Network->findChannel(myChan->getName());
  if(!netChan) { continue; }
   
  if((netChan->getCreationTime() < myChan->getChannelTS()) || (myChan->getChannelTS() == 0)) {
    bot->Notice(theClient, "Mismatch on %d (%s). DB:%d Net:%d",
     myChan->getID(), myChan->getName().c_str(), myChan->getChannelTS(), netChan->getCreationTime());
    myChan->setChannelTS(netChan->getCreationTime());
    myChan->commit();
    mismatchCount++;
    continue;
  }
}

bot->Notice(theClient, "Total mismatches found: %d", mismatchCount);

bot->logAdminMessage("%s (%s) - UPDATEDB - Updated %d channels",
  theClient->getNickName().c_str(), theUser->getUserName().c_str(),
  mismatchCount);

return true ;
}

} // namespace gnuworld.

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include "sqlChannel.h"
#include "Network.h"

const char UPDATEDBCommand_cc_rcsId[] = "$Id: UPDATEDBCommand.cc,v 1.3 2002-07-31 20:35:31 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

bool UPDATEDBCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.UPDATEDB");

StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return true;
	}

 sqlUser* theUser = bot->isAuthed(theClient, true);
 if(!theUser) { return 0; }

 int aLevel = bot->getAdminAccessLevel(theUser);
 if(aLevel < 950) { return 0; }

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

return true ;
}

} // namespace gnuworld.

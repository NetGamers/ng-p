/* UnloadClientTiemrHandler.cc */

#include	"UnloadClientTimerHandler.h"
#include	"server.h"
#include	"ELog.h"

const char UnloadClientTimerHandler_h_rcsId[] = __UNLOADCLIENTTIMERHANDLER_H ;
const char UnloadClientTimerHandler_cc_rcsId[] = "$Id: UnloadClientTimerHandler.cc,v 1.1 2002-01-14 23:20:58 morpheus Exp $" ;
const char ELog_h_rcsId[] = __ELOG_H ;

namespace gnuworld
{

int UnloadClientTimerHandler::OnTimer( timerID, void* )
{
elog	<< "UnloadClientTimerHandler::OnTimer("
	<< moduleName
	<< ")"
	<< endl ;

theServer->DetachClient( moduleName, reason ) ;

delete this ;
return 0 ;
}

} // namespace gnuworld

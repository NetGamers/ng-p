/**
 * UnloadClientTimerHandler.h
 */

#ifndef __UNLOADCLIENTTIMERHANDLER_H
#define __UNLOADCLIENTTIMERHANDLER_H "$Id: UnloadClientTimerHandler.h,v 1.1 2002-01-14 23:19:24 morpheus Exp $"

#include	<string>

#include	"ServerTimerHandlers.h"

namespace gnuworld
{

using std::string ;

class xServer ;

class UnloadClientTimerHandler : public ServerTimerHandler
{

protected:
	string		moduleName ;
	string		reason ;

public:
	UnloadClientTimerHandler( xServer* theServer,
		const string& _moduleName,
		const string& _reason )
	: ServerTimerHandler( theServer, 0),
	  moduleName( _moduleName ),
	  reason( _reason )
	{}

	virtual ~UnloadClientTimerHandler()
	{}

	virtual	int	OnTimer( timerID, void* ) ;

} ;

} // namespace gnuworld

#endif // __UNLOADCLIENTTIMERHANDLER_H

#ifndef __CSERVICE_H
#define __CSERVICE_H "$Id: cservice.h,v 1.8 2002-09-13 21:30:40 jeekay Exp $"

#include	<string>
#include	<vector>
#include	<map>
#include	<ctime>

#include	"client.h"
#include	"iClient.h"
#include	"iServer.h"
#include	"cserviceCommands.h"
#include "libpq++.h"
#include "../mod.nickserv/nickservClass.h"
#include  "cserviceClass.h"

using std::string ;

namespace gnuworld
{

const string escapeSQLChars(const string& theString);

} // namespace gnuworld

#endif // __CSERVICE_H

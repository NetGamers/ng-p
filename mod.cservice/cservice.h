#ifndef __CSERVICE_H
#define __CSERVICE_H "$Id: cservice.h,v 1.7 2002-03-19 19:49:36 jeekay Exp $"

#include	<string>
#include	<vector>
#include	<hash_map.h>
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
using std::vector ;
using std::hash_map ;
using std::map ;

namespace gnuworld
{

const string escapeSQLChars(const string& theString);

} // namespace gnuworld

#endif // __CSERVICE_H

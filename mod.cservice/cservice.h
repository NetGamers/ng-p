#ifndef __CSERVICE_H
#define __CSERVICE_H "$Id: cservice.h,v 1.6 2002-02-18 03:52:42 jeekay Exp $"

#include	<string>
#include	<vector>
#include	<hash_map.h>
#include	<map>
#include	<ctime>

#include	"client.h"
#include	"iClient.h"
#include	"iServer.h"
#include	"cserviceCommands.h"
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

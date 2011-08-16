#ifndef __NICKSERV_H
#define __NICKSERV_H


#include	<string>
#include	<vector>
#include	<map>
#include	<iomanip>

#include	<cstdio>

#include	"client.h"
#include	"iClient.h"
#include	"server.h"
#include        "match.h"
#include	"libpq++.h"
#include	"md5hash.h" 
#include	"nickservCommands.h"

#include	"../mod.cservice/cserviceClass.h"

#include	"nickservClass.h"

namespace gnuworld
{
 
using std::string ;
using std::vector ;


const string escapeSQLChars(const string& theString);

} // namespace gnuworld
 
#endif // __NICKSERV_H

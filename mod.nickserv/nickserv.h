
#ifndef __NICKSERV_H
#define __NICKSERV_H "$Id: nickserv.h,v 1.17 2002-04-03 20:41:52 jeekay Exp $"


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

#include "../mod.cservice/cserviceClass.h"

#include "nickservClass.h"

// The flag in the cservice db
#define NS_F_AUTOKILL 0x08

namespace gnuworld
{
 
using std::string ;
using std::vector ;


const string escapeSQLChars(const string& theString);

} // namespace gnuworld
 
#endif // __NICKSERV_H

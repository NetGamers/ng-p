/*
 * RELEASECommand.cc
 *
 * (c) Copyright 2002 Rasmus Hansen
 * Released under the GNU Public Licence
 *
 * $Id: RELEASECommand.cc,v 1.4 2004-08-25 20:33:02 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"

#include	"sqlUser.h"


namespace gnuworld
{
using std::string ;

void RELEASECommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.RELEASE");

// RELEASE
StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return ;
	}

// Is the user logged in?
sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return ; }
string theUserName = theUser->getUserName();

/* Does the jupe exist? */
gnuworld::nserv::juUser* theJupe = bot->myNickServ->findJupeNick(theUserName);
if(!theJupe)
	{
	bot->Notice(theClient, "There is no active jupe for %s.", theUserName.c_str());
	return ;
	}

/* Jupe exists */
if(bot->myNickServ->removeJupeNick(theUserName, "Jupe Released"))
	{
	bot->Notice(theClient, "Jupe for %s successfully released.", theUserName.c_str());
	return ;
	}
else
	{
	/* We should never hit this code as we know that the nick is juped */
	bot->Notice(theClient, "Unable to remove jupe for %s.", theUserName.c_str());
	return ;
	}

return ;
} // RELEASECommand::Exec

} // namespace gnuworld.

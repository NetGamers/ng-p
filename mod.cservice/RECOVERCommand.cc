/* RECOVERCommand.cc */

#include	<string>

#include	"md5hash.h"
#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"responses.h"
#include	"networkData.h"
#include	"cservice_config.h"
#include	"Network.h"

const char RECOVERCommand_cc_rcsId[] = "$Id: RECOVERCommand.cc,v 1.7 2002-09-13 21:30:39 jeekay Exp $" ;

namespace gnuworld
{

using std::ends;

bool RECOVERCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.RECOVER");

/* RECOVER */
StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return true;
	}

/*
 * Check theClient is already logged in, if not, tell
 * them they should be.
 */
 
 sqlUser* theUser = bot->isAuthed(theClient, true);
 if(!theUser) return false;
 
 iClient* targetClient = Network->findNick(theUser->getUserName());
 if(!targetClient || targetClient == theClient || targetClient->getMode(iClient::MODE_SERVICES))
 	{
	bot->Notice(theClient, "Unable to recover %s.",
		theUser->getUserName().c_str());
	return false;
	}

/* If the user is suspended, they have their RECOVER priviledges revoked */

if (theUser->getFlag(sqlUser::F_GLOBAL_SUSPEND))
        {
        bot->Notice(theClient,
                "You have been suspended by a Cservice Administrator. This command is disabled untill the suspend expires");
        return false;
        }

/*
 * theUser = this user's authed sqlUser
 * targetClient = nick we are about to kill
 */

stringstream s; 
s       << bot->getCharYY()
	<< " D "
	<< targetClient->getCharYYXXX() << " :" << bot->getNickName()
       	<< " [RECOVER] Ghost removed by " << theClient->getNickName()
       	<< ends;
bot->Write(s);

server->PostEvent(gnuworld::EVT_NSKILL, static_cast<void*>(targetClient));
bot->Notice(theClient,"Recover Successful For %s", theUser->getUserName().c_str());
return true;
}

} // namespace gnuworld.


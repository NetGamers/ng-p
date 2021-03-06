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

#include	"sqlUser.h"


namespace gnuworld
{


void RECOVERCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.RECOVER");

/* RECOVER */
/* RECOVER nick user pass */
StringTokenizer st( Message ) ;
if( st.size() != 1  && !(st.size() >= 4) ) {
  Usage(theClient);
  return ;
}

/*
 * Check theClient is already logged in, if not, tell
 * them they should be.
 */
 
iClient* targetClient;
sqlUser* theUser;
 
if(st.size() == 1) {
  theUser = bot->isAuthed(theClient, true);
  if(!theUser) return ;
  
  targetClient = Network->findNick(theUser->getUserName());
  if(!targetClient) {
    bot->Notice(theClient, "Unable to find %s on the network.", 
      theUser->getUserName().c_str());
    return ;
  }
} else if (st.size() >= 4) {
  /* We need to check:
   *   i) Does the user exist?
   *  ii) Is the user/pass combo correct?
   * iii) Is nick logged in as user?
   */
  theUser = bot->getUserRecord(st[2]);
  
  if(!theUser) {
    bot->Notice(theClient, "AUTHENTICATION FAILED as %s", st[2].c_str());
    return ;
  }
  
  if(!bot->isPasswordRight(theUser, st.assemble(3))) {
    bot->Notice(theClient, "AUTHENTICATION FAILED as %s", st[2].c_str());
    return ;
  }
  
  targetClient = Network->findNick(st[1]);
  if(!targetClient) {
    bot->Notice(theClient, "Unable to find %s on the network.", st[1].c_str());
    return ;
  }
  
  sqlUser* recTargetUser = bot->isAuthed(targetClient, false);
  if(recTargetUser != theUser) {
    bot->Notice(theClient, "%s is not logged in as %s",
      targetClient->getNickName().c_str(), theUser->getUserName().c_str());
    return ;
  }
} else {
	Usage(theClient);
	return ;
}
 
if(!targetClient || targetClient == theClient || targetClient->getMode(iClient::MODE_SERVICES)) {
  bot->Notice(theClient, "Unable to recover %s.",
    theUser->getUserName().c_str());
  return ;
}

/* If the user is suspended, they have their RECOVER priviledges revoked */

if (theUser->getFlag(sqlUser::F_GLOBAL_SUSPEND))
        {
        bot->Notice(theClient,
                "You have been suspended by a Cservice Administrator. This command is disabled until the suspend expires");
        return ;
        }

/*
 * theUser = this user's authed sqlUser
 * targetClient = nick we are about to kill
 */

string targetNick = targetClient->getNickName();

stringstream s; 
s       << bot->getCharYY()
	<< " D "
	<< targetClient->getCharYYXXX() << " :" << bot->getNickName()
       	<< " [RECOVER] Ghost removed by " << theClient->getNickName()
        << " (" << theUser->getUserName() << ")"
       	;
bot->Write(s);

server->PostEvent(gnuworld::EVT_NSKILL, static_cast<void*>(targetClient));
bot->Notice(theClient,"Recover Successful For %s", targetNick.c_str());
return ;
}

} // namespace gnuworld.


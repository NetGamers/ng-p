/*
 * CHANINFOCommand.cc
 *
 * 29/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Template.
 *
 * 30/12/2000 - David Henriksen <david@itwebnet.dk>
 * Started and finished the command. Showing all owners by a
 * SQL Query which returns all the level 500s of the channel.
 *
 * Caveats: Need to determine if the query is aimed at a #
 * or a user. :)
 *
 * Command is aliased "INFO".
 *
 * $Id: CHANINFOCommand.cc,v 1.28 2004-05-16 13:08:16 jeekay Exp $
 */

#include  <string>

#include  "StringTokenizer.h"
#include  "ELog.h"
#include  "libpq++.h"

#include  "cservice.h"
#include  "cservice_config.h"
#include  "levels.h"
#include  "responses.h"

const char CHANINFOCommand_cc_rcsId[] = "$Id: CHANINFOCommand.cc,v 1.28 2004-05-16 13:08:16 jeekay Exp $" ;

namespace gnuworld
{

using std::ends;
using std::string ;

static const char* queryHeader = "SELECT channels.name,users.user_name,levels.access,users_lastseen.last_seen FROM levels,channels,users,users_lastseen ";
static const char* queryString = "WHERE levels.channel_id=channels.id AND users.id=users_lastseen.user_id AND levels.access = 500 AND levels.user_id = users.id ";

void CHANINFOCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.INFO");

StringTokenizer st( Message ) ;
if( st.size() < 2 ) {
  Usage(theClient);
  return ;
}

sqlUser* theUser = bot->isAuthed(theClient, false);

int adminAccess = 0; 
if (theUser) adminAccess = bot->getAdminAccessLevel(theUser); 

/*
 *  Are we checking info about a user or a channel?
 */

// Did we find a '#' ?
if( string::npos == st[ 1 ].find_first_of( '#' ) ) {
  // Nope, look by user then.
  sqlUser* targetUser = bot->getUserRecord(st[1]);

  if (!targetUser) {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::not_registered,
        string("The user %s doesn't appear to be registered.")).c_str(),
      st[1].c_str());
    return ;
  } // if(!targetUser)

  /* Keep details private. */

  if (targetUser->getFlag(sqlUser::F_INVIS)) {
    /* If they don't have * access or not opered, deny. */
    if( !((theUser) && bot->getAdminAccessLevel(theUser)) && (theUser != targetUser) && !(theClient->isOper())) {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::no_peeking,
          string("Unable to view user details (Invisible)")));
      return ;
    } // if(!admin or !oper)
  } // if(target is invisible)

  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::info_about,
      string("Information about: %s (%i)")).c_str(),
    targetUser->getUserName().c_str(), targetUser->getID());

  if (targetUser->getID() == 1)
    bot->Notice(theClient," - The one that was, the one that is, the one that will be.");

  /* Loop over all the people we might be logged in as */
  bot->Notice(theClient, "Currently logged on via:");
  int aCount = 0;
  
  for(sqlUser::networkClientListType::iterator ptr = targetUser->networkClientList.begin();
    ptr != targetUser->networkClientList.end(); ++ptr) {
    
    bot->Notice(theClient, " " + (*ptr)->getNickUserHost());
    aCount++;
  }
  
  if(!aCount) { bot->Notice(theClient, " OFFLINE"); }

  if( !targetUser->getUrl().empty() ) {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::url,
        string("URL: %s")).c_str(),
      targetUser->getUrl().c_str());
  }

  if (targetUser->getFlag(sqlUser::F_INVIS)) {
    bot->Notice(theClient, "INVISIBLE is On");
  } else {
    bot->Notice(theClient, "INVISIBLE is Off");
  }

  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::last_seen,
      string("Last Seen: %s")).c_str(),
    bot->prettyDuration(targetUser->getLastSeen()).c_str());

  if (targetUser->getFlag(sqlUser::F_GLOBAL_SUSPEND)) {
    bot->Notice(theClient, "\002** This account has been suspended by a CService Administrator **\002");
  }

  if(adminAccess) {
    /*
     * Show admins some more details about the user.
     */

    unsigned int theTime; 
    string userComments = targetUser->getLastEvent(sqlUser::EV_COMMENT, theTime); 
    

    if(!targetUser->getComment().empty()) {
      bot->Notice(theClient,"\002Admin Comment\002: %s",
        targetUser->getComment().c_str());
    }

    if (targetUser->getFlag(sqlUser::F_GLOBAL_SUSPEND)) {
      /*
       * Perform a lookup to get the last SUSPEND event from the userlog.
       */
      unsigned int theTime;
      string reason = targetUser->getLastEvent(sqlUser::EV_SUSPEND, theTime);

      bot->Notice(theClient, "Account suspended %s ago, Reason: %s", bot->prettyDuration(theTime).c_str(),
        reason.c_str());
      bot->Notice(theClient, "Due to expire: %s", bot->prettyDuration(-(targetUser->getSuspendedExpire()) + 2*(bot->currentTime())).c_str());
    } else {
      /*
       *  Maybe they where unsuspended recently..
       */

      unsigned int theTime;
      string reason = targetUser->getLastEvent(sqlUser::EV_UNSUSPEND, theTime);
      if (!reason.empty()) {
        bot->Notice(theClient, "Account was unsuspended %s ago", bot->prettyDuration(theTime).c_str());
        bot->Notice(theClient, "Unsuspend Reason: %s", reason.c_str());
      }
    }
  } // if(adminAccess)

  /*
   * Run a query to see what channels this user has access on. :)
   * Only show to those with admin access, or the actual user.
   */

  int targetAdmin = bot->getAdminAccessLevel(targetUser);
  if( adminAccess || (theUser == targetUser) || (theClient->isOper() && ! bot->getAdminAccessLevel(targetUser))) {
    bot->Notice(theClient, "EMail: %s",
      targetUser->getEmail().c_str());

    bot->Notice(theClient, "Last Hostmask: %s", 
      targetUser->getLastHostMask().c_str());
    
    bot->Notice(theClient, "Max Logins: %i",
      targetUser->getMaxLogins());
    
    bot->Notice(theClient, "Default memo action: %s",
    	targetUser->getFlag(sqlUser::F_MEMO_REJECT) ? "Reject" : "Accept");

    sqlCommandLevel* theCommandLevel = bot->getLevelRequired("CHGADMIN", "ADMIN");

    if(adminAccess && (!targetAdmin || (adminAccess >= theCommandLevel->getLevel()))) {
      int myQuestion = targetUser->getQuestionID();
      if(myQuestion >= 1 && myQuestion <= 4) {
        string myQuestions[5];
        myQuestions[0] = ""; // Unused
        myQuestions[1] = "What's your mother's maiden name?";
        myQuestions[2] = "What's your dog's (or cat's) name?";
        myQuestions[3] = "What's your father's birthdate?";
        myQuestions[4] = "What's your spouse's name?";
    
        bot->Notice(theClient, "Verification Question: %s", myQuestions[targetUser->getQuestionID()].c_str());
        bot->Notice(theClient, "Verification Answer  : %s", targetUser->getVerificationData().c_str());
      } else {
        bot->Notice(theClient, "This user has a malformed verification question id.");
      }
    } // if(adminAccess && (!targetAdmin || (adminAccess >= theCommandLevel->getLevel())))
    
    string flags;
    if(targetUser->getFlag(sqlUser::F_AUTOKILL))  flags += "AUTOKILL ";
    if(targetUser->getFlag(sqlUser::F_NOPURGE))   flags += "NOPURGE ";
    if(targetUser->getFlag(sqlUser::F_BOT))       flags += "BOT ";
    if(targetUser->getFlag(sqlUser::F_INVIS))     flags += "INVISIBLE ";
    bot->Notice(theClient, "Flags: %s", flags.c_str());

    stringstream channelsQuery;
    string channelList ;

    channelsQuery  << "SELECT channels.name,levels.access FROM levels,channels "
        << "WHERE levels.channel_id = channels.id AND channels.registered_ts <> 0 AND levels.user_id = "
        << targetUser->getID()
        << " ORDER BY levels.access DESC"
        << ends;

    #ifdef LOG_SQL
      elog  << "CHANINFO::sqlQuery> "
        << channelsQuery.str().c_str()
        << endl;
    #endif

    string chanName ;
    string chanAccess ;

    ExecStatusType status =
      bot->SQLDb->Exec(channelsQuery.str().c_str()) ;

    if( PGRES_TUPLES_OK != status ) {
      bot->Notice( theClient,
        "Internal error: SQL failed" ) ;

      elog  << "CHANINFO> SQL Error: "
        << bot->SQLDb->ErrorMessage()
        << endl ;
      return ;
    }

    for(int i = 0; i < bot->SQLDb->Tuples(); i++) {
      chanName = bot->SQLDb->GetValue(i,0);
      chanAccess = bot->SQLDb->GetValue(i,1);
      // 4 for 2 spaces, 2 brackets + comma.
      if ((channelList.size() + chanName.size() + chanAccess.size() +6) >= 450) {
        bot->Notice(theClient,
          bot->getResponse(theUser,
            language::channels,
            string("Channels: %s")).c_str(),
          channelList.c_str());
        channelList.erase( channelList.begin(),
          channelList.end() ) ;
      }

      if (channelList.size() != 0) channelList += ", ";

      channelList += chanName;
      channelList += " (";
      channelList += chanAccess;
      channelList +=  ")";
    } // for()

    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::channels,
        string("Channels: %s")).c_str(),
      channelList.c_str());

  }

  return ;
} // if(is a user)

/*
 * We are INFOing a channel
 */

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if( !theChan ) {
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::chan_not_reg,
      string("The channel %s is not registered")).c_str(),
    st[1].c_str());
  return ;
}

/*
 * Receiving all the level 500's of the channel through a sql query.
 * The description and url, are received from the cache. --Plexus
 */

stringstream theQuery;
theQuery  << queryHeader
    << queryString
    << "AND levels.channel_id = "
    << theChan->getID()
    << ends;

#ifdef LOG_SQL
  elog  << "CHANINFO::sqlQuery> "
    << theQuery.str().c_str()
    << endl;
#endif

const long int registeredTS = theChan->getRegisteredTS();

struct tm timeStruct;
char timeStamp[50];

gmtime_r(&registeredTS, &timeStruct);

strftime(timeStamp, 49, "%d-%m-%Y at %H:%M", &timeStruct);

bot->Notice(theClient, "%s was registered on %s. Currently owned by:",
	theChan->getName().c_str(),
	timeStamp);

ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str()) ;

if( PGRES_TUPLES_OK == status ) {
  for(int i = 0; i < bot->SQLDb->Tuples(); i++) {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::last_seen_info,
        string("%s - last seen: %s ago")).c_str(),
      bot->SQLDb->GetValue(i, 1),
      bot->prettyDuration(atoi(bot->SQLDb->GetValue(i, 3))).c_str());
  } // for()
}

if( !theChan->getDescription().empty() ) {
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::desc,
      string("Desc: %s")).c_str(),
    theChan->getDescription().c_str());
}

if(theChan->getFlag(sqlChannel::F_SUSPEND)) {
  bot->Notice(theClient, "\002** This channel has been suspended by a CService Administrator **\002");
  if(adminAccess) {
    unsigned int theTime;
    string reason = theChan->getLastEvent(sqlChannel::EV_SUSPEND, theTime);
    bot->Notice(theClient, "Channel suspended %s ago, Reason: %s", bot->prettyDuration(theTime).c_str(), reason.c_str());
    bot->Notice(theClient, "Due to expire: %s", bot->prettyDuration(-(theChan->getSuspendExpires()) + 2*(bot->currentTime())).c_str());
  }
} else {
  // Maybe the channel was unsuspended recently?
  if(adminAccess) {
    unsigned int theTime;
    string reason = theChan->getLastEvent(sqlChannel::EV_UNSUSPEND, theTime);
    if(!reason.empty()) {
      bot->Notice(theClient, "Channel was unsuspended %s ago.", bot->prettyDuration(theTime).c_str());
      bot->Notice(theClient, "Unsuspended By: %s", reason.c_str());
    }
  }
}

if( !theChan->getComment().empty() && adminAccess ) {
  if((theUser) && bot->getAdminAccessLevel(theUser)) {
    bot->Notice(theClient, "Comments: %s",
      theChan->getComment().c_str());
  }
}

if( !theChan->getKeywords().empty() ) {
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::keywords,
      string("Keywords: %s")).c_str(),
    theChan->getKeywords().c_str());
}

if( !theChan->getURL().empty() ) {
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::url,
      string("URL: %s")).c_str(),
    theChan->getURL().c_str());
}

return ;
}

} // namespace gnuworld.

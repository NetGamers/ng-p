/*
 * OFFICIALCommand.cc
 *
 * Allows editing and viewing of information related to miscellaneous verifies.
 *
 * 20030113 Jeekay - Initial writing.
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

const char OFFICIALCommand_cc_rcsId[] = "$Id: OFFICIALCommand.cc,v 1.4 2004-05-16 15:20:21 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void OFFICIALCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.OFFICIAL");

/* Possible uses:
 *   i) OFFICIAL LIST                 - list the verifies available
 *  ii) OFFICIAL CHECK <user>         - returns a clients official level and appropriate verify
 * iii) OFFICIAL CLEAR <user>         - sets a clients official level to 0
 *  iv) OFFICIAL SET   <user> <level> - sets a clients official level
 */

sqlUser* theUser=bot->isAuthed(theClient, true);
if(!theUser) return ;

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* officialLevel = bot->getLevelRequired("OFFICIAL", "ADMIN");

if(admLevel < officialLevel->getLevel()) {
  bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
  return ;
}

StringTokenizer st( Message ) ;

/***********************
 * 2   C O M M A N D S *
 ***********************/

if( st.size() < 2 )	{
	Usage(theClient);
	return ;
}

string command = string_upper(st[1]);

if("LIST" == command) {
  bot->Notice(theClient, "Available verifies (%u total):",
    bot->verifies.size());

  cservice::verifiesType::const_iterator itr = bot->verifies.begin();
  for( ; itr != bot->verifies.end(); ++itr) {
    bot->Notice(theClient, "[%03u] %s",
      itr->first,
      itr->second.c_str());
  }
  
  return ;
}

/***********************
 * 3   C O M M A N D S *
 ***********************/

if( st.size() < 3 )	{
	Usage(theClient);
	return ;
}

sqlUser* targetUser = bot->getUserRecord(st[2]);
if(!targetUser) {
  bot->Notice(theClient, "Sorry, %s is not registered with me.", st[2].c_str());
  return ;
}

/*************
 * C H E C K *
 *************/

if("CHECK" == command) {
  
  unsigned int verify = targetUser->getVerify();
  if(0 == verify) {
    bot->Notice(theClient, "%s does not have an official verify assigned.",
      targetUser->getUserName().c_str());
    return ;
  }
  
  bot->Notice(theClient, "%s has verify %u: %s",
    targetUser->getUserName().c_str(),
    verify,
    bot->getVerify(verify).c_str());
  return ;
}


/*************
 * C L E A R *
 *************/

if("CLEAR" == command) {
  bot->Notice(theClient, "%s had verify %u: %s",
    targetUser->getUserName().c_str(),
    targetUser->getVerify(),
    bot->getVerify(targetUser->getVerify()).c_str());

  targetUser->setVerify(0);
  targetUser->commit();

  bot->Notice(theClient, "%s's official verify has been cleared.",
    targetUser->getUserName().c_str());

  return ;
}

/***********************
 * 4   C O M M A N D S *
 ***********************/

if( st.size() < 4 ) {
  Usage(theClient);
  return ;
}

if("SET" == command) {
  unsigned int newVerifyLevel = atoi(st[3].c_str());
  string newVerify = bot->getVerify(newVerifyLevel);
  
  if("" == newVerify) {
    bot->Notice(theClient, "Verify level %u does not to exist.", newVerifyLevel);
    return ;
  }
  
  bot->Notice(theClient, "%s had verify %u: %s",
    targetUser->getUserName().c_str(),
    targetUser->getVerify(),
    bot->getVerify(targetUser->getVerify()).c_str());
  
  targetUser->setVerify(newVerifyLevel);
  targetUser->commit();
  
  bot->Notice(theClient, "%s now has verify %u: %s",
    targetUser->getUserName().c_str(),
    newVerifyLevel,
    bot->getVerify(newVerifyLevel).c_str());
  
  return ;
}

Usage(theClient);
return ;
}

} // namespace gnuworld.

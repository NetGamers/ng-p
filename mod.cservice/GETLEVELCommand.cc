/*
 * GETLEVELCommand.cc - Get the level required for a given command
 *
 * 20021017 - gk@ng - Initial creation
 */

#include <string>

#include "StringTokenizer.h"

#include "cservice.h"
#include "responses.h"

const char GETLEVELCommand_cc_rcsId[] = "$Id: GETLEVELCommand.cc,v 1.2 2004-05-16 13:08:16 jeekay Exp $";

namespace gnuworld {

void GETLEVELCommand::Exec(iClient* theClient, const string& Message) {

bot->incStat("COMMANDS.GETLEVEL");

// GETLEVEL command domain
StringTokenizer st(Message);
if(st.size() < 2) {
  Usage(theClient);
  return ;
}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) {
  return ;
}

int aLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* getlevelLevel = bot->getLevelRequired("GETLEVEL", "ADMIN");
if(aLevel < getlevelLevel->getLevel()) {
  bot->Notice(theClient,
    bot->getResponse(theUser, language::insuf_access).c_str());
  return ;
}

string command = string_upper(st[1]);
string domain = (st.size() == 3) ? (string_upper(st[2])) : ("ADMIN");

if("*" == command) {
  for(cservice::sqlCommandLevelsType::iterator ptr = bot->sqlCommandLevels.begin();
    ptr != bot->sqlCommandLevels.end(); ptr++) {
    
    sqlCommandLevel* theCommandLevel = ptr->second;
    bot->Notice(theClient, "%s:%s - %d",
      theCommandLevel->getDomain().c_str(),
      theCommandLevel->getCommandName().c_str(),
      theCommandLevel->getLevel());
    
  }

  return ;
}

sqlCommandLevel* theLevel = bot->getLevelRequired(command, domain, false);

if(theLevel) {
  bot->Notice(theClient, "%s:%s - %d",
    theLevel->getDomain().c_str(),
    theLevel->getCommandName().c_str(),
    theLevel->getLevel());
} else {
  bot->Notice(theClient, "No such command %s:%s",
    domain.c_str(), command.c_str());
}

return ;

} // bool GETLEVELCommand::Exec(iClient*, const string&)

} // namespace gnuworld

/* GLOBALNOTICECommand.cc
 *
 * 20020209 - GK@panet - Initial creation
 * 20050320 - GK@ng    - Rename to GLOBALNOTICE
 *
 * Sends a notice to all users as 'CService'
 *
 * $Id: GLOBALNOTICECommand.cc,v 1.3 2005-03-31 23:46:37 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"

#include	"cservice.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{

void GLOBALNOTICECommand::Exec( iClient* theClient, const string& Message )
{

/*
 * To use this command, the user must either have level::globalnotice * access
 * or be an IRCop. If the user is neither, just return. No point in telling
 * everyone about the admin commands.
 */

sqlUser* theUser = bot->isAuthed(theClient, false);

if(!theUser && !theClient->isOper()) { return ; }

int admLevel;
if(theUser) admLevel = bot->getAdminAccessLevel(theUser);
else admLevel = 0;

sqlCommandLevel* globalNoticeLevel = bot->getLevelRequired("GLOBALNOTICE", "ADMIN");

if((admLevel < globalNoticeLevel->getLevel()) && !(theClient->isOper())) { return ; }

// Lets actually get on with the command now ;)

bot->incStat("COMMANDS.GLOBALNOTICE");

// GLOBALNOTICE subject Message
StringTokenizer st( Message );
if(st.size() < 4) {
	Usage(theClient);
	return ;
}

string sourceNick, sourceUser;

cservice::globalsType::const_iterator subject = bot->globals.find(string_lower(st[1]));
if( subject == bot->globals.end() ) {
	/* This subject does not exist */
	bot->Notice(theClient, "The global subject %s does not exist.",
		st[1].c_str()
		);
	return ;
}

sourceNick = subject->second;
sourceUser = string_lower(subject->first);

string outString = st.assemble(2);

// Lets introduce the nick

const char* charYY = bot->getCharYY();
bot->Write("%s N %s 1 31337 %s announce.netgamers.org +iodk B]AAAB %s]]] :Global Notice",
	charYY,
	sourceNick.c_str(),
	sourceUser.c_str(),
	charYY
	);
bot->Write("%s]]] O $*.org :%s", charYY, outString.c_str());
bot->Write("%s]]] Q :Done", charYY);

bot->Notice(theClient, "Successfully global noticed.");
return ;

} // GLOBALNOTICECommand::Exec

} // namespace gnuworld

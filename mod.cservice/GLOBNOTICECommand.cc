/* GLOBNOTICECommand.cc
 *
 * 20020209 - GK@panet - Initial creation
 *
 * Sends a notice to all users as 'CService'
 *
 * $Id: GLOBNOTICECommand.cc,v 1.8 2005-03-20 16:12:07 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"

#include	"cservice.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{

void GLOBNOTICECommand::Exec( iClient* theClient, const string& Message )
{

/*
 * To use this command, the user must either have level::globnotice * access
 * or be an IRCop. If the user is neither, just return. No point in telling
 * everyone about the admin commands.
 */

sqlUser* theUser = bot->isAuthed(theClient, false);

if(!theUser && !theClient->isOper()) { return ; }

int admLevel;
if(theUser) admLevel = bot->getAdminAccessLevel(theUser);
else admLevel = 0;

sqlCommandLevel* globalNoticeLevel = bot->getLevelRequired("GLOBNOTICE", "ADMIN");

if((admLevel < globalNoticeLevel->getLevel()) && !(theClient->isOper())) { return ; }

// Lets actually get on with the command now ;)

bot->incStat("COMMANDS.GLOBNOTICE");

// GLOB[AL]NOTICE subject $target.dom Message
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

if(st[2][0] != '$')
	{
	bot->Notice(theClient, "Target should be of the form $*.org");
	return ;
	}

string outString = st.assemble(3);
string outTarget = st[2];

// Lets introduce the nick

const char* charYY = bot->getCharYY();
bot->Write("%s N %s 1 31337 %s announce.netgamers.org +iodk B]AAAB %s]]] :Global Notice",
	charYY,
	sourceNick.c_str(),
	sourceUser.c_str(),
	charYY
	);
bot->Write("%s]]] O %s :%s", charYY, outTarget.c_str(), outString.c_str());
bot->Write("%s]]] Q :Done", charYY);

bot->Notice(theClient, "Successfully global noticed.");
return ;

} // GLOBNOTICECommand::Exec

} // namespace gnuworld

/* GLOBNOTICECommand.cc
 *
 * 20020209 - GK@panet - Initial creation
 *
 * Sends a notice to all users as 'CService'
 *
 * $Id: GLOBNOTICECommand.cc,v 1.7 2004-08-25 20:32:40 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"

#include "cservice.h"

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

// GLOB[AL]NOTICE $target.dom Message
StringTokenizer st( Message );
if(st.size() < 3)
	{
	Usage(theClient);
	return ;
	}

string sourceNick;
if(string_upper(st[0]) == "GLOBALNOTICE")
	{ sourceNick = "Global"; }
else { sourceNick = "CService"; }

if(st[1][0] != '$')
	{
	bot->Notice(theClient, "Target should be of the form $*.org");
	return ;
	}

string outString = st.assemble(2);
string outTarget = st[1];

// Lets introduce the nick

const char* charYY = bot->getCharYY();
bot->Write("%s N %s 1 31337 global notice.ng +iodk B]AAAB %sAAC :Global Notice",
					 charYY, sourceNick.c_str(), charYY);
bot->Write("%sAAC O %s :%s", charYY, outTarget.c_str(), outString.c_str());
bot->Write("%sAAC Q :Done", charYY);

bot->Notice(theClient, "Successfully global noticed.");
return ;

} // GLOBNOTICECommand::Exec

} // namespace gnuworld

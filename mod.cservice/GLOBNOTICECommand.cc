/* GLOBNOTICECommand.cc
 *
 * 20020209 - GK@panet - Initial creation
 *
 * Sends a notice to all users as 'CService'
 *
 * $Id: GLOBNOTICECommand.cc,v 1.2 2002-02-23 00:57:24 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"
#include "cservice.h"
#include "levels.h"

const char GLOBNOTICECommand_cc_rcsId[] = "$Id: GLOBNOTICECommand.cc,v 1.2 2002-02-23 00:57:24 jeekay Exp $";

namespace gnuworld
{

bool GLOBNOTICECommand::Exec( iClient* theClient, const string& Message )
{

/*
 * To use this command, the user must either have level::globnotice * access
 * or be an IRCop. If the user is neither, just return. No point in telling
 * everyone about the admin commands.
 */

sqlUser* theUser = bot->isAuthed(theClient, false);

if(!theUser && !theClient->isOper()) { return false; }

int admLevel;
if(theUser) admLevel = bot->getAdminAccessLevel(theUser);
else admLevel = 0;

if((admLevel < level::globnotice) && !(theClient->isOper())) { return false; }

// Lets actually get on with the command now ;)

bot->incStat("COMMANDS.GLOBNOTICE");

StringTokenizer st( Message );
if(st.size() < 3)
	{
	Usage(theClient);
	return true;
	}

if(st[1][0] != '$')
	{
	bot->Notice(theClient, "Target should be of the form $*.com");
	return false;
	}

string outString = st.assemble(2);
string outTarget = st[1];

// Lets introduce the nick

const char* charYY = bot->getCharYY();
bot->Write("%s N CService 1 31337 global notice.pa +iodk B]AAAB %sAAC :CService Global Notice",
					 charYY, charYY);
bot->Write("%sAAC O %s :%s", charYY, outTarget.c_str(), outString.c_str());
bot->Write("%sAAC Q :Done", charYY);

bot->Notice(theClient, "Successfully global noticed.");
return true;

} // GLOBNOTICECommand::Exec

} // namespace gnuworld

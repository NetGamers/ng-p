/* REHASHCommand.cc */

#include	<string>

#include	"ELog.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

const char REHASHCommand_cc_rcsId[] = "$Id: REHASHCommand.cc,v 1.4 2004-05-16 13:08:17 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

void REHASHCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.REHASH");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser) return ;

int level = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* rehashCommandLevel = bot->getLevelRequired("REHASH", "ADMIN");

if (level < rehashCommandLevel->getLevel())
{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("You have insufficient access to perform that command.")));
	return ;
}

string option = string_upper(st[1]);

if (option == "TRANSLATIONS")
	{
		bot->languageTable.clear();
		bot->translationTable.clear();
		bot->loadTranslationTable();
		bot->Notice(theClient, "Done. %i entries in language table.",
			bot->translationTable.size());
	}

if (option == "HELP")
	{
		bot->helpTable.clear();
		bot->loadHelpTable();
		bot->Notice(theClient, "Done. %i entries in help table.",
			bot->helpTable.size());
	}

if ("COMMANDS" == option) {
  int noLoaded = bot->preloadCommandLevelsCache();
  bot->Notice(theClient, "Successfully rehashed %d command levels.",
    noLoaded);
}

if ("OFFICIAL" == option) {
  unsigned int noLoaded = bot->preloadVerifiesCache();
  bot->Notice(theClient, "Successfully rehashed %u verifies.",
    noLoaded);
}

return ;
}

} // namespace gnuworld.

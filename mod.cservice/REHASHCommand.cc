/* REHASHCommand.cc */

#include	<string>

#include	"ELog.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"responses.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"


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

if ("COMMANDS" == option) {
	int noLoaded = bot->preloadCommandLevelsCache();
	bot->Notice(theClient, "Successfully rehashed %d command levels.",
		noLoaded);
}

if ("GLOBAL" == option) {
	int noLoaded = bot->preloadGlobalsCache();
	bot->Notice(theClient, "Successfully rehashed %d global subjects.",
		noLoaded);
}

if ("HELP" == option) {
	bot->helpTable.clear();
	bot->loadHelpTable();
	bot->Notice(theClient, "Done. %i entries in help table.",
		bot->helpTable.size());
}

if ("OFFICIAL" == option) {
  unsigned int noLoaded = bot->preloadVerifiesCache();
  bot->Notice(theClient, "Successfully rehashed %u verifies.",
    noLoaded);
}

if ("TRANSLATIONS" == option) {
	bot->languageTable.clear();
	bot->translationTable.clear();
	bot->loadTranslationTable();
	bot->Notice(theClient, "Done. %i entries in language table.",
		bot->translationTable.size());
}

return ;
}

} // namespace gnuworld.

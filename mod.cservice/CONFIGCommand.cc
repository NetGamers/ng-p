/*
 * CONFIGCommand.cc
 *
 * 2003-04-29 GK@NG Initial creation
 */

#include	<string>

#include	"ELog.h"
#include	"StringTokenizer.h"
#include	"cservice.h"

#include	"sqlCommandLevel.h"
#include	"sqlUser.h"

namespace gnuworld
{

using std::string ;

void CONFIGCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.CONFIG");

/*
 * Possible uses:
 *  CONFIG VIEW
 *  CONFIG SET <VAR> <VALUE>
 */

StringTokenizer st( Message ) ;
if( st.size() < 2 ) {
	Usage(theClient);
	return ;
}

sqlUser* theUser = bot->isAuthed(theClient, false);

sqlCommandLevel* theLevel = bot->getLevelRequired("CONFIG", "ADMIN");

if(!theUser || bot->getAdminAccessLevel(theUser) < theLevel->getLevel()) {
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
	return ;
}

string command = string_upper(st[1]);

if("VIEW" == command) {
	bot->Notice(theClient, "%30s - Value", "Name");

	cservice::configListType::const_iterator ptr;

	for(ptr = bot->configList.begin() ; ptr != bot->configList.end() ; ++ptr) {
		bot->Notice(theClient, "%30s - %s",
			ptr->first.c_str(),
			ptr->second.c_str());
	}

	return ;
}

if( st.size() < 4 ) {
	Usage(theClient);
	return ;
}

if("SET" == command) {
	string theName = string_upper(st[2]);
	if(bot->getConfigItem(theName).empty()) {
		bot->Notice(theClient, "Config item %s does not exist.",
			theName.c_str());
		return ;
	}
	
	stringstream theQuery;
	theQuery	<< "UPDATE config SET contents = '"
			<< escapeSQLChars(st.assemble(3))
			<< "' WHERE name = '"
			<< theName
			<< "'";
	
#ifdef LOG_SQL
	elog	<< "CONFIGCommand::SET> Executing: "
		<< theQuery
		<< endl;
#endif

	ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str());
	
	if(PGRES_COMMAND_OK != status) {
		bot->Notice(theClient, "Error updating database.");
		return ;
	}
	
	bot->preloadConfigCache();
	
	bot->Notice(theClient, "Set %s to %s",
		theName.c_str(),
		st.assemble(3).c_str());

	return ;
}

return ;
} // bool CONFIGCommand::Exec(iClient*, const string&)

} // namespace gnuworld.


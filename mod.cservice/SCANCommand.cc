/* SCANCommand.cc - Allow admins to scan for hostmasks/emails
 *
 * (c) Copyright 2002 Rasmus Hansen (GK@panet)
 *
 * Distributed under the GNU Public Licence
 *
 * $Id: SCANCommand.cc,v 1.3 2002-03-28 21:59:25 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include  "levels.h"

const char SCANCommand_cc_rcsId[] = "$Id" ;

namespace gnuworld
{
using std::string ;

bool SCANCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.SCAN");

// SCAN [email|hostmask|nick] string
StringTokenizer st( Message ) ;
if( st.size() != 3 )
	{
	Usage(theClient);
	return true;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return false; }

int aLevel = bot->getAdminAccessLevel(theUser);

if(aLevel < level::scan)
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
	return false;
	}

string option = string_upper(st[1]);
string search = string_lower(st[2]);

if("EMAIL" == option)
	{
	strstream emailQuery;
	emailQuery << "SELECT user_name,email FROM users WHERE"
		<< " lower(email) LIKE '%" << escapeSQLChars(search) << "%'"
		<< " LIMIT 10"
		<< ends;
#ifdef LOG_SQL
	elog << "SCAN:SQL> " << emailQuery.str() << endl;
#endif
	ExecStatusType status = bot->SQLDb->Exec(emailQuery.str());
	delete[] emailQuery.str();

	if(PGRES_TUPLES_OK != status)
		{
		bot->Notice(theClient, "Internal database error.");
		elog << "SCAN:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}
	
	for(int i = 0; i < bot->SQLDb->Tuples(); i++)
		{
		string resUser = bot->SQLDb->GetValue(i, 0);
		string resEmail = bot->SQLDb->GetValue(i, 1);
		bot->Notice(theClient, "User: %s; Email: %s",
			resUser.c_str(), resEmail.c_str());
		}
	
	bot->Notice(theClient, "End of Scan");
	bot->logAdminMessage("%s (%s) - SCAN - EMAIL - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		search.c_str());

	return true;
	}

if("HOSTMASK" == option)
	{
	strstream hostmaskQuery;
	hostmaskQuery << "SELECT user_name,last_hostmask FROM users,users_lastseen"
		<< " WHERE id = user_id AND lower(last_hostmask) LIKE '%"
		<< escapeSQLChars(search) << "%'"
		<< " LIMIT 10"
		<< ends;
#ifdef LOG_SQL
	elog << "SCAN:SQL> " << hostmaskQuery.str() << endl;
#endif
	ExecStatusType status = bot->SQLDb->Exec(hostmaskQuery.str());
	delete[] hostmaskQuery.str();
	
	if(PGRES_TUPLES_OK != status)
		{
		bot->Notice(theClient, "Internal database error.");
		elog << "SCAN:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}
	
	for(int i = 0; i < bot->SQLDb->Tuples(); i++)
		{
		string resUser = bot->SQLDb->GetValue(i, 0);
		string resHostmask = bot->SQLDb->GetValue(i, 1);
		bot->Notice(theClient, "User: %s; Hostmask: %s",
			resUser.c_str(), resHostmask.c_str());
		}
	
	bot->Notice(theClient, "End of Scan");
	bot->logAdminMessage("%s (%s) - SCAN - HOSTMASK - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		search.c_str());
	
	return true;
	}

if("NICK" == option)
	{
	strstream nickQuery;
	nickQuery << "SELECT user_name FROM users WHERE"
		<< " lower(user_name) LIKE '%" << escapeSQLChars(search) << "%'"
		<< " LIMIT 10"
		<< ends;
#ifdef LOG_SQL
	elog << "SCAN:SQL> " << nickQuery.str() << endl;
#endif
	ExecStatusType status = bot->SQLDb->Exec(nickQuery.str());
	
	if(PGRES_TUPLES_OK != status)
		{
		bot->Notice(theClient, "Internal database error.");
		elog << "SCAN:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
		return false;
		}
	
	for(int i = 0; i < bot->SQLDb->Tuples(); i++)
		{
		string resUser = bot->SQLDb->GetValue(i, 0);
		bot->Notice(theClient, "User: %s", resUser.c_str());
		}
	
	bot->Notice(theClient, "End of Scan");
	bot->logAdminMessage("%s (%s) - SCAN - NICK - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		search.c_str());
	
	return true;
	}

return true ;
} // SCANCommand::Exec

} // namespace gnuworld.

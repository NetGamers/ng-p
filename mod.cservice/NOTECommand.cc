/* NOTECommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"responses.h"

#define LOG_SQL

const char NOTECommand_cc_rcsId[] = "$Id: NOTECommand.cc,v 1.6 2002-01-21 14:53:20 morpheus Exp $" ;

namespace gnuworld
{
using std::string ;

bool NOTECommand::Exec(iClient* theClient, const string& Message)
{
	bot->incStat("COMMANDS.NOTE");

	StringTokenizer st(Message) ;
	if (st.size() < 2) {
		Usage(theClient);
		return true;
	}

	/*
	 *  Fetch the sqlUser record attached to this client. If there isn't one,
	 *  they aren't logged in - tell them they should be.
	 */
	sqlUser* theUser = bot->isAuthed(theClient, true);
	if (!theUser)
	        return false;
		
	string cmd = string_upper(st[1]);
	if (cmd == "SEND") {
		if (st.size() < 4) {
			Usage(theClient);
			return true;
		}

		sqlUser* targetUser = bot->getUserRecord(st[2]);
		if (!targetUser) {
			bot->Notice(theClient, bot->getResponse(theUser,
				language::not_registered).c_str(), st[2].c_str());
			return false;
		}
		
		if(targetUser->getFlag(sqlUser::F_NOTE))
        	{
                	bot->Notice(theClient, "%s disabled NOTE",targetUser->getUserName().c_str());
                	return false;
        	}
		strstream thecount;
		thecount	<< "SELECT COUNT(*) as count "
				<< "FROM memo where to_id="
				<< targetUser->getID()
				<< ends;
		
		ExecStatusType status = bot->SQLDb->Exec( thecount.str() );
		delete[] thecount.str() ;

		if( PGRES_TUPLES_OK != status )
                {
                elog    << "SUPPORTCommand> SQL Error: "
                                << bot->SQLDb->ErrorMessage()
                                << endl ;
                return false ;
                }
		
		int count = atoi(bot->SQLDb->GetValue(0,0));
		
		if ( count >= 15 )
		{
			bot->Notice(theClient, "The receivers notebox is full. A user is allowed to have max 15 notes.");
		} else {

		strstream theQuery;
		theQuery	<< "INSERT INTO memo "
				<< "(from_id, to_id, content, ts) "
				<< "VALUES ("
				<< theUser->getID() << ", "
				<< targetUser->getID() << ", "
				<< "'" << escapeSQLChars(st.assemble(3)) << "', "
				<< "now()::abstime::int4)"
				<< ends;
#ifdef LOG_SQL
		elog	<< "NOTE:SEND::sqlQuery> "
			<< theQuery.str()
			<< endl;
#endif
		ExecStatusType status = bot->SQLDb->Exec(theQuery.str());
		delete[] theQuery.str();
		if (PGRES_COMMAND_OK != status) {
			bot->dbErrorMessage(theClient);
			return false;
		}
		// TODO: i18n
		bot->Notice(theClient, "Note sent to %s", targetUser->getUserName().c_str());

		iClient* targetClient = targetUser->isAuthed();
		if (targetClient)
			bot->Notice(targetClient, "You received a note from %s (type \002/msg %s note read\002 to see it)", 
			theUser->getUserName().c_str(), bot->getNickName().c_str());
		}
	} else if (cmd == "READ") {
		strstream theQuery;
		theQuery	<< "SELECT memo.id, memo.ts, memo.content, users.user_name "
				<< "FROM memo, users "
				<< "WHERE users.id = memo.from_id "
				<< "AND memo.to_id = " << theUser->getID()
				<< " ORDER BY ts DESC LIMIT 15"
				<< ends;
#ifdef LOG_SQL
		elog	<< "NOTE:READ::sqlQuery> "
			<< theQuery.str()
			<< endl;
#endif
		ExecStatusType status = bot->SQLDb->Exec(theQuery.str());
		delete[] theQuery.str();
		if (PGRES_TUPLES_OK != status) {
			bot->dbErrorMessage(theClient);
			return false;
		}
		if (!bot->SQLDb->Tuples()) {
			// TODO: i18n
			bot->Notice(theClient, "You have no notes.");
			return false;
		}
		for (int i = 0; i < bot->SQLDb->Tuples(); i++) {
			if (i > 12) {
				// TODO: i18n
				bot->Notice(theClient, "You have more than 12 notes. Please delete some of them and try again.");
				break;
			}
			bot->Notice(theClient, "MEMO #%d", atoi(bot->SQLDb->GetValue(i, 0)));
			bot->Notice(theClient, "FROM: %s", bot->SQLDb->GetValue(i, 3));
			bot->Notice(theClient, "SENT: %s ago", bot->prettyDuration(atoi(bot->SQLDb->GetValue(i, 1))).c_str());
			bot->Notice(theClient, "TEXT: %s", bot->SQLDb->GetValue(i, 2));
		}
		bot->Notice(theClient, "End of notes.");
	} else if (cmd == "ERASE") {
		if (st.size() < 3) {
			Usage(theClient);
			return false;
		}
		strstream theQuery;
		theQuery	<< "DELETE FROM memo "
				<< "WHERE to_id = " << theUser->getID();
		if (string_upper(st[2]) != "ALL")
			theQuery << " AND id=" << atoi(st[2].c_str());
		theQuery	<< ends;
#ifdef LOG_SQL
		elog	<< "NOTE:ERASE::sqlQuery> "
			<< theQuery.str()
			<< endl;
#endif
		ExecStatusType status = bot->SQLDb->Exec(theQuery.str());
		delete[] theQuery.str();
		if (PGRES_COMMAND_OK != status) {
			bot->dbErrorMessage(theClient);
			return false;
		}
		// TODO: i18n
		bot->Notice(theClient, "Deleted %d memo(s).", bot->SQLDb->CmdTuples());
	} else if (cmd == "CONFIG") {
		string option = string_upper(st[2]);
		string value = string_upper(st[3]);
		if (option == "ALLOW") {
			if ( value == "ON") {

			} else if (value == "OFF") {

			} else {
				// Show on/off thingie here
			}
		} else if (option == "BLOCK") {
			if ( value == "ON") {
                        
                        } else if (value == "OFF") {
                        
                        } else {
                                // Show on/off thingie here
                        }
		} else {
			bot->Notice(theClient, "No such config option");
			return true;
		}
	} else {
		Usage(theClient);
		return true;
	}

	return true ;
}

} // namespace gnuworld

/* NOTECommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"responses.h"

#define LOG_SQL

const char NOTECommand_cc_rcsId[] = "$Id: NOTECommand.cc,v 1.14 2004-05-01 15:31:43 jeekay Exp $" ;

namespace gnuworld
{

using std::ends ;
using std::string ;

bool NOTECommand::Exec(iClient* theClient, const string& Message)
{
bot->incStat("COMMANDS.NOTE");

StringTokenizer st(Message) ;
if (st.size() < 2)
	{
	Usage(theClient);
	return true;
	}

/*
 *  Fetch the sqlUser record attached to this client. If there isn't one,
 *  they aren't logged in - tell them they should be.
 */
sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser) return false;
		
string cmd = string_upper(st[1]) ;
	
/*************
 * SEND NOTE *
 *************/

if (cmd == "SEND")
	{
	if (st.size() < 4)
		{
		Usage(theClient);
		return true;
		}

	sqlUser* targetUser = bot->getUserRecord(st[2]);
	if (!targetUser)
		{
		bot->Notice(theClient, bot->getResponse(theUser,
			language::not_registered).c_str(), st[2].c_str());
		return false;
		} // No such target user
		
	/* First things first - is this user allowing incoming notes? */
	std::vector<unsigned int> userIdList;
	
	stringstream getUserIdList;
	getUserIdList	<< "SELECT user_from_id FROM note_allow WHERE"
			<< " user_id = " << targetUser->getID()
			;
	ExecStatusType dbStatus = bot->SQLDb->Exec(getUserIdList.str().c_str());
	
	if( dbStatus != PGRES_TUPLES_OK ) {
		/* Bizarre DB error */
		elog	<< "NOTECommand> SQL Error: "
			<< bot->SQLDb->ErrorMessage()
			<< endl;
		bot->Notice(theClient, "Internal database error whilst sending note.");
		return false;
	} // Database error of some sort

	for( int row = 0 ; row < bot->SQLDb->Tuples() ; ++row ) {
		unsigned int tempId = atoi(bot->SQLDb->GetValue(row, 0));
		userIdList.push_back(tempId);
	}
	
	std::vector<unsigned int>::const_iterator itr;
	itr = find(userIdList.begin(), userIdList.end(), theUser->getID());
	
	/* Four possibilities, two of which lead to rejection:
	 *   i) REJECT: Default ACCEPT, on list
	 *  ii) REJECT: Default REJECT, not on list
	 * iii) ACCEPT: Default ACCEPT, not on list
	 *  iv) ACCEPT: Default REJECT, on list
	 */
	bool inList = ( itr != userIdList.end() );

	if( ( targetUser->getFlag(sqlUser::F_MEMO_REJECT) && !inList ) ||
	    ( !targetUser->getFlag(sqlUser::F_MEMO_REJECT) && inList ) )
		{
		/* This note sending is not allowed */
		bot->Notice(theClient, "Sorry, that user is not accepting notes from you.");
		return true;
		}
		
			
		/* The target user is allowing incoming notes from this user
		 * Now we run the other checks before sending the note
		 */
		
		stringstream thecount;
		thecount	<< "SELECT COUNT(*) as count "
				<< "FROM memo where to_id="
				<< targetUser->getID()
				<< ends;
		
		ExecStatusType status = bot->SQLDb->Exec( thecount.str().c_str() );

		if( PGRES_TUPLES_OK != status )
			{
			elog    << "NOTECommand> SQL Error: "
				<< bot->SQLDb->ErrorMessage()
				<< endl ;
			return false ;
			} // status != PGRES_TUPLES_OK
		
		int count = atoi(bot->SQLDb->GetValue(0,0));
		
		if ( count >= 15 )
			{
			bot->Notice(theClient, "The receivers notebox is full. A user is allowed to have max 15 notes.");
			} // count >= 15
		else
			{ // count < 15

			stringstream theQuery;
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
				<< theQuery.str().c_str()
				<< endl;
#endif
			ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str());
			if (PGRES_COMMAND_OK != status)
				{
				bot->dbErrorMessage(theClient);
				return false;
				} // status != PGRES_COMMAND_OK

			bot->Notice(theClient, "Note sent to %s", targetUser->getUserName().c_str());

			if (targetUser->isAuthed())
				{
				bot->noticeAllAuthedClients(targetUser, "You received a note from %s (type \002/msg %s note read\002 to see it)", 
					theUser->getUserName().c_str(), bot->getNickName().c_str());
				} // targetClient exists
		}// count < 15
	return true;
	} // if(cmd == "SEND"); 

/*************
 * READ NOTE *
 *************/

if (cmd == "READ")
	{
	stringstream theQuery;
	theQuery	<< "SELECT memo.id, memo.ts, memo.content, users.user_name "
		<< "FROM memo, users "
		<< "WHERE users.id = memo.from_id "
		<< "AND memo.to_id = " << theUser->getID()
		<< " ORDER BY ts DESC LIMIT 15"
		<< ends;
#ifdef LOG_SQL
	elog	<< "NOTE:READ::sqlQuery> "
		<< theQuery.str().c_str()
		<< endl;
#endif
	ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str());
	if (PGRES_TUPLES_OK != status)
		{
		bot->dbErrorMessage(theClient);
		return false;
		}
	if (!bot->SQLDb->Tuples())
		{
		bot->Notice(theClient, "You have no notes.");
		return false;
		}

	for (int i = 0; i < bot->SQLDb->Tuples(); i++)
		{
		if (i > 12)
			{
			bot->Notice(theClient, "You have more than 12 notes. Please delete some of them and try again.");
			break;
			} // if( i > 12)
		bot->Notice(theClient, "NOTE #%d", atoi(bot->SQLDb->GetValue(i, 0)));
		bot->Notice(theClient, "FROM: %s", bot->SQLDb->GetValue(i, 3));
		bot->Notice(theClient, "SENT: %s ago", bot->prettyDuration(atoi(bot->SQLDb->GetValue(i, 1))).c_str());
		bot->Notice(theClient, "TEXT: %s", bot->SQLDb->GetValue(i, 2));
		}
	bot->Notice(theClient, "End of notes.");
	return true;
	}

/**************
 * ERASE NOTE *
 **************/

if (cmd == "ERASE")
	{
	if (st.size() < 3)
		{
		Usage(theClient);
		return false;
		}
		
	stringstream theQuery;
	theQuery	<< "DELETE FROM memo "
		<< "WHERE to_id = " << theUser->getID();
		
	if (string_upper(st[2]) != "ALL")
		theQuery << " AND id=" << atoi(st[2].c_str());
	
	theQuery	<< ends;
	
#ifdef LOG_SQL
	elog	<< "NOTE:ERASE::sqlQuery> "
		<< theQuery.str().c_str()
		<< endl;
#endif

	ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str());
	if (PGRES_COMMAND_OK != status)
		{
		bot->dbErrorMessage(theClient);
		return false;
		}
		
	bot->Notice(theClient, "Deleted %d note(s).", bot->SQLDb->CmdTuples());
	return true;
	}

/**************
 * ALLOW NOTE *
 **************/

if (cmd == "ALLOW")
	{
	/* NOTE ALLOW CLEAR or
	 * NOTE ALLOW ADD nick or
	 * NOTE ALLOW DEL nick
	 * NOTE ALLOW LIST
	 */

	string option = ( st.size() < 3 ) ? "LIST" : string_upper(st[2]);
	
	if(option == "CLEAR")
		{
		/* User wants his allow list cleared */
		stringstream clearQuery;
		clearQuery << "SELECT COUNT(*) as count FROM note_allow WHERE"
			<< " user_id = " << theUser->getID() << ends;
		ExecStatusType statusClearQuery = bot->SQLDb->Exec(clearQuery.str().c_str());
#ifdef LOG_SQL
		elog << "NOTE:CLEAR:SQL> "
			<< clearQuery.str().c_str() << endl;
#endif
		
		if(PGRES_TUPLES_OK != statusClearQuery)
			{
			bot->Notice(theClient, "Internal database error clearing list.");
			elog << "NOTECommand::CLEAR> Error: "
				<< bot->SQLDb->ErrorMessage();
			return true;
			}
		
		int clearCount = atoi(bot->SQLDb->GetValue(0,0));
		bot->Notice(theClient, "Clearing %d users from your allow list.", clearCount);
		
		stringstream clearDelete;
		clearDelete << "DELETE FROM note_allow WHERE"
			" user_id = " << theUser->getID() << ends;
		ExecStatusType statusClearDelete = bot->SQLDb->Exec(clearDelete.str().c_str());
#ifdef LOG_SQL
		elog << "NOTE:CLEAR:SQL> "
			<< clearDelete.str().c_str() << endl;
#endif
		
		if(PGRES_COMMAND_OK != statusClearDelete)
			{
			bot->Notice(theClient, "Internal database error clearing list.");
			elog << "NOTE:CLEAR> Error: "
				<< bot->SQLDb->ErrorMessage();
			return true;
			}
		
		bot->Notice(theClient, "Note allow list successfully cleared.");
		return true;
		}
	
	if("LIST" == option)
		{
		stringstream listSelect;
		listSelect << "SELECT user_name FROM users,note_allow WHERE users.id=user_from_id"
			<< " AND user_id = " << theUser->getID()
			<< " ORDER BY user_name"<< ends;
#ifdef LOG_SQL
		elog << "NOTE:LIST:SQL> " << listSelect.str().c_str() << endl;
#endif
		ExecStatusType statusListSelect = bot->SQLDb->Exec(listSelect.str().c_str());
		
		if(PGRES_TUPLES_OK != statusListSelect)
			{
			bot->Notice(theClient, "Internal database error listing allowed users.");
			elog << "NOTE:LIST:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
			return false;
			}
		
		string namesList;
		int maxCount = bot->SQLDb->Tuples();
		for(int i = 0; i < maxCount; i++)
			{
			string allowUser = bot->SQLDb->GetValue(i, 0);
			namesList += allowUser;
			if((i+1) != maxCount) namesList += ", ";
			}
			
		bot->Notice(theClient, "Notes from users on this list will be %s.",
			theUser->getFlag(sqlUser::F_MEMO_REJECT) ? "ACCEPTED" : "REJECTED"
			);
		bot->Notice(theClient, "User list: %s", namesList.c_str());
		return true;
		}
	
	if(st.size() < 4)
		{
		Usage(theClient);
		return false;
		}
	
	string target = st[3];
	sqlUser* targetUser = bot->getUserRecord(target);
	if(!targetUser)
		{
		bot->Notice(theClient, "%s is not registered with me.", target.c_str());
		return false;
		}
	
	if("ADD" == option)
		{
		/* First, does this pair already exist? */
		int userID, targetID;
		userID = theUser->getID();
		targetID = targetUser->getID();
		
		stringstream addSelect;
		addSelect << "SELECT COUNT(*) AS count FROM note_allow WHERE"
			<< " user_id = " << userID << " AND user_from_id = " << targetID
			<< ends;
#ifdef LOG_SQL
		elog << "NOTE:ADD:SQL> " << addSelect.str().c_str() << endl;
#endif
		ExecStatusType statusAddSelect = bot->SQLDb->Exec(addSelect.str().c_str());
		
		if(PGRES_TUPLES_OK != statusAddSelect)
			{
			bot->Notice(theClient, "Internal database error adding user.");
			elog << "NOTE:ADD:SQLError> "
				<< bot->SQLDb->ErrorMessage()
				<< endl;
			return false;
			}
		
		int isExists = atoi(bot->SQLDb->GetValue(0,0));
		if(isExists)
			{
			bot->Notice(theClient, "%s is already in your allowed list.",
				targetUser->getUserName().c_str());
			return false;
			}
		
		stringstream addInsert;
		addInsert << "INSERT INTO note_allow (user_id,user_from_id,last_updated)"
			<< " VALUES (" << userID << "," << targetID
			<< ",now()::abstime::int4)" << ends;
#ifdef LOG_SQL
		elog << "NOTE:ADD:SQL> " << addInsert.str().c_str() << endl;
#endif
		ExecStatusType statusAddInsert = bot->SQLDb->Exec(addInsert.str().c_str());
		
		if(PGRES_COMMAND_OK != statusAddInsert)
			{
			bot->Notice(theClient, "Internal database error adding user.");
			elog << "NOTE:ADD:SQLError> "
				<< bot->SQLDb->ErrorMessage()
				<< endl;
			return false;
			}
		
		bot->Notice(theClient, "%s successfully added to your list.",
			targetUser->getUserName().c_str());
		return true;
		} // if("ADD" == option)
	
	if("REM" == option)
		{
		/* Does this pair exist? */
		int userID, targetID;
		userID = theUser->getID();
		targetID = targetUser->getID();
		
		stringstream remSelect;
		remSelect << "SELECT COUNT(*) AS count FROM note_allow WHERE"
			<< " user_id = " << userID << " AND user_from_id = " << targetID
			<< ends;
#ifdef LOG_SQL
		elog << "NOTE:REM:SQL> " << remSelect.str().c_str() << endl;
#endif
		ExecStatusType statusRemSelect = bot->SQLDb->Exec(remSelect.str().c_str());
		
		if(PGRES_TUPLES_OK != statusRemSelect)
			{
			bot->Notice(theClient, "Internal database error removing from your allow list.");
			elog << "NOTE:LIST:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
			return false;
			}
		
		int isExists = atoi(bot->SQLDb->GetValue(0,0));
		if(!isExists)
			{
			bot->Notice(theClient, "%s is not on your list.",
				targetUser->getUserName().c_str());
			return false;
			}
		
		stringstream remDelete;
		remDelete << "DELETE FROM note_allow WHERE"
			<< " user_id = " << userID << " AND"
			<< " user_from_id = " << targetID
			<< ends;
#ifdef LOG_SQL
		elog << "NOTE:REM:SQL> " << remDelete.str().c_str() << endl;
#endif
		ExecStatusType statusRemDelete = bot->SQLDb->Exec(remDelete.str().c_str());
		if(PGRES_COMMAND_OK != statusRemDelete)
			{
			bot->Notice(theClient, "Internal database error removing from your allow list.");
			elog << "NOTE:REM:SQLError> " << bot->SQLDb->ErrorMessage() << endl;
			return false;
			}
		
		bot->Notice(theClient, "%s successfully removed from your list.",
			targetUser->getUserName().c_str());
		return true;
		}
	return true;
	}

Usage(theClient);
return true;
}

} // namespace gnuworld

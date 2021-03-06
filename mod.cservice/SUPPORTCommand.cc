/* SUPPORTCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"

#include	"sqlChannel.h"
#include	"sqlUser.h"


namespace gnuworld
{

using std::string ;

void SUPPORTCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.SUPPORT");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if (!theUser)
	{
	return ;
	}

string support = st[2];

/*
 * First, check they can type the command correctly.
 */

char supportChar = '?';

if (string_lower(support) == "no")
{
	supportChar = 'N';
} else if (string_lower(support) == "yes")
{
	supportChar = 'Y';
} else
{
	bot->Notice(theClient, "Valid SUPPORT choices are YES and NO.");
	return ;
}

/*
 * Next, check to see if this channel is actually pending.
 */

string channelName = st[1];

stringstream theQuery;

theQuery	<< "SELECT channels.id FROM pending,channels"
			<< " WHERE lower(channels.name) = '"
			<< escapeSQLChars(string_lower(channelName))
			<< "'"
			<< " AND channels.id = pending.channel_id"
			<< " AND pending.status = 0"
			;

#ifdef LOG_SQL
elog	<< "SUPPORTCommand::sqlQuery> "
		<< theQuery.str().c_str()
		<< endl;
#endif

ExecStatusType status = bot->SQLDb->Exec( theQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "SUPPORTCommand> SQL Error: "
			<< bot->SQLDb->ErrorMessage()
			<< endl ;
	return ;
	}

if (bot->SQLDb->Tuples() <= 0)
{
	bot->Notice(theClient,
		"The channel %s doesn't appear to have a pending application. Please ensure you have spelt the name correctly.",
			channelName.c_str());
	return ;
}

/*
 * So it is, lets check they haven't already voiced their support about this channel.
 */

unsigned int channel_id = atoi(bot->SQLDb->GetValue(0, 0));

stringstream supQuery;
supQuery 	<< "SELECT support FROM supporters"
		<< " WHERE channel_id = "
		<< channel_id
		<< " AND user_id = "
		<< theUser->getID()
		;

#ifdef LOG_SQL
elog	<< "SUPPORTCommand::sqlQuery> "
		<< supQuery.str().c_str()
		<< endl;
#endif

status = bot->SQLDb->Exec( supQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "SUPPORTCommand> SQL Error: "
			<< bot->SQLDb->ErrorMessage()
			<< endl ;
	return ;
	}

if (bot->SQLDb->Tuples() <= 0)
{
	bot->Notice(theClient,
		"You don't appear to be listed as a supporter in %s, please ensure you have spelt the channel name correctly.",
			channelName.c_str());
	return ;
}

string currentSupport = bot->SQLDb->GetValue(0, 0);

if ((currentSupport == "Y") || (currentSupport == "N"))
{
	bot->Notice(theClient,
		"You have already made your decision on supporting %s, you cannot change it.",
			channelName.c_str());
		return ;
}

/*
 * Save the changes.
 */

stringstream updateQuery;
updateQuery	<< "UPDATE supporters SET support = '"
		<< supportChar
		<< "'"
		<< ", last_updated = now()::abstime::int4"
		<< " WHERE channel_id = "
		<< channel_id
		<< " AND user_id = "
		<< theUser->getID()
		;

#ifdef LOG_SQL
elog	<< "SUPPORTCommand::sqlQuery> "
		<< updateQuery.str().c_str()
		<< endl;
#endif

status = bot->SQLDb->Exec( updateQuery.str().c_str() ) ;

if( PGRES_COMMAND_OK != status )
	{
	elog	<< "SUPPORTCommand> SQL Error: "
			<< bot->SQLDb->ErrorMessage()
			<< endl ;

	bot->Notice(theClient, "An Error occured whilst processing your support. Please contact a CService Administrator.");
	return ;
	}

bot->Notice(theClient, "Done. Set your support for %s to %s.",
	channelName.c_str(), support.c_str());

bot->logDebugMessage("%s has set their support for %s to %c.",
	theUser->getUserName().c_str(), channelName.c_str(), supportChar);

/*
 * Right, now if they've voted "YES", and there are 10 supporters who have said YES
 * we move this to traffic check stage.
 */

if (supportChar == 'Y')
{
	/*
	 * Check to see if 10 people have said Yes.
	 */

	stringstream tenQuery;
	tenQuery 	<< "SELECT count(*) FROM supporters"
			<< " WHERE channel_id = "
			<< channel_id
			<< " AND support = 'Y'"
			;

#ifdef LOG_SQL
	elog	<< "SUPPORTCommand::sqlQuery> "
			<< tenQuery.str().c_str()
			<< endl;
#endif

	status = bot->SQLDb->Exec( tenQuery.str().c_str() ) ;

	if( PGRES_TUPLES_OK != status )
		{
		elog	<< "SUPPORTCommand> SQL Error: "
				<< bot->SQLDb->ErrorMessage()
				<< endl ;
		return ;
		}

	int count = atoi(bot->SQLDb->GetValue(0,0));

	if (count >= 4)
	{
		stringstream updatePendingQuery;
		updatePendingQuery	<< "UPDATE pending SET status = '1',"
					<< " last_updated = now()::abstime::int4,"
					<< " check_start_ts = now()::abstime::int4"
					<< " WHERE channel_id = "
					<< channel_id
					<< " AND status = '0'"
					;

#ifdef LOG_SQL
		elog	<< "SUPPORTCommand::sqlQuery> "
				<< updatePendingQuery.str().c_str()
				<< endl;
#endif

		bot->SQLDb->Exec( updatePendingQuery.str().c_str() ) ;
		bot->logDebugMessage("%s has just made it to traffic check phase.",
			channelName.c_str());
	}

return ;
}

/*
 * Now, if they don't support it - deny this application, and do all sorts
 * of other stuff.
 */

if (supportChar == 'N')
{
	/*
	 * Reject the application.
	 */

	stringstream updatePendingQuery;
	updatePendingQuery	<< "UPDATE pending SET status = '9',"
				<< " last_updated = now()::abstime::int4,"
				<< " decision_ts = now()::abstime::int4,"
				<< " decision = '--AUTOMATIC (REGPROC)-- NON-SUPPORT'"
				<< " WHERE channel_id = "
				<< channel_id
				<< " AND status = '0'"
				;

#ifdef LOG_SQL
	elog	<< "SUPPORTCommand::sqlQuery> "
			<< updatePendingQuery.str().c_str()
			<< endl;
#endif

	bot->SQLDb->Exec( updatePendingQuery.str().c_str() ) ;

	bot->logDebugMessage("%s has just been declined due to non-support.",
		channelName.c_str());

	/*
	 * Now, add some non-support entries.
	 * First, get the username and email address of the channel manager.
	 */

	stringstream mgrQuery;
	mgrQuery 	<< "SELECT user_name,email FROM users,pending"
			<< " WHERE pending.manager_id = users.id "
			<< " AND pending.channel_id = "
			<< channel_id
			<< " AND pending.status = '9'"
			;

#ifdef LOG_SQL
	elog	<< "SUPPORTCommand::sqlQuery> "
			<< mgrQuery.str().c_str()
			<< endl;
#endif

	status = bot->SQLDb->Exec( mgrQuery.str().c_str() ) ;

	if( PGRES_TUPLES_OK != status )
	{
		elog	<< "SUPPORTCommand> SQL Error: "
				<< bot->SQLDb->ErrorMessage()
				<< endl ;
		return ;
	}

	if(bot->SQLDb->Tuples() >= 0)
	{
		string managerName = bot->SQLDb->GetValue(0,0);
		string managerEmail = bot->SQLDb->GetValue(0,1);
		static const char* cmdHeader = "INSERT INTO noreg (user_name,email,channel_name,type,expire_time,created_ts,set_by,reason) VALUES ";

		stringstream noregQuery;
		noregQuery	<< cmdHeader
				<< "('', '','"
				<< escapeSQLChars(channelName) << "',"
				<< "1, (now()::abstime::int4 + (86400*3)), now()::abstime::int4, '* REGPROC', '-NON SUPPORT-'"
				<< ")" ;

#ifdef LOG_SQL
		elog	<< "SUPPORTCommand::sqlQuery> "
			<< noregQuery.str().c_str()
			<< endl;
#endif

		bot->SQLDb->Exec( noregQuery.str().c_str() ) ;

		/*
	  	 * Add a user-based noreg entry too.
		 */

		stringstream usernoregQuery;
		usernoregQuery	<< cmdHeader
				<< "('"
				<< escapeSQLChars(managerName) << "', '"
				<< escapeSQLChars(managerEmail) << "', "
				<< "'', "
				<< "1, (now()::abstime::int4 + (86400*30)), now()::abstime::int4, '* REGPROC', '-NON SUPPORT-'"
				<< ")" ;

#ifdef LOG_SQL
		elog	<< "SUPPORTCommand::sqlQuery> "
				<< usernoregQuery.str().c_str()
				<< endl;
#endif

		bot->SQLDb->Exec( usernoregQuery.str().c_str() ) ;

		/*
		 * Sigh, and a channel-log entry!
		 */

		stringstream clogQuery;
		clogQuery	<< "INSERT INTO channellog (ts, channelid, event, message, last_updated) VALUES ("
				<< "now()::abstime::int4, "
				<< channel_id << ", "
				<< "1, 'Non-support by "
				<< escapeSQLChars(theUser->getUserName()) << "', "
				<< "now()::abstime::int4)"
				;

#ifdef LOG_SQL
		elog	<< "SUPPORTCommand::sqlQuery> "
				<< clogQuery.str().c_str()
				<< endl;
#endif

		bot->SQLDb->Exec( clogQuery.str().c_str() ) ;
	}

}

return ;
}

} // namespace gnuworld.

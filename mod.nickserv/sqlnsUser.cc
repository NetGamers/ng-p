/*
 * sqlnsUser.cc
 *
 * Storage class for accessing user information either from the backend
 * or internal storage.
 *
 * $Id: sqlnsUser.cc,v 1.1 2002-01-14 23:33:40 jeekay Exp $
 */

#include	<strstream.h>
#include	<string.h>

#include	<cstring>

#include	"ELog.h"
#include	"misc.h"
#include	"sqlnsUser.h"
#include	"constants.h"
#include	"nickserv.h"
//#include	"cservice_config.h"

namespace gnuworld
{

using std::string ;
using std::endl ;

namespace nserv
{


const sqlnsUser::flagType sqlnsUser::F_LOGGEDIN =		0x01 ;

sqlnsUser::sqlnsUser(PgDatabase* _SQLDb)
 : id( 0 ),
   user_name(),
   password(),
   language_id( 0 ),
   flags( 0 ),
   coordX( 0 ),
   coordY( 0 ),
   coordZ( 0 ),
   alliance(),
   SQLDb( _SQLDb )
{
}

/*
 *  Load all data for this user from the backend. (Key: userID)
 */

bool sqlnsUser::loadData(int userID)
{
/*
 *  With the open database handle 'SQLDb', retrieve information about
 *  'userID' and fill our member variables.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlnsUser::loadData> Attempting to load data for user-id: "
		<< userID
		<< endl;
#endif

strstream queryString;
queryString	<< "SELECT "
		<< sql::user_fields
		<< " FROM users WHERE id = "
		<< userID
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlnsUser::loadData> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str()) ;
delete[] queryString.str() ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If the user doesn't exist, we won't get any rows back.
	 */

	if(SQLDb->Tuples() < 1)
		{
		return (false);
		}

	setAllMembers(0);

	return (true);
	}

return (false);
}

#define LOG_DEBUG
#define LOG_SQL

bool sqlnsUser::loadData(const string& userName)
{
/*
 *  With the open database handle 'SQLDb', retrieve information about
 *  'userID' and fill our member variables.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlnsUser::loadData> Attempting to load data for user-name: "
		<< userName
		<< endl;
#endif

strstream queryString;
queryString	<< "SELECT "
		<< sql::user_fields
		<< " FROM users WHERE lower(user_name) = '"
		<< string_lower(userName)
		<< "'"
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlnsUser::loadData> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str()) ;
delete[] queryString.str() ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If the user doesn't exist, we won't get any rows back.
	 */

	if(SQLDb->Tuples() < 1)
		{
		return (false);
		}

	setAllMembers(0);

	return (true);
	}

return (false);
}


void sqlnsUser::setAllMembers(int row)
{
/*
 *  Support function for both loadData's.
 *  Assumes SQLDb contains a valid results set for all user information.
 */

id = atoi(SQLDb->GetValue(row, 0));
user_name = SQLDb->GetValue(row, 1);
password = SQLDb->GetValue(row, 2);
language_id = atoi(SQLDb->GetValue(row, 4));
flags = atoi(SQLDb->GetValue(row, 5));
coordX = atoi(SQLDb->GetValue(row, 9));
coordY = atoi(SQLDb->GetValue(row, 10));
coordZ = atoi(SQLDb->GetValue(row, 11));
alliance = SQLDb->GetValue(row, 12);

/* Fetch the "Last Seen" time from the users_lastseen table. */

}

bool sqlnsUser::commit()
{
/*
 *  Build an SQL statement to commit the transient data in this storage class
 *  back into the database.
 */

static const char* queryHeader =    "UPDATE users ";
static const char* queryCondition = "WHERE id = ";

strstream queryString;
queryString	<< queryHeader
		<< "SET flags = " << flags << ", "
		<< "password = '" << password << "', "
		<< "language_id = " << language_id << ", "
                << "coordX = " << coordX << ", "
                << "coordY = " << coordY << ", "
                << "coordZ = " << coordZ << ", "
                << "alliance = '" << alliance << "' "
		<< queryCondition << id
		<< ends;

#ifdef LOG_SQL
	elog	<< "sqlnsUser::commit> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str()) ;
delete[] queryString.str() ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "sqlnsUser::commit> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< endl;

	return false;
 	}

return true;
}


sqlnsUser::~sqlnsUser()
{
// No heap space allocated
}

}

} // namespace gnuworld.

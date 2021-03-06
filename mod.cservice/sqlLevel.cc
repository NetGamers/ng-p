/*
 * sqlLevel.cc
 *
 * Storage class for accessing channel user/level information either from the backend
 * or internal storage.
 */

#include	<string>

#include	<cstring>
#include	<ctime>

#include	"ELog.h"
#include	"misc.h"
#include	"sqlLevel.h"
#include	"sqlUser.h"
#include	"sqlChannel.h"
#include	"constants.h"
#include	"cservice.h"
#include	"cservice_config.h"

namespace gnuworld
{

using std::string ;
using std::endl ;

const sqlLevel::flagType sqlLevel::F_AUTOOP =	0x01 ;
const sqlLevel::flagType sqlLevel::F_PROTECT =	0x02 ;
const sqlLevel::flagType sqlLevel::F_FORCED =	0x04 ;
const sqlLevel::flagType sqlLevel::F_AUTOVOICE =0x08 ;
const sqlLevel::flagType sqlLevel::F_ONDB =		0x10 ;
#ifdef FEATURE_INVITE
const sqlLevel::flagType sqlLevel::F_AUTOINVITE = 0x20 ;
#endif

sqlLevel::sqlLevel(PgDatabase* _SQLDb)
 :channel_id(0),
 user_id(0),
 access(0),
 forced_access(0),
 flags(0),
 suspend_expires(0),
 suspend_level(0),
 suspend_by(),
 added(0),
 added_by(),
 last_modif(::time(NULL)),
 last_modif_by(),
 last_used(0),
 SQLDb( _SQLDb )
{
}

bool sqlLevel::loadData(unsigned int userID, unsigned int channelID)
{
/*
 * Fetch a matching Level record for this channel and user ID combo.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlLevel::loadData> Attempting to load level data for "
		<< "channel-id: "
		<< channelID
		<< " and user-id: "
		<< userID
		<< endl;
#endif

stringstream queryString;
queryString	<< "SELECT "
		<< sql::level_fields
		<< " FROM levels WHERE channel_id = "
		<< channelID
		<< " AND user_id = "
		<< userID
		;

#ifdef LOG_SQL
	elog 	<< "sqlLevel::loadData> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If this combo doesn't exist, we won't get any rows back.
	 */

	if(SQLDb->Tuples() < 1)
		{
		return (false);
		}

	// Fetch dat from row 0
	setAllMembers(0);

	return (true);
	}

return (false);
}


void sqlLevel::setAllMembers(int row)
{
/*
 *  Support function for both loadData's.
 *  Assumes SQLDb contains a valid results set for all Level information.
 */

channel_id = atoi(SQLDb->GetValue(row, 0));
user_id = atoi(SQLDb->GetValue(row, 1));
access = atoi(SQLDb->GetValue(row, 2));
flags = atoi(SQLDb->GetValue(row, 3));
suspend_expires = atoi(SQLDb->GetValue(row, 4));
suspend_level = atoi(SQLDb->GetValue(row, 5));
suspend_by = SQLDb->GetValue(row, 6);
added = atoi(SQLDb->GetValue(row, 7));
added_by = SQLDb->GetValue(row, 8);
last_modif = atoi(SQLDb->GetValue(row, 9));
last_modif_by = SQLDb->GetValue(row, 10);
last_updated = atoi(SQLDb->GetValue(row, 11));
}

bool sqlLevel::commit()
{
/*
 *  Build an SQL statement to commit the transient data in this
 *  storage class back into the database.
 */

static const char* queryHeader =    "UPDATE levels ";

stringstream queryString;
queryString	<< queryHeader
		<< "SET flags = " << flags << ", "
		<< "access = " << access << ", "
		<< "suspend_expires = " << suspend_expires << ", "
		<< "suspend_level = " << suspend_level << ", "
		<< "suspend_by = '" << escapeSQLChars(suspend_by) << "', "
		<< "added = " << added << ", "
		<< "added_by = '" << escapeSQLChars(added_by) << "', "
		<< "last_modif = " << last_modif << ", "
		<< "last_modif_by = '" << escapeSQLChars(last_modif_by) << "', "
		<< "last_updated = now()::abstime::int4 "
		<< " WHERE channel_id = " << channel_id
		<< " AND user_id = " << user_id
		;

#ifdef LOG_SQL
	elog	<< "sqlLevel::commit> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "sqlLevel::commit> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< endl;

	return false;
 	}

return true;
}

bool sqlLevel::insertRecord()
{
static const char* queryHeader = "INSERT INTO levels (channel_id,user_id,access,flags,added,added_by,last_modif,last_modif_by,last_updated) VALUES (";

stringstream queryString;
queryString	<< queryHeader
		<< channel_id << ", "
		<< user_id << ", "
		<< access << ", "
		<< flags << ", "
		<< added << ", '"
		<< escapeSQLChars(added_by) << "', "
		<< last_modif << ", '"
		<< escapeSQLChars(last_modif_by) << "', "
		<< "now()::abstime::int4)"
		;

#ifdef LOG_SQL
	elog	<< "sqlLevel::insertRecord> "
			<< queryString.str()
			<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "sqlLevel::commit> Something went wrong: "
			<< SQLDb->ErrorMessage()
			<< endl;

	return false ;
 	}

return true;
}

sqlLevel::~sqlLevel()
{
}

} // Namespace gnuworld.

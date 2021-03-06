/*
 * sqlChannel.cc
 *
 * Storage class for accessing user information either from the backend
 * or internal storage.
 *
 * 20/12/2000: Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 * 30/12/2000: Moved static SQL data to constants.h --Gte
 * Set loadData up to take data from rows other than 0.
 */

#include	<string>

#include	<cstring>

#include	"ELog.h"
#include	"misc.h"
#include	"sqlChannel.h"
#include	"constants.h"
#include	"cservice.h"
#include	"cservice_config.h"

namespace gnuworld
{

using std::string ;
using std::endl ;

const sqlChannel::flagType sqlChannel::F_NOPURGE     = 0x00000001 ;
const sqlChannel::flagType sqlChannel::F_NOPART      = 0x00000002 ;
const sqlChannel::flagType sqlChannel::F_IDLE        = 0x00000004 ;
const sqlChannel::flagType sqlChannel::F_PARTNER     = 0x00000008 ;

const sqlChannel::flagType sqlChannel::F_SUSPEND     = 0x00000010 ;
const sqlChannel::flagType sqlChannel::F_TEMP        = 0x00000020 ;
const sqlChannel::flagType sqlChannel::F_CAUTION     = 0x00000040 ;
const sqlChannel::flagType sqlChannel::F_VACATION    = 0x00000080 ;

const sqlChannel::flagType sqlChannel::F_LOCKED      = 0x00000100 ;
const sqlChannel::flagType sqlChannel::F_FLOATLIM    = 0x00000200 ;
const sqlChannel::flagType sqlChannel::F_WELCOME     = 0x00000400 ;

const sqlChannel::flagType sqlChannel::F_STRICTOP    = 0x00020000 ;
const sqlChannel::flagType sqlChannel::F_NOOP        = 0x00040000 ;
const sqlChannel::flagType sqlChannel::F_AUTOTOPIC   = 0x00080000 ;

const sqlChannel::flagType sqlChannel::F_AUTOJOIN    = 0x00200000 ;
const sqlChannel::flagType sqlChannel::F_NOFORCE     = 0x00400000 ;
const sqlChannel::flagType sqlChannel::F_STRICTVOICE = 0x00800000 ;

const sqlChannel::flagType sqlChannel::F_INVISIBLE   = 0x01000000 ;

/* Current mask for channel flags                      0x01EE07FF */

const int sqlChannel::EV_MISC     = 1 ;
const int sqlChannel::EV_JOIN     = 2 ;
const int sqlChannel::EV_PART     = 3 ;
const int sqlChannel::EV_OPERJOIN = 4 ;
const int sqlChannel::EV_OPERPART = 5 ;
const int sqlChannel::EV_FORCE    = 6 ;
const int sqlChannel::EV_REGISTER = 7 ;
const int sqlChannel::EV_PURGE    = 8 ;

/* Manually added Comment */
const int sqlChannel::EV_COMMENT 	= 9  ;
const int sqlChannel::EV_REMOVEALL	= 10 ;
const int sqlChannel::EV_IDLE		= 11 ;

/* Suspend events */
const int sqlChannel::EV_SUSPEND = 12 ;
const int sqlChannel::EV_UNSUSPEND = 13 ;

sqlChannel::sqlChannel(PgDatabase* _SQLDb)
 : id(0),
   name(),
   flags(0),
   mass_deop_pro(3),
   flood_pro(7),
   url(),
   description(),
   comment(),
   keywords(),
   registered_ts(0),
   channel_ts(0),
   channel_mode(),
   userflags(0),
   last_topic(0),
   inChan(false),
   last_used(0),
   limit_offset(3),
   limit_period(20),
   last_limit_check(0),
   limit_grace(2), 
   limit_max(0), 
   welcome(),
   suspendExpires(0),
   invisible(0),
   SQLDb( _SQLDb )
{
}


bool sqlChannel::loadData(const string& channelName)
{
/*
 *  With the open database handle 'SQLDb', retrieve information about
 *  'channelName' and fill our member variables.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlChannel::loadData> Attempting to load data for"
		<< " channel-name: "
		<< channelName
		<< endl;
#endif

stringstream queryString ;
queryString	<< "SELECT "
		<< sql::channel_fields
		<< " FROM channels WHERE registered_ts <> 0"
		<< " AND lower(name) = '"
		<< escapeSQLChars(string_lower(channelName))
		<< "'"
		;

#ifdef LOG_SQL
	elog	<< "sqlChannel::loadData> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If the channel doesn't exist, we won't get any rows back.
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

bool sqlChannel::loadData(int channelID)
{
/*
 *  With the open database handle 'SQLDb', retrieve information about
 *  'channelID' and fill our member variables.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlChannel::loadData> Attempting to load data for "
		<< "channel-id: "
		<< channelID
		<< endl;
#endif

stringstream queryString;
queryString	<< "SELECT "
		<< sql::channel_fields
		<< " FROM channels WHERE registered_ts <> 0 AND id = "
		<< channelID
		;

#ifdef LOG_SQL
	elog	<< "sqlChannel::loadData> "
		<< queryString.str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If the channel doesn't exist, we won't get any rows back.
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


void sqlChannel::setAllMembers(int row)
{
/*
 *  Support function for both loadData's.
 *  Assumes SQLDb contains a valid results set for all channel information.
 */

id = atoi(SQLDb->GetValue(row, 0));
name = SQLDb->GetValue(row, 1);
flags = atoi(SQLDb->GetValue(row, 2));
mass_deop_pro = atoi(SQLDb->GetValue(row,3));
flood_pro = atoi(SQLDb->GetValue(row,4));
url = SQLDb->GetValue(row,5);
description = SQLDb->GetValue(row,6);
comment = SQLDb->GetValue(row,7);
keywords = SQLDb->GetValue(row,8);
registered_ts = atoi(SQLDb->GetValue(row,9));
channel_ts = atoi(SQLDb->GetValue(row,10));
channel_mode = SQLDb->GetValue(row,11);
userflags = atoi(SQLDb->GetValue(row,12));
last_updated = atoi(SQLDb->GetValue(row,13));
limit_offset = atoi(SQLDb->GetValue(row,14));
limit_period = atoi(SQLDb->GetValue(row,15));
limit_grace = atoi(SQLDb->GetValue(row,16));
limit_max = atoi(SQLDb->GetValue(row,17));
welcome = SQLDb->GetValue(row,18);
suspendExpires = atoi(SQLDb->GetValue(row, 19));
invisible = atoi(SQLDb->GetValue(row, 20));
}

bool sqlChannel::commit()
{
/*
 *  Build an SQL statement to commit the transient data in this storage class
 *  back into the database.
 */

static const char* queryHeader =    "UPDATE channels ";
static const char* queryCondition = "WHERE id = ";

stringstream queryString;
queryString	<< queryHeader
		<< "SET flags = " << flags << ", "
		<< "mass_deop_pro = " << mass_deop_pro << ", "
		<< "flood_pro = " << flood_pro << ", "
		<< "url = '" << escapeSQLChars(url) << "', "
		<< "keywords = '" << escapeSQLChars(keywords) << "', "
		<< "registered_ts = " << registered_ts << ", "
		<< "channel_ts = " << channel_ts << ", "
		<< "channel_mode = '" << channel_mode.c_str() << "', "
		<< "userflags = " << userflags << ", "
		<< "last_updated = now()::abstime::int4, "
		<< "limit_offset = " << limit_offset << ", "
		<< "limit_period = " << limit_period << ", "
		<< "limit_grace = " << limit_grace << ", "
		<< "limit_max = " << limit_max << ", "
		<< "description = '" << escapeSQLChars(description) << "', "
		<< "comment = '" << escapeSQLChars(comment) << "', "
		<< "welcome = '" << escapeSQLChars(welcome) << "', "
		<< "suspend_expires_ts = " << suspendExpires << ", "
		<< "invisible = " << invisible << " "
		<< queryCondition << id
		;

#ifdef LOG_SQL
	elog	<< "sqlChannel::commit> "
		<< queryString.str().c_str()
		<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	elog	<< "sqlChannel::commit> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< endl;
	return false;
 	}

return true;
}

bool sqlChannel::insertRecord()
{
static const char* queryHeader = "INSERT INTO channels (name, flags, registered_ts, channel_ts, channel_mode, last_updated) VALUES (";

stringstream queryString;
queryString	<< queryHeader
		<< "'" << escapeSQLChars(name) << "', "
		<< flags << ", "
		<< registered_ts << ", "
		<< channel_ts << ", '"
		<< escapeSQLChars(channel_mode) << "', "
		<< "now()::abstime::int4)"
		;

#ifdef LOG_SQL
	elog	<< "sqlChannel::insertRecord> "
			<< queryString.str()
			<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "sqlChannel::commit> Something went wrong: "
			<< SQLDb->ErrorMessage()
			<< endl;

	return false ;
 	}

return true;
}

const string sqlChannel::getLastEvent(unsigned short eventType, unsigned int& eventTime)
{
stringstream queryString;

queryString	<< "SELECT message,ts FROM channellog"
		<< " WHERE channelid = " << id
		<< " AND event = " << eventType
		<< " ORDER BY ts DESC LIMIT 1"
		;

#ifdef LOG_SQL
elog << "sqlChannel::getLastEvent> "
		 << queryString.str() << endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str());

if(PGRES_TUPLES_OK == status)
	{
	if(SQLDb->Tuples() < 1) { return(""); }
	
	string reason = SQLDb->GetValue(0, 0);
	eventTime = atoi(SQLDb->GetValue(0, 1));
	
	return reason;
	} // P_T_O == status

return("");
} // sqlChannel::getLastEvent()

sqlChannel::~sqlChannel()
{
	/* TODO: Clean up bans */
}

} // Namespace gnuworld

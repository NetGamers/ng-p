/*
 * sqlCommandLevel.cc - Implementation for a command level taken from a database
 *
 * History:
 *  20021013 - gk@ng - Initial creation
 */

#include <sstream>

#include "ELog.h"

#include "constants.h"
#include "sql.h"
#include "sqlCommandLevel.h"

namespace gnuworld {

using std::endl ;
using std::ends ;
using std::stringstream ;

const sqlCommandLevel::flagType sqlCommandLevel::F_LOCKED = 0x01;

sqlCommandLevel::sqlCommandLevel(PgDatabase* _SQLDb) :
  level(0),
  flags(0),
  last_updated(0),
  SQLDb(_SQLDb)
{
}

sqlCommandLevel::~sqlCommandLevel()
{
}

bool sqlCommandLevel::commit() {

  stringstream commitString;
  
  commitString << "UPDATE commands SET "
    << "command_name = '" << escapeSQLChars(command_name) 
    << "WHERE command_name = '" << escapeSQLChars(command_name) << "'"
    << ends;

#ifdef LOG_SQL
  elog << "sqlCommandLevel::commit> "
    << commitString
    << endl;
#endif

  ExecStatusType status = SQLDb->Exec(commitString.str().c_str());
  
  if(PGRES_COMMAND_OK != status) {
    elog << "sqlCommandLevel::commit> SQL Error: "
      << SQLDb->ErrorMessage()
      << endl;
    return false;
  }
  
  return true;

} // bool sqlCommandLevel::commit()


bool sqlCommandLevel::loadData(string& commandName) {

#ifdef LOG_DEBUG
  elog << "sqlCommandLevel::loadData> Attempting to load command level data for "
    << commandName
    << endl;
#endif

  stringstream queryString;
  queryString << "SELECT "
    << sql::command_fields
    << "FROM commands "
    << "WHERE command_name = upper('" << escapeSQLChars(commandName) << "')"
    << ends;
  
#ifdef LOG_SQL
  elog << "sqlCommandLevel::loadData> Executing SQL: "
    << queryString
    << endl;
#endif

  ExecStatusType status = SQLDb->Exec(queryString.str().c_str());
  
  if(PGRES_TUPLES_OK != status) {
    elog << "sqlCommandLevel::loadData> SQL Error: "
      << SQLDb->ErrorMessage()
      << endl;
    return false;
  }
  
  if(SQLDb->Tuples() < 1) {
    return false;
  }
  
  setAllMembers(0);
  
  return true;

} // bool sqlCommandLevel::loadData(string&)


void sqlCommandLevel::setAllMembers(unsigned int row) {

  command_name = SQLDb->GetValue(row, 0);
  domain = SQLDb->GetValue(row, 1);
  level = atoi(SQLDb->GetValue(row, 2));
  flags = atoi(SQLDb->GetValue(row, 3));
  comment = SQLDb->GetValue(row, 4);
  description = SQLDb->GetValue(row, 5);
  last_updated = atoi(SQLDb->GetValue(row, 6));
  last_updated_by = SQLDb->GetValue(row, 7);

} // void sqlCommandLevel::setAllMembers(int)

} // namespace gnuworld

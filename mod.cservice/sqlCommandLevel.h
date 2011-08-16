/*
 * sqlCommandLevel.h - Interface for a command level taken from a database
 *
 * History:
 *  20021013 - gk@ng - Initial creation
 */

#ifndef __SQLCOMMANDLEVEL_H
#define __SQLCOMMANDLEVEL_H

#include <ctime>
#include <string>

#include "libpq++.h"

namespace gnuworld {

using std::string;

class sqlCommandLevel {

public:
  sqlCommandLevel(PgDatabase*);
  virtual ~sqlCommandLevel();
  
  typedef unsigned int flagType;
  static const flagType F_LOCKED;
  
  /* Accessors */
  
  inline const string& getCommandName() const
    { return command_name; }
  
  inline const string& getDomain() const
    { return domain; }
  
  inline const short getLevel() const
    { return level; }
  
  inline const flagType getFlags() const
    { return flags; }
  
  inline const bool getFlag(flagType whichFlag) const
    { return flags & whichFlag; }
  
  inline const string& getComment() const
    { return comment; }
  
  inline const string& getDescription() const
    { return description; }
  
  inline const time_t getLastUpdated() const
    { return last_updated; }
  
  inline const string& getLastUpdatedBy() const
    { return last_updated_by; }
  
  /* Mutators */
  
  // TODO: Add some
  
  
  /* Miscellaneous methods */
  bool commit();
  bool loadData(string&);
  void setAllMembers(unsigned int);

protected:
  string command_name;
  string domain;
  short level;
  flagType flags;
  string comment;
  string description;
  time_t last_updated;
  string last_updated_by;
  
  PgDatabase* SQLDb;

}; // class sqlCommandLevel

} // namespace gnuworld

#endif

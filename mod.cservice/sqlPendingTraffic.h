/*
 * sqlPendingTraffic Class Header.
 *
 * Stores information about the uniqueness of visitors to
 * pending channels.
 */

#ifndef __SQLPENDINGTRAFFIC_H
#define __SQLPENDINGTRAFFIC_H

#include	<string> 
 
using std::string ;

namespace gnuworld
{ 
 
class sqlPendingTraffic
{

public:
	sqlPendingTraffic(PgDatabase*);
	bool insertRecord();
	bool commit();

	unsigned int channel_id; 
	unsigned int ip_number;
	unsigned int join_count; 

	PgDatabase*	SQLDb;
};

}
#endif // __SQLPENDINGTRAFFIC_H 

/*
 * sqlPendingTraffic Class Header.
 *
 * Stores information about the uniqueness of visitors to
 * pending channels.
 */

#ifndef __SQLPENDINGTRAFFIC_H
#define __SQLPENDINGTRAFFIC_H "$Id: sqlPendingTraffic.h,v 1.1 2002-01-14 23:14:25 morpheus Exp $"

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

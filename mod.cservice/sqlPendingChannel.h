/*
 * sqlPendingChannel Class Header.
 * Stores information about channels currently being registered.
 * We do a number of things to channels in this state ;)
 */

#ifndef __SQLPENDINGCHANNEL_H
#define __SQLPENDINGCHANNEL_H

#include	<string> 
#include	"sqlPendingTraffic.h"
 
using std::string ;

namespace gnuworld
{ 
 
class sqlPendingChannel
{

public:
	sqlPendingChannel(PgDatabase*);
	~sqlPendingChannel();

	bool commit();
	bool commitSupporter(unsigned int, unsigned int);
	void loadTrafficCache();

	unsigned int channel_id;
	unsigned int join_count;
	unsigned int unique_join_count;

	typedef map < int, int > supporterListType;
	supporterListType supporterList;

	typedef map < unsigned int, sqlPendingTraffic* > trafficListType;
	trafficListType trafficList;

	PgDatabase*	SQLDb;
};

}
#endif // __SQLPENDINGCHANNEL_H

/* networkData.h */

#ifndef __NETWORKDATA_H
#define __NETWORKDATA_H "$Id: networkData.h,v 1.1 2002-01-14 23:14:24 morpheus Exp $"

#include	<string>
#include	<ctime>

class sqlUser;
 
using std::string ;

namespace gnuworld
{
 
class networkData
{

public:

	networkData() ;
	virtual ~networkData() ; 

 	// Timestamp of when last message was recieved from this user.
	time_t messageTime;

	// Count of all bytes outputted to this client.
	unsigned int outputCount;

	// Total flood points accumulated by input from this client.
	unsigned int flood_points;

	// Pointer to the sqlUser record of this user (if it exists).
	// Ie: If they're authed.
	sqlUser* currentUser; 

	// Are we ignored or not..
	bool ignored;

} ;

#endif // __NETWORKDATA_H

} // Namespace gnuworld

#ifndef __JUUSER_H
#define __JUUSER_H

#include <string>
#include <sys/time.h>

namespace gnuworld
{

namespace nserv
{

using std::string ;

class juUser
{

public:
	juUser(string theNick, string theNumeric, time_t set, time_t expires, string theReason, string hostMask);
	
	virtual ~juUser();
	
	inline const string getNickName( void ) const
		{ return _NickName; }
	
	inline const string getNumeric( void ) const
		{ return _Numeric; }
	
	inline const time_t* getSet( void ) const
		{ return &_Set; }
	
	inline const time_t* getExpires( void ) const
		{ return &_Expires; }
	
	inline const string getReason( void ) const
		{ return _Reason; }
	
	inline const string getHostMask( void ) const
		{ return _HostMask;}

protected:
	string _NickName;
	string _Numeric;
	time_t _Set;
	time_t _Expires;
	string _Reason;
	string _HostMask;

}; // class juUser

} // namespace nserv

} // namespace gnuworld

#endif

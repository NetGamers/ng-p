#ifndef __JUUSER_H
#define __JUUSER_H "$Id"

#include <string>
#include <sys/time.h>

namespace gnuworld
{

namespace nserv
{

class juUser
{

public:
	juUser(string theNick, string theNumeric, time_t expires, string theReason);
	
	virtual ~juUser();
	
	inline const string getNickName( void ) const
		{ return _NickName; }
	
	inline const string getNumeric( void ) const
		{ return _Numeric; }
	
	inline const time_t getExpires( void ) const
		{ return _Expires; }
	
	inline const string getReason( void ) const
		{ return _Reason; }

protected:
	string _NickName;
	string _Numeric;
	time_t _Expires;
	string _Reason;

}; // class juUser

} // namespace nserv

} // namespace gnuworld

#endif

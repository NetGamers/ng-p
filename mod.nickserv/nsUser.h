
#ifndef __NSUSER_H_
#define __NSUSER_H_ "$id: $"

#include <string>
#include <sys/time.h>

namespace gnuworld
{

namespace nserv
{

class nsUser
{

public:

	typedef unsigned int FlagType;
	
	const static FlagType LOGGEDIN = 0x01;
	const static FlagType INQUEUE = 0x02;

	nsUser(string Num, time_t ConnT, string Nick);
	
	virtual ~nsUser();

	void SetData(string Num, time_t ConnT, string Nick);

	inline const string getNumeric() const
		{ return Numeric; }

	inline const string getNickName() const
           	{ return Nickname; }

	inline const time_t getCheckTime() const
		{ return CheckTime; }

	inline const bool getLoggedIn() const
		{ return Flags & LOGGEDIN; }

	inline const string getLoggedNick() const
          	{ return LoggedNick; }
	
	inline const bool getInQueue() const
		{ return Flags & INQUEUE; }

	void setCheckTime();

	inline void setLoggedIn() 
		{ Flags |= LOGGEDIN; }
			
	inline void setInQueue() 
		{ Flags |= INQUEUE; }

	inline void setLoggedNick(string _Nick)
		{ LoggedNick = _Nick; }	

	inline void remLoggedIn() 
		{ Flags &= ~LOGGEDIN; }
			
	inline void remInQueue() 
		{ Flags &= ~INQUEUE; }
		
	void clearFlags();

protected:

	FlagType 	Flags;

	string		Numeric;
	
	string 		Nickname;
	
	time_t		CheckTime;

	string		LoggedNick;
};

}
}

#endif

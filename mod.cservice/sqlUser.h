/* sqlUser.h */

#ifndef __SQLUSER_H
#define __SQLUSER_H "$Id: sqlUser.h,v 1.13 2004-05-01 15:31:43 jeekay Exp $"

#include <ctime>
#include <string>
#include <vector>

#include	"libpq++.h"

namespace gnuworld
{

using std::string ;
using std::vector ;

class iClient;

class sqlUser
{

public:

	sqlUser(PgDatabase*) ;
	virtual ~sqlUser() ;

	typedef unsigned short int	flagType ;
	static const flagType F_GLOBAL_SUSPEND;
	static const flagType F_LOGGEDIN; // Deprecated.
	static const flagType F_INVIS;
	static const flagType F_AUTOKILL;
	static const flagType F_NOPURGE;
	static const flagType F_BOT;
	static const flagType F_MEMO_REJECT;
	
	/*
	 *   User 'Event' Flags, used in the userlog table.
	 */

	static const unsigned int	EV_SUSPEND;
	static const unsigned int	EV_UNSUSPEND;
	static const unsigned int	EV_COMMENT;

	/*
	 *  Methods to get data atrributes.
	 */

	inline const unsigned int&	getID() const
		{ return id ; }

	inline const string&		getUserName() const
		{ return user_name ; }

	inline const string&		getPassword() const
		{ return password ; }

	inline const string&		getUrl() const
		{ return url ; }

	inline const unsigned int&	getLanguageId() const
		{ return language_id ; }

	inline bool		getFlag( const flagType& whichFlag ) const
		{ return (whichFlag == (flags & whichFlag)) ; }

	inline const flagType&		getFlags() const
		{ return flags ; }

	inline const string&		getLastUpdatedBy() const
		{ return last_updated_by ; }

	inline const time_t&		getLastUpdated() const
		{ return last_updated ; }

	inline const time_t&		getLastUsed() const
		{ return last_used ; }

	inline const string&		getEmail() const
		{ return email ; }

	inline bool isAuthed()
		{ return (networkClientList.size() != 0); }
  
  inline void addAuthedClient(iClient* theClient)
    { networkClientList.push_back(theClient); }
  
  inline void removeAuthedClient(iClient* theClient) {
    networkClientListType::iterator ptr = networkClientList.begin();
    while(ptr != networkClientList.end()) {
      iClient* testClient = *ptr;
      if(testClient == theClient) {
        ptr = networkClientList.erase(ptr);
      } else {
        ++ptr;
      }
    }
  }

	inline const string&		getComment() const
		{ return comment; }
	
	inline const time_t& getSuspendedExpire() const
		{ return suspendedExpire; }
	
	inline const unsigned int& getQuestionID() const
		{ return questionID; }
	
	inline const string& getVerificationData() const
		{ return verificationData; }
  
  inline const unsigned int& getMaxLogins() const
    { return maxlogins; }

  inline const unsigned int& getVerify() const
    { return verify; }

	/*
	 *  Methods to set data atrributes.
	 */
	
	inline void setVerificationData(const string& _verificationData)
		{ verificationData = _verificationData; }
	
	inline void setQuestionID(const int& _questionID)
		{ questionID = _questionID; }
	
	inline void setSuspendedExpire( const time_t& _suspendedExpire )
		{ suspendedExpire = _suspendedExpire; }
	
	inline void setFlag( const flagType& whichFlag )
		{ flags |= whichFlag; }

	inline void removeFlag( const flagType& whichFlag )
		{ flags &= ~whichFlag; }

	inline void setPassword( const string& _password )
		{ password = _password; }

	inline void setLastSeen( const time_t& _last_seen, string _last_hostmask )
		{ last_seen = _last_seen; last_hostmask = _last_hostmask ; commitLastSeen(); }

	inline void setLanguageId( const unsigned int& _language_id )
		{ language_id = _language_id; }

	inline void setLastUsed( const time_t& _last_used )
		{ last_used = _last_used; }

	inline void setEmail( const string& _email )
		{ email = _email; }
	
	inline void setUserName( const string& _user_name )
		{ user_name = _user_name; }

	inline void setComment( const string& _comment )
		{ comment = _comment; }
  
  inline void setMaxLogins(const unsigned int& _maxlogins)
    { maxlogins = _maxlogins; }
  
  inline void setVerify(const unsigned int& _verify)
    { verify = _verify; }

	/*
	 * Method to perform a SQL 'UPDATE' and commit changes to this
	 * object back to the database.
	 */

	bool commit();
	bool commitLastSeen();
	time_t	getLastSeen();
	const string getLastHostMask();

	bool loadData( int );
	bool loadData( const string& );
	void setAllMembers( int );
	void writeEvent( unsigned short, sqlUser*, const string& );
	const string getLastEvent( unsigned short, unsigned int&);
  
  typedef vector <iClient*> networkClientListType;
  networkClientListType networkClientList;

protected:

	unsigned int	id ;
	string		user_name ;
	string		password ;
	time_t		last_seen ;
	string		url ;
	unsigned int	language_id ;
	flagType	flags ;
	string		last_updated_by ;
	time_t		last_updated ;
	time_t		last_used;
	string		email ;
	string          last_hostmask ;
	string		comment ;
	time_t suspendedExpire;
	unsigned int questionID;
	string verificationData;
  unsigned int maxlogins;
  unsigned int verify;

	PgDatabase*	SQLDb;
} ;

} // namespace gnuworld

#endif // __SQLUSER_H


/* sqlUser.h */

#ifndef __SQLUSER_H
#define __SQLUSER_H "$Id: sqlUser.h,v 1.3 2002-01-21 14:42:16 morpheus Exp $"

#include	<string>
#include	<ctime>
#include	"libpq++.h"

namespace gnuworld
{

using std::string ;

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
	static const flagType F_NOTE;
	
	/*
	 *   User 'Event' Flags, used in the userlog table.
	 */

	static const unsigned int	EV_SUSPEND;
	static const unsigned int	EV_UNSUSPEND;
	static const unsigned int       EV_COMMENT;

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

        inline const unsigned int&      getCoordX() const
                { return coordX ; }
        
        inline const unsigned int&      getCoordY() const
                { return coordY ; }
         
        inline const unsigned int&      getCoordZ() const
                { return coordZ ; }
        
        inline const string&            getAlliance() const 
                { return alliance ; }

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

	inline iClient* isAuthed()
		{ return networkClient; }

	/*
	 *  Methods to set data atrributes.
	 */

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

        inline void setCoordX( const unsigned int& _coordX )
                { coordX = _coordX; }
        
        inline void setCoordY( const unsigned int& _coordY )
                { coordY = _coordY; }
        
        inline void setCoordZ( const unsigned int& _coordZ )
                { coordZ = _coordZ; }

        inline void setAlliance( const string& _alliance )
                { alliance = _alliance; }

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
	iClient*	networkClient;
	void writeEvent( unsigned short, sqlUser*, const string& );
	const string getLastEvent( unsigned short, unsigned int&);

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
        unsigned int    coordX ;
        unsigned int    coordY ;
        unsigned int    coordZ ;
        string          alliance ;
	string		email ;
	string          last_hostmask ;

	PgDatabase*	SQLDb;
} ;

} // namespace gnuworld

#endif // __SQLUSER_H


/* This is based on the mod.cservice sqlUser class */

#ifndef __SQLUSER_H
#define __SQLUSER_H "$Id: sqlnsUser.h,v 1.1 2002-01-14 23:33:40 jeekay Exp $"

#include	<string>
#include	<ctime>
#include	"libpq++.h"

namespace gnuworld
{

using std::string ;

class iClient;

namespace nserv
{

using gnuworld::iClient;

class sqlnsUser
{

public:

	sqlnsUser(PgDatabase*) ;
	virtual ~sqlnsUser() ;

	typedef unsigned short int	flagType ;
	static const flagType F_LOGGEDIN; // Deprecated.


	/*
	 *  Methods to get data atrributes.
	 */

	inline const unsigned int&	getID() const
		{ return id ; }

	inline const string&		getUserName() const
		{ return user_name ; }

	inline const string&		getPassword() const
		{ return password ; }


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



	/*
	 *  Methods to set data atrributes.
	 */

	inline void setFlag( const flagType& whichFlag )
		{ flags |= whichFlag; }

	inline void removeFlag( const flagType& whichFlag )
		{ flags &= ~whichFlag; }


	/*
	 * Method to perform a SQL 'UPDATE' and commit changes to this
	 * object back to the database.
	 */

	bool commit();

	bool loadData( int );
	bool loadData( const string& );
	void setAllMembers( int );

protected:

	unsigned int	id ;
	string		user_name ;
	string		password ;
	unsigned int	language_id ;
	flagType	flags ;
        unsigned int    coordX ;
        unsigned int    coordY ;
        unsigned int    coordZ ;
        string          alliance ;

	PgDatabase*	SQLDb;
} ;

}
} // namespace gnuworld

#endif // __SQLUSER_H


#ifndef __NICKSERVCLASS_H
#define __NICKSERVCLASS_H "$Id: nickservClass.h,v 1.6 2002-06-30 16:19:57 jeekay Exp $"

#include	"nsUser.h"
#include	"juUser.h"
#include	<set>

namespace gnuworld
{

namespace nserv
{

class Command;
/*
 *  Subclass the postgres API to create our own accessor
 *  to get at the PID information.
 */

class nsDatabase : public PgDatabase
{
public:
	nsDatabase(const string& conninfo)
	 : PgDatabase(conninfo.c_str()) {}
	virtual ~nsDatabase() {}
};

class nickserv : public xClient
{

protected:

	/**
	 * The type used to store the client's command handlers.
	 */
	typedef map< string, Command*, noCaseCompare > commandMapType ;

	/**
	 * The value_type of the command table type.
	 * This type will be used to model key/value pairs
	 * for the command handler table.
	 */
	typedef commandMapType::value_type pairType ;

	// This is where we store the people are are going to kill
	typedef map<string, nsUser*> killQueue;
	typedef killQueue::iterator killIterator;

	// This is the type where we store the current DB users with autokill set
	typedef set<string> autoKillListType;

	// Lists of juped nicks
	typedef map<string, juUser*> jupeNickType;
	typedef jupeNickType::iterator jupeIteratorType;

	// Logs debug information
	string  debugChan;

	// How long to let clients live
	int     timeToLive;
	int initialWait;
	unsigned int checkNickMax;

	// How frequently we refresh admin records
	int     adminRefreshTime;
	
	// How often do we kill juped nicks
	unsigned int jupeNumericStart;
	unsigned int jupeNumericCount;
	unsigned int jupeExpireTime;
	unsigned int jupeDefaultLength;
	
	// DB connection times
	int dbConnCheckTime;
	int dbConnCheckMax;
	int dbConnRetries;
	
	// Where is our cservice?
	gnuworld::cservice* myCService;
	
	// What is it called?
	string nickCService;
	
public:
	inline gnuworld::cservice* getCService()
		{ return myCService; }

	inline const string getDebugChannel() const
	  { return debugChan; }
	
	inline const unsigned int getCheckNickmax() const
		{ return checkNickMax; }

	/**
	 * Default, and only constructor, receives the name
	 * of the configuration file for this client.
	 */
	nickserv( const string& configFileName ) ;

	/**
	 * Destructor cleans up any allocated memory, etc.
	 */
	virtual ~nickserv() ;

	/**
	 * This method is called by the xServer when it wants information
	 * about the channels this client will be on.
	 */

	virtual int BurstChannels() ;

	/**
	 * This method is called by the xServer when it has connected
	 * to the network
	 */
	
	virtual int OnConnect();

	/**
	 * This method is invoked each time the client is sent a
	 * PRIVMSG.
	 */
	virtual int OnPrivateMessage( iClient*, const string&,
			bool secure = false ) ;

	virtual int OnCTCP( iClient* ,
                const string& ,
                const string&,
		bool Secure = false ) ;

	/**
	 * This method is invoked each time a network event occurs.
	 */
	virtual int OnEvent( const eventType&,
		void* = 0, void* = 0,
		void* = 0, void* = 0 ) ;


	/**
	 * This method is invoked each time a channel event occurs
	 * for one of the channels for which this client has registered
	 * to receive channel events.
	 */
	virtual int OnChannelEvent( const channelEventType&,
		Channel*,
		void* = 0, void* = 0,
		void* = 0, void* = 0 ) ;

	virtual int OnTimer(xServer::timerID, void*);

	/**
	 * This method is called once this client has been attached
	 * to an xServer.  This method is overloaded because the
	 * command handlers each require a reference to the xServer
	 * for efficiency.
	 */
	virtual void ImplementServer( xServer* ) ;

	 /*
	 * This method will kick the given user from the given channel
	 * for the given reason (arg 3).
	 */
	virtual bool Kick( Channel*, iClient*, const string& ) ;

	/**
	 * This method will cause the bot to join the channel with
	 * the given name, if the client is not already on that
	 * channel.
	 */
	virtual bool Join( const string&, const string& = string(),
		time_t = 0, bool = false) ;

	/**
	 * This method will cause this client to part the given channel,
	 * if it is already on that channel.
	 */
	virtual bool Part( const string& ) ;

	/**
	 * This method will register a given command handler, removing
	 * (and deallocating) the existing handler for this command,
	 * should one exist.
	 */
	virtual bool RegisterCommand( Command* ) ;

	/**
	 * This method will unregister the command handler for the command
	 * of the given command name, deallocating the object from the
	 * heap as well.
	 */
	virtual bool UnRegisterCommand( const string& ) ;

	bool checkUser(nsUser*);
	
	void removeFromQueue(iClient*);
	
	void authUser(iClient*, const string&);
	
	/* Allow logging of debug messages */
	bool logDebugMessage(const char*, ...);

	/* How to jupe nicks */
	void initialiseJupeNumerics( void );
	bool jupeNick( string theNick, string hostMask, string theReason, time_t duration );
	bool removeJupeNick( string theNick, string theReason = "End Of Jupe" );
	juUser* findJupeNick( string theNick );
	void checkJupeExpire( void );
	
	// Check the DB connection is ok
	void checkDBConnectionStatus( void );
	
	// Get NS admin access level
	int getAdminAccessLevel( iClient* );
	
	// Fill in the list of autokillers
	void refreshAutoKillList( void );
	
	// Process kill queue
	void processKillQueue( void );
	
	/*
	 * The type of a constant iterator to the command map.
	 */
	typedef commandMapType::const_iterator constCommandIterator ;

	/**
	 * Retrieve a constant iterator to the beginning of the command
	 * table.
	 */
	constCommandIterator command_begin() const
		{ return commandMap.begin() ; }

	/**
	 * Retrieve a constant iterator to the end of the command table.
	 */
	constCommandIterator command_end() const
		{ return commandMap.end() ; }

	/**
	 * Retrieve a constant iterator to a command handler for the
	 * given command token.
	 */
	constCommandIterator findCommand( const string& theComm ) const
		{ return commandMap.find( theComm ) ; }

	/**
	 * The type of a mutable iterator to the command map.
	 */
	typedef commandMapType::iterator commandIterator ;

	/**
	 * Retrieve a mutable iterator to the beginning of the command
	 * table.
	 */
	commandIterator command_begin()
		{ return commandMap.begin() ; }

	/**
	 * Retrieve a mutable iterator to the end of the command table.
	 */
	commandIterator command_end()
		{ return commandMap.end() ; }

	/**
	 * Retrieve a mutable iterator to a command handler for the
	 * given command token.
	 */
	commandIterator findCommand( const string& theComm )
		{ return commandMap.find( theComm ) ; }

	inline const nsDatabase* getSQLDb( void ) const
		{ return SQLDb; }

protected:

	/**
	 * PostgreSQL Database
	 */
	nsDatabase* SQLDb;

	/**
	 * The command handler table.
	 */
	commandMapType		commandMap ;
	
	killQueue		KillingQueue;

	jupeNickType jupedNickList;
	
	autoKillListType autoKillList;
	
	xServer::timerID processQueueID;
	xServer::timerID dbConnCheckID;
	xServer::timerID jupeExpireID;
	
	// All our DB config information
	string confSqlHost;
	string confSqlDb;
	string confSqlPort;
	string confSqlUser;
	string confSqlPass;
	
} ; 
 
} //namespace nserv

} //namespace gnuworld

#endif

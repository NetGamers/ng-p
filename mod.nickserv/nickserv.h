
#ifndef __NICKSERV_H
#define __NICKSERV_H "$Id: nickserv.h,v 1.1 2002-01-14 23:33:39 jeekay Exp $"


#include	<string>
#include	<vector>
#include	<map>
#include        <iomanip>

#include	<cstdio>

#include	"client.h"
#include	"iClient.h"
//#include	"iServer.h"
#include	"server.h"
#include	"libpq++.h"
#include	"libpq-int.h"
#include        "match.h"
#include	"md5hash.h" 
#include	"nickservCommands.h"
#include	"nsUser.h"

namespace gnuworld
{
 
using std::string ;
using std::vector ;

namespace nserv
{

class Command;
//using gnuworld::xServer;
/*
 *  Sublcass the postgres API to create our own accessor
 *  to get at the PID information.
 */

class cmDatabase : public PgDatabase
{
public:
	cmDatabase(const string& conninfo)
	 : PgDatabase(conninfo.c_str()) {}
	virtual ~cmDatabase() {}

	inline int getPID() const
		{ return pgConn->be_pid; }
};

/// Forward declaration of command handler class
//class Command ;

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

	//typedef list<nsUser*> killQueue;
	typedef map<string, nsUser*> killQueue;
	
	typedef killQueue::iterator killIterator;
	
	// Logs debug information
	string  debugChan;

	// How long to let clients live
	int     timeToLive;
	
public:

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
	
	nsUser*	isAuth(iClient*);

	void	authUser(iClient*, const string&);
	
	bool	isInQueue(iClient*);

	bool    logDebugMessage(const string&);

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

	

	/**
	 * PostgreSQL Database
	 */
	cmDatabase* SQLDb;

	
		
protected:


	/**
	 * The command handler table.
	 */
	commandMapType		commandMap ;
	
	unsigned int		authLen;
	
	killQueue		KillingQueue;

	xServer::timerID processQueueID;
	
} ; 
 
} //namespace uworld

} // namespace gnuworld
 
#endif // __NICKSERV_H

#ifndef __CSERVICECLASS_H
#define __CSERVICECLASS_H "$Id: cserviceClass.h,v 1.23 2004-08-28 12:30:12 jeekay Exp $"

#include	<map>

#include	"cservice_config.h"

class PgDatabase;

namespace gnuworld
{

using std::map;
using std::string;

class EConfig;

class Command;

class sqlBan;
class sqlChannel;
class sqlCommandLevel;
class sqlLevel;
class sqlPendingChannel;
class sqlUser;
class sqlVerify;

class cservice : public xClient
{
protected:

	EConfig* cserviceConfig; /* Configfile */
	
	/* Command listings */
	typedef map< string, Command*, noCaseCompare > commandMapType ;
	typedef commandMapType::value_type pairType ;
	commandMapType          commandMap;
	
	/* What is our NS instance called? */
	string nickNickServ;

public:

	PgDatabase* SQLDb; /* PostgreSQL Database */
	string confSqlHost;
	string confSqlPass;
	string confSqlDb;
	string confSqlPort;
	string confSqlUser;
	short connectRetries;
	unsigned int connectCheckFreq;
	unsigned int connectRetry;
	unsigned int limitCheckPeriod;
	unsigned int loginDelay;

	void checkDbConnectionStatus();
	string pendingPageURL;

	/* Pointer to our NS instance */
	gnuworld::nserv::nickserv* myNickServ;

	cservice(const string& args);
	virtual ~cservice();

	// Return a PgDatabase* to the current SQLDb
	inline const PgDatabase* getSQLDb( void ) const
		{ return SQLDb; }

	virtual int OnConnect();
	virtual int BurstChannels();
	virtual int OnPrivateMessage( iClient*, const string&,
		bool = false  );
	virtual void ImplementServer( xServer* ) ;
	virtual bool isOnChannel( const string& ) const;
	virtual bool RegisterCommand( Command* ) ;
	virtual bool UnRegisterCommand( const string& ) ;
	virtual void OnChannelModeO( Channel*, ChannelUser*,
		const xServer::opVectorType& ) ;
	virtual void OnChannelModeV( Channel*, ChannelUser*,
		const xServer::voiceVectorType& );
	virtual int OnChannelEvent( const channelEventType& whichEvent,
		Channel* theChan,
		void* data1, void* data2, void* data3, void* data4 );
	virtual int OnEvent( const eventType&,
		void*, void*, void*, void*);
	virtual int OnCTCP( iClient* Sender,
                const string& CTCP,
                const string& Message,
                bool Secure = false ) ;
	virtual int OnTimer(xServer::timerID, void*);
	virtual int OnWhois( iClient* sourceClient,
			iClient* targetClient );

	/* Sends a notice to a channel from the server. */
	bool serverNotice( Channel*, const char*, ... );
	bool serverNotice( Channel*, const string& );

	/* Log an administrative alert to the relay channel & log. */
	bool logAdminMessage(const char*, ... );

	/* Log an debug message to the debug channel */
	bool logDebugMessage(const char*, ... );
	
	/** Log a message to the administrative channel. */
	bool logErrorMessage(const char*, ... );

	/* Write a channel log */
	void writeChannelLog(sqlChannel*, iClient*, unsigned short, const string&);

	typedef commandMapType::const_iterator constCommandIterator ;
	constCommandIterator command_begin() const
                { return commandMap.begin() ; }

	constCommandIterator command_end() const
                { return commandMap.end() ; }

	constCommandIterator findCommand( const string& theComm ) const
                { return commandMap.find( theComm ) ; }

	/* Returns the access sqlUser has in channel sqlChan. */
	short getAccessLevel( sqlUser*, sqlChannel* );

	/* Returns the access sqlUser has in channel sqlChan taking into account
	 * suspensions, etc.
	 * If "bool" is true, then send sqlUser a notice about why they don't
	 * have a particular access. */
	short getEffectiveAccessLevel( sqlUser*, sqlChannel*, bool );

	/* Returns what admin access a user has. */
	short getAdminAccessLevel( sqlUser* );

	/* Returns what access a user has in the coder channel */
	short getCoderAccessLevel( sqlUser* );
  
	/* Returns what access is required for a given command */
	sqlCommandLevel* getLevelRequired(string, string, bool = true);
  
	/* Returns the verify for a given sqlUser* */
	string getVerify(const unsigned int&);

	/* Fetch a user record for a user. */
	sqlUser* getUserRecord( const string& );

	/* Checks if this client is logged in, returns a sqlUser if true.
	 * If "bool" is true, send a notice to the client telling them off. */
	sqlUser* isAuthed(iClient*, bool );

	/* Checks to see if this users is forced on this channel */
	unsigned short isForced(sqlChannel*, sqlUser*);

	/* Fetch a channel record for a channel. */
	sqlChannel* getChannelRecord( const string& );
	sqlChannel* getChannelRecord( int );
	
	/* Part a channel that has become idle. */
	bool partIdleChannel( sqlChannel* );

	/* Fetch a access level record for a user/channel combo. */
	sqlLevel* getLevelRecord(sqlUser*, sqlChannel*);

	/* Fetch the user status flags. L = in cache, P = got password, U = Is authed. */
	string userStatusFlags( const string& );

	/* Formats a timestamp into a "X Days, XX:XX:XX" from 'Now'. */
	const string prettyDuration( int, const string& = "all" ) const ;

	/* Returns the current "Flood Points" this iClient has. */
 	unsigned short getFloodPoints(iClient*);

	/* Sets the flood counter for this iClient. */
 	void setFloodPoints(iClient*, unsigned short);

	/* Determins if a client is in "Flooded" state, and if so Notice them. */
	bool hasFlooded(iClient*, const string&);

	/* Sets the timestamp for when we first recieved a msg from this client.
	 * within the flood period. */
	void setLastRecieved(iClient*, time_t);

	/* Find out when we first heard from this chap. */
	time_t getLastRecieved(iClient*);

	/* Send the current MOTD to a user */
	bool sendMOTD(const iClient* theClient);

	// Typedef's for user/channel Hashmaps.
	// User hash, Key is Username.
	typedef map< string, sqlUser*, noCaseCompare > sqlUserHashType ;

	// Typedef's for command levels
	typedef map< pair <string, string>, sqlCommandLevel* > sqlCommandLevelsType;
	sqlCommandLevelsType sqlCommandLevels;

	// Typedef's for verifies
	typedef map< unsigned int, string > verifiesType;
	verifiesType verifies;

	// Channel hash, Key is channelname.
	typedef map< string, sqlChannel*, noCaseCompare > sqlChannelHashType ;
	typedef map< int, sqlChannel* > sqlChannelIDHashType ;

	// Accesslevel cache, key is pair(chanid, userid).
	typedef map < pair <int, int>, sqlLevel* > sqlLevelHashType ;

 	/* Silence List */
	typedef map < string, pair < time_t, string > > silenceListType;
	silenceListType silenceList;

	bool isIgnored(iClient*);
	void setIgnored(iClient*, bool);

	// Cache of user records.
	sqlUserHashType sqlUserCache;

	// Cache of channel records.
	sqlChannelHashType sqlChannelCache;

	// Cache of channel records indexed by channel ID.
	sqlChannelIDHashType sqlChannelIDCache;

	// Cache of Level records.
	sqlLevelHashType sqlLevelCache;

	// Some counters for statistical purposes.
	unsigned int userHits;
	unsigned int userCacheHits;
	unsigned int channelHits;
	unsigned int channelCacheHits;
	unsigned int levelHits;
	unsigned int levelCacheHits;
	unsigned int banHits;
	unsigned int banCacheHits;
	unsigned int dbErrors;
	unsigned int joinCount;

	/* No of seconds offset our local time is from server time. */
	int dbTimeOffset;

	// To keep track of how many custom data chunks
	// we have allocated.
	unsigned int customDataAlloc;

	// Flood/Notice relay channel - Loaded via config.
	string relayChan;
	string debugChan;

	// Loaded via config.
	// Interval at which we pick up updates from the Db.
	int updateInterval;

	// Interval at which we check for expired bans/suspends.
	int expireInterval;

	// Interval at which we attempt to purge the cache(s).
	int cacheInterval;

	/* Duration in seconds at which an idle user/chan/level/ban
	 * record should be purged from the cache. */
	int idleUserPeriod;
	int idleChannelPeriod;
	int idleLevelPeriod;
	
	/** Duration after which a channel will be idle parted. */
	int idleChannelPartPeriod;

	/* Duration in seconds at which a 'pending' channel should
	 * be notified that it is so. */
	int pendingChanPeriod;

	// Input flood rate.
	unsigned int input_flood;
	unsigned int output_flood;
	int flood_duration;
	int topic_duration;

	// Timestamp's of when we last checked the database for updates.
	time_t lastChannelRefresh;
	time_t lastUserRefresh;
	time_t lastLevelRefresh;
	time_t lastBanRefresh;

	/* TimerID for checking on the database connection. */
	xServer::timerID dBconnection_timerID;

	/* TimerID we recieve for checking for changes in the Db. */
	xServer::timerID update_timerID;

	/* TimerID we recieve every XX seconds for expiration of bans/suspend. */
	xServer::timerID expire_timerID;

	/* TimerID we recieve every XX hours for expiration of cached entries. */
	xServer::timerID cache_timerID;

	/* TimerID we recieve every XX hours for the notification of pending channels. */
	xServer::timerID pending_timerID;

	/* TimerID we recieve every seconds when we should check if a channel limit needs changing */ 
        xServer::timerID limit_timerID; 

	// Language definitions table (Loaded from Db).
	typedef map < string, pair <int, string> > languageTableType;
	languageTableType languageTable;

	// Language translations table (Loaded from Db).
	typedef map < pair <int, int>, string > translationTableType ;
	translationTableType translationTable;

	void loadTranslationTable();

	// Locked commands table (dynamic, reset on restart)
	typedef map <string, string> lockedCommandsType; // command, reason
	lockedCommandsType lockedCommands;

	// Method to retrieve a translation string.
	const string getResponse( sqlUser*, int , string = string() );

	// Check for valid hostmask.
	virtual bool validUserMask(const string& userMask) const ;
	
	/* Help topics (Loaded from Db) */ 
        typedef map < pair <int, string>, string > helpTableType; 
        helpTableType helpTable; 
    
        void loadHelpTable(); 
        const string getHelpMessage(sqlUser*, string); 

	/**
	 * Count channel ops.
	 */
	static size_t countChanOps(const Channel*);

	// Deop everyone on this channel.
	void deopAllOnChan(Channel*);
	void deopAllUnAuthedOnChan(Channel*);

	/* sets a description (url) topic combo. */
	void doAutoTopic(sqlChannel*);

	/* Automatically updates the floating limit for this channel */ 
        void doFloatingLimit(sqlChannel*, Channel*); 

	/* Bans & kicks a specified user with a specific reason */
	bool doInternalBanAndKick(sqlChannel*, iClient*, const string&);

	/* Support function to check if a user is banned on a channel. */
	sqlBan* isBannedOnChan(sqlChannel*, iClient*);

	/* Matches DB bans, and kicks supplied user if neccessary */
	bool checkBansOnJoin( Channel*, sqlChannel* , iClient* );

	time_t currentTime() const ;

	/* Queue to hold pending reops */
	typedef map < string, time_t > reopQType;
	reopQType reopQ;

	/* List of channels in 'pending' registration state. */
	typedef map < string, sqlPendingChannel*, noCaseCompare > pendingChannelListType;
	pendingChannelListType pendingChannelList;

	/* List of configuration values */
	typedef map <string, string, noCaseCompare > configListType;
	configListType configList;
	
	const string getConfigItem(string);

	/* Check for a correct user password */
	bool isPasswordRight(sqlUser* theUser, const string& password);

	/*
	 *  Load the pendingChannelList from the database.
	 *  This list contains details about channels currently
	 *  'Pending' successful registration.
	 */

	void loadPendingChannelList();

	/*
	 *  Timer Functions.
	 *  These support functions are called at periodic
	 *  intervals to perform maintainence, etc.
	 */

	/*
	 * Expire suspends, ignores and bans respectively.
	 */
	void expireSuspends();
	void expireSilence();
	void expireBans();
	void expireGlobalSuspends();

	/*
	 *  Cache expiration functions.
	 *  To expire idle user/level/ban records from the
	 *  cache.
	 *  N.B: We'll never expire out channel records because
	 *  this information may be used to cancel 'unused'
	 *  channels.
	 */

	void cacheExpireUsers();

	/*
	 *  Expire Ban records, only if the channel
	 *  record is 'idle'.
	 */

	void cacheExpireBans();

	/*
	 *  Expire idle Level records.
	 */

	void cacheExpireLevels();

	/*
	 * Process any pending reop requests by the bot.
	 */
	void performReops();

	/*
	 * Process any Postgres notification requests,
	 * reloading cached records if neccessary.
	 */
	void processDBUpdates();

	/*
	 *  Send a generic Error Message, may log/etc at a later date.
	 */
	void dbErrorMessage(iClient*);

	/*
	 *  Misc uncategorisable functions.
	 */

	unsigned int preloadUserDays;

	void preloadBanCache();
	void preloadChannelCache();
	unsigned short int preloadCommandLevelsCache();
	unsigned short int preloadConfigCache();
	void preloadLevelsCache();
	void preloadUserCache();
	unsigned short int preloadVerifiesCache();

	void updateChannels();
	void updateUsers();
	void updateLevels();
	void updateBans();
	void updateLimits();

	typedef map < string, int > statsMapType;
	statsMapType statsMap;

	void incStat(const string& name);
	void incStat(const string& name, unsigned int amount);
  
	void noticeAllAuthedClients(sqlUser* theUser, const char* Message, ... );
  
	unsigned int totalCommands;
  
#ifdef FEATURE_FORCELOG
        void writeForceLog(sqlUser*, sqlChannel*, const string&);
#endif

} ;

}

#endif

/* LOGINCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"responses.h"
#include	"networkData.h"
#include	"cservice_config.h"
#include	"Network.h"
#include	"events.h"

const char LOGINCommand_cc_rcsId[] = "$Id: LOGINCommand.cc,v 1.16 2002-09-24 20:06:18 jeekay Exp $" ;

namespace gnuworld
{
struct autoOpData {
	string channel_name;
	unsigned int flags;
	unsigned int suspend_expires;
} aOp;

using std::ends;

bool LOGINCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.LOGIN");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return true;
	}


/*
 * Are we allowing logins yet?
 */
unsigned int loginTime = bot->getUplink()->getStartTime() + bot->loginDelay;
if(loginTime >= (unsigned int)bot->currentTime())
{
	bot->Notice(theClient, "AUTHENTICATION FAILED as %s. (Unable to login during reconnection, please try again in %i seconds)",
		st[1].c_str(), (loginTime - bot->currentTime()));
	return false;
}

/*
 * Check theClient isn't already logged in, if so, tell
 * them they shouldn't be.
 */

sqlUser* tmpUser = bot->isAuthed(theClient, false);
if (tmpUser)
	{
	bot->Notice(theClient,
		bot->getResponse(tmpUser, language::already_authed).c_str(),
		tmpUser->getUserName().c_str());
	bot->sendMOTD(theClient);
	return false;
	}

/*
 * Find the user record, confirm authorisation and attach the record
 * to this client.
 */

if(st[1][0] == '#')
{
	bot->Notice(theClient, "AUTHENTICATION FAILED as %s.", st[1].c_str());
	return false;
}

// TODO: Force a refresh of the user's info from the db
sqlUser* theUser = bot->getUserRecord(st[1]);
if( !theUser )
	{
	bot->Notice(theClient,
		bot->getResponse(tmpUser,
			language::not_registered,
			string("AUTHENTICATION FAILED as %s.")).c_str(),
		st[1].c_str());
	return false;
	}

/*
 * Check password
 */

if(!bot->isPasswordRight(theUser, st.assemble(2)))
	{
	bot->Notice(theClient, "AUTHENTICATION FAILED as %s.",
		theUser->getUserName().c_str());
	return false;
	}

/* Dont exceed MAXLOGINS */
if(theUser->networkClientList.size() >= theUser->getMaxLogins()) {
  bot->Notice(theClient, "AUTHENTICATION FAILED AS %s.",
    theUser->getUserName().c_str());
  return false;
}

if(theUser->isAuthed()) {
  bot->noticeAllAuthedClients(theUser, "%s has just authenticated as you (%s).",
    theClient->getNickUserHost().c_str(), theUser->getUserName().c_str());
}

string uname = theUser->getUserName();
server->PostEvent(gnuworld::EVT_LOGGEDIN
	,static_cast<void*>(theClient)
	    ,static_cast<void*>(&uname));
theUser->setLastSeen(bot->currentTime(), theClient->getNickUserHost());
theUser->setFlag(sqlUser::F_LOGGEDIN);
theUser->addAuthedClient(theClient);

networkData* newData =
	static_cast< networkData* >( theClient->getCustomData(bot) ) ;
if( NULL == newData )
	{
	bot->Notice( theClient,
		"Internal error." ) ;
	elog	<< "LOGINCommand> newData is NULL for: "
		<< theClient
		<< endl ;
	return false ;
	}

// Pointer back to the sqlUser from this iClient.
newData->currentUser = theUser;

bot->Notice(theClient,
	bot->getResponse(theUser, language::auth_success).c_str(),
	theUser->getUserName().c_str());

bot->sendMOTD(theClient);

/*
 * Send out an AC token to the network for this user. Format:
 * [Source Server] AC [Users Numeric] [Users name]
 * fx: AX AC APAFD jeekay
 */

stringstream ac;
ac << bot->getCharYY()
  << " AC "
  << theClient->getCharYYXXX()
  << " " << theUser->getUserName()
  << ends;
bot->Write(ac);
theClient->setAccount(theUser->getUserName());

/*
 * If the user account has been suspended, make sure they don't get
 * auto-opped.
 */

if (theUser->getFlag(sqlUser::F_GLOBAL_SUSPEND))
	{
	bot->Notice(theClient,
		"..however your account has been suspended by a CService administrator."
		" You will be unable to use any channel access you may have.");
	return true;
	}

/*
 * The fun part! For all channels this user has access on, and has
 * AUTOP set, and isn't already op'd on - do the deed.
 */

stringstream theQuery;
theQuery	<< "SELECT channels.name,levels.flags,levels.suspend_expires FROM "
			<< "levels,channels WHERE channels.id=levels.channel_id AND levels.user_id = "
			<< theUser->getID()
			<< ends;

#ifdef LOG_SQL
	elog	<< "LOGIN::sqlQuery> "
		<< theQuery.str().c_str()
		<< endl;
#endif

ExecStatusType status = bot->SQLDb->Exec(theQuery.str().c_str()) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "LOGIN> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return false ;
	}

typedef vector < autoOpData > autoOpVectorType;
autoOpVectorType autoOpVector;

for(int i = 0; i < bot->SQLDb->Tuples(); i++)
	{
		autoOpData current;

		current.channel_name = bot->SQLDb->GetValue(i, 0);
		current.flags = atoi(bot->SQLDb->GetValue(i, 1));
		current.suspend_expires = atoi(bot->SQLDb->GetValue(i, 2));

		autoOpVector.push_back( autoOpVectorType::value_type(current) );
		
	}

for (autoOpVectorType::const_iterator resultPtr = autoOpVector.begin();
	resultPtr != autoOpVector.end(); ++resultPtr)
	{

	/*
	 *  Would probably be wise to check they're not suspended too :)
	 *  (*smack* Ace)
	 *	GK@PAnet: Lets do this first to save time
	 */

	if(resultPtr->suspend_expires > 0) { continue; }

	/* If the auto(op/voice/invite) flag isn't set in this record */
	if (!(resultPtr->flags & sqlLevel::F_AUTOOP) &&
	#ifdef FEATURE_INVITE
		!(resultPtr->flags & sqlLevel::F_AUTOINVITE) &&
	#endif
		!(resultPtr->flags & sqlLevel::F_AUTOVOICE))
		{
		continue;
		}

	/*
	 * Is this channel registered?
	 */
	
	sqlChannel* theChan = bot->getChannelRecord(resultPtr->channel_name);
	if (!theChan) { continue; }

	/* 
   * Make sure the channel isn't suspended.. 
   */ 
    
	if (theChan->getFlag(sqlChannel::F_SUSPEND)) 
		{ continue; } 

	/*
	 * Check they aren't banned < 75 in the chan.
	 * GK@PAnet: This also stops autoinvite and autovoice.. bug?
	 */

	sqlBan* tmpBan = bot->isBannedOnChan(theChan, theClient);
	if( tmpBan && (tmpBan->getLevel() < 75) )
		{ continue; }

	/*
	 * Does the channel currently exist on the network?
	 */

	Channel* netChan = Network->findChannel(theChan->getName());
	if (!netChan) { continue; }
	
	/*
	 * Don't attempt to do anything if we're not in the channel, or not op'd.
	 */

	ChannelUser* tmpBotUser = netChan->findUser(bot->getInstance());
	if (!tmpBotUser) { continue; }

	if (!theChan->getInChan() || !tmpBotUser->getMode(ChannelUser::MODE_O))
		{ continue; }
	
	/*
	 * Attempt to find the user in the channel
	 */

	ChannelUser* tmpChanUser = netChan->findUser(theClient) ;
	
#ifdef FEATURE_INVITE
	/*
	 * If the user is not in the channel and they have the
	 * AUTOINVITE flag set, invite them!
	 */

	if(!tmpChanUser && (resultPtr->flags & sqlLevel::F_AUTOINVITE))
		{ bot->Invite(theClient, netChan->getName()); }
#endif
	
	/*
	 * If the user is not in the channel, we cant op/voice them
	 */
	
	if(!tmpChanUser) { continue; }

	/*
	 * Check if the channel is NOOP.
	 * N.B: If the channel is strictop, we op them.
	 * They've just logged in! :P
	 */

	if(theChan->getFlag(sqlChannel::F_NOOP))
		{	continue;	}

	/*
 	 *  If its AUTOOP, check for op's and do the deed.
	 *  Otherwise, its just AUTOVOICE :)
	 */

	if (resultPtr->flags & sqlLevel::F_AUTOOP)
		{
		if(!tmpChanUser->getMode(ChannelUser::MODE_O))
			{
			bot->Op(netChan, theClient);
			}
		}
		else if(resultPtr->flags & sqlLevel::F_AUTOVOICE)
		{
		if(!tmpChanUser->getMode(ChannelUser::MODE_V))
			{
			bot->Voice(netChan, theClient);
			}
		}

	}

#ifdef FEATURE_MEMOSERV
	/*
	 *  Now check if we have any notes
	 */
	
	stringstream notesQuery;
	notesQuery	<< "SELECT COUNT(id) FROM memo WHERE to_id = "
							<< theUser->getID() << ends;
#ifdef LOG_SQL
	elog	<< "LOGIN::sqlQuery> "
		<< notesQuery.str().c_str()
		<< endl;
#endif
	status = bot->SQLDb->Exec(notesQuery.str().c_str()) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "LOGIN> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return false ;
	}

if (bot->SQLDb->Tuples() > 0 && atoi(bot->SQLDb->GetValue(0, 0)) > 0)
	{
		bot->Notice(theClient, "You have %d note(s). To view them, type \002/msg %s note read\002.",
			atoi(bot->SQLDb->GetValue(0, 0)), bot->getNickName().c_str());
	}
#endif

/*
 *  And last but by no means least, see if we have been nominated as
 *  a supporter for a channel.
 */

stringstream supporterQuery;
supporterQuery	<< "SELECT channels.name FROM"
			<< " supporters,channels,pending WHERE"
			<< " supporters.channel_id = channels.id"
			<< " AND pending.channel_id = channels.id"
			<< " AND channels.registered_ts = 0"
			<< " AND supporters.support = '?'"
			<< " AND pending.status = 0"
			<< " AND user_id = "
			<< theUser->getID()
			<< ends;

#ifdef LOG_SQL
	elog	<< "LOGIN::sqlQuery> "
		<< supporterQuery.str().c_str()
		<< endl;
#endif

status = bot->SQLDb->Exec(supporterQuery.str().c_str()) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "LOGIN> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return false ;
	}


for(int i = 0; i < bot->SQLDb->Tuples(); i++)
	{
		string channelName = bot->SQLDb->GetValue(i, 0);
		string botName = bot->getNickName();
		bot->Notice(theClient, "You have been named as a supporter in a new channel application for %s. You may visit the website to register your support or to make an objection. Alternatively, you can type '\002/msg %s support %s YES\002' or '\002/msg %s support %s NO\002' to confirm or deny your support.", channelName.c_str(), botName.c_str(), channelName.c_str(), botName.c_str(), channelName.c_str());
	}

return true;
}

} // namespace gnuworld.


/*
 * UNSUSPENDCommand.cc
 *
 * 06/01/2001 - David Henriksen <david@itwebnet.dk>
 * Initial Version.
 *
 * Unsuspends a specified user on a channel.
 *
 * Caveats: None.
 *
 * $Id: UNSUSPENDCommand.cc,v 1.8 2004-05-16 15:20:22 jeekay Exp $
 */

#include	<string>

#include	"ELog.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlLevel.h"
#include	"sqlUser.h"

const char UNSUSPENDCommand_cc_rcsId[] = "$Id: UNSUSPENDCommand.cc,v 1.8 2004-05-16 15:20:22 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;
using namespace level;

void UNSUSPENDCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.UNSUSPEND");

StringTokenizer st( Message ) ;

if( st.size() < 2 )
	{
	Usage(theClient);
	return ;
	}

// Is the user authorized?

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser)
	{
	bot->Notice(theClient,
	bot->getResponse(theUser,
		language::no_longer_auth,
		string("Sorry, you are not authorised with me.")));
	return ;
	}

if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}


// Is the channel registered?

sqlChannel* theChan = bot->getChannelRecord(st[1]);

if(!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::chan_not_reg,
			string("Sorry, %s isn't registered with me.")).c_str(),
		st[1].c_str());
	return ;
	}

#ifdef FEATURE_FORCELOG
unsigned short forcedAccess = bot->isForced(theChan, theUser);
if (forcedAccess <= 900  && forcedAccess > 0)
        {
        bot->writeForceLog(theUser, theChan, Message);
        }

#endif

// Check level.
int level = bot->getEffectiveAccessLevel(theUser, theChan, true);

/*
 * check if we are unsuspending admins or normal users
 * cause unsuspending admins requires high access ;)
 */

if (theChan->getName() == "*")
        {
        sqlCommandLevel* susadminLevel = bot->getLevelRequired("SUSADMIN", "ADMIN");
        if (level < susadminLevel->getLevel())
                {
                bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
                return ;
                }
        }

if(level < level::unsuspend)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::insuf_access,
			string("Sorry, you have insufficient access to perform that command.")));
	return ;
	}

// Check whether the user is in the access list.
sqlUser* Target = bot->getUserRecord(st[2]);
if(!Target)
	{
	bot->Notice(theClient, "I don't know who %s is",
    	st[2].c_str());
	return ;
	}

sqlLevel* aLevel = bot->getLevelRecord(Target, theChan);
if(!aLevel)
	{
	bot->Notice(theClient,
	bot->getResponse(theUser,
		language::not_registered,
		string("I don't know who %s is")).c_str(),
    	Target->getUserName().c_str(), theChan->getName().c_str());
	return ;
	}

if (aLevel->getSuspendExpire() == 0)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::isnt_suspended,
			string("%s isn't suspended on %s")).c_str(),
		Target->getUserName().c_str(), theChan->getName().c_str());
	return ;
	}

/*
 *  Finally, check we have access to perform the unsuspend.
 */

if ((aLevel->getAccess()) >= level)
	{
	bot->Notice(theClient,
		"Cannot unsuspend a user with equal or higher access than your own.");
	return ;
	}

/*
 * Was this suspension set with a higher suspend level?
 */

if (aLevel->getSuspendLevel() > level)
	{
	bot->Notice(theClient,
		"Cannot unsuspend a user that was suspended at a higher level than your own access.");
	return ;
	}

aLevel->setSuspendExpire(0);
aLevel->setSuspendBy(string());
aLevel->setLastModif(bot->currentTime());
aLevel->setLastModifBy( string( "("
	+ theUser->getUserName()
	+ ") "
	+ theClient->getNickUserHost() ) );

if( !aLevel->commit() )
	{
	bot->Notice( theClient,
		"Error updating channel status." ) ;
	elog	<< "UNSUSPEND> SQL error"
		<< endl ;
	return ;
	}

bot->Notice(theClient,
	bot->getResponse(theUser,
		language::susp_cancelled,
		string("SUSPENSION for %s is cancelled")).c_str(),
	Target->getUserName().c_str());

return ;
}

} // namespace gnuworld.


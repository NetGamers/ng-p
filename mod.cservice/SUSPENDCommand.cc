/*
 * SUSPENDCommand.cc
 *
 * 06/01/2001 - David Henriksen <david@itwebnet.dk>
 * Initial Version, done with a fever :/
 *
 * Suspends an user on the specified channel, if suspend duration 0
 * is defined, the user will be unsuspended.
 *
 * $Id: SUSPENDCommand.cc,v 1.7 2004-05-16 13:08:17 jeekay Exp $
 */

#include	<string>

#include	<ctime>

#include	"ELog.h"
#include	"Network.h"
#include	"StringTokenizer.h"

#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"

const char SUSPENDCommand_cc_rcsId[] = "$Id: SUSPENDCommand.cc,v 1.7 2004-05-16 13:08:17 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;
using namespace level;

void SUSPENDCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.SUSPEND");

StringTokenizer st( Message ) ;
/* Is the user authenticated? */
sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser)
	{
	return ;
	}

if( st.size() < 4 )
	{
	Usage(theClient);
	return ;
	}

/* Is the channel registered? */
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if(!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::chan_not_reg,
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

/* Check level. */
int level = bot->getEffectiveAccessLevel(theUser, theChan, true);

/*
 * check if we are suspending admins or normal users
 * cause suspending admins requires high access ;)
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


if(level < level::suspend)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::insuf_access,
			string("Sorry, you have insufficient access to perform that command.")));
	return ;
	}

/* Check whether the user is in the access list. */
sqlUser* Target = bot->getUserRecord(st[2]);
if(!Target)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::not_registered,
			string("I don't know who %s is")).c_str(),
	    	st[2].c_str());
	return ;
	}

int usrLevel = bot->getAccessLevel(Target, theChan);
if(!usrLevel)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::doesnt_have_access,
			string("%s doesn't appear to have access in %s.")).c_str(),
	    	Target->getUserName().c_str(),
		theChan->getName().c_str());
	return ;
	}

if (level <= usrLevel)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::suspend_access_higher,
			string("Cannot suspend a user with equal or higher access than your own.")));
	return ;
	}

string units;
time_t duration = atoi(st[3].c_str());
time_t finalDuration = (duration * 60); /* Default to minutes. */

if( st.size() >= 5 )
	{
	units = string_lower(st[4]);
	if(units == "m")
		{
		finalDuration = duration * 60;
		}
	else if(units == "h")
		{
		finalDuration = duration * 60 * 60;
		}
	else if(units == "d")
		{
		finalDuration = duration * 60 * 60 * 24;
		}
	else
		{
		bot->Notice(theClient,
			bot->getResponse(theUser, language::bogus_time,
				string("bogus time units")));
		return ;
		}
	} /* if( st.size() >= 5 ) */

/* Greater than a year? */
if(finalDuration > 32140800 || finalDuration < 0)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::inval_suspend_dur,
			string("Invalid suspend duration.")));
	return ;
	}

sqlLevel* aLevel = bot->getLevelRecord(Target, theChan);

if( 0 == finalDuration )
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::susp_cancelled,
			string("SUSPENSION for %s is cancelled")).c_str(),
		Target->getUserName().c_str());

	aLevel->setSuspendExpire(finalDuration);
	aLevel->setSuspendBy( string() );
	aLevel->commit();

	return ;
	}

if (aLevel->getSuspendExpire() != 0)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::already_susp,
			string("%s is already suspended on %s")).c_str(),
		Target->getUserName().c_str(),
		theChan->getName().c_str());
	return ;
	}

/*
 * Check any access level supplied, default to our access
 * if not.
 */

int suspendLevel = level;
if( st.size() >= 6 )
	{
	suspendLevel = atoi(st[5].c_str());
	if ((suspendLevel > level) || (suspendLevel > 500) || (suspendLevel <= 0))
		suspendLevel = level;
	}

aLevel->setSuspendExpire(finalDuration + bot->currentTime());
aLevel->setSuspendBy(theClient->getNickUserHost());
aLevel->setSuspendLevel(suspendLevel);
aLevel->setLastModif(bot->currentTime());
aLevel->setLastModifBy( string( "("
	+ theUser->getUserName()
	+ ") "
	+ theClient->getNickUserHost() ) );

aLevel->commit();

bot->Notice(theClient,
	bot->getResponse(theUser,
		language::susp_set,
		string("SUSPENSION for %s will expire in %s")).c_str(),
	Target->getUserName().c_str(),
	bot->prettyDuration(bot->currentTime() - finalDuration ).c_str());

return ;
}

} // namespace gnuworld.

/*
 * 20020211 - GK@panet - Initial writing
 *
 * Allow global suspending of nicks/channels
 *
 * $Id: GSUSPENDCommand.cc,v 1.5 2002-07-20 15:45:06 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"
#include "cservice.h"
#include "levels.h"

const char GSUSPENDCommand_cc_rcsId[] = "$Id: GSUSPENDCommand.cc,v 1.5 2002-07-20 15:45:06 jeekay Exp $";

namespace gnuworld
{

bool GSUSPENDCommand::Exec( iClient* theClient, const string& Message )
{

bot->incStat("COMMANDS.GSUSPEND");

// /msg P gsuspend nick/chan duration reason

StringTokenizer st( Message );
if(st.size() < 4)
	{
	Usage(theClient);
	return true;
	}

// Are they logged in? If not, dont tell them about admin commands.

sqlUser* theUser = bot->isAuthed(theClient, false);
if(!theUser) { return false; }

int admLevel = bot->getAdminAccessLevel(theUser);

string target = st[1];

int duration;
string reason;

/* Find reason - time is optional so work around that */

if(IsNumeric(st[2]))
	{
	duration = atoi(st[2].c_str());
	reason = st.assemble(3);
	}
else
	{
	duration = 8736; // 1 year
	reason = st.assemble(2);
	}

int maxduration = 168;

/* Are we suspending a channel?
 * If so, need to check:
 *			Does the user have sufficient access?
 *			Does the channel exist?
 *			Is the channel suspended?
 */

if((target[0] == '#') && (admLevel >= level::csuspend))
	{ // We are suspending a channel
  /* Establish max allowable suspend */
  if(admLevel >= 600) maxduration = 72; // 3 days
  if(admLevel >= 650) maxduration = 672; // 1 month
  if(duration > maxduration) duration = maxduration;

	sqlChannel* theChan = bot->getChannelRecord(target);
	
	// Does the channel exist?
	if(!theChan)
		{
		bot->Notice(theClient, "Sorry, %s is not registered with me.", target.c_str());
		return false;
		}
	
	if(theChan->getFlag(sqlChannel::F_SUSPEND))
		{
		bot->Notice(theClient, "Sorry, %s is already suspended.", target.c_str());
		return false;
		}
	
	time_t suspendExpires = ::time(NULL) + (duration*3600);
	
	theChan->setSuspendExpires(suspendExpires);
	theChan->setFlag(sqlChannel::F_SUSPEND);
	theChan->commit();
	
	bot->writeChannelLog(theChan, theClient, sqlChannel::EV_SUSPEND, reason);
	
	bot->Notice(theClient, "%s has been suspended for %d hours.", target.c_str(), duration);
	
	bot->logAdminMessage("%s (%s) - GSUSPEND - %s for %d hours - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		theChan->getName().c_str(), duration, reason.c_str());
	
	return true;
	}

// We are suspending a nick
if(admLevel >= level::nsuspend)
	{
  /* Establish max allowable suspend */
  if(admLevel >= 700) maxduration = 72; // 3 days
  if(admLevel >= 750) maxduration = 672; // 1 month
  if(duration > maxduration) duration = maxduration;

	sqlUser* targetUser = bot->getUserRecord(target);
	if(!targetUser)
		{
		bot->Notice(theClient, "The user %s does not appear to be registered.", target.c_str());
		return true;
		}
	
	int targetLevel = bot->getAdminAccessLevel(targetUser);
	if( targetLevel >= admLevel)
		{
		bot->Notice(theClient, "Cannot suspend a user with equal or higher access to your own.");
		return false;
		}
	
	if(targetUser->getFlag(sqlUser::F_GLOBAL_SUSPEND))
		{
		bot->Notice(theClient, "%s is already globally suspended.", targetUser->getUserName().c_str());
		return true;
		}
	
	time_t suspendedExpire = ::time(NULL) + (duration*3600);
	targetUser->setSuspendedExpire(suspendedExpire);
	targetUser->setFlag(sqlUser::F_GLOBAL_SUSPEND);
	targetUser->commit();
	
	targetUser->writeEvent(sqlUser::EV_SUSPEND, theUser, reason);
	
	bot->Notice(theClient, "%s has been globally suspended for %d hours.", targetUser->getUserName().c_str(), duration);
	
	bot->logAdminMessage("%s (%s) - GSUSPEND - %s for %d hours - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		targetUser->getUserName().c_str(), duration, reason.c_str());
	
	return true;
	}

// We have managed to hit exactly nothing. The user does not have access.

bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");

return false;

} // GSUSPENDCommand::Exec

} // namespace gnuworld

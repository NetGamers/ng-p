/*
 * 20020211 - GK@panet - Initial writing
 *
 * Allow global unsuspending of nicks/channels
 *
 * $Id: GUNSUSPENDCommand.cc,v 1.5 2004-05-16 13:08:16 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"

#include "cservice.h"

const char GUNSUSPENDCommand_cc_rcsId[] = "$Id: GUNSUSPENDCommand.cc,v 1.5 2004-05-16 13:08:16 jeekay Exp $";

namespace gnuworld
{

void GUNSUSPENDCommand::Exec( iClient* theClient, const string& Message )
{

bot->incStat("COMMANDS.GUNSUSPEND");

// /msg P gunsuspend nick/chan

StringTokenizer st( Message );
if(st.size() < 3)
	{
	Usage(theClient);
	return ;
	}

// Are they logged in? If not, dont tell them about admin commands.

sqlUser* theUser = bot->isAuthed(theClient, false);
if(!theUser) { return ; }

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* channelSuspendLevel = bot->getLevelRequired("CSUSPEND", "ADMIN");
sqlCommandLevel* nickSuspendLevel = bot->getLevelRequired("NSUSPEND", "ADMIN");

string target = st[1];
string reason = st.assemble(2);

if((target[0] == '#') && (admLevel >= channelSuspendLevel->getLevel()))
	{ // We are unsuspending a channel
	
	sqlChannel* theChan = bot->getChannelRecord(st[1]);
	
	// Does the channel exist?
	if(!theChan)
		{
		bot->Notice(theClient, "Sorry, %s is not registered with me.", st[1].c_str());
		return ;
		}
	
	// Is the channel currently suspended?
	if(!theChan->getFlag(sqlChannel::F_SUSPEND))
		{
		bot->Notice(theClient, "%s is not currently suspended.", st[1].c_str());
		return ;
		}
	
	theChan->setSuspendExpires(0);
	theChan->removeFlag(sqlChannel::F_SUSPEND);
	theChan->commit();
	
	bot->writeChannelLog(theChan, theClient, sqlChannel::EV_UNSUSPEND, reason);
	
	bot->Notice(theClient, "%s has been unsuspended.", theChan->getName().c_str());
	bot->logAdminMessage("%s (%s) - GUNSUSPEND - %s - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		theChan->getName().c_str(),  reason.c_str());
	
	return ;
	} // Unsuspending a channel


/* We are unsuspending an nick
 * Things to check:
 *			Does the user have sufficient access?
 *			Is the user suspended?
 */

if(admLevel >= nickSuspendLevel->getLevel())
	{
	// Does the target user exist?
	sqlUser* targetUser = bot->getUserRecord(target);
	if(!targetUser)
		{
		bot->Notice(theClient, "The user %s does not appear to be registered.", target.c_str());
		return ;
		}
	
	// Is the target suspended?
	if(!targetUser->getFlag(sqlUser::F_GLOBAL_SUSPEND))
		{
		bot->Notice(theClient, "%s is not globally suspended.", target.c_str());
		return ;
		}
	
	targetUser->setSuspendedExpire(0);
	targetUser->removeFlag(sqlUser::F_GLOBAL_SUSPEND);
	targetUser->commit();
	
	bot->Notice(theClient, "%s has been globally unsuspended.", targetUser->getUserName().c_str());
	
	targetUser->writeEvent(sqlUser::EV_UNSUSPEND, theUser, reason);
	
	bot->logAdminMessage("%s (%s) - GUNSUSPEND - %s - %s",
		theClient->getNickName().c_str(), theUser->getUserName().c_str(),
		targetUser->getUserName().c_str(),  reason.c_str());
	
	return ;
	}

// We have managed to hit exactly nothing. The user does not have access.

bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");

return ;

} // GUNSUSPENDCommand::Exec

} // namespace gnuworld

/*
 * ADMINCMDS - Display admin commands
 *
 * (c) Copyright 2002 Rasmus Hansen (GK@panet)
 * Distributed under the GNU Public License
 *
 * $Id: ADMINCMDSCommand.cc,v 1.7 2002-03-24 02:32:03 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include  "levels.h"

const char ADMINCMDSCommand_cc_rcsId[] = "$Id: ADMINCMDSCommand.cc,v 1.7 2002-03-24 02:32:03 jeekay Exp $" ;

namespace gnuworld
{
using std::string ;

bool ADMINCMDSCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.ADMINCMDS");

StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return true;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return false; }

int aLevel = bot->getAdminAccessLevel(theUser);

if(!aLevel)
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
	return false;
	}

if(aLevel >= 950)
	{
	bot->Notice(theClient, "\002Level  950 - CSC Coder");
	bot->Notice(theClient, "QUOTE");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 900)
	{
	bot->Notice(theClient, "\002Level  900 - CSC Manager");
	bot->Notice(theClient, "REHASH [help|translations]");
	bot->Notice(theClient, "SAY (chan) (message)");
	bot->Notice(theClient, "SERVNOTICE (message)");
	bot->Notice(theClient, "SET (chan) NOFORCE (on/off)");
	bot->Notice(theClient, "SHUTDOWN (message)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 850)
	{
	bot->Notice(theClient, "\002Level  850 - CSC Director");
	bot->Notice(theClient, "GLOBNOTICE (message)");
	bot->Notice(theClient, "ADDUSER * (nick) (level)");
	bot->Notice(theClient, "REMUSER * (nick)");
	bot->Notice(theClient, "MODINFO * ACCESS (nick) (level)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 800)
	{
	bot->Notice(theClient, "\002Level  800 - CSC Supervisor");
	bot->Notice(theClient, "SUSPEND * (nick) (duration) (reason)");
	bot->Notice(theClient, "UNSUSPEND * (nick) (reason)");
	bot->Notice(theClient, "SET (chan) CAUTION (on/off)");
	bot->Notice(theClient, "SET (chan) NEVERREG (on/off)");
	bot->Notice(theClient, "SET (chan) NOPURGE (on/off)");
	bot->Notice(theClient, "SET (chan) NOREG (on/off)");
	bot->Notice(theClient, "SET (chan) SPECIAL (on/off)");
	bot->Notice(theClient, "SET (chan) TEMPMAN (on/off)");
	bot->Notice(theClient, "SET (chan) VACATION (on/off)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 750)
	{
	bot->Notice(theClient, "\002Level  750 - CSC Senior Administrator");
	bot->Notice(theClient, "CHINFO (email|nick|verification) (nick) (newdata)");
	bot->Notice(theClient, "COMMENT (nick) (comment)");
	bot->Notice(theClient, "GSUSPEND (nick) (duration) (reason)");
	bot->Notice(theClient, "GUNSUSPEND (nick) (reason)");
	bot->Notice(theClient, "REMUSERID (nick) (reason)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 700)
	{
	bot->Notice(theClient, "\002Level  700 - CSC Senior Administrator");
	bot->Notice(theClient, "GSUSPEND (chan) (duration) (reason)");
	bot->Notice(theClient, "GUNSUSPEND (chan) (reason)");
	bot->Notice(theClient, "PURGE (chan) (reason)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 650)
	{
	bot->Notice(theClient, "\002Level  650 - CSC Junior Administrator");
	bot->Notice(theClient, "COMMENT (chan) (comment)");
	bot->Notice(theClient, "REGISTER (chan) (nick)");
	bot->Notice(theClient, "SET (chan) LOCKED (on/off)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 600)
	{
	bot->Notice(theClient, "\002Level  600 - CSC Junior Administrator");
	bot->Notice(theClient, "INVME");
	bot->Notice(theClient, "REMIGNORE (hostmask)");
	bot->Notice(theClient, "REMOVEALL (chan)");
	bot->Notice(theClient, "SCAN [email|hostmask|nick] (string)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 1)
	{
	bot->Notice(theClient, "\002Level    1 - CSC Junior Administrator");
	bot->Notice(theClient, "ADMINCMDS");
	bot->Notice(theClient, "FORCE (chan)");
	bot->Notice(theClient, "UNFORCE (chan)");
	bot->Notice(theClient, "INFO (nick)");
	bot->Notice(theClient, "STATS *");
	bot->Notice(theClient, "STATUS *");
	}

return true ;
} // ADMINCMDSCommand::Exec

} // namespace gnuworld.

/*
 * ADMINCMDS - Display admin commands
 *
 * (c) Copyright 2002 Rasmus Hansen (GK@panet)
 * Distributed under the GNU Public License
 *
 * $Id: ADMINCMDSCommand.cc,v 1.21 2004-08-25 20:32:29 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include  "levels.h"


namespace gnuworld
{
using std::string ;

void ADMINCMDSCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.ADMINCMDS");

StringTokenizer st( Message ) ;
if( st.size() != 1 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return ; }

int aLevel = bot->getAdminAccessLevel(theUser);

if(!aLevel)
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command.");
	return ;
	}

if(aLevel >= 950)
	{
	bot->Notice(theClient, "\002Level  950 - CSC Coder");
	bot->Notice(theClient, "QUOTE");
	bot->Notice(theClient, "SHUTDOWN (message)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 900)
	{
	bot->Notice(theClient, "\002Level  900 - CSC Manager");
	bot->Notice(theClient, "REHASH [commands|help|official|translations]");
	bot->Notice(theClient, "SAY (chan) (message)");
	bot->Notice(theClient, "SERVNOTICE (message)");
	bot->Notice(theClient, "SET (chan) NOFORCE (on/off)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 850)
	{
	bot->Notice(theClient, "\002Level  850 - CSC Director");
	bot->Notice(theClient, "ADDUSER * (nick) (level)");
	bot->Notice(theClient, "REMUSER * (nick)");
	bot->Notice(theClient, "MODINFO * ACCESS (nick) (level)");
	bot->Notice(theClient, "SET (chan) NOPURGE (on/off)");
	bot->Notice(theClient, "SET (chan) SPECIAL (on/off)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 800)
	{
	bot->Notice(theClient, "\002Level  800 - CSC Supervisor");
	bot->Notice(theClient, "GLOBNOTICE (message)");
	bot->Notice(theClient, "OFFICIAL (CHECK <user>) (CLEAR <user>) (LIST) (SET <user> <level>");
	bot->Notice(theClient, "SUSPEND * (nick) (duration) (reason)");
	bot->Notice(theClient, "UNSUSPEND * (nick) (reason)");
	bot->Notice(theClient, "\002");

	}

if(aLevel >= 750)
	{
	bot->Notice(theClient, "\002Level  750 - CSC Senior Administrator");
	bot->Notice(theClient, "CHINFO [email|nick|verification] (nick) (newdata)");
	bot->Notice(theClient, "GSUSPEND (nick) (duration[extended]) (reason)");
	bot->Notice(theClient, "GUNSUSPEND (nick) (reason)");
	bot->Notice(theClient, "REMUSERID (nick) (reason)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 700)
	{
	bot->Notice(theClient, "\002Level  700 - CSC Senior Administrator");
	bot->Notice(theClient, "GSUSPEND (nick) (duration[limited]) (reason)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 650)
	{
	bot->Notice(theClient, "\002Level  650 - CSC Junior Administrator");
	bot->Notice(theClient, "GSUSPEND (chan) (duration[extended]) (reason)");
	bot->Notice(theClient, "PURGE (chan) (reason)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 600)
	{
	bot->Notice(theClient, "\002Level  600 - CSC Junior Administrator");
	bot->Notice(theClient, "GSUSPEND (chan) (duration[limited]) (reason)");
	bot->Notice(theClient, "GUNSUSPEND (chan) (reason)");
	bot->Notice(theClient, "PURGE FORCE (chan) (reason)");
	bot->Notice(theClient, "REGISTER (chan) (nick)");
	bot->Notice(theClient, "REMOVEALL (chan)");
	bot->Notice(theClient, "SCAN [email|hostmask|nick] (string)");
	bot->Notice(theClient, "\002");
	}

if(aLevel >= 550) {
  bot->Notice(theClient, "\002Level  550 - CSC Junior Administrator");
  bot->Notice(theClient, "SET (chan) TEMPMAN (on/off)");
  bot->Notice(theClient, "\002");
}

if(aLevel >= 450) {
  bot->Notice(theClient, "\002Level  450 - CSC Junior Administrator");
  bot->Notice(theClient, "SET (chan) LOCKED (on/off)");
  bot->Notice(theClient, "\002");
}

if(aLevel >= 400) {
  bot->Notice(theClient, "\002Level  400 - CSC Junior Administrator");
  bot->Notice(theClient, "FORCE (chan)");
  bot->Notice(theClient, "UNFORCE (chan)");
  bot->Notice(theClient, "\002");
}

if(aLevel >= 100) {
  bot->Notice(theClient, "\002Level  100 - Senior Helper");
  bot->Notice(theClient, "REMIGNORE (hostmask)");
  bot->Notice(theClient, "\002");
}

if(aLevel >= 1) {
  bot->Notice(theClient, "\002Level    1 - Senior Helper");
  bot->Notice(theClient, "ADMINCMDS");
  bot->Notice(theClient, "COMMENT (chan) (comment)");
  bot->Notice(theClient, "COMMENT (nick) (comment)");
  bot->Notice(theClient, "INFO (nick)");
  bot->Notice(theClient, "INVME");
  bot->Notice(theClient, "STATS *");
  bot->Notice(theClient, "STATUS *");
}

return ;
} // ADMINCMDSCommand::Exec

} // namespace gnuworld.

/*
 * MODECommand.cc
 *
 * 20020201 - Jeekay - Initial Version
 *
 * $Id: MODECommand.cc,v 1.12 2004-08-25 20:32:46 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"
#include "cservice.h"
#include "Network.h"
#include "levels.h"
#include "ELog.h"


namespace gnuworld
{


void MODECommand::Exec( iClient* theClient, const string& Message )
{

bot->incStat("COMMANDS.MODE");

StringTokenizer st( Message );

if( st.size() < 3)
	{
	Usage(theClient);
	return ;
	}

// Are we authed?
sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return ; }

Channel* theChan = Network->findChannel(st[1]);
if(!theChan)
	{
	bot->Notice(theClient, "Channel %s not found.", st[1].c_str());
	return ;
	}

// Does the channel exist?
sqlChannel* sqlChan = bot->getChannelRecord(st[1]);
if(!sqlChan)
	{
	bot->Notice(theClient, "The channel %s does not appear to be registered.", st[1].c_str());
	return ;
	} // if(!theChan)

#ifdef FEATURE_FORCELOG
unsigned short forcedAccess = bot->isForced(sqlChan, theUser);
if(forcedAccess <= 900 && forcedAccess > 0)
	{
	bot->writeForceLog(theUser, sqlChan, Message);
	}
#endif

int level = bot->getEffectiveAccessLevel(theUser, sqlChan, true);
if(level < level::mode)
	{
	bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
	return ;
	} // if(level < level::mode)


/* Check we are actually opped first */
ChannelUser *tmpBotUser = theChan->findUser(bot->getInstance());
if(!tmpBotUser || !tmpBotUser->getMode(ChannelUser::MODE_O)) {
	bot->Notice(theClient, "I'm not opped in %s.",
		theChan->getName().c_str()
		);
	return ;
}

/*************************************
 * P A R S I N G   D O N E   H E R E *
 *************************************/
 
#define CF_C	0x0001
#define CF_S	0x0002
#define CF_c	0x0004
#define CF_i	0x0008
#define CF_m	0x0010
#define CF_n	0x0020
#define CF_p	0x0040
#define CF_r	0x0080
#define CF_s	0x0100
#define CF_t	0x0200
#define CF_T	0x0400
#define CF_l	0x1000
#define CF_k	0x2000

string modeString = st[2];
unsigned short maxarg = st.size() - 1;
unsigned short curarg = 3;
bool polarity = true;
unsigned short positive = 0;
unsigned short negative = 0;
unsigned short curFlag;

string posargs, negargs;

for( string::const_iterator itr = modeString.begin() ;
     itr != modeString.end() ; ++itr) {
	curFlag = 0;
	switch( *itr ) {
		case '+' : polarity = true;	break;
		case '-' : polarity = false;	break;
		case 'C' : curFlag = CF_C;	break;
		case 'S' : curFlag = CF_S;	break;
		case 'T' : curFlag = CF_T;	break;
		case 'c' : curFlag = CF_c;	break;
		case 'i' : curFlag = CF_i;	break;
		case 'm' : curFlag = CF_m;	break;
		case 'n' : curFlag = CF_n;	break;
		case 'p' : curFlag = CF_p;	break;
		case 'r' : curFlag = CF_r;	break;
		case 's' : curFlag = CF_s;	break;
		case 't' : curFlag = CF_t;	break;
		case 'k' : { if((positive & CF_k) || (negative & CF_k)) break;
			if(curarg <= maxarg) {
				if(polarity) {
					/* We are setting +k */
					if(theChan->getMode(Channel::MODE_k)) {
						/* The channel is +k */
						bot->Notice(theClient, "You cannot set a new key without removing the old one.");
						return ;
					} else {
						/* The channel is not +k */
						if(!posargs.empty()) posargs += " ";
						posargs += st[curarg];
						theChan->setKey(st[curarg]);
						++curarg;
						curFlag = CF_k;
					}
				} else {
					/* We are removing +k */
					if(theChan->getMode(Channel::MODE_k)) {
						if(theChan->getKey() != st[curarg]) {
							bot->Notice(theClient, "To remove a key you must specify the current one.");
							return ;
						} else {
							/* Keys match */
							if(!negargs.empty()) negargs += " ";
							negargs += st[curarg];
							theChan->setKey("");
							++curarg;
							curFlag = CF_k;
						}
					} else {
						bot->Notice(theClient, "You cannot dekey a channel that has no key.");
						return ;
					}
				}
			} else {
				bot->Notice(theClient, "Setting +k requires a key to be specified.");
				return ;
			}
			break;
		}
		case 'l' : { if((positive & CF_l) || (negative & CF_l)) break;
			if(polarity) {
				/* Setting the limit requires an argument */
				if(curarg <= maxarg) {
					unsigned int limit = atoi(st[curarg].c_str());
					if(limit > 0) {
						curFlag = CF_l;
						posargs += st[curarg];
						theChan->setLimit(limit);
						++curarg;
					} else {
						bot->Notice(theClient, "Please specify a limit above zero.");
						return ;
					}
				} else {
					bot->Notice(theClient, "Setting +l requires a limit to be specified.");
					return ;
				}
			} else {
				/* Limit can be unset without an argument */
				if(theChan->getMode(Channel::MODE_l)) {
					curFlag = CF_l;
				} else {
					bot->Notice(theClient, "You cannot remove a nonexistant limit.");
					return ;
				}
			}
			break;
		}
	} // switch( *itr )
	
	if(polarity) positive |= curFlag;
	else negative |= curFlag;
} // iterate over modeString

/* Check for overlap between positive and negative */
if((positive & negative) != 0) {
	bot->Notice(theClient, "You cannot both set and remove a mode.");
	return ;
}

/* Check for no modes */
if(!positive && !negative) {
	bot->Notice(theClient, "Actually setting some modes would be a good idea.");
	return ;
}

/* Check for mutually exclusive modes */
if((positive & CF_p) && (positive & CF_s)) {
	bot->Notice(theClient, "You cannot set +p and +s at the same time.");
	return ;
}

if((positive & CF_S) && (positive & CF_c)) {
	bot->Notice(theClient, "You cannot set +S and +c at the same time.");
	return ;
}

if(positive & CF_S) negative |= CF_c;
if(positive & CF_c) negative |= CF_S;

if(positive & CF_p) negative |= CF_s;
if(positive & CF_s) negative |= CF_p;

string posString, negString;

if(positive & CF_C) { posString += "C"; theChan->setMode(Channel::MODE_C); }
if(positive & CF_S) { posString += "S"; theChan->setMode(Channel::MODE_S); }
if(positive & CF_T) { posString += "T"; theChan->setMode(Channel::MODE_T); }
if(positive & CF_c) { posString += "c"; theChan->setMode(Channel::MODE_c); }
if(positive & CF_i) { posString += "i"; theChan->setMode(Channel::MODE_i); }
if(positive & CF_m) { posString += "m"; theChan->setMode(Channel::MODE_m); }
if(positive & CF_n) { posString += "n"; theChan->setMode(Channel::MODE_n); }
if(positive & CF_p) { posString += "p"; theChan->setMode(Channel::MODE_p); }
if(positive & CF_r) { posString += "r"; theChan->setMode(Channel::MODE_r); }
if(positive & CF_s) { posString += "s"; theChan->setMode(Channel::MODE_s); }
if(positive & CF_t) { posString += "t"; theChan->setMode(Channel::MODE_t); }
if(positive & CF_k) { posString += "k"; theChan->setMode(Channel::MODE_k); }
if(positive & CF_l) { posString += "l"; theChan->setMode(Channel::MODE_l); }

if(negative & CF_C) { negString += "C"; theChan->removeMode(Channel::MODE_C); }
if(negative & CF_S) { negString += "S"; theChan->removeMode(Channel::MODE_S); }
if(negative & CF_T) { negString += "T"; theChan->removeMode(Channel::MODE_T); }
if(negative & CF_c) { negString += "c"; theChan->removeMode(Channel::MODE_c); }
if(negative & CF_i) { negString += "i"; theChan->removeMode(Channel::MODE_i); }
if(negative & CF_m) { negString += "m"; theChan->removeMode(Channel::MODE_m); }
if(negative & CF_n) { negString += "n"; theChan->removeMode(Channel::MODE_n); }
if(negative & CF_p) { negString += "p"; theChan->removeMode(Channel::MODE_p); }
if(negative & CF_r) { negString += "r"; theChan->removeMode(Channel::MODE_r); }
if(negative & CF_s) { negString += "s"; theChan->removeMode(Channel::MODE_s); }
if(negative & CF_t) { negString += "t"; theChan->removeMode(Channel::MODE_t); }
if(negative & CF_k) { negString += "k"; theChan->removeMode(Channel::MODE_k); }
if(negative & CF_l) { negString += "l"; theChan->removeMode(Channel::MODE_l); }


string outString;
if(!posString.empty()) outString += string("+") + posString;
if(!negString.empty()) outString += string("-") + negString;

if(!posargs.empty()) outString += string(" ") + posargs;
if(!negargs.empty()) outString += string(" ") + negargs;

bot->Write("%s M %s %s",
	bot->getCharYYXXX().c_str(),
	st[1].c_str(),
	outString.c_str());

bot->Notice(theClient, "Changing modes on %s to: %s.",
	theChan->getName().c_str(),
	outString.c_str());

return ;
} // MODECommand::Exec
 
} // namespace gnuworld

/*
 * MODECommand.cc
 *
 * 20020201 - Jeekay - Initial Version
 *
 * $Id: MODECommand.cc,v 1.7 2003-03-30 02:54:08 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"
#include "cservice.h"
#include "Network.h"
#include "levels.h"
#include "ELog.h"

#define CF_I  0x01
#define CF_M  0x02
#define CF_N  0x04
#define CF_SP 0x08
#define CF_T  0x10
#define CF_L  0x20
#define CF_K  0x40
#define CF_CStrip 0x80

const char MODECommand_cc_rcsId[] = "$Id: MODECommand.cc,v 1.7 2003-03-30 02:54:08 jeekay Exp $";

namespace gnuworld
{

using std::ends;

bool MODECommand::Exec( iClient* theClient, const string& Message )
{

bot->incStat("COMMANDS.MODE");

StringTokenizer st( Message );

if( st.size() < 3)
	{
	Usage(theClient);
	return true;
	}

// Are we authed?
sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser) { return false; }

Channel* theChan = Network->findChannel(st[1]);
if(!theChan)
	{
	bot->Notice(theClient, "Channel %s not found.", st[1].c_str());
	return false;
	}

// Does the channel exist?
sqlChannel* sqlChan = bot->getChannelRecord(st[1]);
if(!sqlChan)
	{
	bot->Notice(theClient, "The channel %s does not appear to be registered.", st[1].c_str());
	return false;
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
	return false;
	} // if(level < level::mode)

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
#define CF_l	0x1000
#define CF_k	0x2000

string modeString = st[2];
bool polarity = true;
unsigned int positive = 0;
unsigned int negative = 0;
unsigned int curFlag;



for( string::const_iterator itr = modeString.begin() ;
     itr != modeString.end() ; ++itr) {
	curFlag = 0;
	switch( *itr ) {
		case '+' : polarity = true;	break;
		case '-' : polarity = false;	break;
		case 'C' : curFlag = CF_C;	break;
		case 'S' : curFlag = CF_S;	break;
		case 'c' : curFlag = CF_c;	break;
		case 'i' : curFlag = CF_i;	break;
		case 'm' : curFlag = CF_m;	break;
		case 'n' : curFlag = CF_n;	break;
		case 'p' : curFlag = CF_p;	break;
		case 'r' : curFlag = CF_r;	break;
		case 's' : curFlag = CF_s;	break;
		case 't' : curFlag = CF_t;	break;
	} // switch( *itr )
	
	if(polarity) positive |= curFlag;
	else negative |= curFlag;
} // iterate over modeString

/* Check for overlap between positive and negative */
if((positive & negative) != 0) {
	bot->Notice(theClient, "You cannot both set and remove a mode.");
	return true;
}

/* Check for no modes */
if(!positive && !negative) {
	bot->Notice(theClient, "Actually setting some modes would be a good idea.");
	return true;
}

/* Check for mutually exclusive modes */
if((positive & CF_p) && (positive & CF_s)) {
	bot->Notice(theClient, "You cannot set +p and +s at the same time.");
	return true;
}

if((positive & CF_S) && (positive & CF_c)) {
	bot->Notice(theClient, "You cannot set +S and +c at the same time.");
	return true;
}

if(positive & CF_S) negative |= CF_c;
if(positive & CF_c) negative |= CF_S;

if(positive & CF_p) negative |= CF_s;
if(positive & CF_s) negative |= CF_p;

string posString, negString;

if(positive & CF_C) { posString += "C"; theChan->setMode(Channel::MODE_C); }
if(positive & CF_S) { posString += "S"; theChan->setMode(Channel::MODE_S); }
if(positive & CF_c) { posString += "c"; theChan->setMode(Channel::MODE_c); }
if(positive & CF_i) { posString += "i"; theChan->setMode(Channel::MODE_i); }
if(positive & CF_m) { posString += "m"; theChan->setMode(Channel::MODE_m); }
if(positive & CF_n) { posString += "n"; theChan->setMode(Channel::MODE_n); }
if(positive & CF_p) { posString += "p"; theChan->setMode(Channel::MODE_p); }
if(positive & CF_r) { posString += "r"; theChan->setMode(Channel::MODE_r); }
if(positive & CF_s) { posString += "s"; theChan->setMode(Channel::MODE_s); }
if(positive & CF_t) { posString += "t"; theChan->setMode(Channel::MODE_t); }

if(negative & CF_C) { negString += "C"; theChan->removeMode(Channel::MODE_C); }
if(negative & CF_S) { negString += "S"; theChan->removeMode(Channel::MODE_S); }
if(negative & CF_c) { negString += "c"; theChan->removeMode(Channel::MODE_c); }
if(negative & CF_i) { negString += "i"; theChan->removeMode(Channel::MODE_i); }
if(negative & CF_m) { negString += "m"; theChan->removeMode(Channel::MODE_m); }
if(negative & CF_n) { negString += "n"; theChan->removeMode(Channel::MODE_n); }
if(negative & CF_p) { negString += "p"; theChan->removeMode(Channel::MODE_p); }
if(negative & CF_r) { negString += "r"; theChan->removeMode(Channel::MODE_r); }
if(negative & CF_s) { negString += "s"; theChan->removeMode(Channel::MODE_s); }
if(negative & CF_t) { negString += "t"; theChan->removeMode(Channel::MODE_t); }

string outString = "+";
outString += posString;
outString += "-";
outString += negString;

bot->Write("%s M %s %s",
	bot->getCharYYXXX().c_str(),
	st[1].c_str(),
	outString.c_str());

return true;
} // MODECommand::Exec
 
} // namespace gnuworld

/*
 * MODECommand.cc
 *
 * 20020201 - Jeekay - Initial Version
 *
 * $Id: MODECommand.cc,v 1.4 2002-07-01 00:33:05 jeekay Exp $
 */

#include <string>

#include "StringTokenizer.h"
#include "cservice.h"
#include "Network.h"
#include "levels.h"
#include "ELog.h"

#define CF_I 0x01
#define CF_M 0x02
#define CF_N 0x04
#define CF_SP 0x08
#define CF_T 0x10
#define CF_L 0x20
#define CF_K 0x40

const char MODECommand_cc_rcsId[] = "$Id: MODECommand.cc,v 1.4 2002-07-01 00:33:05 jeekay Exp $";

namespace gnuworld
{

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

unsigned int curFlags = 0;
int numArgs = static_cast< int >(st.size()) - 1; // Do we have any +k/+l arguments?
int curArg = 3;
bool polarity = true, chanSet; // True == +, false == -
string modeString = st[2];
string positive, negative, posarg, negarg;
string outString;

for(string::iterator ptr = modeString.begin(); ptr != modeString.end(); ++ptr)
	{
	switch( *ptr )
		{
		case 'i':
			{
			if(curFlags & CF_I) { break; }
			chanSet = theChan->getMode(Channel::MODE_I);
			if(chanSet && !polarity)
				{
				negative += "i";
				theChan->removeMode(Channel::MODE_I);
				} // chanSet && !polarity
			if(!chanSet && polarity)
				{
				positive += "i";
				theChan->setMode(Channel::MODE_I);
				} // !chanSet && polarity
			curFlags |= CF_I;
			break;
			} // case i
		case 'm':
			{
			if(curFlags & CF_M) { break; }
			chanSet = theChan->getMode(Channel::MODE_M);
			if(chanSet && !polarity)
				{
				negative += "m";
				theChan->removeMode(Channel::MODE_M);
				} // chanSet && !polarity
			if(!chanSet && polarity)
				{
				positive += "m";
				theChan->setMode(Channel::MODE_M);
				} // !chanSet && polarity
			curFlags |= CF_M;
			break;
			} // case m
		case 'n':
			{
			if(curFlags & CF_N) { break; }
			chanSet = theChan->getMode(Channel::MODE_N);
			if(chanSet && !polarity)
				{
				negative += "n";
				theChan->removeMode(Channel::MODE_N);
				} // chanSet && !polarity
			if(!chanSet && polarity)
				{
				positive += "n";
				theChan->setMode(Channel::MODE_N);
				} // !chanSet && polarity
			curFlags |= CF_N;
			break;
			} // case n
		case 'p':
			{
			if(curFlags & CF_SP) { break; }
			chanSet = theChan->getMode(Channel::MODE_P);
			bool chanSetS = theChan->getMode(Channel::MODE_S);
			if(chanSet && !polarity)
				{
				negative += "p";
				theChan->removeMode(Channel::MODE_P);
				} // chanSet && !polarity
			if(!chanSet && polarity)
				{
				positive += "p";
				theChan->setMode(Channel::MODE_P);
				if(chanSetS)
					{ // S and P are mutually exclusive
					negative += "s";
					theChan->removeMode(Channel::MODE_S);
					}
				} // !chanSet && polarity
			curFlags |= CF_SP;
			break;
			} // case p
		case 's':
			{
			if(curFlags & CF_SP) { break; }
			chanSet = theChan->getMode(Channel::MODE_S);
			bool chanSetP = theChan->getMode(Channel::MODE_P);
			if(chanSet && !polarity)
				{
				negative += "s";
				theChan->removeMode(Channel::MODE_S);
				} // chanSet && !polarity
			if(!chanSet && polarity)
				{
				positive += "s";
				theChan->setMode(Channel::MODE_S);
				if(chanSetP)
					{ // S and P are mutually exclusive
					negative += "p";
					theChan->removeMode(Channel::MODE_P);
					}
				} // !chanSet && polarity
			curFlags |= CF_SP;
			break;
			} // case s
		case 't':
			{
			if(curFlags & CF_T) { break; }
			chanSet = theChan->getMode(Channel::MODE_T);
			if(chanSet && !polarity)
				{
				negative += "t";
				theChan->removeMode(Channel::MODE_T);
				} // chanSet && !polarity
			if(!chanSet && polarity)
				{
				positive += "t";
				theChan->setMode(Channel::MODE_T);
				} // !chanSet && polarity
			curFlags |= CF_T;
			break;
			} // case t
		case 'l':
			{
			if(curFlags & CF_L) { break; }
			chanSet = theChan->getMode(Channel::MODE_L);
			if(chanSet && !polarity)
				{
				negative += "l";
				theChan->removeMode(Channel::MODE_L);
				}
			if(polarity && (numArgs >= curArg) && IsNumeric(st[curArg]) && (st[curArg].size() <= 9))
				{
				positive += "l";
				posarg += st[curArg] + string(" ");
				theChan->setMode(Channel::MODE_L);
				theChan->setLimit(atoi(st[curArg].c_str()));
				++curArg;
				}
			curFlags |= CF_L;
			break;
			} // case l
		case 'k':
			{
			if(curFlags & CF_K) { break; }
			chanSet = theChan->getMode(Channel::MODE_K);
			if(chanSet && !polarity && (numArgs >= curArg) && (theChan->getKey() == st[curArg]))
				{
				negative += "k";
				negarg = st[curArg];
				theChan->removeMode(Channel::MODE_K);
				++curArg;
				} // Unset +k
			if(!chanSet && polarity && (numArgs >= curArg) && (st[curArg].size() <= 20))
				{
				positive += "k";
				posarg += st[curArg] + string(" ");
				theChan->setMode(Channel::MODE_K);
				theChan->setKey(st[curArg]);
				++curArg;
				}
			curFlags |= CF_K;
			break;
			} // case k
		case '+':
			{
			if(!polarity) {	polarity = true; }
			break;
			} // +
		case '-':
			{
			if(polarity) { polarity = false; }
			break;
			}
		} // switch(*ptr)
	} // for(iterate over modestring)

if(positive.size()) outString += string("+") + positive;
if(negative.size()) outString += string("-") + negative;
if(outString.size() && posarg.size()) outString += string(" ") + posarg;
if(outString.size() && negarg.size()) outString += string(" ") + negarg;

if(outString.size())
	{
	stringstream quoteStream;
	quoteStream << bot->getCharYYXXX() << " M " << st[1] << " " << outString << ends;
	bot->Write(quoteStream.str().c_str());
	}

return true;

} //MODECommand::Exec

} // namespace gnuworld

/*
 * OPERSUSPENDCommand.cc
 *
 *
 * Suspends a channel for 12 hours, can be removed only by cservice admin
 *
 * $Id: OPERSUSPENDCommand.cc,v 1.1 2002-01-14 23:14:19 morpheus Exp $
 */

#include	<string>

#include	<ctime>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"Network.h"
#include	"levels.h"
#include	"responses.h"

const char OPERSUSPENDCommand_cc_rcsId[] = "$Id: OPERSUSPENDCommand.cc,v 1.1 2002-01-14 23:14:19 morpheus Exp $" ;

namespace gnuworld
{
using std::string ;
using namespace level;

bool OPERSUSPENDCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.OPERSUSPEND");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return true;
	}

/* Is the user an oper? */

if(!theClient->isOper())
	{
	bot->Notice(theClient,"This command is reserved to IRC Operators");
	return true;
	}

if (st[1][0] != '#')
{
	bot->Notice(theClient,"You can only suspend Channels");
	return true;
}


/* Is the channel registered? */
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if(!theChan)
	{
	bot->Notice(theClient,"Sorry, %s isn't registered with me.",
		st[1].c_str());
	return false;
	}

/* Check level. */
theChan->setFlag(sqlChannel::F_SUSPEND);
theChan->commit();
bot->Notice(theClient,"Channel %s been suspended",st[1].c_str());
bot->logAdminMessage("%s has asked me to suspend %s",theClient->getNickUserHost().c_str()
			,st[1].c_str());

return true ;
}

} // namespace gnuworld.

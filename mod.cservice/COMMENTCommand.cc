/* COMMENTCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"levels.h"
#include	"responses.h"

const char COMMENTCommand_cc_rcsId[] = "$Id: COMMENTCommand.cc,v 1.1 2002-01-23 01:12:53 ultimate Exp $" ;

namespace gnuworld
{
using std::string ;

bool COMMENTCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.COMMENT");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return true;
	}

sqlUser* theUser = bot->isAuthed(theClient, false);
if (!theUser)
	{
	bot->Notice(theClient, "Sorry, You must be logged in to use this command.");
	return false;
	}

int admLevel = bot->getAdminAccessLevel(theUser);
if (admLevel < level::comment) return false;

/*
 *  Check if we wanna comment a channel or a user!
 */

if(st[1][0] == '#') // we HAVE a channel!!
{

	sqlChannel* targetChan = bot->getChannelRecord(st[1]);
	if(!targetChan)
        	{
	        bot->Notice(theClient,
        	        bot->getResponse(theUser,
                	        language::chan_not_reg).c_str(),
	                st[1].c_str()
        	);
	        return false;
        	}

	if(strlen(st.assemble(2).c_str()) > 230)
		{
		bot->Notice(theClient, "COMMENT can only be 230 chars long!");
		return false;
		}

	if(string_upper(st[2]) == "OFF")
		{
		targetChan->setComment("");
		targetChan->commit();
		bot->Notice(theClient, "COMMENT for channel %s has been removed!", st[1].c_str());
		}
	else
		{
                targetChan->setComment(st.assemble(2));
                targetChan->commit();
		bot->Notice(theClient, "COMMENT for channel %s is now: %s", st[1].c_str(),
			st.assemble(2).c_str()); 
		}
return true;
}
	
/*
 *  Check the person we're trying to add is actually registered.
 */

sqlUser* targetUser = bot->getUserRecord(st[1]);
if (!targetUser)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::not_registered).c_str(),
		st[1].c_str()
	);
	return false;
	}

if(strlen(st.assemble(2).c_str()) > 230)
	{
	bot->Notice(theClient, "COMMENT can only be 230 chars long!");
	return false;
	}


if (string_upper(st[2]) == "OFF")
{
        targetUser->setComment("");
	targetUser->commit();
	bot->Notice(theClient, "COMMENT for user %s has been removed!", 
		targetUser->getUserName().c_str());
} else
{
	string comment = st.assemble(2) + " (by " + theUser->getUserName() + ")";
	targetUser->setComment(comment);
        targetUser->commit();
	bot->Notice(theClient, "COMMENT for user %s set to: %s", targetUser->getUserName().c_str(),
			st.assemble(2).c_str());
}

return true ;
}

} // namespace gnuworld.

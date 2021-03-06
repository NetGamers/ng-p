/* 
 * COMMENTCommand.cc 
 * 23/1/2001 Matthias Crauwels <ultimate_@wol.be
 * 
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"

#include	"cservice.h"
#include	"responses.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{
using std::string ;

void COMMENTCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.COMMENT");

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, false);
if (!theUser)
	{
	bot->Notice(theClient, "Sorry, You must be logged in to use this command.");
	return ;
	}

int admLevel = bot->getAdminAccessLevel(theUser);
sqlCommandLevel* chanCommentLevel = bot->getLevelRequired("CHANCOMMENT", "ADMIN");
sqlCommandLevel* userCommentLevel = bot->getLevelRequired("USERCOMMENT", "ADMIN");

/*
 *  Check if we wanna comment a channel or a user!
 */

if(st[1][0] == '#') // we HAVE a channel!!
{
	if (admLevel < chanCommentLevel->getLevel()) return ;

	sqlChannel* targetChan = bot->getChannelRecord(st[1]);
	if(!targetChan)
        	{
	        bot->Notice(theClient,
        	        bot->getResponse(theUser,
                	        language::chan_not_reg).c_str(),
	                st[1].c_str()
        	);
	        return ;
        	}

	if(strlen(st.assemble(2).c_str()) > 230)
		{
		bot->Notice(theClient, "COMMENT can only be 230 chars long!");
		return ;
		}

	if(string_upper(st[2]) == "OFF")
		{
		targetChan->setComment("");
		targetChan->commit();
		bot->Notice(theClient, "COMMENT for channel %s has been removed!", st[1].c_str());
		bot->logAdminMessage("%s (%s) - COMMENT - %s - <cleared>",
			theClient->getNickName().c_str(), theUser->getUserName().c_str(),
			targetChan->getName().c_str());
    bot->writeChannelLog(targetChan, theClient, sqlChannel::EV_COMMENT, "Comment removed");
		}
	else
		{
		string myComment = "(" + theUser->getUserName() + ") " + st.assemble(2);
		targetChan->setComment(myComment);
		targetChan->commit();
		bot->Notice(theClient, "COMMENT for channel %s is now: %s", st[1].c_str(),
			myComment.c_str()); 
		bot->logAdminMessage("%s (%s) - COMMENT - %s - %s",
			theClient->getNickName().c_str(), theUser->getUserName().c_str(),
			targetChan->getName().c_str(), myComment.c_str());
    bot->writeChannelLog(targetChan, theClient, sqlChannel::EV_COMMENT, st.assemble(2));
		}
return ;
}
	
/*
 *  Check the person we're trying to add is actually registered.
 */

if (admLevel < userCommentLevel->getLevel()) return ;

sqlUser* targetUser = bot->getUserRecord(st[1]);
if (!targetUser)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::not_registered).c_str(),
		st[1].c_str()
	);
	return ;
	}

if(strlen(st.assemble(2).c_str()) > 230)
	{
	bot->Notice(theClient, "COMMENT can only be 230 chars long!");
	return ;
	}


if (string_upper(st[2]) == "OFF") {
  targetUser->setComment("");
  targetUser->commit();
  bot->Notice(theClient, "COMMENT for user %s has been removed!", 
    targetUser->getUserName().c_str());
  
  // Make sure the rest of the world knows about it
  bot->logAdminMessage("%s (%s) - COMMENT - %s - <cleared>",
    theClient->getNickName().c_str(), theUser->getUserName().c_str(),
    targetUser->getUserName().c_str());
  
  // Log a user event to this effect
  targetUser->writeEvent(sqlUser::EV_COMMENT, theUser, "Comment Removed");
} else {
  string comment = "(" + theUser->getUserName() + ") " + st.assemble(2);
  targetUser->setComment(comment);
  targetUser->commit();
  bot->Notice(theClient, "COMMENT for user %s set to: %s", targetUser->getUserName().c_str(),
    st.assemble(2).c_str());
  
  // Make sure the rest of the world knows about it
  bot->logAdminMessage("%s (%s) - COMMENT - %s - %s",
    theClient->getNickName().c_str(), theUser->getUserName().c_str(),
    targetUser->getUserName().c_str(), st.assemble(2).c_str());
  
  // Log a user event to this effect
  targetUser->writeEvent(sqlUser::EV_COMMENT, theUser, st.assemble(2));
}

return ;
}

} // namespace gnuworld.

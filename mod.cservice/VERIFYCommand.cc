/* VERIFYCommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"cservice.h" 
#include	"Network.h"
#include	"levels.h"
#include	"responses.h"

const char VERIFYCommand_cc_rcsId[] = "$Id: VERIFYCommand.cc,v 1.18 2004-05-16 12:45:42 jeekay Exp $" ;

namespace gnuworld
{

using std::string ;
 
bool VERIFYCommand::Exec( iClient* theClient, const string& Message )
{ 
StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return true;
	} 
 
string extra;

iClient* target = Network->findNick(st[1]); 

if(!target)
	{
	bot->Notice(theClient, "Sorry, I don't see %s anywhere.",
		st[1].c_str());
	return true;
	}
        
if (target->getMode(iClient::MODE_SERVICES))
	{
	bot->Notice(theClient, "%s is a NetGamers Service Bot.",
		target->getNickName().c_str()
		);
	return true;
	}

/* 
 *  Firstly, deal with unauthenticated users.
 */

sqlUser* theUser = bot->isAuthed(target, false);

if (target->isOper())
	{
	extra = " and an IRC operator";
	}

if (!theUser) 
	{
	if(target->isOper())
		{ 
		bot->Notice(theClient, "%s is an IRC operator and is NOT logged in.",
			target->getNickUserHost().c_str()
			);
		return true;
		}
	else
		{
		bot->Notice(theClient, "%s is NOT logged in.",
			target->getNickUserHost().c_str()
			);
		}
		return true;
	}

int oLevel = theUser->getVerify();
if(bot->getVerify(oLevel).empty()) oLevel = 0;

int level = bot->getAdminAccessLevel(theUser); 

if ( (0 == level) && (0 == oLevel) ) 
	{
	bot->Notice(theClient, "%s is logged in as %s%s",
		target->getNickUserHost().c_str(),
		theUser->getUserName().c_str(),
		extra.c_str());
	return true;
	}

if ((level >= level::admin::base) && (level < level::admin::cadmin)) 
	{
		bot->Notice(theClient,"%s is a CSC Senior Helper%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::cadmin) && (level < level::admin::nadmin)) 
	{
		bot->Notice(theClient,"%s is a CSC Channel Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if((level >= level::admin::nadmin) && (level < level::admin::director))
	{
	bot->Notice(theClient, "%s is a CSC Nick Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
  return true;
	}

if ((level >= level::admin::director) && (level < level::admin::amanager)) 
	{
	bot->Notice(theClient,"%s is a CSC Director%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::amanager) && (level < level::admin::manager))
	{
	bot->Notice(theClient,"%s is a CSC Assistant Manager%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if (level == level::admin::manager)
	{
	bot->Notice(theClient, "%s is a CSC Manager%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if (level >= level::admin::coder)
	{
	bot->Notice(theClient, "%s is a CSC Developer%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if(oLevel)
	{
	bot->Notice(theClient, "%s is %s%s and logged in as %s",
		target->getNickUserHost().c_str(), bot->getVerify(oLevel).c_str(),
		extra.c_str(), theUser->getUserName().c_str());
	return true;
	}

bot->logErrorMessage("ERROR: Impossible situation in VERIFY reached.");

return true ;
}


} // namespace gnuworld.

/* VERIFYCommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"cservice.h" 
#include	"Network.h"
#include	"levels.h"
#include	"responses.h"

const char VERIFYCommand_cc_rcsId[] = "$Id: VERIFYCommand.cc,v 1.10 2002-02-26 13:42:05 morpheus Exp $" ;

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

sqlUser* tmpUser = bot->isAuthed(theClient, false);
iClient* target = Network->findNick(st[1]); 
if(!target)
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::dont_see_them,
			string("Sorry, I don't see %s anywhere.")).c_str(), st[1].c_str());
	return false;
	}
        
if (target->getMode(iClient::MODE_SERVICES))
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::is_service_bot,
			string("%s is an Official Planetarion Service Bot.")).c_str(), 
		target->getNickName().c_str());
	return false;
	}

/* 
 *  Firstly, deal with unauthenticated users.
 */

sqlUser* theUser = bot->isAuthed(target, false);

if (target->isOper())
	{
	extra = bot->getResponse(tmpUser,
		language::is_also_an_ircop, " and an IRC operator");
	}

if (!theUser) 
	{
	if(target->isOper())
		{ 
		bot->Notice(theClient, 
			bot->getResponse(tmpUser,
				language::is_an_ircop,
				string("%s is an IRC operator")).c_str(), 
			target->getNickUserHost().c_str());
		}
	else
		{
		bot->Notice(theClient, 
			bot->getResponse(tmpUser,
			language::is_not_logged_in,
			string("%s is NOT logged in.")).c_str(), 
		target->getNickUserHost().c_str());
		}
		return false;
	}

sqlChannel* theChan = bot->getChannelRecord("#coder-com");
if (!theChan) elog << "Cannot find the #coder-com channel!" << endl;

sqlChannel* officialChan = bot->getChannelRecord("#official");
if (!officialChan) elog << "Cannot find #official channel!" << endl;


int level = bot->getAdminAccessLevel(theUser); 

int cLevel;
if (!theChan)	cLevel = 0;
else cLevel = bot->getEffectiveAccessLevel(theUser, theChan, false);

int oLevel;
if(!officialChan) oLevel = 0;
else oLevel = bot->getEffectiveAccessLevel(theUser, officialChan, false);

if ( (0 == level) && (0 == cLevel) && (0 == oLevel) ) 
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::logged_in_as,
			string("%s is logged in as %s%s")).c_str(), 
		target->getNickUserHost().c_str(),
		theUser->getUserName().c_str(),
		extra.c_str());
	return false;
	}

if ((level >= level::admin::base) && (level < level::admin::supervisor)) 
	{
		bot->Notice(theClient,"%s is a CSC Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::supervisor) && (level < level::admin::director)) 
	{
		bot->Notice(theClient,"%s is a CSC Supervisor%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if((level >= level::admin::director) && (level < level::admin::amanager))
	{
	bot->Notice(theClient, "%s is a CSC Director%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	}

if ((level >= level::admin::amanager) && (level < level::admin::manager)) 
	{
		bot->Notice(theClient,"%s is the CSC Assistant Manager%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if (level == level::admin::manager)
	{
		bot->Notice(theClient,"%s is the CSC Manager%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level > level::admin::manager) && (level <= level::admin::coder))
	{
	bot->Notice(theClient, "%s is a CSC Developer%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((cLevel >= level::coder::base) && (cLevel <= level::coder::contrib))
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::is_coder_rep,
			string("%s is an Official Coder-Com Representative%s and logged in as %s")).c_str(),
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((cLevel > level::coder::base) && (cLevel <= level::coder::contrib))
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::is_coder_contrib,
			string("%s is an Official Coder-Com Contributer%s and logged in as %s")).c_str(),
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((cLevel > level::coder::contrib) && (cLevel <= level::coder::devel))
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::is_coder_devel,
			string("%s is an Official Coder-Com Developer%s and logged in as %s")).c_str(),
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if (cLevel > level::coder::devel)
	{
	bot->Notice(theClient, 
		bot->getResponse(tmpUser,
			language::is_coder_senior,
			string("%s is an Official Coder-Com Senior%s and logged in as %s")).c_str(),
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

// #official replies
//  1 = Official Planetarion Bot
//  2 = Official Planetarion Virusfix Member
//  3 = Official Planetarion PATeam Member
//  4 = Official Planetarion PACrew Member
// 50 = Official CSC Helper
// 99 = Planetarion Creator

if(1 == oLevel)
	{
	bot->Notice(theClient, "%s is an Official Planetarion Bot%s and logged in as %s", target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}

if(2 == oLevel)
	{
	bot->Notice(theClient, "%s is an Official Planetarion Virusfix Member%s and logged in as %s", target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}

if(3 == oLevel)
	{
	bot->Notice(theClient, "%s is an Official Planetarion PATeam Member%s and logged in as %s", target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}

if(4 == oLevel)
	{
	bot->Notice(theClient, "%s is an Official Planetarion PACrew Member%s and logged in as %s", target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}

if(50 == oLevel)
	{
	bot->Notice(theClient, "%s is an Official CSC Helper%s and logged in as %s", target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}

if(99 == oLevel)
	{
	bot->Notice(theClient, "%s is an Official Planetarion Game Creator%s and logged in as %s", target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}
	
return true ;
}


} // namespace gnuworld.

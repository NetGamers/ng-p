/* VERIFYCommand.cc */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"cservice.h" 
#include	"Network.h"
#include	"levels.h"
#include	"responses.h"

const char VERIFYCommand_cc_rcsId[] = "$Id: VERIFYCommand.cc,v 1.3 2002-01-23 22:07:25 jeekay Exp $" ;

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
if (!theChan) 
	{
	elog << "Cannot find the #coder-com channel!" << endl;
	return true;
	}

sqlChannel* virusChan = bot->getChannelRecord("#virusfix");
if (!virusChan)
        {
        elog << "Cannot find the #virusfix channel!" << endl;
        return true;
        }


// TODO: Move all the levels to constants in levels.h

int level = bot->getAdminAccessLevel(theUser); 
int cLevel;
if (!theChan)
	cLevel = 0;
else
	cLevel = bot->getEffectiveAccessLevel(theUser, theChan, false);

int vLevel;
if (!virusChan)
        vLevel = 0;
else
        vLevel = bot->getEffectiveAccessLevel(theUser, virusChan, false);


if ( (0 == level) && (0 == cLevel) && (level::virusfix::base > vLevel) ) 
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

if ((level >= level::admin::base) && (level <= level::admin::jradmin)) 
	{
	//bot->Notice(theClient, 
	//	bot->getResponse(tmpUser,
	//		language::is_cservice_rep,
	//		string("%s is a Junior CSC Channel Administrator%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is a Junior CSC Channel Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level > level::admin::jradmin) && (level < level::admin::sradmin)) 
	{
	//bot->Notice(theClient, 
	//	bot->getResponse(tmpUser,
	//		language::is_cservice_admin,
	//		string("%s is a Senior CSC Channel Administrator%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is a Senior CSC Channel Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::sradmin) && (level < level::admin::jrweb)) 
	{
	//bot->Notice(theClient, 
	//	bot->getResponse(tmpUser,
	//		language::is_cservice_sradmin,
	//		string("%s is a Junior CSC Web Administrator%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is a Junior CSC Web Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::jrweb) && (level < level::admin::srweb)) 
	{
	//bot->Notice(theClient, 
	//	bot->getResponse(tmpUser,
	//		language::is_cservice_sradmin,
	//		string("%s is a Senior CSC Web Administrator%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is a Senior CSC Web Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::srweb) && (level < level::admin::supervisor)) 
	{
	//bot->Notice(theClient, 
	//	bot->getResponse(tmpUser,
	//		language::is_cservice_sradmin
		//	string("%s is a CSC Supervising Administrator%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is a CSC Supervising Administrator%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level >= level::admin::supervisor) && (level < level::admin::manager)) 
	{
	//bot->Notice(theClient, 
		//bot->getResponse(tmpUser,
		//	language::is_cservice_sradmin,
		//	string("%s is a CSC Personnel Director%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is a CSC Personnel Director%s and logged in as %s"
		,target->getNickUserHost().c_str()
		,extra.c_str()
		,theUser->getUserName().c_str());
	return true;
	}

if (level == level::admin::manager) 
	{
	//bot->Notice(theClient, 
		//bot->getResponse(tmpUser,
		//	language::is_cservice_sradmin,
		//	string("%s is the CSC Manager and Director%s and logged in as %s")).c_str(), 
		bot->Notice(theClient,"%s is the CSC Manager and Director%s and logged in as %s",
		target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if (level == level::admin::webcoder) 
	{
	bot->Notice(theClient, "%s is an Official CService Web Designer%s and logged in as %s",target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if (level == level::admin::manager)
	{
	bot->Notice(theClient, "%s is an Official CService Manager%s and logged in as %s",target->getNickUserHost().c_str(),
		extra.c_str(),
		theUser->getUserName().c_str());
	return true;
	}

if ((level > level::admin::manager) && (level <= level::admin::coder))
        {
        bot->Notice(theClient,
                bot->getResponse(tmpUser,
                        language::is_cservice_dev,
                        string("%s is an Official CService Developer%s and logged in as %s")).c_str(),
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


/*
 * #virusfix verify replies
 *
 * [02:02] <Kheldar> have it verify like:
 * [02:04] <Kheldar> 17:03 -P- Icewatcher!Icewatcher@89dyn165.com21.casema.net is an Official Virusfix
 * Member and logged in as Icewatcher
 */

if(vLevel => level::virusfix::base)
	{
	bot->Notice(theClient, "%s is an Official Virusfix Member%s and logged in as %s",
		target->getNickUserHost().c_str(), extra.c_str(), theUser->getUserName().c_str());
	return true;
	}


return true ;
}


} // namespace gnuworld.

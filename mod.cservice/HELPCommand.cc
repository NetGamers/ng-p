/*
 * HELPCommand.cc
 *
 * 18/02/2001 - David Henriksen <david@itwebnet.dk>
 * Initial Version.
 *
 * Outputs channel service help.
 *
 * Caveats: Needs to be written :)
 *
 * $Id: HELPCommand.cc,v 1.1 2002-01-14 23:14:17 morpheus Exp $
 */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 
#include	"cservice.h" 
#include 	"responses.h"

const char HELPCommand_cc_rcsId[] = "$Id: HELPCommand.cc,v 1.1 2002-01-14 23:14:17 morpheus Exp $" ;

namespace gnuworld
{

using namespace gnuworld;

const char* helpHeader = "\026 Planetarion(TM) IRC's Channel Service                   Version 2 \026";
 
bool HELPCommand::Exec( iClient* theClient, const string& Message )
{ 
	bot->incStat("COMMANDS.HELP");
	StringTokenizer st( Message ) ;
	if( st.size() > 2 )
	{
		Usage(theClient);
		return true;
	}

	if( st.size() < 2 )
	{
		bot->Notice(theClient, helpHeader);
		bot->Notice(theClient, " ");
		bot->Notice(theClient, "P is Planetarion(tm) IRC's channel service which is provided");
		bot->Notice(theClient, "to the channel operators and users. On the channels registered");
		bot->Notice(theClient, "with P, it can administrate channel modes, give channel ops or");
		bot->Notice(theClient, "voice to authorised users, maintain access restrictions - banlists,");
		bot->Notice(theClient, "channel topics, and various other options. It is also able to");
		bot->Notice(theClient, "sit on or off the channel at the descresion of channel managers.");
		bot->Notice(theClient, " ");
		bot->Notice(theClient, "The Planetarion(tm) Administration or the author(s) of this service");
		bot->Notice(theClient, "are \002not\002 responsible for the contents nor the events that occur");
		bot->Notice(theClient, "within channels registered with P.");
		bot->Notice(theClient, " ");
		bot->Notice(theClient, "To register your nick or channel, goto: \002http://cservice.irc.planetarion.com\002");
		bot->Notice(theClient, "If you require assistance or wish to ledge a formal complaint, you");
		bot->Notice(theClient, "can type \002/join #cservice\002 and speak with a cservice representive.");
		bot->Notice(theClient, "To check whether a nick is an official cservice representive, you can");
		bot->Notice(theClient, "type \002/msg P verify <nick>\002.");
		bot->Notice(theClient, " ");
		bot->Notice(theClient, "Information on upgrades, maintenance and other important things about this");
		bot->Notice(theClient, "service can be viewed with \002/msg P MOTD\002. This will show ");
		bot->Notice(theClient, "P's Message Of The Day. It would be wise to check it regularly.");
		bot->Notice(theClient, " ");
		bot->Notice(theClient, "For a command list of available commands, type \002/msg P showcommands\002");
		bot->Notice(theClient, "or \002/msg P showcommands #channel\002 for a specific channel.");
		bot->Notice(theClient, "To get help about a certain command, type \002/msg P help <command>\002");
		bot->Notice(theClient, "You can view a command list at \002http://cservice.irc.planetarion.com\002");
		return true;
	}		
		
	sqlUser* theUser = bot->isAuthed(theClient, false);
	string msg = bot->getHelpMessage(theUser, string_upper(st.assemble(1)));

	if (msg.empty())
	msg = bot->getHelpMessage(theUser, "INDEX");

	if (!msg.empty())
		bot->Notice(theClient, msg);
	else
		bot->Notice(theClient, "There is no help available for that topic.");

	return true ;
} 

} // namespace gnuworld.

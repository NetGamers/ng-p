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
 * $Id: HELPCommand.cc,v 1.3 2002-09-14 14:45:52 jeekay Exp $
 */

#include	<string>
 
#include	"StringTokenizer.h"
#include	"ELog.h" 
#include	"cservice.h" 
#include 	"responses.h"

const char HELPCommand_cc_rcsId[] = "$Id: HELPCommand.cc,v 1.3 2002-09-14 14:45:52 jeekay Exp $" ;

namespace gnuworld
{

const char* helpHeader = "\002NetGamers IRC Channel Services - Version 3\002";
 
bool HELPCommand::Exec( iClient* theClient, const string& Message )
{ 
	bot->incStat("COMMANDS.HELP");
	StringTokenizer st( Message ) ;
	if( st.size() > 2 )
	{
		Usage(theClient);
		return true;
	}

  if( st.size() < 2 ) {
    bot->Notice(theClient, helpHeader);
    bot->Notice(theClient, " ");
    bot->Notice(theClient, "P is Netgamers IRC channel service which is provided to the channel");
    bot->Notice(theClient, "operators and users. On the channels registered with P, it can");
    bot->Notice(theClient, "administrate channel modes, give channel ops or voice to authorised");
    bot->Notice(theClient, "users, maintain access restrictions - banlists, channel topics, and");
    bot->Notice(theClient, "various other options. It is also able to sit on or off the channel at");
    bot->Notice(theClient, "the discretion of channel managers.");
    bot->Notice(theClient, " ");
    bot->Notice(theClient, "The Netgamers Administration or the author(s) of this service are not");
    bot->Notice(theClient, "responsible for the contents nor the events that occur within channels");
    bot->Notice(theClient, "registered with P.");
    bot->Notice(theClient, " ");
    bot->Notice(theClient, "To register your nick or channel, goto: \002http://www.netgamers.org/\002");
    bot->Notice(theClient, "If you require assistance or wish to ledge a formal complaint, you can");
    bot->Notice(theClient, "type \002/join #cservice\002 and speak with a CService Administrator. To check");
    bot->Notice(theClient, "whether a nick is an official CService Administrator, you can type:");
    bot->Notice(theClient, "\002/msg P verify <nick>\002");
    bot->Notice(theClient, " ");
    bot->Notice(theClient, "Information on important things about this service can be viewed with");
    bot->Notice(theClient, "/msg P MOTD. This will show P's Message of The Day.");
    bot->Notice(theClient, "It would be wise to check it regularly.");
    bot->Notice(theClient, " ");
    bot->Notice(theClient, "For a command list of available commands, type \002/msg P showcommands\002 or");
    bot->Notice(theClient, "\002/msg P showcommands #Example\002 for a specific channel.");
    bot->Notice(theClient, "To get help about a certain command, type \002/msg P help <command>\002");
    bot->Notice(theClient, "You can view a full command list at \002http://www.netgamers.org/\002");
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

/*
 * 20020207 - GK@panet - Initial creation
 *
 * $Id: RELEASECommand.cc,v 1.1 2002-02-08 00:24:34 jeekay Exp $
 *
 */
 
 #include <string>
 #include "StringTokenizer.h"
 #include "nickserv.h"

const char RELEASECommand_cc_rcsId[] = "$Id: RELEASECommand.cc,v 1.1 2002-02-08 00:24:34 jeekay Exp $";

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool RELEASECommand::Exec( iClient* theClient, const string& Message )
{

StringTokenizer st( Message );
if(st.size() != 2)
	{
	Usage(theClient);
	return true;
	}

nsUser* theUser = static_cast< nsUser* >( theClient->getCustomData(bot));

/*
 * Is the user logged in as the nick he is trying to release?
 */

if(!(string_lower(theUser->getLoggedNick()) == string_lower(st[1])))
	{
	bot->Notice(theClient, "You must be logged in as the nick you are trying to release.");
	return false;
	}

/*
 * Is there an existing jupe for the requested nick?
 */

juUser* theJupe = bot->findJupeNick(st[1]);
if(!theJupe)
	{
	bot->Notice(theClient, "There is no active jupe for %s.", st[1].c_str());
	return false;
	}

/*
 * Jupe exists and user is logged in as the juped nick
 */

if(bot->removeJupeNick(st[1], "Jupe Released"))
	{
	bot->Notice(theClient, "Jupe for %s successfully released.", st[1].c_str());
	return true;
	}
else
	{
	/*
	 * We should never ever hit this code as by now, we know that
	 * this nick is juped and the user is logged in as that nick
	 */
	 
	bot->Notice(theClient, "Unable to remove jupe for %s.", st[1].c_str());
	return false;
	}

return false;

} // RELEASECommand::Exec

} // namespace nserv

} // namespace gnuworld

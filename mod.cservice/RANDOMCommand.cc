/* RANDOMCommand.cc */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"

const char RANDOMCommand_cc_rcsId[] = "$Id: RANDOMCommand.cc,v 1.1 2002-01-14 23:14:20 morpheus Exp $" ;

namespace gnuworld
{
using std::string ;

bool RANDOMCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.RANDOM");

StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return true;
	}

return true ;
}

} // namespace gnuworld.

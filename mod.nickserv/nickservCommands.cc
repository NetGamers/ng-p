/* nickservCommands.cc
 */

#include	<string>
#include	<cstdlib>

#include	"nickserv.h"
#include	"nickservCommands.h"
#include	"misc.h"

namespace gnuworld
{


using std::string ;

namespace nserv
{

void Command::Usage( iClient* theClient )
{
bot->Notice( theClient, string( "Usage: " ) + ' ' + getInfo() ) ;
}

}
} // namespace gnuworld

#include        <string>

#include        "cservice.h"

namespace gnuworld
{

using std::string;
using namespace gnuworld;
 
bool SETTIMECommand::Exec( iClient* theClient, const string& Message )
{ 

if(!theClient->isOper())
        {
        bot->Notice(theClient, "This command is reserved to IRC Operators");
        return true;
        }
								
stringstream s;
s	<< server->getCharYY() << " SE " << time(NULL) << ":" << server->getName()
	<< ends;

bot->Write( s ) ; 

return true;
} 

} // namespace gnuworld.


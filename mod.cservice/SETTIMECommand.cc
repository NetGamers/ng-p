#include        <string>

#include        "cservice.h"

namespace gnuworld
{

using std::string;
 
void SETTIMECommand::Exec( iClient* theClient, const string& Message )
{ 

if(!theClient->isOper())
        {
        bot->Notice(theClient, "This command is reserved to IRC Operators");
        return ;
        }
								
stringstream s;
s	<< server->getCharYY()
	<< " SE " 
	<< time(NULL) 
	<< ":" 
	<< server->getName()
	;

bot->Write( s ) ; 

return ;
} 

} // namespace gnuworld.


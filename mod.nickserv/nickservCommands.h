#ifndef __NICKSERVCOMMANDS_H
#define __NICKSERVCOMMANDS_H "$Id: nickservCommands.h,v 1.7 2002-02-08 00:24:35 jeekay Exp $"

#include	<string>
#include	"iClient.h"

using std::string ;

namespace gnuworld
{

namespace nserv
{

using gnuworld::xServer;

class nickserv;
//class xServer;

class Command
{

public:
        Command( nickserv* _bot, const string& _commName,
                const string& _help)
         : bot( _bot ),
           server( 0 ),
           commName( _commName ),
           help( _help )
        {}
        virtual ~Command() {}

        /// Exec returns true if the command was successfully
        /// executed, false otherwise.
        virtual bool Exec( iClient*, const string& ) = 0 ;

        void    setServer( xServer* _server )
                { server = _server ; }
        virtual string getInfo() const
                { return commName + ' ' + help ; }
        virtual void Usage( iClient* theClient ) ;

        inline const string& getName() const
                { return commName ; }
        inline const string& getHelp() const
                { return help ; }

protected:
        nickserv*       bot ;
        xServer*        server ;
        string          commName ;
        string          help ;

} ;

#define DECLARE_COMMAND(commName) \
class commName##Command : public Command \
{ \
public: \
        commName##Command( nickserv* _bot, \
                const string& _commName, \
                const string& _help) \
        : Command( _bot, _commName, _help) \
        {} \
        virtual bool Exec( iClient*, const string& ) ; \
        virtual ~commName##Command() {} \
} ;

DECLARE_COMMAND( JUPE );
DECLARE_COMMAND( RELEASE );
DECLARE_COMMAND( STATS );
DECLARE_COMMAND( SAY );

} // namespace gnuworld

}

#endif // __NICKSERVCOMMANDS_H


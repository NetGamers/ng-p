/**
 * msg_T.cc
 * Copyright (C) 2002 Daniel Karrels <dan@karrels.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#include	<iostream>

#include	"server.h"
#include	"xparameters.h"
#include	"Network.h"
#include	"ELog.h"
#include	"config.h"
#include	"Channel.h"
#include	"ServerCommandHandler.h"

namespace gnuworld
{

using std::endl ;

CREATE_HANDLER(msg_T)

// Channel topics currently are not tracked.
// kAI T #omniplex :-=[ Washington.DC.US.Krushnet.Org / Luxembourg.
// LU.EU.KrushNet.Org
// Admin Channel ]=-
bool msg_T::Execute( const xParameters& Param )
{
if( Param.size() < 3 )
	{
	elog	<< "msg_T> Invalid number of arguments"
		<< endl ;
	return false ;
	}

#ifdef TOPIC_TRACK

Channel* Chan = Network->findChannel( Param[ 1 ] ) ;
if( 0 == Chan )
	{
	elog	<< "msg_T> Unable to locate channel: "
		<< Param[ 1 ]
		<< endl;
	return false ;
	}

Chan->setTopic( Param[ 2 ] ) ;

#endif // TOPIC_TRACK

return true ;
}

} // namespace gnuworld

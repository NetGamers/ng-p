/**
 * msg_Error.cc
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
 *
 * $Id: msg_Error.cc,v 1.3 2002-07-27 14:54:13 jeekay Exp $
 */

#include	"server.h"
#include	"xparameters.h"
#include	"ServerCommandHandler.h"

const char server_h_rcsId[] = __SERVER_H ;
const char xparameters_h_rcsId[] = __XPARAMETERS_H ;
const char msg_Error_cc_rcsId[] = "$Id: msg_Error.cc,v 1.3 2002-07-27 14:54:13 jeekay Exp $" ;

namespace gnuworld
{

CREATE_HANDLER(msg_Error)

bool msg_Error::Execute( const xParameters& )
{
return true ;
}


} // namespace gnuworld

/**
 * msg_DS.cc
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
 * $Id: msg_DS.cc,v 1.2 2002-07-01 00:28:27 jeekay Exp $
 */

#include	"server.h"
#include	"xparameters.h"

const char server_h_rcsId[] = __SERVER_H ;
const char xparameters_h_rcsId[] = __XPARAMETERS_H ;
const char msg_DS_cc_rcsId[] = "$Id: msg_DS.cc,v 1.2 2002-07-01 00:28:27 jeekay Exp $" ;

namespace gnuworld
{

// DeSynch handler?
// 0 DS :HACK: JavaDude MODE #irc.core.com +smtink lamers [957881646]
int xServer::MSG_DS( xParameters& )
{
// TODO
return 0 ;
}

} // namespace gnuworld

/* 
 * networkData.cc
 * 
 * This class is used for storing/modifying "Runtime" specific data applicable to a
 * active iClient on the network.
 *
 * N.B: ENSURE this class is free'd on EVENT QUIT/KILL for the iClient is it relating too,
 * or it'll leak. :)
 *
 */
 
#include	<strstream>
#include	<string> 

#include	<cstring> 

#include	"ELog.h"
#include	"misc.h"
#include	"networkData.h" 

const char networkData_h_rcsId[] = __NETWORKDATA_H ;
const char networkData_cc_rcsId[] = "$Id: networkData.cc,v 1.1 2002-01-14 23:14:24 morpheus Exp $" ;

namespace gnuworld
{

using std::string ;
using std::endl ;
 
networkData::networkData()
 : messageTime( 0 ),
   outputCount( 0 ),
   flood_points( 0 ),
   currentUser( 0 ), 
   ignored( false )
{
}
 
networkData::~networkData()
{}
 
} // Namespace gnuworld.

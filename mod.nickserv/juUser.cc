/*
 * juUser.cc - 20020204 - Initial Version - GK@PAnet
 *
 * $Id: juUser.cc,v 1.3 2002-02-06 01:06:49 jeekay Exp $
 *
 */

#include "juUser.h"

namespace gnuworld
{

namespace nserv
{

juUser::juUser(string theNick, string theNumeric, time_t set, time_t expires, string theReason, string hostMask)
{
	_NickName = theNick;
	_Numeric = theNumeric;
	_Set = set;
	_Expires = expires;
	_Reason = theReason;
	_HostMask = hostMask;
} // juUser::juUser

juUser::~juUser()
{
} // juUser::~juUser

} // namespace nserv

} // namespace gnuworld

/*
 * juUser.cc - 20020204 - Initial Version - GK@PAnet
 *
 * $Id: juUser.cc,v 1.1 2002-02-04 04:45:34 jeekay Exp $
 *
 */

#include "juUser.h"

namespace gnuworld
{

namespace nserv
{

juUser::juUser(string theNick, string theNumeric, time_t expires, string theReason)
{
	_NickName = theNick;
	_Numeric = theNumeric;
	_Expires = expires;
	_Reason = theReason;
} // juUser::juUser

juUser::~juUser()
{
} // juUser::~juUser

} // namespace nserv

} // namespace gnuworld

/*
 * juUser.cc - 20020204 - Initial Version - GK@PAnet
 *
 * $Id: juUser.cc,v 1.2 2002-02-05 02:27:56 jeekay Exp $
 *
 */

#include "juUser.h"

namespace gnuworld
{

namespace nserv
{

juUser::juUser(string theNick, string theNumeric, time_t set, time_t expires, string theReason)
{
	_NickName = theNick;
	_Numeric = theNumeric;
	_Set = set;
	_Expires = expires;
	_Reason = theReason;
} // juUser::juUser

juUser::~juUser()
{
} // juUser::~juUser

} // namespace nserv

} // namespace gnuworld

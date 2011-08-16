#include "nsUser.h"
#include <sys/time.h>
namespace gnuworld
{

namespace nserv
{

nsUser::nsUser(string Num, time_t ConnT , string Nick)
{
    Flags = 0;
    Numeric = Num;
    CheckTime = ConnT;
    Nickname = Nick;
}

nsUser::~nsUser()
{

}

void nsUser::SetData(string Num, time_t ConnT, string Nick)
{
  // If the user is already logged in, we need to preserve that
  // Otherwise clear all flags
    Flags = Flags & LOGGEDIN;
    Numeric = Num;
    CheckTime = ConnT;
    Nickname = Nick;
}

void nsUser::setCheckTime()
{
	CheckTime = ::time(NULL);
}

void nsUser::clearFlags()
{
	Flags = 0;
}

}
}

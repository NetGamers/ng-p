/* LOGINCommand.cc */

#include	<string>
#include	<iomanip.h>

#include	"md5hash.h"
#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"Network.h"
#include        "nsUser.h"
#include        "nickserv.h"
#include        "sqlnsUser.h"

const char RECOVERCommand_cc_rcsId[] = "$Id: RECOVERCommand.cc,v 1.1 2002-01-16 18:33:12 jeekay Exp $" ;

namespace gnuworld
{

namespace nserv
{

using namespace gnuworld;

bool RECOVERCommand::Exec( iClient* theClient, const string& Message )
{

StringTokenizer st( Message ) ;
if( st.size() < 2 )
	{
	Usage(theClient);
	return true;
	}

/*
 * Check theClient isn't already logged in, if so, tell
 * them they shouldn't be.
 */
if(!strcasecmp(theClient->getNickName(),st[1]))
	{
	bot->Notice(theClient,"You can't recover yourself!");
	return false;
	}

nsUser* tmpUser = bot->isAuth(theClient);

if(st[1][0] == '#')
  {
    bot->Notice(theClient, "AUTHENTICATION FAILED as %s.", st[1].c_str());
    return false;
  }

sqlnsUser* theUser = new (std::nothrow) sqlnsUser(bot->SQLDb);

if(!theUser->loadData(theClient->getNickName()))
  {
    bot->Notice(theClient, "AUTHENTICATION FAILED as %s.", st[1].c_str());
    return false;
  }

/*
 *  Compare password with MD5 hash stored in user record.
 */

// MD5 hash algorithm object.
md5	hash;

// MD5Digest algorithm object.
md5Digest digest;

string salt = theUser->getPassword().substr(0, 8);
string md5Part = theUser->getPassword().substr(8);
string guess = salt + st.assemble(2);

// Build a MD5 hash based on our salt + the guessed password.
hash.update( (const unsigned char *)guess.c_str(), guess.size() );
hash.report( digest );

// Convert the digest into an array of int's to output as hex for
// comparison with the passwords generated by PHP.
int data[ MD5_DIGEST_LENGTH ] = { 0 } ;

for( size_t ii = 0; ii < MD5_DIGEST_LENGTH; ii++ )
	{
	data[ii] = digest[ii];
	}

strstream output;
output << hex;
output.fill('0');

for( size_t ii = 0; ii < MD5_DIGEST_LENGTH; ii++ )
	{
	output << setw(2) << data[ii];
	}
output << ends;

if(md5Part != output.str() ) // If the MD5 hash's don't match..
{
  bot->Notice(theClient, "AUTHENTICATION FAILED AS %s.", st[1].c_str());
  delete[] output.str() ;
  return false;
}

delete[] output.str();

// We now have an authed user trying to recover
iClient* tmpClient = Network->findNick(st[1]);
if(tmpClient)
  {
    strstream s;
    s << bot->getCharYY() << " D " << tmpClient->getCharYYXXX() << " :"
      << bot->getNickName() << " [NickServ] Recover by "
      << theClient->getNickName() << ends;
    bot->Write(s.str());
    delete[] s.str();
    bot->Notice(theClient, "Recover successful for %s.", st[1].c_str());
    return true;
  }
else
  {
    bot->Notice(theClient, "I can't see %s anywhere.", st[1].c_str());
    return false;
  }

return true;
}

} // namespace gnuworld.

} // namespace nserv

/*
 * ACCESSCommand.cc
 *
 * 24/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 * 15/02/2001 - David Henriksen <david@itwebnet.dk>
 * Added -op/-voice/-none support
 *
 * 01/03/01 - Daniel Simard <svr@undernet.org>
 * Fixed Language module stuff.
 *
 * Displays all "Level" records for a specified channel.
 * Can optionally narrow down selection using a number of switches.
 *
 * $Id: ACCESSCommand.cc,v 1.11 2004-08-25 20:32:24 jeekay Exp $
 */

#include	<string>

#include	"StringTokenizer.h"
#include	"ELog.h"
#include	"cservice.h"
#include	"libpq++.h"
#include	"match.h"
#include	"responses.h"
#include	"cservice_config.h"
#include	"Network.h"

#include	"sqlChannel.h"
#include	"sqlLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{


static const char* queryHeader =    "SELECT channels.name,users.user_name,levels.access,levels.flags,users_lastseen.last_seen,levels.suspend_expires,levels.last_modif,levels.last_modif_by,levels.suspend_level FROM levels,channels,users,users_lastseen ";
static const char* queryCondition = "WHERE levels.channel_id=channels.id AND levels.user_id=users.id AND users.id=users_lastseen.user_id ";
static const char* queryFooter =    "ORDER BY levels.access DESC;";

void ACCESSCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.ACCESS");

/*
 * This command will build up a custom SQL query and execute it on
 * the 'levels' table.
 */

StringTokenizer st( Message ) ;
if( st.size() < 3 )
	{
	Usage(theClient);
	return ;
	}

sqlUser* theUser = bot->isAuthed(theClient, false);
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser,
			language::chan_not_reg).c_str(),
		st[1].c_str()
		);
	return ;
	}

/* Don't let ordinary people view * accesses */
if (theChan->getName() == "*")
	{
	sqlUser* theUser = bot->isAuthed(theClient, false);
	if (!theUser)
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::chan_not_reg).c_str(),
			st[1].c_str()
		);
		return ;
		}

	if (theUser && !bot->getAdminAccessLevel(theUser))
		{
		bot->Notice(theClient,
			bot->getResponse(theUser,
				language::chan_not_reg).c_str(),
			st[1].c_str()
		);
		return ;
		}
	}

/* 
 * Should we allow them to see the access?
 * If the channel is set INVISIBLE, only
 * admins, opers and users with channel access may.
 */

/* Prettyness variables */
bool hasAccess = bot->getAccessLevel(theUser, theChan);
bool isAdmin = bot->getAdminAccessLevel(theUser);
bool isInvis = theChan->getFlag(sqlChannel::F_INVISIBLE);
bool isOper = theClient->isOper();

/* Karnaugh maps ahoy! */
if(!(hasAccess || !isInvis || isAdmin || isOper)) {
  bot->Notice(theClient, "Sorry, you have insufficient access to list the users in %s.",
    theChan->getName().c_str());
  return ;
}

/*
 *  Figure out the switches and append to the SQL statement accordingly.
 */

/* 0 = None, 1 = min, 2 = max, 3 = modif, 4 = op, 5 = voice, 6 = none. 7 = autoinvite*/
unsigned short currentType = 0;

unsigned int minAmount = 0;
unsigned int maxAmount = 0;
bool modif = false;
bool showAll = false;
bool aOp = false;
bool aVoice = false;
bool aNone = false;
#ifdef FEATURE_INVITE
bool aInvite = false;
#endif

string modifMask;

for( StringTokenizer::const_iterator ptr = st.begin() ; ptr != st.end() ;
	++ptr )
	{
	if (string_lower(*ptr) == "-min")
		{
		currentType = 1;
		continue;
		}

	if (string_lower(*ptr) == "-max")
		{
		currentType = 2;
		continue;
		}

	if (string_lower(*ptr) == "-modif")
		{
		currentType = 3;
		modif = true;
		continue;
		}

	if (string_lower(*ptr) == "-op")
		{
		currentType = 4;
		aOp = true;
		continue;
		}

	if (string_lower(*ptr) == "-voice")
		{
		currentType = 5;
		aVoice = true;
		continue;
		}

	if (string_lower(*ptr) == "-none")
		{
		currentType = 6;
		aNone = true;
		continue;
		}
	#ifdef FEATURE_INVITE
	if (string_lower(*ptr) == "-invite")
		{
		currentType = 7;
		aInvite = true;
		continue;
		}
	#endif

	if (string_lower(*ptr) == "-all")
		{
		sqlUser* tmpUser = bot->isAuthed(theClient, false);
		if( tmpUser  && bot->getAdminAccessLevel(tmpUser) )
			{
			showAll = true;
			}
		continue;
		}

	switch(currentType)
		{
		case 1: /* Min */
			{
			minAmount = atoi( (*ptr).c_str() );
			if ((minAmount > 1000) || (minAmount < 0))
				{
				bot->Notice(theClient,
					bot->getResponse(theUser,
						language::inval_min_lvl).c_str()
				);
				return ;
				}
			currentType = 0;
			break;
			}
		case 2: /* Max */
			{
			maxAmount = atoi( (*ptr).c_str() );
			if ((maxAmount > 1000) || (maxAmount < 0))
				{
				bot->Notice(theClient,
					bot->getResponse(theUser,
						language::inval_max_lvl).c_str()
				);
				return ;
				}

			currentType = 0;
			break;
			}
		case 3: /* Modif */
			{
			// [22:13] <DrCkTaiL> backburner
			break;
			}
		case 4: /* Automode Op */
			{
			break;
			}
		case 5: /* Automode Voice */
			{
			break;
			}
		case 6: /* Automode None */
			{
			break;
			}
		#ifdef FEATURE_INVITE
		case 7: /* Automode Invite */
			{
			break;
			}
		#endif

		}
	}

/* Sort out the additional conditions */

stringstream extraCond;
if (minAmount)
	{
	extraCond << "AND levels.access >= " << minAmount << " ";
	}
if (maxAmount)
	{
	extraCond << "AND levels.access <= " << maxAmount << " ";
	}

stringstream theQuery;
theQuery	<< queryHeader
		<< queryCondition
		<< extraCond.str().c_str()
		<< "AND levels.channel_id = "
		<< theChan->getID()
		<< " "
		<< queryFooter
		;

#ifdef LOG_SQL
	elog	<< "ACCESS::sqlQuery> "
		<< theQuery.str().c_str()
		<< endl;
#endif

/*
 *  All done, display the output. (Only fetch 15 results).
 */

ExecStatusType status = bot->SQLDb->Exec( theQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "ACCESS> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< endl ;
	return ;
	}

sqlLevel::flagType flag = 0 ;

string autoMode;
int duration = 0;
int suspend_expires = 0;
int suspend_expires_d = 0;
int suspend_expires_f = 0;
int results = 0;
string matchString = st[2];

if(matchString[0] == '-')
	{
	matchString = "*";
	}

/*
 * Convert =nick to username.
 */

if (matchString[0] == '=')
	{
	const char* theNick = matchString.c_str();
	// Skip the '='
	++theNick;

	iClient *theClient = Network->findNick(theNick);
	if (theClient)
		{
		sqlUser* tmpUser = bot->isAuthed(theClient,false);
		if (tmpUser)
			{
			matchString = tmpUser->getUserName();
			}
		}
	}


for (int i = 0 ; i < bot->SQLDb->Tuples(); i++)
	{
	autoMode = "None";

	/* Does the username match the query? */
	if (match(matchString, bot->SQLDb->GetValue(i, 1)) == 0)
		{
		flag = atoi(bot->SQLDb->GetValue(i, 3));
		duration = atoi(bot->SQLDb->GetValue(i, 4));
		suspend_expires = atoi(bot->SQLDb->GetValue(i, 5));
		suspend_expires_d = suspend_expires - bot->currentTime();
		suspend_expires_f = bot->currentTime() - suspend_expires_d;

		if (flag & sqlLevel::F_AUTOOP) autoMode = "OP";
		if (flag & sqlLevel::F_AUTOVOICE) autoMode = "VOICE";
		
		#ifdef FEATURE_INVITE
		if (flag & sqlLevel::F_AUTOINVITE) { autoMode += " INVITE"; }
		#endif
		if(aVoice == true || aOp == true || aNone == true
		#ifdef FEATURE_INVITE
		   || aInvite == true
		#endif
		)
			{
			if(aNone == true)
				{
				if(!aVoice && (flag & sqlLevel::F_AUTOVOICE)) continue;
				if(!aOp && (flag & sqlLevel::F_AUTOOP)) continue;
				#ifdef FEATURE_INVITE
				if(!aInvite && (flag & sqlLevel::F_AUTOINVITE)) continue;
				#endif
				}
			else
				{
				if(!(flag & sqlLevel::F_AUTOVOICE) &&
				   !(flag & sqlLevel::F_AUTOOP)
				#ifdef FEATURE_INVITE
				   && !(flag & sqlLevel::F_AUTOINVITE)
				#endif
				) continue;
				if(!aVoice && (flag & sqlLevel::F_AUTOVOICE)) continue;

				if(!aOp && (flag & sqlLevel::F_AUTOOP)) continue;
				#ifdef FEATURE_INVITE
				if(!aInvite && (flag & sqlLevel::F_AUTOINVITE)) continue;
				#endif
				}
			}

		results++;

		bot->Notice(theClient,
			bot->getResponse(theUser, language::user_access_is).c_str(),
			bot->SQLDb->GetValue(i, 1),
			bot->SQLDb->GetValue(i, 2),
			bot->userStatusFlags(bot->SQLDb->GetValue(i, 1)).c_str()
		);

		bot->Notice(theClient,
			bot->getResponse(theUser, language::channel_automode_is).c_str(),
			bot->SQLDb->GetValue(i, 0),
			autoMode.c_str()
		);

		if(suspend_expires != 0)
			{
			unsigned int suspendLevel = atoi(bot->SQLDb->GetValue(i, 8));

			bot->Notice(theClient,
				bot->getResponse(theUser,
					language::suspend_expires_in).c_str(),
				bot->prettyDuration(suspend_expires_f).c_str(),
				suspendLevel
				);
			}
		bot->Notice(theClient,
			bot->getResponse(theUser,
					language::last_seen).c_str(),
			bot->prettyDuration(duration).c_str()
		);

		if(modif)
			{
			time_t lastMod = static_cast< time_t >( atoi(bot->SQLDb->GetValue(i, 6)));
			char myCtime[27];
			ctime_r(&lastMod, myCtime);
			myCtime[strlen(myCtime)-1] = 0;
			bot->Notice(theClient, "LAST MODIFIED: %s (%s - %s ago)",
				bot->SQLDb->GetValue(i, 7),
				myCtime,
				bot->prettyDuration(lastMod, string("d")).c_str()
				);
			}
		}
	if ((results >= MAX_ACCESS_RESULTS) && !showAll) break;

	} // for()

if ((results >= MAX_ACCESS_RESULTS) && !showAll)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::more_than_max).c_str()
	);
	bot->Notice(theClient,
		bot->getResponse(theUser, language::restrict_query).c_str()
		);
	}
else if (results > 0)
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::end_access_list).c_str()
		);
	}
else
	{
	bot->Notice(theClient,
		bot->getResponse(theUser, language::no_match).c_str()
		);
	}

return ;
}

} // namespace gnuworld.

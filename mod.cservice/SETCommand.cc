/*
 * SETCommand.cc
 *
 * 28/12/2000 - David Henriksen <david@itwebnet.dk>
 * Initial Version.
 * 01/01/2001 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Modifications.
 * 10/02/2001 - David Henriksen <david@itwebnet.dk>
 * Minor bug fixes.
 *
 * 20/02/2001 - Gator Robert White <gator@cajun-gator.net>
 * removed AlwaysOp
 * Sets channel options on the specified channel.
 * 2001-03-16 - Perry Lorier <isomer@coders.net>
 * Added 'DESC' as an alias for 'DESCRIPTION'
 * 2001-04-16 - Alex Badea <vampire@go.ro>
 * Changed the implementation for SET LANG, everything is dynamic now.
 *
 * Caveats: None.
 */

#include  <string>

#include  "StringTokenizer.h"
#include  "cservice.h"
#include  "Network.h"
#include  "levels.h"
#include  "responses.h"
#include  "cservice_config.h"

#include	"sqlChannel.h"
#include	"sqlCommandLevel.h"
#include	"sqlLevel.h"
#include	"sqlUser.h"


namespace gnuworld
{
using namespace level;

void SETCommand::Exec( iClient* theClient, const string& Message )
{
bot->incStat("COMMANDS.SET");

StringTokenizer st( Message ) ;

if( st.size() < 3 )
  {
  Usage(theClient);
  return ;
  }

/* Is the user authorised? */

sqlUser* theUser = bot->isAuthed(theClient, true);
if(!theUser)
  {
  return ;
  }

/*
 * First, is this a #channel or user set?
 */

if( st[1][0] != '#' ) // Didn't find a hash?
{
  // Look by user then.
  string option = string_upper(st[1]);
  string value = string_upper(st[2]);
  if (option == "INVISIBLE")
  {
    if (value == "ON")
    {
      theUser->setFlag(sqlUser::F_INVIS);
      theUser->commit();
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::invis_on,
          string("Your INVISIBLE setting is now ON.")));
      return ;
    }

    if (value == "OFF")
    {
      theUser->removeFlag(sqlUser::F_INVIS);
      theUser->commit();
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::invis_off,
          string("Your INVISIBLE setting is now OFF.")));
      return ;
    }

    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
          return ;
  }

  if (option == "MAXLOGINS") {
    unsigned int maxlogins = atoi(value.c_str());
    if(maxlogins > 3 || maxlogins < 1) {
      bot->Notice(theClient, "Max Logins cannot be greater than 3 or less than 1");
      return ;
    }
    
    theUser->setMaxLogins(maxlogins);
    theUser->commit();
    
    bot->Notice(theClient, "Max Logins now set to %i", maxlogins);
    return ;
  } // option == MAXLOGINS
  
  if (option == "NOTE") {
  	if (value == "ALLOW") {
		theUser->removeFlag(sqlUser::F_MEMO_REJECT);
	} else if (value == "REJECT") {
		theUser->setFlag(sqlUser::F_MEMO_REJECT);
	} else {
		bot->Notice(theClient, "Value of %s must be ALLOW or REJECT.",
			option.c_str()
			);
		return ;
	}
	
	theUser->commit();
	
	bot->Notice(theClient, "Notes will now be %s by default.",
		theUser->getFlag(sqlUser::F_MEMO_REJECT) ? "REJECTED" : "ACCEPTED"
		);
	
	return ;
  }

  if (option == "LANG")
  {
    cservice::languageTableType::iterator ptr = bot->languageTable.find(value);
    if (ptr != bot->languageTable.end())
    {
      string lang = ptr->second.second;
      theUser->setLanguageId(ptr->second.first);
      theUser->commit();
      bot->Notice(theClient,
          bot->getResponse(theUser,
                language::lang_set_to,
            string("Language is set to %s.")).c_str(), lang.c_str());
      return ;
    }

    bot->Notice(theClient,
            "ERROR: Invalid language selection.");
    return ;
  }

  if (option == "NS" || option == "NICKSERV" || option == "AUTOKILL")
  {
    if (value == "ON")
                {
                        theUser->setFlag(sqlUser::F_AUTOKILL);
                        theUser->commit();
                        bot->Notice(theClient, "Your NICKSERV setting is now ON.");
                        return ;
                }
                
                if (value == "OFF")
                {
                        theUser->removeFlag(sqlUser::F_AUTOKILL);
                        theUser->commit();
                        bot->Notice(theClient, "Your NICKSERV setting is now OFF.");
      return ;
                }
                
                bot->Notice(theClient,
                        bot->getResponse(theUser,
                                language::set_cmd_syntax_on_off,
                                string("value of %s must be ON or OFF")).c_str(),
                        option.c_str());
    return ;
  }

  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::invalid_option,
      string("Invalid option.")));
  return ;
}

Channel* tmpChan = Network->findChannel(st[1]);

/* Is the channel registered? */

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if(!theChan)
  {
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::chan_not_reg,
      string("Sorry, %s isn't registered with me.")).c_str(),
    st[1].c_str());
  return ;
  }

#ifdef FEATURE_FORCELOG
unsigned short forcedAccess = bot->isForced(theChan, theUser);
if (forcedAccess <= 900  && forcedAccess > 0)
        {
        bot->writeForceLog(theUser, theChan, Message);
        }

#endif

// Check level.

int level = bot->getEffectiveAccessLevel(theUser, theChan, false);
string option = string_upper(st[2]);
string value;

if (st.size() < 4)
  {
  value = "";
  }
else
  {
   value = string_upper(st[3]);
  }

  /*
   * Check the "Locked" status first, so admin's can bypass to turn it OFF :)
   */

  if(option == "LOCKED")
  {
      // Check for admin access
      sqlChannel* admChan = bot->getChannelRecord("*");
      int admLevel = bot->getAccessLevel(theUser, admChan);
      if(admLevel < level::set::locked)
      {
      // No need to tell users about admin commands.
      Usage(theClient);
      return ;
      }
      if(value == "ON") theChan->setFlag(sqlChannel::F_LOCKED);
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_LOCKED);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->logAdminMessage("%s (%s) - SET - LOCKED - %s %s",
        theClient->getNickName().c_str(), theUser->getUserName().c_str(),
        theChan->getName().c_str(), value.c_str());
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),

      theChan->getFlag(sqlChannel::F_LOCKED) ? "ON" : "OFF");

      return ;
  }

  /*
   *  Check if the channel is "Locked", if so, only allow admins to change settings.
   */

  if(theChan->getFlag(sqlChannel::F_LOCKED))
  {
    int admLevel = bot->getAdminAccessLevel(theUser);
    if (admLevel < level::set::locked)
      {
      bot->Notice(theClient, "The channel settings for %s have been locked by a CService"
        " Administrator and cannot be changed.", theChan->getName().c_str());
      return ;
      }
  }

  if(option == "CAUTION")
  {
    /* Check for admin access */
    if(bot->getAdminAccessLevel(theUser) < level::set::caution)
  {
  /* No need to tell users about admin commands. */
  Usage(theClient);
  return ;
  }

    if(value == "ON") theChan->setFlag(sqlChannel::F_CAUTION);
    else if(value == "OFF") theChan->removeFlag(sqlChannel::F_CAUTION);
    else
  {
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::set_cmd_syntax_on_off,
      string("value of %s must be ON or OFF")).c_str(),
    option.c_str());
  return ;
  }

  theChan->commit();
  bot->logAdminMessage("%s (%s) - SET - CAUTION - %s %s",
    theClient->getNickName().c_str(), theUser->getUserName().c_str(),
    theChan->getName().c_str(), value.c_str());
  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::set_cmd_status,
      string("%s for %s is %s")).c_str(),
  option.c_str(),
    theChan->getName().c_str(),
    theChan->getFlag(sqlChannel::F_CAUTION) ? "ON" : "OFF");
  return ;
  }

  if (option == "WELCOME")
  {
            string welcome = st.assemble(3);
            if(level < level::set::welcome)
            {
                bot->Notice(theClient,
                                  bot->getResponse(theUser,
                                  language::insuf_access,
                                  string("You do not have enough access!")));
                return ;
            }
            if(strlen(welcome.c_str()) > 255)
            {
                        bot->Notice(theClient, "The WELCOME message can be a maximum of 255 chars!");
                        return ;
            }
                theChan->setWelcome(welcome);

                if(welcome == "")
                {
                        bot->Notice(theClient, "WELCOME message for %s is cleared.",
                                theChan->getName().c_str());
                        theChan->removeFlag(sqlChannel::F_WELCOME);
                } else
                {
                        bot->Notice(theClient, "WELCOME message for %s is: %s",
                                        theChan->getName().c_str(), welcome.c_str());
                        theChan->setFlag(sqlChannel::F_WELCOME);
                }
                theChan->commit();

            return ;

  }
  
  if(option == "PARTNER") {
	// Check for admin access
	sqlChannel *admChan = bot->getChannelRecord("*");
	int admLevel = bot->getAccessLevel(theUser, admChan);
	if(admLevel < level::set::partner) {
		Usage(theClient);
		return ;
	}
	
	if("ON" == value) theChan->setFlag(sqlChannel::F_PARTNER);
	else if("OFF" == value) theChan->removeFlag(sqlChannel::F_PARTNER);
	else {
		bot->Notice(theClient, "Value of %s must be ON or OFF",
			option.c_str()
			);
		return ;
	}
	
	theChan->commit();
	
	bot->logAdminMessage("%s (%s) - SET - PARTNER - %s %s",
		theClient->getNickName().c_str(),
		theUser->getUserName().c_str(),
		theChan->getName().c_str(),
		value.c_str()
		);
	bot->Notice(theClient, "%s for %s set to %s",
		option.c_str(),
		theChan->getName().c_str(),
		value.c_str()
		);
	
	return;
  }

  if(option == "NOPART")
  {
      // Check for admin access
      sqlChannel* admChan = bot->getChannelRecord("*");
      int admLevel = bot->getAccessLevel(theUser, admChan);
      if(admLevel < level::set::nopart)
      {
      // No need to tell users about admin commands.
      Usage(theClient);
      return ;
      }
      if(value == "ON") theChan->setFlag(sqlChannel::F_NOPART);
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_NOPART);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->logAdminMessage("%s (%s) - SET - NOPART - %s %s",
        theClient->getNickName().c_str(), theUser->getUserName().c_str(),
        theChan->getName().c_str(), value.c_str());
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_NOPART) ? "ON" : "OFF");

      return ;
  }

  if(option == "NOPURGE")
  {
      // Check for admin access
      sqlChannel* admChan = bot->getChannelRecord("*");
      int admLevel = bot->getAccessLevel(theUser, admChan);
      if(admLevel < level::set::nopurge)
      {
      // No need to tell users about admin commands.
      Usage(theClient);
      return ;
      }
      if(value == "ON") theChan->setFlag(sqlChannel::F_NOPURGE);
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_NOPURGE);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->logAdminMessage("%s (%s) - SET - NOPURGE - %s %s",
        theClient->getNickName().c_str(), theUser->getUserName().c_str(),
        theChan->getName().c_str(), value.c_str());
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_NOPURGE) ? "ON" : "OFF");
      return ;
  }

  if(option == "TEMPMAN")
  {
      // Check for admin access
      sqlChannel* admChan = bot->getChannelRecord("*");
      int admLevel = bot->getAccessLevel(theUser, admChan);
      if(admLevel < level::set::tempman)
      {
      // No need to tell users about admin commands.
      Usage(theClient);
      return ;
      }

      if(value == "ON") theChan->setFlag(sqlChannel::F_TEMP);
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_TEMP);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->logAdminMessage("%s (%s) - SET - TEMPMAN - %s %s",
        theClient->getNickName().c_str(), theUser->getUserName().c_str(),
        theChan->getName().c_str(), value.c_str());
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_TEMP) ? "ON" : "OFF");
      return ;
  }

  if(option == "VACATION")
  {
      // Check for admin access
      sqlChannel* admChan = bot->getChannelRecord("*");
      int admLevel = bot->getAccessLevel(theUser, admChan);
      if(admLevel < level::set::vacation)
      {
      // No need to tell users about admin commands.
      Usage(theClient);
      return ;
      }
      if(value == "ON") theChan->setFlag(sqlChannel::F_VACATION);
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_VACATION);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->logAdminMessage("%s (%s) - SET - VACATION - %s %s",
        theClient->getNickName().c_str(), theUser->getUserName().c_str(),
        theChan->getName().c_str(), value.c_str());
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_VACATION) ? "ON" : "OFF");
      return ;
  }

  if(option == "NOOP")
  {
      if(level < level::set::noop)
      {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::insuf_access,
          string("You do not have enough access!")));
      return ;
      }
      if(value == "ON")
      {
      theChan->setFlag(sqlChannel::F_NOOP);
      if (tmpChan) bot->deopAllOnChan(tmpChan); // Deop everyone. :)
      }
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_NOOP);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_NOOP) ? "ON" : "OFF");
      return ;
  }

  if(option == "STRICTOP")
  {
      if(level < level::set::strictop)
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::insuf_access,
        string("You do not have enough access!")));
    return ;
      }
      if(value == "ON")
      {
        theChan->setFlag(sqlChannel::F_STRICTOP);
      if (tmpChan) bot->deopAllUnAuthedOnChan(tmpChan);
    }
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_STRICTOP);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_STRICTOP) ? "ON" : "OFF");
      return ;
  }

#ifdef FEATURE_STRICTVOICE  
  if(option == "STRICTVOICE")
    {
    if(level < level::set::strictvoice)
      {
      bot->Notice(theClient, "Sorry, you have insufficient access to perform that command");
      return ;
      }
    
    if(value == "ON")
      {
      theChan->setFlag(sqlChannel::F_STRICTVOICE);
      }
    else if(value == "OFF")
      {
      theChan->removeFlag(sqlChannel::F_STRICTVOICE);
      }
    else
      {
      bot->Notice(theClient, "Value of STRICTVOICE must be ON or OFF");
      return ;
      }
    
    theChan->commit();
    bot->Notice(theClient, "STRICTVOICE for %s is %s",
                theChan->getName().c_str(),
                theChan->getFlag(sqlChannel::F_STRICTVOICE) ? "ON" : "OFF");
    return ;
    }
#endif

  if(option == "AUTOTOPIC")
  {
      if(level < level::set::autotopic)
      {
    bot->Notice(theClient,
        bot->getResponse(theUser,
        language::insuf_access,
        string("You do not have enough access!")));
    return ;
      }
      if(value == "ON")
      {
        theChan->setFlag(sqlChannel::F_AUTOTOPIC);
      bot->doAutoTopic(theChan);
    }
      else if(value == "OFF") theChan->removeFlag(sqlChannel::F_AUTOTOPIC);
      else
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
      option.c_str());
    return ;
      }
      theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_AUTOTOPIC) ? "ON" : "OFF");
      return ;
  }
#ifdef FEATURE_NOFORCE
  if(option == "NOFORCE")
  {
    sqlChannel* admChan = bot->getChannelRecord("*");
              int admLevel = bot->getAccessLevel(theUser, admChan);
    if(admLevel < level::set::noforce)
    {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::insuf_access,
          string("You do not have enough access!")));
      return ;
    }
    if(value == "ON") 
    {
      theChan->setFlag(sqlChannel::F_NOFORCE);
    }
    else if(value == "OFF") theChan->removeFlag(sqlChannel::F_NOFORCE);
    else
    {
      bot->Notice(theClient,
                        bot->getResponse(theUser,
                                language::set_cmd_status,
                                string("Your %s for %s is %s")).c_str(),
                                option.c_str(),
                                theChan->getName().c_str(),
                                theChan->getFlag(sqlChannel::F_NOFORCE) ? "ON" : "OFF");
                  return ;
    }
    theChan->commit();
    bot->logAdminMessage("%s (%s) - SET - NOFORCE - %s %s",
      theClient->getNickName().c_str(), theUser->getUserName().c_str(),
      theChan->getName().c_str(), value.c_str());
    bot->Notice(theClient,
                        bot->getResponse(theUser,
                                language::set_cmd_status,
                                string("Your %s for %s is %s")).c_str(),
                                option.c_str(),
                                theChan->getName().c_str(),
                                theChan->getFlag(sqlChannel::F_NOFORCE) ? "ON" : "OFF");
                return ;
  }
#endif

#ifdef FEATURE_INVITE
  if(option == "AUTOINVITE")
  {
    if(level < level::set::autoinvite)
    {
    bot->Notice(theClient,
                   bot->getResponse(theUser,
                              language::insuf_access,
                                 string("You do not have enough access!")));
    return ;
    }
    sqlLevel* aLevel = bot->getLevelRecord(theUser, theChan);
    
    if(!aLevel) {
      bot->Notice(theClient, "You are not added to channel %s",
        theChan->getName().c_str());
      return ;
    }
    
    if(value == "ON") aLevel->setFlag(sqlLevel::F_AUTOINVITE);
    else if(value == "OFF") aLevel->removeFlag(sqlLevel::F_AUTOINVITE);
    else
    {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_syntax_on_off,
        string("value of %s must be ON or OFF")).c_str(),
        option.c_str());
    return ;
    }
//              aLevel->setkastModif(bot->currentTime());
//              aLevel->setlastModifBy(theUser->getNickUserHost());
                aLevel->commit();
    bot->Notice(theClient,
      bot->getResponse(theUser,
              language::set_cmd_status,
        string("Your %s for %s is %s")).c_str(),
        option.c_str(),
        theChan->getName().c_str(),
        aLevel->getFlag(sqlLevel::F_AUTOINVITE) ? "ON" : "OFF");
    return ;
  }
#endif

  if(option == "AUTOJOIN") {
    if(level < level::set::autojoin) {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::insuf_access,
          string("You do not have enough access!")));

      return ;
    }
    
    if(value == "ON") {
      theChan->setFlag(sqlChannel::F_AUTOJOIN);
      theChan->removeFlag(sqlChannel::F_IDLE);
      theChan->setInChan(true);
      bot->writeChannelLog(theChan, theClient, sqlChannel::EV_JOIN, "");
      bot->getUplink()->RegisterChannelEvent( theChan->getName(), bot ) ;
      bot->Join(theChan->getName(), "", theChan->getChannelTS(), false);
      bot->joinCount++;
      bot->reopQ.insert(cservice::reopQType::value_type(theChan->getName(), bot->currentTime() + 15) );
    
      if (tmpChan) {
        if(theChan->getFlag(sqlChannel::F_NOOP)) bot->deopAllOnChan(tmpChan);
        if(theChan->getFlag(sqlChannel::F_STRICTOP)) bot->deopAllUnAuthedOnChan(tmpChan);
      }
    } else if(value == "OFF") {
      theChan->removeFlag(sqlChannel::F_AUTOJOIN);
      theChan->setInChan(false);
      bot->joinCount--;
      bot->getUplink()->UnRegisterChannelEvent( theChan->getName(), bot ) ;
      bot->Part(theChan->getName());
    } else {
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getFlag(sqlChannel::F_AUTOJOIN) ? "ON" : "OFF");
      return ;
    }
    
    theChan->commit();
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
        option.c_str(),
    theChan->getName().c_str(),
    theChan->getFlag(sqlChannel::F_AUTOJOIN) ? "ON" : "OFF");
    
    return ;
  } // AUTOJOIN

  if(option == "USERFLAGS")
  {
      if(level < level::set::userflag)
      {
                bot->Notice(theClient,
                                bot->getResponse(theUser,
                                language::insuf_access,
                                string("You do not have enough access!")));
    return ;
      }

    int setting = atoi(value.c_str());
    if ( (setting < 0) || (setting > 3))
    {
      bot->Notice(theClient, "Invalid USERFLAGS setting. Correct values are 0, 1, 2, 3.");
      return ;
    }

    theChan->setUserFlags(setting);
    theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::userflags_status,
        string("USERFLAGS for %s is %i")).c_str(),
      theChan->getName().c_str(), setting);
      return ;
  }

  if(option == "MASSDEOPPRO")
  {
      if(level < level::set::massdeoppro)
      {
                  bot->Notice(theClient,
                                  bot->getResponse(theUser,
                                  language::insuf_access,
                                  string("You do not have enough access!")));
      return ;
      }
      // Temporary MASSDEOPPRO range! 0-7.. is this correct?
      if(!IsNumeric(value))
      {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::massdeoppro_syntax,
          string("value of MASSDEOPPRO has to be 0-7")));
      return ;
      }
      int numValue = atoi(value.c_str());
      if(numValue > 7 || numValue < 0)
      {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::massdeoppro_syntax,
          string("value of MASSDEOPPRO has to be 0-7")));
      return ;
      }
    theChan->setMassDeopPro(numValue);
      theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::massdeoppro_status,
        string("MASSDEOPPRO for %s is set to %d")).c_str(),
      theChan->getName().c_str(), numValue);
      return ;
  }
// FLOADLIMIT
/*  if(option == "FLOODPRO")
  {
      if(level < level::set::floodpro)
      {
                        bot->Notice(theClient,
                                        bot->getResponse(theUser,
                                        language::insuf_access,
                                        string("You do not have enough access!")));
      return ;
      }
      // Temporary MASSDEOPPRO range! 0-7.. is this correct?
      if(!IsNumeric(value))
      {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::floodpro_syntax,
          string("value of FLOODPRO has to be 0-7")));
      return ;
      }
        int numValue = atoi(value.c_str());
        if(numValue > 7 || numValue < 0)
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::floodpro_syntax,
        string("value of FLOODPRO has to be 0-7")));
    return ;
      }
    theChan->setFloodPro(numValue);
      theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::floodpro_status,
        string("FLOODPRO for %s is set to %d")).c_str(),
      theChan->getName().c_str(), numValue);
      return ;
  }
*/
  if(option == "DESCRIPTION" || option == "DESC")
  {
    string desc = st.assemble(3);
      if(level < level::set::desc)
      {
                bot->Notice(theClient,
                                  bot->getResponse(theUser,
                                  language::insuf_access,
                                  string("You do not have enough access!")));
    return ;
      }
      if(strlen(desc.c_str()) > 128)
      {
      bot->Notice(theClient, "The DESCRIPTION can be a maximum of 128 chars!");
      return ;
      }
    theChan->setDescription(desc);
      theChan->commit();

    if(desc == "")
    {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::desc_cleared,
          string("DESCRIPTION for %s is cleared.")).c_str(),
        theChan->getName().c_str());
    } else
    {
        bot->Notice(theClient,
      bot->getResponse(theUser,
        language::desc_status,
        string("DESCRIPTION for %s is: %s")).c_str(),
      theChan->getName().c_str(),
      desc.c_str());
    }

    if (theChan->getFlag(sqlChannel::F_AUTOTOPIC))
    {
      bot->doAutoTopic(theChan);
    }

      return ;
  }

  if(option == "URL")
  {
    string url = st.assemble(3);
      if(level < level::set::url)
      {
                        bot->Notice(theClient,
                                        bot->getResponse(theUser,
                                        language::insuf_access,
                                        string("You do not have enough access!")));
      return ;
      }
      if(strlen(url.c_str()) > 75) // Gator - changed to 75
      {
      bot->Notice(theClient, "The URL can be a maximum of 75 chars!");
      return ;
      }
    theChan->setURL(url);
      theChan->commit();

    if(url == "")
    {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::url_cleared,
          string("URL for %s is cleared.")).c_str(),
        theChan->getName().c_str());
    } else
    {
        bot->Notice(theClient,
      bot->getResponse(theUser,
        language::url_status,
        string("URL for %s is: %s")).c_str(),
      theChan->getName().c_str(),
      url.c_str());
    }

    if (theChan->getFlag(sqlChannel::F_AUTOTOPIC))
    {
      bot->doAutoTopic(theChan);
    }

      return ;
  }

  if(option == "KEYWORDS")
  {
      /* Keywords are being processed as a long string. */
      string keywords = st.assemble(3);
      if(level < level::set::keywords)
      {
                bot->Notice(theClient,
                               bot->getResponse(theUser,
                               language::insuf_access,
                               string("You do not have enough access!")));
    return ;
      }
      if(strlen(value.c_str()) > 80) // is 80 ok as an max keywords length?
      {
    bot->Notice(theClient,
      bot->getResponse(theUser,
        language::keywords_max_len,
        string("The string of keywords cannot exceed 80 chars!")));
    return ;
      }
      theChan->setKeywords(keywords);
      theChan->commit();
      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::keywords_status,
        string("KEYWORDS for %s are: %s")).c_str(),
      theChan->getName().c_str(),
      keywords.c_str());
      return ;
  }

  if(option == "MODE")
  {
      if(level < level::set::mode)
      {
                bot->Notice(theClient,
                               bot->getResponse(theUser,
                               language::insuf_access,
                               string("You do not have enough access!")));
    return ;
      }
    if (!tmpChan)
    {
      bot->Notice(theClient,
        bot->getResponse(theUser,
          language::no_such_chan,
          string("Can't locate channel %s on the network!")).c_str(),
        st[1].c_str());
      return ;
    }

      theChan->setChannelMode(tmpChan->getModeString());
      theChan->commit();

      bot->Notice(theClient,
      bot->getResponse(theUser,
        language::set_cmd_status,
        string("%s for %s is %s")).c_str(),
      option.c_str(),
      theChan->getName().c_str(),
      theChan->getChannelMode().c_str());
      return ;
  }

  if(option == "INVISIBLE") {
    if(level < level::set::invisible) {
      bot->Notice(theClient, "You do not have enough access!");
      return ;
    }
    
    int tempvalue = atoi(value.c_str());
    
    if("ON" == value) {
    	theChan->setFlag(sqlChannel::F_INVISIBLE);
	theChan->setInvisible(1);
    }
    else if("OFF" == value) {
    	theChan->removeFlag(sqlChannel::F_INVISIBLE);
	theChan->setInvisible(0);
    }
    else if( tempvalue >= 0 && tempvalue <= level::adduser ) {
    	if( tempvalue == 0 ) {
		theChan->removeFlag(sqlChannel::F_INVISIBLE);
	} else {
	    	theChan->setFlag(sqlChannel::F_INVISIBLE);
	}
	theChan->setInvisible(tempvalue);
    }
    else {
      bot->Notice(theClient, "Value of INVISIBLE must be ON, OFF or a level");
      return ;
    }
    
    bot->Notice(theClient, "INVISIBLE set to %s", value.c_str());
    theChan->commit();
    
    return ;
  }

  if(option == "FLOATLIM") { 
    if(level < level::set::floatlim) { 
      bot->Notice(theClient, 
        bot->getResponse(theUser, 
          language::insuf_access, 
          string("You do not have enough access!"))); 
      return ;
    }
    
    if(value == "ON") theChan->setFlag(sqlChannel::F_FLOATLIM); 
    else if(value == "OFF") theChan->removeFlag(sqlChannel::F_FLOATLIM); 
    else { 
      bot->Notice(theClient, 
        bot->getResponse(theUser, 
          language::set_cmd_syntax_on_off, 
          string("value of %s must be ON or OFF")).c_str(), 
          option.c_str()); 
      return ; 
    } 
    
    theChan->commit(); 
    bot->Notice(theClient, 
      bot->getResponse(theUser, 
        language::set_cmd_status, 
        string("%s for %s is %s")).c_str(), 
        option.c_str(), 
        theChan->getName().c_str(), 
        theChan->getFlag(sqlChannel::F_FLOATLIM) ? "ON" : "OFF"); 
    return ; 
  } 
    
           if(option == "FLOATPERIOD") 
           { 
               if(level < level::set::floatlim) 
               { 
                   bot->Notice(theClient, 
                                   bot->getResponse(theUser, 
                                   language::insuf_access, 
                                   string("You do not have enough access!"))); 
                   return ; 
               } 
    
           unsigned int limit_period = atoi(value.c_str()); 
    
           if ((limit_period < 20) | (limit_period > 200)) 
                   { 
                           bot->Notice(theClient, "Invalid floating-limit period (20-200 Allowed)."); 
                           return ; 
                   } 
    
     theChan->setLimitPeriod(limit_period);
           theChan->commit(); 
    
           bot->Notice(theClient, "Floating-limit period now set to %i", limit_period); 
           return ; 
           } 
    
     if(option == "FLOATMARGIN")
           { 
               if(level < level::set::floatlim) 
               { 
                   bot->Notice(theClient, 
                                   bot->getResponse(theUser, 
                                   language::insuf_access, 
                                   string("You do not have enough access!"))); 
                   return ; 
               } 
    
     unsigned int limit_offset = atoi(value.c_str());
    
           if ((limit_offset <= 1) || (limit_offset > 20))
                   { 
                           bot->Notice(theClient, "Invalid floating limit margin (2-20 Allowed)."); 
                           return ; 
                   } 
    
           if (limit_offset <= theChan->getLimitGrace())
          { 
                           bot->Notice(theClient, "FLOATMARGIN cannot be less than or equal to FLOATGRACE."); 
                           return ; 
                   } 
    
           theChan->setLimitOffset(limit_offset); 

           theChan->commit(); 
    
           bot->Notice(theClient, "Floating-limit Margin now set to %i", limit_offset);
           
           return ;
  }
    if(option == "FLOATGRACE") 
           { 
               if(level < level::set::floatlim) 
               { 
                   bot->Notice(theClient, 
                                   bot->getResponse(theUser, 
                                   language::insuf_access, 
                                   string("You do not have enough access!"))); 
                   return ; 
               } 
    
           unsigned int limit_grace = atoi(value.c_str()); 
    
           if ((limit_grace < 0) | (limit_grace > 19)) 
                   { 
                           bot->Notice(theClient, "Invalid floating-grace setting (0-19 Allowed)."); 
                           return ; 
                   } 
    
           if (limit_grace > theChan->getLimitOffset()) 
                   { 
                           bot->Notice(theClient, "FLOATGRACE cannot be greater than FLOATMARGIN."); 
                           return ; 
                   } 
    
           theChan->setLimitGrace(limit_grace); 
           theChan->commit(); 
    
           bot->Notice(theClient, "Floating-limit grace now set to %i", limit_grace); 
           return ; 
           } 
    
           if(option == "FLOATMAX") 
           { 
               if(level < level::set::floatlim) 
               { 
                   bot->Notice(theClient, 
                                   bot->getResponse(theUser, 
                                   language::insuf_access, 
                                   string("You do not have enough access!"))); 
                   return ; 
               } 
    
           unsigned int limit_max = atoi(value.c_str()); 
    
           if ((limit_max < 0) | (limit_max > 65536)) 
                   { 
                           bot->Notice(theClient, "Invalid floating-limit max (0-65536 Allowed)."); 
                           return ; 
                   } 
    
    
           theChan->setLimitMax(limit_max); 
           theChan->commit(); 
    
           if (!limit_max) 
           { 
                   bot->Notice(theClient, "Floating-limit MAX setting has now been disabled."); 
           } else { 
                   bot->Notice(theClient, "Floating-limit max now set to %i", limit_max); 
           } 

           return ; 
           }


  bot->Notice(theClient,
    bot->getResponse(theUser,
      language::mode_invalid,
      string("ERROR: Invalid channel setting.")));
  return ;
}

} // namespace gnuworld.


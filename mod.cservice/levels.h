#ifndef __LEVELS_H
#define __LEVELS_H "$Id: levels.h,v 1.20 2002-02-19 01:30:57 jeekay Exp $"

/* 
 * levels.h
 *
 * 20/12/2000 - Perry Lorier <perry@coders.net>
 * Initial Version.
 * 
 * Defines access level constants for use in command handlers. 
 *
 * $Id: levels.h,v 1.20 2002-02-19 01:30:57 jeekay Exp $
 */

namespace gnuworld {

 namespace level {
  const int access = 0;
  const int chaninfo = 0;
  const int deauth = 0; // Depreciated?
  const int help = 0;
  const int motd = 0;
  
  const int banlist = 1;
  const int force = 1; // And by definition, unforce.
  const int globalbanlist = 1;
  const int lbanlist = 1;
  const int status = 1;
  
  const int voice = 25;
  const int devoice = 25;
  const int invite = 24;
  
  const int kick = 50;
  const int topic = 50;
  
  const int ban = 75;
  const int unban = 75;
  
  const int deop = 100;
  const int op = 100;
  const int suspend = 100;
  const int unsuspend = 100;
  
  const int status2 = 100; // Users can see the channel modes too
  
  const int masskick = 200;
  const int massban = 200;

  const int adduser = 400;
  const int clearmode = 400;
  const int modinfo = 400;
  const int remuser = 400;

  const int join = 450;
	const int mode = 450;
  const int part = 450;
  const int setcmd = 450;

  const int logs = 501; // Level that logs are visible at 
  const int invme = 501;
  const int remignore = 501;
  const int chancomment = 501;

  const int addcommentcmd = 600;
  const int usercomment = 600;
  const int purge = 600; 
  const int registercmd = 600;
  const int removeall = 600;
  const int csuspend = 600; // Level required to suspend a channel

  const int globalsuspend = 750;
  const int nsuspend = 750; // Level required to suspend a nick

  const int force2 = 850; // Allow forcing of NOFORCE channels
  const int chgadmin = 850; // changing ppl on * (add/del/mod/susp/unsusp)
	const int globnotice = 850;
  
  const int rehash = 900;
  const int say = 900;
  const int servnotice = 900;

  // Debug Commands
  const int shutdown = 900;
  const int quote = 950;
  const int boostall = 1000;
 
  namespace set {
#ifdef FEATURE_INVITE
    const int autoinvite = 24;
#endif
    const int alwaysop = 450;
    const int userflag = 450;
    const int autotopic = 450;
    const int url = 450;
    const int keywords = 450;
    const int floatlim = 450;
    const int desc = 450;
    const int mode = 450;
    const int welcome = 450;

    const int massdeoppro = 500;
    const int noop = 500;
    const int oponly = 500;
#ifdef FEATURE_STRICTVOICE
    const int strictvoice = 500;
#endif
    const int strictop = 500;
    const int lang = 500;
    const int floodpro = 500;
    const int autojoin = 500;
    
    const int comment = 501;
    const int locked = 501;

    const int nopurge = 800;
    const int special = 800;
    const int noreg = 800;
    const int neverreg = 800;
    const int suspend = 600;
    const int tempman = 800;
    const int caution = 800;
    const int vacation = 800;
    const int noforce = 900;
  }

  namespace immune {
    const int massdeop = 450;
    const int suspendop = 450; // Immune from op'ing a suspended user.
    const int floodpro = 501;
  }
 
  namespace admin {
	const int base = 1;
	const int supervisor = 800;
	const int director = 850;
	const int amanager = 890;
	const int manager = 900;
	const int coder = 1000;
  }

  namespace coder {
	const int base = 1;
	const int contrib = 200;
	const int devel = 400;
	const int senior = 499;
  }

  namespace virusfix {
        const int base = 100;
  }


 }
}

#endif

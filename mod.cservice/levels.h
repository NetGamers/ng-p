#ifndef __LEVELS_H
#define __LEVELS_H "$Id: levels.h,v 1.3 2002-01-17 19:24:04 morpheus Exp $"

/* 
 * levels.h
 *
 * 20/12/2000 - Perry Lorier <perry@coders.net>
 * Initial Version.
 * 
 * Defines access level constants for use in command handlers. 
 *
 * $Id: levels.h,v 1.3 2002-01-17 19:24:04 morpheus Exp $
 */

namespace gnuworld {

 namespace level {
  const int access = 0;
  const int banlist = 0;
  const int chaninfo = 0;
  const int deauth = 0; // Depreciated?
  const int help = 0;
  const int lbanlist = 0;
  const int map = 0;
  const int motd = 0;
  
  const int status = 1;
  const int force = 1; // And by definition, unforce.
  
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
  
  const int masskick = 200;
  const int status2 = 100; // Users can see the channel modes too
  
  const int adduser = 400;
  const int clearmode = 400;
  const int modinfo = 400;
  const int remuser = 400;

  const int join = 450;
  const int part = 450;
  const int setcmd = 450;

  const int logs = 501; // Level that logs are visible at 
  
  const int invme = 501;
  
  const int globalbanlist = 600;

  const int addcommentcmd = 600;
  const int registercmd = 600;
  const int globalsuspend = 750;
  const int rehash = 900;

  const int purge = 600; 
  const int removeall = 600;

  const int remignore = 501;

  const int servnotice = 900;
  const int say = 900;

  // Debug Commands
  const int shutdown = 900;
  const int quote = 950;
  const int boostall = 1000;
 
  namespace set {
#ifdef FEATURE_INVITE
    const int autoinvite = 25;
#endif
    const int alwaysop = 450;
    const int userflag = 450;
    const int autotopic = 450;
    const int url = 450;
    const int keywords = 450;
    const int floatlim = 450;
    const int desc = 450;
    const int mode = 450;

    const int massdeoppro = 500;
    const int noop = 500;
    const int oponly = 500;
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
  	const int jradmin = 500;
	const int sradmin = 600;
	const int jrweb = 750;
	const int srweb = 800;
	const int supervisor = 850;
	const int psersonal = 900;
	const int manager = 900;
	const int webcoder = 951;
	const int coder = 1000;
  }

  namespace coder {
	const int base = 1;
	const int contrib = 200;
	const int devel = 400;
	const int senior = 499;
  }

 }
}

#endif

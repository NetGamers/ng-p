#ifndef __LEVELS_H
#define __LEVELS_H "$Id: levels.h,v 1.39 2003-01-14 17:08:12 jeekay Exp $"

/* 
 * levels.h
 *
 * 20/12/2000 - Perry Lorier <perry@coders.net>
 * Initial Version.
 * 
 * Defines access level constants for use in command handlers. 
 *
 * $Id: levels.h,v 1.39 2003-01-14 17:08:12 jeekay Exp $
 */

namespace gnuworld {

 namespace level {
  const int access = 0;
  const int chaninfo = 0;
  const int deauth = 0; // Depreciated?
  const int help = 0;
  const int motd = 0;
  
  const int banlist = 1;
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
    const int invisible = 450;

// Owner commands moved to 490
    const int massdeoppro = 490;
    const int noop = 490;
    const int strictop = 490;
    const int lang = 490;
#ifdef FEATURE_STRICTVOICE
    const int strictvoice = 490;
#endif

// Owner commands

    const int floodpro = 500;
    const int autojoin = 500;

// Admin commands

    const int caution = 400;
    const int comment = 400;

    const int locked = 450;
    const int vacation = 450;

    const int oponly = 500;
    

    const int tempman = 550;

    const int neverreg = 800;
    const int noreg = 800;

    const int nopurge = 850;
    const int special = 850;

    const int noforce = 900;
  }

	namespace chinfo {
		const int email = 750;
		const int nick = 750;
		const int verification = 750;
		
		const int autokill = 800;
		
		const int admin = 850; // Level required to use chinfo on a *
	}

  namespace immune {
    const int massdeop = 450;
    const int suspendop = 450; // Immune from op'ing a suspended user.
    const int floodpro = 501;
  }
 
  namespace admin {
	  const int base = 1;
    const int cadmin = 400;
    const int nadmin = 700;
	  const int director = 800;
  	const int amanager = 850;
	  const int manager = 900;
	  const int coder = 901;
  }

  namespace coder {
	const int base = 100;
	const int contrib = 200;
	const int devel = 400;
	const int senior = 499;
  }

 }
}

#endif

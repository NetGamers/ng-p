#ifndef __CONSTANTS_H
#define __CONSTANTS_H "$Id: constants.h,v 1.12 2004-11-26 22:39:52 jeekay Exp $"

/*
 * constants.h
 *
 * 27/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 *
 * Defines constants used throughout the application.
 *
 * $Id: constants.h,v 1.12 2004-11-26 22:39:52 jeekay Exp $
 */

namespace gnuworld
{
namespace sql
	{
	/*
	 *  Comma seperated lists of fields for use in retrieving various
	 *  articles of data.
	 */
	const string ban_fields = "id,channel_id,banmask,set_by,set_ts,level,expires,reason,last_updated";
	const string channel_fields = "id,name,flags,mass_deop_pro,flood_pro,url,description,comment,keywords,registered_ts,channel_ts,channel_mode,userflags,last_updated,limit_offset,limit_period,limit_grace,limit_max,welcome,suspend_expires_ts,invisible";
	const string command_fields = "command_name,domain,level,flags,comment,description,last_updated,last_updated_by";
	const string config_fields = "name,contents";
	const string level_fields = "channel_id,user_id,access,flags,suspend_expires,suspend_level,suspend_by,added,added_by,last_Modif,last_Modif_By,last_Updated";
	const string user_fields = "users.id,users.user_name,users.password,users.url,users.language_id,users.flags,users.last_updated_by,users.last_updated,users.email,users.comment,users.suspended_expire_ts,users.question_id,users.verificationdata,users.maxlogins,verify";
	const string verify_fields = "id,title";
	}
}

#endif

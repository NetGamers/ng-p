#ifndef __RESPONSES_H
#define __RESPONSES_H

/* 
 * responses.h
 *
 * 28/12/2000 - Greg Sikorski <gte@atomicrevs.demon.co.uk>
 * Initial Version.
 * 
 * Defines 'response' ID's, unique references to textual responses
 * from the bot.
 */

namespace gnuworld
	{
	namespace language
	{
		const int already_authed =			1;
		const int auth_success =			2;
		const int insuf_access =			3;
		const int chan_is_empty =			4;
		const int dont_see_them =			5;
		const int cant_find_on_chan =			6;
		const int chan_not_reg =			7; 
		const int youre_opped_by =			8; 
		const int youre_voiced_by =			9;
		const int youre_not_in_chan =			10;
		const int already_opped =			11;
		const int already_voiced =			12;
		const int youre_deopped_by =			13;
		const int youre_devoiced_by =			14;
		const int not_opped =				15;
		const int not_voiced =				16;
		const int auth_failed =				17;
		const int i_am_not_on_chan =			18;
		const int ban_level_range =			19;
		const int ban_duration =			20;
		const int ban_reason_size =			21;
		const int ban_exists =				22;
		const int ban_covered =				23;
		const int inval_min_lvl =			24;
		const int inval_max_lvl =			25;
		const int user_access_is =			26;
		const int channel_automode_is =			27;
		const int last_mod =				28;
		const int suspend_expires_in =			29;
		const int last_seen =				30;
		const int more_than_max =			31;
		const int restrict_query =			32;
		const int end_access_list =			33;
		const int no_match =				34;
		const int access_higher =			35;
		const int inval_access =			36;
		const int already_in_list =			37;
		const int add_success =				38;
		const int its_bad_mmkay =			39;
		const int end_ban_list =			40;

		// "Unable to view user details (Invisible)"
		const int no_peeking =				41;
		const int info_about =				42; 
		const int curr_logged_on =			43;
		const int url =					44;
		const int lang =				45;
		const int channels =				46;
		const int inp_flood =				47;
		const int out_flood =				48;
		const int reg_by =				49;
		const int last_seen_info =			50;
		const int desc =				51;
		const int flood_msg =				52;
		const int i_almost_flooded =			53;
		const int incomplete_cmd =			54;
		const int warn_secure =				55;
		const int not_logged_in =			56;
		const int chan_suspended =			57;
		const int user_suspended =			58;
		const int warn_noop =				59;
		const int warn_strictop =			60;
		const int warn_exceeded_maxdeop =		61;
		const int syntax =				62;
		const int temp_inc_access =			63;
		const int is_reg =				64;
		const int is_not_reg =				65;
		const int wouldnt_appreciate =			66;
		const int lbanlist_for =			67;
		const int lban_entry =				68;
		const int lban_added_by =			69;
		const int lban_since =				70;
		const int lban_exp =				71;
		const int lban_end =				72;

		// user, not chan!
		const int not_registered =			73;
		const int no_longer_auth =			74;
		const int doesnt_have_access =			75;
		const int mod_access_higher =			76;
		const int cant_give_higher =			77;
		const int mod_access_to =			78;
		const int automode_op =				79;
		const int automode_voice =			80;
		const int automode_none =			81;
		const int pass_cant_be_nick =			82;
		const int pass_changed =			83;
		const int noop_set =				84;
		const int strictop_not_authed =			85;
		const int strictop_insuf_access =		86;
		const int purged_chan =				87;
		const int chan_already_reg =			88;
		const int inval_chan_name =			89;
		const int regged_chan =				90;
		const int unsilenced =				91;
		const int couldnt_find_silence =		92;
		const int cant_rem_higher =			93;
		const int cant_rem_owner_self =			94;
		const int removed_user =			95;
		const int invis_on =				96;
		const int invis_off =				97;
		const int set_cmd_status =			98;
		const int set_cmd_syntax_on_off =		99;
		const int userflags_syntax =			100;
		const int userflags_status =			101;
		const int massdeoppro_syntax =			102;
		const int massdeoppro_status =			103;
		const int floodpro_syntax =			104;
		const int floodpro_status =			105;
		const int desc_max_len =			106;
		const int desc_cleared =			107;
		const int desc_status =				108;
		const int url_max_len =				109;
		const int url_cleared =				110;
		const int url_status =				111;
		const int keywords_max_len =			112;
		const int keywords_status =			113;
		const int lang_set_to =				114;
		const int lang_invalid =			115;
		const int no_such_chan =			116;
		const int mode_invalid =			117;
		const int ignore_list_start =			118;
		const int ignore_list_empty =			119;
		const int ignore_list_end =			120;
		const int status_tagline =			121;
		const int status_chan_rec =			122;
		const int status_user_rec =			123;
		const int status_access_rec =			124;
		const int status_ban_rec =			125;
		const int status_last_user_not =		126;
		const int status_last_chan_not =		127;
		const int status_last_lvl_not =			128;
		const int status_last_ban_not =			129;
		const int status_data_alloc =			130;
		const int status_uptime =			131;
		const int status_chan_info =			132;
		const int status_mode =				133;
		const int status_flags =			134;
		const int suspend_access_higher =		135;
		const int bogus_time =				136;
		const int inval_suspend_dur =			137;
		const int susp_cancelled =			138;
		const int already_susp =			139;
		const int susp_set =				140;
		const int topic_max_len =			141;
		const int cant_rem_ban =			142;
		const int bans_removed =			143;
		const int rem_temp_access =			144;
		const int no_forced_access =			145;
		const int isnt_suspended =			146;
		const int is_an_ircop =				147;
		const int is_not_logged_in =			148;
		const int logged_in_as =			149;
		const int is_cservice_rep =			150;
		const int is_cservice_admin =			151;
		const int is_cservice_dev =			152;
		const int exc_search =				153;
		const int restrict_search =			154;
		const int no_search_match =			155;
		const int modeclear_done =			156;
		const int inv_invalid =				157;
		const int invalid_option =			158;
		const int is_service_bot =			159;
		const int is_coder_rep =			160;
		const int is_coder_contrib =			161;
		const int is_coder_devel =			162;
		const int is_coder_senior =			163;
		const int is_also_an_ircop = 			164;
		const int ban_added =				165;
		const int ban_list_empty = 			166;
		const int already_on_chan = 			167;
		const int ircops_only_cmd =			168;
		const int im_not_op =				169;
		const int rpl_ignorelist = 			170;
		const int im_not_opped = 			171;
		const int keywords = 				172;
		const int is_cservice_sradmin = 		173;
		const int automode_invite =			174;
		const int login_msg =				200;
		const int motd =				9999;

		// help command system
		const int help_access =				500;
		const int help_banlist =			501;
		const int help_chaninfo =			502;
		const int help_info =				503;
		const int help_lbanlist =			504;
		const int help_login =				505;
		const int help_motd =				506;
		const int help_newpass =			507;
		const int help_showcommands =			508;
		const int help_showignore =			509;
		const int help_verify =				510;
		const int help_killghost =			511;
		const int help_status =				512;
		const int help_voice =				513;
		const int help_devoice =			514;
		const int help_kick =				515;
		const int help_topic =				516;
		const int help_ban =				517;
		const int help_unban =				518;
		const int help_op =				519;
		const int help_deop =				520;
		const int help_invite =				521;
		const int help_suspend =			522;
		const int help_unsuspend =			523;
		const int help_adduser =			524;
		const int help_clearmode =			525;
		const int help_modinfo =			526;
		const int help_modinfo_access =			527;
		const int help_modinfo_automode =		528;
		const int help_remuser =			529;
		const int help_part =				530;
		const int help_join =				531;
		const int help_set =				532;
		const int help_set_alliance =			533;
		const int help_set_autojoin =			534;
		const int help_set_autokill =			535;
		const int help_set_autotopic =			536;
		const int help_set_coords =			537;
		const int help_set_description =		538;
		const int help_set_floodpro =			539;
		const int help_set_invisible =			540;
		const int help_set_keywords =			541;
		const int help_set_lang =			542;
		const int help_set_massdeoppro =		543;
		const int help_set_mode =			544;
		const int help_set_noop =			545;
		const int help_set_strictop =			546;
		const int help_set_url =			547;
		const int help_set_userflags =			548;
		const int help_register =			549;
		const int help_remignore =			550;
		const int help_invme =				551;
		const int help_purge =				552;
		const int help_removeall =			553;
		const int help_shutdown =			554;
		const int help_servnotice =			555;
		const int help_rehash =				556;
		const int help_say =				557;
	}
}

#endif

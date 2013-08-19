// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"

level_locals_t	level;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
  qboolean teamShader;        // track and if changed, update shader state
} cvarTable_t;

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

qboolean gDuelExit = qfalse;

vmCvar_t	g_trueJedi;

vmCvar_t	g_gametype;
vmCvar_t	g_MaxHolocronCarry;
vmCvar_t	g_ff_objectives;
vmCvar_t	g_autoMapCycle;
vmCvar_t	g_dmflags;
vmCvar_t	g_maxForceRank;
vmCvar_t	g_forceBasedTeams;
vmCvar_t	g_privateDuel;
vmCvar_t	g_saberLocking;
vmCvar_t	g_saberLockFactor;

vmCvar_t  twimod_statusprinthp;




vmCvar_t	hookChangeProtectTime;

vmCvar_t	twimod_highpingduel;
vmCvar_t	twimod_duelsabertoggle;

vmCvar_t	twimod_dueldistance;
vmCvar_t  twimod_itempush;
vmCvar_t	twimod_votecontrol_minmaptime;
vmCvar_t	twimod_allowDebugKnockMeDown;
vmCvar_t  twimod_sleepmsg;
vmCvar_t	twimod_flvl3jumphigh;
vmCvar_t 	twimod_loginpuplicmsg;
vmCvar_t  twimod_joinsound;
vmCvar_t  twimod_combatadmin;

vmCvar_t	twimod_slapstrength;
vmCvar_t	twimod_slapzstrength;
vmCvar_t	twimod_slapdowntime;

vmCvar_t	twimod_chatprotecttimer;
vmCvar_t	twimod_chatprotect;

vmCvar_t  twimod_clantagpcounter;
vmCvar_t	twimod_allowselfkill;
vmCvar_t	twimod_allowduelselfkill;
vmCvar_t	twimod_clantagprotect;
vmCvar_t	twimod_clantag;

vmCvar_t  twimod_allowemotes;
vmCvar_t  twimod_emotebreak;
vmCvar_t  twimod_antipadawan;
vmCvar_t  twimod_padanewname;
vmCvar_t	twimod_votecontrol_allowmapvote;
vmCvar_t	twimod_votecontrol_allowgametypevote;
vmCvar_t	twimod_votecontrol_allowSGTvote;
vmCvar_t	twimod_votecontrol_allowlimitvote;
vmCvar_t	twimod_votecontrol_allowkickvote;
vmCvar_t  twimod_bruteforceprotect;
vmCvar_t	twimod_kickmsg;
vmCvar_t	twimod_banmsg;
vmCvar_t  twimod_lvl1name;
vmCvar_t  twimod_lvl2name;
vmCvar_t  twimod_lvl3name;
vmCvar_t  twimod_lvl4name;
vmCvar_t  twimod_lvl5name;
vmCvar_t  twimod_lvl6name;

vmCvar_t	twimod_allowjetpack;
vmCvar_t	g_saberTraceSaberFirst;

#ifdef G2_COLLISION_ENABLED
vmCvar_t	g_saberGhoul2Collision;
#endif
vmCvar_t	g_saberAlwaysBoxTrace;
vmCvar_t	g_saberBoxTraceSize;

vmCvar_t	g_logClientInfo;

vmCvar_t	g_slowmoDuelEnd;

vmCvar_t	g_saberDamageScale;
vmCvar_t	twimod_blacknames;
vmCvar_t	g_forceRegenTime;
vmCvar_t	g_spawnInvulnerability;
vmCvar_t	g_forcePowerDisable;
vmCvar_t	g_weaponDisable;
vmCvar_t	g_duelWeaponDisable;
vmCvar_t	g_allowDuelSuicide;
vmCvar_t	g_fraglimitVoteCorrection;
vmCvar_t	g_fraglimit;
vmCvar_t	g_duel_fraglimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_capturelimit;
vmCvar_t	g_saberInterpolate;
vmCvar_t	g_friendlyFire;
vmCvar_t	g_friendlySaber;
vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
vmCvar_t	g_quadfactor;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_weaponTeamRespawn;
vmCvar_t	g_adaptRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;
vmCvar_t	g_warmup;
vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t	g_log;
vmCvar_t	g_logSync;
vmCvar_t	g_statLog;
vmCvar_t	g_statLogFile;
vmCvar_t	g_blood;
vmCvar_t	g_podiumDist;
vmCvar_t	g_podiumDrop;
vmCvar_t	g_allowVote;
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_banIPs;
vmCvar_t	g_filterBan;
vmCvar_t	g_debugForward;
vmCvar_t	g_debugRight;
vmCvar_t	g_debugUp;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	g_rankings;
vmCvar_t	g_listEntity;
vmCvar_t	g_redteam;
vmCvar_t	g_blueteam;
vmCvar_t	g_singlePlayer;
vmCvar_t	g_enableDust;
vmCvar_t	g_enableBreath;
vmCvar_t	g_dismember;
vmCvar_t	g_forceDodge;
vmCvar_t	g_timeouttospec;

vmCvar_t	g_saberDmgVelocityScale;
vmCvar_t	g_saberDmgDelay_Idle;
vmCvar_t	g_saberDmgDelay_Wound;

vmCvar_t	g_saberDebugPrint;
vmCvar_t	twimod_motd;
vmCvar_t	twimod_motdtime;
vmCvar_t	g_austrian;

vmCvar_t	mc_rocket_vel;
vmCvar_t	mc_rocket_alt_think;
vmCvar_t	mc_rocket_damage;
vmCvar_t	mc_rocket_splash_damage;
vmCvar_t	mc_rocket_splash_radius;
vmCvar_t	mc_stun_baton_damage;
vmCvar_t	mc_stun_baton_altdamage;
vmCvar_t	mc_demp_radius;
vmCvar_t	mc_demp_altdamage;
vmCvar_t	mc_demp_altrange;
vmCvar_t	mc_disrupter_damage;
vmCvar_t	mc_disrupter_altdamage;
vmCvar_t	mc_spawnmessage;
vmCvar_t	mc_sentrylimit;
vmCvar_t	mc_spawntimelimit;
vmCvar_t	mc_teleportspeed;
vmCvar_t	mc_emp_dmg_mod;
vmCvar_t	mc_blaster_vel;
vmCvar_t	mc_bryar_vel;
vmCvar_t	mc_bowcaster_vel;
vmCvar_t	mc_repeater_vel;
vmCvar_t	mc_repeater_alt_vel;
vmCvar_t	mc_demp2_vel;
vmCvar_t	mc_flechette_vel;
vmCvar_t	mc_flechette_shots;
vmCvar_t	mc_bowcaster_spread;
vmCvar_t	mc_bowcaster_alt_bounces;
vmCvar_t	mc_rocket_locktime;
vmCvar_t	mc_rocket_bounces;
vmCvar_t	mc_emplaced_weap;
vmCvar_t	mc_tripmine_limit;
vmCvar_t	mc_detpack_limit;
vmCvar_t	mc_weapons_impactsky;
vmCvar_t	mc_weapons_life;
vmCvar_t	mc_bryar_bounces;
vmCvar_t	mc_blaster_bounces;
vmCvar_t	mc_turretweap_bounces;
vmCvar_t	mc_seekershot_bounces;
vmCvar_t	mc_bowcaster_bounces;
vmCvar_t	mc_repeater_bounces;
vmCvar_t	mc_repeater_alt_bounces;
vmCvar_t	mc_demp2_bounces;
vmCvar_t	mc_flechette_bounces;
vmCvar_t	mc_flechette_alt_bounces;
vmCvar_t	mc_seekershot_vel;
vmCvar_t	mc_flechette_alt_vel;
vmCvar_t	mc_drown_delay;
vmCvar_t	mc_water_damage;
vmCvar_t	mc_water_damage_max;

vmCvar_t	mc_bryar_firetime;
vmCvar_t	mc_blaster_firetime;
vmCvar_t	mc_stunbaton_firetime;
vmCvar_t	mc_disruptor_firetime;
vmCvar_t	mc_bowcaster_firetime;
vmCvar_t	mc_repeater_firetime;
vmCvar_t	mc_flechette_firetime;
vmCvar_t	mc_rocket_launcher_firetime;
vmCvar_t	mc_demp2_firetime;
vmCvar_t	mc_detpack_firetime;
vmCvar_t	mc_tripmine_firetime;
vmCvar_t	mc_thermal_firetime;
vmCvar_t	mc_emplaced_firetime;

vmCvar_t	mc_bryar_alt_firetime;
vmCvar_t	mc_blaster_alt_firetime;
vmCvar_t	mc_stunbaton_alt_firetime;
vmCvar_t	mc_disruptor_alt_firetime;
vmCvar_t	mc_bowcaster_alt_firetime;
vmCvar_t	mc_repeater_alt_firetime;
vmCvar_t	mc_flechette_alt_firetime;
vmCvar_t	mc_rocket_launcher_alt_firetime;
vmCvar_t	mc_demp2_alt_firetime;
vmCvar_t	mc_detpack_alt_firetime;
vmCvar_t	mc_tripmine_alt_firetime;
vmCvar_t	mc_thermal_alt_firetime;
vmCvar_t	mc_emplaced_alt_firetime;


vmCvar_t	mc_jetpack_forcedrain;
vmCvar_t	mc_jetpack_fueldrain;
vmCvar_t	mc_jetpack_fuelmax;
vmCvar_t	mc_jetpack_fuelregen;
vmCvar_t	mc_jetpack_effect;

vmCvar_t	mc_telefrag;

vmCvar_t	mc_group1_name;
vmCvar_t	mc_group2_name;
vmCvar_t	mc_group3_name;
vmCvar_t	mc_group4_name;
vmCvar_t	mc_group5_name;
vmCvar_t	mc_group6_name;

vmCvar_t	mc_aerial_emotes;
vmCvar_t	mc_detpack_life;
vmCvar_t	mc_detpack_speed;
vmCvar_t	mc_tripmine_speed;
vmCvar_t	mc_namelength;


vmCvar_t	mc_bryar_damage;
vmCvar_t	mc_bryar_alt_damage;
vmCvar_t	mc_bryar_chargetime;
vmCvar_t	mc_insta;
vmCvar_t	mc_jedimaster2;
vmCvar_t	mc_onlydefaults;
vmCvar_t	mc_nofakeplayers;

vmCvar_t	mc_userfolder;
vmCvar_t	mc_logfile;
vmCvar_t	mc_editfolder;
vmCvar_t	mc_rocketdist;
vmCvar_t	mc_addknockback;
vmCvar_t	mc_nevercrash;
vmCvar_t	mc_disruptorrange;
vmCvar_t	mc_maxbmodel;
vmCvar_t	mc_safemap;
vmCvar_t	mc_request_timeout;
vmCvar_t	mc_crash_fix;
vmCvar_t	mc_dualsaber;
vmCvar_t	mc_max_admin_rank;
vmCvar_t	mc_flechette_damage;
vmCvar_t	mc_lms;
vmCvar_t	mc_allitems;
vmCvar_t	mc_allow_amremote;
vmCvar_t	mc_disruptor_bounces;
vmCvar_t	mc_newbansystem;
vmCvar_t	mc_nobanmessage;
vmCvar_t	mc_weaponstealing;
vmCvar_t	mc_quietcvars;
vmCvar_t	mc_customhelp_name;
vmCvar_t	mc_customhelp_desc;
vmCvar_t	mc_customhelp_info;
vmCvar_t	mc_jedivmerc;
vmCvar_t	mc_newvotesystem;
vmCvar_t	mc_mapvotefix;
vmCvar_t	mc_afktime;
vmCvar_t	mc_showflagtime;
vmCvar_t	mc_votedelaytime;
vmCvar_t	mc_afkisdeadtime;
vmCvar_t	mc_falltodeathdeath;
vmCvar_t	mc_unlagged;
vmCvar_t	mc_fixjumpbug;
vmCvar_t	mc_zombies;
vmCvar_t	mc_lockmessage;
vmCvar_t	mc_maxsnaps;
vmCvar_t	mc_betterghosting;
vmCvar_t	mc_teleeffect;
vmCvar_t	mc_duelkickdamage;





int gDuelist1 = -1;
int gDuelist2 = -1;

// bk001129 - made static to avoid aliasing
static cvarTable_t		gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },

	// latched vars
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },
	{ &g_MaxHolocronCarry, "g_MaxHolocronCarry", "3", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },

	// change anytime vars
	{ &g_ff_objectives, "g_ff_objectives", "0", /*CVAR_SERVERINFO |*/  CVAR_NORESTART, 0, qtrue },

	{ &g_trueJedi, "g_jediVmerc", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qtrue },

	{ &g_autoMapCycle, "g_autoMapCycle", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },
	
	{ &g_maxForceRank, "g_maxForceRank", "6", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },
	{ &g_forceBasedTeams, "g_forceBasedTeams", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },
	{ &g_privateDuel, "g_privateDuel", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberLocking, "g_saberLocking", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberLockFactor, "g_saberLockFactor", "6", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberTraceSaberFirst, "g_saberTraceSaberFirst", "1", CVAR_ARCHIVE, 0, qtrue  },

#ifdef G2_COLLISION_ENABLED
	{ &g_saberGhoul2Collision, "g_saberGhoul2Collision", "0", 0, 0, qtrue  },
#endif
	{ &g_saberAlwaysBoxTrace, "g_saberAlwaysBoxTrace", "0", 0, 0, qtrue  },
	{ &g_saberBoxTraceSize, "g_saberBoxTraceSize", "2", 0, 0, qtrue  },

	{ &g_logClientInfo, "g_logClientInfo", "0", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_slowmoDuelEnd, "g_slowmoDuelEnd", "0", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_saberDamageScale, "g_saberDamageScale", "1", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_forceRegenTime, "g_forceRegenTime", "200", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },

	{ &g_spawnInvulnerability, "g_spawnInvulnerability", "3000", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_forcePowerDisable, "g_forcePowerDisable", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  },
	{ &g_weaponDisable, "g_weaponDisable", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  },
	{ &g_duelWeaponDisable, "g_duelWeaponDisable", "1", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  },

	{ &g_allowDuelSuicide, "g_allowDuelSuicide", "0", CVAR_ARCHIVE, 0, qtrue },

	{ &g_fraglimitVoteCorrection, "g_fraglimitVoteCorrection", "1", CVAR_ARCHIVE, 0, qtrue },

	{ &g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_duel_fraglimit, "duel_fraglimit", "10", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },

	{ &g_saberInterpolate, "g_saberInterpolate", "1", CVAR_ARCHIVE, 0, qtrue },

	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_friendlySaber, "g_friendlySaber", "0", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", 0, 0, qtrue  },
	{ &g_log, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_statLog, "g_statLog", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_statLogFile, "g_statLogFile", "statlog.log", CVAR_ARCHIVE, 0, qfalse },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse  },

	{ &g_speed, "g_speed", "250", 0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue  },
	{ &g_knockback, "g_knockback", "1000", 0, 0, qtrue  },
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue  },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "5", 0, 0, qtrue },
	{ &g_adaptRespawn, "g_adaptrespawn", "1", 0, 0, qtrue  },		// Make weapons respawn faster with a lot of players.
	{ &g_forcerespawn, "g_forcerespawn", "60", 0, 0, qtrue },		// One minute force respawn.  Give a player enough time to reallocate force.
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse },
	{ &g_blood, "com_blood", "1", 0, 0, qfalse },

	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },

#if 0
	{ &g_debugForward, "g_debugForward", "0", 0, 0, qfalse },
	{ &g_debugRight, "g_debugRight", "0", 0, 0, qfalse },
	{ &g_debugUp, "g_debugUp", "0", 0, 0, qfalse },
#endif

	{ &g_redteam, "g_redteam", "Empire", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue },
	{ &g_blueteam, "g_blueteam", "Rebellion", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue  },
	{ &g_singlePlayer, "ui_singlePlayerActive", "", 0, 0, qfalse, qfalse  },
	{ &g_enableDust, "g_enableDust", "0", 0, 0, qtrue, qfalse },
	{ &g_enableBreath, "g_enableBreath", "0", 0, 0, qtrue, qfalse },
	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},
	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse},
	{ &g_dismember, "g_dismember", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_forceDodge, "g_forceDodge", "1", 0, 0, qtrue  },
	{ &g_timeouttospec, "g_timeouttospec", "70", CVAR_ARCHIVE, 0, qfalse },
	{ &g_saberDmgVelocityScale, "g_saberDmgVelocityScale", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberDmgDelay_Idle, "g_saberDmgDelay_Idle", "350", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberDmgDelay_Wound, "g_saberDmgDelay_Wound", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberDebugPrint, "g_saberDebugPrint", "0", CVAR_CHEAT, 0, qfalse  },
	{ &twimod_votecontrol_minmaptime, "twimod_votecontrol_minmaptime", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_itempush, "twimod_itempush", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_dueldistance, "twimod_dueldistance", "1024", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_duelsabertoggle, "twimod_duelsabertoggle", "1", 0, 0, qfalse  },
	{ &twimod_highpingduel, "twimod_highpingduel", "none", 0, 0, qfalse  },
	{ &twimod_statusprinthp, "twimod_statusprinthp", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_allowDebugKnockMeDown, "twimod_allowDebugKnockMeDown", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_sleepmsg, "twimod_sleepmsg", "none", 0, 0, qfalse  },
	{ &twimod_flvl3jumphigh, "twimod_flvl3jumphigh", "384", 0, 0, qfalse  },
	{ &twimod_joinsound, "twimod_joinsound", "none", 0, 0, qfalse  },
	{ &twimod_loginpuplicmsg, "twimod_loginpuplicmsg", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_slapstrength, "twimod_slapstrength", "8000", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_slapzstrength, "twimod_slapzstrength", "8000", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_slapdowntime, "twimod_slapdowntime", "3", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_chatprotect, "twimod_chatprotect", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_chatprotecttimer, "twimod_chatprotecttimer", "3", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_clantagpcounter, "twimod_clantagpcounter", "30000", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_combatadmin, "twimod_combatadmin", "0", 0, 0, qfalse  },
	{ &twimod_padanewname, "twimod_padanewname", "^2Noob", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_emotebreak, "twimod_emotebreak", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_allowselfkill, "twimod_allowselfkill", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_allowduelselfkill, "twimod_allowduelselfkill", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_allowemotes, "twimod_allowemotes", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_clantag, "twimod_clantag", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_clantagprotect, "twimod_clantagprotect", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_antipadawan, "twimod_antipadawan", "0", 0, 0, qfalse  },
	{ &twimod_bruteforceprotect, "twimod_bruteforceprotect", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_allowjetpack, "twimod_allowjetpack", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_votecontrol_allowmapvote, "twimod_votecontrol_allowmapvote", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_votecontrol_allowgametypevote, "twimod_votecontrol_allowgametypevote", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_votecontrol_allowSGTvote, "twimod_votecontrol_allowSGTvote", "1", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_votecontrol_allowkickvote, "twimod_votecontrol_allowkickvote", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_votecontrol_allowlimitvote, "twimod_votecontrol_allowlimitvote", "0", CVAR_ARCHIVE, 0, qfalse  },
	//{ &twimod_lvl1name, "twimod_lvl1name", "^2Level^7-^21", 0, 0, qfalse  },
	//{ &twimod_lvl2name, "twimod_lvl2name", "^2Level^7-^22", 0, 0, qfalse  },
	//{ &twimod_lvl3name, "twimod_lvl3name", "^2Level^7-^23", 0, 0, qfalse  },
	//{ &twimod_lvl4name, "twimod_lvl4name", "^2Level^7-^24", 0, 0, qfalse  },
	//{ &twimod_lvl5name, "twimod_lvl5name", "^2Level^7-^25", 0, 0, qfalse  },
	//{ &twimod_lvl6name, "twimod_lvl6name", "^2Level^7-^26", 0, 0, qfalse  },
	{ &twimod_blacknames, "twimod_blacknames", "1", 0, 0, qtrue },
	{ &twimod_motdtime, "twimod_motdtime", "6", 0, 0, qtrue  },
	{ &twimod_motd, "twimod_motd", "^5Twi^1Fire^7 Mod", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_austrian, "g_austrian", "0", 0, 0, qfalse  },
	{ &mc_rocket_vel, "mc_rocket_vel", "900", 0, 0, qfalse  },
	{ &mc_rocket_alt_think, "mc_rocket_alt_think", "100", 0, 0, qfalse  },
	{ &mc_rocket_damage, "mc_rocket_damage", "100", 0, 0, qfalse  },
	{ &mc_rocket_splash_damage, "mc_rocket_splash_damage", "100", 0, 0, qfalse  },
	{ &mc_rocket_splash_radius, "mc_rocket_splash_radius", "160", 0, 0, qfalse  },
	{ &mc_stun_baton_damage, "mc_stun_baton_damage", "20", 0, 0, qfalse  },
	{ &mc_stun_baton_altdamage, "mc_stun_baton_altdamage", "20", 0, 0, qfalse  },
	{ &mc_demp_radius, "mc_demp_radius", "256", 0, 0, qfalse  },
	{ &mc_demp_altdamage, "mc_demp_altdamage", "8", 0, 0, qfalse  },
	{ &mc_demp_altrange, "mc_demp_altrange", "4096", 0, 0, qfalse  },
	{ &mc_disrupter_damage, "mc_disrupter_damage", "30", 0, 0, qfalse  },
	{ &mc_disrupter_altdamage, "mc_disrupter_altdamage", "100", 0, 0, qfalse  },
	{ &mc_spawnmessage, "mc_spawnmessage", "^1Twifire Mod By mcmonkey!^7", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_sentrylimit, "mc_sentrylimit", "1", 0, 0, qfalse  },
	{ &mc_spawntimelimit, "mc_spawntimelimit", "60", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_teleportspeed, "mc_teleportspeed", "400", CVAR_ARCHIVE, 0, qfalse  },
	//{ &mc_emp_dmg_mod, "mc_emp_dmg_mod", "2", 0, 0, qfalse  },
	{ &mc_blaster_vel, "mc_blaster_vel", "2300", 0, 0, qfalse  },
	{ &mc_bryar_vel, "mc_bryar_vel", "1600", 0, 0, qfalse  },
	{ &mc_bowcaster_vel, "mc_bowcaster_vel", "1300", 0, 0, qfalse  },
	{ &mc_repeater_vel, "mc_repeater_vel", "1600", 0, 0, qfalse  },
	{ &mc_repeater_alt_vel, "mc_repeater_alt_vel", "1100", 0, 0, qfalse  },
	{ &mc_demp2_vel, "mc_demp2_vel", "1800", 0, 0, qfalse  },
	{ &mc_flechette_vel, "mc_flechette_vel", "3500", 0, 0, qfalse  },
	{ &mc_flechette_shots, "mc_flechette_shots", "5", 0, 0, qfalse  },
	{ &mc_bowcaster_spread, "mc_bowcaster_spread", "5", 0, 0, qfalse  },
	{ &mc_bowcaster_alt_bounces, "mc_bowcaster_alt_bounces", "3", 0, 0, qfalse  },
	{ &mc_rocket_locktime, "mc_rocket_locktime", "10", 0, 0, qfalse  },
	{ &mc_rocket_bounces, "mc_rocket_bounces", "0", 0, 0, qfalse  },
	{ &mc_emplaced_weap, "mc_emplaced_weap", "0", 0, 0, qfalse  },
	{ &mc_tripmine_limit, "mc_tripmine_limit", "9", 0, 0, qfalse  },
	{ &mc_detpack_limit, "mc_detpack_limit", "9", 0, 0, qfalse  },
	{ &mc_weapons_impactsky, "mc_weapons_impactsky", "0", 0, 0, qfalse  },
	{ &mc_weapons_life, "mc_weapons_life", "10000", 0, 0, qfalse  },
	{ &mc_bryar_bounces, "mc_bryar_bounces", "0", 0, 0, qfalse  },
	{ &mc_blaster_bounces, "mc_blaster_bounces", "0", 0, 0, qfalse  },
	{ &mc_turretweap_bounces, "mc_turretweap_bounces", "0", 0, 0, qfalse  },
	{ &mc_seekershot_bounces, "mc_seekershot_bounces", "0", 0, 0, qfalse  },
	{ &mc_bowcaster_bounces, "mc_bowcaster_bounces", "0", 0, 0, qfalse  },
	{ &mc_repeater_bounces, "mc_repeater_bounces", "0", 0, 0, qfalse  },
	{ &mc_repeater_alt_bounces, "mc_repeater_alt_bounces", "0", 0, 0, qfalse  },
	{ &mc_demp2_bounces, "mc_demp2_bounces", "0", 0, 0, qfalse  },
	{ &mc_flechette_bounces, "mc_flechette_bounces", "6", 0, 0, qfalse  },
	{ &mc_flechette_alt_bounces, "mc_flechette_alt_bounces", "50", 0, 0, qfalse  },
	{ &mc_seekershot_vel, "mc_seekershot_vel", "2000", 0, 0, qfalse  },
	{ &mc_flechette_alt_vel, "mc_flechette_alt_vel", "700", 0, 0, qfalse  },
	{ &mc_drown_delay, "mc_drown_delay", "1000", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_water_damage, "mc_water_damage", "2", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_water_damage_max, "mc_water_damage_max", "15", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_jetpack_forcedrain, "mc_jetpack_forcedrain", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_jetpack_fueldrain, "mc_jetpack_fueldrain", "2", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_jetpack_fuelmax, "mc_jetpack_fuelmax", "100", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_jetpack_fuelregen, "mc_jetpack_fuelregen", "5", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_bryar_firetime, "mc_bryar_firetime", "400", 0, 0, qfalse  },
	{ &mc_bryar_alt_firetime, "mc_bryar_alt_firetime", "400", 0, 0, qfalse  },
	{ &mc_blaster_firetime, "mc_blaster_firetime", "450", 0, 0, qfalse  },
	{ &mc_blaster_alt_firetime, "mc_blaster_alt_firetime", "150", 0, 0, qfalse  },
	{ &mc_disruptor_firetime, "mc_disruptor_firetime", "600", 0, 0, qfalse  },
	{ &mc_disruptor_alt_firetime, "mc_disruptor_alt_firetime", "1300", 0, 0, qfalse  },
	{ &mc_bowcaster_firetime, "mc_bowcaster_firetime", "1000", 0, 0, qfalse  },
	{ &mc_bowcaster_alt_firetime, "mc_bowcaster_alt_firetime", "750", 0, 0, qfalse  },
	{ &mc_repeater_firetime, "mc_repeater_firetime", "100", 0, 0, qfalse  },
	{ &mc_repeater_alt_firetime, "mc_repeater_alt_firetime", "800", 0, 0, qfalse  },
	{ &mc_demp2_firetime, "mc_demp2_firetime", "500", 0, 0, qfalse  },
	{ &mc_demp2_alt_firetime, "mc_demp2_alt_firetime", "900", 0, 0, qfalse  },
	{ &mc_flechette_firetime, "mc_flechette_firetime", "700", 0, 0, qfalse  },
	{ &mc_flechette_alt_firetime, "mc_flechette_alt_firetime", "800", 0, 0, qfalse  },
	{ &mc_rocket_launcher_firetime, "mc_rocket_launcher_firetime", "900", 0, 0, qfalse  },
	{ &mc_rocket_launcher_alt_firetime, "mc_rocket_launcher_alt_firetime", "1200", 0, 0, qfalse  },
	{ &mc_thermal_firetime, "mc_thermal_firetime", "800", 0, 0, qfalse  },
	{ &mc_thermal_alt_firetime, "mc_thermal_alt_firetime", "400", 0, 0, qfalse  },
	{ &mc_tripmine_firetime, "mc_tripmine_firetime", "800", 0, 0, qfalse  },
	{ &mc_tripmine_alt_firetime, "mc_tripmine_alt_firetime", "400", 0, 0, qfalse  },
	{ &mc_detpack_firetime, "mc_detpack_firetime", "800", 0, 0, qfalse  },
	{ &mc_detpack_alt_firetime, "mc_detpack_alt_firetime", "400", 0, 0, qfalse  },
	{ &mc_emplaced_firetime, "mc_emplaced_firetime", "100", 0, 0, qfalse  },
	{ &mc_emplaced_alt_firetime, "mc_emplaced_alt_firetime", "100", 0, 0, qfalse  },
	{ &mc_stunbaton_firetime, "mc_stunbaton_firetime", "400", 0, 0, qfalse  },
	{ &mc_stunbaton_alt_firetime, "mc_stunbaton_alt_firetime", "400", 0, 0, qfalse  },

	{ &mc_jetpack_effect, "mc_jetpack_effect_OLDOLDOLD", "env/fire", 0, 0, qfalse  },

	{ &mc_telefrag, "mc_telefrag", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &mc_group1_name, "mc_group1_name", "^7members of group^51^7", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_group2_name, "mc_group2_name", "^7members of group^52^7", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_group3_name, "mc_group3_name", "^7members of group^53^7", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_group4_name, "mc_group4_name", "^7members of group^54^7", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_group5_name, "mc_group5_name", "^7members of group^55^7", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_group6_name, "mc_group6_name", "^7members of group^56^7", CVAR_ARCHIVE, 0, qfalse  },

	{ &mc_aerial_emotes, "mc_aerial_emotes", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &mc_detpack_life, "mc_detpack_life", "30000", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_detpack_speed, "mc_detpack_speed", "300", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_tripmine_speed, "mc_tripmine_speed", "256", CVAR_ARCHIVE, 0, qfalse },

	{ &mc_namelength, "mc_namelength", "36", 0, 0, qfalse },

	{ &mc_bryar_damage, "mc_bryar_damage", "10", 0, 0, qfalse },
	{ &mc_bryar_alt_damage, "mc_bryar_alt_damage", "1", 0, 0, qfalse },
	{ &mc_bryar_chargetime, "mc_bryar_chargetime", "200", 0, 0, qfalse },
	{ &mc_insta, "mc_insta", "0", 0, 0, qfalse },
	{ &mc_jedimaster2, "mc_jedimaster2", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_onlydefaults, "mc_onlydefaults", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_nofakeplayers, "mc_nofakeplayers", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_userfolder, "mc_userfolder", "users", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_editfolder, "mc_editfolder", "mapedits", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_logfile, "mc_logfile", "mainlog", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_rocketdist, "mc_rocketdist", "2048", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_addknockback, "mc_addknockback", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_nevercrash, "mc_nevercrash", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_disruptorrange, "mc_disruptorrange", "8192", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_maxbmodel, "mc_maxbmodel", "150", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_safemap, "mc_safemap", "ffa_yavin", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_request_timeout, "mc_request_timeout", "30", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_crash_fix, "mc_crash_fix", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_dualsaber, "mc_dualsaber", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_max_admin_rank, "mc_max_admin_rank", "5", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_flechette_damage, "mc_flechette_damage", "12", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_lms, "mc_lms", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_allitems, "mc_allitems", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_allow_amremote, "mc_allow_amremote", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_disruptor_bounces, "mc_disruptor_bounces", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_newbansystem, "mc_newbansystem", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_nobanmessage, "mc_nobanmessage", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_weaponstealing, "mc_weaponstealing", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_quietcvars, "mc_quietcvars", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_customhelp_name, "mc_customhelp_name", "", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_customhelp_desc, "mc_customhelp_desc", "", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_customhelp_info, "mc_customhelp_info", "", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_jedivmerc, "mc_jedivmerc", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_newvotesystem, "mc_newvotesystem", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_mapvotefix, "mc_mapvotefix", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_afktime, "mc_afktime", "120", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_showflagtime, "mc_showflagtime", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_votedelaytime, "mc_votedelaytime", "60", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_afkisdeadtime, "mc_afkisdeadtime", "600", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_falltodeathdeath, "mc_falltodeathdeath", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_unlagged, "mc_unlagged", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_fixjumpbug, "mc_fixjumpbug", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_zombies, "mc_zombies", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_lockmessage, "mc_lockmessage", "Invalid password", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_maxsnaps, "mc_maxsnaps", "40", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_betterghosting, "mc_betterghosting", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_teleeffect, "mc_teleeffect", "", CVAR_ARCHIVE, 0, qfalse },
	{ &mc_duelkickdamage, "mc_duelkickdamage", "1", CVAR_ARCHIVE, 0, qfalse },



	{ &twimod_kickmsg, "twimod_kickmsg", "has been kicked from the server.", CVAR_ARCHIVE, 0, qfalse  },
	{ &twimod_banmsg, "twimod_banmsg", "has been banned from the server.", CVAR_ARCHIVE, 0, qfalse  },
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );


void G_InitGame					( int levelTime, int randomSeed, int restart );
void G_RunFrame					( int levelTime );
void G_ShutdownGame				( int restart );
void CheckExitRules				( void );
void G_ROFF_NotetrackCallback	( gentity_t *cent, const char *notetrack);


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (int)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0 );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0, qtrue );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
		G_RunFrame( arg0 );
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		return BotAIStartFrame( arg0 );
	case GAME_ROFF_NOTETRACK_CALLBACK:
		G_ROFF_NotetrackCallback( &g_entities[arg0], (const char *)arg1 );
	}

	return -1;
}


void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];
	fileHandle_t	Mz;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	trap_Printf( text );
	if (Q_stricmp(mc_logfile.string, "") == 0)
	{
		return;
	}
	trap_FS_FOpenFile(va("logs/%s.log", mc_logfile.string), &Mz, FS_APPEND);
	if ( !Mz )
	{
		return;
	}
	trap_FS_Write( text, strlen(text), Mz);
	trap_FS_FCloseFile( Mz );
}

void QDECL mc_print( const char *fmt, ... ) {

	va_list		argptr;
	char		text[1024];
	fileHandle_t	Mz;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);
	trap_Printf( text );
	if (Q_stricmp(mc_logfile.string, "") == 0)
	{
		return;
	}
	trap_FS_FOpenFile(va("logs/%s.log", mc_logfile.string), &Mz, FS_APPEND);
	if ( !Mz )
	{
		return;
	}
	trap_FS_Write( text, strlen(text), Mz);
	trap_FS_FCloseFile( Mz );
}

void QDECL mc_printERR( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];
	fileHandle_t	Mz;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);
	trap_Printf( text );
	trap_FS_FOpenFile(va("logs/%s_ERRORS.log", mc_logfile.string), &Mz, FS_APPEND);
	if ( !Mz )
	{
		return;
	}
	trap_FS_Write( text, strlen(text), Mz);
	trap_FS_FCloseFile( Mz );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];
	vmCvar_t		mapname;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);
	G_Printf(text);
	if (level.endall == qfalse)
	{
	G_Printf("--- Error --- %s --- %s ---", text, fmt);
	G_Printf("Error- %s- %s --- %s", mc_timer(), text, fmt);
	mc_printERR("Error- %s- %s --- %s\n", mc_timer(), text, fmt);
	//trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	//trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", mapname.string));
	if (mc_nevercrash.integer != 1)
	{
	trap_SendConsoleCommand( EXEC_APPEND, va(";g_gametype 0;rmap %s;\n", mc_safemap.string));
	}
	level.endall = qtrue;
	}
	//trap_Error( text );
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf ("%i teams with %i entities\n", c, c2);
}

void G_RemapTeamShaders( void ) {
#if 0
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf( string, sizeof(string), "team_icon/%s_red", g_redteam.string );
	AddRemap("textures/ctf2/redteam01", string, f); 
	AddRemap("textures/ctf2/redteam02", string, f); 
	Com_sprintf( string, sizeof(string), "team_icon/%s_blue", g_blueteam.string );
	AddRemap("textures/ctf2/blueteam01", string, f); 
	AddRemap("textures/ctf2/blueteam02", string, f); 
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;

		if (cv->teamShader) {
			remapped = qtrue;
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}

	// check some things
	if ( g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE ) {
		G_Printf( "g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer );
		trap_Cvar_Set( "g_gametype", "0" );
	}

	level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					if (mc_quietcvars.integer != 1)
					{
					trap_SendServerCommand( -1, va("print \"Cvar Control: %s changed to %s.\n\"", cv->cvarName, cv->vmCvar->string ) );
					mc_print(va("Cvar Control: %s changed to %s\n", cv->cvarName, cv->vmCvar->string ) );
					}
				}

				if (cv->teamShader) {
					remapped = qtrue;
				}
			}
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}
}

/*
============
G_InitGame

============
*/
void testwpshow(void);
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int					i;
		vmCvar_t	mapname;
	B_InitAlloc(); //make sure everything is clean


	srand( randomSeed );

	G_RegisterCvars();

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", __DATE__);
	G_ProcessIPBans();
	G_InitMemory();
	// set some level globals
	memset( &level, 0, sizeof( level ) );
	bans_init();
	level.rnextcheck = 5000;
	level.nextstatus = 5000;
	level.time = levelTime;
	level.startTime = levelTime;
	level.reFix = 0;
	level.thisistpm = 0;
	level.lmsnojoin = 0;
	level.lmsvote = 0;
	channels_init();
	level.statuswrite = 0;
	level.jetpack_effect = G_EffectIndex( "env/fire" );
	level.jetfx = 1;
	level.snd_fry = G_SoundIndex("sound/player/fry.wav");	// FIXME standing in lava / slime

	//trap_SP_RegisterServer("mp_svgame");

	if ( g_log.string[0] ) {
		if ( g_logSync.integer ) {
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_SYNC );
		} else {
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND );
		}
		if ( !level.logFile ) {
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_log.string );
		} else {
			char	serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", serverinfo );
		}
	} else {
		G_Printf( "Not logging to disk.\n" );
	}

	G_LogWeaponInit();

	G_InitWorldSession();

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i=0 ; i<level.maxclients ; i++ ) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ), 
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// initialize saga mode before spawning entities so we know
	// if we should remove any saga-related entities on spawn
	InitSagaMode();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();
	enableallitems();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if( g_gametype.integer >= GT_TEAM ) {
		G_CheckTeamItems();
	}
	else if ( g_gametype.integer == GT_JEDIMASTER )
	{
		trap_SetConfigstring ( CS_CLIENT_JEDIMASTER, "-1" );
	}

	trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("-1|-1") );
	trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("-1") );
	gDuelist1 = -1;
	gDuelist2 = -1;

	SaveRegisteredItems();

	G_Printf ("-----------------------------------\n");

	if( g_gametype.integer == GT_SINGLE_PLAYER || trap_Cvar_VariableIntegerValue( "com_buildScript" ) ) {
		G_ModelIndex( SP_PODIUM_MODEL );
		G_SoundIndex( "sound/player/gurp1.wav" );
		G_SoundIndex( "sound/player/gurp2.wav" );
	}

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}
	testwpshow();
	G_Printf(".\n");

	G_RemapTeamShaders();

	if ( g_gametype.integer == GT_TOURNAMENT )
	{
		G_LogPrintf("Duel Tournament Begun: kill limit %d, win limit: %d\n", g_fraglimit.integer, g_duel_fraglimit.integer );
	}
	testwpshow();
	teleporters_init();
	testwpshow();
	//if (qtrue)
	//{
		//fileHandle_t	f;
		level.mmdeletes = 0;
		level.mmmodels = 0;
		level.mmgmodels = 0;
		level.mmshaders = 0;
		level.mmeffects = 0;
		//level.mmfailbmodel = 0;
		//level.mmfailclass = 0;
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		//trap_FS_FOpenFile(va("%s/%s.cfg", mc_editfolder.string, mapname.string), &f, FS_READ);
		//if (f)
		//{
			trap_SendConsoleCommand( EXEC_INSERT, va( ";exec bans.cfg;exec defaults/credits.cfg;exec defaults/admin_free.cfg;exec %s/allmaps.cfg;exec %s/allmaps2.cfg;exec %s/mapedits_1_%s.cfg;exec %s/teleporters_1_%s.cfg;mapeditsdone;", mc_editfolder.string, mc_editfolder.string, mc_editfolder.string, mapname.string, mc_editfolder.string, mapname.string ) );
			//if (level.errorwarn > 0)
			//{
			//	trap_SendConsoleCommand( EXEC_APPEND, va("TFERROR %i;",level.errorwarn));
			//}
		//}
		//trap_SendConsoleCommand( EXEC_INSERT, va( "exec %s/allmaps.cfg", mc_editfolder.string ) );
	//}
	level.baht = level.time + 1000*60;
	testwpshow();
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) 
{
	if (level.safeend != 1)
	{
		//trap_SendConsoleCommand( EXEC_APPEND, va("rmap ffa_yavin;TFERROR 32;" ) );
	}
	G_Printf ("==== ShutdownGame ====\n");
	if (mc_crash_fix.integer == 1)
	{
		if (level.reFix > 0)
		{
			G_Printf("Safe shutdown, no crash.\n");
		}
		else
		{
			G_Printf("Bad shutdown! Error!\n");
			mc_printERR("Bad shutdown! Error!\n");
			trap_SendConsoleCommand( EXEC_APPEND, va(";mc_crash_fix 20;map ffa_yavin;\n" ) );
		}
	}
	G_LogWeaponOutput();

	if ( level.logFile ) {
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();

	trap_ROFF_Clean();

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAIShutdown( restart );
	}

	B_CleanupAlloc();
	

	
	 //clean up all allocations made with B_Alloc
}



//===================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	G_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	G_Printf ("%s", text);
}

#endif

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer( void ) {
	int			i;
	gclient_t	*client;
	gclient_t	*nextInLine;

	if ( level.numPlayingClients >= 2 ) {
		return;
	}

	// never change during intermission
//	if ( level.intermissiontime ) {
//		return;
//	}

	nextInLine = NULL;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD || 
			client->sess.spectatorClient < 0  ) {
			continue;
		}

		if ( !nextInLine || client->sess.spectatorTime < nextInLine->sess.spectatorTime ) {
			nextInLine = client;
		}
	}

	if ( !nextInLine ) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam( &g_entities[ nextInLine - level.clients ], "f" );
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

void RemoveDuelDrawLoser(void)
{
	int clFirst = 0;
	int clSec = 0;
	int clFailure = 0;

	if ( level.clients[ level.sortedClients[0] ].pers.connected != CON_CONNECTED )
	{
		return;
	}
	if ( level.clients[ level.sortedClients[1] ].pers.connected != CON_CONNECTED )
	{
		return;
	}

	clFirst = level.clients[ level.sortedClients[0] ].ps.stats[STAT_HEALTH] + level.clients[ level.sortedClients[0] ].ps.stats[STAT_ARMOR];
	clSec = level.clients[ level.sortedClients[1] ].ps.stats[STAT_HEALTH] + level.clients[ level.sortedClients[1] ].ps.stats[STAT_ARMOR];

	if (clFirst > clSec)
	{
		clFailure = 1;
	}
	else if (clSec > clFirst)
	{
		clFailure = 0;
	}
	else
	{
		clFailure = 2;
	}

	if (clFailure != 2)
	{
		SetTeam( &g_entities[ level.sortedClients[clFailure] ], "s" );
	}
	else
	{ //we could be more elegant about this, but oh well.
		SetTeam( &g_entities[ level.sortedClients[1] ], "s" );
	}
}

/*
=======================
RemoveTournamentWinner
=======================
*/
void RemoveTournamentWinner( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[0];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores( void ) {
	int			clientNum;

	if (level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE] ==
		level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE] &&
		level.clients[level.sortedClients[0]].pers.connected == CON_CONNECTED &&
		level.clients[level.sortedClients[1]].pers.connected == CON_CONNECTED)
	{
		int clFirst = level.clients[ level.sortedClients[0] ].ps.stats[STAT_HEALTH] + level.clients[ level.sortedClients[0] ].ps.stats[STAT_ARMOR];
		int clSec = level.clients[ level.sortedClients[1] ].ps.stats[STAT_HEALTH] + level.clients[ level.sortedClients[1] ].ps.stats[STAT_ARMOR];
		int clFailure = 0;
		int clSuccess = 0;

		if (clFirst > clSec)
		{
			clFailure = 1;
			clSuccess = 0;
		}
		else if (clSec > clFirst)
		{
			clFailure = 0;
			clSuccess = 1;
		}
		else
		{
			clFailure = 2;
			clSuccess = 2;
		}

		if (clFailure != 2)
		{
			clientNum = level.sortedClients[clSuccess];

			level.clients[ clientNum ].sess.wins++;
			ClientUserinfoChanged( clientNum );
			trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("%i", clientNum ) );

			clientNum = level.sortedClients[clFailure];

			level.clients[ clientNum ].sess.losses++;
			ClientUserinfoChanged( clientNum );
		}
		else
		{
			clSuccess = 0;
			clFailure = 1;

			clientNum = level.sortedClients[clSuccess];

			level.clients[ clientNum ].sess.wins++;
			ClientUserinfoChanged( clientNum );
			trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("%i", clientNum ) );

			clientNum = level.sortedClients[clFailure];

			level.clients[ clientNum ].sess.losses++;
			ClientUserinfoChanged( clientNum );
		}
	}
	else
	{
		clientNum = level.sortedClients[0];
		if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
			level.clients[ clientNum ].sess.wins++;
			ClientUserinfoChanged( clientNum );

			trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("%i", clientNum ) );
		}

		clientNum = level.sortedClients[1];
		if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
			level.clients[ clientNum ].sess.losses++;
			ClientUserinfoChanged( clientNum );
		}
	}
}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	gclient_t	*ca, *cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}


	// then spectators
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( ca->sess.spectatorTime < cb->sess.spectatorTime ) {
			return -1;
		}
		if ( ca->sess.spectatorTime > cb->sess.spectatorTime ) {
			return 1;
		}
		return 0;
	}
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return -1;
	}

	// then sort by score
	if ( ca->ps.persistant[PERS_SCORE]
		> cb->ps.persistant[PERS_SCORE] ) {
		return -1;
	}
	if ( ca->ps.persistant[PERS_SCORE]
		< cb->ps.persistant[PERS_SCORE] ) {
		return 1;
	}
	return 0;
}

qboolean gQueueScoreMessage = qfalse;
int gQueueScoreMessageTime = 0;

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int		i;
	int		rank;
	int		score;
	int		newScore;
	int		preNumSpec = 0;
	int		nonSpecIndex = -1;
	gclient_t	*cl;

	preNumSpec = level.numNonSpectatorClients;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots
	for ( i = 0; i < TEAM_NUM_TEAMS; i++ ) {
		level.numteamVotingClients[i] = 0;
	}
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
				level.numNonSpectatorClients++;
				nonSpecIndex = i;
			
				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) {
					level.numPlayingClients++;
					if ( !(g_entities[i].r.svFlags & SVF_BOT) ) {
						level.numVotingClients++;
						if ( level.clients[i].sess.sessionTeam == TEAM_RED )
							level.numteamVotingClients[0]++;
						else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
							level.numteamVotingClients[1]++;
					}
					if ( level.follow1 == -1 ) {
						level.follow1 = i;
					} else if ( level.follow2 == -1 ) {
						level.follow2 = i;
					}
				}
			}
		}
	}

	if (!g_warmup.integer)
	{
		level.warmupTime = 0;
	}

	/*
	if (level.numNonSpectatorClients == 2 && preNumSpec < 2 && nonSpecIndex != -1 && g_gametype.integer == GT_TOURNAMENT && !level.warmupTime)
	{
		gentity_t *currentWinner = G_GetDuelWinner(&level.clients[nonSpecIndex]);

		if (currentWinner && currentWinner->client)
		{
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
			currentWinner->client->pers.netname, G_GetStripEdString("SVINGAME", "VERSUS"), level.clients[nonSpecIndex].pers.netname));
		}
	}
	*/
	//NOTE: for now not doing this either. May use later if appropriate.

	qsort( level.sortedClients, level.numConnectedClients, 
		sizeof(level.sortedClients[0]), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numConnectedClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {	
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if ( i == 0 || newScore != score ) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) {
		trap_SetConfigstring( CS_SCORES1, va("%i", level.teamScores[TEAM_RED] ) );
		trap_SetConfigstring( CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE] ) );
	} else {
		if ( level.numConnectedClients == 0 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", SCORE_NOT_PRESENT) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else if ( level.numConnectedClients == 1 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] ) );
		}

		if (g_gametype.integer != GT_TOURNAMENT)
		{ //when not in duel, use this configstring to pass the index of the player currently in first place
			if ( level.numConnectedClients >= 1 )
			{
				trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("%i", level.sortedClients[0] ) );
			}
			else
			{
				trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );
			}
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission or in multi-frag Duel game mode, send the new info to everyone
	if ( level.intermissiontime || g_gametype.integer == GT_TOURNAMENT ) {
		gQueueScoreMessage = qtrue;
		gQueueScoreMessageTime = level.time + 500;
		//SendScoreboardMessageToAllClients();
		//rww - Made this operate on a "queue" system because it was causing large overflows
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( ent );
	}


	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) {
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if ( !ent ) {	// the map creator forgot to put in an intermission point...
		SelectSpawnPoint ( vec3_origin, level.intermission_origin, level.intermission_angle );
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

}

qboolean DuelLimitHit(void);

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int			i;
	gentity_t	*client;

	if ( level.intermissiontime ) {
		return;		// already active
	}

	// if in tournement mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );

		AdjustTournamentScores();
		if (DuelLimitHit())
		{
			gDuelExit = qtrue;
		}
		else
		{
			gDuelExit = qfalse;
		}
	}

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	if (g_singlePlayer.integer) {
		trap_Cvar_Set("ui_singlePlayerActive", "0");
		UpdateTournamentInfo();
	}

	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (client->health <= 0) {
			respawn(client);
		}
		MoveClientToIntermission( client );
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

}

qboolean DuelLimitHit(void)
{
	int i;
	gclient_t *cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		if ( g_duel_fraglimit.integer && cl->sess.wins >= g_duel_fraglimit.integer )
		{
			return qtrue;
		}
	}

	return qfalse;
}

void DuelResetWinsLosses(void)
{
	int i;
	gclient_t *cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		cl->sess.wins = 0;
		cl->sess.losses = 0;
	}
}

/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar 

=============
*/
void ExitLevel (void) {
	int		i;
	gclient_t *cl;

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if ( g_gametype.integer == GT_TOURNAMENT  ) {
		if (!DuelLimitHit())
		{
			if ( !level.restarted ) {
				trap_SendConsoleCommand( EXEC_APPEND, "rmap_restart 0\n" );
				level.restarted = qtrue;
				level.changemap = NULL;
				level.intermissiontime = 0;
			}
			return;	
		}

		DuelResetWinsLosses();
	}


	trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	vsprintf( string +7 , fmt,argptr );
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + 7 );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
	mc_print( va("LogPrint: %s",string) );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int				i, numSorted;
	gclient_t		*cl;
	qboolean		won = qtrue;
	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		G_LogPrintf( "red:%i  blue:%i\n",
			level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],	cl->pers.netname );
		if (g_singlePlayer.integer && g_gametype.integer == GT_TOURNAMENT) {
			if (g_entities[cl - level.clients].r.svFlags & SVF_BOT && cl->ps.persistant[PERS_RANK] == 0) {
				won = qfalse;
			}
		}
	}

	if (g_singlePlayer.integer) {
		if (g_gametype.integer >= GT_CTF) {
			won = level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE];
		}
		trap_SendConsoleCommand( EXEC_APPEND, (won) ? "spWin\n" : "spLose\n" );
	}
}

qboolean gDidDuelStuff = qfalse; //gets reset on game reinit

/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) {
	int			ready, notReady;
	int			i;
	gclient_t	*cl;
	int			readyMask;

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) {
			continue;
		}

		if ( cl->readyToExit ) {
			ready++;
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	if ( g_gametype.integer == GT_TOURNAMENT && !gDidDuelStuff &&
		(level.time > level.intermissiontime + 2000) )
	{
		gDidDuelStuff = qtrue;

		if ( g_austrian.integer )
		{
			G_LogPrintf("Duel Results:\n");
			//G_LogPrintf("Duel Time: %d\n", level.time );
			G_LogPrintf("winner: %s, score: %d, wins/losses: %d/%d\n", 
				level.clients[level.sortedClients[0]].pers.netname,
				level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE],
				level.clients[level.sortedClients[0]].sess.wins,
				level.clients[level.sortedClients[0]].sess.losses );
			G_LogPrintf("loser: %s, score: %d, wins/losses: %d/%d\n", 
				level.clients[level.sortedClients[1]].pers.netname,
				level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE],
				level.clients[level.sortedClients[1]].sess.wins,
				level.clients[level.sortedClients[1]].sess.losses );
		}
		// if we are running a tournement map, kick the loser to spectator status,
		// which will automatically grab the next spectator and restart
		if (!DuelLimitHit())
		{
			if (level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE] ==
				level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE] &&
				level.clients[level.sortedClients[0]].pers.connected == CON_CONNECTED &&
				level.clients[level.sortedClients[1]].pers.connected == CON_CONNECTED)
			{
				RemoveDuelDrawLoser();
			}
			else
			{
				RemoveTournamentLoser();
			}

			AddTournamentPlayer();

			if ( g_austrian.integer )
			{
				G_LogPrintf("Duel Initiated: %s %d/%d vs %s %d/%d, kill limit: %d\n", 
					level.clients[level.sortedClients[0]].pers.netname,
					level.clients[level.sortedClients[0]].sess.wins,
					level.clients[level.sortedClients[0]].sess.losses,
					level.clients[level.sortedClients[1]].pers.netname,
					level.clients[level.sortedClients[1]].sess.wins,
					level.clients[level.sortedClients[1]].sess.losses,
					g_fraglimit.integer );
			}
			
			if (level.numPlayingClients >= 2)
			{
				trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
				trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );

				gDuelist1 = level.sortedClients[0];
				gDuelist2 = level.sortedClients[1];
			}

			return;	
		}

		if ( g_austrian.integer )
		{
			G_LogPrintf("Duel Tournament Winner: %s wins/losses: %d/%d\n", 
				level.clients[level.sortedClients[0]].pers.netname,
				level.clients[level.sortedClients[0]].sess.wins,
				level.clients[level.sortedClients[0]].sess.losses );
		}
		//this means we hit the duel limit so reset the wins/losses
		//but still push the loser to the back of the line, and retain the order for
		//the map change
		if (level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE] ==
			level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE] &&
			level.clients[level.sortedClients[0]].pers.connected == CON_CONNECTED &&
			level.clients[level.sortedClients[1]].pers.connected == CON_CONNECTED)
		{
			RemoveDuelDrawLoser();
		}
		else
		{
			RemoveTournamentLoser();
		}

		AddTournamentPlayer();

		if (level.numPlayingClients >= 2)
		{
			trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
			trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );

			gDuelist1 = level.sortedClients[0];
			gDuelist2 = level.sortedClients[1];
		}
	}

	if (g_gametype.integer == GT_TOURNAMENT && !gDuelExit)
	{ //in duel, we have different behaviour for between-round intermissions
		if ( level.time > level.intermissiontime + 4000 )
		{ //automatically go to next after 4 seconds
			ExitLevel();
			return;
		}

		for (i=0 ; i< g_maxclients.integer ; i++)
		{ //being in a "ready" state is not necessary here, so clear it for everyone
		  //yes, I also thinking holding this in a ps value uniquely for each player
		  //is bad and wrong, but it wasn't my idea.
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED )
			{
				continue;
			}
			cl->ps.stats[STAT_CLIENTS_READY] = 0;
		}
		return;
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) {
		return;
	}

	// if nobody wants to go, clear timer
	if ( !ready ) {
		level.readyToExit = qfalse;
		return;
	}

	// if everyone wants to go, go now
	if ( !notReady ) {
		ExitLevel();
		return;
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 10000 ) {
		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) {
	int		a, b;

	if ( level.numPlayingClients < 2 ) {
		return qfalse;
	}
	
	if ( g_gametype.integer >= GT_TEAM ) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) {
 	int			i;
	gclient_t	*cl;
	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) {
		CheckIntermissionExit ();
		return;
	}
	if ((mc_lms.integer > 0)&&(level.lmsnojoin == 1))
	{
		int		count;
		int		winner;
		count = 0;
		winner = -1;
		for (i = 0;i < 32;i += 1)
		{
			gentity_t	*pl = &g_entities[i];
			if (pl && pl->inuse && pl->client && (pl->client->sess.sessionTeam == TEAM_FREE))
			{
				if (pl->client->ps.persistant[PERS_SCORE] <= 0)
				{
					SetTeam( pl, "s" );
					continue;
				}
				count += 1;
				winner = pl->s.number;
			}
		}
		if (count == 1)
		{
			gentity_t	*pl = &g_entities[winner];
			char	*fs;
			char	*sf;
			int	wins;
			if (pl->client->ps.persistant[PERS_SCORE] != 1)
			{
				fs = "s";
				sf = "v";
			}
			else
			{
				fs = "";
				sf = "f";
			}
			wins = pl->client->ps.persistant[PERS_SCORE];
			level.lmsnojoin = 2;
			SetTeam( pl, "s" );
			LogExit( "Kill limit hit." );
			trap_SendServerCommand( -1, va("print \"^7%s^7 won the LMS round with ^5%i^7 li%se%s remaining!\n\"", pl->client->pers.netname, wins, sf, fs));
			level.bahg = 32;
		}
	}
	if (gDoSlowMoDuel)
	{ //don't go to intermission while in slow motion
		return;
	}

	if (gEscaping)
	{
		int i = 0;
		int numLiveClients = 0;

		while (i < MAX_CLIENTS)
		{
			if (g_entities[i].inuse && g_entities[i].client && g_entities[i].health > 0)
			{
				if (g_entities[i].client->sess.sessionTeam != TEAM_SPECTATOR &&
					!(g_entities[i].client->ps.pm_flags & PMF_FOLLOW))
				{
					numLiveClients++;
				}
			}

			i++;
		}
		if (gEscapeTime < level.time)
		{
			gEscaping = qfalse;
			LogExit( "Escape time ended." );
			return;
		}
		if (!numLiveClients)
		{
			gEscaping = qfalse;
			LogExit( "Everyone failed to escape." );
			return;
		}
	}

	if ( level.intermissionQueued ) {
		int time = (g_singlePlayer.integer) ? SP_INTERMISSION_DELAY_TIME : INTERMISSION_DELAY_TIME;
		if ( level.time - level.intermissionQueued >= time ) {
			level.intermissionQueued = 0;
		level.bahg = 32;
			BeginIntermission();
		}
		return;
	}

	// check for sudden death
	if ( ScoreIsTied() ) {
		// always wait for sudden death
		if (g_gametype.integer != GT_TOURNAMENT || !g_timelimit.integer)
		{
			return;
		}
	}

	if ( g_timelimit.integer && !level.warmupTime ) {
		if ( level.time - level.startTime >= g_timelimit.integer*60000 ) {
//			trap_SendServerCommand( -1, "print \"Timelimit hit.\n\"");
			trap_SendServerCommand( -1, va("print \"%s.\n\"",G_GetStripEdString("SVINGAME", "TIMELIMIT_HIT")));
			LogExit( "Timelimit hit." );
			return;
		}
	}

	if ( level.numPlayingClients < 2 ) {
		return;
	}
	if (mc_lms.integer == 0)
	{
	if ( g_gametype.integer < GT_CTF && g_fraglimit.integer ) {
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, va("print \"Red %s\n\"", G_GetStripEdString("SVINGAME", "HIT_THE_KILL_LIMIT")) );
			LogExit( "Kill limit hit." );
		level.bahg = 32;
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, va("print \"Blue %s\n\"", G_GetStripEdString("SVINGAME", "HIT_THE_KILL_LIMIT")) );
			LogExit( "Kill limit hit." );
		level.bahg = 32;
			return;
		}

		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( cl->sess.sessionTeam != TEAM_FREE ) {
				continue;
			}

			if ( g_gametype.integer == GT_TOURNAMENT && g_duel_fraglimit.integer && cl->sess.wins >= g_duel_fraglimit.integer )
			{
				LogExit( "Duel limit hit." );
		level.bahg = 32;
				gDuelExit = qtrue;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the win limit.\n\"",
					cl->pers.netname ) );
				return;
			}

			if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer ) {
				LogExit( "Kill limit hit." );
		level.bahg = 32;
				gDuelExit = qfalse;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " %s.\n\"",
												cl->pers.netname,
												G_GetStripEdString("SVINGAME", "HIT_THE_KILL_LIMIT")
												) 
										);
				return;
			}
		}
	}

	if ( g_gametype.integer >= GT_CTF && g_capturelimit.integer ) {

		if ( level.teamScores[TEAM_RED] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
		level.bahg = 32;
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
		level.bahg = 32;
			return;
		}
	}
	}
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/

/*
=============
CheckTournament

Once a frame, check for changes in tournement player state
=============
*/
void CheckTournament( void ) {
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( level.numPlayingClients == 0 ) {
		return;
	}

	if ( g_gametype.integer == GT_TOURNAMENT ) {

		// pull in a spectator if needed
		if ( level.numPlayingClients < 2 ) {
			AddTournamentPlayer();

			if (level.numPlayingClients >= 2)
			{
				trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
				gDuelist1 = level.sortedClients[0];
				gDuelist2 = level.sortedClients[1];
			}
		}

		if (level.numPlayingClients >= 2)
		{
			if (gDuelist1 == -1 ||
				gDuelist2 == -1)
			{
				trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
				gDuelist1 = level.sortedClients[0];
				gDuelist2 = level.sortedClients[1];
				if ( g_austrian.integer )
				{
					G_LogPrintf("Duel Initiated: %s %d/%d vs %s %d/%d, kill limit: %d\n", 
						level.clients[level.sortedClients[0]].pers.netname,
						level.clients[level.sortedClients[0]].sess.wins,
						level.clients[level.sortedClients[0]].sess.losses,
						level.clients[level.sortedClients[1]].pers.netname,
						level.clients[level.sortedClients[1]].sess.wins,
						level.clients[level.sortedClients[1]].sess.losses,
						g_fraglimit.integer );
				}
				//trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
				//FIXME: This seems to cause problems. But we'd like to reset things whenever a new opponent is set.
			}
		}

		//rww - It seems we have decided there will be no warmup in duel.
		//if (!g_warmup.integer)
		{ //don't care about any of this stuff then, just add people and leave me alone
			level.warmupTime = 0;
			return;
		}
#if 0
		// if we don't have two players, go back to "waiting for players"
		if ( level.numPlayingClients != 2 ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );
			}
			return;
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if ( level.warmupTime < 0 ) {
			if ( level.numPlayingClients == 2 ) {
				// fudge by -1 to account for extra delays
				level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;

				if (level.warmupTime < (level.time + 3000))
				{ //rww - this is an unpleasent hack to keep the level from resetting completely on the client (this happens when two map_restarts are issued rapidly)
					level.warmupTime = level.time + 3000;
				}
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			}
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "rmap_restart 0\n" );
			level.restarted = qtrue;
			return;
		}
#endif
	} else if ( level.warmupTime != 0 ) {
		int		counts[TEAM_NUM_TEAMS];
		qboolean	notEnough = qfalse;

		if ( g_gametype.integer > GT_TEAM ) {
			counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( -1, TEAM_RED );

			if (counts[TEAM_RED] < 1 || counts[TEAM_BLUE] < 1) {
				notEnough = qtrue;
			}
		} else if ( level.numPlayingClients < 2 ) {
			notEnough = qtrue;
		}

		if ( notEnough ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );
			}
			return; // still waiting for team members
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if ( level.warmupTime < 0 ) {
			// fudge by -1 to account for extra delays
			level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "rmap_restart 0\n" );
			level.restarted = qtrue;
			return;
		}
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote( void ) {
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );

		if (level.votingGametype)
		{
			if (trap_Cvar_VariableIntegerValue("g_gametype") != level.votingGametypeTo)
			{ //If we're voting to a different game type, be sure to refresh all the map stuff
				const char *nextMap = G_RefreshNextMap(level.votingGametypeTo, qtrue);

				//if (nextMap && nextMap[0])
				//{
					if (level.votingGametypeTo >= GT_TEAM)
					{
						trap_SendConsoleCommand( EXEC_APPEND, va("rmap ctf_yavin\n" ) );
					}
					else if (level.votingGametypeTo == GT_TOURNAMENT)
					{
						trap_SendConsoleCommand( EXEC_APPEND, va("rmap duel_temple\n" ) );
					}
					else
					{
						trap_SendConsoleCommand( EXEC_APPEND, va("rmap ffa_yavin\n" ) );
					}
					//trap_SendConsoleCommand( EXEC_APPEND, va("rmap %s\n", nextMap ) );
				//}

			}
			else
			{ //otherwise, just leave the map until a restart
				//G_RefreshNextMap(level.votingGametypeTo, qfalse);
			}

			if (g_fraglimitVoteCorrection.integer)
			{ //This means to auto-correct fraglimit when voting to and from duel.
				int currentGT = trap_Cvar_VariableIntegerValue("g_gametype");
				int currentFL = trap_Cvar_VariableIntegerValue("fraglimit");

				if (level.votingGametypeTo == GT_TOURNAMENT && currentGT != GT_TOURNAMENT)
				{
					if (currentFL > 3 || !currentFL)
					{ //if voting to duel, and fraglimit is more than 3 (or unlimited), then set it down to 3
						//trap_SendConsoleCommand(EXEC_APPEND, "fraglimit 3\n");
					}
				}
				else if (level.votingGametypeTo != GT_TOURNAMENT && currentGT == GT_TOURNAMENT)
				{
					if (currentFL && currentFL < 20)
					{ //if voting from duel, an fraglimit is less than 20, then set it up to 20
						//trap_SendConsoleCommand(EXEC_APPEND, "fraglimit 20\n");
					}
				}
			}

			level.votingGametype = qfalse;
			level.votingGametypeTo = 0;
		}
	}
	if ( !level.voteTime ) {
		return;
	}
	if (level.lmsvote >= 1)
	{
		if ((level.lmsvote == 1)&&(level.time - level.voteTime >= 10000))
		{
			trap_SendServerCmd(-1, va("print \"^7The LMS round starts in 50 seconds.\n\""));
			trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime+30000 ) );
			trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			level.lmsvote = 2;
		}
		if ((level.lmsvote == 2)&&(level.time - level.voteTime >= 20000))
		{
			trap_SendServerCmd(-1, va("print \"^7The LMS round starts in 40 seconds.\n\""));
			trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime+30000 ) );
			trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			level.lmsvote = 3;
		}
		if ((level.lmsvote == 3)&&(level.time - level.voteTime >= 30000))
		{
			trap_SendServerCmd(-1, va("print \"^7The LMS round starts in 30 seconds.\n\""));
			trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime+30000 ) );
			trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			level.lmsvote = 4;
		}
		if ((level.lmsvote == 4)&&(level.time - level.voteTime >= 40000))
		{
			trap_SendServerCmd(-1, va("print \"^7The LMS round starts in 20 seconds.\n\""));
			trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime+30000 ) );
			trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			level.lmsvote = 5;
		}
		if ((level.lmsvote == 5)&&(level.time - level.voteTime >= 50000))
		{
			trap_SendServerCmd(-1, va("print \"^7The LMS round starts in 10 seconds.\n\""));
			trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime+30000 ) );
			trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			level.lmsvote = 6;
		}
		if ((level.lmsvote == 6)&&(level.time - level.voteTime >= 55000))
		{
			trap_SendServerCmd(-1, va("print \"^7The LMS round starts in 5 seconds.\n\""));
			trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime+30000 ) );
			trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			level.lmsvote = 7;
		}
		if (level.time - level.voteTime >= 60000)
		{
			int	i;
			level.lmsnojoin = 1;
			level.lmsvote = 0;
			for (i = 0;i < 32;i += 1)
			{
				gentity_t	*pl = &g_entities[i];
				if (pl && pl->inuse && pl->client && (pl->client->sess.sessionTeam == TEAM_FREE))
				{
					pl->client->ps.persistant[PERS_SCORE] = mc_lms.integer;
				}
			}
			trap_SendServerCmd(-1, va("print \"^7The LMS round has begun!\n\""));
			CalculateRanks();
			level.voteTime = 0;
			level.lmsvote = 0;
			trap_SetConfigstring( CS_VOTE_TIME, "" );

		}
		return;
	}
	if (level.vote_is_poll == 1)
	{
		if (level.voteYes > 32)
		{
			trap_SendServerCommand( -1, va("print \"^7Poll ~^2%s^7~ passed with ^2Everyone^7 for yes and ^2%i^7 for no.\n\"", level.pollstring, level.voteNo) );
			level.voteTime = 0;
			trap_SetConfigstring( CS_VOTE_TIME, "" );
			return;
		}
		if (level.voteNo > 32)
		{
			trap_SendServerCommand( -1, va("print \"^7Poll ~^2%s^7~ failed with ^2%i^7 for yes and ^2Everyone^7 for no.\n\"", level.pollstring, level.voteYes) );
			level.voteTime = 0;
			trap_SetConfigstring( CS_VOTE_TIME, "" );
			return;
		}
	}
	if ( level.time - level.voteTime >= VOTE_TIME ) {
		if (level.vote_is_poll == 0)
		{
			if (mc_newvotesystem.integer != 0)
			{
			if ( level.voteYes > level.voteNo )
			{
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEPASSED")) );
				level.voteExecuteTime = level.time + 3000;
				level.voteTime = 0;
				trap_SetConfigstring( CS_VOTE_TIME, "" );
				return;
			}
			else
			{
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEFAILED")) );
			}
			}
			else
			{
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEFAILED")) );
			}
		}
		else
		{
			if ( level.voteYes > level.voteNo )
			{
				trap_SendServerCommand( -1, va("print \"^7Poll ~^2%s^7~ passed with ^2%i^7 for yes and ^2%i^7 for no.\n\"", level.pollstring, level.voteYes, level.voteNo) );
			}
			else if ( level.voteNo > level.voteYes)
			{
				trap_SendServerCommand( -1, va("print \"^7Poll ~^2%s^7~ failed with ^2%i^7 for yes and ^2%i^7 for no.\n\"", level.pollstring, level.voteYes, level.voteNo) );
			}
			else
			{
				trap_SendServerCommand( -1, va("print \"^7Poll ~^2%s^7~ tied with ^2%i^7 for yes and ^2%i^7 for no.\n\"", level.pollstring, level.voteYes, level.voteNo) );
			}
		}
	} else {
		if ((level.vote_is_poll == 0)&&(mc_newvotesystem.integer == 0))
		{
			if ( level.voteYes > level.numVotingClients/2 )
			{
				// execute the command, then remove the vote
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEPASSED")) );
				level.voteExecuteTime = level.time + 3000;
				level.voteTime = 0;
				trap_SetConfigstring( CS_VOTE_TIME, "" );
			}
			else if ( level.voteNo >= level.numVotingClients/2 )
			{
				// same behavior as a timeout
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEFAILED")) );
				level.voteTime = 0;
				trap_SetConfigstring( CS_VOTE_TIME, "" );
			}
			else
			{
				// still waiting for a majority
				return;
			}
			return;
		}
		else
		{
			if ( level.voteYes > 32 )
			{
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEPASSED")) );
				level.voteExecuteTime = level.time + 3000;
				level.voteTime = 0;
				trap_SetConfigstring( CS_VOTE_TIME, "" );
			}
			else if ( level.voteNo >= 32 )
			{
				trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEFAILED")) );
				level.voteTime = 0;
				trap_SetConfigstring( CS_VOTE_TIME, "" );
			}
			return;
		}
	}
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand( i, message );
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader(int team, int client) {
	int i;

	if ( level.clients[client].pers.connected == CON_DISCONNECTED ) {
		PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
		return;
	}
	if (level.clients[client].sess.sessionTeam != team) {
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
		return;
	}
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader) {
			level.clients[i].sess.teamLeader = qfalse;
			ClientUserinfoChanged(i);
		}
	}
	level.clients[client].sess.teamLeader = qtrue;
	ClientUserinfoChanged( client );
	PrintTeam(team, va("print \"%s %s\n\"", level.clients[client].pers.netname, G_GetStripEdString("SVINGAME", "NEWTEAMLEADER")) );
}

/*
==================
CheckTeamLeader
==================
*/
void CheckTeamLeader( int team ) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader)
			break;
	}
	if (i >= level.maxclients) {
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			if (!(g_entities[i].r.svFlags & SVF_BOT)) {
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			level.clients[i].sess.teamLeader = qtrue;
			break;
		}
	}
}

/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote( int team ) {
	int cs_offset;

	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		return;
	}
	if ( level.time - level.teamVoteTime[cs_offset] >= VOTE_TIME ) {
		trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TEAMVOTEFAILED")) );
	} else {
		if ( level.teamVoteYes[cs_offset] > level.numteamVotingClients[cs_offset]/2 ) {
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TEAMVOTEPASSED")) );
			//
			if ( !Q_strncmp( "leader", level.teamVoteString[cs_offset], 6) ) {
				//set the team leader
				SetLeader(team, atoi(level.teamVoteString[cs_offset] + 7));
			}
			else {
				trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.teamVoteString[cs_offset] ) );
			}
		} else if ( level.teamVoteNo[cs_offset] >= level.numteamVotingClients[cs_offset]/2 ) {
			// same behavior as a timeout
			trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TEAMVOTEFAILED")) );
		} else {
			// still waiting for a majority
			return;
		}
	}
	level.teamVoteTime[cs_offset] = 0;
	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, "" );

}


/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) {
	float	thinktime;

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}
	
	ent->nextthink = 0;
	if (!ent->think) {
		//G_Error ( "NULL ent->think");
		return;
	}
	ent->think (ent);
}

int g_LastFrameTime = 0;
int g_TimeSinceLastFrame = 0;

qboolean gDoSlowMoDuel = qfalse;
int gSlowMoDuelTime = 0;

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/

void G_RunFrame( int levelTime ) {
	int			i;
	gentity_t	*ent;
	int			msec;
	int start, end;
	int	iPlayer;
	vmCvar_t		mapname;

	if (gDoSlowMoDuel)
	{
		if (level.restarted)
		{
			char buf[128];
			float tFVal = 0;

			trap_Cvar_VariableStringBuffer("timescale", buf, sizeof(buf));

			tFVal = atof(buf);

			trap_Cvar_Set("timescale", "1");
			if (tFVal == 1.0f)
			{
				gDoSlowMoDuel = qfalse;
			}
		}
		else
		{
			float timeDif = (level.time - gSlowMoDuelTime); //difference in time between when the slow motion was initiated and now
			float useDif = 0; //the difference to use when actually setting the timescale

			if (timeDif < 150)
			{
				trap_Cvar_Set("timescale", "0.1f");
			}
			else if (timeDif < 1150)
			{
				useDif = (timeDif/1000); //scale from 0.1 up to 1
				if (useDif < 0.1)
				{
					useDif = 0.1;
				}
				if (useDif > 1.0)
				{
					useDif = 1.0;
				}
				trap_Cvar_Set("timescale", va("%f", useDif));
			}
			else
			{
				char buf[128];
				float tFVal = 0;

				trap_Cvar_VariableStringBuffer("timescale", buf, sizeof(buf));

				tFVal = atof(buf);

				trap_Cvar_Set("timescale", "1");
				if (timeDif > 1500 && tFVal == 1.0f)
				{
					gDoSlowMoDuel = qfalse;
				}
			}
		}
	}

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) {
		return;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	msec = level.time - level.previousTime;

	g_TimeSinceLastFrame = (level.time - g_LastFrameTime);

	// get any cvar changes
	G_UpdateCvars();
	if (mc_lms.integer > 0)
	{
		for (i = 0;i < 32;i += 1)
		{
			gentity_t	*pl = &g_entities[i];
			if (pl && pl->client && pl->inuse && (pl->client->sess.sessionTeam == TEAM_FREE))
			{
				goto lawlnope;
			}
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "lmsbegin" );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "LMS Round Starting" );
		level.voteYes = 1;
		level.voteNo = 0;
		level.voteTime = level.time;
		level.lmsnojoin = 0;
		level.lmsvote = 1;
	}
	lawlnope:
	//
	// go through all allocated objects
	//
	start = trap_Milliseconds();
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if ( ent->client ) {
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				if (ent->s.eFlags & EF_SOUNDTRACKER)
				{ //don't trigger the event again..
					ent->s.event = 0;
					ent->s.eventParm = 0;
					ent->s.eType = 0;
					ent->eventTime = 0;
				}
				else
				{
					G_FreeEntity( ent );
					continue;
				}
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}

		if (mc_unlagged.integer != 1)
		{
		if ( ent->s.eType == ET_MISSILE ) {
			G_RunMissile( ent );
			continue;
		}
		}

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER ) {
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) 
		{
			G_CheckClientTimeouts ( ent );
			
			if((!level.intermissiontime)&&!(ent->client->ps.pm_flags&PMF_FOLLOW))
			{
				WP_ForcePowersUpdate(ent, &ent->client->pers.cmd );
				WP_SaberPositionUpdate(ent, &ent->client->pers.cmd);
			}
			G_RunClient( ent );
			continue;
		}

		G_RunThink( ent );
	}
	if (mc_unlagged.integer == 1)
	{
	G_TimeShiftAllClients( level.previousTime, NULL );

	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( ent->s.eType == ET_MISSILE ) {
			G_RunMissile( ent );
		}
	}

	G_UnTimeShiftAllClients( NULL );
	}
end = trap_Milliseconds();

	trap_ROFF_UpdateEntities();

start = trap_Milliseconds();
	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}
end = trap_Milliseconds();

	// see if it is time to do a tournement restart
	CheckTournament();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	// check team votes
	CheckTeamVote( TEAM_RED );
	CheckTeamVote( TEAM_BLUE );

	// for tracking changes
	CheckCvars();

	if (g_listEntity.integer) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	if (level.time >= level.baht)
	{
	if (level.bahg != 32)
	{
		for (iPlayer = 0;iPlayer < 31;iPlayer += 1)
		{
			gentity_t	*iPl2;
			iPl2 = &g_entities[iPlayer];
			if (!iPl2 || !iPl2->inuse || !iPl2->client)
			{
				continue;
			}
			if (iPl2->r.svFlags & SVF_BOT)
			{
				continue;
			}
			if ((mc_afkisdeadtime.integer != 0)&&(iPl2->client->sess.ticksAFK > mc_afkisdeadtime.integer))
			{
				continue;
			}
			if (iPl2->client->sess.sessionTeam == TEAM_SPECTATOR && iPl2->client->sess.commandtime + 24000 < level.time)
			{
				continue;
			}
			goto nomapfix;
		}
		if (Q_stricmp(mapname.string, mc_safemap.string) != 0)
		{
			// Somehow, this is neccessary. Compiler error?
			level.bahg = 32;
			level.bahg = 32;
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype 0;wait 50;rmap %s\n", mc_safemap.string));
			level.bahg = 32;
			level.bahg = 32;
		}
	}
	}
	nomapfix:
	if (level.time >= level.nextstatus)
	{
		level.nextstatus = level.time+5000;
		status_update();
	}
	if (level.time >= level.rnextcheck)
	{
		fileHandle_t	f;
		char		timebuffer[1024];
		level.rnextcheck = level.time + 60000;
		mc_timerup();
		/*trap_FS_FOpenFile("time2.dat", &f, FS_READ);
		if (f)
		{
			trap_FS_Read( timebuffer , 1024, f );
			trap_FS_FCloseFile( f );
			if ((float)atof(timebuffer) != level.lasthour)
			{
				int	plays;
				plays = 0;
				for (iPlayer = 0;iPlayer < 31;iPlayer += 1)
				{
					gentity_t	*iPl2;
					iPl2 = &g_entities[iPlayer];
					if (!iPl2 || !iPl2->inuse || !iPl2->client)
					{
						continue;
					}
					if (iPl2->r.svFlags & SVF_BOT)
					{
						continue;
					}
					plays += 1;
				}
				level.lasthour = (float)atof(timebuffer);
				G_Printf("TIME- %s -- MAP- %s -- PLAYERS- %i\n", mc_timer(), mapname.string, plays);
			}
		}
		level.rnextcheck = level.time + 60000;*/
	}
	if (level.time >= level.renextcheck)
	{
		Svcmd_mcStatus_f();
		level.renextcheck = level.time + 300000;
	}

	//At the end of the frame, send out the ghoul2 kill queue, if there is one
	if (level.otherframe == 1)
	{
		level.otherframe = 0;
	}
	else
	{
		level.otherframe = 1;
	}
	G_SendG2KillQueue();
	for (i = 0;i < 32;i += 1)
	{
		gentity_t	*mplayer;
		mplayer = &g_entities[i];
		if (mplayer && mplayer->inuse && mplayer->client)
		{
			if ( mplayer->client->ps.eFlags & EF_JETPACK_ACTIVE )
			{
				if (level.otherframe == 1)
				{
					if (level.jetfx == 1)
					{
						if (mplayer->client->sess.veh_isactive != 1)
						{
						vec3_t	newangle;
						G_PlayEffect_ID(level.jetpack_effect, mplayer->client->ps.origin, newangle);
						}
					}
				}
			}
			if (mplayer->client->sess.massgravity != 0)
			{
				int ji;
				for (ji = 0;ji < 1020;ji += 1)
				{
					gentity_t	*t;
					t = &g_entities[ji];
					if (t && t->inuse && t->s.eType == ET_MISSILE)
					{
						vec3_t	iVel, iDir, fwd, iMark;
						float d;
						VectorClear(iVel);
						VectorClear(iDir);
						VectorClear(fwd);
						VectorClear(iMark);
						iMark[0] = mplayer->r.currentOrigin[0] - t->r.currentOrigin[0];
						iMark[1] = mplayer->r.currentOrigin[1] - t->r.currentOrigin[1];
						iMark[2] = mplayer->r.currentOrigin[2]- t->r.currentOrigin[2];
						vectoangles( iMark, iDir );
						AngleVectors(iDir, fwd, NULL, NULL);
						d = (sqrt((mplayer->r.currentOrigin[0]-t->r.currentOrigin[0])*(mplayer->r.currentOrigin[0]-t->r.currentOrigin[0])+(mplayer->r.currentOrigin[1]-t->r.currentOrigin[1])*(mplayer->r.currentOrigin[1]-t->r.currentOrigin[1])+(mplayer->r.currentOrigin[2]-t->r.currentOrigin[2])*(mplayer->r.currentOrigin[2]-t->r.currentOrigin[2])))/10;
						t->s.pos.trDelta[0] += fwd[0]*(((float)mplayer->client->sess.massgravity)/(d));
						t->s.pos.trDelta[1] += fwd[1]*(((float)mplayer->client->sess.massgravity)/(d));
						t->s.pos.trDelta[2] += fwd[2]*(((float)mplayer->client->sess.massgravity)/(d));
					}
				}
			}
		}
	}

	if (gQueueScoreMessage)
	{
		if (gQueueScoreMessageTime < level.time)
		{
			SendScoreboardMessageToAllClients();

			gQueueScoreMessageTime = 0;
			gQueueScoreMessage = 0;
		}
	}

	g_LastFrameTime = level.time;

	if (mc_crash_fix.integer > 1)
	{
		trap_SendConsoleCommand( EXEC_INSERT, va(";mc_crash_fix 1;\n" ) );
	}
	if (mc_unlagged.integer == 1)
	{
		level.frameStartTime = trap_Milliseconds();
	}


}

const char *G_GetStripEdString(char *refSection, char *refName)
{
	/*
	static char text[1024]={0};
	trap_SP_GetStringTextString(va("%s_%s", refSection, refName), text, sizeof(text));
	return text;
	*/

	//Well, it would've been lovely doing it the above way, but it would mean mixing
	//languages for the client depending on what the server is. So we'll mark this as
	//a striped reference with @@@ and send the refname to the client, and when it goes
	//to print it will get scanned for the striped reference indication and dealt with
	//properly.
	static char text[1024]={0};
	Com_sprintf(text, sizeof(text), "@@@%s", refName);
	return text;
}
void mc_SetBrushModel(gentity_t *ent, const char *name)
{
	char	iBuf[64];
	int	i;
	vmCvar_t	mapname;
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	if (!name)
	{
		return;
	}
	if (strlen(name) < 2)
	{
		return;
	}
	if (name[0] != '*')
	{
		return;
	}
	if (mc_maxbmodel.integer != 0)
	{
		iBuf[0] = '0';
		for (i = 1;i < 20;i += 1)
		{
			//if (!(iBuf[i] == '*'))
			//{
				iBuf[i] = name[i];
			//}
		}
		//G_Printf("=%s -- %s -- %i=", name, iBuf, atoi(iBuf));
		if (atoi(iBuf) == 0)
		{
			G_FreeEntity(ent);
			//G_Printf("===============ErDelete!===============");
			return;
		}
		if (atoi(iBuf) > 127)
		{
		if ((Q_stricmp(mapname.string,"cairn_assembly") == 0)||
		(Q_stricmp(mapname.string,"bespin_streets") == 0)||
		(Q_stricmp(mapname.string,"bespin_platform") == 0)||
		(Q_stricmp(mapname.string,"doom_shields") == 0)||
		(Q_stricmp(mapname.string,"yavin_temple") == 0)||
		(Q_stricmp(mapname.string,"yavin_swamp") == 0)||
		(Q_stricmp(mapname.string,"yavin_trial") == 0)||
		(Q_stricmp(mapname.string,"yavin_canyon") == 0)||
		(Q_stricmp(mapname.string,"yavin_courtyard") == 0)||
		(Q_stricmp(mapname.string,"yavin_final") == 0)||
		(Q_stricmp(mapname.string,"pit") == 0)||
		(Q_stricmp(mapname.string,"valley") == 0)||
		(Q_stricmp(mapname.string,"artus_topside") == 0)||
		(Q_stricmp(mapname.string,"cairn_reactor") == 0))
		{
			G_FreeEntity(ent);
			//G_Printf("%i > %i == Ent Freed!", atoi(iBuf), mc_maxbmodel.integer);
			level.mmfailbmodel += 1;
			return;
		}
		}
		if (atoi(iBuf) >= mc_maxbmodel.integer)
		{
			G_FreeEntity(ent);
			//G_Printf("%i > %i == Ent Freed!", atoi(iBuf), mc_maxbmodel.integer);
			level.mmfailbmodel += 1;
			return;
		}
		else
		{
			//G_Printf("%i < %i == Ent Not Freed!", atoi(iBuf), mc_maxbmodel.integer);
		}
	}
	if (Q_stricmp(name, "") == 0)
	{
		G_FreeEntity(ent);
		//G_Printf("===============ErDelete!===============");
		level.mmfailbmodel += 1;
		return;
	}
	trap_SetBrushModel(ent, name);
}

int validcharacter_number(char chr)
{
	if (
		(chr == '0')||
		(chr == '1')||
		(chr == '2')||
		(chr == '3')||
		(chr == '4')||
		(chr == '5')||
		(chr == '6')||
		(chr == '7')||
		(chr == '8')||
		(chr == '9')
	)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
int validcharacter_letter(char chr)
{
	if (
		(chr == 'A')||
		(chr == 'B')||
		(chr == 'C')||
		(chr == 'D')||
		(chr == 'E')||
		(chr == 'F')||
		(chr == 'G')||
		(chr == 'H')||
		(chr == 'I')||
		(chr == 'J')||
		(chr == 'K')||
		(chr == 'L')||
		(chr == 'M')||
		(chr == 'N')||
		(chr == 'O')||
		(chr == 'P')||
		(chr == 'Q')||
		(chr == 'R')||
		(chr == 'S')||
		(chr == 'T')||
		(chr == 'U')||
		(chr == 'V')||
		(chr == 'W')||
		(chr == 'X')||
		(chr == 'Y')||
		(chr == 'Z')||
		(chr == 'a')||
		(chr == 'b')||
		(chr == 'c')||
		(chr == 'd')||
		(chr == 'e')||
		(chr == 'f')||
		(chr == 'g')||
		(chr == 'h')||
		(chr == 'i')||
		(chr == 'j')||
		(chr == 'k')||
		(chr == 'l')||
		(chr == 'm')||
		(chr == 'n')||
		(chr == 'o')||
		(chr == 'p')||
		(chr == 'q')||
		(chr == 'r')||
		(chr == 's')||
		(chr == 't')||
		(chr == 'u')||
		(chr == 'v')||
		(chr == 'w')||
		(chr == 'x')||
		(chr == 'y')||
		(chr == 'z')
	)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
void stringclear(char *str, int len)
{
	int	i;
	for (i = 0;i < len;i += 1)
	{
		str[i] = '';
	}
}
gteleporter_t		g_teleporters[256];
void teleporters_init( void )
{
	int i;
	for (i = 0;i <= 512;i += 1)
	{
		gteleporter_t *tele = &g_teleporters[i];
		if (tele)
		{
			/*tele->active = 0;
			VectorSet(tele->pos, 0, 0, 0);
			tele->telenum = i;
			tele->angle = 0;
			tele->type = 0;
			tele->type2 = 0;
			stringclear(tele->name,1024);
			stringclear(tele->group,1024);*/
			memset(tele, sizeof(gteleporter_t), 0);
		}
	}
}
void teleporters_save( void )
{
	int		i;
	vmCvar_t	mapname;
	char		savePath[1024];
	fileHandle_t	f;
	char		line[13000];
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "%s/teleporters_1_%s.cfg", mc_editfolder.string, mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_WRITE);
	if ( !f )
	{
		G_Printf("Error: Cannot access map teleport file.\n");
		return;
	}
	stringclear(line,12000);
	for (i = 0;i < 512;i += 1)
	{
		gteleporter_t	*tele = &g_teleporters[i];
		if (tele && (tele->active == 1))
		{
			strcpy(line, va("%s\nmcaddtele \"%s\" %i %i %i %i %i %i \"%s\";", line, tele->name, (int)tele->pos[0], (int)tele->pos[1], (int)tele->pos[2], tele->angle, tele->type, tele->type2, tele->group));
		}
	}
	strcpy(line, va("%s\n", line));
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
}
void teleporter_delete(int teleN)
{
	gteleporter_t	*tele;
	if (teleN > 511)
	{
		return;
	}
	tele = &g_teleporters[teleN];
	if (tele)
	{
		tele->active = 0;
		VectorSet(tele->pos, 0, 0, 0);
		stringclear(tele->name,1024);
		tele->angle = 0;
		tele->type = 0;
		tele->type2 = 0;
		stringclear(tele->group,1024);
		teleporters_save();
	}
}
void teleporter_add(vec3_t pos, char *name, int angle, int type, int type2, char *group)
{
	int	i;
	for (i = 0;i < 1024;i += 1)
	{
		if ((name[i] == ';') || (name[i] == '\n') || (name[i] == ' ') || (name[i] == '	'))
		{
			return;
		}
		if ((group[i] == ';') || (group[i] == '\n') || (group[i] == ' ') || (group[i] == '	'))
		{
			return;
		}
	}
	for (i = 0;i < 512;i += 1)
	{
		gteleporter_t	*tele = &g_teleporters[i];
		if (tele)
		{
			if (tele->active == 0)
			{
				tele->active = 1;
				VectorCopy(pos, tele->pos);
				tele->angle = angle;
				tele->type = type;
				tele->type2 = type2;
				strcpy(tele->name, name);
				strcpy(tele->group, group);
				teleporters_save();
				return;
			}
		}
	}
}

/*
void save_all_notes( void )
{
	int		i;
	fileHandle_t	Mz;
	int		len;
	char		line[2048];
	int		fi;
	trap_FS_FOpenFile(va("%s/tpm_notes.cfg", mc_editfolder.string), &Mz, FS_WRITE);
	trap_FS_Write( "\n", strlen("\n"), Mz);
	trap_FS_FCloseFile( Mz );
	trap_FS_FOpenFile(va("%s/tpm_notes2.cfg", mc_editfolder.string), &Mz, FS_WRITE);
	trap_FS_Write( "\n", strlen("\n"), Mz);
	trap_FS_FCloseFile( Mz );
	trap_FS_FOpenFile(va("%s/tpm_notes3.cfg", mc_editfolder.string), &Mz, FS_WRITE);
	trap_FS_Write( "\n", strlen("\n"), Mz);
	trap_FS_FCloseFile( Mz );
	stringclear(line,2040);
	trap_FS_FOpenFile(va("%s/tpm_notes.cfg", mc_editfolder.string), &Mz, FS_APPEND);
	len = 0;
	fi = 1;
	for (i = 0;i < 1020;i += 1)
	{
		gentity_t	*chosenent = &g_entities[i];
		if (Q_stricmp(chosenent->classname,"mc_note") == 0)
		{
				stringclear(line,2040);
				Com_sprintf( line, sizeof(line), "\naddnotercon \"%s\" \"%s\" %i %i %i %i\n", chosenent->mctargetname,
				chosenent->mcmessage,
				(int)chosenent->s.bolt1,
				(int)chosenent->s.origin[0],
				 (int)chosenent->s.origin[1],
				 (int)chosenent->s.origin[2]);
				trap_FS_Write( line, strlen(line), Mz);
				len += strlen(line);
				if (len > 11000)
				{
					fi += 1;
					trap_FS_FCloseFile( Mz );
					trap_FS_FOpenFile(va("%s/tpm_notes%i.cfg", mc_editfolder.string, fi), &Mz, FS_APPEND);
					len = 0;
				}
		}
	}
	trap_FS_FCloseFile( Mz );
}
*/

void string_zero(char *str, int len)
{
	int	i;
	for (i = 0;i < len;i += 1)
	{
		str[i] = '0';
	}
}



gchannel_t	g_channels[64];

void channels_print(int ch, const char *msg)
{
	int		i;
	gchannel_t	*chan = &g_channels[ch];
	char		msg2[2048];
	stringclear(msg2,2000);
	strcpy(msg2, msg);
	if ((!chan) || (chan->active == 0) || (Q_stricmp(chan->name, "") == 0))
	{
		return;
	}
	for (i = 0;i < 32;i += 1)
	{
		if ((chan->inchannel[i] == 'A') || (g_entities[i].client->sess.monchan == 1))
		{
			trap_SendServerCommand(i, va("chat \"^5[%s^5]^7%s\"", chan->name, msg2));
		}
	}
}
void channels_newadmin(int ch, int cl)
{
	gentity_t	*ent = &g_entities[cl];
	channels_print(ch, va("^7%s^5 is now a channel admin.", ent->client->pers.netname));
}
void channels_quit(int ch, int cl)
{
	gentity_t	*ent = &g_entities[cl];
	channels_print(ch, va("^7%s^5 left the channel.", ent->client->pers.netname));
}
void channels_joined(int ch, int cl)
{
	gentity_t	*ent = &g_entities[cl];
	channels_print(ch, va("^7%s^5 joined the channel.", ent->client->pers.netname));
}
void channels_remove(int ch, int cl, int si)
{
	gchannel_t	*chan = &g_channels[ch];
	if (chan)
	{
		int	i;
		int	client;
		int	admin;
		admin = -1;
		client = 0;
		if (chan->inchannel[cl] == '0')
		{
			return;
		}
		chan->inchannel[cl] = '0';
		chan->isadmin[cl] = '0';
		for (i = 0;i < 32;i += 1)
		{
			if (chan->inchannel[i] == 'A')
			{
				client = i;
			}
		}
		if (client == 0)
		{
			channels_delete(ch);
			return;
		}
		if (si == 0)
		{
			channels_quit(ch, cl);
		}
		for (i = 0;i < 32;i += 1)
		{
			if (chan->isadmin[i] == 'A')
			{
				admin = i;
			}
		}
		if (admin == -1)
		{
			chan->isadmin[client] = 'A';
			channels_newadmin(ch, client);
		}
	}
}
void channels_remove_all (int cl)
{
	int	i;
	for (i = 0;i < 63;i += 1)
	{
		channels_remove(i, cl, 1);
	}
}
void channels_remove_all_nosi (int cl)
{
	int	i;
	for (i = 0;i < 63;i += 1)
	{
		channels_remove(i, cl, 0);
	}
}
void channels_unban_all (int cl)
{
	int	i;
	for (i = 0;i < 63;i += 1)
	{
		g_channels[i].isbanned[cl] = '0';
	}
}
int channels_create (char *name, char *pass)
{
	gchannel_t	*chan;
	int		i;
	for (i = 0;i < 62;i += 1)
	{
		chan = &g_channels[i];
		if ((Q_stricmp(name, chan->name) == 0))
		{
			return -2;
		}
	}
	for (i = 0;i < 62;i += 1)
	{
		chan = &g_channels[i];
		if (chan->active == 0)
		{
			strcpy(chan->name, name);
			strcpy(chan->password, pass);
			chan->active = 1;
			return i;
		}
	}
	G_Printf("Too many channels!\n");
	return -1;
}
void channels_delete ( int ch)
{
	gchannel_t	*chan = &g_channels[ch];
	channels_print(ch, "Channel closed.");
	if (chan)
	{
		chan->active = 0;
		stringclear(chan->name,1020);
		stringclear(chan->password,1020);
		string_zero(chan->inchannel,62);
		string_zero(chan->isadmin,62);
		string_zero(chan->isbanned,62);
	}
}
int channels_find(char *channel)
{
	int	i;
	for (i = 0;i < 62;i += 1)
	{
		if (g_channels[i].active == 1)
		{
			//G_Printf("%i: %s vs %s: ", i, channel, g_channels[i].name);
			if ((Q_stricmp(g_channels[i].name, channel) == 0))//||(strstr(g_channels[i].name,channel)))
			{
				//G_Printf("yes!\n");
				return i;
			}
			//G_Printf("No.\n");
		}
		else
		{
			//G_Printf("Skip %i\n", i);
		}
	}
	//G_Printf("Gave up.\n");
	return -1;
}
void channels_init( void )
{
	int	i;
	for (i = 0;i < 63;i += 1)
	{
		channels_delete(i);
	}
}

char *nameforgametype(int gt)
{
	switch (gt)
	{
		case 0: return "FFA_Free For All";
		case 1: return "HFFA_Holocron Free For All";
		case 2: return "Jedi Master";
		case 3: return "Duel";
		case 5: return "TFFA_Team Free For All";
		case 7: return "CTF_Capture The Flag";
		case 8: return "CTY_Capture The Ysalimari";
		default: return "Unknown";
	}
}
char *nameforteam(int te)
{
	switch (te)
	{
		case TEAM_FREE: return "Free";
		case TEAM_SPECTATOR: return "Spectator";
		case TEAM_RED: return "Red";
		case TEAM_BLUE: return "Blue";
		default: return "Unknown";
	}
}

char *mcm_nameclean(char *name)
{
	char	newname[1024];
	int	iLn;
	int	inpos;
	int	i;
	if (!name)
	{
		return "?";
	}
	iLn = strlen(name);
	if (iLn > 1020)
	{
		iLn = 1020;
	}
	if (iLn < 2)
	{
		return "?";
	}
	stringclear(newname, 1021);
	inpos = 1;
		if ((name[0] == '\n')||
			(name[0] == '&')||
			(name[0] == '<')||
			(name[0] == '>')
			)
		{
			newname[0] = ' ';
		}
		else
		{
			newname[0] = name[0];
		}
	for (i = 1;i < iLn;i += 1)
	{
		if ((name[i] == '\n')||
			(name[i] == '&')||
			(name[i] == '<')||
			(name[i] == '>')
			)
		{
			continue;
		}
		if (name[i-1] == '^')
		{
			if ((name[i] == '1')||
				(name[i] == '2')||
				(name[i] == '3')||
				(name[i] == '4')||
				(name[i] == '5')||
				(name[i] == '6')||
				(name[i] == '7')||
				(name[i] == '8')||
				(name[i] == '9')||
				(name[i] == '0'))
			{
				inpos -= 1;
				newname[inpos] = '';
				continue;
			}
		}
		newname[inpos] = name[i];
		inpos += 1;
	}
	return va(newname);
}
void status_update( void )
{
	int		i;
	fileHandle_t	f;
	char		line[8192];
	char		*flen;
	int		iPl;
	vmCvar_t	mapname;
	if (level.statuswrite == 32)
	{
	stringclear(line,8190);
	trap_FS_FOpenFile("serverstatus.dat", &f, FS_WRITE);
	if (!f)
	{
		return;
	}
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(line, 8192, "%s<table  class=\"lultable_map\" cellpadding=\"2\" cellspacing=\"2\" width=\"500\"><tr><td class=\"First\">Map</td><td class=\"Second\">%s</td></tr>", line, mapname.string);
	Com_sprintf(line, 8192, "%s<tr><td class=\"First\">Gametype</td><td class=\"Second\">%s</td></tr>", line, nameforgametype(g_gametype.integer));
	iPl = 0;
	for (i = 0;i < 32;i += 1)
	{
		gentity_t	*pl = &g_entities[i];
		if (pl && pl->client && pl->inuse)
		{
			iPl += 1;
		}
	}
	Com_sprintf(line, 8192, "%s\n<tr><td class=\"First\">Players</td><td class=\"Second\">%i</tr></table>\n", line, iPl);
	Com_sprintf(line, 8192, "%s<table class=\"lultable_players\" cellpadding=\"6\" cellspacing=\"2\" width=\"500\"><tr class=\"players_info\"><td class=\"light\">Number</td><td class=\"dark\">Name</td><td class=\"light\">Ping</td><td class=\"dark\">Team</td><td class=\"light\">Account</td><td class=\"dark\">Rank</td></tr>\n", line);
	for (i = 0;i < 32;i += 1)
	{
		gentity_t	*pl = &g_entities[i];
		if (pl && pl->client && pl->inuse)
		{
			//if (Q_stricmp(pl->client->sess.userlogged,"") == 0)
			//{
			//	Com_sprintf(line, 8192, "%s%i)%s / ping %i / team %s<br>", line, i, pl->client->pers.netname, (pl->client->ps.ping < 999 ? pl->client->ps.ping : 999), nameforteam(pl->client->sess.sessionTeam));
			//}
			//else
			//{
				Com_sprintf(line, 8192, "%s<tr class=\"player\"><td class=\"dark\">%i</td><td class=\"light\">%s</td><td class=\"dark\">%i</td><td class=\"light\">%s</td><td class=\"dark\">%s</td><td class=\"light\">%i</td></tr>\n", line, i, mcm_nameclean(pl->client->pers.netname), (pl->client->ps.ping < 999 ? pl->client->ps.ping : 999), nameforteam(pl->client->sess.sessionTeam), pl->client->sess.userlogged, pl->client->sess.adminloggedin);
			//}
		}
	}
	Com_sprintf(line, 8192, "%s</table>\n", line);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	trap_FS_FOpenFile("serverstatus2.dat", &f, FS_WRITE);
	if (!f)
	{
		return;
	}
	flen = va("%i", level.time);
	trap_FS_Write( flen, strlen(flen), f);
	trap_FS_FCloseFile( f );
	}
}


gban_t	g_bans[512];


void bans_write(void)
{
	int		i;
	fileHandle_t	f;
	char		banfile[8192];
	trap_FS_FOpenFile("bans.cfg", &f, FS_WRITE);
	if (!f)
	{
		return;
	}
	stringclear(banfile,8190);
	banfile[0] = '\n';
	for (i = 0;i < 510;i += 1)
	{
		gban_t	*ban = &g_bans[i];
		if (ban && (ban->active == 1))
		{
			Com_sprintf(banfile, 8192, "%smcmaddban %i %i %i %i\n", banfile, ban->ip1, ban->ip2, ban->ip3, ban->ip4);
		}
	}
	trap_FS_Write( banfile, strlen(banfile), f);
	trap_FS_FCloseFile( f );
}

void bans_init(void)
{
	int	i;
	for (i = 0;i < 510;i += 1)
	{
		gban_t	*ban = &g_bans[i];
		if (ban)
		{
			ban->active = 0;
			ban->ip1 = 0;
			ban->ip2 = 0;
			ban->ip3 = 0;
			ban->ip4 = 0;
		}
	}
}

void bans_add(int ip1, int ip2, int ip3, int ip4)
{
	int	i;
	for (i = 0;i < 510;i += 1)
	{
		gban_t	*ban = &g_bans[i];
		if (ban && (ban->active == 1))
		{
			if (ban->ip1 == ip1)
			{
				if (ban->ip2 == ip2)
				{
					if (ban->ip3 == ip3)
					{
						if (ban->ip4 == ip4)
						{
							return; // Already banned.
						}
					}
				}
			}
		}
		else if (ban)
		{
			ban->active = 1;
			ban->ip1 = ip1;
			ban->ip2 = ip2;
			ban->ip3 = ip3;
			ban->ip4 = ip4;
			return;
		}
	}
}

int ip_is_banned(char *ip)
{
	int	ip1;
	int	ip2;
	int	ip3;
	int	ip4;
	int	i;
	int	iL;
	int	dots;
	char	ipn[1024];

	//char	*bans;
	char	ipworkwith[1024];
	int	ippos;
	int	ipat;

	G_Printf("Check IP %s\n", ip);
	iL = strlen(ip);
	if (iL > 1020)
	{
		G_Printf("IP way too long.\n");
		return 1; // Way too long, faked somehow, ban.
	}
	if (Q_stricmp(ip,"localhost") == 0)
	{
		return 0;
	}
	stringclear(ipn, 1020);
	dots = 0;
	ippos = 0;
	for (i = 0;i < iL;i += 1)
	{
		if (ip[i] == ':')
		{
			//G_Printf(": at %i\n", i);
			break;
		}
		if (ip[i] == '')
		{
			//G_Printf("null at %i\n", i);
			break;
		}
		if (ip[i] == ' ')
		{
			continue;
		}
		if (! (((ip[i] > 47)&&(ip[i] < 58))|| (ip[i] == 46)))
		{
			//G_Printf("NonCh at %i: %s (%i)\n", i, ip[i], ip[i]);
			break;
		}
		if (ip[i] == 46)
		{
			dots += 1;
		}
		ipn[ippos] = ip[i];
		ippos += 1;
	}
	iL = strlen(ipn);
	if (iL < 7)
	{
		G_Printf("IP too short: %s: %s: %i.\n", ip, ipn, iL);
		return 1; // Too short, faked somehow, ban.
	}
	if (iL > 16)
	{
		G_Printf("IP too long.\n");
		return 1; // Too long, faked somehow, ban.
	}
	if (dots != 3)
	{
		G_Printf("IP has wrong number of dots.\n");
		return 1; // Not exactly 3 dots, faked somehow, ban.
	}

	stringclear(ipworkwith, 1020);
	ippos = 0;
	ipat = 1;
	for (i = 0;i <= iL;i += 1)
	{
		if ((ipn[i] == '.')||(i == iL))
		{
			switch (ipat)
			{
				case 1: ip1 = atoi(ipworkwith);break;
				case 2: ip2 = atoi(ipworkwith);break;
				case 3: ip3 = atoi(ipworkwith);break;
				case 4: ip4 = atoi(ipworkwith);break;
				default: G_Printf("IP Refused: switch statement error.\n");return 1; // Should never happen. If it does, something's broken, ban just to be safe.
			}
			stringclear(ipworkwith, 1020);
			ippos = 0;
			ipat += 1;
			continue;
		}
		ipworkwith[ippos] = ipn[i];
		ippos += 1;
	}

	for (i = 0;i < 510;i += 1)
	{
		gban_t	*ban = &g_bans[i];
		if (ban && (ban->active==1))
		{
			//G_Printf("Compare %i.%i.%i.%i to %i.%i.%i.%i\n", ban->ip1, ban->ip2, ban->ip3, ban->ip4, ip1, ip2, ip3, ip4);
			if ((ban->ip1 == ip1)||(ban->ip1 == -1))
			{
				if ((ban->ip2 == ip2)||(ban->ip2 == -1))
				{
					if ((ban->ip3 == ip3)||(ban->ip3 == -1))
					{
						if ((ban->ip4 == ip4)||(ban->ip4 == -1))
						{
							G_Printf("IP banned.\n");
							return 1; // IP is in the system. Ban.
						}
					}
				}
			}
		}
	}
	G_Printf("IP not banned.\n");
	return 0; // All other checks failed. Probably clean. Not banned.
}


void exit_vehicle(gentity_t *ent)
{
	gentity_t *flent;
	if (ent->client->sess.veh_isactive == 1)
	{
	if (ent->client->sess.veh_modent >= 32 && ent->client->sess.veh_modent < 1024)
	{
	flent = &g_entities[ent->client->sess.veh_modent];
	if (flent && flent->inuse)
	{
		G_FreeEntity(flent);
	}
	}
	ent->client->sess.veh_forcedweapon = 0;
	ent->client->sess.veh_modent = 0;
	ent->client->sess.veh_medown = 0;
	ent->client->sess.veh_xdown = 0;
	ent->client->sess.veh_ydown = 0;
	ent->client->sess.veh_xmin = 0;
	ent->client->sess.veh_ymin = 0;
	ent->client->sess.veh_zmin = 0;
	ent->client->sess.veh_xmax = 0;
	ent->client->sess.veh_ymax = 0;
	ent->client->sess.veh_zmax = 0;
	ent->client->sess.veh_movetype = 0;
	ent->client->sess.veh_speed = 0;
	ent->client->sess.veh_isactive = 0;
	ent->client->sess.veh_pitch = 0;
	ent->client->ps.stats[STAT_WEAPONS] = ent->client->sess.veh_recweap;
			ent->client->sess.isglowing = 0;
			WP_ForcePowerStop( ent, FP_TELEPATHY );
			ent->client->ps.eFlags &= ~EF_NODRAW;
			ent->s.eFlags &= ~EF_NODRAW;
			ent->r.svFlags &= ~SVF_NOCLIENT;
		ent->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
	if (ent->client->sess.veh_toresp != 0)
	{
		trap_LinkEntity(&g_entities[ent->client->sess.veh_toresp]);
		ent->client->sess.veh_toresp = 0;
	}
	}
}
void enter_vehicle(gentity_t *ent, char *vehicle)
{
	int		len;
	fileHandle_t	f;
	int		i;
	int		iL;
	gentity_t	*flent;
	char		buffer[4096];
	
	G_Printf("Enter vehicle\n");
	len = trap_FS_FOpenFile(va("vehicles/vehicle_%s.cfg", vehicle), &f, FS_READ);
	if (!f)
	{
		trap_SendServerCmd(ent->s.number, va("print \"^1Unknown vehicle ~^5%s^1~.\n\"", vehicle));
		return;
	}
	if (len > 4090)
	{
		len = 4090;
	}
	iL = 0;
	trap_FS_Read( buffer, len, f );
	flent = G_Spawn();
	trap_FS_FCloseFile(f);
	ent->client->sess.veh_recweap = ent->client->ps.stats[STAT_WEAPONS];
	ent->client->ps.saberHolstered = qtrue;
	ent->client->sess.veh_forcedweapon = WP_BLASTER;
	ent->client->sess.veh_medown = -50;
	ent->client->sess.veh_xdown = 0;
	ent->client->sess.veh_ydown = 0;
	ent->client->sess.veh_xmin = -200;
	ent->client->sess.veh_ymin = -200;
	ent->client->sess.veh_zmin = -200;
	ent->client->sess.veh_xmax = 200;
	ent->client->sess.veh_ymax = 200;
	ent->client->sess.veh_zmax = 200;
	ent->client->sess.veh_movetype = 2; // 1 = walk, 2 = float
	ent->client->sess.veh_speed = 100;
			ent->client->sess.isglowing = 1;
			ent->client->ps.eFlags |= EF_NODRAW;
			ent->s.eFlags |= EF_NODRAW;
			ent->r.svFlags |= SVF_NOCLIENT | SVF_BROADCAST;
			ent->s.eType = ET_INVISIBLE;
	ent->client->sess.veh_modent = flent->s.number;
	ent->client->sess.veh_isactive = 1;
	flent->classname = "vehicle_model";
	flent->s.eType = ET_GENERAL;
	flent->s.modelindex = G_ModelIndex("models/map_objects/ships/tie_fighter.md3");
	flent->s.modelindex2 = flent->s.modelindex;
	flent->r.contents = CONTENTS_WATER;
	flent->clipmask = MASK_WATER;
	//G_Printf(buffer);
	for (i = 0;i < len;i += 1)
	{
		if (buffer[i] == '\n')
		{
			int iF;
			int iS;
			int iT;
			char	name[1024];
			char	value[1024];
			iS = 0;
			iT = 0;
			//G_Printf("SKIP%i\n", i);
			stringclear(name,1020);
			stringclear(value,1020);
			for (iF = iL; iF < i;iF += 1)
			{
				//G_Printf("iF%i iL%i i%
				if (buffer[iF] == ':')
				{
					iT = 1;
					iS = 0;
					//G_Printf("COLON%i\n",iF);
					continue;
				}
				if (iT == 0)
				{
					name[iS] = buffer[iF];
				}
				else
				{
					value[iS] = buffer[iF];
				}
				iS += 1;
			}
			//G_Printf("D: %s - %s\n", name, value);
			if (Q_stricmp(name,"speed") == 0)
			{
				ent->client->sess.veh_speed = atoi(value);
			}
			else if (Q_stricmp(name,"model") == 0)
			{
				if (strstr(value,".glm"))
				{
					flent->bolt_Head = 0;
					flent->s.modelindex = G_ModelIndex(value);
					flent->s.modelGhoul2 = 1;
					strcpy(flent->mcmessage, value);
					flent->s.g2radius = 110;
					ent->s.pos.trType = TR_STATIONARY;
				}
				else
				{
					strcpy(flent->mcmessage, value);
					flent->s.modelindex = G_ModelIndex(value);
					flent->s.modelindex2 = flent->s.modelindex;
				}
			}
			else if (Q_stricmp(name,"movetype") == 0)
			{
				ent->client->sess.veh_movetype = atoi(value);
			}
			else if (Q_stricmp(name,"zadjust") == 0)
			{
				ent->client->sess.veh_medown = atoi(value);
			}
			else if (Q_stricmp(name,"xadjust") == 0)
			{
				ent->client->sess.veh_xdown = atoi(value);
			}
			else if (Q_stricmp(name,"yadjust") == 0)
			{
				ent->client->sess.veh_ydown = atoi(value);
			}
			else if (Q_stricmp(name,"xmin") == 0)
			{
				ent->client->sess.veh_xmin = atoi(value);
			}
			else if (Q_stricmp(name,"ymin") == 0)
			{
				ent->client->sess.veh_ymin = atoi(value);
			}
			else if (Q_stricmp(name,"zmin") == 0)
			{
				vec3_t	fixpos;
				fixpos[0] = ent->client->ps.origin[0];
				fixpos[1] = ent->client->ps.origin[1];
				ent->client->sess.veh_zmin = atoi(value);
				fixpos[2] = (ent->client->ps.origin[2] - ent->client->sess.veh_zmin)+24;
				TeleportPlayer(ent, fixpos, ent->client->ps.viewangles);
			}
			else if (Q_stricmp(name,"xmax") == 0)
			{
				ent->client->sess.veh_xmax = atoi(value);
			}
			else if (Q_stricmp(name,"ymax") == 0)
			{
				ent->client->sess.veh_ymax = atoi(value);
			}
			else if (Q_stricmp(name,"zmax") == 0)
			{
				ent->client->sess.veh_zmax = atoi(value);
			}
			else if (Q_stricmp(name,"weapon") == 0)
			{
				ent->client->sess.veh_forcedweapon = weapforname(value);
			}
			else if (Q_stricmp(name,"pitch") == 0)
			{
				ent->client->sess.veh_pitch = atoi(value);
			}
			//stringclear(name,1020);
			//stringclear(value,1020);
			iL = i+1;
		}
	}
}



void fix_col_box(int num, vec3_t mins, vec3_t maxes)
{
	gentity_t *ent = &g_entities[num];
	if (!ent || !ent->client || !ent->inuse)
	{
		return;
	}
	if (ent->client->sess.veh_isactive != 1)
	{
		return;
	}
	mins[0] = (float)ent->client->sess.veh_xmin;
	mins[1] = (float)ent->client->sess.veh_ymin;
	mins[2] = (float)ent->client->sess.veh_zmin;
	maxes[0] = (float)ent->client->sess.veh_xmax;
	maxes[1] = (float)ent->client->sess.veh_ymax;
	maxes[2] = (float)ent->client->sess.veh_zmax;
}








/*
============
G_ResetHistory

Clear out the given client's history (should be called when the teleport bit is flipped)
============
*/
void G_ResetHistory( gentity_t *ent ) {
	int		i, time;

	// fill up the history with data (assume the current position)
	ent->client->historyHead = NUM_CLIENT_HISTORY - 1;
	for ( i = ent->client->historyHead, time = level.time; i >= 0; i--, time -= 50 ) {
		VectorCopy( ent->r.mins, ent->client->history[i].mins );
		VectorCopy( ent->r.maxs, ent->client->history[i].maxs );
		VectorCopy( ent->r.currentOrigin, ent->client->history[i].currentOrigin );
		ent->client->history[i].leveltime = time;
	}
}


/*
============
G_StoreHistory

Keep track of where the client's been
============
*/
void G_StoreHistory( gentity_t *ent ) {
	int		head, frametime;

	frametime = level.time - level.previousTime;

	ent->client->historyHead++;
	if ( ent->client->historyHead >= NUM_CLIENT_HISTORY ) {
		ent->client->historyHead = 0;
	}

	head = ent->client->historyHead;

	// store all the collision-detection info and the time
	VectorCopy( ent->r.mins, ent->client->history[head].mins );
	VectorCopy( ent->r.maxs, ent->client->history[head].maxs );
	VectorCopy( ent->s.pos.trBase, ent->client->history[head].currentOrigin );
	SnapVector( ent->client->history[head].currentOrigin );
	ent->client->history[head].leveltime = level.time;
}


/*
=============
TimeShiftLerp

Used below to interpolate between two previous vectors
Returns a vector "frac" times the distance between "start" and "end"
=============
*/
static void TimeShiftLerp( float frac, vec3_t start, vec3_t end, vec3_t result ) {
// From CG_InterpolateEntityPosition in cg_ents.c:
/*
	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );
*/
// Making these exactly the same should avoid floating-point error

	result[0] = start[0] + frac * ( end[0] - start[0] );
	result[1] = start[1] + frac * ( end[1] - start[1] );
	result[2] = start[2] + frac * ( end[2] - start[2] );
}


/*
=================
G_TimeShiftClient

Move a client back to where he was at the specified "time"
=================
*/
void G_TimeShiftClient( gentity_t *ent, int time, qboolean debug, gentity_t *debugger ) {
	int		j, k;
	char msg[2048];

	// this will dump out the head index, and the time for all the stored positions
/*
	if ( debug ) {
		char	str[MAX_STRING_CHARS];

		Com_sprintf(str, sizeof(str), "print \"head: %d, %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n\"",
			ent->client->historyHead,
			ent->client->history[0].leveltime,
			ent->client->history[1].leveltime,
			ent->client->history[2].leveltime,
			ent->client->history[3].leveltime,
			ent->client->history[4].leveltime,
			ent->client->history[5].leveltime,
			ent->client->history[6].leveltime,
			ent->client->history[7].leveltime,
			ent->client->history[8].leveltime,
			ent->client->history[9].leveltime,
			ent->client->history[10].leveltime,
			ent->client->history[11].leveltime,
			ent->client->history[12].leveltime,
			ent->client->history[13].leveltime,
			ent->client->history[14].leveltime,
			ent->client->history[15].leveltime,
			ent->client->history[16].leveltime);

		trap_SendServerCommand( debugger - g_entities, str );
	}
*/

	// find two entries in the history whose times sandwich "time"
	// assumes no two adjacent records have the same timestamp
	j = k = ent->client->historyHead;
	do {
		if ( ent->client->history[j].leveltime <= time )
			break;

		k = j;
		j--;
		if ( j < 0 ) {
			j = NUM_CLIENT_HISTORY - 1;
		}
	}
	while ( j != ent->client->historyHead );

	// if we got past the first iteration above, we've sandwiched (or wrapped)
	if ( j != k ) {
		// make sure it doesn't get re-saved
		if ( ent->client->saved.leveltime != level.time ) {
			// save the current origin and bounding box
			VectorCopy( ent->r.mins, ent->client->saved.mins );
			VectorCopy( ent->r.maxs, ent->client->saved.maxs );
			VectorCopy( ent->r.currentOrigin, ent->client->saved.currentOrigin );
			ent->client->saved.leveltime = level.time;
		}

		// if we haven't wrapped back to the head, we've sandwiched, so
		// we shift the client's position back to where he was at "time"
		if ( j != ent->client->historyHead ) {
			float	frac = (float)(time - ent->client->history[j].leveltime) /
				(float)(ent->client->history[k].leveltime - ent->client->history[j].leveltime);

			// interpolate between the two origins to give position at time index "time"
			TimeShiftLerp( frac,
				ent->client->history[j].currentOrigin, ent->client->history[k].currentOrigin,
				ent->r.currentOrigin );

			// lerp these too, just for fun (and ducking)
			TimeShiftLerp( frac,
				ent->client->history[j].mins, ent->client->history[k].mins,
				ent->r.mins );

			TimeShiftLerp( frac,
				ent->client->history[j].maxs, ent->client->history[k].maxs,
				ent->r.maxs );

			if ( debug && debugger != NULL ) {
				// print some debugging stuff exactly like what the client does

				// it starts with "Rec:" to let you know it backward-reconciled
				Com_sprintf( msg, sizeof(msg),
					"print \"^1Rec: time: %d, j: %d, k: %d, origin: %0.2f %0.2f %0.2f\n"
					"^2frac: %0.4f, origin1: %0.2f %0.2f %0.2f, origin2: %0.2f %0.2f %0.2f\n"
					"^7level.time: %d, est time: %d, level.time delta: %d, est real ping: %d\n\"",
					time, ent->client->history[j].leveltime, ent->client->history[k].leveltime,
					ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
					frac,
					ent->client->history[j].currentOrigin[0],
					ent->client->history[j].currentOrigin[1],
					ent->client->history[j].currentOrigin[2], 
					ent->client->history[k].currentOrigin[0],
					ent->client->history[k].currentOrigin[1],
					ent->client->history[k].currentOrigin[2],
					level.time, level.time + debugger->client->frameOffset,
					level.time - time, level.time + debugger->client->frameOffset - time);

				trap_SendServerCommand( debugger - g_entities, msg );
			}

			// this will recalculate absmin and absmax
			trap_LinkEntity( ent );
		} else {
			// we wrapped, so grab the earliest
			VectorCopy( ent->client->history[k].currentOrigin, ent->r.currentOrigin );
			VectorCopy( ent->client->history[k].mins, ent->r.mins );
			VectorCopy( ent->client->history[k].maxs, ent->r.maxs );

			// this will recalculate absmin and absmax
			trap_LinkEntity( ent );
		}
	}
	else {
		// this only happens when the client is using a negative timenudge, because that
		// number is added to the command time

		// print some debugging stuff exactly like what the client does

		// it starts with "No rec:" to let you know it didn't backward-reconcile
		if ( debug && debugger != NULL ) {
			Com_sprintf( msg, sizeof(msg),
				"print \"^1No rec: time: %d, j: %d, k: %d, origin: %0.2f %0.2f %0.2f\n"
				"^2frac: %0.4f, origin1: %0.2f %0.2f %0.2f, origin2: %0.2f %0.2f %0.2f\n"
				"^7level.time: %d, est time: %d, level.time delta: %d, est real ping: %d\n\"",
				time, level.time, level.time,
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
				0.0f,
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], 
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
				level.time, level.time + debugger->client->frameOffset,
				level.time - time, level.time + debugger->client->frameOffset - time);

			trap_SendServerCommand( debugger - g_entities, msg );
		}
	}
}


/*
=====================
G_TimeShiftAllClients

Move ALL clients back to where they were at the specified "time",
except for "skip"
=====================
*/
void G_TimeShiftAllClients( int time, gentity_t *skip ) {
	int			i;
	gentity_t	*ent;
	qboolean debug = ( skip != NULL && skip->client && 
			skip->client->pers.debugDelag && skip->s.weapon == WP_DISRUPTOR );

	// for every client
	ent = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent++ ) {
		if ( ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip ) {
			G_TimeShiftClient( ent, time, debug, skip );
		}
	}
}


/*
================
G_DoTimeShiftFor

Decide what time to shift everyone back to, and do it
================
*/
void G_DoTimeShiftFor( gentity_t *ent ) {	
#ifndef MISSIONPACK
	int wpflags[WP_NUM_WEAPONS] = { 0, 0, 2, 4, 0, 0, 8, 16, 0, 0, 0 };
#else
	int wpflags[WP_NUM_WEAPONS] = { 0, 0, 2, 4, 0, 0, 8, 16, 0, 0, 0, 32, 0, 64 };
#endif
	int wpflag = wpflags[ent->client->ps.weapon];
	int time;

	// don't time shift for mistakes or bots
	if ( !ent->inuse || !ent->client || (ent->r.svFlags & SVF_BOT) ) {
		return;
	}

	// if it's enabled server-side and the client wants it or wants it for this weapon
	if ( ( ent->client->pers.delag & 1 || ent->client->pers.delag & wpflag ) ) {
		// do the full lag compensation, except what the client nudges
		time = ent->client->attackTime + ent->client->pers.cmdTimeNudge;
	}
	else {
		// do just 50ms
		time = level.previousTime + ent->client->frameOffset;
	}

	G_TimeShiftAllClients( time, ent );
}


/*
===================
G_UnTimeShiftClient

Move a client back to where he was before the time shift
===================
*/
void G_UnTimeShiftClient( gentity_t *ent ) {
	// if it was saved
	if ( ent->client->saved.leveltime == level.time ) {
		// move it back
		VectorCopy( ent->client->saved.mins, ent->r.mins );
		VectorCopy( ent->client->saved.maxs, ent->r.maxs );
		VectorCopy( ent->client->saved.currentOrigin, ent->r.currentOrigin );
		ent->client->saved.leveltime = 0;

		// this will recalculate absmin and absmax
		trap_LinkEntity( ent );
	}
}


/*
=======================
G_UnTimeShiftAllClients

Move ALL the clients back to where they were before the time shift,
except for "skip"
=======================
*/
void G_UnTimeShiftAllClients( gentity_t *skip ) {
	int			i;
	gentity_t	*ent;

	ent = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent++) {
		if ( ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip ) {
			G_UnTimeShiftClient( ent );
		}
	}
}


/*
==================
G_UndoTimeShiftFor

Put everyone except for this client back where they were
==================
*/
void G_UndoTimeShiftFor( gentity_t *ent ) {

	// don't un-time shift for mistakes or bots
	if ( !ent->inuse || !ent->client || (ent->r.svFlags & SVF_BOT) ) {
		return;
	}

	G_UnTimeShiftAllClients( ent );
}


/*
===========================
G_PredictPlayerClipVelocity

Slide on the impacting surface
===========================
*/

#define	OVERCLIP		1.001f

void G_PredictPlayerClipVelocity( vec3_t in, vec3_t normal, vec3_t out ) {
	float	backoff;

	// find the magnitude of the vector "in" along "normal"
	backoff = DotProduct (in, normal);

	// tilt the plane a bit to avoid floating-point error issues
	if ( backoff < 0 ) {
		backoff *= OVERCLIP;
	} else {
		backoff /= OVERCLIP;
	}

	// slide along
	VectorMA( in, -backoff, normal, out );
}


/*
========================
G_PredictPlayerSlideMove

Advance the given entity frametime seconds, sliding as appropriate
========================
*/
#define	MAX_CLIP_PLANES	5

qboolean G_PredictPlayerSlideMove( gentity_t *ent, float frametime ) {
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, velocity, origin;
	vec3_t		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	float		into;
	vec3_t		endVelocity;
	vec3_t		endClipVelocity;
	vec3_t		worldUp = { 0.0f, 0.0f, 1.0f };
	
	numbumps = 4;

	VectorCopy( ent->s.pos.trDelta, primal_velocity );
	VectorCopy( primal_velocity, velocity );
	VectorCopy( ent->s.pos.trBase, origin );

	VectorCopy( velocity, endVelocity );

	time_left = frametime;

	numplanes = 0;

	for ( bumpcount = 0; bumpcount < numbumps; bumpcount++ ) {

		// calculate position we are trying to move to
		VectorMA( origin, time_left, velocity, end );

		// see if we can make it there
		trap_Trace( &trace, origin, ent->r.mins, ent->r.maxs, end, ent->s.number, ent->clipmask );

		if (trace.allsolid) {
			// entity is completely trapped in another solid
			VectorClear( velocity );
			VectorCopy( origin, ent->s.pos.trBase );
			return qtrue;
		}

		if (trace.fraction > 0) {
			// actually covered some distance
			VectorCopy( trace.endpos, origin );
		}

		if (trace.fraction == 1) {
			break;		// moved the entire distance
		}

		time_left -= time_left * trace.fraction;

		if ( numplanes >= MAX_CLIP_PLANES ) {
			// this shouldn't really happen
			VectorClear( velocity );
			VectorCopy( origin, ent->s.pos.trBase );
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for ( i = 0; i < numplanes; i++ ) {
			if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) {
				VectorAdd( trace.plane.normal, velocity, velocity );
				break;
			}
		}

		if ( i < numplanes ) {
			continue;
		}

		VectorCopy( trace.plane.normal, planes[numplanes] );
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for ( i = 0; i < numplanes; i++ ) {
			into = DotProduct( velocity, planes[i] );
			if ( into >= 0.1 ) {
				continue;		// move doesn't interact with the plane
			}

			// slide along the plane
			G_PredictPlayerClipVelocity( velocity, planes[i], clipVelocity );

			// slide along the plane
			G_PredictPlayerClipVelocity( endVelocity, planes[i], endClipVelocity );

			// see if there is a second plane that the new move enters
			for ( j = 0; j < numplanes; j++ ) {
				if ( j == i ) {
					continue;
				}

				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				G_PredictPlayerClipVelocity( clipVelocity, planes[j], clipVelocity );
				G_PredictPlayerClipVelocity( endClipVelocity, planes[j], endClipVelocity );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct( planes[i], planes[j], dir );
				VectorNormalize( dir );
				d = DotProduct( dir, velocity );
				VectorScale( dir, d, clipVelocity );

				CrossProduct( planes[i], planes[j], dir );
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				VectorScale( dir, d, endClipVelocity );

				// see if there is a third plane the the new move enters
				for ( k = 0; k < numplanes; k++ ) {
					if ( k == i || k == j ) {
						continue;
					}

					if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) {
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear( velocity );
					VectorCopy( origin, ent->s.pos.trBase );
					return qtrue;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, velocity );
			VectorCopy( endClipVelocity, endVelocity );
			break;
		}
	}

	VectorCopy( endVelocity, velocity );
	VectorCopy( origin, ent->s.pos.trBase );

	return (bumpcount != 0);
}


/*
============================
G_PredictPlayerStepSlideMove

Advance the given entity frametime seconds, stepping and sliding as appropriate
============================
*/
#define	STEPSIZE 18

void G_PredictPlayerStepSlideMove( gentity_t *ent, float frametime ) {
	vec3_t start_o, start_v, down_o, down_v;
	vec3_t down, up;
	trace_t trace;
	float stepSize;

	VectorCopy (ent->s.pos.trBase, start_o);
	VectorCopy (ent->s.pos.trDelta, start_v);

	if ( !G_PredictPlayerSlideMove( ent, frametime ) ) {
		// not clipped, so forget stepping
		return;
	}

	VectorCopy( ent->s.pos.trBase, down_o);
	VectorCopy( ent->s.pos.trDelta, down_v);

	VectorCopy (start_o, up);
	up[2] += STEPSIZE;

	// test the player position if they were a stepheight higher
	trap_Trace( &trace, start_o, ent->r.mins, ent->r.maxs, up, ent->s.number, ent->clipmask );
	if ( trace.allsolid ) {
		return;		// can't step up
	}

	stepSize = trace.endpos[2] - start_o[2];

	// try slidemove from this position
	VectorCopy( trace.endpos, ent->s.pos.trBase );
	VectorCopy( start_v, ent->s.pos.trDelta );

	G_PredictPlayerSlideMove( ent, frametime );

	// push down the final amount
	VectorCopy( ent->s.pos.trBase, down );
	down[2] -= stepSize;
	trap_Trace( &trace, ent->s.pos.trBase, ent->r.mins, ent->r.maxs, down, ent->s.number, ent->clipmask );
	if ( !trace.allsolid ) {
		VectorCopy( trace.endpos, ent->s.pos.trBase );
	}
	if ( trace.fraction < 1.0 ) {
		G_PredictPlayerClipVelocity( ent->s.pos.trDelta, trace.plane.normal, ent->s.pos.trDelta );
	}
}


/*
===================
G_PredictPlayerMove

Advance the given entity frametime seconds, stepping and sliding as appropriate

This is the entry point to the server-side-only prediction code
===================
*/
void G_PredictPlayerMove( gentity_t *ent, float frametime ) {
	G_PredictPlayerStepSlideMove( ent, frametime );
}

int checkteament (int p1, int p2)
{
	gentity_t *first;
	gentity_t *second;
	if (p1 > 31 || p2 > 31 || p1 < 0 || p2 < 0)
	{
		return 0;
	}
	first = &g_entities[p1];
	second = &g_entities[p2];
	if (!first || !first->inuse || !first->client || !second || !second->inuse || !second->client)
	{
		return 0;
	}
	if (first->client->sess.sessionTeam == TEAM_BLUE && second->client->sess.sessionTeam == TEAM_BLUE)
	{
		return 1;
	}
	if (first->client->sess.sessionTeam == TEAM_RED && second->client->sess.sessionTeam == TEAM_RED)
	{
		return 1;
	}
	return 0;
}
void clearteaments(int p1)
{
	int i;
	gentity_t *ent = &g_entities[p1];
	if (!ent || !ent->inuse || !ent->client)
	{
		G_Printf("Clearteaments error??\n");
		return;
	}
	for (i = 0;i < 32;i++)
	{
		gentity_t *flent = &g_entities[i];
		if (!flent || !flent->inuse || !flent->client)
		{
			continue;
		}
		flent->clipstore = flent->r.contents;
		if (ent->client->sess.sessionTeam == TEAM_RED && flent->client->sess.sessionTeam == TEAM_RED)
		{
			flent->r.contents = 0;
		}
		if (ent->client->sess.sessionTeam == TEAM_BLUE && flent->client->sess.sessionTeam == TEAM_BLUE)
		{
			flent->r.contents = 0;
		}
		if (ent->client->sess.sessionTeam == TEAM_RED && flent->client->sess.sessionTeam == TEAM_BLUE)
		{
			if (flent->client->sess.solid != 1)
			{
				flent->r.contents = CONTENTS_SOLID;
			}
		}
		if (ent->client->sess.sessionTeam == TEAM_BLUE && flent->client->sess.sessionTeam == TEAM_RED)
		{
			if (flent->client->sess.solid != 1)
			{
				flent->r.contents = CONTENTS_SOLID;
			}
		}
		if (ent->client->ps.duelInProgress)
		{
			if (flent->s.number == ent->client->ps.duelIndex)
			{
				flent->r.contents = CONTENTS_SOLID;
			}
			else
			{
				flent->r.contents = 0;
			}
		}
	}
}

void fixteaments(int p1)
{
	int i;
	gentity_t *ent = &g_entities[p1];
	if (!ent || !ent->inuse || !ent->client)
	{
		G_Printf("Clearteaments error??\n");
		return;
	}
	for (i = 0;i < 32;i++)
	{
		gentity_t *flent = &g_entities[i];
		if (!flent || !flent->inuse || !flent->client)
		{
			continue;
		}
		flent->r.contents = flent->clipstore;
	}
}








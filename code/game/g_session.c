// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;
	const char	*var2;
	//const char	*var3;
	const char	*var4;
	const char	*var5;
	const char	*var6;
	const char	*sf2;
	//const char	*sf3;

	s = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", 
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.teamLeader,
		client->sess.setForce,
		client->sess.saberLevel,
		client->sess.selectedFP,
		client->sess.adminloggedin,
		client->sess.mcgroup,
		client->sess.credits,
		client->sess.ampowers,
		client->sess.ampowers2,
		client->sess.ampowers3,
		client->sess.ampowers4,
		client->sess.ampowers5,
		client->sess.ampowers6,
		client->sess.allowToggle,
		client->sess.allowTeam,
		client->sess.allowKill,
		client->sess.mcspeed,
		client->sess.mcgravity,
		client->sess.noteleport,
		client->sess.forcegod,
		client->sess.nodrown,
		client->sess.stealth,
		client->sess.ampowers7,
		client->sess.monchan
		);
	var = va( "session%i", client - level.clients );
	trap_Cvar_Set( var, s );

	client->sess.isglowing = 0;
	sf2 = va("%i %i %i %i %i %i %f %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", 
		//client->sess.solid,
		client->sess.jetspeed,
		client->sess.isglowing,
		client->sess.jetshowfuel,
		client->sess.fakelag,
		client->sess.cheat,
		client->sess.allpowerful,
		client->sess.saberspeed,
		client->sess.supergod,
		client->sess.empower,
		client->sess.backupint,
		client->sess.gripdamage,
		client->sess.terminator,
		client->sess.fakepingmin,
		client->sess.fakepingmax,
		client->sess.silence,
		client->sess.IP0,
		client->sess.IP1,
		client->sess.IP2,
		client->sess.IP3,
		client->sess.monitor1,
		client->sess.monitor2,
		client->sess.mcshootdelay,
		client->sess.noforceme,
		client->sess.reversedmg
		);
	var2 = va( "sessionX%i", client - level.clients );
	trap_Cvar_Set( var2, sf2 );

	/*sf3 = va("%i %i %i",
		client->sess.monitor1,
		client->sess.monitor2,
		client->sess.mcshootdelay
		);
	var3 = va( "sessionY%i", client - level.clients );
	trap_Cvar_Set( var3, sf3 );*/

	var4 = va( "mcsessionU%i", client - level.clients );
	trap_Cvar_Set( var4, client->sess.userlogged );

	var5 = va( "mcsessionP%i", client - level.clients );
	trap_Cvar_Set( var5, client->sess.userpass );

	var6 = va( "mcsessionG%i", client - level.clients );
	trap_Cvar_Set( var6, client->sess.mygroup );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	char	s2[MAX_STRING_CHARS];
	//char	s3[MAX_STRING_CHARS];
	const char	*var2;
	const char	*var3;
	const char	*var4;
	const char	*var5;

	// bk001205 - format
	int teamLeader;
	int spectatorState;
	int sessionTeam;
	int SelFP;
	int Los;
	int Win;
	int SpecCli;
	int SpecTim;
	int SabLev;
	int SetFor;
	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
		&sessionTeam,                 // bk010221 - format
		&SpecTim,
		&spectatorState,              // bk010221 - format
		&SpecCli,
		&Win,
		&Los,
		&teamLeader,                   // bk010221 - format
		&SetFor,
		&SabLev,
		&SelFP,
		&client->sess.adminloggedin,
		&client->sess.mcgroup,
		&client->sess.credits,
		&client->sess.ampowers,
		&client->sess.ampowers2,
		&client->sess.ampowers3,
		&client->sess.ampowers4,
		&client->sess.ampowers5,
		&client->sess.ampowers6,
		&client->sess.allowToggle,
		&client->sess.allowTeam,
		&client->sess.allowKill,
		&client->sess.mcspeed,
		&client->sess.mcgravity,
		&client->sess.noteleport,
		&client->sess.forcegod,
		&client->sess.nodrown,
		&client->sess.stealth,
		&client->sess.ampowers7,
		&client->sess.monchan
		);

	client->sess.wins = Win;
	client->sess.losses = Los;
	client->sess.spectatorTime = SpecTim;
	client->sess.spectatorClient = SpecCli;
	client->sess.selectedFP = SelFP;
	client->sess.setForce = SetFor;
	client->sess.saberLevel = SabLev;
	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;

	client->ps.fd.saberAnimLevel = client->sess.saberLevel;
	client->ps.fd.forcePowerSelected = client->sess.selectedFP;
	var2 = va( "sessionX%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var2, s2, sizeof(s2) );
	sscanf( s2, "%i %i %i %i %i %i %f %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
		//&client->sess.solid,
		&client->sess.jetspeed,
		&client->sess.isglowing,
		&client->sess.jetshowfuel,
		&client->sess.fakelag,
		&client->sess.cheat,
		&client->sess.allpowerful,
		&client->sess.saberspeed,
		&client->sess.supergod,
		&client->sess.empower,
		&client->sess.backupint,
		&client->sess.gripdamage,
		&client->sess.terminator,
		&client->sess.fakepingmin,
		&client->sess.fakepingmax,
		&client->sess.silence,
		&client->sess.IP0,
		&client->sess.IP1,
		&client->sess.IP2,
		&client->sess.IP3,
		&client->sess.monitor1,
		&client->sess.monitor2,
		&client->sess.mcshootdelay,
		&client->sess.noforceme,
		&client->sess.reversedmg
		);

	/*var2 = va( "sessionY%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var2, s3, sizeof(s3) );
	sscanf( s3, "%i %i %i",
		&client->sess.monitor1,
		&client->sess.monitor2,
		&client->sess.mcshootdelay
		);*/


	client->sess.veh_isactive = 0;
	var3 = va( "mcsessionU%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var3, client->sess.userlogged, sizeof(client->sess.userlogged) );

	var4 = va( "mcsessionP%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var4, client->sess.userpass, sizeof(client->sess.userpass) );

	var5 = va( "mcsessionG%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var5, client->sess.mygroup, sizeof(client->sess.mygroup) );
	strcpy(client->sess.ignoring,"0000000000000000000000000000000000000000");
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo, qboolean isBot ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		if ( g_teamAutoJoin.integer ) {
			sess->sessionTeam = PickTeam( -1 );
			BroadcastTeamChange( client, -1 );
		} else {
			// always spawn as spectator in team games
			if (!isBot)
			{
				sess->sessionTeam = TEAM_SPECTATOR;	
			}
			else
			{ //Bots choose their team on creation
				value = Info_ValueForKey( userinfo, "team" );
				if (value[0] == 'r' || value[0] == 'R')
				{
					sess->sessionTeam = TEAM_RED;
				}
				else if (value[0] == 'b' || value[0] == 'B')
				{
					sess->sessionTeam = TEAM_BLUE;
				}
				else
				{
					sess->sessionTeam = PickTeam( -1 );
				}
				BroadcastTeamChange( client, -1 );
			}
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_HOLOCRON:
			case GT_JEDIMASTER:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				if (isBot)
				{
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;
	Q_strncpyz(sess->userlogged, "", sizeof(sess->userlogged));
	Q_strncpyz(sess->userpass, "", sizeof(sess->userpass));
	Q_strncpyz(sess->amprefix, "", sizeof(sess->amprefix));
	Q_strncpyz(sess->amsuffix, "", sizeof(sess->amsuffix));
	Q_strncpyz(sess->channel_01name, "", sizeof(sess->channel_01name));
	Q_strncpyz(sess->channel_02name, "", sizeof(sess->channel_02name));
	Q_strncpyz(sess->channel_03name, "", sizeof(sess->channel_03name));
	Q_strncpyz(sess->channel_01pass, "", sizeof(sess->channel_01pass));
	Q_strncpyz(sess->channel_02pass, "", sizeof(sess->channel_02pass));
	Q_strncpyz(sess->channel_03pass, "", sizeof(sess->channel_03pass));
	Q_strncpyz(sess->doorpassword, "", sizeof(sess->doorpassword));
	Q_strncpyz(sess->rrname, "", sizeof(sess->rrname));
	strcpy(sess->mygroup,"00000000");
	sess->adminloggedin = 0;
	sess->ampowers = 0;
	sess->ampowers2 = 0;
	sess->ampowers3 = 0;
	sess->ampowers4 = 0;
	sess->ampowers5 = 0;
	sess->ampowers6 = 0;
	sess->ampowers7 = 0;
	sess->credits = 0;
	sess->mcgroup = 0;
	sess->monitor1 = 0;
	sess->monitor2 = 0;
	sess->allowToggle = 1;
	sess->veh_isactive = 0;
	sess->allowTeam = 1;
	sess->allowKill = 1;
	sess->logintrys = 0;
	sess->mcspeed = 0;
	sess->mcshootdelay = 0;
	sess->thisconnectuc = 0;
	sess->grabbedent = 0;
	sess->grabbedentyaw = 0;
	sess->grabbedentpitch = 0;
	sess->grabbedgroup = 0;
	channels_remove_all(client - level.clients);
	channels_unban_all (client - level.clients);
	sess->grabbedplayer = 0;
	sess->grabbedpdist = 0;
	sess->mcgravity = 0;
	sess->slapping = 0;
	sess->gripdamage = 0;
	sess->freeze = 0;
	sess->noteleport = 0;
	sess->forcegod = 0;
	sess->nodrown = 0;
	sess->watching = 0;
	sess->isglowing = 0;
	sess->sentries = 0;
	sess->empower = 0;
	sess->backupint = 0;
	sess->solid = 0;
	sess->jetspeed = 0;
	sess->jetdelayusefix = 0;
	sess->dienow = 0;
	sess->padawan = 0;
	sess->noforceme = 0;
	sess->flying = 0;
	sess->movebackX = 0;
	sess->movebackY = 0;
	sess->movebackZ = 0;
	sess->massgravity = 0;
	sess->movebacktime = 0;
	sess->fakepingmin = 0;
	sess->fakepingmax = 0;
	sess->controller = 0;
	sess->lvote = 0;
	sess->punish = 0;
	sess->protect = 0;
	sess->knockbackuponly = 0;
	sess->torture = 0;
	sess->freeze = 0;
	sess->mcPing = 0;
	sess->mcTime = 0;
	sess->aimbot = 0;
	sess->sleep = 0;
	sess->parachute = 0;
	sess->silence = 0;
	sess->fspec = 0;
	sess->jetfuel = mc_jetpack_fuelmax.integer;
	sess->isglowing = 0;
	sess->jetshowfuel = 1;
	sess->fakelag = 0;
	sess->cheat = 0;
	sess->knockbackonly = 0;
	sess->teamchattype = 0;
	sess->allpowerful = 0;
	sess->forcegod = 0;
	sess->terminator = 0;
	sess->forcegod2 = 0;
	sess->dodging = 0;
	sess->reflect = 0;
	sess->saberspeed = 0;
	sess->supergod = 0;
	sess->sentries = 0;
	sess->amjump = 0;
	sess->damagemod = 0;
	sess->steve = 0;
	sess->grabbedpoffz = 0;
	sess->grabbedentoffz = 0;
	sess->pendingtype = 0;
	sess->pendingvalue = 0;
	sess->pendingtimeout = 0;
	sess->blocktype = 0;
	sess->specweapon = 0;
	sess->stealth = 0;
	sess->viewrandom = 0;
	sess->abused = 0;
	sess->traced = 0;
	sess->monchan = 0;
	sess->noknockback = 0;
	sess->isAFK = 0;
	sess->xAFK = 0;
	sess->ticksAFK = 0;
	sess->blockweapon = 0;
	sess->blockforce = 0;
	sess->blockrename = 0;
	sess->worshipped = 0;
	sess->shock = 0;
	sess->duel_is_ff = 0;
	sess->reversedmg = 0;
	strcpy(client->sess.ignoring,"0000000000000000000000000000000000000000");

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		//level.newSession = qtrue;
		level.mnewtype = 1;
		//G_Printf( "Gametype changed, clearing session data.\n" );
	}
	if (mc_lms.integer > 0)
	{
		level.voteExecuteTime = 0;
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		level.lmsvote = 1;
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "New LMS Round Starting" );
		level.voteTime = level.time;
		level.voteYes = 0;
		level.voteNo = 0;
		Com_sprintf( level.voteString, sizeof( level.voteString ), "" );
		trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
		trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
		trap_SetConfigstring( CS_VOTE_YES, va("0") );
		trap_SetConfigstring( CS_VOTE_NO, va("0") );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_Set( "session", va("%i", g_gametype.integer) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}

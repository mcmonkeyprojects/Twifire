// File added by mcmonkey to lower g_cmds.c file size
//
#include "g_local.h"
#include "../../ui/menudef.h"

void dsp_doEmote(gentity_t *ent, int cmd);
void dsp_stringEscape(char *in, char *out, int outSize);
void BG_CycleInven(playerState_t *ps, int direction);
void BG_CycleForce(playerState_t *ps, int direction);
extern int allaktiv;
void dspSanitizeString( char *in, char *out );
void dsp_doMOTD( gentity_t *ent );
int ent_respawn(gentity_t *ent);
void mcmusetargets(gentity_t *ent);
void shadder(gentity_t *ent, int clientNum);
void dsp_clanBother(int clientNum);
void StandardSetBodyAnim(gentity_t *self, int anim, int flags);
void dsp_setIP(int clientNum, char *valueIP);
void twimod_admincmds(gentity_t *ent, int clientNum, int cmd);
void twimod_adminnotarget(gentity_t *ent, int clientNum, int cmd);
void twimod_status(gentity_t *ent, int clientNum);
void twimod_status2(gentity_t *ent, int clientNum);
void twimod_whois(gentity_t *ent, int clientNum);
extern void AddSpawnField(char *field, char *value);
extern void SP_fx_runner( gentity_t *ent );
void mc_buildercmds(gentity_t *ent, int clientNum, int cmd);
void mc_addcredits( gentity_t *ent, int Mnewcount );
gentity_t *G_PlayEffect_ID(int fxID, vec3_t org, vec3_t ang);
char	*ConcatArgs( int start );


int isinnewgroup(gentity_t *ent, char *group)
{
	int	iLen;
	int	i;
	int	iC;
	int	iN;
	char	gval[1024];
	char	gtype[1024];
	if (!group)
	{
		return 1;
	}
	if (!ent || !ent->inuse || !ent->client)
	{
		return 0;
	}
	if ((ent->client->sess.adminloggedin > 0)&&(ent->client->sess.ampowers7 & 256))
	{
		return 1;
	}
	iLen = strlen(group);
	if (iLen < 2)
	{
		return 1;
	}
	stringclear(gval, 1020);
	stringclear(gtype, 1020);
	iC = 0;
	iN = 0;
	for (i = 0;i < iLen;i += 1)
	{
		if (iC == 0)
		{
			if (group[i] == ':')
			{
				iC = 1;
				iN = 0;
				continue;
			}
			gtype[iN] = group[i];
			iN += 1;
		}
		if (iC == 1)
		{
			if (group[i] == ']')
			{
				if (Q_stricmp(gtype,"user") == 0)
				{
					if (Q_stricmp(gval,ent->client->sess.userlogged) == 0)
					{
						return 1;
					}
				}
				else if (Q_stricmp(gtype,"adminrank") == 0)
				{
					if (ent->client->sess.adminloggedin >= atoi(gval))
					{
						return 1;
					}
				}
				else if (Q_stricmp(gtype,"group") == 0)
				{
					if (atoi(gval) < 20)
					{
						if (ent->client->sess.mygroup[atoi(gval)-1] == 'A')
						{
							return 1;
						}
					}
				}
				iC = 0;
				iN = 0;
				stringclear(gval, 1020);
				stringclear(gtype, 1020);
				continue;
			}
			gval[iN] = group[i];
			iN += 1;
		}
	}
	return 0;
}



int monkeyspot(char *spot, int xyorz, gentity_t *playerent, gentity_t *otherent);

gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
extern void SP_jakes_model( gentity_t *ent );
extern void SP_jakes_model_zocken( gentity_t *ent );

void mcmusetargets(gentity_t *ent)
{
	char par1[MAX_STRING_CHARS];
	trap_Argv(1, par1, sizeof(par1));
	if ( Q_stricmp(par1, "info") == 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1/ammap_usetarget <target>\n\"" ) );
		return;
	}
	G_UseTargets2( ent, ent, par1 );
}
float mc_fix360(float rnum)
{
	float rinum;
	for (rinum = rnum;rinum > 360;rinum -= 360);
	for (rinum = rinum;rinum < 0;rinum += 360);
	return rinum;
}
int mc_fix360i(int rnum)
{
	int rinum;
	rinum = 0;
	for (rinum = rnum;rinum > 360;rinum -= 360);
	for (rinum = rinum;rinum < 0;rinum += 360);
	return rinum;
}
void spycamthink(gentity_t *ent)
{
	gentity_t	*other = &g_entities[ent->r.ownerNum];
	vec3_t		dir;
	vec3_t		cang;
	if (!other || !other->inuse)
	{
		G_FreeEntity(ent);
		return;
	}
	VectorCopy(other->client->ps.origin, ent->s.origin2);
	//cang[ROLL] = mc_fix360(other->client->ps.viewangles[YAW]);
	//cang[YAW] = 180;
	//cang[PITCH] = mc_fix360(other->client->ps.viewangles[PITCH]);
	cang[YAW] = mc_fix360(other->client->ps.viewangles[YAW]);
	cang[ROLL] = 0;
	cang[PITCH] = mc_fix360(other->client->ps.viewangles[PITCH]);
	G_SetMovedir(cang, dir);
	ent->s.eventParm = DirToByte(dir);
	ent->nextthink = level.time + 50;
}
void twimod_admincmds(gentity_t *ent, int clientNum, int cmd)
{
	vec3_t		origin, angles, oppDir, origin2, angles2, origin3, origin4, fwd;
	gentity_t	*other; // We mark the target with other.. ent has no use at this point
	gentity_t	*lolzw;
	int		client2num;
	int		orig1;
	int		orig2;
	int		orig3;
	char		*concatted;
	gentity_t	*client2;
	int	j;
	int	color;
	gentity_t	*other2;
	char	*newName;
	char	buffer[MAX_STRING_CHARS]; // The buffer is used for storaged of temp things such as kickreason
	char	string[MAX_STRING_CHARS];
	char	userinfo[MAX_INFO_VALUE];

	char	par1[MAX_STRING_CHARS]; // This is the targetnumber/name
	char	par2[MAX_STRING_CHARS]; // If needed, this is the second parameter
	char	par3[MAX_STRING_CHARS];
	char	par4[MAX_STRING_CHARS];
	char	par5[MAX_STRING_CHARS];
	char	par6[MAX_STRING_CHARS];
	char		name[MAX_TOKEN_CHARS];
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];
		char			savePath[MAX_QPATH];
		fileHandle_t	z;
		char			line[256];
		char			userwrite[2048];
	char	LogCommand[128];
	float	hoek;

	int		angle;
	int		idnum; // The id number obtained from the targetlist
	int		f; // Used by commands to set multiple things
	trap_Argv(1, par1, sizeof(par1));
	trap_Argv(2, par2, sizeof(par2));
	trap_Argv(3, par3, sizeof(par3));
	trap_Argv(4, par4, sizeof(par4));
	trap_Argv(5, par5, sizeof(par5));
	trap_Argv(6, par6, sizeof(par6));
	if ( allaktiv == 36 )
		{
			/*if ( Q_stricmp(par1, "info") == 0)
				{
					idnum = 0;
				}
			else
				{*/
					idnum = dsp_adminTarget(ent, par1, clientNum);
				//}
		}
	else if ( allaktiv != 36 )
		{
		idnum = allaktiv;
		}
		if (idnum == -2)
		{
			trap_SendServerCmd( clientNum, va("print \"^2.:^7No target has been found^2:.\n\""));
			return;
		}
		else if (idnum == -3)
		{
			trap_SendServerCmd( clientNum, va("print \"^2.:^7No target has been found^2:.\n\""));
			return;
		}
		else if (idnum == -1)
		{
			if ( cmd == 7 || cmd == 8 || cmd == 9 || cmd == 21 || cmd == 50 )
				{
					trap_SendServerCmd( clientNum, va("print \"^2.:^7This command can't be target on all^2:.\n\""));
				}
				else if ( par3 == "get" && cmd == 12 )
					{
						trap_SendServerCmd( clientNum, va("print \"^2.:^7This command can't be target like that on all^2:.\n\""));
					}
				// --------------------------------------------------------------------------------------------------
			else
				{
			for (i=0; i < 32; i+=1)
			{
				lolzw = &g_entities[i];
				if ( !lolzw->client || ( lolzw->client->pers.connected != CON_CONNECTING && lolzw->client->pers.connected != CON_CONNECTED ) || (lolzw->client->sess.stealth == 1))
				{
					continue;
				}
				allaktiv = i;
				twimod_admincmds(ent, clientNum, cmd);
			}
			allaktiv = 36;
				}
		}
		other = &g_entities[idnum];
		if (!other->client || !other->inuse)
		{
			return;
		}
		if ( twimod_combatadmin.integer == 0 && idnum >= 0 && idnum != ent->s.number && ( cmd == 2 || cmd == 4 || cmd == 5 || /*cmd == 7 ||*/ cmd == 10 || cmd == 12 || cmd == 13 || cmd == 14 || cmd == 15 || cmd == 16 || cmd == 17 || cmd == 18 || cmd == 21 || cmd == 11 || cmd == 24 || cmd == 26 || cmd == 22 || cmd == 50
							|| cmd ==  56 || cmd == 57 || cmd == 58 || cmd == 60 || cmd == 78 || cmd == 84 || cmd == 89 || cmd == 94 || cmd == 95 || cmd == 98 || cmd == 104 || cmd == 105 || cmd == 112 || cmd == 125
							|| ((cmd == 71)&&(other->client->sess.ampowers3 & 4))|| ((cmd == 96)&&(other->client->sess.ampowers5 & 1024))|| ((cmd == 73)&&(other->client->sess.ampowers3 & 16)) ) )
			{
		if (ent->client->sess.adminloggedin <= other->client->sess.adminloggedin)
			{
				if ( allaktiv == 36 )
					{
				trap_SendServerCmd( clientNum, va("print \"^2.:^7You can not use this command on a higher or equal ranked admin^2:.\n\""));
				return;
					}
				else
					{
				return;
					}
			}
		}


		switch ( cmd )
	{
		case 1:
		//////////////////
		// amprotect
		//////////////////
			if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amprotect <player>\n\"" ) );
				 return;
				}
			if ( other->client->sess.protect == 0 )
				{
			other->client->ps.eFlags |= EF_INVULNERABLE;
			other->client->invulnerableTimer = level.time + 60000;
			/*if ( !ent->client->sess.empower && !ent->client->sess.terminator )
				{
			WP_InitForcePowers(ent);
				}*/
			other->client->sess.protect = 1;
				}
			else
				{
			other->client->ps.eFlags &= ~EF_INVULNERABLE;
			other->client->invulnerableTimer = 0;
			/*if ( !ent->client->sess.empower && !ent->client->sess.terminator )
				{
			WP_InitForcePowers(ent);
				}*/
			other->client->sess.protect = 0;
				}
			break;
		case 2:
		//////////////////
		// amkick
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amkick <player>\n\"" ) );
				 return;
				}
			if ( idnum == ent->s.number )
				{
				 trap_SendServerCmd( clientNum, va("print \"You can not kick yourself.\n\"" ) );
				 return;
				}
			else
				{
			if (other->client->sess.sessionTeam == TEAM_SPECTATOR)
			{
				other->client->sess.spectatorState = SPECTATOR_FREE;
				other->client->sess.spectatorClient = clientNum;
			}
				trap_DropClient(idnum, va("%s", twimod_kickmsg.string));
			 }
			break;
		case 3:
		//////////////////
		// amunprotect
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amunprotect <player>\n\"" ) );
				 return;
				}
			other->client->ps.eFlags &= ~EF_INVULNERABLE;
			other->client->invulnerableTimer = 0;
			/*if ( !ent->client->sess.empower && !ent->client->sess.terminator )
				{
			WP_InitForcePowers(ent);
				}*/
			other->client->sess.protect = 0;
			break;
		case 4:
		//////////////////
		// amban
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amban <player>\n\"" ) );
				 return;
				}
				if ( idnum == ent->s.number )
				{
				 trap_SendServerCmd( clientNum, va("print \"You can not ban yourself.\n\"" ) );
				 return;
				}
						else if ( idnum == -1 )
							{
										trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
										return;
							}
				else
				{
			if (other->client->sess.sessionTeam == TEAM_SPECTATOR)
			{
				other->client->sess.spectatorState = SPECTATOR_FREE;
				other->client->sess.spectatorClient = clientNum;
			}

				if (mc_newbansystem.integer == 0)
				{trap_SendConsoleCommand(EXEC_APPEND, va("addip %i.%i.%i.%i", other->client->sess.IP0, other->client->sess.IP1,other->client->sess.IP2, other->client->sess.IP3));}
				else
				{
					bans_add(other->client->sess.IP0, other->client->sess.IP1, other->client->sess.IP2, other->client->sess.IP3);
					bans_write();
				}

				trap_DropClient(idnum, va("%s", twimod_banmsg.string));
				}
			break;
		case 5:
		//////////////////
		// ambanrange
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/ambanrange <player>\\n\"" ) );
				 return;
				}
						if ( idnum == ent->s.number )
				{
				 trap_SendServerCmd( clientNum, va("print \"You can not ban yourself.\n\"" ) );
				 return;
				}
						else if ( idnum == -1 )
							{
										trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
										return;
							}
				else
					{
			if (other->client->sess.sessionTeam == TEAM_SPECTATOR)
			{
				other->client->sess.spectatorState = SPECTATOR_FREE;
				other->client->sess.spectatorClient = clientNum;
			}

				if (mc_newbansystem.integer == 0){trap_SendConsoleCommand(EXEC_APPEND, va("addip %i.%i.0.0", other->client->sess.IP0, other->client->sess.IP1));}				else
				{
					bans_add(other->client->sess.IP0, other->client->sess.IP1, -1, -1);
					bans_write();
				}

				trap_DropClient(idnum, va("%s", twimod_banmsg.string));
			}
			break;
		case 6:
		//////////////////
		// amorigin
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amorigin <player>\n\"" ) );
				 return;
				}
			if ( idnum == -1 )
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
					return;
				}
			else
				{
			if (other->client && other->inuse)
			{
				VectorCopy(other->client->ps.origin, origin);
				VectorCopy(other->client->ps.viewangles, angles);
				trap_SendServerCmd(clientNum, va("print \"%s: ^2(^7%i^2) (^7%i^2) (^7%i^2) : (^7%i^2)\n\"", other->client->pers.netname, (int)origin[0], (int)origin[1], (int)origin[2], mc_fix360i((int)angles[YAW])));
			}
				}
			break;
		case 7:
		//////////////////
		// amtele
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				trap_SendServerCmd( clientNum, va("print \"^1/amtele <player> <x> <y> <z> <yaw>\n\"" ) );
				 return;
				}
			if ( idnum == -1 )
			{
			trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
			return;
			}
			else
			{

				if ((ent->s.number != other->s.number)&&(other->client->sess.noteleport == 1))
				{
					if (!(ent->client->sess.ampowers7 & 32))
					{
						trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported.^7\n\"" ) );
						return;
					}
				}
			trap_UnlinkEntity (other);
			// This was needed because the teleport function does this for a different name.

			VectorClear( origin );
			VectorClear( angles );


			if (trap_Argc() == 5)
			{
			origin[0] = (float)monkeyspot(par2, 0, ent, other);//atof(par2); // X
			origin[1] = (float)monkeyspot(par3, 1, ent, other);//atof(par3); // Y
			origin[2] = (float)monkeyspot(par4, 2, ent, other);//atof(par4); // Z
			angles[YAW] = 0; // Player viewangles [YAW]

			TeleportPlayer( other, origin, angles );
			}
			else if (trap_Argc() < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/amtele <player> <x> <y> <z> <yaw>\n\"" ) );
				return;
			}
			else
			{
			origin[0] = (float)monkeyspot(par2, 0, ent, other);//atof(par2); // X
			origin[1] = (float)monkeyspot(par3, 1, ent, other);//atof(par3); // Y
			origin[2] = (float)monkeyspot(par4, 2, ent, other);//atof(par4); // Z
			angles[YAW] = atof(par5); // Player viewangles [YAW]

			TeleportPlayer( other, origin, angles );
			// Tada, and you're poof'ed away

			}
		}
			break;
		case 8:
		//////////////////
		// amget
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amget <player>\n\"" ) );
				 return;
				}
			if (trap_Argc() == 1)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/amget <player>\n\"" ) );
				return;
			}
			else
			{
				if ( idnum == -1 )
				{
				trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
					return;
				}
			else
			{
			if ((ent->s.number != other->s.number)&&(other->client->sess.noteleport == 1))
			{
				if (!(ent->client->sess.ampowers7 & 32))
				{
					trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported.^7\n\"" ) );
					return;
				}
			}
			trap_UnlinkEntity (other);
			// This was needed because the teleport function does this for a different name.

			VectorClear( origin );
			VectorClear( angles );

			//hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
			//origin[0] = ent->client->ps.origin[0] + ( cos(hoek) * 200 ); // X
			//origin[1] = ent->client->ps.origin[1] + ( sin(hoek) * 200 ); // Y

			//origin[2] = ent->client->ps.origin[2]; // Z
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0] + fwd[0]*128;
		origin[1] = ent->client->ps.origin[1] + fwd[1]*128;
		origin[2] = ent->client->ps.origin[2]/* + fwd[2]*64*/;
			if ( idnum == ent->s.number )
			{
			angles[YAW] = AngleNormalize360(ent->client->ps.viewangles[YAW]); // Other-Player viewangles [YAW]
			}
			else
			{
			angles[YAW] = AngleNormalize360(ent->client->ps.viewangles[YAW] + 180); // Other-Player viewangles [YAW]
			}

			TeleportPlayer( other, origin, angles );
			// Tada, and you're poof'ed away

			ent->client->ps.forceHandExtend = HANDEXTEND_FORCEPULL;
			ent->client->ps.forceHandExtendTime = level.time + 300;
			// Gonna look cool by doing the pull handanimation
			}
		}
			break;
		case 9:
		//////////////////
		// amgoto
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amgoto <player>\n\"" ) );
				 return;
				}
			if (trap_Argc() == 1)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgoto <player>\n\"" ) );
			return;
		}
			else
		{
			if ((ent->s.number != other->s.number)&&(other->client->sess.noteleport == 1))
			{
				if (!(ent->client->sess.ampowers7 & 32))
				{
					trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported to.^7\n\"" ) );
					return;
				}
			}
			if ( idnum == -1 )
			{
			trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
			return;
			}
			else
			{
			trap_UnlinkEntity (other);
			// This was needed because the teleport function does this for a different name.

			VectorClear( origin );
			VectorClear( angles );

			//hoek = other->client->ps.viewangles[YAW] * (M_PI*2 / 360);
			//origin[0] = other->client->ps.origin[0] + ( cos(hoek) * 100 ); // X
			//origin[1] = other->client->ps.origin[1] + ( sin(hoek) * 100 ); // Y
			//origin[2] = other->client->ps.origin[2]; // Z
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = AngleNormalize360(other->client->ps.viewangles[YAW]);
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = other->client->ps.origin[0] + fwd[0]*128;
		origin[1] = other->client->ps.origin[1] + fwd[1]*128;
		origin[2] = other->client->ps.origin[2]/* + fwd[2]*64*/;
			angles[YAW] = AngleNormalize360(other->client->ps.viewangles[YAW] + 180); // Other-Player viewangles [YAW]

			TeleportPlayer( ent, origin, angles );

			ent->client->ps.forceHandExtend = HANDEXTEND_FORCEPUSH;
			ent->client->ps.forceHandExtendTime = level.time + 300;
			// Tada, and you're poof'ed away
			}
		}
		break;
	case 10:
		//////////////////
		// amslap
		//////////////////
					if ((Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amslap <player>\n\"" ) );
				 return;
				}
			if (!other->client->sess.punish && !other->client->sess.protect && !other->client->sess.freeze)
			{
				if (!other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}
				VectorNormalize2( ent->client->ps.velocity, oppDir );
				VectorScale( oppDir, -1, oppDir );
				angle = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
				oppDir[1] = (sin(angle)*twimod_slapstrength.integer);
				oppDir[0] = (cos(angle)*twimod_slapstrength.integer);
				other->client->ps.velocity[0] = oppDir[0];
				other->client->ps.velocity[1] = oppDir[1];
				other->client->ps.velocity[2] = (twimod_slapzstrength.integer);
				other->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
				other->client->ps.forceHandExtendTime = level.time + (twimod_slapdowntime.integer * 1000);
				other->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
			}
			break;
		case 11:
		//////////////////
		// amfreeze
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amfreeze <player>\n\"" ) );
				 return;
				}
				if ( other->client->sess.punish == 1 )
				{
					trap_SendServerCmd( clientNum, va("print \"^1Client is punished, you can't freeze the client.\n\"" ) );
					return;
				}
				else if ( other->client->sess.sleep == 1 )
				{
					trap_SendServerCmd( clientNum, va("print \"^1Client is sleeping, you can't freeze the client.\n\"" ) );
					return;
				}
				else
				{
				if ( other->client->sess.freeze == 1 )
					{
						other->client->ps.pm_type = PM_NORMAL;
						other->client->sess.freeze = 0;
						trap_SendServerCmd( clientNum, va("print \"^2Client is already frozen, the client has been unfrozen.\n\"" ) );
					}
				else
					{
						other->client->ps.pm_type = PM_FREEZE;
						other->client->sess.freeze = 1;
					}
				}
				break;
		case 12:
		//////////////////
		// amsleep
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amsleep <player>\n\"" ) );
				 return;
				}
			if (!other->client->sess.sleep)
			{
				twimod_admincmds(ent, clientNum, 15);
				if ( (trap_Argc() == 1) )
					{
						trap_SendServerCmd( clientNum, va("print \"^1/amsleep <player>\n\"" ) );
						return;
					}
			else
			{
				if (!other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}
				other->client->ps.pm_type = PM_NORMAL;
				other->client->sess.sleep = 1;
				other->client->sess.allowKill = qfalse;
				other->client->sess.allowTeam = qfalse;

				other->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
				other->client->ps.forceHandExtendTime = level.time + 60000;
				other->client->ps.forceDodgeAnim = 0;
				other->client->ps.eFlags |= EF_INVULNERABLE;
				other->client->invulnerableTimer = level.time + 60000;
				WP_InitForcePowers(other);
				other->client->sess.protect = 1;
			}
		}
			break;
		case 13:
		//////////////////
		// amwake
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amwake <player>\n\"" ) );
				 return;
				}
			if (other->client->sess.sleep)
			{
				other->client->sess.sleep = 0;
				other->client->pers.cmd.weapon = WP_SABER;
				other->client->ps.pm_flags = PM_NORMAL;
				other->client->ps.forceHandExtendTime = level.time + 0;
				other->client->sess.allowKill = qtrue;
				other->client->sess.allowTeam = qtrue;
				other->client->ps.eFlags |= EF_INVULNERABLE;
				other->client->invulnerableTimer = level.time + 0;
				WP_InitForcePowers(other);
				other->client->sess.protect = 0;
			}
			break;
		case 14:
		//////////////////
		// ampunish
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/ampunish <player>\n\"" ) );
				 return;
				}
			if (!other->client->sess.punish)
			{
				twimod_admincmds(ent, clientNum, 13);
				if (!other->client->ps.saberHolstered)
				{
					other->client->ps.saberHolstered = qtrue;
				}
				other->client->ps.pm_type = PM_NORMAL;
				other->client->sess.freeze = qfalse;
				StandardSetBodyAnim(other, BOTH_CHOKE1STARTHOLD, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				other->client->sess.punish = qtrue;
				other->client->sess.allowTeam = qfalse;
				other->client->sess.allowToggle = qfalse;
				other->client->sess.allowKill = qfalse;

				other->client->sess.silence = qtrue;
			}
			break;
		case 15:
		//////////////////
		// amunpunish
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amunpunish <player>\n\"" ) );
				 return;
				}
			if (other->client->sess.punish)
			{
				if (!other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}

				other->client->sess.freeze = qfalse;
				other->client->ps.pm_type = PM_NORMAL;
				other->client->sess.punish = qfalse;
				other->client->sess.allowKill = qtrue;
				other->client->sess.allowTeam = qtrue;
				other->client->sess.allowToggle = qtrue;
				other->client->sess.silence = qfalse;
			}
			break;
		case 16:
		//////////////////
		// amsilence
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amsilence <player>\n\"" ) );
				 return;
				}
			if (!other->client->sess.silence)
			{
				other->client->sess.silence = qtrue;
			}
			break;
		case 17:
		//////////////////
		// amunsilence
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amunsilence <player>\n\"" ) );
				 return;
				}
			if (other->client->sess.silence)
			{
				other->client->sess.silence = qfalse;
			}
			break;
		case 18:
		//////////////////
		// amkill
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amkill <player>\n\"" ) );
				 return;
				}
			//if ( !other->client->sess.sleep || !other->client->sess.punish )
			//{//let punished/sleeped people die..

				Cmd_Kill_f (other);
			if (other->health < 1)
				{
				float presaveVel = other->client->ps.velocity[2];
				other->client->ps.velocity[2] = 500;
				DismembermentTest(other);
				other->client->ps.velocity[2] = presaveVel;
				}
			//}
			break;
		case 19:
		//////////////////
		// ampsay
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/ampsay <player> <message>\n\"" ) );
				 return;
				}
				concatted = ConcatArgs( 2 );
				dsp_stringEscape(concatted, concatted, MAX_STRING_CHARS);
				trap_SendServerCmd( idnum, va("cp \"\n\n\n\n\n\n%s^7:\n^7%s\"", ent->client->pers.netname, concatted));
			break;
		case 20:
		//////////////////
		// amshowmotd
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amshowmotd <player>\n\"" ) );
				 return;
				}
			dsp_doMOTD( other );
			break;
		case 21:
		//////////////////
		// amrename
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amrename <player> <newname>\n\"" ) );
				 return;
				}
			if (trap_Argc() == 3)
			{
			newName = ConcatArgs(2);
			if (other->client && other->inuse)
				{
				trap_SendServerCmd(-1, va("print \"%s ^7has been renamed to %s ^7by %s^7.\n\"", other->client->pers.netname, newName, ent->client->pers.netname));

				trap_GetUserinfo(idnum, userinfo, MAX_INFO_STRING);
				strcpy(other->client->pers.netname, newName);
				Info_SetValueForKey(userinfo, "name", newName);
				trap_SetUserinfo(idnum, userinfo);
				ClientUserinfoChanged( idnum );
				}
			}
			break;
		case 22:
		//////////////////
		// amcheats
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amcheats <player>\n\"" ) );
				 return;
				}
			if (other->client->sess.cheat == 0)
			{
				other->client->sess.cheat = 1;
				trap_SendServerCmd(clientNum, va("print \"%s^1: Cheats activated.\n\"", other->client->pers.netname));
			}
			else
			{
				other->client->sess.cheat = 0;
				trap_SendServerCmd(clientNum, va("print \"%s^1: Cheats deactivated.\n\"", other->client->pers.netname));
			}
			break;
		case 23:
		//////////////////
		// amempower
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amempower <player>\n\"" ) );
				 return;
				}
		if (!other->client->sess.empower)
			{
				twimod_admincmds(ent, clientNum, 26);
				other->client->sess.empower = qtrue;
				other->client->ps.saberHolstered = qtrue;
				other->client->ps.isJediMaster = qtrue;

				other->client->ps.fd.forcePower = 400;
				other->client->ps.forceRestricted = qfalse;

						//other->client->ps.dualBlade = qtrue;

					G_ScreenShake(vec3_origin, NULL, 5.0f, 500, qtrue);
					G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex("sound/chars/reborn2/misc/victory1") );

				for (f=0; f<NUM_FORCE_POWERS; f++)
				{
					other->client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
					other->client->ps.fd.forcePowersKnown |= (1 << f);
				}
			}
			break;
		case 24:
		//////////////////
		// amunempower
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amunempower <player>\n\"" ) );
				 return;
				}
			if (other->client->sess.empower)
			{
				other->client->sess.empower = qfalse;
				other->client->ps.saberHolstered = qtrue;
				other->client->ps.isJediMaster = qfalse;
				other->client->ps.fd.forcePower = 100;
				//other->client->ps.dualBlade = qfalse;
				WP_InitForcePowers(other);
			}
			break;
		case 25:
		//////////////////
		// amterminator
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amterminator <player>\n\"" ) );
				 return;
				}
			if (!other->client->sess.terminator)
			{
				twimod_admincmds(ent, clientNum, 24);

				other->client->sess.terminator = qtrue;
				other->client->ps.saberHolstered = qtrue;
				other->client->sess.freeze = qfalse;

        G_ScreenShake(vec3_origin, NULL, 5.0f, 500, qtrue);
	VectorClear(angles2);
	angles2[YAW] = 0;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
	VectorClear(angles2);
	angles2[YAW] = 90;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
	VectorClear(angles2);
	angles2[YAW] = 180;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
	VectorClear(angles2);
	angles2[YAW] = 270;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);


				/*if (other->health > 0)
				{
					other->health = other->client->ps.stats[STAT_HEALTH] = other->client->ps.stats[STAT_MAX_HEALTH];
					other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH];
				}*/

				while (f < HI_NUM_HOLDABLE)
				{
					other->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << f);
					f++;
				}
				// Holdable objects such as the drone

				other->client->ps.stats[STAT_WEAPONS] = (1 << (WP_DET_PACK+1))  - ( 1 << WP_NONE );
				other->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
				// Offcourse give the player all the weapons..

				for ( f = 0 ; f < MAX_WEAPONS ; f++ )
				{
					other->client->ps.ammo[f] = 9999;
				}
				other->client->ps.forceRestricted = qtrue;
				other->client->ps.weapon = WP_BRYAR_PISTOL;
			}
			break;
		case 26:
		//////////////////
		// amunterminator
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amunterminator <player>\n\"" ) );
				 return;
				}
			if (other->client->sess.terminator)
			{
				other->client->sess.terminator = qfalse;
				other->client->ps.isJediMaster = qfalse;

				other->client->sess.freeze = qfalse;
				/*if (other->health > 0)
				{
					other->health = other->client->ps.stats[STAT_HEALTH] = other->client->ps.stats[STAT_MAX_HEALTH];
					other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH];
				}*/
				other->client->ps.stats[STAT_HOLDABLE_ITEMS] = 0;
				other->client->ps.stats[STAT_WEAPONS] = 0;
				other->client->ps.stats[STAT_WEAPONS] = (1 << WP_SABER);
				other->client->ps.forceRestricted = qfalse;
				other->client->ps.weapon = WP_SABER;

				WP_InitForcePowers(other);
			}
			break;
		case 50:
		//////////////////
		// ammakeadmin
		//////////////////
		if ( (trap_Argc() < 4) || ( Q_stricmp(par1, "info") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1/ammakeadmin <player> <Set[0] or add[1]> <adminsetting>\n\""));
			return;
		}

		if ( (Q_stricmp(other->client->sess.userlogged, "") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1Target is not logged in.\n\""));
			return;
		}

		if ( idnum == ent->s.number )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You can not target this command on yourself.\n\""));
			return;
		}
		if (qtrue)
		{
			//char	filename[MAX_STRING_CHARS];
			//char	mcampowers1[MAX_STRING_CHARS];
			//char	mcampowers2[MAX_STRING_CHARS];
			//char	mcampowers3[MAX_STRING_CHARS];
			//char	mcampowers4[MAX_STRING_CHARS];
			//char	mcampowers5[MAX_STRING_CHARS];
			//char	mcampowers6[MAX_STRING_CHARS];
			//int		len;
			//int		c;
			//int		o;
			Com_sprintf(savePath, 1024, "defaults/admin_def_%s.cfg", par3);
			trap_FS_FOpenFile(savePath, &f, FS_READ);
			if ( f )
			{
				trap_FS_FCloseFile( f );
				orig1 = trap_FS_FOpenFile(savePath, &f, FS_READ);
				trap_FS_Read( buffer ,orig1 , f );
				i = 0;
				color = 0;
				j = 0;
				for ( i = 0; i <= strlen (buffer); i++ )
				{
					if ((buffer[i] == '²')||(buffer[i] == '\n'))
					{
						j++;
						if ( j == 1 )
							memcpy (string, buffer, i);
						else if ( j == 2 )
							memcpy (userwrite, buffer+color+1, i-color-1);
						else if ( j == 3 )
							memcpy (LogCommand, buffer+color+1, i-color-1);
						else if ( j == 4 )
							memcpy (line, buffer+color+1, i-color-1);
						else if ( j == 5 )
							memcpy (name, buffer+color+1, i-color-1);
						else if ( j == 6 )
							memcpy (userinfo, buffer+color+1, i-color-1);
						color = i;
					}
				}
				if (atoi(par2) == 1)
				{
					other->client->sess.ampowers |= atoi(string);
					other->client->sess.ampowers2 |= atoi(userwrite);
					other->client->sess.ampowers3 |= atoi(LogCommand);
					other->client->sess.ampowers4 |= atoi(line);
					other->client->sess.ampowers5 |= atoi(name);
					other->client->sess.ampowers6 |= atoi(userinfo);
					trap_SendServerCmd( clientNum, va("print \"^2Player ^7%s^2 admin powers adjusted.\n\"", other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers = atoi(string);
					other->client->sess.ampowers2 = atoi(userwrite);
					other->client->sess.ampowers3 = atoi(LogCommand);
					other->client->sess.ampowers4 = atoi(line);
					other->client->sess.ampowers5 = atoi(name);
					other->client->sess.ampowers6 = atoi(userinfo);
					trap_SendServerCmd( clientNum, va("print \"^2Player ^7%s^2 admin powers set.\n\"", other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
		}
		//if ( other->client->sess.adminloggedin == atoi ( par2 ) )
		//{
		//	trap_SendServerCmd( clientNum, va("print \"^1Target is already this adminlevel.\n\""));
		//	return;
		//}
				//other->client->sess.adminloggedin = atoi ( par2 );
				/*if ( other->client->sess.adminloggedin == 0 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7denied admin rights.\n\"", other->client->pers.netname));
						//other->client->sess.ampowers = 0;
						//other->client->sess.ampowers2 = 0;
						//other->client->sess.ampowers3 = 0;
					}
				else if ( other->client->sess.adminloggedin == 1 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7granted %s ^7admin rights.\n\"", other->client->pers.netname, twimod_lvl1name.string));
						//other->client->sess.ampowers = mc_admin_lvl1.integer;
						//other->client->sess.ampowers2 = mc_admin2_lvl1.integer;
						//other->client->sess.ampowers3 = mc_admin3_lvl1.integer;
					}
				else if ( other->client->sess.adminloggedin == 2 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7granted %s ^7admin rights.\n\"", other->client->pers.netname, twimod_lvl2name.string));
						//other->client->sess.ampowers = mc_admin_lvl2.integer;
						//other->client->sess.ampowers2 = mc_admin2_lvl2.integer;
						//other->client->sess.ampowers3 = mc_admin3_lvl2.integer;
					}
				else if ( other->client->sess.adminloggedin == 3 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7granted %s ^7admin rights.\n\"", other->client->pers.netname, twimod_lvl3name.string));
						//other->client->sess.ampowers = mc_admin_lvl3.integer;
						//other->client->sess.ampowers2 = mc_admin2_lvl3.integer;
						//other->client->sess.ampowers3 = mc_admin3_lvl3.integer;
					}
				else if ( other->client->sess.adminloggedin == 4 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7granted %s ^7admin rights.\n\"", other->client->pers.netname, twimod_lvl4name.string));
						//other->client->sess.ampowers = mc_admin_lvl4.integer;
						//other->client->sess.ampowers2 = mc_admin2_lvl4.integer;
						//other->client->sess.ampowers3 = mc_admin3_lvl4.integer;
					}
				else if ( other->client->sess.adminloggedin == 5 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7granted %s ^7admin rights.\n\"", other->client->pers.netname, twimod_lvl5name.string));
						//other->client->sess.ampowers = mc_admin_lvl5.integer;
						//other->client->sess.ampowers2 = mc_admin2_lvl5.integer;
						//other->client->sess.ampowers3 = mc_admin3_lvl5.integer;
					}
				else if ( other->client->sess.adminloggedin == 6 )
					{
						trap_SendServerCmd( ent->s.number, va("print \"%s ^7granted %s ^7admin rights.\n\"", other->client->pers.netname, twimod_lvl6name.string));
						//other->client->sess.ampowers = mc_admin_lvl6.integer;
						//other->client->sess.ampowers2 = mc_admin2_lvl6.integer;
						//other->client->sess.ampowers3 = mc_admin3_lvl6.integer;
					}*/
				//mc_updateaccount(other);
				break;

		case 52:
		//////////////////
		// amannounce (old)
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par2, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amannounce <player> <message>\n\"" ) );
				 return;
				}
				trap_SendServerCmd( idnum, va("print \"%s \n\"", par2));
			break;
		case 53:
		/////////////////
		// amaddscore
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amaddscore <player> <score to add>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amaddscore <player> <score to add>\n\"" ) );
			return;
		}
		other->client->ps.persistant[PERS_SCORE] += atoi (par2);
		CalculateRanks();
		break;
		case 54:
		/////////////////
		// amsetscore
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetscore <player> <score>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetscore <player> <score>\n\"" ) );
			return;
		}
		other->client->ps.persistant[PERS_SCORE] = atoi (par2);
		CalculateRanks();
		break;

		case 55:
		//////////////////
		// amannounce
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par2, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amannounce <player> <message>\n\"" ) );
				 return;
				}
				concatted = ConcatArgs( 2 );
				dsp_stringEscape(concatted, concatted, MAX_STRING_CHARS);
				trap_SendServerCmd( idnum, va("print \"%s\n\"", concatted));
			break;

		case 56:
		/////////////////
		// amgivehealth
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgivehealth <player> <health to add> ( max health is 32677)\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgivehealth <player> <health to add> ( max health is 32677)\n\"" ) );
			return;
		}
		if (other->health <= 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Cannot revive.\n\"" ) );
			return;
		}
		other->health += atoi (par2);
		break;

		case 57:
		/////////////////
		// amsethealth
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsethealth <player> <health> ( max health is 32677)\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsethealth <player> <health> ( max health is 32677)\n\"" ) );
			return;
		}
		if (other->health <= 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Cannot revive.\n\"" ) );
			return;
		}
		other->health = atoi (par2);
		break;
		case 58:
		//////////////////
		// amslap2
		//////////////////
					if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par4, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amslap2 <player> <0 = away, 1 = towards, 2 = random> <XY speed> <Z speed>\n\"" ) );
				 return;
				}
			if (!other->client->sess.punish && !other->client->sess.protect && !other->client->sess.freeze)
			{
				if (!other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}
				VectorNormalize2( ent->client->ps.velocity, oppDir );
				VectorScale( oppDir, -1, oppDir );
				if (atoi (par2) == 0) // slap away
				{
					angle = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
				}
				else if (atoi (par2) == 1) // slap towards
				{
					angle = (ent->client->ps.viewangles[YAW]+180) * (M_PI*2 / 360);
				}
				else// slap randomly
				{
					angle = Q_irand(1, 360) * (M_PI*2 / 360);
				}
				oppDir[1] = (sin(angle)*atoi (par3));
				oppDir[0] = (cos(angle)*atoi (par3));
				other->client->ps.velocity[0] = oppDir[0];
				other->client->ps.velocity[1] = oppDir[1];
				other->client->ps.velocity[2] = (atoi (par4));
				other->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
				other->client->ps.forceHandExtendTime = level.time + (twimod_slapdowntime.integer * 1000);
				other->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
			}
			break;

		case 59:
		/////////////////
		// amplayfx
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amplayfx <player> <zoffset> <effect> OR /amplayfx <player> <zoffset> <effect> <yaw> <pitch> <roll>\n\"" ) );
			return;
		}
		if (trap_Argc() < 4)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amplayfx <player> <zoffset> <effect> OR /amplayfx <player> <zoffset> <effect> <yaw> <pitch> <roll>\n\"" ) );
			return;
		}
		//G_AddEvent( other, EV_PLAY_EFFECT_ID, G_EffectIndex( par2 ) );
		//G_PlayEffect(EFFECT_EXPLOSION_PAS, self->s.pos.trBase, self->s.angles);
		strcpy(buffer,par3);
		VectorClear(origin2);
		origin2[0] = other->client->ps.origin[0];
		origin2[1] = other->client->ps.origin[1];
		origin2[2] = other->client->ps.origin[2]+atoi(par2);
		if ( Q_stricmp(par4, "") == 0)
		{
			G_PlayEffect_ID(G_EffectIndex( buffer ), origin2, other->client->ps.viewangles);
			return;
		}
		else
		{
			VectorClear(angles2);
			angles2[YAW] = atoi(par4);
			angles2[PITCH] = atoi(par5);
			angles2[ROLL] = atoi(par6);
			G_PlayEffect_ID(G_EffectIndex( buffer ), origin2, angles2);
		}
		/*ent = G_Spawn();
		G_ParseField( "origin", vtos(other->s.origin), ent );
		ent->s.origin[0] = other->s.origin[0];
		ent->s.origin[1] = other->s.origin[1];
		ent->s.origin[2] = other->s.origin[2];
		VectorCopy(ent->s.origin, other->s.origin);
		ent->bolt_Head = G_EffectIndex( par2 );
		SP_fx_mcrunner(ent);*/
		break;

		case 60:
		/////////////////
		// amforceteam
		/////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amforceteam <player> <team>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amforceteam <player> <team>\n\"" ) );
			return;
		}
		//Cmd_Team_f(other);
		SetTeam( other, par2 );
		break;
		case 61:
		/////////////////
		// amdualsaber
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amdualsaber <player>\n\"" ) );
			return;
		}
		if (trap_Argc() < 2)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amdualsaber <player>\n\"" ) );
			return;
		}
			if (other->client->ps.dualBlade)
			{
				other->client->ps.dualBlade = qfalse;
			}
			else
			{
				other->client->ps.dualBlade = qtrue;
			}
		break;
		case 63:
		/////////////////
		// amdoemote
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amdoemote <player> <emotenum>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amdoemote <player> <emotenum>\n\"" ) );
			return;
		}
		dsp_doEmote(other, atoi(par2));
		break;
		case 64:
		/////////////////
		// amgivecredits
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgivecredits <player> <amount to add>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgivecredits <player> <amount to add>\n\"" ) );
			return;
		}
		mc_addcredits(other, atoi(par2));
		trap_SendServerCmd( ent->s.number, va("print \"^7Client %s^7 has been given ^5%i^7 credits for a total of ^5%i^7.\n\"", other->client->pers.netname, atoi(par2), other->client->sess.credits ) );
		break;
		case 65:
		/////////////////
		// amsetcredits
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetcredits <player> <credits>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetcredits <player> <credits>\n\"" ) );
			return;
		}
		mc_setcredits(other, atoi(par2));
		trap_SendServerCmd( ent->s.number, va("print \"^7Client %s^7 now has ^5%i^7 credits.\n\"", other->client->pers.netname, other->client->sess.credits ) );
		break;
		case 66:
		/////////////////
		// amsetspeed
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetspeed <player> <newspeed>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetspeed <player> <newspeed>\n\"" ) );
			return;
		}
		other->client->sess.mcspeed = atoi(par2);
		break;
		case 67:
		/////////////////
		// amsetgravity
		/////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetspeed <player> <newspeed>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetspeed <player> <newspeed>\n\"" ) );
			return;
		}
		other->client->sess.mcgravity = atoi(par2);
		break;
		case 68:
		//////////////////
		// amgettoother
		//////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgettoother <player to tele> <player to tele to>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1amgettoother <player to tele> <player to tele to>\n\"" ) );
			return;
		}
		else
		{
			if ( idnum == -1 )
			{
				trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
				return;
			}
			else
			{
				client2num = dsp_adminTarget(ent, par2, clientNum);
				if (client2num == -1)
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
					return;
				}
				if (client2num == -2)
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7No secondary target found^2:.\n\"" ) );
					return;
				}
				if (client2num == -3)
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7No secondary target found^2:.\n\"" ) );
					return;
				}
				if (other->client->sess.noteleport == 1)
				{
					if (!(ent->client->sess.ampowers7 & 32))
					{
						trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported.^7\n\"" ) );
						return;
					}
				}
				client2 = &g_entities[client2num];
			if (client2->client->sess.noteleport == 1)
			{
				if (!(ent->client->sess.ampowers7 & 32))
				{
					trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported.^7\n\"" ) );
					return;
				}
			}
				trap_UnlinkEntity (other);
				VectorClear( origin );
				VectorClear( angles );
				//hoek = client2->client->ps.viewangles[YAW] * (M_PI*2 / 360);
				//origin[0] = client2->client->ps.origin[0] + ( cos(hoek) * 200 ); // X
				//origin[1] = client2->client->ps.origin[1] + ( sin(hoek) * 200 ); // Y
				//origin[2] = client2->client->ps.origin[2]; // Z
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = client2->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = client2->client->ps.origin[0] + fwd[0]*128;
		origin[1] = client2->client->ps.origin[1] + fwd[1]*128;
		origin[2] = client2->client->ps.origin[2]/* + fwd[2]*64*/;
				if (Q_stricmp( par3, "1" ) == 0)
				{
					angles[YAW] = AngleNormalize360(client2->client->ps.viewangles[YAW]); // Other-Player viewangles [YAW]
				}
				else
				{
					angles[YAW] = AngleNormalize360(client2->client->ps.viewangles[YAW] + 180); // Other-Player viewangles [YAW]
				}
				TeleportPlayer( other, origin, angles );
			}
		}
		break;
		case 69:
		//////////////////
		// amswapplayers
		//////////////////
		if ( Q_stricmp(par1, "info") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amswapplayers <player1> <player2>\n\"" ) );
			return;
		}
		if (trap_Argc() < 3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1amswapplayers <player1> <player2>\n\"" ) );
			return;
		}
		else
		{
			if ( idnum == -1 )
			{
				trap_SendServerCmd( clientNum, va("print \"^1You can not use this command on all.^7\n\"" ) );
				return;
			}
			else
			{
			if (other->client->sess.noteleport == 1)
			{
				if (!(ent->client->sess.ampowers7 & 32))
				{
					trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported.^7\n\"" ) );
					return;
				}
			}
				client2num = dsp_adminTarget(ent, par2, clientNum);
				if (client2num == -1)
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
					return;
				}
				if (client2num == -2)
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7No secondary target found^2:.\n\"" ) );
					return;
				}
				if (client2num == -3)
				{
					trap_SendServerCmd( clientNum, va("print \"^2.: ^7No secondary target found^2:.\n\"" ) );
					return;
				}
				client2 = &g_entities[client2num];
			if (client2->client->sess.noteleport == 1)
			{
				if (!(ent->client->sess.ampowers7 & 32))
				{
					trap_SendServerCmd( clientNum, va("print \"^3Chosen client does not want to be teleported.^7\n\"" ) );
					return;
				}
			}
				trap_UnlinkEntity (other);
				trap_UnlinkEntity (client2);
				VectorClear( origin );
				VectorClear( origin2 );
				VectorClear( angles );
				VectorClear( angles2 );
				VectorCopy(client2->client->ps.origin,origin);
				VectorCopy(other->client->ps.origin,origin2);
				VectorCopy(client2->client->ps.viewangles,angles);
				VectorCopy(other->client->ps.viewangles,angles2);
				/*VectorClear( origin );
				VectorClear( angles );
				origin[0] = client2->client->ps.origin[0]; // X
				origin[1] = client2->client->ps.origin[1]; // Y
				origin[2] = client2->client->ps.origin[2]; // Z
				angles[YAW] = AngleNormalize360(client2->client->ps.viewangles[YAW]);
				trap_UnlinkEntity (client2);
				VectorClear( origin );
				VectorClear( origin2 );
				VectorClear( angles );
				orig1 = other->client->ps.origin[0]; // X
				orig2 = other->client->ps.origin[1]; // Y
				orig3 = other->client->ps.origin[2]; // Z
				angles2[YAW] = AngleNormalize360(other->client->ps.viewangles[YAW]);*/

				TeleportPlayer( other, origin, angles );
				/*origin2[0] = orig1;
				origin2[1] = orig2;
				origin2[2] = orig3;*/
				TeleportPlayer( client2, origin2, angles2 );
			}
		}
		break;/*
		case 70:
		//////////////////
		// ammakebuilderadmin
		//////////////////
		if ( (trap_Argc() != 3) || ( Q_stricmp(par1, "info") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1/ammakebuilderadmin <player> <builderadminlevelnumber>\n\""));
			return;
		}

		if ( (Q_stricmp(other->client->sess.userlogged, "") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1Target is not logged in.\n\""));
			return;
		}

		if ( idnum == ent->s.number )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You can not target this command on yourself.\n\""));
			return;
		}

		if ( other->client->sess.builderadmin == atoi ( par2 ) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1Target is already this builderadminlevel.\n\""));
			return;
		}
				other->client->sess.builderadmin = atoi ( par2 );
				mc_updateaccount(other);
				trap_SendServerCmd( ent->s.number, va("print \"%s ^7denied admin rights.\n\"", other->client->pers.netname));
		break;*/
		case 71:
		//////////////////
		// amgod
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgod <player>\n\""));
			return;
		}
		other->flags ^= FL_GODMODE;
		if (!(other->flags & FL_GODMODE) )
		{
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 has lost godmode.\n\"", other->client->pers.netname));
		}
		else
		{
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 has been given godmode.\n\"", other->client->pers.netname));
		}
		break;
		case 72:
		//////////////////
		// amnoclip
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amnoclip <player>\n\""));
			return;
		}
		if (other->health <= 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Cannot noclip dead people.\n\""));
			return;
		}
		if (other->client->noclip)
		{
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now clipped.\n\"", other->client->pers.netname));
		}
		else
		{
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now noclipped.\n\"", other->client->pers.netname));
		}
		other->client->noclip = !other->client->noclip;
		break;
		case 73:
		//////////////////
		// amgive
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amgive <player> <item> <count>\n\""));
			return;
		}
		if (other->health <= 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Cannot give to dead people.\n\""));
			return;
		}
		trap_Argv( 2, name, sizeof( name ) );
		if (Q_stricmp(name, "all") == 0)
			give_all = qtrue;
		else
			give_all = qfalse;

		if (give_all)
		{
			i = 0;
			while (i < HI_NUM_HOLDABLE)
			{
				other->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
				i++;
			}
			i = 0;
		}
		if (give_all || Q_stricmp( name, "health") == 0)
		{
			if (trap_Argc() == 4) {
				trap_Argv( 3, arg, sizeof( arg ) );
				other->health = atoi(arg);
				if (other->health > other->client->ps.stats[STAT_MAX_HEALTH]) {
					other->health = other->client->ps.stats[STAT_MAX_HEALTH];
				}
			}
			else {
				other->health = other->client->ps.stats[STAT_MAX_HEALTH];
			}
			if (!give_all)
				return;
		}

		if (give_all || Q_stricmp(name, "weapons") == 0)
		{
			other->client->ps.stats[STAT_WEAPONS] = (1 << (WP_DET_PACK+1))  - ( 1 << WP_NONE );
			if (!give_all)
				return;
		}
		if (give_all || Q_stricmp(name, "battlesuit") == 0)
		{
			int num = 1000;
			if (trap_Argc() == 4) {
				trap_Argv( 3, arg, sizeof( arg ) );
				num = atoi(arg);
			}
			if (other->client->ps.powerups[PW_BATTLESUIT] < level.time)
			{
				other->client->ps.powerups[PW_BATTLESUIT] = level.time+num;
			}
			else
			{
				other->client->ps.powerups[PW_BATTLESUIT] += num;
			}
			if (!give_all)
				return;
		}

		if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
		{
			trap_Argv( 3, arg, sizeof( arg ) );
			other->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
			return;
		}
		if (give_all || Q_stricmp(name, "ammo") == 0)
		{
			int num = 999;
			if (trap_Argc() == 4) {
				trap_Argv( 3, arg, sizeof( arg ) );
				num = atoi(arg);
			}
			for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
				other->client->ps.ammo[i] = num;
			}
			if (!give_all)
				return;
		}

		if (give_all || Q_stricmp(name, "armor") == 0)
		{
			if (trap_Argc() == 4) {
				trap_Argv( 3, arg, sizeof( arg ) );
				other->client->ps.stats[STAT_ARMOR] = atoi(arg);
			} else {
				other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH];
			}

			if (!give_all)
				return;
		}

		if (give_all || Q_stricmp(name, "force") == 0)
		{
			if (trap_Argc() == 4) {
				trap_Argv( 3, arg, sizeof( arg ) );
				other->client->ps.fd.forcePower = atoi(arg);
			} else {
				other->client->ps.fd.forcePower = 100;
			}

			if (!give_all)
				return;
		}

		if (give_all || Q_stricmp(name, "fuel") == 0)
		{
			if (trap_Argc() == 4) {
				trap_Argv( 3, arg, sizeof( arg ) );
				other->client->sess.jetfuel = atoi(arg);
			} else {
				other->client->sess.jetfuel = mc_jetpack_fuelmax.integer;
			}

			if (!give_all)
				return;
		}

		// spawn a specific item right on the player
		if ( !give_all ) {
			it = BG_FindItem (name);
			if (!it) {
				return;
			}

			it_ent = G_Spawn();
			VectorCopy( other->r.currentOrigin, it_ent->s.origin );
			it_ent->classname = it->classname;
			G_SpawnItem (it_ent, it);
			FinishSpawningItem(it_ent );
			memset( &trace, 0, sizeof( trace ) );
			Touch_Item (it_ent, other, &trace);
			if (it_ent->inuse) {
				G_FreeEntity( it_ent );
			}
		}
		break;
		case 74:
		//////////////////
		// amillusion
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amillusion <player>\n\""));
			return;
		}
		//CopyToBodyQue(other);
		illusionspawn(other);
		break;
		case 75:
		//////////////////
		// amforcegod
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amforcegod <player>\n\""));
			return;
		}
		if (other->client->sess.forcegod == 0)
		{
			VectorClear(angles2);
			angles2[PITCH] = 270;
			mc_doEmote(other,710);
			G_PlayEffect_ID(G_EffectIndex( "env/pool" ), other->client->ps.origin, angles2);
			other->client->sess.freeze = qtrue;
			other->client->ps.pm_type = PM_FREEZE;
			other->client->sess.forcegod = 2;
			other->client->sess.forcegod2 = 1;
			G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex("sound/chars/reborn3/misc/victory3") );
			G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex("sound/chars/reborn3/misc/victory3") );
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now a force god.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.forcegod = 0;
			other->client->sess.forcegod2 = 0;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer a force god.\n\"", other->client->pers.netname));
		}
		break;
		case 76:
		//////////////////
		// amsetprefix
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetprefix <player> <prefix>\n\""));
			return;
		}
		concatted = ConcatArgs( 2 );
		strcpy(other->client->sess.amprefix, concatted);
		break;
		case 77:
		//////////////////
		// amsetsuffix
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsetsuffix <player> <suffix>\n\""));
			return;
		}
		concatted = ConcatArgs( 2 );
		strcpy(other->client->sess.amsuffix, concatted);
		break;
		case 78:
		//////////////////
		// amfakechat
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par2, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amfakechat <player> <chat message>\n\""));
			return;
		}
		concatted = ConcatArgs( 2 );
		//for (j = 0; j < level.maxclients; j++) {
			//other2 = &g_entities[j];
			trap_SendServerCmd( -1, va("chat \"%s^7: ^2%s%s%s\"", other->client->pers.netname, other->client->sess.amprefix, concatted, other->client->sess.amsuffix));
		//}
		break;
		case 79:
		//////////////////
		// amnodrown
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amnodrown <player>\n\""));
			return;
		}
		if (other->client->sess.nodrown == 0)
		{
			other->client->sess.nodrown = 1;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 will no longer drown.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.nodrown = 0;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 can now drown.\n\"", other->client->pers.netname));
		}
		break;
		case 80:
		//////////////////
		// amsolid
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsolid <player>\n\""));
			return;
		}
		if (other->client->sess.solid == 0)
		{
			other->client->sess.solid = 1;
			//other->s.eFlags |= EF_NODRAW;
			//other->r.svFlags |= SVF_BROADCAST;
			other->r.contents = 0;
			other->clipmask = 0;
			//trap_UnlinkEntity( other );
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now nonsolid.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.solid = 0;
			other->clipmask = MASK_SOLID;
			other->r.contents = CONTENTS_SOLID;
			//other->s.eFlags &= ~EF_NODRAW;
			//other->r.svFlags &= ~SVF_BROADCAST;
			//trap_LinkEntity( other );
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now solid.\n\"", other->client->pers.netname));
		}
		break;
		case 81:
		//////////////////
		// amresetfiretime
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amresetfiretime <player> <time>\n\""));
			return;
		}
		//other->client->dangerTime += atoi(par2);
		other->client->ps.weaponTime = atoi(par2);
		break;
		case 82:
		//////////////////
		// aminvisible
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/aminvisible <player>\n\""));
			return;
		}
		if (other->client->sess.isglowing == 0)
		{
			other->client->sess.isglowing = 1;
			other->client->ps.eFlags |= EF_NODRAW;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now invisible.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.isglowing = 0;
			WP_ForcePowerStop( other, FP_TELEPATHY );
			//for (i = 0;i < 33;i += 1)
			//{
				//other->client->sess.trickedplayers[i] = '';
			//}
			other->client->ps.eFlags &= ~EF_NODRAW;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer invisible.\n\"", other->client->pers.netname));
		}
		break;
		case 84:
		//////////////////
		// amfakelag
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amfakelag <player>\n\""));
			return;
		}
		if (other->client->sess.fakelag == 0)
		{
			other->client->sess.fakelag = 1;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now lagging.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.fakelag = 0;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer lagging.\n\"", other->client->pers.netname));
		}
		break;
		case 85:
		//////////////////
		// amdropfakeweapon
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amdropfakeweapon <player> <weapnum> <speed>\n\""));
			return;
		}
		VectorClear(origin2);
		VectorClear(origin3);
		hoek = other->client->ps.viewangles[YAW] * (M_PI*2 / 360);
		origin2[0] = other->client->ps.viewangles[0]*atoi(par3);
		origin2[1] = other->client->ps.viewangles[1]*atoi(par3);
		origin2[2] = other->client->ps.viewangles[2]*atoi(par3);
		origin3[0] = other->client->ps.origin[0] + ( cos(hoek) * 100 );
		origin3[1] = other->client->ps.origin[1] + ( sin(hoek) * 100 );
		origin3[2] = other->client->ps.origin[2];
		if (atoi(par2) == 2)
		{
			LaunchItem(BG_FindItemForWeapon( WP_SABER ), origin3, origin2);
		}
		else if (atoi(par2) == 1)
		{
			LaunchItem(BG_FindItemForWeapon( WP_STUN_BATON ), origin3, origin2);
		}
		else if (atoi(par2) == 3)
		{
			LaunchItem(BG_FindItemForWeapon( WP_BRYAR_PISTOL ), origin3, origin2);
		}
		else if (atoi(par2) == 4)
		{
			LaunchItem(BG_FindItemForWeapon( WP_BLASTER ), origin3, origin2);
		}
		else if (atoi(par2) == 5)
		{
			LaunchItem(BG_FindItemForWeapon( WP_DISRUPTOR ), origin3, origin2);
		}
		else if (atoi(par2) == 6)
		{
			LaunchItem(BG_FindItemForWeapon( WP_BOWCASTER ), origin3, origin2);
		}
		else if (atoi(par2) == 7)
		{
			LaunchItem(BG_FindItemForWeapon( WP_REPEATER ), origin3, origin2);
		}
		else if (atoi(par2) == 8)
		{
			LaunchItem(BG_FindItemForWeapon( WP_DEMP2 ), origin3, origin2);
		}
		else if (atoi(par2) == 9)
		{
			LaunchItem(BG_FindItemForWeapon( WP_FLECHETTE ), origin3, origin2);
		}
		else if (atoi(par2) == 10)
		{
			LaunchItem(BG_FindItemForWeapon( WP_ROCKET_LAUNCHER ), origin3, origin2);
		}
		else if (atoi(par2) == 11)
		{
			LaunchItem(BG_FindItemForWeapon( WP_THERMAL ), origin3, origin2);
		}
		else if (atoi(par2) == 12)
		{
			LaunchItem(BG_FindItemForWeapon( WP_TRIP_MINE ), origin3, origin2);
		}
		else if (atoi(par2) == 13)
		{
			LaunchItem(BG_FindItemForWeapon( WP_DET_PACK ), origin3, origin2);
		}
		else
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^3/amdropfakeweapon <player> <weapon[1 to 13]> <speed>^7\n\""));
		}
		break;
		case 86:
		//////////////////
		// amallpowerful
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amallpowerful <player>\n\"" ) );
			return;
		}
		if (other->client->sess.allpowerful == 1)
		{
			other->client->sess.empower = qfalse;
			other->client->sess.allpowerful = 0;
			other->client->ps.dualBlade = qfalse;
			other->client->sess.terminator = qfalse;
			other->client->ps.isJediMaster = qfalse;
			WP_InitForcePowers(other);
			return;
		}
		other->client->sess.allpowerful = 1;
		other->client->sess.empower = qtrue;
		other->client->ps.isJediMaster = qfalse;
		other->client->ps.dualBlade = qtrue;
		other->client->ps.fd.forcePower = 400;
		other->client->ps.forceRestricted = qfalse;
		other->client->sess.terminator = qtrue;
		other->client->sess.freeze = qfalse;
		for (f=0; f<NUM_FORCE_POWERS; f++)
		{
			other->client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
			other->client->ps.fd.forcePowersKnown |= (1 << f);
		}
        	G_ScreenShake(vec3_origin, NULL, 5.0f, 500, qtrue);
		G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex("sound/chars/reborn2/misc/victory1") );
	VectorClear(angles2);
	angles2[YAW] = 0;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
	VectorClear(angles2);
	angles2[YAW] = 90;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
	VectorClear(angles2);
	angles2[YAW] = 180;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
	VectorClear(angles2);
	angles2[YAW] = 270;
	G_PlayEffect_ID(G_EffectIndex( "env/fire_wall" ), other->client->ps.origin, angles2);
		if (other->health > 0)
		{
			other->health = other->client->ps.stats[STAT_HEALTH] = other->client->ps.stats[STAT_MAX_HEALTH];
			other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH];
		}
		while (f < HI_NUM_HOLDABLE)
		{
			other->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << f);
			f++;
		}
		other->client->ps.stats[STAT_WEAPONS] = (1 << (WP_DET_PACK+1))  - ( 1 << WP_NONE );
		for ( f = 0 ; f < MAX_WEAPONS ; f++ )
		{
			other->client->ps.ammo[f] = 9999;
		}
		break;
		case 87:
		//////////////////
		// amfastjet
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amfastjet <player>\n\""));
			return;
		}
		if (other->client->sess.jetspeed == 0)
		{
			other->client->sess.jetspeed = 500;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 now has a fast jetpack.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.jetspeed = 0;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 no longer has a fast jetpack.\n\"", other->client->pers.netname));
		}
		break;
		case 88:
		//////////////////
		// amjetspeed
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amjetspeed <player> <speed>\n\""));
			return;
		}
		if (( Q_stricmp(par2, "") == 0))
		{
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 has a jetpack speed of ^5%i^3.\n\"", other->client->pers.netname, other->client->sess.jetspeed));
			return;
		}
		other->client->sess.jetspeed = atoi(par2);
		trap_SendServerCmd( ent-g_entities, va("print \"%s^7 now has a jetpack speed of ^5%i^3.\n\"", other->client->pers.netname, atoi(par2)));
		break;
		case 89:
		//////////////////
		// amjoingroup
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amjoingroup <player> <group>\n\""));
			return;
		}
		if (( Q_stricmp(par2, "") == 0))
		{
		i = 0;
		strcpy(buffer,va("%s^7 is in group(s): ",other->client->pers.netname));
		if (isingroup(other,1)){i = 1;strcpy(buffer,va("%s^7%s(1)",buffer,mc_group1_name.string));}
		else if (isingroup(other,2)){i = 2;strcpy(buffer,va("%s^7%s(2)",buffer,mc_group2_name.string));}
		else if (isingroup(other,3)){i = 3;strcpy(buffer,va("%s^7%s(3)",buffer,mc_group3_name.string));}
		else if (isingroup(other,4)){i = 4;strcpy(buffer,va("%s^7%s(4)",buffer,mc_group4_name.string));}
		else if (isingroup(other,5)){i = 5;strcpy(buffer,va("%s^7%s(5)",buffer,mc_group5_name.string));}
		else if (isingroup(other,6)){i = 5;strcpy(buffer,va("%s^7%s(6)",buffer,mc_group6_name.string));}
		//if (isingroup(other,1)){strcpy(buffer,va("%s^7%s(1)",buffer,mc_group1_name.string));}
		if (isingroup(other,2)&&i < 2){strcpy(buffer,va("%s^7 and ^7%s(2)",buffer,mc_group2_name.string));}
		if (isingroup(other,3)&&i < 3){strcpy(buffer,va("%s^7 and ^7%s(3)",buffer,mc_group3_name.string));}
		if (isingroup(other,4)&&i < 4){strcpy(buffer,va("%s^7 and ^7%s(4)",buffer,mc_group4_name.string));}
		if (isingroup(other,5)&& i < 5){strcpy(buffer,va("%s^7 and ^7%s(5)",buffer,mc_group5_name.string));}
		if (isingroup(other,6)&& i < 6){strcpy(buffer,va("%s^7 and ^7%s(6)",buffer,mc_group6_name.string));}
		strcpy(buffer,va("%s^7,",buffer));
		trap_SendServerCmd( ent-g_entities, va("print \"%s\"", buffer));
		strcpy(buffer,va(" but is not in group(s): ",other->client->pers.netname));
		i = 0;
		if (!isingroup(other,1)){i = 1;strcpy(buffer,va("%s^7%s(1)",buffer,mc_group1_name.string));}
		else if (!isingroup(other,2)){i = 2;strcpy(buffer,va("%s^7 and ^7%s(2)",buffer,mc_group2_name.string));}
		else if (!isingroup(other,3)){i = 3;strcpy(buffer,va("%s^7 and ^7%s(3)",buffer,mc_group3_name.string));}
		else if (!isingroup(other,4)){i = 4;strcpy(buffer,va("%s^7 and ^7%s(4)",buffer,mc_group4_name.string));}
		else if (!isingroup(other,5)){i = 5;strcpy(buffer,va("%s^7 and ^7%s(5)",buffer,mc_group5_name.string));}
		else if (!isingroup(other,6)){i = 5;strcpy(buffer,va("%s^7 and ^7%s(6)",buffer,mc_group6_name.string));}
		//if ((!isingroup(other,1))&&i = 1){strcpy(buffer,va("%s^7%s(1)",buffer,mc_group1_name.string));}
		if ((!isingroup(other,2))&&i < 2){strcpy(buffer,va("%s^7 and ^7%s(2)",buffer,mc_group2_name.string));}
		if ((!isingroup(other,3))&&i < 3){strcpy(buffer,va("%s^7 and ^7%s(3)",buffer,mc_group3_name.string));}
		if ((!isingroup(other,4))&&i < 4){strcpy(buffer,va("%s^7 and ^7%s(4)",buffer,mc_group4_name.string));}
		if ((!isingroup(other,5))&&i < 5){strcpy(buffer,va("%s^7 and ^7%s(5)",buffer,mc_group5_name.string));}
		if ((!isingroup(other,6))&&i < 5){strcpy(buffer,va("%s^7 and ^7%s(6)",buffer,mc_group6_name.string));}
		strcpy(buffer,va("%s^7.",buffer));
			trap_SendServerCmd( ent-g_entities, va("print \"%s\n^3/amjoingroup <player> <groupnumber>\n\"", buffer));
			return;
		}
		//other->client->sess.mcgroup = atoi(par2);
		switch (atoi(par2))
		{
			case 1:
				if (isingroup(other,1) == 1)
				{
					other->client->sess.mygroup[0] = '0';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer in group %s^7(^51^7).\n\"", other->client->pers.netname, mc_group1_name.string));
				}
				else
				{
					other->client->sess.mygroup[0] = 'A';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group %s^7(^51^7).\n\"", other->client->pers.netname, mc_group1_name.string));
				}
				break;
			case 2:
				if (isingroup(other,2) == 1)
				{
					other->client->sess.mygroup[1] = '0';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer in group %s^7(^52^7).\n\"", other->client->pers.netname, mc_group2_name.string));
				}
				else
				{
					other->client->sess.mygroup[1] = 'A';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group %s^7(^52^7).\n\"", other->client->pers.netname, mc_group2_name.string));
				}
				break;
			case 3:
				if (isingroup(other,3) == 1)
				{
					other->client->sess.mygroup[2] = '0';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer in group %s^7(^53^7).\n\"", other->client->pers.netname, mc_group3_name.string));
				}
				else
				{
					other->client->sess.mygroup[2] = 'A';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group %s^7(^53^7).\n\"", other->client->pers.netname, mc_group3_name.string));
				}
				break;
			case 4:
				if (isingroup(other,4) == 1)
				{
					other->client->sess.mygroup[3] = '0';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer in group %s^7(^54^7).\n\"", other->client->pers.netname, mc_group4_name.string));
				}
				else
				{
					other->client->sess.mygroup[3] = 'A';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group %s^7(^54^7).\n\"", other->client->pers.netname, mc_group4_name.string));
				}
				break;
			case 5:
				if (isingroup(other,5) == 1)
				{
					other->client->sess.mygroup[4] = '0';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer in group %s^7(^55^7).\n\"", other->client->pers.netname, mc_group5_name.string));
				}
				else
				{
					other->client->sess.mygroup[4] = 'A';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group %s^7(^55^7).\n\"", other->client->pers.netname, mc_group5_name.string));
				}
				break;
			case 6:
				if (isingroup(other,6) == 1)
				{
					other->client->sess.mygroup[5] = '0';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer in group %s^7(^56^7).\n\"", other->client->pers.netname, mc_group6_name.string));
				}
				else
				{
					other->client->sess.mygroup[5] = 'A';
					trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group %s^7(^56^7).\n\"", other->client->pers.netname, mc_group6_name.string));
				}
				break;
			default:
				trap_SendServerCmd( ent-g_entities, va("print \"^3Bad group number. Must be 1 through 6.\n\""));
				break;
		}
		mc_updateaccount(other);
		//trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now in group ^5%i^3.\n\"", other->client->pers.netname, atoi(par2)));
		break;
		case 90:
		//////////////////
		// amallowsentry
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amallowsentry <player>\n\""));
			return;
		}
		if (other->client->sess.sentries > 0)
		{
			other->client->sess.sentries -= 1;
		}
		break;
		case 91:
		//////////////////
		// amsetfakeping
		//////////////////
		if (( Q_stricmp(par2, "") == 0)|| (Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^3/amsetfakeping <player> <min> <max>\n\""));
			return;
		}
		other->client->sess.fakepingmin = atoi(par2);
		other->client->sess.fakepingmax = atoi(par3);
		break;
		case 92:
		//////////////////
		// amsetgripdamage
		//////////////////
		if (( Q_stricmp(par2, "") == 0))
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^7%s^3 has an empowered grip damage of ^5%i^3.\n\"", other->client->pers.netname, other->client->sess.gripdamage));
			return;
		}
		other->client->sess.gripdamage = atoi(par2);
		break;
		case 93:
		//////////////////
		// amsetsaberspeed
		//////////////////
		if (( Q_stricmp(par2, "") == 0))
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^7%s^3 has a saber speed of ^5%f^3.\n\"", other->client->pers.netname, other->client->sess.saberspeed));
			return;
		}
		other->client->sess.saberspeed = atof(par2);
		break;
		case 94:
		//////////////////
		// amsetpowers
		//////////////////
		if (( Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^3/amsetpowers <person> <set> <value>\n\""));
			return;
		}
		switch (atoi(par2))
		{
			case 1:
				other->client->sess.ampowers = atoi(par3);
				break;
			case 2:
				other->client->sess.ampowers2 = atoi(par3);
				break;
			case 3:
				other->client->sess.ampowers3 = atoi(par3);
				break;
			case 4:
				other->client->sess.ampowers4 = atoi(par3);
				break;
			case 5:
				other->client->sess.ampowers5 = atoi(par3);
				break;
			case 6:
				other->client->sess.ampowers6 = atoi(par3);
				break;
			case 7:
				other->client->sess.ampowers7 = atoi(par3);
				break;
			default:
				break;
		}
		mc_updateaccount(other);
		break;
		case 95:
		//////////////////
		// amaddpowers
		//////////////////
		if (( Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^3/amaddpowers <person> <set> <value>\n\""));
			return;
		}
		switch (atoi(par2))
		{
			case 1:
				other->client->sess.ampowers += atoi(par3);
				break;
			case 2:
				other->client->sess.ampowers2 += atoi(par3);
				break;
			case 3:
				other->client->sess.ampowers3 += atoi(par3);
				break;
			case 4:
				other->client->sess.ampowers4 += atoi(par3);
				break;
			case 5:
				other->client->sess.ampowers5 += atoi(par3);
				break;
			case 6:
				other->client->sess.ampowers6 += atoi(par3);
				break;
			case 7:
				other->client->sess.ampowers7 += atoi(par3);
				break;
			default:
				break;
		}
		mc_updateaccount(other);
		break;
		case 96:
		//////////////////
		// amsupergod
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amsupergod <player>\n\""));
			return;
		}
		if (other->client->sess.supergod == 0)
		{
			other->client->sess.supergod = 1;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is now a supergod.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.supergod = 0;
			trap_SendServerCmd( ent-g_entities, va("print \"%s^7 is no longer a supergod.\n\"", other->client->pers.netname));
		}
		break;
		case 97:
		//////////////////
		// amweapondelay
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amweaponspeed <player> <delay in milliseconds, 0 for default>\n\""));
			return;
		}
		if (( Q_stricmp(par2, "") == 0))
		{
			trap_SendServerCmd(ent-g_entities, va("print \"%s^7 has a weapon speed of %i.\n^1/amweapondelay <player> <delay in milliseconds, 0 for default>\n\"", other->client->pers.netname, other->client->sess.mcshootdelay));
			return;
		}
		other->client->sess.mcshootdelay = atoi(par2);
		break;
		case 98:
		//////////////////
		// amforcelogout
		//////////////////
		if (( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^1/amforcelogout <player>\""));
			return;
		}
		if (Q_stricmp(other->client->sess.userlogged,"") == 0)
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^3User is not logged in.\""));
			return;
		}
		mclogout(other);
		trap_SendServerCmd(ent-g_entities, va("print \"^2Logged out.\""));
		break;
		case 99:
		//////////////////
		// amdodge
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amdodge <player>\n\""));
			return;
		}
		if (other->client->sess.dodging == 1)
		{
			other->client->sess.dodging = 0;
			trap_SendServerCmd(ent-g_entities, va("print \"%s^7 stopped dodging.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.dodging = 1;
			trap_SendServerCmd(ent-g_entities, va("print \"%s^7 is now dodging.\n\"", other->client->pers.netname));
		}
		break;
		case 100:
		//////////////////
		// amreflect
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amreflect <player>\n\""));
			return;
		}
		if (other->client->sess.reflect == 1)
		{
			other->client->sess.reflect = 0;
			trap_SendServerCmd(ent-g_entities, va("print \"%s^7 stopped reflecting.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.reflect = 1;
			trap_SendServerCmd(ent-g_entities, va("print \"%s^7 is now reflective.\n\"", other->client->pers.netname));
		}
		break;
		case 101:
		//////////////////
		// amadminslap
		//////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amadminslap <player>\n\""));
			return;
		}
		other->client->sess.slapping = level.time + 1000;
		dsp_doEmote(other, 1059);
		break;
		case 102:
		/////////////////
		// amplaysound2
		/////////////////
		if (( Q_stricmp(par2, "info") == 0)||( Q_stricmp(par2, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^3/amplaysound2 <player> <soundfile>\n\"" ) );
			return;
		}
		//G_Sound( other, CHAN_VOICE, G_SoundIndex(par2) );
		VectorClear(origin);
		VectorCopy(other->client->ps.origin, origin);
		G_SoundAtLoc( origin, CHAN_VOICE, G_SoundIndex(par2) );
		break;
		case 103:
		/////////////////
		// amgrabplayer
		/////////////////
		if (ent->client->sess.grabbedplayer != 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^2Dropped player ^5%i^2.\n\"", ent->client->sess.grabbedplayer-1));
			ent->client->sess.grabbedpdist = 0;
			ent->client->sess.grabbedplayer = 0;
			ent->client->sess.grabbedpoffz = 0;
			return;
		}
		if (Q_stricmp(par2, "") == 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amgrabplayer <player> <distance> [zoff]\n\""));
			return;
		}
		ent->client->sess.grabbedplayer = other->s.number+1;
		ent->client->sess.grabbedpdist = atoi(par2);
		ent->client->sess.grabbedpoffz = atoi(par3);
		trap_SendServerCmd(ent->s.number, va("print \"^2Grabbed player ^5%i^2.\n\"", ent->client->sess.grabbedplayer-1));
		break;
		case 104:
		/////////////////
		// amsetrank
		/////////////////
		if (Q_stricmp(par2, "") == 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amsetrank <player> <rank>\n\""));
			return;
		}
		if (Q_stricmp(other->client->sess.userlogged,"") == 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1Chosen client is not logged in.\n\""));
			return;
		}
		if (atoi(par2) > mc_max_admin_rank.integer)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1Admin rank must be level ^5%i^1 or lower.\n\"", mc_max_admin_rank.integer));
			return;
		}
		if (other->client->sess.adminloggedin >= ent->client->sess.adminloggedin)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1Cannot setrank higher or equal admins.\n\""));
			return;
		}
		other->client->sess.adminloggedin = atoi(par2);
		mc_updateaccount(other);
		trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2's admin set to ^5%i^2.\n\"", other->client->pers.netname, atoi(par2)));
		break;
		case 105:
		/////////////////
		// amtorture
		/////////////////
		if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1,"info") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amtorture <player>\n\""));
			return;
		}
		if (other->client->sess.torture == 1)
		{
			other->client->sess.torture = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^2Torture stopped.\n\""));
		}
		else
		{
			other->client->sess.torture = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^1Torturing ^7%s^1.\n\"", other->client->pers.netname));
		}
		break;
		case 106:
		/////////////////
		// amknockbackonly
		/////////////////
		if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1,"info") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amknockbackonly <player>\n\""));
			return;
		}
		if (other->client->sess.knockbackonly == 0)
		{
			other->client->sess.knockbackonly = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Knockbackonly on.\n\""));
		}
		else
		{
			other->client->sess.knockbackonly = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Knockbackonly off.\n\""));
		}
		break;
		case 107:
		/////////////////
		// ammassgravity
		/////////////////
		if ((Q_stricmp(par2, "") == 0)||(Q_stricmp(par1,"info") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/ammassgravity <player> <new mass gravity>\n\""));
			return;
		}
		trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2's massgravity changed from ^5%i^2 to ^5%i^2.\n\"", other->client->pers.netname, other->client->sess.massgravity, atoi(par2)));
		other->client->sess.massgravity = atoi(par2);
		break;
		case 108:
		/////////////////
		// amknockbackuponly
		/////////////////
		if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1,"info") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amknockbackuponly <player>\n\""));
			return;
		}
		if (other->client->sess.knockbackuponly == 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 now is knocked back upwards only.\n\"", other->client->pers.netname));
			other->client->sess.knockbackuponly = 1;
		}
		else
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 is no longer knocked back upwards only.\n\"", other->client->pers.netname));
			other->client->sess.knockbackuponly = 0;
		}
		break;
		case 109:
		/////////////////
		// amaimbot
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amaimbot <player>\n\""));
			return;
		}
		if (other->client->sess.aimbot == 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 now has an aimbot.\n\"", other->client->pers.netname));
			other->client->sess.aimbot = 1;
		}
		else
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 no longer has an aimbot.\n\"", other->client->pers.netname));
			other->client->sess.aimbot = 0;
		}
		break;
		case 110:
		/////////////////
		// amwatchme
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par2,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amwatchme <player> <player2>\n\""));
			return;
		}
		if (other->client->sess.watching == 0)
		{
			client2num = dsp_adminTarget(ent, par2, clientNum);
			if (client2num == -1)
			{
				trap_SendServerCmd( clientNum, va("print \"^2.: ^7You can not use this command on all^2:.\n\"" ) );
				return;
			}
			if (client2num == -2)
			{
				trap_SendServerCmd( clientNum, va("print \"^2.: ^7No secondary target found^2:.\n\"" ) );
				return;
			}
			if (client2num == -3)
			{
				trap_SendServerCmd( clientNum, va("print \"^2.: ^7No secondary target found^2:.\n\"" ) );
				return;
			}
			client2 = &g_entities[client2num];
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is now watching %s^2.\n\"", other->client->pers.netname, client2->client->pers.netname));
			other->client->sess.watching = client2num+1;
		}
		else
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 is no longer watching anyone.\n\"", other->client->pers.netname));
			other->client->sess.watching = 0;
		}
		break;
		case 111:
		/////////////////
		// amspycamera
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amspycamera <player>\n\""));
			return;
		}
		else
		{
			gentity_t	*newsurf = G_Spawn();
			vec3_t		dir;
			VectorCopy(ent->client->ps.origin, newsurf->s.origin);
			G_SetOrigin(newsurf, newsurf->s.origin);
			VectorClear( newsurf->r.mins );
			VectorClear( newsurf->r.maxs );
			trap_LinkEntity(newsurf);
			newsurf->r.svFlags = SVF_PORTAL;
			newsurf->s.eType = ET_PORTAL;
			newsurf->r.ownerNum = other->s.number;
			newsurf->s.frame = 0;
			newsurf->s.powerups = 0;
			newsurf->s.clientNum = 0;
			VectorCopy(other->client->ps.origin, newsurf->s.origin2);
			//G_SetMovedir(other->client->ps.viewangles, dir);
			dir[PITCH] = other->client->ps.viewangles[PITCH];
			dir[ROLL] = other->client->ps.viewangles[YAW];
			newsurf->s.eventParm = DirToByte(dir);//other->client->ps.viewangles);//dir);
			newsurf->think = spycamthink;
			newsurf->nextthink = level.time + 50;
			newsurf->classname = "mc_spycamera";
		}
		break;
		case 112:
		/////////////////
		// amgivecommand
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par2,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amgivecommand <player> <command>\n\""));
			return;
		}
		else
		{
			int	validcmd;
			int	powerset;
			int	powernum;
			validcmd = 0;
			powerset = 0;
			powernum = 0;
			if (Q_stricmp(par2,"ammakeadmin") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 1;
			}
			else if (Q_stricmp(par2,"amprotect") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 2;
			}
			else if (Q_stricmp(par2,"amcontrol") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"amkick") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"amban") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 16;
			}
			else if (Q_stricmp(par2,"ambanrange") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 32;
			}
			else if (Q_stricmp(par2,"amorigin") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 64;
			}
			else if (Q_stricmp(par2,"amtele") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 128;
			}
			else if (Q_stricmp(par2,"amsentry") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 256;
			}
			else if (Q_stricmp(par2,"amget") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 512;
			}
			else if (Q_stricmp(par2,"amnpcspawn") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 1024;
			}
			else if (Q_stricmp(par2,"amgoto") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"amslap") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 4096;
			}
			else if (Q_stricmp(par2,"amslap2") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"amdoemote") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 16384;
			}
			else if (Q_stricmp(par2,"amplayfx") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 32768;
			}
			else if (Q_stricmp(par2,"amforceteam") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 65536;
			}
			else if (Q_stricmp(par2,"amdualsaber") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 131072;
			}
			else if (Q_stricmp(par2,"amscreenshake") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 262144;
			}
			else if (Q_stricmp(par2,"amgametype") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 524288;
			}
			else if (Q_stricmp(par2,"amremote") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 1048576;
			}
			else if (Q_stricmp(par2,"amfreeze") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 2097152;
			}
			else if ((Q_stricmp(par2,"amadminsleep") == 0)||(Q_stricmp(par2,"amwake") == 0)||(Q_stricmp(par2,"amsleep") == 0))
			{
				validcmd = 1;
				powerset = 1;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"ampunish") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 8388608;
			}
			else if (Q_stricmp(par2,"amsilence") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 16777216;
			}
			else if (Q_stricmp(par2,"amkill") == 0)
			{
				validcmd = 1;
				powerset = 1;
				powernum = 33554432;
			}
			else if (Q_stricmp(par2,"amrename") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 1;
			}
			else if (Q_stricmp(par2,"amcheats") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 2;
			}
			else if (Q_stricmp(par2,"amstatus") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"amwhois") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"amempower") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 16;
			}
			else if (Q_stricmp(par2,"amterminator") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 32;
			}
			else if (Q_stricmp(par2,"amsetspeed") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 64;
			}
			else if (Q_stricmp(par2,"amsetgravity") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 128;
			}
			else if (Q_stricmp(par2,"amgettoother") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 512;
			}
			else if (Q_stricmp(par2,"amswapplayers") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 1024;
			}
			else if (Q_stricmp(par2,"amtelegun") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"ammap_shader") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 4096;
			}
			else if (Q_stricmp(par2,"ammap_placeent") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"ammap_place") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"amteles") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 2;
			}
			else if (Q_stricmp(par2,"ammap_entitylist") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 16384;
			}
			else if (Q_stricmp(par2,"ammap_addmodel") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 32768;
			}
			else if (Q_stricmp(par2,"ammap_addeffect") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 65536;
			}
			else if (Q_stricmp(par2,"ammap_nudge") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"ammap_nudgeent") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"ammap_delent") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 8388608;
			}
			else if (Q_stricmp(par2,"ammap_respawn") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 16777216;
			}
			else if (Q_stricmp(par2,"ammap_setvar") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 33554432;
			}
			else if (Q_stricmp(par2,"amshowmotd") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 1;
			}
			else if (Q_stricmp(par2,"amgod") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"amnoclip") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"amgive") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 16;
			}
			else if (Q_stricmp(par2,"amillusion") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 32;
			}
			else if (Q_stricmp(par2,"amforcegod") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 64;
			}
			else if (Q_stricmp(par2,"amsetprefix") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 128;
			}
			else if (Q_stricmp(par2,"amsetsuffix") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 256;
			}
			else if (Q_stricmp(par2,"amfakechat") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 512;
			}
			else if (Q_stricmp(par2,"amnodrown") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 1024;
			}
			else if (Q_stricmp(par2,"amsolid") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"amresetfiretime") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 4096;
			}
			else if (Q_stricmp(par2,"aminvisible") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"amtimescale") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 16384;
			}
			else if (Q_stricmp(par2,"amfakelag") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 32768;
			}
			else if (Q_stricmp(par2,"amweaponspeed") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 65536;
			}
			/*else if ((Q_stricmp(par2,"poll") == 0)||(Q_stricmp(par2,"callvote poll") == 0)||(Q_stricmp(par2,"callvote_poll") == 0))
			{
				validcmd = 1;
				powerset = 3;
				powernum = 131072;
			}*/
			else if (Q_stricmp(par2,"amdropfakeweapon") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 262144;
			}
			else if (Q_stricmp(par2,"ampsay") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 524288;
			}
			else if ((Q_stricmp(par2,"amsetscore") == 0)||(Q_stricmp(par2,"amaddscore") == 0))
			{
				validcmd = 1;
				powerset = 3;
				powernum = 1048576;
			}
			else if ((Q_stricmp(par2,"amsetcredits") == 0)||(Q_stricmp(par2,"amgivecredits") == 0))
			{
				validcmd = 1;
				powerset = 3;
				powernum = 2097152;
			}
			else if ((Q_stricmp(par2,"amsethealth") == 0)||(Q_stricmp(par2,"amgivehealth") == 0))
			{
				validcmd = 1;
				powerset = 3;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"amannounce") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 8388608;
			}
			else if (Q_stricmp(par2,"ammap_trace") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 16777216;
			}
			else if (Q_stricmp(par2,"ammap_clearedits") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 1;
			}
			else if (Q_stricmp(par2,"ammap_shader2") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"amturretweapon") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"amweaponbounces") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 16;
			}
			else if (Q_stricmp(par2,"amallpowerful") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 32;
			}
			else if (Q_stricmp(par2,"ammap_duplicate") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 64;
			}
			else if (Q_stricmp(par2,"ampassvote") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 128;
			}
			else if (Q_stricmp(par2,"amfailvote") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 256;
			}
			else if (Q_stricmp(par2,"amweaponfirerate") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 512;
			}
			else if (Q_stricmp(par2,"amsupershield") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 1024;
			}
			else if (Q_stricmp(par2,"amsupershield2") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"amfastjet") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 4096;
			}
			else if (Q_stricmp(par2,"amjetspeed") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"amjoingroup") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 16384;
			}
			else if (Q_stricmp(par2,"ammap_place2") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 32768;
			}
			else if (Q_stricmp(par2,"ammap_setangles") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 65536;
			}
			else if (Q_stricmp(par2,"amgetpos") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 131072;
			}
			else if (Q_stricmp(par2,"amsupersentry") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 524288;
			}
			else if (Q_stricmp(par2,"amsupersentry2") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 1048576;
			}
			else if (Q_stricmp(par2,"amdefaultweapons") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 2097152;
			}
			else if (Q_stricmp(par2,"ammap_useent") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"ammap_use") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"ammap_usetarget") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 8388608;
			}
			else if (Q_stricmp(par2,"ammap_nearby") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 16777216;
			}
			else if (Q_stricmp(par2,"amsetfakeping") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 33554432;
			}
			else if (Q_stricmp(par2,"amsetgripdamage") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 1;
			}
			else if (Q_stricmp(par2,"amsetsaberspeed") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 2;
			}
			else if (Q_stricmp(par2,"amplaysound") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"amaddbot") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 32;
			}
			else if (Q_stricmp(par2,"amclearips") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 64;
			}
			else if (Q_stricmp(par2,"amotherweaponsettings") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 128;
			}
			else if ((Q_stricmp(par2,"amaddpowers") == 0)||(Q_stricmp(par2,"amsetpowers") == 0))
			{
				validcmd = 1;
				powerset = 5;
				powernum = 256;
			}
			else if ((Q_stricmp(par2,"ammap_setmins") == 0)||(Q_stricmp(par2,"ammap_setmaxes") == 0))
			{
				validcmd = 1;
				powerset = 5;
				powernum = 512;
			}
			else if (Q_stricmp(par2,"amsupergod") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 1024;
			}
			else if (Q_stricmp(par2,"ammap_move") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"ammap_moveent") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"amweapondelay") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 4096;
			}
			else if (Q_stricmp(par2,"amforcelogout") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"amsetmusic") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 16384;
			}
			else if (Q_stricmp(par2,"aminsta") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 32768;
			}
			else if (Q_stricmp(par2,"amstatus2") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 65536;
			}
			else if (Q_stricmp(par2,"amdodge") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 131072;
			}
			else if (Q_stricmp(par2,"amreflect") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 262144;
			}
			else if (Q_stricmp(par2,"ammap_save") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 524288;
			}
			else if (Q_stricmp(par2,"amadminslap") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 1048576;
			}
			else if (Q_stricmp(par2,"amaimbot") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 2097152;
			}
			else if (Q_stricmp(par2,"amsteve") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 8388608;
			}
			else if (Q_stricmp(par2,"amspycamera") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 16777216;
			}
			else if (Q_stricmp(par2,"amwatchme") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 33554432;
			}
			else if (Q_stricmp(par2,"ammassgravity") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 1;
			}
			else if (Q_stricmp(par2,"amknockbackuponly") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 2;
			}
			else if (Q_stricmp(par2,"ammap_grab") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"ammap_grabent") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 4;
			}
			else if (Q_stricmp(par2,"ammap_group") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"ammap_groupent") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"ammap_addlight") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 16;
			}
			else if (Q_stricmp(par2,"ammap_glow") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 32;
			}
			else if (Q_stricmp(par2,"ammap_groupleader") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 64;
			}
			else if (Q_stricmp(par2,"ammap_addangles") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 128;
			}
			else if (Q_stricmp(par2,"ammap_setorigin") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 256;
			}
			else if (Q_stricmp(par2,"ammap_addglm") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 512;
			}
			else if (Q_stricmp(par2,"ammap_setanim") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 1024;
			}
			else if (Q_stricmp(par2,"ammap_adddoor") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 2048;
			}
			else if (Q_stricmp(par2,"amaccountinfo") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 4096;
			}
			else if (Q_stricmp(par2,"amfury") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 8192;
			}
			else if (Q_stricmp(par2,"amexplode") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 16384;
			}
			else if (Q_stricmp(par2,"amgrabplayer") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 32768;
			}
			else if (Q_stricmp(par2,"ammap_undomove") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 65536;
			}
			else if (Q_stricmp(par2,"ammap_undorot") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 131072;
			}
			else if (Q_stricmp(par2,"amgivecommand") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 262144;
			}
			else if ((Q_stricmp(par2,"amaddnote") == 0)||(Q_stricmp(par2,"amdelnote") == 0))
			{
				validcmd = 1;
				powerset = 6;
				powernum = 524288;
			}
			else if (Q_stricmp(par2,"amreadnote") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 1048576;
			}
			else if (Q_stricmp(par2,"amaddlulglm") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 2097152;
			}
			else if (Q_stricmp(par2,"amsetrank") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 4194304;
			}
			else if (Q_stricmp(par2,"amtorture") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 8388608;
			}
			else if (Q_stricmp(par2,"amknockbackonly") == 0)
			{
				validcmd = 1;
				powerset = 6;
				powernum = 16777216;
			}
			else if ((Q_stricmp(par2,"amaddtsent") == 0)||(Q_stricmp(par2,"ammap_addtsent") == 0))
			{
				validcmd = 1;
				powerset = 6;
				powernum = 33554432;
			}
			else if (Q_stricmp(par2,"amvote") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 256;
			}
			else if (Q_stricmp(par2,"amrget") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 131072;
			}
			else if (Q_stricmp(par2,"amrgoto") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 262144;
			}
			else if (Q_stricmp(par2,"amspecweapon") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 524288;
			}
			else if (Q_stricmp(par2,"amdontforceme") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 1048576;
			}
			else if (Q_stricmp(par2,"amfly") == 0)
			{
				validcmd = 1;
				powerset = 2;
				powernum = 2097152;
			}
			else if (Q_stricmp(par2,"amhome") == 0)
			{
				validcmd = 1;
				powerset = 4;
				powernum = 262144;
			}
			else if (Q_stricmp(par2,"amstealth") == 0)
			{
				validcmd = 1;
				powerset = 3;
				powernum = 33554432;
			}
			else if (Q_stricmp(par2,"amviewrandom") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 8;
			}
			else if (Q_stricmp(par2,"amabuse") == 0)
			{
				validcmd = 1;
				powerset = 5;
				powernum = 16;
			}
			else if ((Q_stricmp(par2,"ammonitorclientcommands") == 0)||(Q_stricmp(par2,"ammonitorservercommands") == 0))
			{
				validcmd = 1;
				powerset = 5;
				powernum = 4194304;
			}
			else if ((Q_stricmp(par2,"ammonitorchannels") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 1;
			}
			else if ((Q_stricmp(par2,"ammodel") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 2;
			}
			else if ((Q_stricmp(par2,"amsudo") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 4;
			}
			else if ((Q_stricmp(par2,"ammap_delent2") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 8;
			}
			else if ((Q_stricmp(par2,"callvote") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 16;
			}
			else if ((Q_stricmp(par2,"amteles") == 0)||(Q_stricmp(par2,"ammap_teles") == 0))
			{
				validcmd = 1;
				powerset = 4;
				powernum = 2;
			}
			else if ((Q_stricmp(par2,"bypass-notele") == 0)||(Q_stricmp(par2,"bypass_notele") == 0)||(Q_stricmp(par2,"noteleport") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 32;
			}
			else if ((Q_stricmp(par2,"ambans") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 64;
			}
			else if ((Q_stricmp(par2,"amshock") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 128;
			}
			else if ((Q_stricmp(par2,"realentnum") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 256;
			}
			else if ((Q_stricmp(par2,"allgroup") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 512;
			}
			else if ((Q_stricmp(par2,"amsetweapon") == 0))
			{
				validcmd = 1;
				powerset = 7;
				powernum = 1024;
			}
			else
			{
				trap_SendServerCmd(ent->s.number, va("print \"^1Unknown command ~^5%s^1~.\n\"", par2));
				return;
			}
			if (powerset == 1)
			{
				if (other->client->sess.ampowers & powernum)
				{
					other->client->sess.ampowers -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else if (powerset == 2)
			{
				if (other->client->sess.ampowers2 & powernum)
				{
					other->client->sess.ampowers2 -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers2 += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else if (powerset == 3)
			{
				if (other->client->sess.ampowers3 & powernum)
				{
					other->client->sess.ampowers3 -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers3 += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else if (powerset == 4)
			{
				if (other->client->sess.ampowers4 & powernum)
				{
					other->client->sess.ampowers4 -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers4 += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else if (powerset == 5)
			{
				if (other->client->sess.ampowers5 & powernum)
				{
					other->client->sess.ampowers5 -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers5 += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else if (powerset == 6)
			{
				if (other->client->sess.ampowers6 & powernum)
				{
					other->client->sess.ampowers6 -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers6 += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else if (powerset == 7)
			{
				if (other->client->sess.ampowers7 & powernum)
				{
					other->client->sess.ampowers7 -= powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^1Removed command ~^5%s^1~ from ^7%s^1.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
				else
				{
					other->client->sess.ampowers7 += powernum;
					trap_SendServerCmd(ent->s.number, va("print \"^2Gave command ~^5%s^2~ to ^7%s^2.\n\"", par2, other->client->pers.netname));
					mc_updateaccount(other);
					return;
				}
			}
			else
			{
				trap_SendServerCmd(ent->s.number, va("print \"^1Powerset ^5%i^1 does not exist.\n\"", powerset));
			}
		}
		break;
		case 113:
		/////////////////
		// amfury
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par2,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amfury <player> <amount>\n\""));
			return;
		}
		else
		{
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2's fury level changed from ^5%f^2 to ^5%f^2.\n\"", other->client->pers.netname, other->client->sess.damagemod+1,atof(par2)));
			other->client->sess.damagemod = atof(par2)-1;
		}
		break;
		case 114:
		/////////////////
		// amsteve
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amsteve <player>\n\""));
			return;
		}
		else
		{
			if (other->client->sess.steve == 1)
			{
				other->client->sess.steve = 0;
				trap_SendServerCmd(ent->s.number, va("print \"^7%s^1 is no longer Steve.\n\"", other->client->pers.netname));
				return;
			}
			else
			{
				other->client->sess.steve = 1;
				trap_SendServerCmd(ent->s.number, va("print \"^7%s^2 is now Steve.\n\"", other->client->pers.netname));
				return;
			}
		}
		break;
		case 116:
		/////////////////
		// amcontrol
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amcontrol <player>\n\""));
			return;
		}
		else
		{
			if (other->client->sess.controller <= 0)
			{
				other->client->sess.controller = ent->s.number+1;
				trap_SendServerCmd(ent->s.number, va("print \"^7%s^1 is now under your control.\n\"", other->client->pers.netname));
				return;
			}
			else
			{
				other->client->sess.controller = 0;
				trap_SendServerCmd(ent->s.number, va("print \"^7%s^2 is no longer being controlled.\n\"", other->client->pers.netname));
				return;
			}
		}
		break;
		case 117:
		/////////////////
		// amignore
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amignore <player>\n\""));
			return;
		}
		if (ent->client->sess.ignoring[other->s.number] == 'A')
		{
			ent->client->sess.ignoring[other->s.number] = '0';
			trap_SendServerCmd(ent->s.number, va("print \"^2No longer ignoring ^7%s^2.\n\"", other->client->pers.netname));
		}
		else
		{
			ent->client->sess.ignoring[other->s.number] = 'A';
				trap_SendServerCmd(ent->s.number, va("print \"^1Ignoring ^7%s^1.\n\"", other->client->pers.netname));
		}
		break;
		case 118:
		/////////////////
		// amrget
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amrget <player>\n\""));
			return;
		}
		other->client->sess.pendingtype = 1;
		other->client->sess.pendingvalue = ent->s.number;
		other->client->sess.pendingtimeout = level.time + mc_request_timeout.integer*1000;
		trap_SendServerCmd(other->s.number, va("print \"^7%s^2 wants to teleport you to them. ^5/amaccept^2 to accept, ^5/amreject^2 to reject. You have ^5%i^2 seconds before automatic rejection.\n\"", ent->client->pers.netname, mc_request_timeout.integer));
		trap_SendServerCmd(ent->s.number, va("print \"^2Sent request to ^7%s^2.\n\"", other->client->pers.netname));
		break;
		case 119:
		/////////////////
		// amrgoto
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amrgoto <player>\n\""));
			return;
		}
		other->client->sess.pendingtype = 2;
		other->client->sess.pendingvalue = ent->s.number;
		other->client->sess.pendingtimeout = level.time + mc_request_timeout.integer*1000;
		trap_SendServerCmd(other->s.number, va("print \"^7%s^2 wants to teleport to you. ^5/amaccept^2 to accept, ^5/amreject^2 to reject. You have ^5%i^2 seconds before automatic rejection.\n\"", ent->client->pers.netname, mc_request_timeout.integer));
		trap_SendServerCmd(ent->s.number, va("print \"^2Sent request to ^7%s^2.\n\"", other->client->pers.netname));
		break;
		case 120:
		/////////////////
		// amspecweapon
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par2,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amspecweapon <player> <weapon#>\n\""));
			return;
		}
		other->client->sess.specweapon = atoi(par2);
		trap_SendServerCmd(ent->s.number, va("print \"^2Special weapon set.\n\""));
		break;
		case 121:
		/////////////////
		// amdontForceme
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amdontForceme <player>\n\""));
			return;
		}
		if (other->client->sess.noforceme == 0)
		{
			other->client->sess.noforceme = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 cannot be Force'd.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.noforceme = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 can now be Force'd.\n\"", other->client->pers.netname));
		}
		break;
		case 122:
		/////////////////
		// amfly
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amfly <player>\n\""));
			return;
		}
		if (other->client->sess.flying == 0)
		{
			other->client->sess.flying = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is now flying.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.flying = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 is no longer flying.\n\"", other->client->pers.netname));
		}
		break;
		case 123:
		/////////////////
		// amhome
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amhome <player>\n\""));
			return;
		}
		if (qtrue)
		{
			gentity_t	*missile;
			vec3_t		forward;
			AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
			missile = CreateMissile( ent->client->ps.origin, forward, mc_rocket_vel.integer*0.5, mc_weapons_life.integer, ent, qtrue );
			missile->s.eFlags |= EF_BOUNCE;
			missile->enemy = other;
			missile->think = rocketThink;
			missile->nextthink = level.time + mc_rocket_alt_think.integer;
			missile->classname = "rocket_proj";
			missile->s.weapon = WP_ROCKET_LAUNCHER;
			VectorSet( missile->r.maxs, 3, 3, 3 );
			VectorScale( missile->r.maxs, -1, missile->r.mins );
			missile->damage = mc_rocket_damage.integer;
			missile->dflags = DAMAGE_DEATH_KNOCKBACK;
			missile->methodOfDeath = MOD_ROCKET_HOMING;
			missile->splashMethodOfDeath = MOD_ROCKET_HOMING_SPLASH;
			missile->clipmask = MASK_SHOT;// | CONTENTS_LIGHTSABER;
			missile->splashDamage = mc_rocket_splash_damage.integer;
			missile->splashRadius = mc_rocket_splash_radius.integer;
			missile->bounceCount = mc_rocket_bounces.integer;
		}
		break;
		case 124:
		/////////////////
		// amstealth
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amstealth <player>\n\""));
			return;
		}
		if (other->client->sess.stealth == 0)
		{
			other->client->sess.stealth = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is now stealth.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.stealth = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 is no longer stealth.\n\"", other->client->pers.netname));
		}
		break;
		case 125:
		/////////////////
		// amabuse
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amabuse <player>\n\""));
			return;
		}
		if (other->client->sess.abused == 0)
		{
			other->client->sess.abused = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is now being abused.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.abused = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 is no longer being abused.\n\"", other->client->pers.netname));
		}
		break;
		case 126:
		/////////////////
		// amviewrandom
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amviewrandom <player>\n\""));
			return;
		}
		if (other->client->sess.viewrandom == 0)
		{
			other->client->sess.viewrandom = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 now has a randomizing view.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.viewrandom = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 no longer has a randomizing view.\n\"", other->client->pers.netname));
		}
		break;
		case 127:
		/////////////////
		// ammodel
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par1,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/ammodel <player> <model>\n\""));
			return;
		}
		else
		{
		char	userinfo[MAX_INFO_STRING];
		trap_GetUserinfo( other->s.number, userinfo, sizeof( userinfo ) );
		if (Q_stricmp(par2, "") == 0)
		{
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is using the model ~^5%s^2~.\n\"", other->client->pers.netname, Info_ValueForKey (userinfo, "model")));
		}
		else
		{
			Info_SetValueForKey( userinfo, "model", par2 );
			trap_SetUserinfo( other->s.number, userinfo );
			ClientUserinfoChanged(other->s.number);
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is now using the model ~^5%s^2~.\n\"", other->client->pers.netname, par2));
		}
		}
		break;
		case 128:
		/////////////////
		// ammonitorchannels
		/////////////////
		if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/ammonitorchannels <player>\n\""));
			return;
		}
		if (other->client->sess.monchan == 0)
		{
			other->client->sess.monchan = 1;
			trap_SendServerCmd(ent->s.number, va("print \"^2Player ^7%s^2 is now monitoring channels.\n\"", other->client->pers.netname));
		}
		else
		{
			other->client->sess.monchan = 0;
			trap_SendServerCmd(ent->s.number, va("print \"^1Player ^7%s^1 is no longer monitoring channels.\n\"", other->client->pers.netname));
		}
		break;
		case 129:
		/////////////////
		// amshock
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par2,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amshock <player> <time in seconds>\n\""));
			return;
		}
		else
		{
			float	shocktime;
			shocktime = atof(par2);
			if (shocktime > 9001)
			{
				shocktime = 9001;
			}
			shocktime *= 1000;
			other->client->ps.electrifyTime = level.time + (int)shocktime;
			trap_SendServerCmd(ent->s.number, va("print \"^2Shocking ^7%s^2.\n\"", other->client->pers.netname));
		}
		break;
		case 130:
		/////////////////
		// amsetweapon
		/////////////////
		if ((Q_stricmp(par1,"info") == 0)||(Q_stricmp(par2,"") == 0))
		{
			trap_SendServerCmd(ent->s.number, va("print \"^1/amsetweapon <player> <weapon>\n\""));
			return;
		}
		else
		{
			if (Q_stricmp(par2,"saberon") == 0)
			{
				other->client->ps.saberHolstered = qfalse;
			}
			if (Q_stricmp(par2,"saberoff") == 0)
			{
				other->client->ps.saberHolstered = qtrue;
			}
			other->client->ps.weapon = weapforname(par2);
			//other->client->ps.weaponstate = WEAPON_READY;
			other->client->ps.stats[STAT_WEAPONS] |= ( 1 << other->client->ps.weapon );
			other->client->ps.weaponTime = 400;
		}
		break;
	}
}

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "../../ui/menudef.h"			// for the voice chats

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
int allaktiv = 36;
//end rww
void dsp_doEmote(gentity_t *ent, int cmd);
void dsp_stringEscape(char *in, char *out, int outSize);
void BG_CycleInven(playerState_t *ps, int direction);
void BG_CycleForce(playerState_t *ps, int direction);
void dspSanitizeString( char *in, char *out ) ;
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

int monkeyspot(char *spot, int xyorz, gentity_t *playerent, gentity_t *otherent);


void mcm_amadmin(gentity_t *ent, int clientNum, char *par1, int iPowers, int iPowers2, int iPowers3, int iPowers4, int iPowers5, int iPowers6, int iPowers7);

gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
extern void SP_jakes_model( gentity_t *ent );
extern void SP_jakes_model_zocken( gentity_t *ent );

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

extern ipFilter_t	ipFilters[MAX_IPFILTERS];
extern int			numIPFilters;
void trap_SendServerCmd( int clientNum, const char *text )
{
	if (clientNum >= -1 && clientNum <= 31)
	{
		trap_SendServerCommand( clientNum, text );
	}
	else
	{
		G_Printf("Sent message to noplayer: %s", text);
	}
}
// __________________________________________________________________


char *mc_lower (char *orig)
{
	int	iml;
	int	mllen;
	mllen = strlen(orig);
	for (iml = 0;iml < mllen;iml++)
	{
		if (orig[iml] >= 'A' && orig[iml] <= 'Z')
		{
			orig[iml] += ('a' - 'A');
		}
	}
	return orig;
}


void reForceThrow(gentity_t *self, qboolean pull, int powermod);
void mcm_shout(gentity_t *ent, char *cmd_a2)
{
	int	powermod = 4;
	if (Q_stricmp(cmd_a2,"") != 0)
	{
		powermod = atoi(cmd_a2);
	}
	reForceThrow(ent, qfalse, powermod);
}

void mc_clearedits(gentity_t *ent)
{
	vmCvar_t	mapname;
	int		i;
	fileHandle_t	f;
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	for (i = 1;i < 32;i += 1)
	{
		trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_READ);
		if (f)
		{
			trap_FS_FCloseFile(f);
			trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_WRITE);
			if ( !f )
			{
				trap_SendServerCmd( ent->s.number, va("print \"^1Error: Cannot access map file.\n\"" ) );
			}
			trap_FS_Write( va("\n"), strlen(va("\n")), f);
			trap_FS_FCloseFile( f );
		}
	}
	trap_SendServerCmd(ent->fixednum, va("print \"^2Edits cleared.\n\""));
}

void mc_lockserver(gentity_t *ent, char *password, char *message)
{
	if (g_needpass.integer == 1)
	{
		trap_SendConsoleCommand( EXEC_APPEND, va("g_password \"\";g_needpass 0;\n" ) );
		trap_SendServerCmd(ent->fixednum, va("print \"^2Server unlocked.\n\""));
		return;
	}
	if ((Q_stricmp(password,"") == 0)||(Q_stricmp(password,"info") == 0))
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1/amlockserver <password> [message]\n\""));
		return;
	}
	if (strchr(password,';')||strchr(password,'\n')||strchr(message,';')||strchr(message,'\n'))
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You should probably be banned for that.\n\""));
		return;
	}
	trap_SendConsoleCommand( EXEC_APPEND, va("g_password \"%s\";g_needpass 1;mc_lockmessage \"%s\";\n", password, message ) );
	trap_SendServerCmd(ent->fixednum, va("print \"^1Server locked.\n\""));
	return;
}


void mc_amlightning(gentity_t *ent, char *cmd_a2, char *cmd_a3, char *cmd_a4)
{
	int i = 0;
	vec3_t angles2;
	vec3_t origin2;
	if (Q_stricmp(cmd_a4,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1/amlightning <X> <Y> <Z>\n\""));
		return;
	}
	origin2[0] = monkeyspot(cmd_a2, 0, ent, ent);
	origin2[1] = monkeyspot(cmd_a3, 1, ent, ent);
	origin2[2] = monkeyspot(cmd_a4, 2, ent, ent);
	for (i = 0;i < 5;i += 1)
	{
		VectorClear(angles2);
		angles2[YAW] = irand(0, 360);
		angles2[PITCH] = -90 + irand(-30, 30);
		angles2[ROLL] = irand(0, 360);
		G_PlayEffect_ID(G_EffectIndex( "lightning" ), origin2, angles2);
	}
	G_RadiusDamage( origin2, ent, 200, 100, NULL, MOD_REPEATER_ALT_SPLASH );
	G_SoundAtLoc( origin2, CHAN_VOICE, G_SoundIndex("sound/effects/energy_crackle") );
}


void mc_amdistance(gentity_t *ent, char *cmd_a2, char *cmd_a3, char *cmd_a4, char *cmd_a5, char *cmd_a6, char *cmd_a7)
{
	int x1 = monkeyspot(cmd_a2, 0, ent, ent);
	int x2 = monkeyspot(cmd_a5, 0, ent, ent);
	int y1 = monkeyspot(cmd_a3, 1, ent, ent);
	int y2 = monkeyspot(cmd_a6, 1, ent, ent);
	int z1 = monkeyspot(cmd_a4, 2, ent, ent);
	int z2 = monkeyspot(cmd_a7, 2, ent, ent);
	trap_SendServerCmd(ent->fixednum, va("print \"^7From (^5%i^7,^5%i^7,^5%i^7) to (^5%i^7,^5%i^7,^5%i^7)\n\"", x1, y1, z1, x2, y2, z2));
	trap_SendServerCmd(ent->fixednum, va("print \"^7X distance: ^5%i^7. Y distance: ^5%i^7. Z distance: ^5%i^7.\n\"", x1 - x2, y1 - y2, z1 - z2));
	trap_SendServerCmd(ent->fixednum, va("print \"^7Flat (XY) distance: ^5%i^7. Total (XYZ) distance: ^5%i^7.\n\"", (int)sqrt((double) ((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)) ), (int)sqrt((double) ((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2))+((z1-z2)*(z1-z2)) )   ));
}








void mcbansystem(gentity_t *ent, char *cmd_a2, char *cmd_a3, char *cmd_a4, char *cmd_a5, char *cmd_a6)
{
	int		i;
	gban_t		*ban;
	if ((Q_stricmp(cmd_a2,"") == 0)||(Q_stricmp(cmd_a2,"info") == 0))
	{
		goto failed;
	}
	if (Q_stricmp(cmd_a2, "test") == 0)
	{
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		if (ip_is_banned(cmd_a3) == 1)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1IP is banned.\n\""));
		}
		else
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^2IP is not banned.\n\""));
		}
		return;
	}
	else if (Q_stricmp(cmd_a2, "list") == 0)
	{
		int	found;
		found = 0;
		for (i = 0;i < 510;i += 1)
		{
			ban = &g_bans[i];
			if (ban->active == 1)
			{
				found += 1;
				trap_SendServerCmd(ent->fixednum, va("print \"^5%i^2)^1%i^7.^1%i^7.^1%i^7.^1%i\n\"", i, ban->ip1, ban->ip2, ban->ip3, ban->ip4));
			}
		}
		if (found == 0)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1No bans found.\n\""));
		}
		return;
	}
	else if (Q_stricmp(cmd_a2, "add") == 0)
	{
		if (Q_stricmp(cmd_a6, "") == 0)
		{
			goto failed;
		}
		bans_add(atoi(cmd_a3), atoi(cmd_a4), atoi(cmd_a5), atoi(cmd_a6));
		bans_write();
		trap_SendServerCmd(ent->fixednum, va("print \"^1IP banned.\n\""));
		return;
	}
	else if (Q_stricmp(cmd_a2, "remove") == 0)
	{
		if (Q_stricmp(cmd_a6, "") == 0)
		{
			goto failed;
		}
		for (i = 0;i < 510;i += 1)
		{
			ban = &g_bans[i];
			if (ban && (ban->active == 1))
			{
				if (ban->ip1 == atoi(cmd_a3))
				{
					if (ban->ip2 == atoi(cmd_a4))
					{
						if (ban->ip3 == atoi(cmd_a5))
						{
							if (ban->ip4 == atoi(cmd_a6))
							{
								ban->active = 0;
								trap_SendServerCmd(ent->fixednum, va("print \"^2IP removed.\n\""));
								bans_write();
								return;
							}
						}
					}
				}
			}
		}
		trap_SendServerCmd(ent->fixednum, va("print \"^1IP not found.\n\""));
		return;
	}
	failed:
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ambans test <ip>                      ^2Test if an IP is banned.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ambans list                           ^2See a list of all banned IPs.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ambans remove <ip1> <ip2> <ip3> <ip4> ^2Remove a banned IP.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ambans add <ip1> <ip2> <ip3> <ip4>    ^2Ban a new IP. Use -1 as a wildcard.\n\""));
}

void amchannels(gentity_t *ent, char *cmd_a2, char *cmd_a3, char *cmd_a4)
{
	int		ch;
	gchannel_t	*chan;
	if ((Q_stricmp(cmd_a2,"") == 0)||(Q_stricmp(cmd_a2,"info") == 0))
	{
		goto failed;
	}
	if ((Q_stricmp(cmd_a2, "say") == 0)||(Q_stricmp(cmd_a2, "join") == 0)||(Q_stricmp(cmd_a2, "makeadmin") == 0)||(Q_stricmp(cmd_a2, "leave") == 0)||
		(Q_stricmp(cmd_a2, "list") == 0)||(Q_stricmp(cmd_a2, "ban") == 0)||(Q_stricmp(cmd_a2, "close") == 0))
	{
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		ch = channels_find(cmd_a3);
		if (ch == -1)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown channel.\n\""));
			return;
		}
		chan = &g_channels[ch];
		if ((chan->inchannel[ent->s.number] == '0') && (Q_stricmp(cmd_a2, "join") != 0))
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1You are not in this channel.\n\""));
			return;
		}
	}
	if (Q_stricmp(cmd_a2, "say") == 0)
	{
		char	*p = ConcatArgs(3);
		if (Q_stricmp(cmd_a4, "") == 0)
		{
			goto failed;
		}
		channels_print(ch, va("^7%s^7: ^5%s", ent->client->pers.netname, p));
		return;
	}
	else if (Q_stricmp(cmd_a2, "create") == 0)
	{
		if (Q_stricmp(cmd_a4, "") == 0)
		{
			goto failed;
		}
		ch = channels_create(cmd_a3, cmd_a4);
		if (ch == -1)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Too many active channels.\n\""));
			return;
		}
		if (ch == -2)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Channel already exists.\n\""));
			return;
		}
		chan = &g_channels[ch];
		trap_SendServerCmd(ent->fixednum, va("print \"^2Channel created.\n\""));
		chan->inchannel[ent->s.number] = 'A';
		chan->isadmin[ent->s.number] = 'A';
		return;
	}
	else if (Q_stricmp(cmd_a2, "join") == 0)
	{
		if (Q_stricmp(cmd_a4, "") == 0)
		{
			goto failed;
		}
		if (Q_stricmp(cmd_a4, chan->password) != 0)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Wrong password.\n\""));
			return;
		}
		if (chan->isbanned[ent->s.number] == 'A')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1You are banned from this channel.\n\""));
			return;
		}
		if (chan->inchannel[ent->s.number] == 'A')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1You are already in this channel.\n\""));
			return;
		}
		chan->inchannel[ent->s.number] = 'A';
		channels_joined(ch, ent->s.number);
		return;
	}
	else if (Q_stricmp(cmd_a2, "leave") == 0)
	{
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		channels_remove(ch, ent->s.number, 0);
		trap_SendServerCmd(ent->fixednum, va("print \"^1Left the channel.\n\""));
		return;
	}
	else if (Q_stricmp(cmd_a2, "list") == 0)
	{
		int	i;
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		for (i = 0;i < 32;i += 1)
		{
			if (chan->inchannel[i] == 'A')
			{
				if (chan->isadmin[i] == 'A')
				{
					trap_SendServerCmd(ent->fixednum, va("print \"^1[ADMIN]^5%i^2)^7%s\n\"", i, g_entities[i].client->pers.netname));
				}
				else
				{
					trap_SendServerCmd(ent->fixednum, va("print \"^5%i^2)^7%s\n\"", i, g_entities[i].client->pers.netname));
				}
			}
		}
		return;
	}
	else if (Q_stricmp(cmd_a2, "ban") == 0)
	{
		int	target;
		if (Q_stricmp(cmd_a4, "") == 0)
		{
			goto failed;
		}
		if (chan->isadmin[ent->s.number] == '0')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1You are not a channel admin.\n\""));
			return;
		}
		target = dsp_adminTarget(ent, cmd_a4, ent->s.number);
		if (target < 0)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown player.\n\""));
			return;
		}
		if (chan->inchannel[target] == '0')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Player ^7%s ^1is not in the channel.\n\"", g_entities[target].client->pers.netname));
			return;
		}
		if (chan->isadmin[target] == 'A')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Cannot ban a channel admin.\n\""));
			return;
		}
		channels_print(ch, va("^7%s^5 was banned from the channel.", g_entities[target].client->pers.netname));
		chan->inchannel[target] = '0';
		chan->isbanned[target] = 'A';
		return;
	}
	else if (Q_stricmp(cmd_a2, "makeadmin") == 0)
	{
		int	target;
		if (Q_stricmp(cmd_a4, "") == 0)
		{
			goto failed;
		}
		if (chan->isadmin[ent->s.number] == '0')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1You are not a channel admin.\n\""));
			return;
		}
		target = dsp_adminTarget(ent, cmd_a4, ent->s.number);
		if (target < 0)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown player.\n\""));
			return;
		}
		if (chan->inchannel[target] == '0')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Player ^7%s ^1is not in the channel.\n\"", g_entities[target].client->pers.netname));
			return;
		}
		if (chan->isadmin[target] == 'A')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Player ^7%s^1 is already a channel admin.\n\"", g_entities[target].client->pers.netname));
			return;
		}
		channels_print(ch, va("^7%s^5 is now a channel admin.", g_entities[target].client->pers.netname));
		chan->isadmin[target] = 'A';
		return;
	}
	else if (Q_stricmp(cmd_a2, "close") == 0)
	{
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		if (chan->isadmin[ent->s.number] == '0')
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1You are not a channel admin.\n\""));
			return;
		}
		channels_delete(ch);
		return;
	}
	failed:
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels say <channel> <message>     ^2Say something in the chosen channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amteamchat <channel>                   ^2Lock teamchat to speaking in the chosen channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels create <channel> <password> ^2Create a new channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels join <channel> <password>   ^2Join the chosen channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels leave <channel>             ^2Leave the chosen channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels list <channel>              ^2See who is in the channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels ban <channel> <player>      ^2If you are a channel admin, ban the player from the chosen channel.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels makeadmin <channel> <player>^2If you are a channel admin, make the player a channel admin.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchannels close <channel>             ^2If you are a channel admin, close the chosen channel.\n\""));
}
extern void mctsent_think(gentity_t *ent);
int send_mail(gentity_t *ent, char *account, char *message);

void mailsys(gentity_t *ent, char *cmd_a2, char *cmd_a3)
{
	if ((Q_stricmp(cmd_a2,"") == 0)||(Q_stricmp(cmd_a2,"info") == 0))
	{
		goto failed;
	}
	if (Q_stricmp(ent->client->sess.userlogged,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^2You are not logged in.\n\""));
		return;
	}
	if (Q_stricmp(cmd_a2,"clear") == 0)
	{
		fileHandle_t		Mz;
		trap_FS_FOpenFile(va("%s/mail_1_%s.cfg", mc_userfolder.string, ent->client->sess.userlogged), &Mz, FS_WRITE);
		trap_FS_Write( " ", strlen(" "), Mz);
		trap_FS_FCloseFile( Mz );
		trap_SendServerCmd(ent->fixednum, va("print \"^2Mail cleared.\n\""));
		return;
	}
	if (Q_stricmp(cmd_a2,"send") == 0)
	{
		char	*message;
		int	result;
		message = ConcatArgs( 3 );
		if (Q_stricmp(message,"") == 0)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1/ammail send <account> <message> ^2Sends new mail to chosen account.\n\""));
			return;
		}
		result = send_mail(ent, cmd_a3, message);
		switch (result)
		{
			case 1:
				trap_SendServerCmd(ent->fixednum, va("print \"^2Sent.\n\""));
				break;
			case -1:
				trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown account.\n\""));
				break;
			case -2:
				trap_SendServerCmd(ent->fixednum, va("print \"^1Target inbox full.\n\""));
				break;
			default:
				trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown error.\n\""));
				break;
		}
		return;
	}
	if (Q_stricmp(cmd_a2,"read") == 0)
	{
		char		buffer[4096];
		char		messageline[4096];
		fileHandle_t	f;
		int		i;
		int		iSp;
		int		iC;
		int		len;
		iSp = 0;
		stringclear(buffer, 4090);
		stringclear(messageline,4090);
		len = trap_FS_FOpenFile(va("%s/mail_1_%s.cfg", mc_userfolder.string, ent->client->sess.userlogged), &f, FS_READ);
		buffer[len] = '';
		trap_FS_Read( buffer, 4090, f );
		if (buffer[0] == ' ')
		{
			iSp = 1;
		}
		iC = 0;
		G_Printf("%i %s", len, buffer);
		for (i = 0;i < len;i += 1)
		{
			/*if (buffer[i] = '')
			{
				return;
			}*/
			if (buffer[i] == '\n')
			{
				iC += 1;
				if (iC == 1)
				{
					memcpy (messageline, buffer+iSp, i-iSp);
				}
				else
				{
					memcpy (messageline, buffer+iSp+1, i-iSp-1);
				}
				G_Printf("^5%i^7)%s\n", iC, messageline);
				trap_SendServerCmd(ent->fixednum, va("print \"^5%i^7)%s\n\"", iC, messageline));
				stringclear(messageline,4090);
				iSp = i;
			}
		}
		if (iC == 0)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^2Inbox empty.\n\""));
		}
		if (len > 2048)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^2Inbox full.\n\""));
		}
		return;
	}
	failed:
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ammail send <account> <message> ^2Sends new mail to chosen account.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ammail clear                    ^2Deletes all your mail.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/ammail read                     ^2Read all your mail.\n\""));
	//trap_SendServerCmd(ent->fixednum, va("print \"^1/ammail delete <number>          ^2Deletes specific message.\n\""));
	return;
}
int send_mail(gentity_t *ent, char *account, char *message)
{
	char			*Mfullwrite;
	int			len;
	fileHandle_t		Mz;
	int			i;
	len = trap_FS_FOpenFile(va("%s/account_1_%s.cfg", mc_userfolder.string, account), &Mz, FS_READ);
	if ( !Mz )
	{
		return -1; // Bad account
	}
	trap_FS_FCloseFile( Mz );
	len = trap_FS_FOpenFile(va("%s/mail_1_%s.cfg", mc_userfolder.string, account), &Mz, FS_APPEND);
	if (len > 2048)
	{
		return -2; // Inbox full
	}
	Mfullwrite = va("%s^7: %s\n", ent->client->sess.userlogged, message);
	trap_FS_Write( Mfullwrite, strlen(Mfullwrite), Mz);
	trap_FS_FCloseFile( Mz );
	for (i = 0;i < 32;i += 1)
	{
		gentity_t	*pl = &g_entities[i];
		if (pl && pl->client && pl->inuse)
		{
			if (Q_stricmp(pl->client->sess.userlogged, account) == 0)
			{
				trap_SendServerCmd(i, va("print \"^2You have mail!\n^2Type /ammail in your console to view it.\n\""));
			}
		}
	}
	return 1; // All went well
}


void cmd_teles(gentity_t *ent, char *cmd_a2, char *cmd_a3, char *cmd_a4, char *cmd_a5, char *cmd_a6, char *cmd_a7, char *cmd_a8, char *cmd_a9, char *cmd_a10)
{
	if ((Q_stricmp(cmd_a2,"") == 0)||(Q_stricmp(cmd_a2,"info") == 0))
	{
		goto failed;
	}
	if (Q_stricmp(cmd_a2,"add") == 0)
	{
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		else
		{
			vec3_t	pos;
			int	angle;
			VectorClear(pos);
			pos[0] = monkeyspot(cmd_a7, 0, ent, ent);
			pos[1] = monkeyspot(cmd_a8, 1, ent, ent);
			pos[2] = monkeyspot(cmd_a9, 2, ent, ent);
			if ((Q_stricmp(cmd_a10,"") == 0)||(Q_stricmp(cmd_a10,"*") == 0))
			{
				angle = ent->client->ps.viewangles[YAW];
			}
			else
			{
				angle = atoi(cmd_a10);
			}
			if (strchr(cmd_a3,';') || strchr(cmd_a4, ';') || strchr(cmd_a3,'\n') || strchr(cmd_a4, '\n'))
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^1Illegal chatcommand - cannot contain a ;semicolon;.\n\""));
				return;
			}
			teleporter_add(pos, cmd_a3, angle, atoi(cmd_a5), atoi(cmd_a6), cmd_a4);
			trap_SendServerCmd(ent->fixednum, va("print \"^2Added chatcommand.\n\""));
			return;
		}
	}
	else if (Q_stricmp(cmd_a2,"delete") == 0)
	{
		int i;
		if (Q_stricmp(cmd_a3, "") == 0)
		{
			goto failed;
		}
		for (i = 0;i < 512;i += 1)
		{
			gteleporter_t	*tele = &g_teleporters[i];
			if (tele && (tele->active == 1))
			{
				if (Q_stricmp(tele->name, cmd_a3) == 0)
				{
					trap_SendServerCmd(ent->fixednum, va("print \"^1Deleted chatcommand.\n\""));
					teleporter_delete(i);
					return;
				}
			}
		}
		trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown chatcommand.\n\""));
		return;
	}
	else if (Q_stricmp(cmd_a2, "list") == 0)
	{
		int	i;
		int	nums = 1;
		for (i = 0;i < 512;i += 1)
		{
			gteleporter_t	*tele = &g_teleporters[i];
			if (tele && (tele->active == 1))
			{
					trap_SendServerCmd(ent->fixednum, va("print \"^5%i^2)^7 %s\n\"", nums, tele->name));
					nums += 1;
			}
		}
		if (nums == 1)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1No chatcommands found.\n\""));
		}
		return;
	}
	failed:
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchatcommands add <name> [group] [type] [type2] [x] [y] [z] [yaw] ^2Creates a new chatcommand. Type: 0=teleport,1=empower,2=terminator,3=protect,4=kick,5=censor. type2: 0=fulltext, 1=start of text, 2=contained within text. (All options marked in [brackets] will automatically fill themselves in if left blank.) (Assumes teleporter at your current position)\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchatcommands delete <name>                ^2Deletes a chatcommand.\n\""));
	trap_SendServerCmd(ent->fixednum, va("print \"^1/amchatcommands list                         ^2See all chatcommands.\n\""));
	return;
}








int get_max_rocket_dist(int forluls)
{
	return mc_rocketdist.integer+(forluls*0);
}
int mcfix360(int orig)
{
	int	lul;
	for (lul = orig;lul > 360;lul -= 360){}
	for (lul = lul;lul < 0;lul += 360){}
	return lul;
}
void mcamignore(gentity_t *ent, char *enttarget)
{
	int targetplayer;
	targetplayer = dsp_adminTarget(ent, enttarget, ent->s.number);
	if (targetplayer == -1)
	{
		int i;
		for (i = 0;i <= 32;i += 1)
		{
			gentity_t	*glorp = &g_entities[i];
			if (glorp && glorp->inuse && glorp->client)
			{
				if (ent->client->sess.ignoring[i] == '0')
				{
					ent->client->sess.ignoring[i] = '0';
					trap_SendServerCmd(ent->fixednum, va("print \"^2No longer ignoring ^7%s^2.\n\"", glorp->client->pers.netname));
				}
				else
				{
					ent->client->sess.ignoring[i] = 'A';
					trap_SendServerCmd(ent->fixednum, va("print \"^1Ignoring ^7%s^1.\n\"", glorp->client->pers.netname));
				}
			}
		}
	}
	else if (targetplayer < 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown player.\n\""));
	}
	else
	{
		gentity_t	*glorp = &g_entities[targetplayer];
		if (!glorp || !glorp->inuse || !glorp->client)
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Unknown player.\n\""));
			return;
		}
		if (ent->client->sess.ignoring[targetplayer] == '0')
		{
			ent->client->sess.ignoring[targetplayer] = '0';
			trap_SendServerCmd(ent->fixednum, va("print \"^2No longer ignoring ^7%s^2.\n\"", glorp->client->pers.netname));
		}
		else
		{
			ent->client->sess.ignoring[targetplayer] = 'A';
				trap_SendServerCmd(ent->fixednum, va("print \"^1Ignoring ^7%s^1.\n\"", glorp->client->pers.netname));
		}
	}
}
/*
void addtsent(gentity_t *ent, char *usersafe, char *reaction, char *usersafe2)
{
	gentity_t	*t;
	t = G_Spawn();
	if (Q_stricmp(reaction, "kick") == 0)
	{
		t->bolt_Waist = 1;
	}
	else if (Q_stricmp(reaction, "kill") == 0)
	{
		t->bolt_Waist = 2;
	}
	else
	{
		G_Printf("mctsent auto removed - no reaction tag\n");
		trap_SendServerCmd( ent->s.number, va("print \"^1Bad reaction tag, refused spawn.\n\""));
		G_FreeEntity(t);
		return;
	}
	strcpy(t->mctarget, usersafe);
	strcpy(t->mctargetname, usersafe2);
	t->think = mctsent_think;
	t->nextthink = level.time+50;
	VectorCopy(ent->client->ps.origin, t->s.origin);
	G_SetOrigin(t, t->s.origin);
	trap_SendServerCmd( ent->s.number, va("print \"^2Spawned as ^5%i^2.\n\"", t->s.number));
}
*/

void SP_mc_note( gentity_t *ent, gentity_t *spawner, char *mpublic );
void mcmc_readnote(gentity_t *ent, char *noteread)
{
	int		chosennote;
	gentity_t	*note;
	if (Q_stricmp(noteread, "") == 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1/amreadnote <gun or notenum>\n\""));
		return;
	}
	if (Q_stricmp(noteread,"gun") == 0)
	{
		chosennote = find_ent(ent,"triggergun");
	}
	else
	{
		chosennote = find_ent(ent,noteread);
	}
	if (chosennote < 0)
	{
		if (chosennote == -1)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown entity. Please give a number or ~gun~.\n\""));
			return;
		}
		if (chosennote == -2)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1No entity found at gun position.\n\""));
			return;
		}
		if (chosennote == -3)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1The chosen entity is auto-protected.\n\""));
			return;
		}
		if (chosennote == -4)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1The chosen entity is protected.\n\""));
			return;
		}
		else
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown entity. Please give a number or ~gun~.\n\""));
			return;
		}
	}
	note = &g_entities[chosennote];
	if (Q_stricmp(note->classname,"mc_note") != 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1The chosen entity is not a note.\n\""));
		return;
	}
	if (((note->s.bolt1 == 0)&&(Q_stricmp(note->mctargetname, ent->client->sess.userlogged) == 0))||(note->s.bolt1 == 1)||(ent->client->sess.ampowers6 & 262144))
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^2--Note--^7Num^2:^7 %i^2,^7 Owner^2:^7 %s^2,^7 Contents^2:^7 %s^2.\n\"", note->s.number, note->mctargetname, note->mcmessage));
		return;
	}
	else
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1The note owner has chosen to keep this note private.\n\""));
		return;
	}
}
void mcmc_delnote(gentity_t *ent, char *noteread)
{
	int		chosennote;
	gentity_t	*note;
	if (Q_stricmp(noteread, "") == 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1/amdelnote <gun or notenum>\n\""));
		return;
	}
	if (Q_stricmp(noteread,"gun") == 0)
	{
		chosennote = find_ent(ent,"triggergun");
	}
	else
	{
		chosennote = find_ent(ent,noteread);
	}
	if (chosennote < 0)
	{
		if (chosennote == -1)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown entity. Please give a number or ~gun~.\n\""));
			return;
		}
		if (chosennote == -2)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1No entity found at gun position.\n\""));
			return;
		}
		if (chosennote == -3)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1The chosen entity is auto-protected.\n\""));
			return;
		}
		if (chosennote == -4)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1The chosen entity is protected.\n\""));
			return;
		}
		else
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown entity. Please give a number or ~gun~.\n\""));
			return;
		}
	}
	note = &g_entities[chosennote];
	if (Q_stricmp(note->classname,"mc_note") != 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1The chosen entity is not a note.\n\""));
		return;
	}
	if (((Q_stricmp(note->mctargetname, ent->client->sess.userlogged) == 0))||(ent->client->sess.ampowers6 & 262144))
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^1Removed - ^2Note -^7 Owner^2:^7 %s^2,^7 contents^2:^7 %s^2.\n\"", note->mctargetname, note->mcmessage));
		G_FreeEntity(note);
		/*if (level.thisistpm == 32)
		{
			save_all_notes();
		}*/
		return;
	}
}

void	Svcmd_AddNote( gentity_t *ent, int clientNum )
{
	gentity_t		*jakes_model = G_Spawn();
	char			*model;//[MAX_STRING_CHARS];
	vec3_t			origin;
	//char			xdlol[MAX_STRING_CHARS];
	char			mcpub[MAX_STRING_CHARS];
	//trap_Argv( 2, xdlol, sizeof( xdlol ) );
	//trap_Argv( 2, model, sizeof( model ) );
	model = ConcatArgs( 2 );

	trap_Argv( 1, mcpub, sizeof( mcpub ) );
	if (trap_Argc() == 1)
	{
		trap_SendServerCmd( clientNum, va("print \"^1/amaddnote <public> <do this here>\n\"" ) );
		G_FreeEntity(jakes_model);
		return;
	}
	else if (trap_Argc() >= 2)
	{
		level.numSpawnVars = 0;
		level.numSpawnVarChars = 0;
		AddSpawnField("model", model);
		VectorCopy(ent->client->ps.origin, origin);
		jakes_model->s.origin[0] = (int)origin[0];
		jakes_model->s.origin[1] = (int)origin[1];
		jakes_model->s.origin[2] = (int)origin[2] - 25;
		jakes_model->classname = "mc_note";
		//ent->client->ps.viewangles[0] = 0.0f;
		//ent->client->ps.viewangles[2] = 0.0f;
		strcpy(jakes_model->mctargetname, ent->client->sess.userlogged);
		SP_mc_note(jakes_model, ent, mcpub);
		jakes_model->custom = 1;
		VectorCopy(ent->client->ps.origin, origin);
		G_LogPrintf("AddNote admin command has been executed by %s. (%s) (%s) <%i %i %i>\n", ent->client->pers.netname, model, mcpub,(int)origin[0], (int)origin[1], (int)origin[2]);
		//VectorClear( origin );
		/*if (!ent->client->noclip)
		{
			VectorCopy(ent->client->ps.origin, origin);
			origin[2] += 20;
			VectorCopy(origin, ent->client->ps.origin);
		}* /
		ent->client->sess.lastent = jakes_model->s.number;
		ent->client->sess.traced = jakes_model->s.number;
		trap_SendServerCmd(ent->fixednum, va("print \"^2Added note ^5%i^2.\n\"", jakes_model->s.number));
		/*if (level.thisistpm == 32)
		{
			save_all_notes();
		}*/
	}
}


void fixforundopl(gentity_t *ent)
{
	VectorCopy(ent->client->ps.origin,ent->lastorigin);
}
void fixforundo(gentity_t *ent)
{
	VectorCopy(ent->r.currentOrigin,ent->lastorigin);
}
void fixforundor(gentity_t *ent)
{
	VectorCopy(ent->r.currentAngles,ent->lastrot);
}
#define		TPM_POWERSET1		66;
#define		TPM_POWERSET2		8;
#define		TPM_POWERSET3		131073;
#define		TPM_POWERSET4		135168;
#define		TPM_POWERSET5		0;
#define		TPM_POWERSET6		524288;
qboolean candocommand(gentity_t *ent, int powerset, int powerz)
{
	if (Q_stricmp(ent->client->sess.userlogged,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You are not logged in.\n\""));
		G_Printf("Refused command.\n");
		return qfalse;
	}

		switch (powerset)
		{
			case 1: if ((level.freepower1) & powerz){return qtrue;}break;
			case 2: if ((level.freepower2) & powerz){return qtrue;}break;
			case 3: if ((level.freepower3) & powerz){return qtrue;}break;
			case 4: if ((level.freepower4) & powerz){return qtrue;}break;
			case 5: if ((level.freepower5) & powerz){return qtrue;}break;
			case 6: if ((level.freepower6) & powerz){return qtrue;}break;
			case 7: if ((level.freepower7) & powerz){return qtrue;}break;
			default: G_Printf("Error - attempted to check powergroup %i for %i - unknown powergroup", powerset, powerz);
		}

	if (ent->client->sess.adminloggedin == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You are not an admin.\n\""));
		G_Printf("Refused command.\n");
		return qfalse;
	}
	else
	{
		switch (powerset)
		{
			case 1: if ((ent->client->sess.ampowers) & powerz){return qtrue;}break;
			case 2: if ((ent->client->sess.ampowers2) & powerz){return qtrue;}break;
			case 3: if ((ent->client->sess.ampowers3) & powerz){return qtrue;}break;
			case 4: if ((ent->client->sess.ampowers4) & powerz){return qtrue;}break;
			case 5: if ((ent->client->sess.ampowers5) & powerz){return qtrue;}break;
			case 6: if ((ent->client->sess.ampowers6) & powerz){return qtrue;}break;
			case 7: if ((ent->client->sess.ampowers7) & powerz){return qtrue;}break;
			default: G_Printf("Error - attempted to check powergroup %i for %i - unknown powergroup", powerset, powerz);
		}
	}
	trap_SendServerCmd(ent->fixednum, va("print \"^1You are not allowed to use this command.\n\""));
	G_Printf("Refused command.\n");
	return qfalse;
}
char *fixed500(char *beforefixed)
{
	char fixer[1024];
	int i;
	for (i = 0;i < 1023;i += 1)
	{
		fixer[i] = beforefixed[i+1];
	}
	return va(fixer);
}
int monkeyspot(char *spot, int xyorz, gentity_t *playerent, gentity_t *otherent)
{
	vec3_t	origin, origin2, angles, angles2;
   	int xyz;
	xyz = xyorz;
	trap_UnlinkEntity(playerent);
	VectorClear(origin);
	VectorClear(origin2);
	VectorClear(angles);
	VectorClear(angles2);
	VectorCopy(playerent->client->ps.origin, origin);
	VectorCopy(playerent->client->ps.viewangles, angles);
	if (otherent->client)
	{
		trap_UnlinkEntity(otherent);
		VectorCopy(otherent->client->ps.origin, origin2);
		VectorCopy(otherent->client->ps.viewangles, angles2);
	}
	else
	{
		VectorCopy(otherent->r.currentOrigin, origin2);
		VectorCopy(otherent->r.currentAngles, angles2);
	}
	if (xyz > 2)
	{
		xyz = 2;
	}
	if (xyz < 0)
	{
		xyz = 0;
	}
	if (Q_stricmp(spot,"") == 0)
	{
		if (xyz == 2)
		{
			return (int)origin[2]-25;
		}
		else
		{
			return (int)origin[xyz];
		}
	}
	else if (Q_stricmp(spot,"*") == 0)
	{
		return (int)origin[xyz];
	}
	else if (Q_stricmp(spot,"#") == 0)
	{
		return (int)otherent->lastorigin[xyz];
	}
	else if (spot[0] == '&')
	{
		gentity_t *targ;
		int	iTarg;
		vec3_t forigin;
		iTarg = dsp_adminTarget(playerent, fixed500(spot), playerent->s.number);
		if (iTarg < 0)
		{
			trap_SendServerCmd(playerent->s.number, va("print \"^1Unknown player %s^1.\n\"", fixed500(spot)));
			return 0;
		}
		targ = &g_entities[iTarg];
		if (targ->client->sess.noteleport == 1)
		{
			if (!(playerent->client->sess.ampowers7 & 32))
			{
				trap_SendServerCmd(playerent->s.number, va("print \"^1Player does not want to be teleported to.\n\""));
				return 0;
			}
		}
		VectorClear(forigin);
		VectorCopy(targ->client->ps.origin, forigin);
		return (int)forigin[xyz];
	}
	else if (spot[0] == '!')
	{
		gentity_t *targ;
		int	iTarg;
		vec3_t forigin, fangles;
		iTarg = dsp_adminTarget(playerent, fixed500(spot), playerent->s.number);
		if (iTarg < 0)
		{
			trap_SendServerCmd(playerent->s.number, va("print \"^1Unknown player %s^1.\n\"", fixed500(spot)));
			return 0;
		}
		else
		{
			vec3_t  viewspot, vf, src, dest;
			trace_t trace;
			targ = &g_entities[iTarg];
		if (targ->client->sess.noteleport == 1)
		{
			if (!(playerent->client->sess.ampowers7 & 32))
			{
				trap_SendServerCmd(playerent->s.number, va("print \"^1Player does not want to be teleported to.\n\""));
				return 0;
			}
		}
			VectorClear(forigin);
			VectorClear(fangles);
			VectorCopy(targ->client->ps.origin, forigin);
			VectorCopy(targ->client->ps.viewangles, fangles);
			VectorCopy(forigin, viewspot);
			viewspot[2] += 35;
			VectorCopy( viewspot, src );
			AngleVectors( fangles, vf, NULL, NULL );
			VectorMA( src, 8192, vf, dest );
			trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, targ->s.number, MASK_SHOT );
			trace.endpos[2] += 25;
			return trace.endpos[xyz];
		}
	}
	else if (Q_stricmp(spot,"@") == 0)
	{
			return (int)origin2[xyz];
	}
	else if (Q_stricmp(spot,"^") == 0)
	{
		vec3_t  viewspot, vf, src, dest;
		trace_t trace;
		VectorCopy(origin, viewspot);
		viewspot[2] += 35;
		VectorCopy( viewspot, src );
		AngleVectors( angles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, playerent->s.number, MASK_SHOT );
		trace.endpos[2] += 25;
		return trace.endpos[xyz];
	}
	else if (Q_stricmp(spot,"$") == 0)
	{
		vec3_t  viewspot, vf, src, dest;
		trace_t trace;
		VectorCopy(origin2, viewspot);
		viewspot[2] += 35;
		VectorCopy( viewspot, src );
		AngleVectors( angles2, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, otherent->s.number, MASK_SHOT );
		trace.endpos[2] += 25;
		return trace.endpos[xyz];
	}
	else
	{
		return atoi(spot);
	}
	//return 0;
}




gentity_t *G_Findbygroup (gentity_t *from, int rgroup)
{
	char	*s;

	if (!from)
		from = g_entities;
	else
		from++;

	for ( ; from < &g_entities[level.num_entities] ; from++)
	{
		if (!from->inuse)
		{
			continue;
		}
		if (from->entgroup == rgroup)
		{
			return from;
		}
	}

	return NULL;
}

void mc_addlight(gentity_t *ent)
{
	char par1[MAX_STRING_CHARS];
	char par2[MAX_STRING_CHARS];
	char par3[MAX_STRING_CHARS];
	char par4[MAX_STRING_CHARS];
	trap_Argv( 1, par1, sizeof( par1 ) );
	trap_Argv( 2, par2, sizeof( par2 ) );
	trap_Argv( 3, par3, sizeof( par3 ) );
	trap_Argv( 4, par4, sizeof( par4 ) );
	if (Q_stricmp(par4,"") != 0)
	{
		gentity_t	*light = G_Spawn();
		light->s.origin[0] = ent->client->ps.origin[0];
		light->s.origin[1] = ent->client->ps.origin[1];
		light->s.origin[2] = ent->client->ps.origin[2];
		light->custom = 1;
		SP_mc_light ( light );
		light->s.constantLight = atoi(par1) + (atoi(par2)<<8) + (atoi(par3)<<16) + (atoi(par4)<<24);
		strcpy(light->mcmlight,va("%i %i %i %i",atoi(par1), atoi(par2), atoi(par3), atoi(par4)));
		trap_SendServerCmd(ent->fixednum, va("print \"^2Spawned a light as ^5%i^2.\n\"", light->s.number));
	}
	else
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1/ammap_addlight <red> <green> <blue> <intensity>\n\""));
		return;
	}
}
void blankvoidthink( gentity_t *self)
{
}
void illusionspawn( gentity_t *ent ) {
	gentity_t		*body;
	int			contents;

	if (level.intermissiontime)
	{
		return;
	}

	trap_UnlinkEntity (ent);
	body = G_Spawn();
	trap_UnlinkEntity (body);
	body->s = ent->s;
	//avoid oddly angled corpses floating around
	body->s.angles[PITCH] = body->s.angles[ROLL] = body->s.apos.trBase[PITCH] = body->s.apos.trBase[ROLL] = 0;
	body->s.g2radius = 100;
	body->s.eType = ET_BODY;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
	VectorCopy(ent->client->ps.lastHitLoc, body->s.origin2);
	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;
	//body->s.weapon = ent->s.bolt2;
	body->s.weapon = ent->client->ps.weapon;
	G_AddEvent(body, EV_BODY_QUEUE_COPY, ent->s.clientNum);
	body->r.svFlags = ent->r.svFlags | SVF_BROADCAST;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);
	body->s.torsoAnim = body->s.legsAnim = ent->client->ps.legsAnim & ~ANIM_TOGGLEBIT;
	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	ent->clipmask = MASK_SOLID;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;
	body->nextthink = level.time + 500;
	body->think = blankvoidthink;
	body->die = body_die;
	body->classname = "amillusion";
	// don't take more damage if already gibbed
	body->health = 300;
	body->takedamage = qtrue;

	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}
void channelprint(char *group, char *message)
{
	int i;
	gentity_t *flent;
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(group,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("print \"%s\"", message));
		}
		if (Q_stricmp(group,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("print \"%s\"", message));
		}
		if (Q_stricmp(group,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("print \"%s\"", message));
		}
	}
}
void channelkick(gentity_t *ent, int group, char *player)
{
	int i;
	gentity_t	*other;
	if (group == 1)
	{
		if (ent->client->sess.channel_01rank == 5)
		{
			i = dsp_adminTarget(ent, player, ent->s.number);
			if (i < 0)
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^3Invalid player.\n\""));
				return;
			}
			if (Q_stricmp(ent->client->sess.channel_01name,other->client->sess.channel_01name) == 0)
			{
				strcpy(other->client->sess.channel_01name,"");
				strcpy(other->client->sess.channel_01pass,"");
				other->client->sess.channel_01rank = 0;
			}
			else if (Q_stricmp(ent->client->sess.channel_01name,other->client->sess.channel_02name) == 0)
			{
				strcpy(other->client->sess.channel_02name,"");
				strcpy(other->client->sess.channel_02pass,"");
				other->client->sess.channel_02rank = 0;
			}
			else if (Q_stricmp(ent->client->sess.channel_01name,other->client->sess.channel_03name) == 0)
			{
				strcpy(other->client->sess.channel_03name,"");
				strcpy(other->client->sess.channel_03pass,"");
				other->client->sess.channel_03rank = 0;
			}
			else
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^7%s^3 is not in your group.\n\"", other->client->pers.netname));
				return;
			}
			channelprint(ent->client->sess.channel_01name,va("^5[%s]^7%s^5 has been kicked from the channel.\n", ent->client->sess.channel_01name, other->client->pers.netname));
		}
	}
	else if (group == 2)
	{
		if (ent->client->sess.channel_02rank == 5)
		{
			i = dsp_adminTarget(ent, player, ent->s.number);
			if (i < 0)
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^3Invalid player.\n\""));
				return;
			}
			if (Q_stricmp(ent->client->sess.channel_02name,other->client->sess.channel_01name) == 0)
			{
				strcpy(other->client->sess.channel_01name,"");
				strcpy(other->client->sess.channel_01pass,"");
				other->client->sess.channel_01rank = 0;
			}
			else if (Q_stricmp(ent->client->sess.channel_02name,other->client->sess.channel_02name) == 0)
			{
				strcpy(other->client->sess.channel_02name,"");
				strcpy(other->client->sess.channel_02pass,"");
				other->client->sess.channel_02rank = 0;
			}
			else if (Q_stricmp(ent->client->sess.channel_02name,other->client->sess.channel_03name) == 0)
			{
				strcpy(other->client->sess.channel_03name,"");
				strcpy(other->client->sess.channel_03pass,"");
				other->client->sess.channel_03rank = 0;
			}
			else
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^7%s^3 is not in your group.\n\"", other->client->pers.netname));
				return;
			}
			channelprint(ent->client->sess.channel_01name,va("^5[%s]^7%s^5 has been kicked from the channel.\n", ent->client->sess.channel_02name, other->client->pers.netname));
		}
	}
	else if (group == 1)
	{
		if (ent->client->sess.channel_03rank == 5)
		{
			i = dsp_adminTarget(ent, player, ent->s.number);
			if (i < 0)
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^3Invalid player.\n\""));
				return;
			}
			if (Q_stricmp(ent->client->sess.channel_03name,other->client->sess.channel_01name) == 0)
			{
				strcpy(other->client->sess.channel_01name,"");
				strcpy(other->client->sess.channel_01pass,"");
				other->client->sess.channel_01rank = 0;
			}
			else if (Q_stricmp(ent->client->sess.channel_03name,other->client->sess.channel_02name) == 0)
			{
				strcpy(other->client->sess.channel_02name,"");
				strcpy(other->client->sess.channel_02pass,"");
				other->client->sess.channel_02rank = 0;
			}
			else if (Q_stricmp(ent->client->sess.channel_03name,other->client->sess.channel_03name) == 0)
			{
				strcpy(other->client->sess.channel_03name,"");
				strcpy(other->client->sess.channel_03pass,"");
				other->client->sess.channel_03rank = 0;
			}
			else
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^7%s^3 is not in your group.\n\"", other->client->pers.netname));
				return;
			}
			channelprint(ent->client->sess.channel_01name,va("^5[%s]^7%s^5 has been kicked from the channel.\n", ent->client->sess.channel_03name, other->client->pers.netname));
		}
	}
}
void listchannelmembers(gentity_t *ent, char *channel)
{
	int i;
	gentity_t *flent;
	trap_SendServerCmd(ent->fixednum, va("print \"^3Channel: ^5%s^3, password: ^5%s^3.\n\"", ent->client->sess.channel_01name, ent->client->sess.channel_01pass));
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (!flent || !flent->client || !flent->inuse)
		{
			continue;
		}
		if (Q_stricmp(channel,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^5%i^3)^7%s^5, \n\"", flent->client->sess.channel_01rank, flent->client->pers.netname ));
		}
		if (Q_stricmp(channel,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^5%i^3)^7%s^5, \n\"", flent->client->sess.channel_02rank, flent->client->pers.netname ));
		}
		if (Q_stricmp(channel,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( ent-g_entities, va("print \"^5%i^3)^7%s^5, \n\"", flent->client->sess.channel_03rank, flent->client->pers.netname ));
		}
	}
}
void listchannels(gentity_t *ent)
{
	if (ent->client->sess.channel_01rank > 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^31 - %s^3(%i)\n\"", ent->client->sess.channel_01name, ent->client->sess.channel_01rank));
	}
	if (ent->client->sess.channel_02rank > 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^32 - %s^3(%i)\n\"", ent->client->sess.channel_02name, ent->client->sess.channel_02rank));
	}
	if (ent->client->sess.channel_03rank > 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^33 - %s^3(%i)\n\"", ent->client->sess.channel_03name, ent->client->sess.channel_03rank));
	}
}
void createchannel(gentity_t *ent, char *channelname, char *channelpass)
{
	int i;
	gentity_t *flent;
	if ((Q_stricmp(ent->client->sess.channel_01name,"") != 0)&&(Q_stricmp(ent->client->sess.channel_02name,"") != 0)&&(Q_stricmp(ent->client->sess.channel_03name,"") != 0))
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^3You are in 3 channels already. Please leave 1 channel before creating a new one.\n\""));
		return;
	}
	if (Q_stricmp(channelname,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^3Please specifiy a channel name.\n\""));
		return;
	}
	if (Q_stricmp(channelpass,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^3Please specifiy a channel pass.\n\""));
		return;
	}
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (flent && flent->client && flent->inuse)
		{
			if (Q_stricmp(channelname,flent->client->sess.channel_01name) == 0)
			{
				trap_SendServerCmd( ent-g_entities, va("chat \"^3Channel already exists.\""));
				return;
			}
		}
	}
	if (Q_stricmp(ent->client->sess.channel_01name,"") == 0)
	{
		strcpy(ent->client->sess.channel_01name,channelname);
		strcpy(ent->client->sess.channel_01pass,channelpass);
		ent->client->sess.channel_01rank = 5;
		trap_SendServerCmd(ent->fixednum, va("print \"^3Channel 1 successfully created.\n\""));
		return;
	}
	if (Q_stricmp(ent->client->sess.channel_02name,"") == 0)
	{
		strcpy(ent->client->sess.channel_02name,channelname);
		strcpy(ent->client->sess.channel_02pass,channelpass);
		ent->client->sess.channel_02rank = 5;
		trap_SendServerCmd(ent->fixednum, va("print \"^3Channel 2 successfully created.\n\""));
		return;
	}
	if (Q_stricmp(ent->client->sess.channel_03name,"") == 0)
	{
		strcpy(ent->client->sess.channel_03name,channelname);
		strcpy(ent->client->sess.channel_03pass,channelpass);
		ent->client->sess.channel_03rank = 5;
		trap_SendServerCmd(ent->fixednum, va("print \"^3Channel 3 successfully created.\n\""));
		return;
	}
}
void leavechannel(gentity_t *ent, int channel)
{
	int i;
	gentity_t *flent;
	char	oldname[MAX_STRING_CHARS];
	switch (channel)
	{
		case 0:
			trap_SendServerCmd(ent->fixednum, va("print \"^3Please specify a channel number.\n\""));
			break;
		case 1:
			if (Q_stricmp(ent->client->sess.channel_01name,"") != 0)
			{
				//trap_SendServerCmd(ent->fixednum, va("print \"^3Left channel 1.\n\""));
				strcpy(oldname,ent->client->sess.channel_01name);
				channelprint(ent->client->sess.channel_01name,va("^5[%s^5]^7%s^5 has left the channel.\n", oldname, ent->client->pers.netname ));
				strcpy(ent->client->sess.channel_01name,"");
				strcpy(ent->client->sess.channel_01pass,"");
				ent->client->sess.channel_01rank = 0;
/*
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(oldname,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
		if (Q_stricmp(oldname,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
		if (Q_stricmp(oldname,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
	}
*/
			}
			else
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^3You do not have a channel 1.\n\""));
			}
			break;
		case 2:
			if (Q_stricmp(ent->client->sess.channel_02name,"") != 0)
			{
				strcpy(oldname,ent->client->sess.channel_02name);
			channelprint(ent->client->sess.channel_01name,va("^5[%s^5]^7%s^5 has left the channel.\n", oldname, ent->client->pers.netname ));
				strcpy(ent->client->sess.channel_02name,"");
				strcpy(ent->client->sess.channel_02pass,"");
				ent->client->sess.channel_02rank = 0;
/*
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(oldname,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
		if (Q_stricmp(oldname,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
		if (Q_stricmp(oldname,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
	}*/
				//trap_SendServerCmd(ent->fixednum, va("print \"^3Left channel 2.\n\""));
			}
			else
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^3You do not have a channel 2.\n\""));
			}
			break;
		case 3:
			if (Q_stricmp(ent->client->sess.channel_03name,"") != 0)
			{
				strcpy(oldname,ent->client->sess.channel_03name);
			channelprint(ent->client->sess.channel_01name,va("^5[%s^5]^7%s^5 has left the channel.\n", oldname, ent->client->pers.netname ));
				strcpy(ent->client->sess.channel_03name,"");
				strcpy(ent->client->sess.channel_03pass,"");
				ent->client->sess.channel_03rank = 0;
/*
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(oldname,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
		if (Q_stricmp(oldname,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
		if (Q_stricmp(oldname,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^5 has left the channel.\"", oldname, ent->client->pers.netname ));
		}
	}*/
				//trap_SendServerCmd(ent->fixednum, va("print \"^3Left channel 3.\n\""));
			}
			else
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^3You do not have a channel 3.\n\""));
			}
			break;
		default:
			trap_SendServerCmd(ent->fixednum, va("print \"^3Channel must be between 1 and 3.\n\""));
			break;
	}
}
void joinchannel(gentity_t *ent, char *group, char *grouppass)
{
	gentity_t	*flent;
	int		i;
	int		found;
	int		i2;
	gentity_t	*flent2;
	char		oldname[MAX_STRING_CHARS];
	found = 0;
	for (i = 0;i < 31;i+=1)
	{
		flent = &g_entities[i];
		if (!flent || !flent->client || !flent->inuse)
		{
			continue;
		}
		if (flent->client->sess.channel_01rank == 5)
		{
			if ((Q_stricmp(flent->client->sess.channel_01name,group) == 0))
			{
				found = 1;
				if (Q_stricmp(ent->client->sess.channel_01name,flent->client->sess.channel_01name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(ent->client->sess.channel_02name,flent->client->sess.channel_01name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(ent->client->sess.channel_03name,flent->client->sess.channel_01name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(flent->client->sess.channel_01pass,grouppass) == 0)
				{
					if (Q_stricmp(ent->client->sess.channel_01name,"") == 0)
					{
						ent->client->sess.channel_01rank = 1;
						strcpy(ent->client->sess.channel_01name,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_01pass,flent->client->sess.channel_01pass);
						//return;
					}
					else if (Q_stricmp(ent->client->sess.channel_02name,"") == 0)
					{
						ent->client->sess.channel_02rank = 1;
						strcpy(oldname,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_02name,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_02pass,flent->client->sess.channel_01pass);
						//return;
					}
					else if (Q_stricmp(ent->client->sess.channel_03name,"") == 0)
					{
						ent->client->sess.channel_03rank = 1;
						strcpy(oldname,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_03name,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_03pass,flent->client->sess.channel_01pass);
						//return;
					}
					else
					{
						trap_SendServerCmd(ent->fixednum,va("print \"^3You are in 3 channels. This is the channel limit. Please leave 1 channel to join a new one.\n\""));
						return;
					}
			channelprint(ent->client->sess.channel_01name,va("^5[%s^5]^7%s^5 has joined the channel.\n", oldname, ent->client->pers.netname ));
/*
	for (i2 = 0; i2 < 32; i2+=1)
	{
		flent2 = &g_entities[i2];
		if (Q_stricmp(group,flent2->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
		if (Q_stricmp(group,flent2->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
		if (Q_stricmp(group,flent2->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
	}*/
					//trap_SendServerCmd(ent->fixednum,va("print \"^3Joined group ^5%s^3.\n\"",flent->client->sess.channel_01name));
					return;
				}
				else
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3Incorrect password.\n\""));
					return;
				}
			}
		}
	}
	for (i = 0;i < 31;i+=1)
	{
		flent = &g_entities[i];
		if (!flent || !flent->client)
		{
			continue;
		}
		if (flent->client->sess.channel_02rank == 5)
		{
			if ((Q_stricmp(flent->client->sess.channel_02name,group) == 0))
			{
				found = 1;
				if (Q_stricmp(ent->client->sess.channel_01name,flent->client->sess.channel_01name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(ent->client->sess.channel_02name,flent->client->sess.channel_01name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(ent->client->sess.channel_03name,flent->client->sess.channel_01name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(flent->client->sess.channel_02pass,grouppass) == 0)
				{
					if (Q_stricmp(ent->client->sess.channel_01name,"") == 0)
					{
						ent->client->sess.channel_01rank = 1;
						strcpy(ent->client->sess.channel_01name,flent->client->sess.channel_02name);
						strcpy(ent->client->sess.channel_01pass,flent->client->sess.channel_02pass);
						//return;
					}
					else if (Q_stricmp(ent->client->sess.channel_02name,"") == 0)
					{
						ent->client->sess.channel_02rank = 1;
						strcpy(oldname,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_02name,flent->client->sess.channel_02name);
						strcpy(ent->client->sess.channel_02pass,flent->client->sess.channel_02pass);
						//return;
					}
					else if (Q_stricmp(ent->client->sess.channel_03name,"") == 0)
					{
						ent->client->sess.channel_03rank = 1;
						strcpy(oldname,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_03name,flent->client->sess.channel_02name);
						strcpy(ent->client->sess.channel_03pass,flent->client->sess.channel_02pass);
						//return;
					}
					else
					{
						trap_SendServerCmd(ent->fixednum,va("print \"^3You are in 3 channels. This is the channel limit. Please leave 1 channel to join a new one.\n\""));
						return;
					}
			channelprint(ent->client->sess.channel_02name,va("^5[%s^5]^7%s^5 has joined the channel.\n", oldname, ent->client->pers.netname ));
/*
	for (i2 = 0; i2 < 32; i2+=1)
	{
		flent2 = &g_entities[i2];
		if (Q_stricmp(group,flent2->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
		if (Q_stricmp(group,flent2->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
		if (Q_stricmp(group,flent2->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
	}*/
					//trap_SendServerCmd(ent->fixednum,va("print \"^3Joined group ^5%s^3.\n\"",flent->client->sess.channel_02name));
					return;
				}
				else
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3Incorrect password.\n\""));
					return;
				}
			}
		}
	}
	for (i = 0;i < 31;i+=1)
	{
		flent = &g_entities[i];
		if (!flent || !flent->client)
		{
			continue;
		}
		if (flent->client->sess.channel_03rank == 5)
		{
			if ((Q_stricmp(flent->client->sess.channel_03name,group) == 0))
			{
				found = 1;
				if (Q_stricmp(ent->client->sess.channel_01name,flent->client->sess.channel_03name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(ent->client->sess.channel_02name,flent->client->sess.channel_03name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(ent->client->sess.channel_03name,flent->client->sess.channel_03name) == 0)
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3You are already in this group.\n\""));
					return;
				}
				if (Q_stricmp(flent->client->sess.channel_03pass,grouppass) == 0)
				{
					if (Q_stricmp(ent->client->sess.channel_01name,"") == 0)
					{
						ent->client->sess.channel_01rank = 1;
						strcpy(ent->client->sess.channel_01name,flent->client->sess.channel_03name);
						strcpy(ent->client->sess.channel_01pass,flent->client->sess.channel_03pass);
						//return;
					}
					else if (Q_stricmp(ent->client->sess.channel_02name,"") == 0)
					{
						ent->client->sess.channel_02rank = 1;
						strcpy(oldname,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_02name,flent->client->sess.channel_03name);
						strcpy(ent->client->sess.channel_02pass,flent->client->sess.channel_03pass);
						//return;
					}
					else if (Q_stricmp(ent->client->sess.channel_03name,"") == 0)
					{
						ent->client->sess.channel_03rank = 1;
						strcpy(oldname,flent->client->sess.channel_01name);
						strcpy(ent->client->sess.channel_03name,flent->client->sess.channel_03name);
						strcpy(ent->client->sess.channel_03pass,flent->client->sess.channel_03pass);
						//return;
					}
					else
					{
						trap_SendServerCmd(ent->fixednum,va("print \"^3You are in 3 channels. This is the channel limit. Please leave 1 channel to join a new one.\n\""));
						return;
					}
			channelprint(ent->client->sess.channel_03name,va("^5[%s^5]^7%s^5 has joined the channel.\n", oldname, ent->client->pers.netname ));
/*
	for (i2 = 0; i2 < 32; i2+=1)
	{
		flent2 = &g_entities[i2];
		if (Q_stricmp(group,flent2->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
		if (Q_stricmp(group,flent2->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
		if (Q_stricmp(group,flent2->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent2-g_entities, va("chat \"^5[%s^5]^7%s^5 has joined the channel.\"", group, ent->client->pers.netname ));
		}
	}*/
					//trap_SendServerCmd(ent->fixednum,va("print \"^3Joined group ^5%s^3.\n\"",flent->client->sess.channel_01name));
					return;
				}
				else
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^3Incorrect password.\n\""));
					return;
				}
			}
		}
	}
	if (found == 0)
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^3Unknown channel.\n\""));
	}
}
int dsp_isEmpowered(int clientNum)
{
	// We're gonna call this Int to see if you're empowered and using grip. (bg_pmove.c)
	// If you're empowered you should be able to walk with normal speed, instead of slow speed
	gentity_t *other = &g_entities[clientNum];
	if (other && other->client && other->inuse && other->client->sess.empower)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
int bitpower(int rnum)
{
	int i;
	int f;
	f = 1;
	for (i = 1; i < rnum; i+=1)
	{
		f *= 2;
	}
	return f;
}
int isingroup(gentity_t *ent, int rgroup)
{
	char	buffer[MAX_STRING_CHARS];
	strcpy(buffer,ent->client->sess.mygroup);
	if (!ent)
	{
		return 0;
	}
	if (!ent->client)
	{
		return 0;
	}
	if (Q_stricmp(ent->client->sess.userlogged, "") == 0)
	{
		return 0;
	}
	if (rgroup > MAX_STRING_CHARS)
	{
		return 0;
	}
	if (buffer[rgroup-1] == 'A')
	{
		return 1; // 1 = In the group
	}
	else
	{
		return 0; // not in group
	}
}
char *stringforgroup(int rgroup)
{
	if (rgroup > MAX_STRING_CHARS)
	{
		return "Group Number Error:OverSized";
	}
	switch (rgroup)
	{
		case 1:
			return mc_group1_name.string;
			break;
		case 2:
			return mc_group2_name.string;
			break;
		case 3:
			return mc_group3_name.string;
			break;
		case 4:
			return mc_group4_name.string;
			break;
		case 5:
			return mc_group5_name.string;
			break;
		case 6:
			return mc_group6_name.string;
			break;
		default:
			return "Group Number Error: Not between 1 and 6";
			break;
	}
}
char *stringforrank(int rgroup)
{
	return va("^2Level^7-^2%i", rgroup);
	/*
	if (rgroup > MAX_STRING_CHARS)
	{
		return "Group Number Error:OverSized";
	}

	switch (rgroup)
	{
		case 1:
			return "^2Level^7-^21";
			break;
		case 2:
			return "^2Level^7-^22";
			break;
		case 3:
			return "^2Level^7-^23";
			break;
		case 4:
			return "^2Level^7-^24";
			break;
		case 5:
			return "^2Level^7-^25";
			break;
			break;
		case 6:
			return "^2Level^7-^26";
			break;
		default:
			return "Rank Number Error: Not between 1 and 6";
			break;
	}*/
}
/*
=================
mclogout
=================
*/
void mclogout(gentity_t *ent)
{
	if ( (Q_stricmp(ent->client->sess.userlogged, "") == 0) )
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1You are not logged in.\n\""));
	}
	else
	{
		ent->client->sess.clanCounter = level.time + 1000;
		ent->client->sess.clanTagTime = level.time + twimod_clantagpcounter.integer;
		Q_strncpyz(ent->client->sess.userlogged, "", sizeof(ent->client->sess.userlogged));
		Q_strncpyz(ent->client->sess.userpass, "", sizeof(ent->client->sess.userpass));
		Q_strncpyz(ent->client->sess.mygroup, "0000000000", sizeof(ent->client->sess.mygroup));
		ent->client->sess.adminloggedin = 0;
		ent->client->sess.credits = 0;
		ent->client->sess.ampowers = 0;
		ent->client->sess.ampowers2 = 0;
		ent->client->sess.ampowers3 = 0;
		ent->client->sess.ampowers4 = 0;
		ent->client->sess.ampowers5 = 0;
		ent->client->sess.ampowers6 = 0;
		ent->client->sess.ampowers7 = 0;
		ent->client->sess.mcgroup = 0;
		trap_SendServerCmd( -1, va("print \"^7%s^7 has logged out.\n\"", ent->client->pers.netname));
	}
	status_update();
}
/*
=================
renameself
=================
*/
void	renameself(gentity_t *ent)
{
	char	userinfo[MAX_INFO_VALUE];
	char	*newName;
	if (trap_Argc() > 1)
	{
		newName = ConcatArgs(1);
		trap_GetUserinfo(ent->s.number, userinfo, MAX_INFO_STRING);
		strcpy(ent->client->pers.netname, newName);
		Info_SetValueForKey(userinfo, "name", newName);
		trap_SetUserinfo(ent->s.number, userinfo);
		ClientUserinfoChanged( ent->s.number );
	}
	else
	{
		trap_SendServerCmd( ent->s.number, va("print \"^3/renameself <name>\n\"" ) );
	}
}
/*
=================
noteleport
=================
*/
void	setnoteleport(gentity_t *ent)
{
	if (ent->client->sess.noteleport == 0)
	{
		ent->client->sess.noteleport = 1;
		trap_SendServerCmd( ent->s.number, va("print \"^3Noteleport ^2activated^3. Low admins can no longer teleport you.\n\"" ) );
	}
	else
	{
		ent->client->sess.noteleport = 0;
		trap_SendServerCmd( ent->s.number, va("print \"^3Noteleport ^1deactivated^3. Admins can now teleport you.\n\"" ) );
	}
}
void	setAFKOff(gentity_t *ent)
{
	ent->client->sess.ticksAFK = 0;
		ent->client->sess.xAFK = (int)ent->client->ps.origin[0];
	if (ent->client->sess.isAFK == 1)
	{
		char	userinfo[MAX_INFO_STRING];
	G_Printf("Client %i un-AFK'd\n", ent->s.number);
		toggolo = 1;
		ent->client->sess.isAFK = 0;
			trap_GetUserinfo(ent->s.number, userinfo, MAX_INFO_STRING);
			strcpy(ent->client->pers.netname, ent->client->sess.rrname);
			Info_SetValueForKey(userinfo, "name", ent->client->sess.rrname);
			trap_SetUserinfo(ent->s.number, userinfo);
			ClientUserinfoChanged( ent->s.number );
		Q_strncpyz(ent->client->sess.rrname, "", sizeof(ent->client->sess.rrname));
		trap_SendServerCmd( -1, va("print \"^7#%s^7 is no longer AFK.\n\"", ent->client->pers.netname ) );
		toggolo = 0;
	}
}
void	setAFKOn(gentity_t *ent)
{
	if (ent->client->sess.isAFK == 0)
	{
		char	userinfo[MAX_INFO_STRING];
		char	*newName = va("^7[AFK]%s", ent->client->pers.netname);
		toggolo = 1;
		trap_SendServerCmd( -1, va("print \"^7#%s^7 is now AFK.\n\"", ent->client->pers.netname ) );
		Q_strncpyz(ent->client->sess.rrname, ent->client->pers.netname, sizeof(ent->client->sess.rrname));
			trap_GetUserinfo(ent->s.number, userinfo, MAX_INFO_STRING);
			strcpy(ent->client->pers.netname, newName);
			Info_SetValueForKey(userinfo, "name", newName);
			trap_SetUserinfo(ent->s.number, userinfo);
			ClientUserinfoChanged( ent->s.number );
		ent->client->sess.isAFK = 1;
		ent->client->sess.xAFK = (int)ent->client->ps.origin[0];
		toggolo = 0;
	}
}
void	setAFK(gentity_t *ent)
{
	if (ent->client->sess.isAFK == 0)
	{
		setAFKOn(ent);
	}
	else
	{
		setAFKOff(ent);
	}
}
/*
=================
svcmd_addmcbutton
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_mcbutton( gentity_t *ent );
void	svcmd_addmcbutton2( gentity_t *ent ) {
	gentity_t *mcbutton = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctarget[MAX_STRING_CHARS];
	char   mcmessage[MAX_STRING_CHARS];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 7 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcbutton <cost> <target> <message> <x> <y> <z>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcbutton->count = atoi(mccount);
	trap_Argv( 2, mctarget, 1024 );
	G_ParseField( "target", mctarget, mcbutton );
	trap_Argv( 3, mcmessage, 1024 );
	G_ParseField( "message", mcmessage, mcbutton );
	trap_Argv( 4, origin_number, 1024 );
	mcbutton->s.origin[0] = atoi(origin_number);
	trap_Argv( 5, origin_number2, 1024 );
	mcbutton->s.origin[1] = atoi(origin_number2);
	trap_Argv( 6, origin_number3, 1024 );
	mcbutton->s.origin[2] = atoi(origin_number3) - 19;
	SP_mcbutton(mcbutton);
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n", mccount, mctarget, mcmessage, origin_number, origin_number2, origin_number3));
	return;
}
/*
=================
svcmd_addmcspeed
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mcspeed( gentity_t *ent );
void	svcmd_addmcspeed2( gentity_t *ent ) {
	gentity_t *mcspeed = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcspeed <newspeed> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcspeed->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mcspeed );
	mcspeed->s.origin[0] = 0;
	mcspeed->s.origin[1] = 0;
	mcspeed->s.origin[2] = 0;
	SP_target_mcspeed(mcspeed);
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mccount, mctargetname));
	return;
}
/*
=================
svcmd_addmcgravity
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mcgravity( gentity_t *ent );
void	svcmd_addmcgravity2( gentity_t *ent ) {
	gentity_t *mcgravity = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcgravity <newgravity> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcgravity->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mcgravity );
	mcgravity->s.origin[0] = 0;
	mcgravity->s.origin[1] = 0;
	mcgravity->s.origin[2] = 0;
	SP_target_mcgravity(mcgravity);
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mccount, mctargetname));
	return;
}
/*
=================
svcmd_addmccredits
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mccredits( gentity_t *ent );
void	svcmd_addmccredits2( gentity_t *ent ) {
	gentity_t *mccredits = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmccredits <newcredits> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mccredits->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mccredits );
	mccredits->s.origin[0] = 0;
	mccredits->s.origin[1] = 0;
	mccredits->s.origin[2] = 0;
	SP_target_mccredits(mccredits);
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mccount, mctargetname));
	return;
}
/*
=================
svcmd_addmcchat
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mcchat( gentity_t *ent );
void	svcmd_addmcchat2( gentity_t *ent ) {
	gentity_t *mcchat = G_Spawn();
	char   mcmessage[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcchat <message> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mcmessage, sizeof( mcmessage ) );
	G_ParseField( "message", mcmessage, mcchat );
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mcchat );
	mcchat->s.origin[0] = 0;
	mcchat->s.origin[1] = 0;
	mcchat->s.origin[2] = 0;
	SP_target_mcchat(mcchat);
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mcmessage, mctargetname));
	return;
}
/*
=================
svcmd_addmcbutton
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_mcbutton( gentity_t *ent );
void	svcmd_addmcbutton3( gentity_t *ent ) {
	gentity_t *mcbutton = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctarget[MAX_STRING_CHARS];
	char   mcmessage[MAX_STRING_CHARS];
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	fileHandle_t	f;
	char		line[512];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 7 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcbutton <cost> <target> <message> <x> <y> <z>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcbutton->count = atoi(mccount);
	trap_Argv( 2, mctarget, 1024 );
	G_ParseField( "target", mctarget, mcbutton );
	trap_Argv( 3, mcmessage, 1024 );
	G_ParseField( "message", mcmessage, mcbutton );
	trap_Argv( 4, origin_number, 1024 );
	mcbutton->s.origin[0] = atoi(origin_number);
	trap_Argv( 5, origin_number2, 1024 );
	mcbutton->s.origin[1] = atoi(origin_number2);
	trap_Argv( 6, origin_number3, 1024 );
	mcbutton->s.origin[2] = atoi(origin_number3) - 19;
	SP_mcbutton(mcbutton);
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmcbutton \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n", mccount, mctarget, mcmessage, origin_number, origin_number2, origin_number3);
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: %s %s %s %s %s %s\n", mccount, mctarget, mcmessage, origin_number, origin_number2, origin_number3));
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	return;
}
/*
=================
svcmd_addmcspeed
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mcspeed( gentity_t *ent );
void	svcmd_addmcspeed3( gentity_t *ent ) {
	gentity_t *mcspeed = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	char   mctargetname[MAX_STRING_CHARS];
	fileHandle_t	f;
	char		line[512];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcspeed <newspeed> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcspeed->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mcspeed );
	mcspeed->s.origin[0] = 0;
	mcspeed->s.origin[1] = 0;
	mcspeed->s.origin[2] = 0;
	SP_target_mcspeed(mcspeed);
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmcspeed \"%s\" \"%s\"\n", mccount, mctargetname);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mccount, mctargetname));
	return;
}
/*
=================
svcmd_addmcgravity
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mcgravity( gentity_t *ent );
void	svcmd_addmcgravity3( gentity_t *ent ) {
	gentity_t *mcgravity = G_Spawn();
	fileHandle_t	f;
	char		line[512];
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcgravity <newgravity> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcgravity->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mcgravity );
	mcgravity->s.origin[0] = 0;
	mcgravity->s.origin[1] = 0;
	mcgravity->s.origin[2] = 0;
	SP_target_mcgravity(mcgravity);
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmcgravity \"%s\" \"%s\"\n", mccount, mctargetname);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mccount, mctargetname));
	return;
}
/*
=================
svcmd_addmccredits
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mccredits( gentity_t *ent );
void	svcmd_addmccredits3( gentity_t *ent ) {
	gentity_t *mccredits = G_Spawn();
	fileHandle_t	f;
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	char		line[512];
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmccredits <newcredits> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mccredits->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mccredits );
	mccredits->s.origin[0] = 0;
	mccredits->s.origin[1] = 0;
	mccredits->s.origin[2] = 0;
	SP_target_mccredits(mccredits);
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmccredits \"%s\" \"%s\"\n", mccount, mctargetname);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mccount, mctargetname));
	return;
}
/*
=================
svcmd_addmcchat
=================
*/
extern void AddSpawnField(char *field, char *value);
extern void SP_target_mcchat( gentity_t *ent );
void	svcmd_addmcchat3( gentity_t *ent ) {
	gentity_t *mcchat = G_Spawn();
	fileHandle_t	f;
	char		line[512];
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	char   mcmessage[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/addmcchat <message> <targetname>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, mcmessage, sizeof( mcmessage ) );
	G_ParseField( "message", mcmessage, mcchat );
	trap_Argv( 2, mctargetname, 1024 );
	G_ParseField( "targetname", mctargetname, mcchat );
	mcchat->s.origin[0] = 0;
	mcchat->s.origin[1] = 0;
	mcchat->s.origin[2] = 0;
	SP_target_mcchat(mcchat);
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmcchat \"%s\" \"%s\"\n", mcmessage, mctargetname);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	G_Printf(va("Mapedits: mcbutton spawned permanently with parameters of: \"%s\" \"%s\"\n",mcmessage, mctargetname));
	return;
}
void mcspawnent(gentity_t *ent)
{
	char	type[MAX_STRING_CHARS];
	char	iBuffer[MAX_STRING_CHARS];
	char	iXi[MAX_STRING_CHARS];
	char	iYi[MAX_STRING_CHARS];
	char	iZi[MAX_STRING_CHARS];
	char	iA1i[MAX_STRING_CHARS];
	char	iA2i[MAX_STRING_CHARS];
	char	iA3i[MAX_STRING_CHARS];
	char	spfstring[MAX_STRING_CHARS];
	char	myval4[MAX_STRING_CHARS];
	char	current_type[MAX_STRING_CHARS];
	char	current_value[MAX_STRING_CHARS];
	char	teststr1[MAX_STRING_CHARS];
	int	iStringPos;
	int	iStringPart;
	int	i;
	int	c;
	int	o;
	int	g;
	int	fixorg;
	int	needrespawn;
	gentity_t	*newent = G_Spawn();
	fixorg = 0;
	if (trap_Argc() < 5)
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/ammap_place <classname> <x> <y> <z> <spawnstring>\n\"" ) );
		return;
	}
	stringclear(iBuffer,MAX_STRING_CHARS-2);
	stringclear(type,MAX_STRING_CHARS-2);
	stringclear(iXi,MAX_STRING_CHARS-2);
	stringclear(iYi,MAX_STRING_CHARS-2);
	stringclear(iZi,MAX_STRING_CHARS-2);
	stringclear(iA1i,MAX_STRING_CHARS-2);
	stringclear(iA2i,MAX_STRING_CHARS-2);
	stringclear(iA3i,MAX_STRING_CHARS-2);
	stringclear(spfstring,MAX_STRING_CHARS-2);
	stringclear(myval4,MAX_STRING_CHARS-2);
	stringclear(current_type,MAX_STRING_CHARS-2);
	stringclear(current_value,MAX_STRING_CHARS-2);
	stringclear(teststr1,MAX_STRING_CHARS-2);
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
	trap_Argv(1, type, sizeof(type));
	trap_Argv(2, iXi, sizeof(iXi));
	trap_Argv(3, iYi, sizeof(iYi));
	trap_Argv(4, iZi, sizeof(iZi));
	trap_Argv(5, spfstring, sizeof(spfstring));
	trap_Argv(6, iA1i, sizeof(iA1i));
	trap_Argv(7, iA2i, sizeof(iA2i));
	trap_Argv(8, iA3i, sizeof(iA3i));
	newent->s.origin[0] = monkeyspot(iXi, 0, ent, ent);//atoi(iXi);
	newent->s.origin[1] = monkeyspot(iYi, 1, ent, ent);//atoi(iYi);
	newent->s.origin[2] = monkeyspot(iZi, 2, ent, ent);//atoi(iZi);
	VectorCopy(ent->s.origin, ent->origOrigin);
	newent->s.angles[YAW] = atoi(iA1i);
	newent->s.angles[PITCH] = atoi(iA2i);
	newent->s.angles[ROLL] = atoi(iA2i);
	G_ParseField( "classname", type, newent );
	AddSpawnField("classname", type);
	//G_CallSpawn(newent);
	G_SetAngles(newent,newent->s.angles);
	//G_SetOrigin(newent,newent->s.origin);
	needrespawn = 0;
	trap_LinkEntity(newent);
	Com_sprintf( iBuffer, sizeof(iBuffer), "");
	//Com_sprintf( iBuffer, sizeof(iBuffer), "Spawned new %s : %i", type, newent->s.number);
	iStringPos = 0;
	iStringPart = 0;
	i = 0;
	c = 0;
	o = 0;
	g = 0;
	newent->custom = 1;
	if (ent->s.number > 31)
	{
		newent->issaved = 1;
	}
	for ( iStringPos = 0; iStringPos <= strlen (spfstring); iStringPos++ )
	{
		if ((spfstring[iStringPos] == ','))
		{
			if (iStringPart == 0)
			{
				iStringPart = 1;
				//pnum = 0;
			}
			else if (iStringPart == 1)
			{
				iStringPart = 0;
				//G_ParseField( current_type, current_value, newent );
				//AddSpawnField(current_type, current_value);
				if (Q_stricmp( current_type, "angle" ) == 0)
				{
					newent->s.angles[YAW] = atoi(current_value);
					newent->s.angles[PITCH] = 0;
					newent->s.angles[ROLL] = 0;
					//needrespawn = 1;
					G_SetAngles(newent,newent->s.angles);
				}
				if (Q_stricmp( current_type, "originfix") == 0)
				{
					G_SetOrigin(newent, newent->s.origin);
					fixorg = 1;
				}
				if (Q_stricmp( current_type, "angles" ) == 0)
				{
				for ( i = 0; i <= strlen (current_value); i++ )
				{
					if ((current_value[i] == ' '))
					{
						c++;
						if ( c == 1 )
							memcpy (iA1i, current_value, i);
						else if ( c == 2 )
							memcpy (iA2i, current_value+o+1, i-o-1);
						else if ( c == 3 )
							memcpy (iA3i, current_value+o+1, i-o-1);
						o = i;
					}
				}
					newent->s.angles[YAW] = atoi(iA1i);
					newent->s.angles[PITCH] = atoi(iA2i);
					newent->s.angles[ROLL] = atoi(iA3i);
					//needrespawn = 1;
					G_SetAngles(newent,newent->s.angles);
				}
				Com_sprintf( iBuffer, sizeof(iBuffer), "%s ('%s' = '%s')", iBuffer, current_type, current_value);
				AddSpawnField(current_type, current_value);
				G_ParseField( current_type, current_value, newent );
				Com_sprintf( current_type, sizeof(current_type), "");
				Com_sprintf( current_value, sizeof(current_value), "");
			}
		}
		else if (iStringPart == 0)
		{
			teststr1[0] = spfstring[iStringPos];
			Com_sprintf( current_type, sizeof(current_type), "%s%s", current_type, teststr1);
		}
		else if (iStringPart == 1)
		{
			teststr1[0] = spfstring[iStringPos];
			Com_sprintf( current_value, sizeof(current_value), "%s%s", current_value, teststr1);
			//Com_sprintf( current_value, sizeof(current_value), "%s%s", current_value, spfstring[iStringPos]);
		}
	}
	G_CallSpawn(newent);
	if (fixorg == 1)
	{
		newent->s.origin[0] = monkeyspot(iXi, 0, ent, ent);//atoi(iXi);
		newent->s.origin[1] = monkeyspot(iYi, 1, ent, ent);//atoi(iYi);
		newent->s.origin[2] = monkeyspot(iZi, 2, ent, ent);//atoi(iZi);
		G_SetOrigin(newent, newent->s.origin);
		trap_LinkEntity(newent);
	}
	if (needrespawn == 1)
	{
		needrespawn = ent_respawn(newent);
		//trap_SendServerCmd( ent-g_entities, va("print \"^3Ignore following message. The correct entity number is ^5%i^3.\n\"", needrespawn ) );
		trap_SendServerCmd(ent-g_entities,  va("print \"^7Spawned new ^5%s^7 : ^5%i^7\"", type, needrespawn));
	}
	else
	{
		trap_SendServerCmd(ent-g_entities,  va("print \"^7Spawned new ^5%s^7 : ^5%i^7\"", type, newent->s.number));
	}
	trap_SendServerCmd( ent-g_entities, va("print \"^7%s\n\"", iBuffer ) );
	G_ParseField( "classname", type, newent );
}
void	mcspawnent3( gentity_t *ent ) {
	char			savePath[MAX_QPATH];
	char			arg0[MAX_STRING_CHARS];
	char			arg1[MAX_STRING_CHARS];
	char			arg2[MAX_STRING_CHARS];
	char			arg3[MAX_STRING_CHARS];
	char			arg4[MAX_STRING_CHARS];
	char			arg5[MAX_STRING_CHARS];
	char			arg6[MAX_STRING_CHARS];
	char			arg7[MAX_STRING_CHARS];
	char			arg8[MAX_STRING_CHARS];
	char			arg9[MAX_STRING_CHARS];
	vmCvar_t		mapname;
	fileHandle_t	f;
	char		line[2048];
	int		i;
	int		len;
	if (trap_Argc() < 5)
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^7^1/ammap_place2 <classname> <x> <y> <z> <spawnstring>^7\n\"" ) );
		return;
	}
	trap_Argv( 1, arg0, sizeof( arg0 ) );
	trap_Argv( 2, arg1, sizeof( arg1 ) );
	trap_Argv( 3, arg2, sizeof( arg2 ) );
	trap_Argv( 4, arg3, sizeof( arg3 ) );
	trap_Argv( 5, arg4, sizeof( arg4 ) );
	trap_Argv( 6, arg5, sizeof( arg5 ) );
	trap_Argv( 7, arg6, sizeof( arg6 ) );
	trap_Argv( 8, arg7, sizeof( arg7 ) );
	trap_Argv( 9, arg8, sizeof( arg8 ) );
	trap_Argv( 10, arg9, sizeof( arg9 ) );
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "mapedits/mapedits_1_%s.cfg", mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	if (Q_stricmp( arg7, "" ) == 0)
	{
		Com_sprintf( line, sizeof(line), "addanyent \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n", arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	}
	else
	{
		Com_sprintf( line, sizeof(line), "addanyent \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	}
	len = strlen(line);
	for (i = 0;i < len;i += 1)
	{
		if (line[i] == ';')
		{
			line[i] = ' ';
		}
		if (line[i] == '\n')
		{
			line[i] = ' ';
		}
	}
	Com_sprintf(line, sizeof(line), va("%s\n", line));
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	if (Q_stricmp( arg2, "nospawn" ) == 0)
	{
		return;
	}
	else
	{
		mcspawnent(ent);
		return;
	}
	return;
}
/*
gentity_t *g_mcfind (gentity_t *from, gentity_t *match, gentity_t *activator)
{
	char	match2[MAX_STRING_CHARS];
	char	match3[MAX_STRING_CHARS];
	char	match4[MAX_STRING_CHARS];
	strcpy(match2, match->mctarget);
	if (!from)
		from = g_entities;
	else
		from++;


	for ( ; from < &g_entities[level.num_entities] ; from++)
	{
		if (from->inuse)
		{
			if (from->mctargetname)
			{
				strcpy(match3, from->mctargetname);
				if ((match3 != NULL) && (match3 != "")){
					if (Q_stricmp( match3, match2 ) == 0)
					{
						return from;
					}
				}
			}
			if (from->targetname)
			{
				strcpy(match4, from->targetname);
				if ((match4 != NULL) && (match4 != ""))
				{
					if (Q_stricmp( match4, match2 ) == 0)
					{
						return from;
					}
				}
			}
		}
	}

	return NULL;
}

void g_usemctargets( gentity_t *ent, gentity_t *activator ) {
	gentity_t		*t;

	if ( !ent ) {
		return;
	}
	if ( !ent->mctarget ) {
		return;
	}

	t = NULL;
	while ( (t = g_mcfind(t, ent, activator)) != NULL ) {
		if ( t == ent ) {
			G_Printf ("WARNING: Entity used itself.\n");
		} else {
			if ( t->use ) {
				t->use (t, ent, activator);
			}
		}
		if ( !ent->inuse ) {
			G_Printf("entity was removed while using targets\n");
			return;
		}
	}
}
*/
extern void Touch_Button(gentity_t *ent, gentity_t *other, trace_t *trace );
void mcusetarget( gentity_t *ent )
{
	gentity_t	*target;
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;

	VectorCopy(ent->client->ps.origin, viewspot);
	viewspot[2] += ent->client->ps.viewheight;

	VectorCopy( viewspot, src );
	AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );

	VectorMA( src, 64, vf, dest );

	//Trace ahead to find a valid target
	trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );

	if ( trace.fraction == 1.0f || trace.entityNum < 1 )
	{
		return;
	}

	target = &g_entities[trace.entityNum];

	//Check for a use command
	if ( ValidUseTarget( target ) )
	{
		//target->use(target, ent, ent);
		if (target->count == 0)
		{
			//g_usemctargets(target, ent);
			G_UseTargets (target, ent);
		}
		if (target->count > 0)
		{
			if (ent->client->sess.credits >= target->count)
			{
				//g_usemctargets(target, ent);
				G_UseTargets (target, ent);
				ent->client->sess.credits -= target->count;
			}
			else
			{
				trap_SendServerCmd( ent->s.number, va("print \"^3Not enough credits.\n\"" ) );
			}
		}
		dsp_doEmote(ent, 1037);
		return;
	}
}
int find_ent(gentity_t *ent, char *entname)
{
	int			entnum;
	gentity_t	*target;
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;
	gentity_t	*entfinal;
	if (Q_stricmp(entname, "") == 0)
	{
		return -14;
	}
	else if (Q_stricmp(entname, "gun") == 0)
	{
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;//ent->client->ps.viewheight;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;/*return trace.entityNum*/;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "widegun") == 0)
	{
		vec3_t	mins, maxs;
		mins[0] = -15;
		mins[1] = -15;
		mins[2] = -15;
		maxs[0] = 15;
		maxs[1] = 15;
		maxs[2] = 15;
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;//ent->client->ps.viewheight;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, mins, maxs, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;/*return trace.entityNum*/;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "widegun2") == 0)
	{
		vec3_t	mins, maxs;
		mins[0] = -25;
		mins[1] = -25;
		mins[2] = -25;
		maxs[0] = 25;
		maxs[1] = 25;
		maxs[2] = 25;
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;//ent->client->ps.viewheight;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, mins, maxs, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;/*return trace.entityNum*/;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "widefullgun") == 0)
	{
		vec3_t	mins, maxs;
		mins[0] = -15;
		mins[1] = -15;
		mins[2] = -15;
		maxs[0] = 15;
		maxs[1] = 15;
		maxs[2] = 15;
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;//ent->client->ps.viewheight;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, mins, maxs, dest, ent->s.number, CONTENTS_ABSEIL|CONTENTS_TRIGGER|MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;/*return trace.entityNum*/;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "widefullgun2") == 0)
	{
		vec3_t	mins, maxs;
		mins[0] = -25;
		mins[1] = -25;
		mins[2] = -25;
		maxs[0] = 25;
		maxs[1] = 25;
		maxs[2] = 25;
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;//ent->client->ps.viewheight;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, mins, maxs, dest, ent->s.number, CONTENTS_ABSEIL|CONTENTS_TRIGGER|MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;/*return trace.entityNum*/;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname,"lastent") == 0)
	{
		entnum = ent->client->sess.lastent;
		goto lastcheck;
	}
	else if (strstr(entname,"byclass_"))
	{
		return -5;
	}
	else if (strstr(entname,"bytarget_"))
	{
		return -63;
	}
	else if (strstr(entname,"bygroup_"))
	{
		return -6;
	}
	else if (strstr(entname,"area_"))
	{
		return -12;
	}
	else if (Q_stricmp(entname,"all") == 0)
	{
		return -42;
	}
	else if (Q_stricmp(entname,"custom") == 0)
	{
		return -43;
	}
	else if ((strstr(entname,"realentnum"))&&(ent->client->sess.adminloggedin > 0)&&(ent->client->sess.ampowers7 & 256))
	{
		return atoi(entname);
	}
	else if (Q_stricmp(entname, "triggergun") == 0)
	{
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, CONTENTS_TRIGGER|MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "fullgun") == 0)
	{
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, CONTENTS_ABSEIL|CONTENTS_TRIGGER|MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "fxgun") == 0)
	{
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += 35;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, CONTENTS_ABSEIL|MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if ( trace.fraction == 1.0f || trace.entityNum < 1 )
		{
			return -2;
		}
		//target = &g_entities[trace.entityNum];
		entfinal = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum > 31)
			{
				if (entfinal->inuse)
				{
					entnum = trace.entityNum;
					goto lastcheck;
				}
				else
				{
					return -2;
				}
			}
			else
			{
				return -2;
			}
		//}
	}
	else if (Q_stricmp(entname, "") == 0)
	{
		return -14;
	}
	else if (Q_stricmp(entname,"traced") == 0)
	{
		entnum = ent->client->sess.traced;
		goto lastcheck;
	}
	else
	{
		entnum = atoi(entname);
		if (entnum == 0)
		{
			return -14;
		}
		if (entnum >= 1020)
		{
			return -14;
		}
		entfinal = &g_entities[entnum];
		if (entfinal != NULL)
		{
			if (entnum > 31)
			{
				if (entfinal->inuse)
				{
					//if (!ent->client)
					//{
						goto lastcheck;
					//}
				}
			}
		}
		else
		{
			return -1;
		}
	}
	lastcheck:
	if (entnum >= 1020)
	{
		return -14;
	}
	if (entnum < 32)
	{
		return -14;
	}
	entfinal = &g_entities[entnum];
	if (entfinal->inuse)
	{
		if (entfinal->s.number > 31)
		{
			if (Q_stricmp(entfinal->classname,"player") != 0)
			{
				if (Q_stricmp(entfinal->classname,"lightsaber") != 0)
				{
					if (entfinal->rprotected)
					{
						return -4;
					}
					ent->client->sess.lastent = entnum;
					return entnum;
				}
			}
		}
		return -3;
	}
	return -1;
}
void mc_pay(gentity_t *ent)
{
	int			entnum;
	gentity_t	*target;
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;
	gentity_t	*entfinal;
	char		par1[MAX_STRING_CHARS];
	trap_Argv( 1, par1, sizeof( par1 ) );
	if (ent->client->sess.credits <= 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You do not have any credits.\n\""));
		return;
	}
	if (atoi(par1) == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You can not give someone nothing.\n\""));
		return;
	}
	if (atoi(par1) < 0)
	{
		//trap_SendServerCmd(ent->fixednum, va("print \"^1Trying to give negative credits, really? I prefer positive.\n\""));
		strcpy(par1,va("%i",abs(atoi(par1))));
	}
	if (abs(atoi(par1)) > ent->client->sess.credits)
	{
		//trap_SendServerCmd(ent->fixednum, va("print \"^1All yer credits r belong to him!\n\""));
		strcpy(par1,va("%i",ent->client->sess.credits));
	}
		VectorCopy(ent->client->ps.origin, viewspot);
		viewspot[2] += ent->client->ps.viewheight;
		VectorCopy( viewspot, src );
		AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
		VectorMA( src, 8192, vf, dest );
		trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
		if (trace.entityNum == ENTITYNUM_WORLD )
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1Not targeting an entity.\n\""));
			return;
		}
		entfinal = &g_entities[trace.entityNum];
		if ( trace.fraction == 1.0f || trace.entityNum < 0 || ((!entfinal->payable)&&(!entfinal->client)))
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1The targeted entity is not payable.\n\""));
			return;
		}
		//target = &g_entities[trace.entityNum];
		//return (g_entities-target);
		//if (!entfinal->client)
		//{
			if (trace.entityNum < 31)
			{
				if (entfinal->inuse)
				{
					if (entfinal->client)
					{
						if (entfinal->r.svFlags & SVF_BOT)
						{
							trap_SendServerCmd(ent->fixednum, va("print \"^1Cannot pay a bot.\n\""));
							return;
						}
						if (Q_stricmp(entfinal->client->sess.userlogged,"") == 0)
						{
							trap_SendServerCmd(ent->fixednum, va("print \"^1Targeted player is not logged in.\n\""));
							return;
						}
						mc_addcredits(entfinal, atoi(par1));
						trap_SendServerCmd(ent->fixednum, va("print \"^3You have payed ^7%s^3 ^5%i^3 credits.\n\"", entfinal->client->pers.netname, atoi(par1)));
						trap_SendServerCmd(entfinal->s.number, va("print \"^7%s^3 has given you ^5%i^3 credits.\n\"", ent->client->pers.netname, atoi(par1)));
						mc_addcredits(ent, -atoi(par1));
						return;
					}
				}
			}
		//}
		if (entfinal->payable)
		{
			if (atoi(par1) == entfinal->count)
			{				mc_addcredits(ent, -atoi(par1));
				G_UseTargets (entfinal, ent);
				return;
			}
			else if (atoi(par1) > entfinal->count)
			{
				//trap_SendServerCmd(entfinal->s.number, va("print \"^3Targeted ent has been overpayed.\n\""));
				mc_addcredits(ent, -atoi(par1));
				G_UseTargets (entfinal, ent);
				return;
			}
			else if (atoi(par1) < entfinal->count)
			{
				trap_SendServerCmd(entfinal->s.number, va("print \"^3This object is priced ^5%i^3.\n\"", entfinal->count));
				//mc_addcredits(ent, -atoi(par1));
				return;
			}
		}
		else
		{
			trap_SendServerCmd(entfinal->s.number, va("print \"^3Targeted ent is inanimate.\n\""));
		}
	return;
}
void	Svcmd_EntityList_f2(gentity_t *ent) {
	int			e;
	int			i_entcount;
	char			i_startcount[MAX_STRING_CHARS];
	int			i_startcount2;
	gentity_t		*check;
	i_entcount = 0;
	if (trap_Argc() > 1)
	{
		trap_Argv( 1, i_startcount, sizeof( i_startcount ) );
		i_startcount2 = atoi(i_startcount);
	}
	else
	{
		i_startcount2 = 1;
	}
	if (i_startcount2 > level.num_entities)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^3Start Number too high.\n\"" ) );
	}

	check = g_entities+i_startcount2;
	for (e = i_startcount2; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		trap_SendServerCmd( ent->s.number, va("print \"%3i:\"", e));
		switch ( check->s.eType ) {
		case ET_GENERAL:
			trap_SendServerCmd( ent->s.number, va("print \"ET_GENERAL          \"" ) );
			break;
		case ET_PLAYER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_PLAYER           \"" ) );
			break;
		case ET_ITEM:
			trap_SendServerCmd( ent->s.number, va("print \"ET_ITEM             \"" ) );
			break;
		case ET_MISSILE:
			trap_SendServerCmd( ent->s.number, va("print \"ET_MISSILE          \"" ) );
			break;
		case ET_MOVER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_MOVER            \"" ) );
			break;
		case ET_BEAM:
			trap_SendServerCmd( ent->s.number, va("print \"ET_BEAM             \"" ) );
			break;
		case ET_PORTAL:
			trap_SendServerCmd( ent->s.number, va("print \"ET_PORTAL           \"" ) );
			break;
		case ET_SPEAKER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_SPEAKER          \"" ) );
			break;
		case ET_PUSH_TRIGGER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_PUSH_TRIGGER     \"" ) );
			break;
		case ET_TELEPORT_TRIGGER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_TELEPORT_TRIGGER \"" ) );
			break;
		case ET_INVISIBLE:
			trap_SendServerCmd( ent->s.number, va("print \"ET_INVISIBLE        \"" ) );
			break;
		case ET_GRAPPLE:
			trap_SendServerCmd( ent->s.number, va("print \"ET_GRAPPLE          \"" ) );
			break;
		default:
			trap_SendServerCmd( ent->s.number, va("print \"%3i                 \"", check->s.eType));
			break;
		}

		if ( check->classname ) {
			trap_SendServerCmd( ent->s.number, va("print \"%s\"", check->classname));
		}
		trap_SendServerCmd( ent->s.number, va("print \"\n\"" ) );;
		i_entcount += 1;
		if (i_entcount > 24)
		{
			return;
		}
	}
}
void	Svcmd_EntityList_nearby(gentity_t *ent) {
	int			e;
	int			i_entcount;
	char			i_startcount[MAX_STRING_CHARS];
	int			i_startcount2;
	gentity_t		*check;
	int			i;
	vec3_t			org2;
	int			count;
	gentity_t		*entity_list[MAX_GENTITIES];
	i_entcount = 0;
	if (trap_Argc() > 1)
	{
		trap_Argv( 1, i_startcount, sizeof( i_startcount ) );
		i_startcount2 = atoi(i_startcount);
	}
	else
	{
		i_startcount2 = 400;
	}
		//G_Printf("Showing ent list ");
	VectorClear(org2);
	VectorCopy(ent->client->ps.origin, org2);
	check = g_entities/*+i_startcount2*/;
	for (e = 0/*i_startcount2*/; e < level.num_entities ; e++, check++)// {
	//count = G_RadiusList( org2, i_startcount2, NULL, qtrue, &g_entities/*entity_list*/ );
	//for ( i = 0; i < count; i++ )
	{
		float			dist;
		vec3_t			angles;
		//check = /*entity_list*/&g_entities[i];
		//G_Printf("newent");
		if ( !check->inuse ) {
			continue;
		}
		if ((Q_stricmp(check->classname,"player") == 0)||(Q_stricmp(check->classname,"lightsaber") == 0))
		{
		    continue;
		}
		//G_Printf("Addent");
		VectorSubtract( check->r.currentOrigin, ent->client->ps.origin, angles );
		dist = VectorLengthSquared ( angles );
		//vectoangles ( angles, angles );

		if ( dist > i_startcount2*i_startcount2 )
		{
			continue;
		}
		trap_SendServerCmd( ent->s.number, va("print \"%3i:(%i,%i,%i)\"", e, (int)check->r.currentOrigin[0], (int)check->r.currentOrigin[1], (int)check->r.currentOrigin[2]));
		switch ( check->s.eType ) {
		case ET_GENERAL:
			trap_SendServerCmd( ent->s.number, va("print \"ET_GENERAL          \"" ) );
			break;
		case ET_PLAYER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_PLAYER           \"" ) );
			break;
		case ET_ITEM:
			trap_SendServerCmd( ent->s.number, va("print \"ET_ITEM             \"" ) );
			break;
		case ET_MISSILE:
			trap_SendServerCmd( ent->s.number, va("print \"ET_MISSILE          \"" ) );
			break;
		case ET_MOVER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_MOVER            \"" ) );
			break;
		case ET_BEAM:
			trap_SendServerCmd( ent->s.number, va("print \"ET_BEAM             \"" ) );
			break;
		case ET_PORTAL:
			trap_SendServerCmd( ent->s.number, va("print \"ET_PORTAL           \"" ) );
			break;
		case ET_SPEAKER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_SPEAKER          \"" ) );
			break;
		case ET_PUSH_TRIGGER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_PUSH_TRIGGER     \"" ) );
			break;
		case ET_TELEPORT_TRIGGER:
			trap_SendServerCmd( ent->s.number, va("print \"ET_TELEPORT_TRIGGER \"" ) );
			break;
		case ET_INVISIBLE:
			trap_SendServerCmd( ent->s.number, va("print \"ET_INVISIBLE        \"" ) );
			break;
		case ET_GRAPPLE:
			trap_SendServerCmd( ent->s.number, va("print \"ET_GRAPPLE          \"" ) );
			break;
		default:
			trap_SendServerCmd( ent->s.number, va("print \"%3i                 \"", check->s.eType));
			break;
		}

		if ( check->classname ) {
			trap_SendServerCmd( ent->s.number, va("print \"%s\"", check->classname));
		}
		trap_SendServerCmd( ent->s.number, va("print \"\n\"" ) );;
		i_entcount += 1;
		//G_Printf("Success");
		if (i_entcount > 24)
		{
		//G_Printf("no more ents\n");
			return;
		}
	}
	if (i_entcount == 0)
	{
	    trap_SendServerCmd(ent->fixednum, va("print \"^3No ents found.\n\""));
	}
		//G_Printf("Out of ents\n");
}
void DS_doHug( gentity_t *ent )
{
    trace_t tr;
    vec3_t fPos;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		// || ent->client->ps.forceRestricted
		// || ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		// || ent->client->ps.saberInFlight
		)
		return;

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}/*
	if (twimod_emotebreak.integer == 0)
	{
		if ((ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_TALKCOMM1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 655 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_BARTENDER_COWERLOOP &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1049 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 708 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != TORSO_SURRENDER_START &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 641 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND4 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 715 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 804 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 595 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 645)
		{
			return;
		}
	}*/

	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
	fPos[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + fPos[2]*40;

	trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

	if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
	{
		gentity_t *other = &g_entities[tr.entityNum];
 		if (other && other->inuse && other->client)
		{
			vec3_t entDir;
			vec3_t otherDir;
			vec3_t entAngles;
			vec3_t otherAngles;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		//|| ent->client->ps.forceRestricted
		//|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		//|| ent->client->ps.saberInFlight
		)
				return;

			if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
			if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered) Cmd_ToggleSaber_f(other);

			// If for some reason they dont turn of don't try to hug the target.
			/*if ((ent->client->ps.weapon == WP_SABER && other->client->ps.weapon == WP_SABER) && (ent->client->ps.saberHolstered && other->client->ps.saberHolstered))*/
			//{
				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );
				StandardSetBodyAnim(ent, BOTH_HUGGER1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
		                ent->client->ps.saberMove = LS_NONE;
		                ent->client->ps.saberBlocked = 0;
		                ent->client->ps.saberBlocking = 0;

		                VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
		                VectorCopy(other->client->ps.viewangles, otherAngles );
		                otherAngles[YAW] = vectoyaw( entDir );
		                SetClientViewAngle( other, otherAngles );

		                StandardSetBodyAnim(other, BOTH_HUGGEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);

		                other->client->ps.saberMove = LS_NONE;
		                other->client->ps.saberBlocked = 0;
		                other->client->ps.saberBlocking = 0;
			//}
		}
	}
}

/*
void DS_doHump( gentity_t *ent )
{
    trace_t tr;
    vec3_t fPos;
	int	hoek;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		// || ent->client->ps.forceRestricted
		// || ent->client->ps.duelInProgress
		// || BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		// || ent->client->ps.saberInFlight
		)
		return;

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}
	if (twimod_emotebreak.integer == 0)
	{
		if ((ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_TALKCOMM1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 655 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_BARTENDER_COWERLOOP &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1049 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 708 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != TORSO_SURRENDER_START &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 641 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND4 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 715 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 804 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 595 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 645)
		{
			return;
		}
	}

	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
	fPos[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + fPos[2]*40;

	trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

	if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
	{
		gentity_t *other = &g_entities[tr.entityNum];
 		if (other && other->inuse && other->client)
		{
			vec3_t entDir;
			vec3_t otherDir;
			vec3_t entAngles;
			vec3_t otherAngles;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		//|| ent->client->ps.forceRestricted
		//|| ent->client->ps.duelInProgress
		//|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		//|| ent->client->ps.saberInFlight
		)
				return;

			if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
			if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered) Cmd_ToggleSaber_f(other);

			// If for some reason they dont turn of don't try to hug the target.
			//if ((ent->client->ps.weapon == WP_SABER && other->client->ps.weapon == WP_SABER) && (ent->client->ps.saberHolstered && other->client->ps.saberHolstered))
			//{
				other->client->sess.movebackX = other->client->ps.origin[0];
				other->client->sess.movebackY = other->client->ps.origin[1];
				other->client->sess.movebackZ = other->client->ps.origin[2];
				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );
				StandardSetBodyAnim(ent, BOTH_HUGGER1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
		                ent->client->ps.saberMove = LS_NONE;
		                ent->client->ps.saberBlocked = 0;
		                ent->client->ps.saberBlocking = 0;

		                VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
		                VectorCopy(other->client->ps.viewangles, otherAngles );
		                otherAngles[YAW] = vectoyaw( entDir );
				if (otherAngles[YAW] > 180)
				{
					otherAngles[YAW] -= 180;
				}
				else
				{
					otherAngles[YAW] += 180;
				}
				hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
				other->client->ps.origin[0] = ent->client->ps.origin[0] + ( cos(hoek) * 20 ); // X
				other->client->ps.origin[1] = ent->client->ps.origin[1] + ( sin(hoek) * 20 ); // Y
				VectorCopy( other->client->ps.origin, other->r.currentOrigin );
				other->client->sess.movebacktime = level.time + 2000;
		                SetClientViewAngle( other, otherAngles );

		                StandardSetBodyAnim(other, BOTH_HUGGEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);

		                other->client->ps.saberMove = LS_NONE;
		                other->client->ps.saberBlocked = 0;
		                other->client->ps.saberBlocking = 0;
			//}
		}
	}
}
*/
void DS_doKiss( gentity_t *ent )
{
	trace_t tr;
	vec3_t fPos;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		//|| ent->client->ps.forceRestricted
		//|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		//|| ent->client->ps.saberInFlight
		)
		return;

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}

	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
	fPos[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + fPos[2]*40;

	trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

	if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
	{
		gentity_t *other = &g_entities[tr.entityNum];

		if (other && other->inuse && other->client && !other->client->ps.duelInProgress)
		{
			vec3_t entDir;
			vec3_t otherDir;
			vec3_t entAngles;
			vec3_t otherAngles;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		//|| ent->client->ps.forceRestricted
		//|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		//|| ent->client->ps.saberInFlight
		)
				return;

			if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
			if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered) Cmd_ToggleSaber_f(other);

			/*if ((ent->client->ps.weapon == WP_SABER && other->client->ps.weapon == WP_SABER) && (ent->client->ps.saberHolstered && other->client->ps.saberHolstered))
			{*/
				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );
				StandardSetBodyAnim(ent, BOTH_KISSER1LOOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				ent->client->ps.saberMove = LS_NONE;
				ent->client->ps.saberBlocked = 0;
				ent->client->ps.saberBlocking = 0;
				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );

				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );
				StandardSetBodyAnim(other, BOTH_KISSEE1LOOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				other->client->ps.saberMove = LS_NONE;
				other->client->ps.saberBlocked = 0;
				other->client->ps.saberBlocking = 0;
			//}
		}
	}
}

void DS_doPunch( gentity_t *ent )
{
	trace_t tr;
	vec3_t fPos;
	int	quickrand;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		//|| ent->client->ps.forceRestricted
		//|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		//|| ent->client->ps.saberInFlight
		)
		return;

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}

	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
	fPos[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + fPos[2]*40;

	trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

	if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
	{
		gentity_t *other = &g_entities[tr.entityNum];

		if (other && other->inuse && other->client && !other->client->ps.duelInProgress)
		{
			vec3_t entDir;
			vec3_t otherDir;
			vec3_t entAngles;
			vec3_t otherAngles;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		//|| ent->client->ps.forceRestricted
		//|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		//|| ent->client->ps.saberInFlight
		)
				return;

			if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
			if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered) Cmd_ToggleSaber_f(other);

			/*if ((ent->client->ps.weapon == WP_SABER && other->client->ps.weapon == WP_SABER) && (ent->client->ps.saberHolstered && other->client->ps.saberHolstered))
			{*/
				quickrand = irand(1, 3);
				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );
				StandardSetBodyAnim(ent, (quickrand == 2)?130:131, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				ent->client->ps.saberMove = LS_NONE;
				ent->client->ps.saberBlocked = 0;
				ent->client->ps.saberBlocking = 0;
				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
				G_Damage( other, ent, ent, ent->client->ps.viewangles, ent->client->ps.viewangles, (quickrand==2)?1:2, DAMAGE_NO_ARMOR | DAMAGE_NO_KNOCKBACK, MOD_MELEE );
				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );
				StandardSetBodyAnim(other, (quickrand == 2)?98:545, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				other->client->ps.saberMove = LS_NONE;
				other->client->ps.saberBlocked = 0;
				other->client->ps.saberBlocking = 0;
			//}
		}
	}
}


void shadder(gentity_t *ent, int clientNum)
{
	vmCvar_t	mapname;
	fileHandle_t	f;
	char		oldshader[1024];
	int		i;
	int		iL;
	char		newshader[1024];
	char		savePath[MAX_QPATH];
	char		line[256];
	float		hornstinkt = level.time * 0.001;
		trap_Argv( 1, oldshader, sizeof( oldshader ) );
	  trap_Argv( 2, newshader, sizeof( newshader ) );


	if( strchr( oldshader, ';' ) || strchr( newshader, ';' ) || strchr( oldshader, '\n' ) || strchr( newshader, '\n' ) ) {
		trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
		return;
	}
	  if (trap_Argc() == 3)
	  	{
    AddRemap(oldshader, newshader, hornstinkt);
    trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());

    		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
			for (i = 1;i < 32;i += 1)
			{
				iL = trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_READ);
				if (!f)
				{
					trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i-1, mapname.string), &f, FS_APPEND);
					if ( !f )
					{
						trap_SendServerCmd( clientNum, va("print \"^1Error: Cannot access map file.\n\"" ) );
						return;
					}
					trap_FS_Write( va("\nmapeditsexec mapedits_%i_%s;\n", i, mapname.string), strlen(va("\nmapeditsexec mapedits_%i_%s;\n", i, mapname.string)), f);
					trap_FS_FCloseFile( f );
					goto loadmap;
				}
				trap_FS_FCloseFile(f);
				if (iL < 10000)
				{
					goto loadmap;
				}
			}
			loadmap:
			Com_sprintf(savePath, 1024*4, "%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}

				Com_sprintf( line, sizeof(line), "cshaderrcon %s %s\n", oldshader, newshader);

				trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );

  		}
		else
	  	{
	  		trap_SendServerCmd( clientNum, va("print \"^1/ammap_shader2 <oldshader> <newshader>\"" ) );
	  	}

}
void shadder2(gentity_t *ent, int clientNum)
{
	char 			oldshader[1024];
	char 			newshader[1024];
	char			savePath[MAX_QPATH];
	char			line[256];
	float hornstinkt = level.time * 0.001;
	trap_Argv( 1, oldshader, sizeof( oldshader ) );
	trap_Argv( 2, newshader, sizeof( newshader ) );
	if (trap_Argc() == 3)
	{
    		AddRemap(oldshader, newshader, hornstinkt);
    		trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
	}
	else
	{
	  	trap_SendServerCmd( clientNum, va("print \"^1/ammap_shader <oldshader> <newshader>\n\"" ) );
	}
}
void shadderdel(gentity_t *ent, int clientNum)
{
	char 			oldshader[1024];
	char 			newshader[1024];
	char			savePath[MAX_QPATH];
	char			line[256];
	float hornstinkt = level.time * 0.001;
	trap_Argv( 1, oldshader, sizeof( oldshader ) );
	trap_Argv( 2, newshader, sizeof( newshader ) );
	if (trap_Argc() == 1)
	{
	  	trap_SendServerCmd( clientNum, va("print \"^1/ammap_delshader <oldshader> <newshader>\n\"" ) );
	}
	else if (trap_Argc() == 3)
	{
    		DelRemap(oldshader);
    		trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
	}

}
void shadderawesome(gentity_t *ent, int clientNum)
{
	char 			oldshader[1024];
	char 			newshader[1024];
	char			savePath[MAX_QPATH];
	char			line[256];
	float hornstinkt = level.time * 0.001;
	trap_Argv( 1, oldshader, sizeof( oldshader ) );
	trap_Argv( 2, newshader, sizeof( newshader ) );
	if (trap_Argc() == 1)
	{
	  	trap_SendServerCmd( clientNum, va("print \"^1/ammap_shader3 <oldshader> <newshader>\n\"" ) );
	}
	else if (trap_Argc() == 3)
	{
    		AddRemap(oldshader, newshader, hornstinkt);
	}

}
void mc_timerup(void)
{
	fileHandle_t	f;
	char		timebuffer[64];
	char		timebuf2[64];
	int		i;
	int		iL;
	iL = trap_FS_FOpenFile("time.dat", &f, FS_READ);
	if (!f)
	{
		stringclear(level.leveltimestring, 1020);
		strcpy(level.leveltimestring, "Unknown Time");
	}
	trap_FS_Read( timebuffer , 64, f );
	for (i = 0;i <= 63;i += 1)
	{
		if (timebuffer[i] == '-')
		{
			break;
		}
		if (timebuffer[i] == '')
		{
			timebuf2[i] = '';
			break;
		}
		if (iL == i)
		{
			timebuf2[i] = '';
			break;
		}
		timebuf2[i] = timebuffer[i];
	}
	trap_FS_FCloseFile( f );
	stringclear(level.leveltimestring, 1020);
	strcpy(level.leveltimestring, timebuf2);
}
char *mc_timer(void)
{
	return va(level.leveltimestring);
}
void	clearedits( gentity_t *ent, int clientNum )
{
		char   arg1[MAX_STRING_CHARS];
		//gentity_t *fx_runner = G_Spawn();
		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		trap_Argv( 1,  arg1, sizeof( arg1 ) );

		// Anfang
				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/mapedits_1_%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_WRITE);

				if ( !f )
				{
					return;
				}

				Com_sprintf( line, sizeof(line), " ");
				trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
				trap_SendServerCmd( clientNum, va("print \"^1You have to restart the map.\n\"" ) );
		// Ende
}
void	getaccountinfo( gentity_t *ent, char *Saccount, char *p2)
{
	fileHandle_t	f;
	char		buffer[4096];
	char		thisisalie[1024];
	char		username[1024] = "";
	char		loginlevel[1024] = "";
	char		creds[1024] = "";
	char		mcbuildadmin[1024] = "";
	char		mcampower[1024] = "";
	char		mcampower2[1024] = "";
	char		mcampower3[1024] = "";
	char		mcampower4[1024] = "";
	char		mcampower5[1024] = "";
	char		mcampower6[1024] = "";
	char		mcampower7[1024] = "";
	char		mcamgroup[1024] = "";
	char		mctime[1024] = "";
	int i = 0;
	int o = 0;
	int c = 0;
	int g = 0;
	trap_FS_FOpenFile(va("%s/account_1_%s.cfg", mc_userfolder.string, Saccount), &f, FS_READ);
	if (!f)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1Account ~^5%s^1~ not found.\n\"", Saccount));
		return;
	}
	trap_FS_Read( buffer , 4096, f );
	for ( i = 0; i <= strlen (buffer); i++ )
	{
		if ((buffer[i] == '²')||(buffer[i] == '\n'))
		{
			c++;
			if ( c == 1 )
				memcpy (thisisalie, buffer, i);
			else if ( c == 2 )
				memcpy (loginlevel, buffer+o+1, i-o-1);
			else if ( c == 3 )
				memcpy (username, buffer+o+1, i-o-1);
			else if ( c == 4 )
				memcpy (creds, buffer+o+1, i-o-1);
			else if ( c == 5 )
				memcpy (mcampower, buffer+o+1, i-o-1);
			else if ( c == 6 )
				memcpy (mcampower2, buffer+o+1, i-o-1);
			else if ( c == 7 )
				memcpy (mcampower3, buffer+o+1, i-o-1);
			else if ( c == 8 )
				memcpy (mcampower4, buffer+o+1, i-o-1);
			else if ( c == 9 )
				memcpy (mcamgroup, buffer+o+1, i-o-1);
			else if ( c == 10 )
				memcpy (mcampower5, buffer+o+1, i-o-1);
			else if ( c == 11 )
				memcpy (mcampower6, buffer+o+1, i-o-1);
			else if ( c == 12 )
				memcpy (mcampower7, buffer+o+1, i-o-1);
			else if ( c == 13 )
				memcpy (mctime, buffer+o+1, i-o-1);
			o = i;
		}
	}
	trap_SendServerCmd(ent->fixednum, va("print \"^2Account data for ~^5%s^2~-\n^2Last Login Name: ~^5%s^2~\n\"", Saccount, username));
	trap_SendServerCmd(ent->fixednum, va("print \"^2credits: ^5%s^2\n^2Admin Rank: ^5%s^2\n^2powers1: ^5%s^2\n^2powers2: ^5%s^2\n^2powers3: ^5%s^2\"", creds, loginlevel, mcampower, mcampower2, mcampower3));
	trap_SendServerCmd(ent->fixednum, va("print \"\n^2powers4: ^5%s^2\n^2powers5: ^5%s^2\n^2powers6: ^5%s^2\n^2powers7: ^5%s^2\"", mcampower4, mcampower5, mcampower6, mcampower7));
	trap_SendServerCmd(ent->fixednum, va("print \"\n^2Groups: ^5%s^2\n^2Last Login Time: ^5%s^2\n\"", mcamgroup, mctime));
	trap_FS_FCloseFile( f );
	if (Q_stricmp(p2, "") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1/amaccountinfo <account> <admin command type>\n\""));
	}
	else
	{
		mcm_amadmin(ent, ent->s.number, p2, atoi(mcampower), atoi(mcampower2), atoi(mcampower3), atoi(mcampower4), atoi(mcampower5), atoi(mcampower6), atoi(mcampower7));
	}
}
void	mc_updateaccount( gentity_t *ent )
{
	char			MsavePath[MAX_QPATH];
	char			Mfullwrite[4096];
	char			Mline[256];
	fileHandle_t	Mz;
	if (Q_stricmp(ent->client->sess.userlogged,"") == 0)
	{
		return;
	}
	trap_FS_FOpenFile(mc_lower(va("%s/account_1_%s.cfg", mc_userfolder.string, ent->client->sess.userlogged)), &Mz, FS_WRITE);
	if ( !Mz )
	{
		return;
	}
	if (ent->client->sess.credits > 1000000)
	{
		ent->client->sess.credits = 1000000;
	}
	if (ent->client->sess.credits < 0)
	{
		ent->client->sess.credits = 0;
	}
	status_update();
	//Com_sprintf(Mfullwrite, sizeof(Mfullwrite), va("%s²%i²%s²", ent->client->sess.userpass, ent->client->sess.adminloggedin, ent->client->pers.netname));
	//Com_sprintf(Mfullwrite, sizeof(Mfullwrite), va("%s²%i²%s²%i²%i²", ent->client->sess.userpass, ent->client->sess.adminloggedin, ent->client->pers.netname, ent->client->sess.credits, ent->client->sess.builderadmin));
	Com_sprintf(Mfullwrite, sizeof(Mfullwrite), va("%s\n%i\n%s\n%i\n%i\n%i\n%i\n%i\n%s\n%i\n%i\n%i\n%s\n", ent->client->sess.userpass, ent->client->sess.adminloggedin, ent->client->pers.netname, ent->client->sess.credits, ent->client->sess.ampowers, ent->client->sess.ampowers2, ent->client->sess.ampowers3, ent->client->sess.ampowers4, ent->client->sess.mygroup, ent->client->sess.ampowers5, ent->client->sess.ampowers6, ent->client->sess.ampowers7, mc_timer()));
	trap_FS_Write( Mfullwrite, strlen(Mfullwrite), Mz);
	trap_FS_FCloseFile( Mz );
}
void mc_addcredits( gentity_t *ent, int Mnewcount )
{
	if (Q_stricmp(ent->client->sess.userlogged,"") == 0)
	{
		return;
	}
	if (Mnewcount == 0)
	{
		return;
	}
	ent->client->sess.credits += Mnewcount;
	if (ent->client->sess.credits > 1000000)
	{
		ent->client->sess.credits = 1000000;
	}
	if (ent->client->sess.credits < 0)
	{
		ent->client->sess.credits = 0;
	}
	if (Mnewcount > 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^7You received ^5%i^7 credits for a total of ^5%i^7.\n\"", Mnewcount, ent->client->sess.credits ) );
	}
	else
	{
		trap_SendServerCmd( ent->s.number, va("print \"^7You lost ^5%i^7 credits for a total of ^5%i^7.\n\"", -Mnewcount, ent->client->sess.credits ) );
	}
	mc_updateaccount(ent);
}
void mc_setcredits( gentity_t *ent, int Mnewcount )
{
	ent->client->sess.credits = Mnewcount;
	//trap_SendServerCmd( ent->s.number, va("print \"^7Client %s^7 now has ^5%i^7 credits.\n\"", ent->client->pers.netname, ent->client->sess.credits ) );
	mc_updateaccount(ent);
}
void	Svcmd_AddModel( gentity_t *ent, int clientNum )
{

gentity_t *jakes_model = G_Spawn();
//gentity_t *jakes_model_zocken = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
int	hoek;
vec3_t		origin, angles;

		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		char   xdlol[MAX_STRING_CHARS];
		char   dings3[MAX_STRING_CHARS];
	trap_Argv( 1, xdlol, sizeof( xdlol ) );

	trap_Argv( 2, dings3, sizeof( dings3 ) );
	trap_Argv( 1, model, sizeof( model ) );
	if (!strstr(model,".md3"))
	{
		strcpy(model,va("%s.md3",model));
	}
	if( strchr( ConcatArgs(0), ';' ) ) {
		trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
		G_FreeEntity(jakes_model);
		return;
	}
	if (trap_Argc() == 1)
	{
	trap_SendServerCmd( clientNum, va("print \"^1You have to give me an modelpath :)\n^1Example: /amaddmodel2 models/map_objects/bespin/bench.md3\n^1or if you want to plant the model by an origin, than do that:\n^1Example: /amaddmodel2 models/map_objects/bespin/bench.md3 0 0 0 YAW\n\"" ) );
	G_FreeEntity(jakes_model);
	}

	else if (trap_Argc() == 2)
	{

	AddSpawnField("model", model);
	VectorCopy(ent->client->ps.origin, origin);
	jakes_model->s.origin[0] = (int)origin[0];
	jakes_model->s.origin[1] = (int)origin[1];
	jakes_model->s.origin[2] = (int)origin[2] - 25;
	ent->client->ps.viewangles[0] = 0.0f;
	ent->client->ps.viewangles[2] = 0.0f;
	VectorCopy(ent->client->ps.viewangles, angles);
	jakes_model->classname = "mc_model";
	G_SetAngles(jakes_model, angles);
	SP_jakes_model(jakes_model);
	jakes_model->custom = 1;
	VectorCopy(ent->client->ps.origin, origin);
				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}

				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel2 admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[1]);

				Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[YAW]);


				trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );

	// Tele because you dont want to be freeze in this nonwalkable block
	//trap_UnlinkEntity ( ent );
	VectorClear( origin );
	VectorClear( angles );
	//VectorCopy(ent->client->ps.origin, origin);
	//hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
	//origin[0] = ent->client->ps.origin[0] + ( cos(hoek) * 60 ); // X
	//origin[1] = ent->client->ps.origin[1] + ( sin(hoek) * 60 ); // Y
	//origin[2] = ent->client->ps.origin[2]; // Z
	/*origin[0] = (int)origin[0] - 50; // X
	origin[1] = (int)origin[1] - 10; // Y
	origin[2] = (int)origin[2];*/ // Z
	//angles[YAW] = ent->client->ps.viewangles[YAW]; // Player viewangles [YAW]
	if (!ent->client->noclip)
	{
		vec3_t	fwd;
		trap_UnlinkEntity(ent);
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0] + fwd[0]*64;
		origin[1] = ent->client->ps.origin[1] + fwd[1]*64;
		origin[2] = ent->client->ps.origin[2]/* + fwd[2]*64*/;
		TeleportPlayer( ent , origin, angles );
	}
	ent->client->sess.lastent = jakes_model->s.number;

	}
else if (trap_Argc() == 6)
	{
	AddSpawnField("model", model);
	trap_Argv( 2, origin_number, 1024 );
	jakes_model->s.origin[0] = atoi(origin_number);
	trap_Argv( 3, origin_number2, 1024 );
	jakes_model->s.origin[1] = atoi(origin_number2);
	trap_Argv( 4, origin_number3, 1024 );
	jakes_model->s.origin[2] = atoi(origin_number3);
	trap_Argv( 5, yaw_number, 1024 );
	jakes_model->s.angles[1] = atoi(yaw_number);
	SP_jakes_model(jakes_model);
	jakes_model->classname = "mc_model";
	G_Printf("^7Added model^2: ^7%s at ^2(^7%s^2) (^7%s^2) (^7%s^2) (^7YAW: %s^2)", model, origin_number, origin_number2, origin_number3, yaw_number);

				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}
	VectorClear( origin );
	VectorClear( angles );
		VectorCopy(ent->client->ps.origin, origin);
	angles[YAW] = ent->client->ps.viewangles[YAW];
				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel2 admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
				ent->client->sess.lastent = jakes_model->s.number;
	}
else
	{
	trap_SendServerCmd( clientNum, va("print \"^1Bah.. not like that :P just /addmodel for infos\n\"" ) );
	G_FreeEntity(jakes_model);
	}
}
void	Svcmd_AddModel2_old( gentity_t *ent, int clientNum )
{

gentity_t *jakes_model = G_Spawn();
//gentity_t *jakes_model_zocken = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
int	hoek;
vec3_t		origin, angles;

		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		char   xdlol[MAX_STRING_CHARS];
		char   dings3[MAX_STRING_CHARS];
	trap_Argv( 1, xdlol, sizeof( xdlol ) );
	trap_Argv( 1, model, sizeof( model ) );

	if (!strstr(model,".md3"))
	{
		strcpy(model,va("%s.md3",model));
	}
	trap_Argv( 2, dings3, sizeof( dings3 ) );

	if (trap_Argc() == 1)
	{
	trap_SendServerCmd( clientNum, va("print \"^1You have to give me an modelpath :)\n^1Example: /amaddmodel models/map_objects/bespin/bench.md3\n^1or if you want to plant the model by an origin, than do this:\n^1Example: /amaddmodel models/map_objects/bespin/bench.md3 0 0 0 YAW\n\"" ) );
	G_FreeEntity(jakes_model);
	}

	else if (trap_Argc() == 2)
	{

	AddSpawnField("model", model);
	VectorCopy(ent->client->ps.origin, origin);
	jakes_model->s.origin[0] = (int)origin[0];
	jakes_model->s.origin[1] = (int)origin[1];
	jakes_model->s.origin[2] = (int)origin[2] - 25;
	jakes_model->classname = "mc_model";
	ent->client->ps.viewangles[0] = 0.0f;
	ent->client->ps.viewangles[2] = 0.0f;
	VectorCopy(ent->client->ps.viewangles, angles);

	G_SetAngles(jakes_model, angles);
	SP_jakes_model(jakes_model);
	VectorCopy(ent->client->ps.origin, origin);
				//trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				//Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				//trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				//if ( !f )
				//{
				//	return;
				//}

				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[1]);

				//Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[YAW]);


				//trap_FS_Write( line, strlen(line), f);

				//trap_FS_FCloseFile( f );

	// Tele because you dont want to be freeze in this nonwalkable block
	//trap_UnlinkEntity ( ent );
	VectorClear( origin );
	VectorClear( angles );
	//VectorCopy(ent->client->ps.origin, origin);
	//hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
	//origin[0] = ent->client->ps.origin[0] + ( cos(hoek) * 60 ); // X
	//origin[1] = ent->client->ps.origin[1] + ( sin(hoek) * 60 ); // Y
	//origin[2] = ent->client->ps.origin[2]; // Z
	/*origin[0] = (int)origin[0] - 50; // X
	origin[1] = (int)origin[1] - 10; // Y
	origin[2] = (int)origin[2];*/ // Z
	//angles[YAW] = ent->client->ps.viewangles[YAW]; // Player viewangles [YAW]
	if (!ent->client->noclip)
	{
		vec3_t	fwd;
		trap_UnlinkEntity(ent);
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0] + fwd[0]*64;
		origin[1] = ent->client->ps.origin[1] + fwd[1]*64;
		origin[2] = ent->client->ps.origin[2]/* + fwd[2]*64*/;
		TeleportPlayer( ent , origin, angles );
	}
	ent->client->sess.lastent = jakes_model->s.number;


	}
else if (trap_Argc() == 6)
	{
	AddSpawnField("model", model);
	trap_Argv( 2, origin_number, 1024 );
	jakes_model->s.origin[0] = atoi(origin_number);
	trap_Argv( 3, origin_number2, 1024 );
	jakes_model->s.origin[1] = atoi(origin_number2);
	trap_Argv( 4, origin_number3, 1024 );
	jakes_model->s.origin[2] = atoi(origin_number3);
	trap_Argv( 5, yaw_number, 1024 );
	jakes_model->s.angles[1] = atoi(yaw_number);
	jakes_model->classname = "mc_model";
	SP_jakes_model(jakes_model);
	G_Printf("^7Added model^2: ^7%s at ^2(^7%s^2) (^7%s^2) (^7%s^2) (^7YAW: %s^2)", model, origin_number, origin_number2, origin_number3, yaw_number);

				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				//trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				//if ( !f )
				//{
				//	return;
				//}
	VectorClear( origin );
	VectorClear( angles );
		VectorCopy(ent->client->ps.origin, origin);
	angles[YAW] = ent->client->ps.viewangles[YAW];
				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				//Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				//trap_FS_Write( line, strlen(line), f);

				//trap_FS_FCloseFile( f );
				ent->client->sess.lastent = jakes_model->s.number;
	}
else
	{
	trap_SendServerCmd( clientNum, va("print \"^1Bah.. not like that :P just /amaddmodel for infos\n\"" ) );
	G_FreeEntity(jakes_model);
	}
}
////////////////////////////






void	Svcmd_AddModel2( gentity_t *ent, int clientNum )
{
	gentity_t	*model;
	char		modelname[MAX_STRING_CHARS];
	char		x[MAX_STRING_CHARS];
	char		y[MAX_STRING_CHARS];
	char		z[MAX_STRING_CHARS];
	char		angle[MAX_STRING_CHARS];
	trap_Argv(1, modelname, sizeof(modelname));
	trap_Argv(2, x, sizeof(x));
	trap_Argv(3, y, sizeof(y));
	trap_Argv(4, z, sizeof(z));
	trap_Argv(5, angle, sizeof(angle));
	if ((Q_stricmp(modelname,"") == 0)||(Q_stricmp(modelname,"info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^1/amaddmodel <modelpath> [x] [y] [z] [yaw]\n\"" ) );
		return;
	}
	if (!strstr(modelname,".md3"))
	{
		strcpy(modelname,va("%s.md3",modelname));
	}
	model = G_Spawn();
	model->s.origin[0] = monkeyspot(x, 0, ent, ent);
	model->s.origin[1] = monkeyspot(y, 1, ent, ent);
	model->s.origin[2] = monkeyspot(z, 2, ent, ent);
	if (Q_stricmp(angle,"") == 0)
	{
		model->s.angles[YAW] = mc_fix360(ent->client->ps.viewangles[YAW]);
	}
	else
	{
		model->s.angles[YAW] = mc_fix360(atoi(angle));
	}
	model->classname = "mc_model";
	model->custom = 1;
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
	AddSpawnField("model", modelname);
	SP_jakes_model(model);
	G_SetAngles(model, model->s.angles);
	ent->client->sess.lastent = model->s.number;
	if (!ent->client->noclip)
	{
		//vec3_t	fwd, angles, origin;
		vec3_t	origin;
		//VectorClear(angles);
		//angles[0] = ent->client->ps.viewangles[0];
		trap_UnlinkEntity(ent);
		//AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0];// + fwd[0]*64;
		origin[1] = ent->client->ps.origin[1];// + fwd[1]*64;
		origin[2] = ent->client->ps.origin[2] + 21;/* + fwd[2]*64*/;
		TeleportPlayer( ent , origin, ent->client->ps.viewangles );
	}
	trap_SendServerCommand( clientNum, va("print \"^2Spawned new model as entity ^5%i^2.\n\"", model->s.number ) );
	ent->client->sess.traced = model->s.number;
}


void	Svcmd_AddEffect2( gentity_t *ent, int clientNum )
{
	gentity_t	*model;
	char		modelname[MAX_STRING_CHARS];
	char		x[MAX_STRING_CHARS];
	char		y[MAX_STRING_CHARS];
	char		z[MAX_STRING_CHARS];
	char		angle[MAX_STRING_CHARS];
	trap_Argv(1, modelname, sizeof(modelname));
	trap_Argv(2, x, sizeof(x));
	trap_Argv(3, y, sizeof(y));
	trap_Argv(4, z, sizeof(z));
	trap_Argv(5, angle, sizeof(angle));
	if ((Q_stricmp(modelname,"") == 0)||(Q_stricmp(modelname,"info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^1/amaddeffect <effect> [x] [y] [z] [yaw]\n\"" ) );
		return;
	}
	model = G_Spawn();
	model->s.origin[0] = monkeyspot(x, 0, ent, ent);
	model->s.origin[1] = monkeyspot(y, 1, ent, ent);
	model->s.origin[2] = monkeyspot(z, 2, ent, ent);
	if (Q_stricmp(angle,"") == 0)
	{
		model->s.angles[YAW] = mc_fix360(ent->client->ps.viewangles[YAW]);
	}
	else
	{
		model->s.angles[YAW] = mc_fix360(atoi(angle));
	}
	model->s.angles[PITCH] = -90;
	model->classname = "mc_effect";
	model->custom = 1;
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
	AddSpawnField("fxFile", modelname);
	SP_fx_runner(model);
	G_SetAngles(model, model->s.angles);
	G_SetOrigin(model, model->s.origin);
	ent->client->sess.lastent = model->s.number;
	trap_SendServerCommand( clientNum, va("print \"^2Spawned new effect as entity ^5%i^2.\n\"", model->s.number ) );
	ent->client->sess.traced = model->s.number;
}




























#if 0
void	Svcmd_AddModelDoor( gentity_t *ent, int clientNum )
{

gentity_t *jakes_model = G_Spawn();
//gentity_t *jakes_model_zocken = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
int	hoek;
vec3_t		origin, angles;

		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		char   xdlol[MAX_STRING_CHARS];
		char   dings3[MAX_STRING_CHARS];
		char   lip[MAX_STRING_CHARS];
	trap_Argv( 1, xdlol, sizeof( xdlol ) );
	trap_Argv( 1, model, sizeof( model ) );
	trap_Argv( 2, lip, sizeof( lip ) );

	if (!strstr(model,".md3"))
	{
		strcpy(model,va("%s.md3",model));
	}
	trap_Argv( 2, dings3, sizeof( dings3 ) );

	if (trap_Argc() < 3)
	{
	trap_SendServerCmd( clientNum, va("print \"^1You have to give me an modelpath :)\n^1Example: /amaddmodel models/map_objects/bespin/bench.md3\n^1or if you want to plant the model by an origin, than do this:\n^1Example: /amaddmodel models/map_objects/bespin/bench.md3 0 0 0 YAW\n\"" ) );
	G_FreeEntity(jakes_model);
	}

	else if (trap_Argc() == 3)
	{

	AddSpawnField("model", model);
	VectorCopy(ent->client->ps.origin, origin);
	jakes_model->s.origin[0] = (int)origin[0];
	jakes_model->s.origin[1] = (int)origin[1];
	jakes_model->s.origin[2] = (int)origin[2] - 25;
	jakes_model->classname = "mc_model";
	ent->client->ps.viewangles[0] = 0.0f;
	ent->client->ps.viewangles[2] = 0.0f;
	VectorCopy(ent->client->ps.viewangles, angles);

	G_SetAngles(jakes_model, angles);
	SP_mc_door(jakes_model, atoi(lip));
	VectorCopy(ent->client->ps.origin, origin);
				//trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				//Com_sprintf(savePath, 1024*4, "%s/%s.cfg", mc_editfolder.string, mapname.string);

				//trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				//if ( !f )
				//{
				//	return;
				//}

				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[1]);

				//Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[YAW]);


				//trap_FS_Write( line, strlen(line), f);

				//trap_FS_FCloseFile( f );

	// Tele because you dont want to be freeze in this nonwalkable block
	//trap_UnlinkEntity ( ent );
	VectorClear( origin );
	VectorClear( angles );
	//VectorCopy(ent->client->ps.origin, origin);
	//hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
	//origin[0] = ent->client->ps.origin[0] + ( cos(hoek) * 60 ); // X
	//origin[1] = ent->client->ps.origin[1] + ( sin(hoek) * 60 ); // Y
	//origin[2] = ent->client->ps.origin[2]; // Z
	/*origin[0] = (int)origin[0] - 50; // X
	origin[1] = (int)origin[1] - 10; // Y
	origin[2] = (int)origin[2];*/ // Z
	//angles[YAW] = ent->client->ps.viewangles[YAW]; // Player viewangles [YAW]
	if (!ent->client->noclip)
	{
		vec3_t	fwd;
		trap_UnlinkEntity(ent);
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0] + fwd[0]*64;
		origin[1] = ent->client->ps.origin[1] + fwd[1]*64;
		origin[2] = ent->client->ps.origin[2]/* + fwd[2]*64*/;
		TeleportPlayer( ent , origin, angles );
	}
	
	ent->client->sess.lastent = jakes_model->s.number;

	}
else if (trap_Argc() == 6)
	{
	AddSpawnField("model", model);
	trap_Argv( 2, origin_number, 1024 );
	jakes_model->s.origin[0] = atoi(origin_number);
	trap_Argv( 3, origin_number2, 1024 );
	jakes_model->s.origin[1] = atoi(origin_number2);
	trap_Argv( 4, origin_number3, 1024 );
	jakes_model->s.origin[2] = atoi(origin_number3);
	trap_Argv( 5, yaw_number, 1024 );
	jakes_model->s.angles[1] = atoi(yaw_number);
	jakes_model->classname = "mc_model";
	SP_mc_door(jakes_model);
	G_Printf("^7Added model^2: ^7%s at ^2(^7%s^2) (^7%s^2) (^7%s^2) (^7YAW: %s^2)", model, origin_number, origin_number2, origin_number3, yaw_number);

				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "%s/%s.cfg", mc_editfolder.string, mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}
	VectorClear( origin );
	VectorClear( angles );
		VectorCopy(ent->client->ps.origin, origin);
	angles[YAW] = ent->client->ps.viewangles[YAW];
				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				//trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
	}
else
	{
	trap_SendServerCmd( clientNum, va("print \"^1Bah.. not like that :P just /amaddmodel2 for infos\n\"" ) );
	G_FreeEntity(jakes_model);
	}
}
#endif
////////////////////////////















void	Svcmd_AddModelghoul2( gentity_t *ent, int clientNum )
{

gentity_t *jakes_model = G_Spawn();
//gentity_t *jakes_model_zocken = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
		fileHandle_t	fI;
int	hoek;
vec3_t		origin, angles;

		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		char   xdlol[MAX_STRING_CHARS];
		char   dings3[MAX_STRING_CHARS];
	trap_Argv( 1, xdlol, sizeof( xdlol ) );
	trap_Argv( 1, model, sizeof( model ) );

	trap_Argv( 2, dings3, sizeof( dings3 ) );

	if ((trap_Argc() == 1)||(Q_stricmp(model,"info") == 0))
	{
	trap_SendServerCmd( clientNum, va("print \"^1You have to give me an modelpath :)\n^1Example: /ammap_addglm models/weapons2/blaster_r/blaster_w.glm\n^1or if you want to plant the model by an origin, than do this:\n^1Example: /amaddmodel models/weapons2/blaster_r/blaster_w.glm 0 0 0 YAW\n\"" ) );
	G_FreeEntity(jakes_model);
	}

	if (!strstr(model,".glm"))
	{
		strcpy(model,va("%s.glm",model));
	}
	trap_FS_FOpenFile(model, &fI, FS_READ);
	if (!fI)
	{
		trap_SendServerCmd(clientNum, va("print \"^1Cannot find model ^5%s^1.\n\"", model));
	G_FreeEntity(jakes_model);
		return;
	}
	trap_FS_FCloseFile( fI );
	if (trap_Argc() == 2)
	{

	AddSpawnField("model", model);
	VectorCopy(ent->client->ps.origin, origin);
	jakes_model->s.origin[0] = (int)origin[0];
	jakes_model->s.origin[1] = (int)origin[1];
	jakes_model->s.origin[2] = (int)origin[2] - 25;
	jakes_model->classname = "mc_model2";
	ent->client->ps.viewangles[0] = 0.0f;
	ent->client->ps.viewangles[2] = 0.0f;
	VectorCopy(ent->client->ps.viewangles, angles);
	jakes_model->custom = 1;

	G_SetAngles(jakes_model, angles);
	SP_mc_ghoul(jakes_model);
	VectorCopy(ent->client->ps.origin, origin);
				//trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				//Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				//trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				//if ( !f )
				//{
				//	return;
				//}

				//File saved in format: EffectName X Y Z
				G_LogPrintf("Addglm admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[1]);

				//Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, (int)origin[0], (int)origin[1], (int)origin[2], (int)ent->client->ps.viewangles[YAW]);


				//trap_FS_Write( line, strlen(line), f);

				//trap_FS_FCloseFile( f );

	// Tele because you dont want to be freeze in this nonwalkable block
	//trap_UnlinkEntity ( ent );
	//VectorClear( origin );
	//VectorClear( angles );
	//VectorCopy(ent->client->ps.origin, origin);
	//hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
	//origin[0] = ent->client->ps.origin[0] + ( cos(hoek) * 60 ); // X
	//origin[1] = ent->client->ps.origin[1] + ( sin(hoek) * 60 ); // Y
	//origin[2] = ent->client->ps.origin[2]; // Z
	/*origin[0] = (int)origin[0] - 50; // X
	origin[1] = (int)origin[1] - 10; // Y
	origin[2] = (int)origin[2];*/ // Z
	//angles[YAW] = ent->client->ps.viewangles[YAW]; // Player viewangles [YAW]
	if (!ent->client->noclip)
	{
		vec3_t	fwd;
		trap_UnlinkEntity(ent);
		angles[ROLL] = 0;
		angles[PITCH] = 0;
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0] + fwd[0]*64;
		origin[1] = ent->client->ps.origin[1] + fwd[1]*64;
		origin[2] = ent->client->ps.origin[2]/* + fwd[2]*64*/;
		TeleportPlayer( ent , origin, angles );
	}
	ent->client->sess.lastent = jakes_model->s.number;


	}
else if (trap_Argc() == 6)
	{
	AddSpawnField("model", model);
	trap_Argv( 2, origin_number, 1024 );
	jakes_model->s.origin[0] = atoi(origin_number);
	trap_Argv( 3, origin_number2, 1024 );
	jakes_model->s.origin[1] = atoi(origin_number2);
	trap_Argv( 4, origin_number3, 1024 );
	jakes_model->s.origin[2] = atoi(origin_number3);
	trap_Argv( 5, yaw_number, 1024 );
	jakes_model->s.angles[1] = atoi(yaw_number);
	jakes_model->classname = "mc_model2";
	SP_mc_ghoul(jakes_model);
	G_Printf("^7Added model^2: ^7%s at ^2(^7%s^2) (^7%s^2) (^7%s^2) (^7YAW: %s^2)", model, origin_number, origin_number2, origin_number3, yaw_number);

				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}
	VectorClear( origin );
	VectorClear( angles );
		VectorCopy(ent->client->ps.origin, origin);
	angles[YAW] = ent->client->ps.viewangles[YAW];
				//File saved in format: EffectName X Y Z
				G_LogPrintf("AddModel admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				Com_sprintf( line, sizeof(line), "addmodelrcon %s %i %i %i %i\n", xdlol, origin_number, origin_number2, origin_number3, yaw_number);
				//trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
				ent->client->sess.lastent = jakes_model->s.number;
	}
else
	{
	trap_SendServerCmd( clientNum, va("print \"^1Bah.. not like that :P just /ammap_addglm for infos\n\"" ) );
	G_FreeEntity(jakes_model);
	}
}




















////////////////////////////
// __________________________________________________________________

void	Svcmd_AddEffect( gentity_t *ent, int clientNum )
{
vec3_t		origin;
gentity_t *fx_runner = G_Spawn();
char   effect[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		char   xdlol[MAX_STRING_CHARS];
	trap_Argv( 1, xdlol, sizeof( xdlol ) );


	if( strchr( ConcatArgs(0), ';' ) ) {
		trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
		return;
	}
	if (trap_Argc() == 1)
	{
	trap_SendServerCmd( clientNum, va("print \"^1You have to give me an effectpath :)\n^1Example: /amaddeffect2 effects/env/small_fire.efx\n^1or if you want to plant the effect by an origin, than do this:\n^1Example: /amaddeffect2 effects/env/small_fire.efx 0 0 0\n\"" ) );
	G_FreeEntity(fx_runner);
	}
	else if (trap_Argc() == 2)
	{
	trap_Argv( 1, effect, sizeof( effect ) );
	AddSpawnField("fxFile", effect);
	VectorCopy(ent->client->ps.origin, origin);
	fx_runner->s.origin[0] = (int)origin[0];
	fx_runner->s.origin[1] = (int)origin[1];
	fx_runner->s.origin[2] = (int)origin[2] - 19;
	fx_runner->classname = "mc_effect";
	SP_fx_runner(fx_runner);
	fx_runner->custom = 1;
	ent->client->sess.lastent = fx_runner->s.number;
				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "%s/%s.cfg", mc_editfolder, mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}

				//File saved in format: EffectName X Y Z
				 G_LogPrintf("AddEffect2 admin command has been executed by %s. (%s) <%i %i %i>\n", ent->client->pers.netname, xdlol, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2]);
				Com_sprintf( line, sizeof(line), "addeffectrcon %s %i %i %i 0 0 400\n", xdlol, (int)origin[0], (int)origin[1], (int)origin[2]);
				trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
	}
	else if (trap_Argc() == 5)
	{
	trap_Argv( 1, effect, sizeof( effect ) );
	AddSpawnField("fxFile", effect);
	trap_Argv( 2, origin_number, 1024 );
	fx_runner->s.origin[0] = atoi(origin_number);
	trap_Argv( 3, origin_number2, 1024 );
	fx_runner->s.origin[1] = atoi(origin_number2);
	trap_Argv( 4, origin_number3, 1024 );
	fx_runner->s.origin[2] = atoi(origin_number3);
	fx_runner->classname = "mc_effect";
	SP_fx_runner(fx_runner);
	ent->client->sess.lastent = fx_runner->s.number;
				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}

				//File saved in format: EffectName X Y Z
				 G_LogPrintf("AddEffect2 admin command has been executed by %s. (%s) <%i %i %i>\n", ent->client->pers.netname, xdlol, origin_number, origin_number2, origin_number3);
				Com_sprintf( line, sizeof(line), "addeffectrcon %s %i %i %i 0 0 400\n", xdlol, origin_number, origin_number2, origin_number3);
				trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
}
else
{
	trap_SendServerCmd( clientNum, va("print \"^1Bah.. not like that :P just /amaddeffect2 for infos\n\"" ) );
	G_FreeEntity(fx_runner);
}
}

void	Svcmd_AddEffect2_old( gentity_t *ent, int clientNum )
{
vec3_t		origin;
gentity_t *fx_runner = G_Spawn();
char   effect[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
		char			savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t	f;
		char			line[256];
		char   xdlol[MAX_STRING_CHARS];
	trap_Argv( 1, xdlol, sizeof( xdlol ) );


	if (trap_Argc() == 1)
	{
	trap_SendServerCmd( clientNum, va("print \"^1You have to give me an effectpath :)\n^1Example: /amaddeffect effects/env/small_fire.efx\n^1or if you want to plant the effect by an origin, than do this:\n^1Example: /amaddeffect effects/env/small_fire.efx 0 0 0\n\"" ) );
	G_FreeEntity(fx_runner);
	}
	else if (trap_Argc() == 2)
	{
	trap_Argv( 1, effect, sizeof( effect ) );
	AddSpawnField("fxFile", effect);
	VectorCopy(ent->client->ps.origin, origin);
	fx_runner->s.origin[0] = (int)origin[0];
	fx_runner->s.origin[1] = (int)origin[1];
	fx_runner->s.origin[2] = (int)origin[2] - 19;
	fx_runner->classname = "mc_effect";
	SP_fx_runner(fx_runner);
	ent->client->sess.lastent = fx_runner->s.number;
				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}

				//File saved in format: EffectName X Y Z
				 G_LogPrintf("AddEffect admin command has been executed by %s. (%s) <%i %i %i>\n", ent->client->pers.netname, xdlol, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2]);
				Com_sprintf( line, sizeof(line), "addeffectrcon %s %i %i %i\n", xdlol, (int)origin[0], (int)origin[1], (int)origin[2]);
				//trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
	}
	else if (trap_Argc() == 5)
	{
	trap_Argv( 1, effect, sizeof( effect ) );
	AddSpawnField("fxFile", effect);
	trap_Argv( 2, origin_number, 1024 );
	fx_runner->s.origin[0] = atoi(origin_number);
	trap_Argv( 3, origin_number2, 1024 );
	fx_runner->s.origin[1] = atoi(origin_number2);
	trap_Argv( 4, origin_number3, 1024 );
	fx_runner->s.origin[2] = atoi(origin_number3);
	fx_runner->classname = "mc_effect";
	SP_fx_runner(fx_runner);
	ent->client->sess.lastent = fx_runner->s.number;

				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf(savePath, 1024*4, "mapedits/%s.cfg", mapname.string);

				trap_FS_FOpenFile(savePath, &f, FS_APPEND);

				if ( !f )
				{
					return;
				}

				//File saved in format: EffectName X Y Z
				 G_LogPrintf("AddEffect admin command has been executed by %s. (%s) <%i %i %i>\n", ent->client->pers.netname, xdlol, origin_number, origin_number2, origin_number3);
				Com_sprintf( line, sizeof(line), "addeffectrcon %s %i %i %i\n", xdlol, origin_number, origin_number2, origin_number3);
				//trap_FS_Write( line, strlen(line), f);

				trap_FS_FCloseFile( f );
}
else
{
	trap_SendServerCmd( clientNum, va("print \"^1Bah.. not like that :P just /amaddeffect for infos\n\"" ) );
	G_FreeEntity(fx_runner);
}
}

void	Svcmd_addtarget_credits( gentity_t *ent, int clientNum )
{
vec3_t		origin;
gentity_t *target_credits = G_Spawn();
char   credcount[MAX_STRING_CHARS];
char mtargetname[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
		//char			line[256];
		//char   xdlol[MAX_STRING_CHARS];
	//trap_Argv( 1, xdlol, sizeof( xdlol ) );


	if (trap_Argc() == 1)
	{
	trap_SendServerCmd( clientNum, va("print \"^7/amaddtarget_credits <credits> <targetname>\"" ) );
	}
	else if (trap_Argc() == 2)
	{
	trap_Argv( 1, credcount, sizeof( credcount ) );
	AddSpawnField("count", credcount);
	trap_Argv( 2, mtargetname, 1024 );
	AddSpawnField("targetname", mtargetname);
	VectorCopy(ent->client->ps.origin, origin);
	target_credits->s.origin[0] = (int)origin[0];
	target_credits->s.origin[1] = (int)origin[1];
	target_credits->s.origin[2] = (int)origin[2];
	SP_target_mccredits(target_credits);
	}
	else
	{
		trap_SendServerCmd( clientNum, va("print \"^7/amaddtarget_credits <credits> <targetname>\n\"" ) );
	}
}
// --------------------------------------------------------------------------------------------------------------
/*
==================
DeathmatchScoreboardMessage

==================
*/

void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;
	int			fnums;
	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;
	fnums = 0;
	numSorted = level.numConnectedClients;

	if (numSorted > MAX_CLIENT_SCORE_SEND)
	{
		numSorted = MAX_CLIENT_SCORE_SEND;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;
		int		time;
		cl = &level.clients[level.sortedClients[i]];
		if (cl->sess.stealth == 1)
		{
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
			time = 0;
		} else {
			time = (level.time - cl->pers.enterTime)/60000;
			if (cl->sess.fakepingmin != 0)
			{
				ping = irand(cl->sess.fakepingmin, cl->sess.fakepingmax);
			}
			else if (cl->sess.PingHax > 0)
			{
				ping = irand(80, 110);
			}
			else
			{
			if (mc_unlagged.integer == 1)
			{
//unlagged - true ping
			//ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
			ping = cl->pers.realPing < 999 ? cl->pers.realPing : 999;
//unlagged - true ping
			}
			else
			{
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
			}
			}
		}

		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;
		fnums += 1;
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, time,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap_SendServerCmd( ent-g_entities, va("scores %i %i %i%s", fnums,
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( ent->client->sess.cheat == 1 )
		{
		return qtrue;
		}
	if ( !g_cheats.integer ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOCHEATS")));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MUSTBEALIVE")));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[2048];
	int		len;
	char	arg[2048];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	int dsp = atoi(in);
	// Deathspike: Another flood protect here.
	// Its the check on dsp and in characters.. meh should be another lil thing fixed. Lol another one =/
	while ( *in && dsp < 1022 )
	{
		if ( *in == 27 )
		{
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCmd( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCmd( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCmd( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		name[MAX_TOKEN_CHARS];
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];

	if ( !CheatsOk( ent ) ) {
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all)
	{
		i = 0;
		while (i < HI_NUM_HOLDABLE)
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
			i++;
		}
		i = 0;
	}

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			ent->health = atoi(arg);
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (WP_DET_PACK+1))  - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}

	if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
	{
		trap_Argv( 2, arg, sizeof( arg ) );
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
		return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		int num = 999;
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			num = atoi(arg);
		}
		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
			ent->client->ps.ammo[i] = num;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
		} else {
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}

		if (!give_all)
			return;
	}
	if (give_all || Q_stricmp(name, "force") == 0)
	{
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			ent->client->ps.fd.forcePower = atoi(arg);
		} else {
			ent->client->ps.fd.forcePower = 100;
		}

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCmd( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCmd( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCmd( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent ) ) {
		return;
	}

	// doesn't work in single player
	if ( g_gametype.integer != 0 ) {
		trap_SendServerCmd( ent-g_entities,
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_SendServerCmd( ent-g_entities, "clientLevelShot" );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if (ent->health <= 0) {
		trap_SendServerCmd( ent->s.number, va("print \"^1Cannot kill yourself while dead.\n\""));
		return;
	}
	if(mc_falltodeathdeath.integer == 0 && ent->client->ps.fallingToDeath)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1Cannot kill yourself while dead.\n\""));
		return;
	}
	if (ent->client->sess.freeze)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1Cannot kill yourself while frozen.\n\""));
		return;
	}
	if ( !twimod_allowselfkill.integer && !ent->client->ps.duelInProgress )
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1You are not allowed to kill yourself.\n\""));
		return;
	}

	if ( !twimod_allowduelselfkill.integer && ent->client->ps.duelInProgress )
	{
		trap_SendServerCmd( ent->s.number, va("print \"^1Cannot kill yourself while dueling.\n\""));
		return;
	}

	if (g_gametype.integer == GT_TOURNAMENT && level.numPlayingClients > 1 && !level.warmupTime)
	{
		if (!g_allowDuelSuicide.integer)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Cannot kill yourself while dueling.\n\""));
			return;
		}
	}
	
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
	gclient_t *wCl;
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		wCl = &level.clients[i];

		if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
			wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return &g_entities[wCl->ps.clientNum];
		}
	}

	return NULL;
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCmd( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEREDTEAM")) );
		trap_SendServerCmd( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEREDTEAM")) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCmd( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBLUETEAM")));
		trap_SendServerCmd( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBLUETEAM")));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap_SendServerCmd( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHESPECTATORS")));
		trap_SendServerCmd( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHESPECTATORS")));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		if (g_gametype.integer == GT_TOURNAMENT)
		{
			/*
			gentity_t *currentWinner = G_GetDuelWinner(client);

			if (currentWinner && currentWinner->client)
			{
				trap_SendServerCmd( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
				currentWinner->client->pers.netname, G_GetStripEdString("SVINGAME", "VERSUS"), client->pers.netname));
				trap_SendServerCmd( -1, va("print \"%s" S_COLOR_WHITE " %s %s\n\"",
				currentWinner->client->pers.netname, G_GetStripEdString("SVINGAME", "VERSUS"), client->pers.netname));
			}
			else
			{
				trap_SendServerCmd( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBATTLE")));
				trap_SendServerCmd( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBATTLE")));
			}
			*/
			//NOTE: Just doing a vs. once it counts two players up
		}
		else
		{
			trap_SendServerCmd( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBATTLE")));
			trap_SendServerCmd( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBATTLE")));
		}
	}

	G_LogPrintf ( "setteam:  %i %s %s\n",
				  client - &level.clients[0],
				  TeamName ( oldTeam ),
				  TeamName ( client->sess.sessionTeam ) );
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		ent->client->ps.saberHolstered = qtrue;
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			//For now, don't do this. The legalize function will set powers properly now.
			/*
			if (g_forceBasedTeams.integer)
			{
				if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					team = TEAM_BLUE;
				}
				else
				{
					team = TEAM_RED;
				}
			}
			else
			{
			*/
				team = PickTeam( clientNum );
			//}
		}

		if ( g_teamForceBalance.integer && !g_trueJedi.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
				{
					trap_SendServerCmd( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TOOMANYRED_SWITCH")) );
				}
				else
				*/
				{
					trap_SendServerCmd( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TOOMANYRED")) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					trap_SendServerCmd( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TOOMANYBLUE_SWITCH")) );
				}
				else
				*/
				{
					trap_SendServerCmd( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TOOMANYBLUE")) );
				}
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		//For now, don't do this. The legalize function will set powers properly now.
		/*
		if (g_forceBasedTeams.integer)
		{
			if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
			{
				trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MUSTBELIGHT")) );
				return;
			}
			if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
			{
				trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MUSTBEDARK")) );
				return;
			}
		}
		*/

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( (g_gametype.integer == GT_TOURNAMENT)
		&& level.numNonSpectatorClients >= 2 ) {
		team = TEAM_SPECTATOR;
	} else if ( g_maxGameClients.integer > 0 &&
		level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		if (mc_insta.integer == 0)
		{
		CopyToBodyQue(ent);
		}
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);

	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		if ( (g_gametype.integer != GT_TOURNAMENT) || (oldTeam != TEAM_SPECTATOR) )	{//so you don't get dropped to the bottom of the queue for changing skins, etc.
			client->sess.spectatorTime = level.time;
		}
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	ClientBegin( clientNum, qfalse );
}
/*void	Svcmd_ForceTeam_f2( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}*/
/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	//vec3_t	orig1;
	//vec3_t	ang1, src, vf, dest;
	//vec3_t	dest;
	//trace_t	trace;
	//VectorClear( orig1 );
	//VectorClear( ang1 );
	//AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
	//VectorMA( src, 100, vf, dest );
	//dest[0] = ent->client->ps.origin[0];
	//dest[1] = ent->client->ps.origin[1];
	//dest[2] = ent->client->ps.origin[2]+100;
	//VectorCopy(ent->client->ps.origin,orig1);
	//trap_Trace( &trace, ent->client->ps.origin, vec3_origin, vec3_origin, dest, ent->s.number, MASK_SOLID );
	//SetTeam(ent,"spectator");
	trap_UnlinkEntity (ent);
	//TeleportPlayer( ent, trace.endpos, ent->client->ps.viewangles );
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	//ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	//ent->client->ps.weapon = WP_NONE;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];
	if (!ent->client->sess.allowTeam && ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		trap_SendServerCmd( ent-g_entities, "print \"You are not allowed to change the team in the moment.\n\"" );
		return;
	}
	if ( trap_Argc() != 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
		switch ( oldTeam ) {
		case TEAM_BLUE:
			trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "PRINTBLUETEAM")) );
			break;
		case TEAM_RED:
			trap_SendServerCmd( ent-g_entities, va("print \"Red team\n\"", G_GetStripEdString("SVINGAME", "PRINTREDTEAM")) );
			break;
		case TEAM_FREE:
			trap_SendServerCmd( ent-g_entities, va("print \"Free team\n\"", G_GetStripEdString("SVINGAME", "PRINTFREETEAM")) );
			break;
		case TEAM_SPECTATOR:
			trap_SendServerCmd( ent-g_entities, va("print \"Spectator team\n\"", G_GetStripEdString("SVINGAME", "PRINTSPECTEAM")) );
			break;
		}
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOSWITCH")) );
		return;
	}
	if ((ent->client->sess.sessionTeam != TEAM_FREE)&&(level.lmsnojoin == 1))
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1LMS round in progress, cannot join.\n\""));
		return;
	}

	if (gEscaping)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1Can't change team while escaping.\n\""));
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
		//disallow changing teams
		trap_SendServerCmd( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
		return;
		//FIXME: why should this be a loss???
		//ent->client->sess.losses++;
	}

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	ent->client->switchTeamTime = level.time + 5000;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	char fpChStr[1024];
	const char *buf;
//	Cmd_Kill_f(ent);
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	buf = G_GetStripEdString("SVINGAME", "FORCEPOWERCHANGED");

	strcpy(fpChStr, buf);

	trap_SendServerCmd( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (g_gametype.integer == GT_TOURNAMENT)
	{ //If this is duel, don't even bother changing team in relation to this.
		return;
	}

	if (trap_Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap_Argv( 1, arg, sizeof( arg ) );

		if (arg && arg[0])
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {\
		//WTF???
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		//G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
		dir = 1;
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	if (original < 0)
	{
		original = 0;
	}
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if (g_entities[clientnum].client->sess.stealth == 1)
		{
			continue;
		}
		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message ) {
	if (!other) {
		return;
	}
	if (ent->client->sess.silence)
	{
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
		return;
	}
	else if (mode == SAY_ADMIN)
	{
		if (other->client->sess.adminloggedin)
		{
			trap_SendServerCmd( other-g_entities, va("chat \"%s%c%c%s%s%s\"",
				name, Q_COLOR_ESCAPE, color, ent->client->sess.amprefix, message, ent->client->sess.amsuffix));
		}
		else
		{
			return;
		}
	}

	trap_SendServerCmd( other-g_entities, va("%s \"%s%c%c%s%s%s\"",
		mode == SAY_TEAM ? "tchat" : "chat",
		name, Q_COLOR_ESCAPE, color, ent->client->sess.amprefix, message, ent->client->sess.amsuffix));
}

#define EC		"\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[512];
	// don't let text be too long for malicious reasons
	char		text[2048];
	char		location[64];

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		//G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		//G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC") (%s)"EC": ",
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ",
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
		else
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	case SAY_ADMIN:
		Com_sprintf (name, sizeof(name), "%c%c.:AdminSay:.%c%c%s%c%c: ", Q_COLOR_ESCAPE, COLOR_YELLOW, Q_COLOR_ESCAPE, COLOR_WHITE, ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_YELLOW;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		//G_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		if (other->client->sess.ignoring[ent->s.number] != 'A')
		{
			G_SayTo( ent, other, mode, color, name, text );
		}
	}
}


/*
==================
Cmd_Say_f
==================
*/
void adminsay(gentity_t *ent, char *message)
{
	int i;
	gentity_t *flent;
	for (i = 0; i < MAX_CLIENTS; i+=1)
	{
		flent = &g_entities[i];
		if (flent->client->sess.adminloggedin > 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^3[ADMIN]^7%s^7: ^3%s\"", ent->client->pers.netname, message ));
		}
	}
}
void channelsay1(gentity_t *ent, char *message)
{
	int i;
	gentity_t *flent;
	if (Q_stricmp(ent->client->sess.channel_01name,"") == 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^3You do not have a channel 1.\n\""));
		return;
	}
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(ent->client->sess.channel_01name,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_01name, ent->client->pers.netname, message ));
		}
		if (Q_stricmp(ent->client->sess.channel_01name,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_01name, ent->client->pers.netname, message ));
		}
		if (Q_stricmp(ent->client->sess.channel_01name,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_01name, ent->client->pers.netname, message ));
		}
	}
}
void channelsay2(gentity_t *ent, char *message)
{
	int i;
	gentity_t *flent;
	if (Q_stricmp(ent->client->sess.channel_02name,"") == 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^3You do not have a channel 2.\n\""));
		return;
	}
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(ent->client->sess.channel_02name,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_02name, ent->client->pers.netname, message ));
		}
		if (Q_stricmp(ent->client->sess.channel_02name,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_02name, ent->client->pers.netname, message ));
		}
		if (Q_stricmp(ent->client->sess.channel_02name,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_02name, ent->client->pers.netname, message ));
		}
	}
}
void channelsay3(gentity_t *ent, char *message)
{
	int i;
	gentity_t *flent;
	if (Q_stricmp(ent->client->sess.channel_03name,"") == 0)
	{
		trap_SendServerCmd( ent->s.number, va("print \"^3You do not have a channel 3.\n\""));
		return;
	}
	for (i = 0; i < 32; i+=1)
	{
		flent = &g_entities[i];
		if (Q_stricmp(ent->client->sess.channel_03name,flent->client->sess.channel_01name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_03name, ent->client->pers.netname, message ));
		}
		if (Q_stricmp(ent->client->sess.channel_03name,flent->client->sess.channel_02name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_03name, ent->client->pers.netname, message ));
		}
		if (Q_stricmp(ent->client->sess.channel_03name,flent->client->sess.channel_03name) == 0)
		{
			trap_SendServerCmd( flent-g_entities, va("chat \"^5[%s^5]^7%s^7: ^5%s\"", ent->client->sess.channel_03name, ent->client->pers.netname, message ));
		}
	}
}
int stringcontains(char *str, char *str2);
int stringcontains(char *str, char *str2)
{
	int	iX;
	int	iXLen = strlen(str);
	int	iXLen2 = strlen(str2);
	for (iX = 0;iX <= iXLen - iXLen2;iX++)
	{
		int iY;
		for (iY = 0;iY < iXLen2;iY++)
		{
			if (((str[iX+iY] >= 'a' && str[iX+iY] <= 'z')?(str[iX+iY]-('a'-'A')):str[iX+iY]) != ((str2[iY] >= 'a' && str2[iY] <= 'z')?(str2[iY]-('a'-'A')):str2[iY]))
			{
				goto nextex;
			}
		}
		return 1;
		nextex:
		continue;
	}
	return 0;
}
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;
	vec3_t		origin, angles;
	int		i;
	int		o;
	int		len;
	int		ilen;
	int		c;
	int		lawl;
	char		buffer[1024];
	char		filename[1024];
	char		blchar[1024];
	char		Tname[1024];
	char		Tx[1024];
	char		Ty[1024];
	char		Tz[1024];
	char		Tangle[1024];
	vec3_t		neworigin;
	vec3_t		newangles;
	vmCvar_t	mapname;
	int		matched;
	fileHandle_t	f;

	matched = 0;
	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (ent->client->sess.silence)
	{
		return;
	}
	if ((mode == 955)||(mode == 956)||(mode == 957))
	{
		p = ConcatArgs( 2 );
	}
	else
	{
	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}
	}
	ilen = strlen(p);
	for (i = 0;i < ilen;i += 1)
	{
		if (p[i] == '\"')
		{
			p[i] = '~';
		}
		if (p[i] == '"')
		{
			p[i] = '~';
		}
		if (p[i] == '\n')
		{
			p[i] = ' ';
		}
	}
	if ((Q_stricmpn ( p, "login", 5 ) == 0) ||
	(Q_stricmpn ( p, ",amregister", 7 ) == 0) ||
	(Q_stricmpn ( p, "7amregister", 7 ) == 0) ||
	(Q_stricmpn ( p, ",register", 7 ) == 0) ||
	(Q_stricmpn ( p, ".amregister", 7 ) == 0) ||
	(Q_stricmpn ( p, ".register", 7 ) == 0) ||
	(Q_stricmpn ( p, "/amregister", 7 ) == 0) ||
	(Q_stricmpn ( p, "/register", 7 ) == 0) ||
	(Q_stricmpn ( p, "?amregister", 7 ) == 0) ||
	(Q_stricmpn ( p, "?register", 7 ) == 0) ||
	(Q_stricmpn ( p, "register", 7 ) == 0) ||
	(Q_stricmpn ( p, "amregister", 7 ) == 0) ||
	(Q_stricmpn ( p, "amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, "'amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, ";amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, ":amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, "<amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, ">amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, ",amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, ".amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, "?amlogin", 7 ) == 0) ||
	(Q_stricmpn ( p, "/amlogin", 8 ) == 0) ||
	(Q_stricmpn ( p, "7amlogin", 8 ) == 0) ||
	(Q_stricmpn ( p, "'login", 7 ) == 0) ||
	(Q_stricmpn ( p, ";login", 7 ) == 0) ||
	(Q_stricmpn ( p, ":login", 7 ) == 0) ||
	(Q_stricmpn ( p, "<login", 7 ) == 0) ||
	(Q_stricmpn ( p, ">login", 7 ) == 0) ||
	(Q_stricmpn ( p, ",login", 7 ) == 0) ||
	(Q_stricmpn ( p, ".login", 7 ) == 0) ||
	(Q_stricmpn ( p, "?login", 7 ) == 0) ||
	(Q_stricmpn ( p, "7login", 8 ) == 0) ||
	(Q_stricmpn ( p, "/login", 6 ) == 0))
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^7 Broadcast of login data blocked. If it was intentional, add a few dots before the message.\n\"" ));
		return;
	}
	if ( Q_stricmp (p, "!version") == 0 )
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^4Twi^1Fire^7 mod by ^2MCMONKEY^7.\n\""));
		trap_SendServerCmd( ent-g_entities, va("cp \"^4Twi^1Fire^7 mod by ^2MCMONKEY^7.\n\""));
	}
	else if ((mc_insta.integer == 0) && (Q_stricmpn (p, "!buy", 4) == 0 || Q_stricmpn (p, "!price", 4) == 0))
	{
		char tobuy[MAX_STRING_CHARS];
		int ibuy = 0;
		int ibuypos = 0;
		int ibuylen = 0;
		int parttwo = 0;
		if (Q_stricmp(ent->client->sess.userlogged,"") == 0)
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^1You are not logged in.\n\""));
		}
		else
		{
			stringclear(tobuy, MAX_STRING_CHARS - 1);
			ibuylen = strlen(p);
			for (ibuy = 0;ibuy < ibuylen;ibuy++)
			{
				if (p[ibuy] == ' ')
				{
					parttwo = 1;
				}
				else if (parttwo == 1)
				{
					tobuy[ibuypos] = p[ibuy];
					ibuypos++;
				}
			}
			if (strlen(tobuy) == 0)
			{
				trap_SendServerCmd( ent-g_entities, va("print \"^1!buy <item>\n\""));
			}
			else
			{
			ibuy = weapforname(tobuy);
			if ((ibuy != WP_BLASTER) || (ibuy == WP_BLASTER && (strstr(tobuy,"blaster") || strstr(tobuy,"rifle") || strstr(tobuy,"e-11"))))
			{
				if (level.price_weapons[ibuy] > 0)
				{
					if (Q_stricmpn (p, "!price", 4) == 0)
					{
						trap_SendServerCmd( ent-g_entities, va("print \"^2That costs ^5%i^2 credits.\n\"", level.price_weapons[ibuy]));
					}
					else
					{
						if (ent->client->sess.credits < level.price_weapons[ibuy])
						{
							trap_SendServerCmd( ent-g_entities, va("print \"^2That costs ^5%i^2 credits, you don't have enough.\n\"", level.price_weapons[ibuy]));
						}
						else
						{
							mc_addcredits(ent, -level.price_weapons[ibuy]);
							ent->client->ps.stats[STAT_WEAPONS] |= (1 << ibuy);
							if (weaponData[ibuy].ammoIndex > AMMO_NONE)
							{
								ent->client->ps.ammo[weaponData[ibuy].ammoIndex] = 999;
							}
							trap_SendServerCmd( ent-g_entities, va("print \"^2Gave one ^5%s^2.\n\"", tobuy));
						}
					}
				}
				else
				{
					trap_SendServerCmd( ent-g_entities, va("print \"^1That weapon is not for sale here.\n\""));
				}
			}
			else
			{
				ibuy = -1;
				if (Q_stricmp(tobuy, "heal") == 0)
				{
					ibuy = FP_HEAL;
				}
				else if (Q_stricmp(tobuy, "jump") == 0)
				{
					ibuy = FP_LEVITATION;
				}
				else if (Q_stricmp(tobuy, "speed") == 0)
				{
					ibuy = FP_SPEED;
				}
				else if (Q_stricmp(tobuy, "push") == 0)
				{
					ibuy = FP_PUSH;
				}
				else if (Q_stricmp(tobuy, "pull") == 0)
				{
					ibuy = FP_PULL;
				}
				else if (Q_stricmp(tobuy, "mindtrick") == 0)
				{
					ibuy = FP_TELEPATHY;
				}
				else if (Q_stricmp(tobuy, "grip") == 0)
				{
					ibuy = FP_GRIP;
				}
				else if (Q_stricmp(tobuy, "lightning") == 0)
				{
					ibuy = FP_LIGHTNING;
				}
				else if (Q_stricmp(tobuy, "rage") == 0)
				{
					ibuy = FP_RAGE;
				}
				else if (Q_stricmp(tobuy, "protect") == 0)
				{
					ibuy = FP_PROTECT;
				}
				else if (Q_stricmp(tobuy, "absorb") == 0)
				{
					ibuy = FP_ABSORB;
				}
				else if (Q_stricmp(tobuy, "team_heal") == 0)
				{
					ibuy = FP_TEAM_HEAL;
				}
				else if (Q_stricmp(tobuy, "team_energize") == 0)
				{
					ibuy = FP_TEAM_FORCE;
				}
				else if (Q_stricmp(tobuy, "drain") == 0)
				{
					ibuy = FP_DRAIN;
				}
				else if (Q_stricmp(tobuy, "seeing") == 0)
				{
					ibuy = FP_SEE;
				}
				else if (Q_stricmp(tobuy, "saberthrow") == 0)
				{
					ibuy = FP_SABERTHROW;
				}
				else if (Q_stricmp(tobuy, "saberoffense") == 0)
				{
					ibuy = FP_SABERATTACK;
				}
				else if (Q_stricmp(tobuy, "saberdefense") == 0)
				{
					ibuy = FP_SABERDEFEND;
				}
				else
				{
					trap_SendServerCmd( ent-g_entities, va("print \"^1Unknown item ~^5%s^1~.\n\"", tobuy));
				}
				if (ibuy > -1)
				{
					if (level.price_force[ibuy] > 0)
					{
						if (Q_stricmpn (p, "!price", 4) == 0)
						{
							trap_SendServerCmd( ent-g_entities, va("print \"^2That costs ^5%i^2 credits.\n\"", level.price_force[ibuy]));
						}
						else
						{
							if (ent->client->sess.credits < level.price_force[ibuy])
							{
								trap_SendServerCmd( ent-g_entities, va("print \"^2That costs ^5%i^2 credits, you don't have enough.\n\"", level.price_force[ibuy]));
							}
							else
							{
								mc_addcredits(ent, -level.price_force[ibuy]);
								ent->client->ps.fd.forcePowerLevel[ibuy] = FORCE_LEVEL_3;
								if (!(ent->client->ps.fd.forcePowersKnown & (1 << ibuy)))
								{
									ent->client->sess.fpknown |= (1 << ibuy);
								}
								ent->client->ps.fd.forcePowersKnown |= (1 << ibuy);
								trap_SendServerCmd( ent-g_entities, va("print \"^2Received force power ^5%s^2.\n\"", tobuy));
							}
						}
					}
					else
					{
						trap_SendServerCmd( ent-g_entities, va("print \"^1That force power is not for sale here.\n\""));
					}
				}
			}
			}
		}
	}
	else if ( Q_stricmpn (p, "!credit", 7) == 0 )
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1/amhelp mod in console for Twifire mod credits.\n\""));
		trap_SendServerCmd( ent-g_entities, va("cp \"^4/amhelp mod\nin console for\nTwifire mod credits\n\""));
	}
	else if ( Q_stricmp (p, "!time") == 0 )
	{
		trap_SendServerCmd( -1, va("print \"^2The current server time is ^5%s^2.\n\"", mc_timer()));
	}
	else if ( Q_stricmp (p, "!motd") == 0 || Q_stricmp (p, "!rules") == 0 ) // Saycommand: !motd && !rules
	{
		//dsp_stringEscape(g_dsMOTD.string, MOTD, MAX_STRING_CHARS);
		//trap_SendServerCmd(ent-g_entities, va("cp \"%s\"", MOTD));
		dsp_doMOTD( ent );
	}
	for (i = 0;i < 512;i += 1)
	{
		gteleporter_t	*tele = &g_teleporters[i];
		if (tele && tele->active)
		{
			if (((tele->type2 == 0)&&(Q_stricmp(p, tele->name) == 0))||((tele->type2 == 2)&&(stringcontains(p, tele->name)==1))||((tele->type2 == 1)&&(Q_stricmpn(p, tele->name, strlen(tele->name)) == 0)))
			{
				if (isinnewgroup(ent, tele->group))
				{
				if (tele->type == 0)
				{

					vec3_t	angles;
					vec3_t	location;
					VectorClear(angles);
					VectorClear(location);
					location[0] = tele->pos[0];
					location[1] = tele->pos[1];
					location[2] = tele->pos[2]+25;
					angles[YAW] = tele->angle;
					TeleportPlayer( ent, location, angles );
				}
				else if (tele->type == 1)
				{
					if (ent->client->sess.empower)
					{
					ent->client->sess.empower = qfalse;
					ent->client->ps.isJediMaster = qfalse;
					ent->client->ps.fd.forcePower = 100;
					WP_InitForcePowers(ent);
					}
					else
					{
					ent->client->sess.empower = qtrue;
					ent->client->ps.saberHolstered = qtrue;
					ent->client->ps.isJediMaster = qtrue;
					ent->client->ps.fd.forcePower = 100;
					ent->client->ps.forceRestricted = qfalse;
					G_ScreenShake(vec3_origin, NULL, 5.0f, 500, qtrue);
					G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex("sound/chars/reborn2/misc/victory1") );
					for (f=0; f<NUM_FORCE_POWERS; f++)
					{
						ent->client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
						ent->client->ps.fd.forcePowersKnown |= (1 << f);
					}
					}
				}
				else if (tele->type == 2)
				{
					if (ent->client->sess.terminator)
					{
						ent->client->sess.terminator = qfalse;
						ent->client->ps.forceRestricted = qfalse;
						WP_InitForcePowers(ent);
						ent->client->ps.stats[STAT_HOLDABLE_ITEMS] = 0;
						ent->client->ps.stats[STAT_WEAPONS] = 0;
						ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_SABER);
						ent->client->ps.weapon = WP_SABER;
					}
					else
					{
						ent->client->sess.terminator = qtrue;
						ent->client->ps.saberHolstered = qtrue;
						G_ScreenShake(vec3_origin, NULL, 5.0f, 500, qtrue);
						ent->client->ps.stats[STAT_WEAPONS] = (1 << (WP_DET_PACK+1))  - ( 1 << WP_NONE );
						ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
						for ( f = 0 ; f < MAX_WEAPONS ; f++ )
						{
							ent->client->ps.ammo[f] = 9999;
						}
						ent->client->ps.forceRestricted = qtrue;
						ent->client->ps.weapon = WP_BRYAR_PISTOL;
					}
				}
				else if (tele->type == 3)
				{
					ent->client->ps.eFlags |= EF_INVULNERABLE;
					ent->client->invulnerableTimer = level.time + 60000;
					ent->client->sess.protect = 1;
				}
				else if (tele->type == 4)
				{
					trap_DropClient(ent->s.number, "quit.");
				}
				else if (tele->type == 5)
				{
					int	iX;
					int	iXLen = strlen(p);
					int	iXLen2 = strlen(tele->name);
					for (iX = 0;iX <= iXLen - iXLen2;iX++)
					{
						int iY;
						for (iY = 0;iY < iXLen2;iY++)
						{
							if (((p[iX+iY] >= 'a' && p[iX+iY] <= 'z')?(p[iX+iY]-('a'-'A')):p[iX+iY]) != ((tele->name[iY] >= 'a' && tele->name[iY] <= 'z')?(tele->name[iY]-('a'-'A')):tele->name[iY]))
							{
								goto nextex;
							}
						}
						for (iY = 0;iY < iXLen2;iY++)
						{
							p[iX+iY] = '*';
						}
						nextex:
						continue;
					}
				}
				}
				else
				{
					trap_SendServerCmd(ent-g_entities, "print \"^1You cannot use this chat command.\n\"");
				}
			}
		}
	}
	if (mode == SAY_TEAM && g_gametype.integer < GT_TEAM)
	{
		if (ent->client->sess.teamchattype >= 100)
		{
			if (g_channels[ent->client->sess.teamchattype-100].inchannel[ent->s.number] == 'A')
			{
				channels_print(ent->client->sess.teamchattype-100, va("^7%s^7: ^5%s", ent->client->pers.netname, p));
				return;
			}
			else
			{
				goto justsayit;
			}
		}
		switch (ent->client->sess.teamchattype)
		{
			case 1:
				if (ent->client->sess.adminloggedin > 0)
				{
					adminsay(ent, p);
					return;
				}
				break;
			case 2:
				channelsay1(ent, p);
				return;
				break;
			case 3:
				channelsay2(ent, p);
				return;
				break;
			case 4:
				channelsay3(ent, p);
				return;
				break;
			case 5:
				if (ent->client->sess.adminloggedin > 0)
				{
					if (ent->client->sess.ampowers3 & 524288)
					{
						dsp_stringEscape(p, p, MAX_STRING_CHARS);
						trap_SendServerCmd( -1, va("cp \"\n\n\n\n\n\n%s^7:\n^7%s\"", ent->client->pers.netname, p));
						return;
					}
				}
				break;
			case 6:
				if (ent->client->sess.adminloggedin > 0)
				{
					if (ent->client->sess.ampowers3 & 8388608)
					{
						dsp_stringEscape(p, p, MAX_STRING_CHARS);
						trap_SendServerCmd( -1, va("print \"%s\n\"", p));
						return;
					}
				}
				break;
			default:
				break;
		}
	}
	if (mode == 951)
	{
		//if (ent->client->sess.adminloggedin > 0)
		//{
			adminsay(ent, p);
			return;
		//}
	}
	if (mode == 955)
	{
		//if (ent->client->sess.adminloggedin > 0)
		//{
			channelsay1(ent, p);
			return;
		//}
	}
	if (mode == 956)
	{
		//if (ent->client->sess.adminloggedin > 0)
		//{
			channelsay2(ent, p);
			return;
		//}
	}
	if (mode == 957)
	{
		//if (ent->client->sess.adminloggedin > 0)
		//{
			channelsay3(ent, p);
			return;
		//}
	}
	justsayit:
	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	int			i;
	gentity_t	*target;
	char		*p;
	int		ilen;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = dsp_adminTarget(ent, arg, ent->s.number);
	if (targetNum < 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^7Invalid Target.\n\""));
		return;
	}
	target = &g_entities[targetNum];
	/*targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}*/

	p = ConcatArgs( 2 );
	ilen = strlen(p);
	for (i = 0;i < ilen;i += 1)
	{
		if (p[i] == '\"')
		{
			p[i] = ' ';
		}
	}

	trap_SendServerCmd(target->s.number, va("chat \"^7[%s^7 to you]:^6 %s\"", ent->client->pers.netname, p));
	trap_SendServerCmd(ent->fixednum, va("chat \"^7[you to %s^7]:^6 %s\"", target->client->pers.netname, p));

	//if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		//G_Say( ent, ent, SAY_TELL, p );
	//}
}


static void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonly ) {
	int color;
	char *cmd;

	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
		return;
	}
	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_TOURNAMENT )) {
		return;
	}

	if (mode == SAY_TEAM) {
		color = COLOR_CYAN;
		cmd = "vtchat";
	}
	else if (mode == SAY_TELL) {
		color = COLOR_MAGENTA;
		cmd = "vtell";
	}
	else {
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	trap_SendServerCmd( other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
}

void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
	int			j;
	gentity_t	*other;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	if ( target ) {
		G_VoiceTo( ent, target, mode, id, voiceonly );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "voice: %s %s\n", ent->client->pers.netname, id);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f( gentity_t *ent, qboolean voiceonly ) {
	int			targetNum;
	gentity_t	*target;
	char		*id;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	id = ConcatArgs( 2 );

	G_LogPrintf( "vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, id );
	G_Voice( ent, target, SAY_TELL, id, voiceonly );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Voice( ent, ent, SAY_TELL, id, voiceonly );
	}
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f( gentity_t *ent ) {
	gentity_t *who;
	int i;

	if (!ent->client) {
		return;
	}

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
		// i am a dead corpse
		if (!(ent->enemy->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		if (!(ent->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
		who = g_entities + ent->client->lastkilled_client;
		if (who->client) {
			// who is the person I just killed
			if (who->client->lasthurt_mod == MOD_STUN_BATON) {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );	// and I killed them with a gauntlet
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );
				}
			} else {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );	// and I killed them with something else
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if (g_gametype.integer >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
				if (who->client->rewardTime > level.time) {
					if (!(who->r.svFlags & SVF_BOT)) {
						G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					if (!(ent->r.svFlags & SVF_BOT)) {
						G_Voice( ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					return;
				}
			}
		}
	}

	// just say something
	G_Voice( ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
}



static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
	int		player;
	int		order;
	char	str[MAX_TOKEN_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	player = atoi( str );
	trap_Argv( 2, str, sizeof( str ) );
	order = atoi( str );

	if ( player < 0 || player >= MAX_CLIENTS ) {
		return;
	}
	if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
		return;
	}
	G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCmd( ent-g_entities, va("print \"(%i %i %i)\n\"", (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2] ) );
}

static const char *gameNames[] = {
	"Free For All",
	"Holocron FFA",
	"Jedi Master",
	"Duel",
	"Single Player",
	"Team FFA",
	"N/A",
	"Capture the Flag",
	"Capture the Ysalamiri"
};

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ )
	{
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) )
		{
			return i;
		}
	}

	return -1;
}

/*
==================
SanitizeString2

Rich's revised version of SanitizeString
==================
*/
void SanitizeString2( char *in, char *out )
{
	int i = 0;
	int r = 0;

	while (in[i])
	{
		if (i >= MAX_NAME_LENGTH-1)
		{ //the ui truncates the name here..
			break;
		}

		if (in[i] == '^')
		{
			if (in[i+1] >= 48 && //'0'
				in[i+1] <= 57) //'9'
			{ //only skip it if there's a number after it for the color
				i += 2;
				continue;
			}
			else
			{ //just skip the ^
				i++;
				continue;
			}
		}

		if (in[i] < 32)
		{
			i++;
			continue;
		}

		out[r] = in[i];
		r++;
		i++;
	}
	out[r] = 0;
}

/*
==================
G_ClientNumberFromStrippedName

Same as above, but strips special characters out of the names before comparing.
==================
*/
int G_ClientNumberFromStrippedName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ )
	{
		SanitizeString2( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) )
		{
			return i;
		}
	}

	return -1;
}

/*
==================
Cmd_CallVote_f
==================
*/

void Cmd_CallVote_f( gentity_t *ent) {
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	*concatted;

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	if ( !g_allowVote.integer) {
		if (!Q_stricmp( arg1, "poll" ))
		{
		}
		else
		{
			trap_SendServerCmd( ent-g_entities, va("print \"Voting not allowed.\n\"", G_GetStripEdString("SVINGAME", "NOVOTE")) );
			return;
		}
	}

	if ( level.voteTime) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEINPROGRESS")) );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT && Q_stricmp( arg1, "poll" ) != 0) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MAXVOTES")) );
		return;
	}
	/*if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOSPECVOTE")) );
		return;
	}*/
	if (ent->client->sess.votetime + (mc_votedelaytime.integer*1000) > level.time )
	{
		if (!(ent->client->sess.ampowers7 & 16))
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^7You may only call a vote once every ^5%i^7 seconds.\n\"", mc_votedelaytime.integer));
			return;
		}
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) || strchr( arg1, '\n' ) || strchr( arg2, '\n' ) ) {
		trap_SendServerCmd( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}


	if ( !Q_stricmp( arg1, "map_restart" ) )
	{
			
			if ( twimod_votecontrol_allowmapvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Map-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowmapvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Map-vote is not allowed here.\n\"" );
					return;
					}
				}
			if ( (level.time - level.startTime <= twimod_votecontrol_minmaptime.integer * 60000) && ( twimod_votecontrol_minmaptime.string != "none" ) && ( twimod_votecontrol_minmaptime.integer != 0 ) )
				{
					trap_SendServerCmd( ent-g_entities, va("print \"^7This map has to be run ^1%s ^7minutes before it can be changed again.\n\"", twimod_votecontrol_minmaptime.string) );
					return;
				}
		//return;
	}
	else if ( !Q_stricmp( arg1, "nextmap" ) )
		{
			if ( twimod_votecontrol_allowmapvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Map-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowmapvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Map-vote is not allowed here.\n\"" );
					return;
					}
				}
			if ( (level.time - level.startTime <= twimod_votecontrol_minmaptime.integer * 60000) && ( twimod_votecontrol_minmaptime.string != "none" ) && ( twimod_votecontrol_minmaptime.integer != 0 ) )
				{
					trap_SendServerCmd( ent-g_entities, va("print \"^7This map has to be run ^1%s ^7minutes before it can be changed again.\n\"", twimod_votecontrol_minmaptime.string) );
					return;
				}
	}
	else if ( !Q_stricmp( arg1, "map" ) )
		{
			if ( twimod_votecontrol_allowmapvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Map-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowmapvote.integer == 2 )
				{
					if (ent->client->sess.ampowers7 & 16)
					{
					G_Printf("Vote accepted.\n");
					}
					else
					{
					G_Printf("Vote rejected.\n");
					trap_SendServerCmd( ent-g_entities, "print \"^1You can not call map votes here.\n\"" );
					return;
					}
				}
			if ( (level.time - level.startTime <= twimod_votecontrol_minmaptime.integer * 60000) && ( twimod_votecontrol_minmaptime.string != "none" ) && ( twimod_votecontrol_minmaptime.integer != 0 ) )
				{
					trap_SendServerCmd( ent-g_entities, va("print \"^7This map has to be run ^1%s ^7minutes before it can be changed again.\n\"", twimod_votecontrol_minmaptime.string) );
					return;
				}
		}
	else if ( !Q_stricmp( arg1, "g_gametype" ) )
		{
			if ( twimod_votecontrol_allowgametypevote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Gametype-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowgametypevote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Gametype-vote is not allowed here.\n\"" );
					return;
					}
				}
		}
	else if ( !Q_stricmp( arg1, "kick" ) )
		{
			if ( twimod_votecontrol_allowkickvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Kick-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowkickvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Kick-vote is not allowed here.\n\"" );
					return;
					}
				}
		}
	else if ( !Q_stricmp( arg1, "clientkick" ) )
		{
			if ( twimod_votecontrol_allowkickvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Kick-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowkickvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Kick-vote is not allowed here.\n\"" );
					return;
					}
				}
		}/*
	else if ( !Q_stricmp( arg1, "g_doWarmup" ) )
		{
			if ( twimod_votecontrol_allowgametypevote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Warmup-vote is not allowed here.\n\"" );
					return;
				}
		}*/
	else if ( !Q_stricmp( arg1, "timelimit" ) )
		{
			if ( twimod_votecontrol_allowlimitvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Timelimit-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowlimitvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Timelimit-vote is not allowed here.\n\"" );
					return;
					}
				}
		}
	else if ( !Q_stricmp( arg1, "fraglimit" ) )
		{
			if ( twimod_votecontrol_allowlimitvote.integer == 0 )
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1Fraglimit-vote is not allowed here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowlimitvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1Fraglimit-vote is not allowed here.\n\"" );
					return;
					}
				}
		}
	else if ( !Q_stricmp( arg1, "poll" ) )
		{
			//if (!( ent->client->sess.ampowers3 & 131072 ))
				//{
					//trap_SendServerCmd( ent-g_entities, "print \"^1You are not allowed to call a poll.\n\"" );
					//return;
				//}
		}
	else if ( !Q_stricmp( arg1, "specialgametype" ) )
		{
			if ( twimod_votecontrol_allowSGTvote.integer == 0)
				{
					trap_SendServerCmd( ent-g_entities, "print \"^1You are not allowed to call a SpecialGameType vote here.\n\"" );
					return;
				}
			if ( twimod_votecontrol_allowSGTvote.integer == 2 )
				{
					if (!(ent->client->sess.ampowers7 & 16))
					{
					trap_SendServerCmd( ent-g_entities, "print \"^1You are not allowed to call a SpecialGameType vote here.\n\"" );
					return;
					}
				}
		}
	else
	{
		trap_SendServerCmd( ent-g_entities, "print \"^1This votestring does not exist.\nnextmap - map - g_gametype - kick - clientkick - timelimit - fraglimit - poll - specialgametype\n\"" );
		return;
	}
	ent->client->sess.votetime = level.time;
	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) )
	{
		if (qtrue)//( twimod_votecontrol_allowgametypevote.integer == 1 )
			{
		i = atoi( arg2 );
		if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
			trap_SendServerCmd( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}

		level.votingGametype = qtrue;
		level.votingGametypeTo = i;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
	}
	else
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1gametype-vote is not allowed here.\n\"" ) );
	}

}

	else if ( !Q_stricmp( arg1, "map" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		if (qtrue)//( twimod_votecontrol_allowmapvote.integer == 1 )
			{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		if (mc_onlydefaults.integer == 0)
		{
			if (!G_DoesMapSupportGametype(arg2, trap_Cvar_VariableIntegerValue("g_gametype")))
			{
				//trap_SendServerCmd( ent-g_entities, "print \"You can't vote for this map, it isn't supported by the current gametype.\n\"" );
				trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
				return;
			}
		}
		else
		{
			if (!(Q_stricmp(arg2,"ffa_bespin") == 0||
				Q_stricmp(arg2,"ffa_deathstar") == 0||
				Q_stricmp(arg2,"ffa_imperial") == 0||
				Q_stricmp(arg2,"ffa_ns_hideout") == 0||
				Q_stricmp(arg2,"ffa_ns_streets") == 0||
				Q_stricmp(arg2,"ffa_raven") == 0||
				Q_stricmp(arg2,"ffa_yavin") == 0||
				Q_stricmp(arg2,"ctf_bespin") == 0||
				Q_stricmp(arg2,"ctf_imperial") == 0||
				Q_stricmp(arg2,"ctf_ns_streets") == 0||
				Q_stricmp(arg2,"ctf_yavin") == 0||
				Q_stricmp(arg2,"duel_bay") == 0||
				Q_stricmp(arg2,"duel_carbon") == 0||
				Q_stricmp(arg2,"duel_jedi") == 0||
				Q_stricmp(arg2,"duel_pit") == 0||
				Q_stricmp(arg2,"duel_bespin") == 0||
				Q_stricmp(arg2,"duel_hangar") == 0||
				Q_stricmp(arg2,"duel_temple") == 0||
				Q_stricmp(arg2,"duel_training") == 0||
				Q_stricmp(arg2,"bespin_streets") == 0||
				Q_stricmp(arg2,"yavin_temple") == 0||
				Q_stricmp(arg2,"yavin_swamp") == 0||
				Q_stricmp(arg2,"yavin_trial") == 0||
				Q_stricmp(arg2,"bespin_platform") == 0||
				Q_stricmp(arg2,"cairn_reactor") == 0||
				Q_stricmp(arg2,"doom_shields") == 0||
				Q_stricmp(arg2,"yavin_canyon") == 0||
				Q_stricmp(arg2,"yavin_courtyard") == 0||
				Q_stricmp(arg2,"artus_topside") == 0||
				Q_stricmp(arg2,"cairn_assembly") == 0||
				Q_stricmp(arg2,"pit") == 0||
				Q_stricmp(arg2,"valley") == 0||
				Q_stricmp(arg2,"yavin_final") == 0))
				{
					trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
					return;
				}
		}
		if (qtrue)
		{
			fileHandle_t	f;
			char		blacklistbuffer[4096];
			char		mapname[1024];
			int		i;
			int		iL;
			int		iD;
			int		j;
			iL = trap_FS_FOpenFile("map_blacklist.cfg", &f, FS_READ);
			if (f)
			{
				trap_FS_Read( blacklistbuffer , 4096, f );
				iD = 0;
				for (i = 0;i <= 4096;i += 1)
				{
					if (blacklistbuffer[i] == ',')
					{
						if (Q_stricmp(mapname, arg2) == 0)
						{
							G_Printf("Callvote map for %s denied - blacklisted.\n", mapname);
							trap_SendServerCmd(ent->fixednum, "print \"^1That map is not allowed here.\n\"");
							return;
						}
						for (j = 0;j < 1024;j += 1)
						{
							mapname[j] = 0;
						}
						iD = 0;
						continue;
					}
					if (blacklistbuffer[i] == ';' || blacklistbuffer[i] == '' || iL == i)
					{
						break;
					}
					mapname[iD] = blacklistbuffer[i];
					iD += 1;
				}
				trap_FS_FCloseFile( f );
			}
		}
		//trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		//if (*s) {
		//	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		//} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "rmap %s", arg2 );
		//}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", arg2 );
	}
	else
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1Map-vote is not allowed here.\n\"" ) );
	}
}
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
				if (qtrue)//( twimod_votecontrol_allowkickvote.integer == 1 )
			{
		int n = atoi ( arg2 );

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap_SendServerCmd( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCmd( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );
	}
		else
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1kick-vote is not allowed here.\n\"" ) );
	}
}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
						if (qtrue)//( twimod_votecontrol_allowkickvote.integer == 1 )
			{
		int clientid = G_ClientNumberFromName ( arg2 );

		if ( clientid == -1 )
		{
			clientid = G_ClientNumberFromStrippedName(arg2);

			if (clientid == -1)
			{
				trap_SendServerCmd( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
				return;
			}
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
	}
			else
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1kick-vote is not allowed here.\n\"" ) );
	}
}
	else if ( !Q_stricmp ( arg1, "specialgametype" ) )
	{
		int	iL;
		fileHandle_t	f;
		iL = strlen(arg2);
		for (i = 0;i < iL;i += 1)
		{
			if (arg2[i] == ';')
			{
				trap_SendServerCmd( -1, va("print \"^3Cannot have ; in gametype name.\n\""));
				return;
			}
			if (arg2[i] == '\n')
			{
				trap_SendServerCmd( -1, va("print \"^3Cannot have lineskip in gametype name.\n\""));
				return;
			}
		}
		trap_FS_FOpenFile(va("gametypes/gt_%s.cfg", arg2), &f, FS_READ);
		if (!f)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown specialgametype ~^5%s^1~.\n\"", arg2));
			return;
		}
		trap_FS_FCloseFile(f);
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "specialgametype %s", arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "Special GameType %s", arg2 );
	}
	else if ( !Q_stricmp( arg1, "nextmap" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
							if (qtrue)//( twimod_votecontrol_allowmapvote.integer == 1 )
			{
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCmd( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	else
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1Mapvote is not allowed here.\n\"" ) );
	}
	}
	else if ( !Q_stricmp( arg1, "poll" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		//if ( ent->client->sess.ampowers3 & 131072 )
		//{
			concatted = ConcatArgs( 2 );
			Com_sprintf( level.voteString, sizeof( level.voteString ), "");
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s^7POLL: ^2%s^7", level.voteString, concatted );
			strcpy(level.pollstring,concatted);
		//}
		//else
		//{
		//	trap_SendServerCmd( ent-g_entities, va("print \"^1You are not allowed to call a poll.\n\"" ) );
		//}
	}
	else if ( !Q_stricmp( arg1, "fraglimit" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		if (strlen(arg2) > 5)
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^1Fraglimit too high.\n\""));
			return;
		}
		if (atoi(arg2) < 0)
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^1Fraglimit too low.\n\""));
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "fraglimit %i", atoi(arg2) );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	else if ( !Q_stricmp( arg1, "timelimit" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		if (strlen(arg2) > 5)
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^1Timelimit too high.\n\""));
			return;
		}
		if (atoi(arg2) < 0)
		{
			trap_SendServerCmd(ent-g_entities, va("print \"^1Timelimit too low.\n\""));
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "timelimit %i", atoi(arg2) );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	else if ( !Q_stricmp( arg1, "map_restart" ) )
	{
		level.votingGametypeTo = 0;
		level.votingGametype = qfalse;
		Com_sprintf( level.voteString, sizeof( level.voteString ), "rmap_restart" );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map_restart" );
	}
	else
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	if ( !Q_stricmp( arg1, "poll" ) )
	{
		concatted = ConcatArgs( 2 );
		trap_SendServerCmd( -1, va("print \"%s ^7called a poll: ^2%s^7\n\"", ent->client->pers.netname, concatted ) );
		level.vote_is_poll = 1;
	}
	else
	{
		//trap_SendServerCmd( -1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, G_GetStripEdString("SVINGAME", "PLCALLEDVOTE") ) );
		trap_SendServerCmd(-1, va("print \"^7%s^7 called a vote: ^5%s^7.\n\"", ent->client->pers.netname, level.voteDisplayString));
		level.vote_is_poll = 0;
	}

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
		level.clients[i].sess.lvote = 0;
	}
	ent->client->ps.eFlags |= EF_VOTED;
	ent->client->sess.lvote = 2;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTEINPROG")) );
		return;
	}
	/*if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEALREADY")) );
		return;
	}*/
	/*if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTEASSPEC")) );
		return;
	}*/

	trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "PLVOTECAST")) );


	trap_Argv( 1, msg, sizeof( msg ) );
	if (Q_stricmp(msg, "") == 0)
	{
		trap_SendServerCmd( ent-g_entities, va("print \"^1/vote <yes/no>\n\"") );
		return;
	}
	if ( msg[0] == 'y' || msg[0] == 'Y' || msg[0] == '1' || msg[0] == 'a' || msg[0] == 'A' ) {
		if ((ent->client->ps.eFlags & EF_VOTED)&&(ent->client->sess.lvote == 3))
		{
			level.voteYes+=1;
			level.voteNo-=1;
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		}
		if (!(ent->client->ps.eFlags & EF_VOTED))
		{
			level.voteYes += 1;
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		}
		ent->client->sess.lvote = 2;
	} else {
		if ((ent->client->ps.eFlags & EF_VOTED)&&(ent->client->sess.lvote == 2))
		{
			level.voteNo+=1;
			level.voteYes-=1;
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
			trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		}
		if (!(ent->client->ps.eFlags & EF_VOTED))
		{
			level.voteNo += 1;
			trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
		}
		ent->client->sess.lvote = 3;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTE")) );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TEAMVOTEALREADY")) );
		return;
	}
	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MAXTEAMVOTES")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOSPECVOTE")) );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCmd( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap_SendServerCmd( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCmd( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap_SendServerCmd( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCmd( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCmd( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCmd( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
	}
	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOTEAMVOTEINPROG")) );
		return;
	}
	if ( ent->client->ps.eFlags & EF_TEAMVOTED ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TEAMVOTEALREADYCAST")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTEASSPEC")) );
		return;
	}

	trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "PLTEAMVOTECAST")) );

	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOCHEATS")));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCmd( ent-g_entities, va("print \"^1setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCmd( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCmd( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	if (ps->usingATST)
	{
		return 0;
	}

	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		if (ps->fd.sentryDeployed/* && mc_sentrylimit.integer == 0*/)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap_Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap_Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SOLID );
		if (tr.fraction > 0.9 && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap_Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	default:
		return 1;
	}
}

extern int saberOffSound;
extern int saberOnSound;

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	if (!saberOffSound || !saberOnSound)
	{
		saberOffSound = G_SoundIndex("sound/weapons/saber/saberoffquick.wav");
		saberOnSound = G_SoundIndex("sound/weapons/saber/saberon.wav");
	}
	if (ent->client->sess.veh_isactive == 1)
	{
		ent->client->ps.saberHolstered = qtrue;
		return;
	}
	if (ent->client->sess.blockweapon == 1)
	{
		ent->client->ps.saberHolstered = qtrue;
		return;
	}
	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

//	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
//	{
//		return;
//	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		if (mc_dualsaber.integer == 0)
		{
		if (ent->client->ps.saberHolstered)
		{
			ent->client->ps.saberHolstered = qfalse;
			G_Sound(ent, CHAN_AUTO, saberOnSound);
		}
		else
		{
			ent->client->ps.saberHolstered = qtrue;
			G_Sound(ent, CHAN_AUTO, saberOffSound);

			//prevent anything from being done for 400ms after holster
			ent->client->ps.weaponTime = 400;
		}
		}
		else
		{
		if (ent->client->ps.saberHolstered)
		{
			ent->client->ps.saberHolstered = qfalse;
			G_Sound(ent, CHAN_AUTO, saberOnSound);
			ent->client->ps.dualBlade = qfalse;
		}
		else
		{
			if (ent->client->ps.dualBlade)
			{
				ent->client->ps.saberHolstered = qtrue;
				G_Sound(ent, CHAN_AUTO, saberOffSound);
				ent->client->ps.weaponTime = 400;
				ent->client->ps.dualBlade = qfalse;
			}
			else
			{
				ent->client->ps.dualBlade = qtrue;
			}
		}
		}
	}
}

void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;

	if ( !ent || !ent->client )
	{
		return;
	}
	if ( ent->client->sess.punish || ent->client->sess.sleep || ent->client->sess.freeze)
	{
		return;
	}
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/

	if (ent->client->saberCycleQueue)
	{ //resume off of the queue if we haven't gotten a chance to update it yet
		selectLevel = ent->client->saberCycleQueue;
	}
	else
	{
		selectLevel = ent->client->ps.fd.saberAnimLevel;
	}

	selectLevel++;
	if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABERATTACK] )
	{
		selectLevel = FORCE_LEVEL_1;
	}
/*
#ifndef FINAL_BUILD
	switch ( selectLevel )
	{
	case FORCE_LEVEL_1:
		trap_SendServerCmd( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
		break;
	case FORCE_LEVEL_2:
		trap_SendServerCmd( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
		break;
	case FORCE_LEVEL_3:
		trap_SendServerCmd( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
		break;
	}
#endif
*/
	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->saberCycleQueue = selectLevel;
	}
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}

void Cmd_EngageDuel_f(gentity_t *ent, int fftype)
{
	trace_t tr;
	char *value;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		//G_Printf("NODUEL: g_privateduel\n");
		return;
	}
	if (ent->client->sess.sleep || ent->client->sess.punish || ent->client->sess.freeze || ent->client->ps.forceHandExtend == HANDEXTEND_KNOCKDOWN)
	{ // Deathspike: Sleeper Bug
		//G_Printf("NODUEL: sleep\n");
		return;
	}
	if (g_gametype.integer == GT_TOURNAMENT)
	{ //rather pointless in this mode..
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NODUEL_GAMETYPE")) );
		//G_Printf("NODUEL: gametype\n");
		return;
	}

	if (g_gametype.integer >= GT_TEAM)
	{ //no private dueling in team modes
		trap_SendServerCmd( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NODUEL_GAMETYPE")) );
		//G_Printf("NODUEL: teamgametype\n");
		return;
	}
		if ( twimod_highpingduel.string[0] && Q_stricmp( twimod_highpingduel.string, "none" ) &&
			strcmp( twimod_highpingduel.string, value ) != 0)
			{
				if ( ent->client->ps.ping >= twimod_highpingduel.integer )
					{
						trap_SendServerCmd( ent-g_entities, va("cp \"You cant challenge someone. Your ping is too high.\n\nThe max. duel ping here is %s.\n\"", twimod_highpingduel.string) );
		//G_Printf("NODUEL: ping\n");
						return;
					}
			}
	if (ent->client->ps.duelTime >= level.time)
	{
		//G_Printf("NODUEL: dueltime\n");
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		//G_Printf("NODUEL: weapon\n");
		return;
	}

	/*
	if (!ent->client->ps.saberHolstered)
	{ //must have saber holstered at the start of the duel
		return;
	}
	*/
	//NOTE: WAS ZUM HOLZER SOLL DES BRINGEN?!

	if (ent->client->ps.saberInFlight)
	{
		G_Printf("NODUEL: saberthrow\n");
		return;
	}

	if (ent->client->ps.duelInProgress)
	{
		//G_Printf("NODUEL: inprog\n");
		return;
	}

	//New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)


	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);


	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
gentity_t *challenged = &g_entities[tr.entityNum];
		if (challenged->client->sess.sleep || challenged->client->sess.punish || challenged->client->sess.freeze)
		{ // Deathspike: Sleeper Bug
			return;
		}
		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
		//G_Printf("NODUEL: invalid target\n");
			return;
		}

		if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
		{
		//G_Printf("NODUEL: teambugwhat\n");
			return;
		}
		if ( twimod_highpingduel.string[0] && Q_stricmp( twimod_highpingduel.string, "none" ) &&
			strcmp( twimod_highpingduel.string, value ) != 0)
			{
				if ( challenged->client->ps.ping >= twimod_highpingduel.integer )
					{
		//G_Printf("NODUEL: ping2\n");
						trap_SendServerCmd( ent-g_entities, va("cp \"You cant challenge %s.\n^7The clients ping is too high.\n\nThe max. duel ping here is %s.\n\"", challenged->client->pers.netname, twimod_highpingduel.string) );
						return;
					}
			}
		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap_SendServerCmd( /*challenged-g_entities*/-1, va("print \"^7%s ^7%s ^7%s^7!\n\"", challenged->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELACCEPT"), ent->client->pers.netname) );
			ent->client->ps.stats[STAT_HEALTH]			= ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			ent->client->ps.stats[STAT_ARMOR]			= 100;
			challenged->client->ps.stats[STAT_HEALTH]			= challenged->health = challenged->client->ps.stats[STAT_MAX_HEALTH];
			challenged->client->ps.stats[STAT_ARMOR]	= 100;
			if (!ent->client->ps.saberHolstered)
			{
				G_Sound(ent, CHAN_AUTO, saberOffSound);
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = qtrue;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				G_Sound(challenged, CHAN_AUTO, saberOffSound);
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = qtrue;
			}
			StandardSetBodyAnim(ent, 668, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			StandardSetBodyAnim(challenged, 668, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);
			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)


		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			if (fftype == 0)
			{
			trap_SendServerCmd( challenged-g_entities, va("cp \"%s\n%s\n\"", ent->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELCHALLENGE")) );
			}
			else
			{
			trap_SendServerCmd( challenged-g_entities, va("cp \"%s\n^7Has challenged you to a\n^5Full-Force^7 duel!\n\"", ent->client->pers.netname) );
			}
			trap_SendServerCmd( ent-g_entities, va("cp \"%s\n%s\n\"", G_GetStripEdString("SVINGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
			ent->client->sess.duel_is_ff = fftype;
			challenged->client->sess.duel_is_ff = fftype;
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
	else
	{
		//G_Printf("NODUEL: hitwall?\n");
	}
}

void PM_SetAnim(int setAnimParts,int anim,int setAnimFlags, int blendTime);
/*
#ifdef _DEBUG
extern stringID_table_t animTable[MAX_ANIMATIONS+1];

void Cmd_DebugSetSaberMove_f(gentity_t *self)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	self->client->ps.saberMove = atoi(arg);
	self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	if (self->client->ps.saberMove >= LS_MOVE_MAX)
	{
		self->client->ps.saberMove = LS_MOVE_MAX-1;
	}

	Com_Printf("Anim for move: %s\n", animTable[saberMoveData[self->client->ps.saberMove].animToUse].name);
}

void Cmd_DebugSetBodyAnim_f(gentity_t *self, int flags)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];
	int i = 0;
	pmove_t pmv;

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(arg, animTable[i].name))
		{
			break;
		}
		i++;
	}

	if (i == MAX_ANIMATIONS)
	{
		Com_Printf("Animation '%s' does not exist\n", arg);
		return;
	}

	memset (&pmv, 0, sizeof(pmv));
	pmv.ps = &self->client->ps;
	pmv.animations = bgGlobalAnimations;
	pmv.cmd = self->client->pers.cmd;
	pmv.trace = trap_Trace;
	pmv.pointcontents = trap_PointContents;
	pmv.gametype = g_gametype.integer;

	pm = &pmv;
	PM_SetAnim(SETANIM_BOTH, i, flags, 0);

	Com_Printf("Set body anim to %s\n", arg);
}
#endif*/

void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
{
	pmove_t pmv;

	memset (&pmv, 0, sizeof(pmv));
	pmv.ps = &self->client->ps;
	pmv.animations = bgGlobalAnimations;
	pmv.cmd = self->client->pers.cmd;
	pmv.trace = nox_trap_Trace;
	pmv.pointcontents = trap_PointContents;
	pmv.gametype = g_gametype.integer;

	pm = &pmv;
	PM_SetAnim(SETANIM_BOTH, anim, flags, 0);
}

void DismembermentTest(gentity_t *self);

#ifdef _DEBUG
void DismembermentByNum(gentity_t *self, int num);
#endif

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[1024];
	char	cmd_a2[1024];
	char	cmd_a3[1024];
	char	cmd_a4[1024];
	char	cmd_a5[1024];
	char	cmd_a6[1024];
	char	cmd_a7[1024];
	char	cmd_a8[1024];
	char	cmd_a9[1024];
	char	*concatted;
	char	*p;
	char	cmd_a10[1024];
	gentity_t	*lolzw;
	int		knowncmd = qfalse;
	int		rofl;
	int		i;


	trap_Argv( 0, cmd, sizeof( cmd ) );
	trap_Argv( 1, cmd_a2, sizeof( cmd_a2 ) );
	trap_Argv( 2, cmd_a3, sizeof( cmd_a3 ) );
	trap_Argv( 3, cmd_a4, sizeof( cmd_a4 ) );
	trap_Argv( 4, cmd_a5, sizeof( cmd_a5 ) );
	trap_Argv( 5, cmd_a6, sizeof( cmd_a6 ) );
	trap_Argv( 6, cmd_a7, sizeof( cmd_a7 ) );
	trap_Argv( 7, cmd_a8, sizeof( cmd_a8 ) );
	trap_Argv( 8, cmd_a9, sizeof( cmd_a9 ) );
	trap_Argv( 9, cmd_a10, sizeof( cmd_a10 ) );
	concatted = ConcatArgs( 0 );
	commandprocessstart:
    if (strstr(concatted, "\n") || strstr(concatted, "\r"))
    {
        G_Printf("Client %i may be attempting to cheat (invalid input codes detected)!\n", clientNum);
		trap_SendServerCmd(clientNum, va("print \"^7Input denied.\n\""));
        return;
    }
	rofl = clientNum;
	ent = g_entities + clientNum;
	ent->s.number = ent-g_entities;
	ent->fixednum = ent-g_entities;
	/*if (ent->client->sess.isAFK == 1)
	{
		G_Printf("Client %i used a command!\n", ent->s.number);
	}*/
	setAFKOff(ent);
	ent->client->sess.commandtime = level.time;
	//G_Printf("TEST %i is %i mark %i\n", ent->s.number, clientNum, ent-g_entities);
	// JK2 will sometimes switch a player's s.number around. I've added this to ensure the s.number always stays correct.
	if ( (!ent->client) || (!ent->inuse) ) {
		return;		// not fully in game yet
	}

	if ((Q_stricmp(cmd, "amlogin") != 0)&&(Q_stricmp(cmd, "amregister") != 0)&&(Q_stricmp(cmd, "tell") != 0)&&(Q_stricmp(cmd, "amnewpass") != 0))
	{
	mc_print(va("ClientCommand: %i %s^7: %s\n", ent->s.number,ent->client->pers.netname, concatted));
	//if ((Q_stricmp (cmd, "say") != 0)&&(Q_stricmp(cmd, "amlogin") != 0)&&(Q_stricmp(cmd, "tell") != 0))
	//{
	for (i=0; i<MAX_CLIENTS; i++)
	{
		lolzw = &g_entities[i];
		if (!lolzw || !lolzw->client)
		{
			continue;
		}
		if (lolzw->client->sess.monitor1 == 1)
		{
			trap_SendServerCmd(lolzw->s.number, va("print \"ClientCommand: %s^7: %s\n\"",ent->client->pers.netname, concatted));
		}
	}
	}
	if (ent->client->sess.fakelag == 1)
	{
		if (Q_stricmp (cmd, "say") == 0)
	for (i=0; i<MAX_CLIENTS; i++)
	{
		lolzw = &g_entities[i];
		if (!lolzw || !lolzw->client)
		{
			continue;
		}
		if (lolzw->client->sess.monitor1 == 1)
		{
			trap_SendServerCmd(lolzw->s.number, va("print \"ClientCommand: %s^7: %s\n\"",ent->client->pers.netname, concatted));
		}
	}
		return;
	}
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if ((Q_stricmp(cmd, "amme") == 0)||(Q_stricmp(cmd, "me") == 0))
	{
		if (!ent->client->sess.silence)
		{
			if (Q_stricmp(cmd_a2,"") == 0)
			{
				trap_SendServerCmd(ent->fixednum, va("print \"/me <does something>\n\""));
				return;
			}
			p = ConcatArgs( 1 );
			trap_SendServerCmd(-1, va("chat \"^7* %s^7 %s\"", ent->client->pers.netname, p));
		}
		return;
	}
	if (Q_stricmp (cmd, "amsay") == 0) {
		Cmd_Say_f (ent, 951, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "ambalance") == 0)
	{
		if (Q_stricmp(ent->client->sess.userlogged, "") != 0)
		{
			trap_SendServerCmd(clientNum, va("print \"^7You have ^5%i^7 credits.\n\"", ent->client->sess.credits));
		}
		else
		{
			trap_SendServerCmd(clientNum, va("print \"^1You are not logged in.\n\""));
		}
		return;
	}
/*
	if (Q_stricmp (cmd, "amchannel_join") == 0) {
		joinchannel(ent, cmd_a2, cmd_a3);
		return;
	}
	if (Q_stricmp (cmd, "amchannel_create") == 0) {
		createchannel(ent, cmd_a2, cmd_a3);
		return;
	}
	if (Q_stricmp (cmd, "amchannel_leave") == 0) {
		leavechannel(ent, atoi(cmd_a2));
		return;
	}
	if (Q_stricmp (cmd, "amchannel_current") == 0) {
		listchannels(ent);
		return;
	}
	if (Q_stricmp (cmd, "amchannel_members") == 0) {
		listchannelmembers(ent, cmd_a2);
		return;
	}
	if (Q_stricmp (cmd, "amchannel_kick") == 0) {
		channelkick(ent, atoi(cmd_a2), cmd_a3);
		return;
	}
	if (Q_stricmp (cmd, "amchannel_say") == 0) {
		if (atoi(cmd_a2) == 1)
		{
			Cmd_Say_f (ent, 955, qtrue);
		}
		else if (atoi(cmd_a2) == 2)
		{
			Cmd_Say_f (ent, 956, qtrue);
		}
		else if (atoi(cmd_a2) == 3)
		{
			Cmd_Say_f (ent, 957, qtrue);
		}
		else
		{
			trap_SendServerCmd(ent->fixednum,va("print \"Invalid channel number.\n\""));
		}
		return;
	}
*/
	if (Q_stricmp (cmd, "say_team") == 0) {
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}
/*
	if (Q_stricmp (cmd, "vsay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vsay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vtell") == 0) {
		Cmd_VoiceTell_f ( ent, qfalse );
		return;
	}
	if (Q_stricmp (cmd, "vosay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "vosay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "votell") == 0) {
		Cmd_VoiceTell_f ( ent, qtrue );
		return;
	}
	if (Q_stricmp (cmd, "vtaunt") == 0) {
		Cmd_VoiceTaunt_f ( ent );
		return;
	}
*/
	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime)
	{
		qboolean giveError = qfalse;

		if (!Q_stricmp(cmd, "give"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "god"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "notarget"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "noclip"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "kill"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamtask"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "levelshot"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follownext"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "followprev"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "team"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "forcechanged"))
		{ //special case: still update force change
			Cmd_ForceChanged_f (ent);
			return;
		}
		else if (!Q_stricmp(cmd, "where"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "vote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callteamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "gc"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "setviewpos"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "stats"))
		{
			giveError = qtrue;
		}

		if (giveError)
		{
			trap_SendServerCmd( clientNum, va("print \"You cannot perform this task (%s) during the intermission.\n\"", cmd ) );
		}
		else
		{
			Cmd_Say_f (ent, qfalse, qtrue);
		}
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
	{
		Cmd_Give_f (ent);
	}
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
	{
		// Deathspike: Allow or Dis-Allow Suicide
		if (ent->client->sess.allowKill)
		{
			Cmd_Kill_f (ent);
		}
		else
		{
			trap_SendServerCmd( clientNum, va("print \"You are not allowed to kill yourself.\n\"" ) );
		}
	}
	//else if (Q_stricmp (cmd, "teamtask") == 0)
		//Cmd_TeamTask_f (ent);
	//else if (Q_stricmp (cmd, "levelshot") == 0)
		//Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);
	else if (Q_stricmp (cmd, "team") == 0)
	{
		if (ent->client->sess.allowKill)
		{
			Cmd_Team_f (ent);
		}
	}
	else if (Q_stricmp (cmd, "forcechanged") == 0)
		Cmd_ForceChanged_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	//else if (Q_stricmp (cmd, "callteamvote") == 0)
		//Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);
	//else if (Q_stricmp (cmd, "gc") == 0)
		//Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	//else if (Q_stricmp (cmd, "stats") == 0)
		//Cmd_Stats_f( ent );
	/*else if (Q_stricmp(cmd, "#mm") == 0 && CheatsOk( ent ))
	{
		G_PlayerBecomeATST(ent);
	}*/
	//I broke the ATST when I restructured it to use a single global anim set for all client animation.
	//You can fix it, but you'll have to implement unique animations (per character) again.
#ifdef _DEBUG //sigh..
	else if (Q_stricmp(cmd, "headexplodey") == 0 && CheatsOk( ent ))
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			float presaveVel = ent->client->ps.velocity[2];
			ent->client->ps.velocity[2] = 500;
			DismembermentTest(ent);
			ent->client->ps.velocity[2] = presaveVel;
		}
	}
	/*else if (Q_stricmp(cmd, "g2animent") == 0 && CheatsOk( ent ))
	{
		G_CreateExampleAnimEnt(ent);
	}*/ // This is admin only now.
	/*else if (Q_stricmp(cmd, "loveandpeace") == 0 && CheatsOk( ent ))
	{
		trace_t tr;
		vec3_t fPos;

		AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

		fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
		fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
		fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

		if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other && other->inuse && other->client)
			{
				vec3_t entDir;
				vec3_t otherDir;
				vec3_t entAngles;
				vec3_t otherAngles;

				if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(ent);
				}

				if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}

				if ((ent->client->ps.weapon != WP_SABER || ent->client->ps.saberHolstered) &&
					(other->client->ps.weapon != WP_SABER || other->client->ps.saberHolstered))
				{
					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );

					StandardSetBodyAnim(ent, BOTH_KISSER1LOOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					ent->client->ps.saberMove = LS_NONE;
					ent->client->ps.saberBlocked = 0;
					ent->client->ps.saberBlocking = 0;

					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					StandardSetBodyAnim(other, BOTH_KISSEE1LOOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					other->client->ps.saberMove = LS_NONE;
					other->client->ps.saberBlocked = 0;
					other->client->ps.saberBlocking = 0;
				}
			}
		}
	}*/ // /amkiss
#endif
	/*else if (Q_stricmp(cmd, "thedestroyer") == 0 && CheatsOk( ent ) && ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
	{
		Cmd_ToggleSaber_f(ent);

		if (!ent->client->ps.saberHolstered)
		{
			if (ent->client->ps.dualBlade)
			{
				ent->client->ps.dualBlade = qfalse;
				//ent->client->ps.fd.saberAnimLevel = FORCE_LEVEL_1;
			}
			else
			{
				ent->client->ps.dualBlade = qtrue;

				trap_SendServerCmd( -1, va("print \"%sTHE DESTROYER COMETH\n\"", S_COLOR_RED) );
				G_ScreenShake(vec3_origin, NULL, 10.0f, 800, qtrue);
				//ent->client->ps.fd.saberAnimLevel = FORCE_LEVEL_3;
			}
		}
	}*/ // admin only now
	/*
#ifdef _DEBUG
	else if (Q_stricmp(cmd, "debugSetSaberMove") == 0)
	{
		Cmd_DebugSetSaberMove_f(ent);
	}
	else if (Q_stricmp(cmd, "debugSetBodyAnim") == 0)
	{
		Cmd_DebugSetBodyAnim_f(ent, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}
	else if (Q_stricmp(cmd, "debugDismemberment") == 0)
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			char	arg[MAX_STRING_CHARS];
			int		iArg = 0;

			if (trap_Argc() > 1)
			{
				trap_Argv( 1, arg, sizeof( arg ) );

				if (arg[0])
				{
					iArg = atoi(arg);
				}
			}

			DismembermentByNum(ent, iArg);
		}
	}
#endif*/
	else if ( Q_stricmp(cmd, "debugKnockMeDown") == 0 )
	{
		if ( twimod_allowDebugKnockMeDown.integer )
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
			ent->client->ps.forceDodgeAnim	= 0;
			if ( trap_Argc() > 1 )
			{
				ent->client->ps.forceHandExtendTime = level.time + 1100;
				ent->client->ps.quickerGetup = qfalse;
			}
			else
			{
				ent->client->ps.forceHandExtendTime = level.time + 700;
				ent->client->ps.quickerGetup = qtrue;
			}
		}
		else
		{
			trap_SendServerCmd( clientNum, va("print \"^1This command is disabled.\n\"" ) );
		}
	}/*
	else if (Q_stricmp(cmd, "renameself") == 0)
	{
		knowncmd = qtrue;
		renameself(ent);
		return;
	}*/
	else if (Q_stricmp(cmd, "amaccept") == 0)
	{
		if ((ent->client->sess.pendingtimeout != 0) && (ent->client->sess.pendingtimeout < level.time))
		{
			ent->client->sess.pendingtimeout = 0;
			ent->client->sess.pendingtype = 0;
			ent->client->sess.pendingvalue = 0;
		}
		if ((ent->client->sess.pendingtimeout == 0) || (ent->client->sess.pendingtype == 0))
		{
			ent->client->sess.pendingtimeout = 0;
			ent->client->sess.pendingtype = 0;
			ent->client->sess.pendingvalue = 0;
			trap_SendServerCmd( clientNum, va("print \"^1You have no pending requests.\n\"" ) );
			return;
		}
		else
		{
			trap_SendServerCmd( clientNum, va("print \"^2Request accepted.\n\"" ) );
			switch (ent->client->sess.pendingtype)
			{
				case 1: // amrget
					{
						vec3_t	angles;
						vec3_t	origin;
						vec3_t	fwd;
						gentity_t	*other = &g_entities[ent->client->sess.pendingvalue];
						if (!other || !other->client || !other->inuse)
						{
							trap_SendServerCmd( clientNum, va("print \"^1Requesting player disconnected.\n\"" ) );
							return;
						}
						angles[ROLL] = 0;
						angles[PITCH] = 0;
						angles[YAW] = other->client->ps.viewangles[YAW];
						AngleVectors(angles, fwd, NULL, NULL);
						origin[0] = other->client->ps.origin[0] + fwd[0]*128;
						origin[1] = other->client->ps.origin[1] + fwd[1]*128;
						origin[2] = other->client->ps.origin[2]/* + fwd[2]*64*/;
						if ( other->s.number == ent->s.number )
						{
							angles[YAW] = AngleNormalize360(ent->client->ps.viewangles[YAW]); // Other-Player viewangles [YAW]
						}
						else
						{
							angles[YAW] = AngleNormalize360(other->client->ps.viewangles[YAW] + 180); // Other-Player viewangles [YAW]
						}
						TeleportPlayer( ent, origin, angles );
						other->client->ps.forceHandExtend = HANDEXTEND_FORCEPULL;
						other->client->ps.forceHandExtendTime = level.time + 300;
						trap_SendServerCmd( other->s.number, va("print \"^7%s^2 accepted your request.\n\"", ent->client->pers.netname ) );
					}
				break;
				case 2: // amrgoto
					{
						vec3_t	angles;
						vec3_t	origin;
						vec3_t	fwd;
						gentity_t	*other = &g_entities[ent->client->sess.pendingvalue];
						if (!other || !other->client || !other->inuse)
						{
							trap_SendServerCmd( clientNum, va("print \"^1Requesting player disconnected.\n\"" ) );
							return;
						}
						angles[ROLL] = 0;
						angles[PITCH] = 0;
						angles[YAW] = ent->client->ps.viewangles[YAW];
						AngleVectors(angles, fwd, NULL, NULL);
						origin[0] = ent->client->ps.origin[0] + fwd[0]*128;
						origin[1] = ent->client->ps.origin[1] + fwd[1]*128;
						origin[2] = ent->client->ps.origin[2]/* + fwd[2]*64*/;
						if ( other->s.number == ent->s.number )
						{
							angles[YAW] = AngleNormalize360(ent->client->ps.viewangles[YAW]); // Other-Player viewangles [YAW]
						}
						else
						{
							angles[YAW] = AngleNormalize360(ent->client->ps.viewangles[YAW] + 180); // Other-Player viewangles [YAW]
						}
						TeleportPlayer( other, origin, angles );
						other->client->ps.forceHandExtend = HANDEXTEND_FORCEPULL;
						other->client->ps.forceHandExtendTime = level.time + 300;
						trap_SendServerCmd( other->s.number, va("print \"^7%s^2 accepted your request.\n\"", ent->client->pers.netname ) );
					}
				break;
				default:
					trap_SendServerCmd( clientNum, va("print \"^1Error accepting request: request invalid.\n\"" ) );
					G_Printf("Player %i received bad request: %i with %i\n", clientNum, ent->client->sess.pendingtype, ent->client->sess.pendingvalue);
				break;
			}
			ent->client->sess.pendingtimeout = 0;
			ent->client->sess.pendingtype = 0;
			ent->client->sess.pendingvalue = 0;
		}
	}
	else if (Q_stricmp(cmd, "amreject") == 0)
	{
		if ((ent->client->sess.pendingtimeout == 0) || (ent->client->sess.pendingtype == 0))
		{
			ent->client->sess.pendingtimeout = 0;
			ent->client->sess.pendingtype = 0;
			ent->client->sess.pendingvalue = 0;
			trap_SendServerCmd( clientNum, va("print \"^1You have no pending requests.\n\"" ) );
			return;
		}
		else
		{
			switch (ent->client->sess.pendingtype)
			{
				case 1:
					{
						gentity_t	*other = &g_entities[ent->client->sess.pendingvalue];
						if (other && other->client && other->inuse)
						{
							trap_SendServerCmd( other->s.number, va("print \"^7%s^1 rejected your request.\n\"", ent->client->pers.netname ) );
						}
						break;
					}
				case 2:
					{
						gentity_t	*other = &g_entities[ent->client->sess.pendingvalue];
						if (other && other->client && other->inuse)
						{
							trap_SendServerCmd( other->s.number, va("print \"^7%s^1 rejected your request.\n\"", ent->client->pers.netname ) );
						}
						break;
					}
				default:
					break;
			}
			ent->client->sess.pendingtimeout = 0;
			ent->client->sess.pendingtype = 0;
			ent->client->sess.pendingvalue = 0;
			trap_SendServerCmd( clientNum, va("print \"^1Rejected request.\n\"" ) );
			return;
		}
	}
	else if (Q_stricmp(cmd, "ampassword") == 0)
	{
		strcpy(ent->client->sess.doorpassword,cmd_a2);
		return;
	}
	else if (Q_stricmp(cmd, "amignore") == 0)
	{
		twimod_admincmds(ent, clientNum, 117);
		return;
	}
	else if (Q_stricmp(cmd, "ampay") == 0)
	{
		//mc_pay(ent);
	int			targetNum;
	gentity_t		*target;
	int			fca;

	if (Q_stricmp(ent->client->sess.userlogged,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You are not logged in.\n\""));
		return;
	}

	if (Q_stricmp(cmd_a3,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1/ampay <player> <credits>\n\""));
		return;
	}

	targetNum = dsp_adminTarget(ent, cmd_a2, ent->s.number);
	if (targetNum < 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^7Invalid Target.\n\""));
		return;
	}
	if (targetNum == ent->s.number)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1You cannot pay yourself.\n\""));
		return;
	}
	target = &g_entities[targetNum];
	if (Q_stricmp(target->client->sess.userlogged,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^1The chosen player is not logged in.\n\""));
		return;
	}

	fca = atoi(cmd_a3);
	if (fca < 1)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^3Credit amount must be a positive number.\n\""));
		return;
	}
	if (fca > ent->client->sess.credits)
	{
		trap_SendServerCmd(ent->fixednum, va("print \"^3You do not have that many credits.\n\""));
		return;
	}
	trap_SendServerCmd(target->s.number, va("print \"^7%s^3 has payed you ^5%i^3 credits.\n\"", ent->client->pers.netname, fca));
	trap_SendServerCmd(ent->fixednum, va("print \"^3You have payed ^7%s^5 %i^3 credits.\n\"", target->client->pers.netname, fca));
	mc_addcredits(target, fca);
	mc_addcredits(ent, -fca);

		return;
	}
	else if ((Q_stricmp(cmd, "noteleport") == 0)||(Q_stricmp(cmd, "amnoteleport") == 0)||(Q_stricmp(cmd, "amnotele") == 0)||(Q_stricmp(cmd, "notele") == 0))
	{
		knowncmd = qtrue;
		setnoteleport(ent);
		return;
	}
	else if ((Q_stricmp(cmd, "amafk") == 0)||(Q_stricmp(cmd, "afk") == 0))
	{
		knowncmd = qtrue;
		setAFK(ent);
		return;
	}
	else if ((Q_stricmp(cmd, "amnumber") == 0)||(Q_stricmp(cmd, "amnumbers") == 0)||(Q_stricmp(cmd, "dcnumbers") == 0)||(Q_stricmp(cmd, "dcnumber") == 0)||(Q_stricmp(cmd, "amlist") == 0)||(Q_stricmp(cmd, "dclist") == 0))
	{
		knowncmd = qtrue;
		for (i = 0;i < 32;i += 1)
		{
			gentity_t *glorp = &g_entities[i];
			if (glorp && glorp->inuse && glorp->client && glorp->client->sess.stealth != 1)
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^7%i^1)^7%s^7(%s^7)\n\"", i, glorp->client->pers.netname, (Q_stricmp(glorp->client->sess.userlogged,"") == 0)?"Not Registered":glorp->client->sess.userlogged));
			}
		}
		return;
	}
	else if (Q_stricmp(cmd, "ammail") == 0)
	{
		knowncmd = qtrue;
		mailsys(ent, cmd_a2, cmd_a3);
		return;
	}
	else if (Q_stricmp(cmd, "amchannels") == 0)
	{
		knowncmd = qtrue;
		amchannels(ent, cmd_a2, cmd_a3, cmd_a4);
		return;
	}
	else if (Q_stricmp(cmd, "amteamchat") == 0)
	{
		knowncmd = qtrue;
		if (Q_stricmp(cmd_a2,"") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \"^1/amteamchat <mode>\nModes: 0: Normal Chat, 1: Admin Chat, 5: ScreenSay, 6: Announce, channelname: channelsay\n\""));
		}
		if (Q_stricmp(cmd_a2,"0") == 0)
		{
			ent->client->sess.teamchattype = 0;
		}
		else if (Q_stricmp(cmd_a2,"1") == 0)
		{
			ent->client->sess.teamchattype = 1;
		}
		else if (Q_stricmp(cmd_a2,"5") == 0)
		{
			ent->client->sess.teamchattype = 5;
		}
		else if (Q_stricmp(cmd_a2,"6") == 0)
		{
			ent->client->sess.teamchattype = 6;
		}
		else
		{
			int	ch;
			ch = channels_find(cmd_a2);
			if (ch < 0)
			{
				trap_SendServerCmd(clientNum, va("print \"^1Unknown channel.\n\""));
			}
			else
			{
				ent->client->sess.teamchattype = 100+ch;
			}
			
		}
		return;
	}
	else if (Q_stricmp(cmd, "engage_ff") == 0)
	{
		knowncmd = qtrue;
		Cmd_EngageDuel_f(ent, 1);
		return;
	}
	else if (Q_stricmp(cmd, "engage_fw") == 0)
	{
		knowncmd = qtrue;
		Cmd_EngageDuel_f(ent, 2);
		return;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	// EMOTES
	/////////////////////////////////////////////////////////////////////////////////////////////
	else if (Q_stricmp(cmd, "amsurrender") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, TORSO_SURRENDER_START);
		return;
	}
	else if (Q_stricmp(cmd, "amhug") == 0)
	{
		knowncmd = qtrue;
		DS_doHug(ent);
		return;
	}/*
	else if (Q_stricmp(cmd, "amhump") == 0)
	{
		knowncmd = qtrue;
		DS_doHump(ent);
		return;
	}*/
	else if (Q_stricmp(cmd, "amkiss") == 0)
	{
		knowncmd = qtrue;
		DS_doKiss(ent);
		return;
	}
	else if (Q_stricmp(cmd, "ampunch") == 0)
	{
		knowncmd = qtrue;
		DS_doPunch(ent);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_roll") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 15);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_flip") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 14);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_headshot") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 13);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_spin") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 12);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_fallforward") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 11);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_whoa") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 5);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_fallback") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 2);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_staydown") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 1);
		return;
	}
	else if (Q_stricmp(cmd, "amdie") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 1);
		return;
	}
	else if (Q_stricmp(cmd, "amtwitch") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 26);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_spinandfall") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 28);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_woah2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 29);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_spinback") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 30);
		return;
	}
	else if (Q_stricmp(cmd, "amswimdown") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 34);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_swimdown") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 36);
		return;
	}
	else if (Q_stricmp(cmd, "amgot_hit") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 98);
		return;
	}
	else if (Q_stricmp(cmd, "amstraighten") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 101);
		return;
	}
	else if (Q_stricmp(cmd, "amfistnod") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 102);
		return;
	}
	else if (Q_stricmp(cmd, "amfistidle") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 107);
		return;
	}
	else if (Q_stricmp(cmd, "amduck") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 109);
		return;
	}
	else if (Q_stricmp(cmd, "amgothit") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 116);
		return;
	}
	else if (Q_stricmp(cmd, "amhandout") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 118);
		return;
	}
	else if (Q_stricmp(cmd, "amaim") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 121);
		return;
	}
	else if (Q_stricmp(cmd, "amdropsaber") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 127);
		return;
	}
	else if (Q_stricmp(cmd, "amhit") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 130);
		return;
	}
	else if (Q_stricmp(cmd, "amhit2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 131);
		return;
	}
	else if (Q_stricmp(cmd, "amgrab") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 138);
		return;
	}
	else if (Q_stricmp(cmd, "amwipe") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 141);
		return;
	}
	else if (Q_stricmp(cmd, "amspin") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 146);
		return;
	}
	else if (Q_stricmp(cmd, "amdisco") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 150);
		return;
	}
	else if (Q_stricmp(cmd, "amspin2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 151);
		return;
	}
	else if (Q_stricmp(cmd, "amdisco3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 178);
		return;
	}
	else if (Q_stricmp(cmd, "amslashdown") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 292);
		return;
	}
	else if (Q_stricmp(cmd, "amdisco4") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 480);
		return;
	}
	else if (Q_stricmp(cmd, "amgot_hit2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 545);
		return;
	}
	else if (Q_stricmp(cmd, "amhop") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 550);
		return;
	}
	else if (Q_stricmp(cmd, "amsit1") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 595);
		return;
	}
	else if (Q_stricmp(cmd, "amsit") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 595);
		return;
	}
	else if (Q_stricmp(cmd, "amfixfeet") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 598);
		return;
	}
	else if (Q_stricmp(cmd, "amhereyougo") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 600);
		return;
	}
	else if (Q_stricmp(cmd, "amhereyougo2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 601);
		return;
	}
	else if (Q_stricmp(cmd, "amhandhips") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 603);
		return;
	}
	else if (Q_stricmp(cmd, "amflex") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 604);
		return;
	}
	else if (Q_stricmp(cmd, "amattention") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 605);
		return;
	}
	else if (Q_stricmp(cmd, "amtype") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 615);
		return;
	}
	else if (Q_stricmp(cmd, "amtype2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 616);
		return;
	}
	else if (Q_stricmp(cmd, "amsurrender") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 620);
		return;
	}
	else if (Q_stricmp(cmd, "amshowoff") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 626);
		return;
	}
	else if (Q_stricmp(cmd, "amcomwalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 629);
		return;
	}
	else if (Q_stricmp(cmd, "amgivesaber") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 630);
		return;
	}
	else if (Q_stricmp(cmd, "amcomtalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 633);
		return;
	}
	else if (Q_stricmp(cmd, "amemptyhands") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 635);
		return;
	}
	else if (Q_stricmp(cmd, "amtalkgesture") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 636);
		return;
	}
	else if (Q_stricmp(cmd, "amtalkgesture2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 637);
		return;
	}
	else if (Q_stricmp(cmd, "amtalkgesture3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 639);
		return;
	}
	else if (Q_stricmp(cmd, "amthink") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 641);
		return;
	}
	else if (Q_stricmp(cmd, "amcrossarms") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 655);
		return;
	}
	else if (Q_stricmp(cmd, "amsitgive") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 647);
		return;
	}
	else if (Q_stricmp(cmd, "amtouchchin") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 650);
		return;
	}
	else if (Q_stricmp(cmd, "amshrug") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 654);
		return;
	}
	else if (Q_stricmp(cmd, "amdrink") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 657);
		return;
	}
	else if (Q_stricmp(cmd, "amtalkgesture4") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 659);
		return;
	}
	else if (Q_stricmp(cmd, "amwave") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 661);
		return;
	}
	else if (Q_stricmp(cmd, "amcroucharm") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 662);
		return;
	}
	else if (Q_stricmp(cmd, "amcroucharm2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 663);
		return;
	}
	else if (Q_stricmp(cmd, "amshrug2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 665);
		return;
	}
	else if (Q_stricmp(cmd, "amwave2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 666);
		return;
	}
	else if (Q_stricmp(cmd, "amparanoid") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 667);
		return;
	}
	else if (Q_stricmp(cmd, "amthepower") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 668);
		return;
	}
	else if (Q_stricmp(cmd, "amflipoff") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 669);
		return;
	}
	else if (Q_stricmp(cmd, "amholdgun") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 673);
		return;
	}
	else if (Q_stricmp(cmd, "amwave3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 674);
		return;
	}
	else if (Q_stricmp(cmd, "amnod") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 680);
		return;
	}
	else if (Q_stricmp(cmd, "amshakehead") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 681);
		return;
	}
	else if (Q_stricmp(cmd, "amgrab2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 691);
		return;
	}
	else if (Q_stricmp(cmd, "ammantis") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 693);
		return;
	}
	else if (Q_stricmp(cmd, "amadjusthigh") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 694);
		return;
	}
	else if (Q_stricmp(cmd, "amadjustsit") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 697);
		return;
	}
	else if (Q_stricmp(cmd, "amsitthrow") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 698);
		return;
	}
	else if (Q_stricmp(cmd, "ambehindyou") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 700);
		return;
	}
	else if (Q_stricmp(cmd, "amthumbsdown") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 704);
		return;
	}
	else if (Q_stricmp(cmd, "amsuperman") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 707);
		return;
	}
	else if (Q_stricmp(cmd, "amsuperman2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 708);
		return;
	}
	else if (Q_stricmp(cmd, "amsuperman2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 709);
		return;
	}
	else if (Q_stricmp(cmd, "amsuper") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 710);
		return;
	}
	else if (Q_stricmp(cmd, "amoverthere") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 711);
		return;
	}
	else if (Q_stricmp(cmd, "amgrabhim") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 712);
		return;
	}
	else if (Q_stricmp(cmd, "amcrosslegs") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 714);
		return;
	}
	else if (Q_stricmp(cmd, "amoverthere2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 717);
		return;
	}
	else if (Q_stricmp(cmd, "amorganize") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 720);
		return;
	}
	else if (Q_stricmp(cmd, "amlookover") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 721);
		return;
	}
	else if (Q_stricmp(cmd, "ampulled") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 722);
		return;
	}
	else if (Q_stricmp(cmd, "ampulled2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 723);
		return;
	}
	else if (Q_stricmp(cmd, "ampulled3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 724);
		return;
	}
	else if (Q_stricmp(cmd, "amdisco2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 726);
		return;
	}
	else if (Q_stricmp(cmd, "amhandshakeestart") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, BOTH_HANDSHAKEE1START);
		return;
	}
	else if (Q_stricmp(cmd, "amhandshakerstart") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, BOTH_HANDSHAKER1START);
		return;
	}
	else if (Q_stricmp(cmd, "amhandshakeeloop") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, BOTH_HANDSHAKEE1LOOP);
		return;
	}
	else if (Q_stricmp(cmd, "amhandshakerloop") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, BOTH_HANDSHAKER1LOOP);
		return;
	}
	else if (Q_stricmp(cmd, "amlaugh") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 733);
		return;
	}
	else if (Q_stricmp(cmd, "amflipforward") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 734);
		return;
	}
	else if (Q_stricmp(cmd, "amkissair") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 745);
		return;
	}
	else if (Q_stricmp(cmd, "amfixobject") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 756);
		return;
	}
	else if (Q_stricmp(cmd, "amthrowback") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 757);
		return;
	}
	else if (Q_stricmp(cmd, "ampleaseno") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 758);
		return;
	}
	else if (Q_stricmp(cmd, "ampleaseno2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 759);
		return;
	}
	else if (Q_stricmp(cmd, "amtotheside") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 762);
		return;
	}
	else if (Q_stricmp(cmd, "amthrowforward") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 767);
		return;
	}
	else if (Q_stricmp(cmd, "amthrowside") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 769);
		return;
	}
	else if (Q_stricmp(cmd, "amgrabside") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 770);
		return;
	}
	else if (Q_stricmp(cmd, "amsit2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 778);
		return;
	}
	else if (Q_stricmp(cmd, "amsit3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 779);
		return;
	}
	else if (Q_stricmp(cmd, "amcrouchwalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 798);
		return;
	}
	else if (Q_stricmp(cmd, "amcrouchwalk2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 799);
		return;
	}
	else if (Q_stricmp(cmd, "amsitlisten") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 804);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_sudden") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 811);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_getbackup") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 812);
		return;
	}
	else if (Q_stricmp(cmd, "ambeg_lookup") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 815);
		return;
	}
	else if (Q_stricmp(cmd, "ambeg") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 814);
		return;
	}
	else if (Q_stricmp(cmd, "amplace") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 816);
		return;
	}
	else if (Q_stricmp(cmd, "amplace2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 820);
		return;
	}
	else if (Q_stricmp(cmd, "amslam") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 821);
		return;
	}
	else if (Q_stricmp(cmd, "amwalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 822);
		return;
	}
	else if (Q_stricmp(cmd, "amsneak") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 826);
		return;
	}
	else if (Q_stricmp(cmd, "amimpwalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 827);
		return;
	}
	else if (Q_stricmp(cmd, "amcrossarmswalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 830);
		return;
	}
	else if (Q_stricmp(cmd, "amthinkwalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 831);
		return;
	}
	else if (Q_stricmp(cmd, "amrun") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 833);
		return;
	}
	else if (Q_stricmp(cmd, "amsidewalk") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 838);
		return;
	}
	else if (Q_stricmp(cmd, "amsidewalk2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 839);
		return;
	}
	else if (Q_stricmp(cmd, "amsiderun") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 840);
		return;
	}
	else if (Q_stricmp(cmd, "amshakey") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 844);
		return;
	}
	else if (Q_stricmp(cmd, "amcrouchshakey") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 849);
		return;
	}
	else if (Q_stricmp(cmd, "amwalkback") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 852);
		return;
	}
	else if (Q_stricmp(cmd, "amrunback") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 854);
		return;
	}
	else if (Q_stricmp(cmd, "ambounce") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 856);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicslam") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 859);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicforward") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 869);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicside") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 875);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicside2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 878);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicroll") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 881);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicroll2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 882);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicroll3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 883);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicroll4") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 884);
		return;
	}
	else if (Q_stricmp(cmd, "amroll") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 885);
		return;
	}
	else if (Q_stricmp(cmd, "amroll2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 886);
		return;
	}
	else if (Q_stricmp(cmd, "amroll3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 887);
		return;
	}
	else if (Q_stricmp(cmd, "amroll4") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 888);
		return;
	}
	else if (Q_stricmp(cmd, "amdance") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 895);
		return;
	}
	else if (Q_stricmp(cmd, "amheyyou") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 904);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicflip") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 905);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicflip2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 906);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicflip3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 909);
		return;
	}
	else if (Q_stricmp(cmd, "ammagicflip4") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 910);
		return;
	}
	else if (Q_stricmp(cmd, "amspin3") == 0)
	{
		knowncmd = qtrue;
		if (ent->client->ps.weapon != WP_SABER)
		{
			trap_SendServerCmd(ent->fixednum, "print \"^1/amspin3 requires a lightsaber.\n\"");
			return;
		}
		dsp_doEmote(ent, 913);
		return;
	}
	else if (Q_stricmp(cmd, "amspin4") == 0)
	{
		knowncmd = qtrue;
		if (ent->client->ps.weapon != WP_SABER)
		{
			trap_SendServerCmd(ent->fixednum, "print \"^1/amspin4 requires a lightsaber.\n\"");
			return;
		}
		dsp_doEmote(ent, 914);
		return;
	}
	else if (Q_stricmp(cmd, "amwallrun") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 915);
		return;
	}
	else if (Q_stricmp(cmd, "amfallback") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 924);
		return;
	}
	else if (Q_stricmp(cmd, "amfallback2") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 925);
		return;
	}
	else if (Q_stricmp(cmd, "amfallforward") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 926);
		return;
	}
	else if (Q_stricmp(cmd, "amfallback3") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 928);
		return;
	}
	else if (Q_stricmp(cmd, "amflipkick") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 945);
		return;
	}
	else if (Q_stricmp(cmd, "amwhatthe") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 946);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_myarm") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 952);
		return;
	}
	else if (Q_stricmp(cmd, "amdie_why") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 958);
		return;
	}
	else if (Q_stricmp(cmd, "amswimforward") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 981);
		return;
	}
	else if (Q_stricmp(cmd, "amchoked") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 1030);
		return;
	}
	else if (Q_stricmp(cmd, "ampress") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 1037);
		return;
	}
	else if (Q_stricmp(cmd, "amfacepalm") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 1049);
		return;
	}
	else if (Q_stricmp(cmd, "ambigslap") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, 1059);
		return;
	}
	else if (Q_stricmp(cmd, "amwait") == 0)
	{
		knowncmd = qtrue;
		dsp_doEmote(ent, BOTH_STAND4);
		return;
	}
// ----------------------------- //
// Twimod RegisterSystem
// ----------------------------- //
	else if ((Q_stricmp (cmd, "amregister") == 0 ) || (Q_stricmp(cmd, "register") == 0))
{
	char username[MAX_NETNAME], password1[80], password2[80], usernameStripped[80], tmp[80];
	char			userfile[MAX_QPATH];
	int	i;
	int	iL;
	char			userwrite[MAX_QPATH];

	fileHandle_t	f;
	trap_Argv( 1, username, sizeof( username ) );
	trap_Argv( 2, password1, sizeof( password1 ) );
	trap_Argv( 3, password2, sizeof( password2 ) );
		rofl = clientNum;
		// Check we got a username and two passwords
		G_Printf("ClientCommand: %s: amregister %s <PasswordHidden>\n", ent->client->pers.netname, username);
		if ( !(strlen(username) && strlen(password1)) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amregister <username> <password> <password>\n\""));
			G_Printf("Refused registration for %s: not enough info.\n", ent->client->pers.netname);
			return;
		}

		// Check the user didnt creat another acc this connection
		if ( ent->client->sess.thisconnectuc == 1 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You are not allowed to create more than one users per connection.\n\""));
			G_Printf("Refused registration for %s: registration spam.\n", ent->client->pers.netname);
			return;
		}
		if (strchr(username, ' ') || strchr(password1, ' ') )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have spaces in your username or password.\n\""));
			G_Printf("Refused registration for %s: illegal character.\n", ent->client->pers.netname);
			return;
		}
		if (strchr(username, ':') || strchr(password1, ':'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have colons in your username or password.\n\""));
			G_Printf("Refused registration for %s: illegal character.\n", ent->client->pers.netname);
			return;
		}
		if (strchr(username, '/') || strchr(password1, '/'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have slashes in your username or password.\n\""));
			G_Printf("Refused registration for %s: illegal character.\n", ent->client->pers.netname);
			return;
		}
		if (strchr(username, '\\') || strchr(password1, '\\'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have slashes in your username or password.\n\""));
			G_Printf("Refused registration for %s: illegal character.\n", ent->client->pers.netname);
			return;
		}
		if (strchr(username, '.') || strchr(password1, '.'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have periods in your username or password.\n\""));
			G_Printf("Refused registration for %s: illegal character.\n", ent->client->pers.netname);
			return;
		}
		if (strchr(username, '\n') || strchr(password1, '\n'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have line skips in your username or password.\n\""));
			G_Printf("Refused registration for %s: illegal character.\n", ent->client->pers.netname);
			return;
		}
		iL = strlen(username);
		for (i = 0;i < iL;i += 1)
		{
			if (!((username[i] == 'A')||
				(username[i] == 'B')||
				(username[i] == 'C')||
				(username[i] == 'D')||
				(username[i] == 'E')||
				(username[i] == 'F')||
				(username[i] == 'G')||
				(username[i] == 'H')||
				(username[i] == 'I')||
				(username[i] == 'J')||
				(username[i] == 'K')||
				(username[i] == 'L')||
				(username[i] == 'M')||
				(username[i] == 'N')||
				(username[i] == 'O')||
				(username[i] == 'P')||
				(username[i] == 'Q')||
				(username[i] == 'R')||
				(username[i] == 'S')||
				(username[i] == 'T')||
				(username[i] == 'U')||
				(username[i] == 'V')||
				(username[i] == 'W')||
				(username[i] == 'X')||
				(username[i] == 'Y')||
				(username[i] == 'Z')||
				(username[i] == 'a')||
				(username[i] == 'b')||
				(username[i] == 'c')||
				(username[i] == 'd')||
				(username[i] == 'e')||
				(username[i] == 'f')||
				(username[i] == 'g')||
				(username[i] == 'h')||
				(username[i] == 'i')||
				(username[i] == 'j')||
				(username[i] == 'k')||
				(username[i] == 'l')||
				(username[i] == 'm')||
				(username[i] == 'n')||
				(username[i] == 'o')||
				(username[i] == 'p')||
				(username[i] == 'q')||
				(username[i] == 'r')||
				(username[i] == 's')||
				(username[i] == 't')||
				(username[i] == 'u')||
				(username[i] == 'v')||
				(username[i] == 'w')||
				(username[i] == 'x')||
				(username[i] == 'y')||
				(username[i] == 'z')||
				(username[i] == '_')||
				(username[i] == '-')||
				(username[i] == '1')||
				(username[i] == '2')||
				(username[i] == '3')||
				(username[i] == '4')||
				(username[i] == '5')||
				(username[i] == '6')||
				(username[i] == '7')||
				(username[i] == '8')||
				(username[i] == '9')||
				(username[i] == '0')||
				(username[i] == '_')||
				(username[i] == '-')||
				(username[i] == '')||
				(username[i] == '^')))
				{
					trap_SendServerCmd( clientNum, va("print \"^1Refused character ^5%s^1 in your username.\n\"", username[i]));
					G_Printf("Refused registration for %s: Bad symbol.\n", ent->client->pers.netname);
					return;
				}
		}
		if (Q_stricmp(password1,username) == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Password cannot match username.\n\""));
			G_Printf("Refused registration for %s: password matches username.\n", ent->client->pers.netname);
			return;
		}

		if (strstr(username,password1))
		{
			trap_SendServerCmd( clientNum, va("print \"^1Password cannot be part of your username.\n\""));
			G_Printf("Refused registration for %s: password is part of username.\n", ent->client->pers.netname);
			return;
		}
		if (strlen(username) > 50 || strlen(password1) > 50)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Username and password must be 50 characters or less.\n\""));
			G_Printf("Refused registration for %s: username or password too long.\n", ent->client->pers.netname);
			return;
		}
		
		if (strstr(password1, username))
		{
			trap_SendServerCmd( clientNum, va("print \"^1Password cannot contain part of your username.\n\""));
			G_Printf("Refused registration for %s: password contains part of username.\n", ent->client->pers.netname);
			return;
		}
		if (Q_stricmp(username,"") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Username cannot be blank.\n\""));
			G_Printf("Refused registration for %s: blank username.\n", ent->client->pers.netname);
			return;
		}
		if (Q_stricmp(password1,"") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Password cannot be blank.\n\""));
			G_Printf("Refused registration for %s: blank password.\n", ent->client->pers.netname);
			return;
		}
		// Check that the passwords match.
		if ( Q_stricmp(password1, password2) != 0)
		{
			//trap_SendServerCmd( clientNum, va("print \"^1Passwords didn't match.\n\""));
			//G_Printf("Refused registration for %s: passwords didn't match.\n", ent->client->pers.netname);
			//return;
		}

		// Check that username doesn't exist.
		Com_sprintf(userfile, 1024*4, "%s/account_1_%s.cfg", mc_userfolder.string, username);
		mc_lower(userfile);
		trap_FS_FOpenFile(userfile, &f, FS_READ);

		if ( f )
			{
				// We found this username, so we inform the user.
				trap_SendServerCmd( clientNum, va("print \"^1User '%s^1' already exists.\n\"", username));
			G_Printf("Refused registration for %s: username already exists", ent->client->pers.netname);
				trap_FS_FCloseFile( f );
				return;
			}
		else if ( !f )
			{
				// We are OK to create the user
			//	trap_FS_FCloseFile( f );
				//trap_FS_FOpenFile(userfile, &f, FS_APPEND);
				//Com_sprintf( userwrite, sizeof(userwrite), "%s 0 %s 0 ", password1, ent->client->pers.netname);
				//trap_FS_Write( userwrite, strlen(userwrite), f);
				//trap_FS_FCloseFile( f );

				ent->client->sess.credits = 0;
				//ent->client->sess.builderadmin = 0;
				ent->client->sess.ampowers = 0;
				ent->client->sess.ampowers2 = 0;
				ent->client->sess.ampowers3 = 0;
				ent->client->sess.ampowers4 = 0;
				ent->client->sess.ampowers5 = 0;
				ent->client->sess.ampowers6 = 0;
				ent->client->sess.ampowers7 = 0;
				strcpy(ent->client->sess.mygroup,"000000000");
				ent->client->sess.adminloggedin = 0;
				stringclear(ent->client->sess.userlogged, 600);
				stringclear(ent->client->sess.userpass, 600);
				Q_strncpyz(ent->client->sess.userlogged, username, sizeof(ent->client->sess.userlogged));
				strcpy(ent->client->sess.userpass, password1);
				trap_SendServerCmd( clientNum, va("print \"^1User '%s^1' successfully created.\n\"", username));
				ent->client->sess.thisconnectuc = 1;
				if (level.thisistpm == 32)
				{
					ent->client->sess.adminloggedin = 1;
				}
				mc_updateaccount(ent);
				// We automatical login the user now
				//strcpy(ent->client->sess.userlogged, username);

							if ( twimod_loginpuplicmsg.integer == 1 )
								{
							trap_SendServerCmd( -1, va("print \"%s ^7has registered and is now logged in as %s^7.\n\"", ent->client->pers.netname, ent->client->sess.userlogged));
								}
							else if ( twimod_loginpuplicmsg.integer == 2 )
								{
							trap_SendServerCmd( -1, va("cp \"%s ^7has registered and is now logged in as %s^7.\n\"", ent->client->pers.netname, ent->client->sess.userlogged));
							trap_SendServerCmd( clientNum, va("print \"You are now logged in as %s^7.\n\"", username));
								}
							else
								{
							trap_SendServerCmd( clientNum, va("print \"You are now logged in as %s^7.\n\"", username));
								}
			}
}



	else if ((Q_stricmp (cmd, "amlogin") == 0 ))
{
	char username[MAX_NETNAME];
	char password[1024] = "";
	char passwordfile[1024] = "";
	char loginlevel[1024] = "";
	char creds[1024] = "";
	char mcbuildadmin[1024] = "";
	char mcampower[1024] = "";
	char mcampower2[1024] = "";
	char mcampower3[1024] = "";
	char mcampower4[1024] = "";
	char mcampower5[1024] = "";
	char mcampower6[1024] = "";
	char mcampower7[1024] = "";
	char mcamgroup[1024] = "";
	char	userinfo[MAX_INFO_VALUE];
	int len;
	int i = 0;
	int o = 0;
	int c = 0;
	int g = 0;
	int fedisch = 0;

	char buffer[4096] = "";
	char			userfile[MAX_QPATH];
	fileHandle_t	f;
	trap_Argv( 1, username, sizeof( username ) );
	trap_Argv( 2, password, sizeof( password ) );
	rofl = clientNum;

		G_Printf("ClientCommand: %s: amlogin %s <PasswordHidden>\n", ent->client->pers.netname, username);
		// Check we got a username and a password
		if ( !(strlen(username) && strlen(password) ))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amlogin <username> <password>\n\""));
			G_Printf(va("Refused login for %s as %s: not enough info.\n", ent->client->pers.netname, username));
			return;
		}

		if (Q_stricmp(ent->client->sess.userlogged, "") != 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1You are already logged in.\n\""));
			G_Printf(va("Refused login for %s as %s: already logged in.\n", ent->client->pers.netname, username));
			return;
		}

		if ( ent->client->sess.logintrys >= 5 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1Login refused: Too many failed logins.\n\""));
			G_Printf(va("Refused login for %s as %s: too many failed logins.\n", ent->client->pers.netname, username));
			return;
		}
		for (i = 0;i < 32;i += 1)
		{
			gentity_t	*flent = &g_entities[i];
			if (flent && flent->inuse)
			{
			if (Q_stricmp(username,flent->client->sess.userlogged) == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1Player ^7%s^1 is already logged in as that account.\n\"", flent->client->pers.netname));
				G_Printf(va("Refused login for %s as %s: other player logged in as same user.\n", ent->client->pers.netname, username));
				return;
			}
			}
		}
		i = 0;

		if (strchr(username, ' ') || strchr(password, ' '))
		{
			// User is not allowed to have spaces in his name or in his password...
			trap_SendServerCmd( clientNum, va("print \"^1Usernames and passwords do not include spaces.\n\""));
			G_Printf(va("Refused login for %s as %s: illegal character\n", ent->client->pers.netname, username));
			return;
		}
		if (strchr(username,':') || strchr(password,':'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1Usernames and passwords do not include colons.\n\""));
			G_Printf(va("Refused login for %s as %s: illegal character\n", ent->client->pers.netname, username));
			return;
		}
		if (strchr(username,'.') || strchr(password,'.'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1Usernames and passwords do not include periods.\n\""));
			G_Printf(va("Refused login for %s as %s: illegal character\n", ent->client->pers.netname, username));
			return;
		}
		if (strchr(username,'/') || strchr(password,'/'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1Usernames and passwords do not include slashes.\n\""));
			G_Printf(va("Refused login for %s as %s: illegal character\n", ent->client->pers.netname, username));
			return;
		}
		if (strchr(username,'\\') || strchr(password,'\\'))
		{
			trap_SendServerCmd( clientNum, va("print \"^1Usernames and passwords do not include slashes.\n\""));
			G_Printf(va("Refused login for %s as %s: illegal character\n", ent->client->pers.netname, username));
			return;
		}
		ent->client->sess.logintrys++;
		// Check that the username exist.
		Com_sprintf(userfile, 1024*4, "%s/account_1_%s.cfg", mc_userfolder.string, username);
		mc_lower(userfile);
		trap_FS_FOpenFile(userfile, &f, FS_READ);

		if ( f )
			{
				// We found this username, so we check the password!
				trap_FS_FCloseFile( f );
				len = trap_FS_FOpenFile(userfile, &f, FS_READ);
				trap_FS_Read( buffer ,len , f );
		for ( i = 0; i <= strlen (buffer); i++ )
		{
			if ((buffer[i] == '\n'))
			{
				c++;
				if ( c == 1 )
					memcpy (passwordfile, buffer, i);
				else if ( c == 2 )
					memcpy (loginlevel, buffer+o+1, i-o-1);
				//else if ( c == 3 )
				else if ( c == 4 )
					memcpy (creds, buffer+o+1, i-o-1);
				else if ( c == 5 )
					memcpy (mcampower, buffer+o+1, i-o-1);
				else if ( c == 6 )
					memcpy (mcampower2, buffer+o+1, i-o-1);
				else if ( c == 7 )
					memcpy (mcampower3, buffer+o+1, i-o-1);
				else if ( c == 8 )
					memcpy (mcampower4, buffer+o+1, i-o-1);
				else if ( c == 9 )
					memcpy (mcamgroup, buffer+o+1, i-o-1);
				else if ( c == 10 )
					memcpy (mcampower5, buffer+o+1, i-o-1);
				else if ( c == 11 )
					memcpy (mcampower6, buffer+o+1, i-o-1);
				else if ( c == 12 )
					memcpy (mcampower7, buffer+o+1, i-o-1);
				o = i;
			}
		}
	i = 0;
	g = 0;
	o = 0;
	c = 0;

		/*for ( i = 0; i <= strlen (buffer); i++ )
		{
			if ( buffer[i] == ' ' )
			{
				g++;
				if ( g >= 2 )
					{
						for ( i = i; i <= strlen (buffer); i++ )
						{
							if ( buffer[i] == '²' )
								{
									memcpy (namenoch, buffer+o+1, i-o-1);
								}
							else
								{
									continue;
								}
								o = i;
						}
					}
					o = i;
			}
		}*/
	i = 0;
	g = 0;
	o = 0;
	c = 0;
	//c = strlen (namenoch);
	/*if ( fedisch == 0 )
		{
		for ( i = 0; i <= strlen (namenoch); i++ )
		{
			if ( ( namenoch[i] == ' ' ) && ( fedisch == 0 ) )
			{
				memcpy (namesollte, namenoch+i+1 , c );
				fedisch = 1;
			}
		}
	}*/

		trap_FS_FCloseFile(f);
				if (Q_stricmp(password, passwordfile) == 0)
					{
						ent->client->sess.logintrys = 0;
						strcpy(ent->client->sess.userlogged, username);
						// We force the old netname..
			strcpy(ent->client->sess.userpass, password);
			clientNum = rofl;
			ent = g_entities + clientNum;
			//trap_GetUserinfo( clientNum, userinfo, MAX_INFO_STRING );
			//clientNum = rofl;
			//ent = g_entities + clientNum;
			//Com_sprintf( ent->client->pers.netname, sizeof( ent->client->pers.netname ), namesollte );
			//clientNum = rofl;
			//ent = g_entities + clientNum;
			//Info_SetValueForKey( userinfo, "name", namesollte );
			//clientNum = rofl;
			//ent = g_entities + clientNum;
			//trap_SetUserinfo( clientNum, userinfo );
			//clientNum = rofl;
			//ent = g_entities + clientNum;
			//ClientUserinfoChanged( clientNum );
			//clientNum = rofl;
			//ent = g_entities + clientNum;
			ent->client->sess.logintrys = 0;
			ent->client->sess.credits = atoi(creds);
			//ent->client->sess.builderadmin = atoi(mcbuildadmin);
			ent->client->sess.ampowers = atoi(mcampower);
			ent->client->sess.ampowers2 = atoi(mcampower2);
			ent->client->sess.ampowers3 = atoi(mcampower3);
			ent->client->sess.ampowers4 = atoi(mcampower4);
			ent->client->sess.ampowers5 = atoi(mcampower5);
			ent->client->sess.ampowers6 = atoi(mcampower6);
			ent->client->sess.ampowers7 = atoi(mcampower7);
			ent->client->sess.mcgroup = atoi(mcamgroup);
			//ent->client->sess.mygroup = mcamgroup;
			for (i = 0;i < 10;i += 1)
			{
				if (mcamgroup[i] == '')
				{
					mcamgroup[i] = '0';
				}
			}
			strcpy(ent->client->sess.mygroup, mcamgroup);
					if (Q_stricmp(loginlevel, "0") == 0)
						{
							ent->client->sess.adminloggedin = 0;

							if ( twimod_loginpuplicmsg.integer == 1 )
								{
							trap_SendServerCmd( -1, va("print \"%s ^7is now logged in as %s^7.\n\"", ent->client->pers.netname, ent->client->sess.userlogged));
								}
							else if ( twimod_loginpuplicmsg.integer == 2 )
								{
							trap_SendServerCmd( -1, va("cp \"%s ^7is now logged in as %s^7.\n\"", ent->client->pers.netname, ent->client->sess.userlogged));
							trap_SendServerCmd( clientNum, va("print \"You are now logged in as %s^7.\n\"", username));
								}
							else if ( twimod_loginpuplicmsg.integer == 0 )
								{
							trap_SendServerCmd( clientNum, va("print \"You are now logged in as %s^7.\n\"", username));
								}

						}
						else if (atoi(loginlevel) > mc_max_admin_rank.integer)
						{
							ent->client->sess.adminloggedin = mc_max_admin_rank.integer;

							if ( twimod_loginpuplicmsg.integer == 1 )
								{
							trap_SendServerCmd( -1, va("print \"%s ^7is now logged in as %s ^7and as a %s ^7admin.\n\"", ent->client->pers.netname, ent->client->sess.userlogged, stringforrank(mc_max_admin_rank.integer)));
								}
							else if ( twimod_loginpuplicmsg.integer == 2 )
								{
							trap_SendServerCmd( -1, va("cp \"%s ^7is now logged in as %s ^7and as a %s ^7admin.\n\"", ent->client->pers.netname, ent->client->sess.userlogged, stringforrank(mc_max_admin_rank.integer)));
							trap_SendServerCmd( clientNum, va("print \"You are now logged in as %s ^7and as a %s ^7admin.\n\"", username, stringforrank(mc_max_admin_rank.integer)));
								}
							else if ( twimod_loginpuplicmsg.integer == 0 )
								{
							trap_SendServerCmd( clientNum, va("print \"You are now logged in as %s ^7and as a %s ^7admin.\n\"", username, stringforrank(mc_max_admin_rank.integer)));
								}
						}
						else
						{
							ent->client->sess.adminloggedin = atoi(loginlevel);

							if ( twimod_loginpuplicmsg.integer == 1 )
								{
							trap_SendServerCmd( -1, va("print \"^7%s ^7is now logged in as %s ^7and as a %s ^7admin.\n\"", ent->client->pers.netname, ent->client->sess.userlogged, stringforrank(ent->client->sess.adminloggedin)));
								}
							else if ( twimod_loginpuplicmsg.integer == 2 )
								{
							trap_SendServerCmd( -1, va("cp \"^7%s ^7is now logged in as %s ^7and as a %s ^7admin.\n\"", ent->client->pers.netname, ent->client->sess.userlogged, stringforrank(ent->client->sess.adminloggedin)));
							trap_SendServerCmd( clientNum, va("print \"^7You are now logged in as %s ^7and as a %s ^7admin.\n\"", username, stringforrank(ent->client->sess.adminloggedin)));
								}
							else if ( twimod_loginpuplicmsg.integer == 0 )
								{
							trap_SendServerCmd( clientNum, va("print \"^7You are now logged in as %s ^7and as a %s ^7admin.\n\"", username, stringforrank(ent->client->sess.adminloggedin)));
								}
						}
						len = trap_FS_FOpenFile(va("%s/mail_1_%s.cfg", mc_userfolder.string, username), &f, FS_READ);
						if (f)
						{
							trap_FS_FCloseFile( f );
						}
						if (len > 2)
						{
							trap_SendServerCmd( clientNum, va("print \"^2You have mail!\n^2/ammail to read it.\n\""));
						}


						mc_updateaccount(ent);
					}
					else
					{
						clientNum = rofl;
						ent = g_entities + clientNum;
						trap_SendServerCmd( clientNum, va("print \"^1Wrong password.\n\"", username));
			G_Printf(va("Refused login for %s as %s: bad password\n", ent->client->pers.netname, username));
					}
			}
		else if ( !f )
			{
				clientNum = rofl;
				ent = g_entities + clientNum;
				// User does not exist, so we inform the client.
				trap_FS_FCloseFile( f );
				trap_SendServerCmd( clientNum, va("print \"^1User '%s^1' does not exist.\n\"", username));
			G_Printf(va("Refused login for %s as %s: nonexistant account\n", ent->client->pers.netname, username));
			}
}



else if ((Q_stricmp (cmd, "amnewpass") == 0 ))
	{
		char			line[256];
		char			oldpass[1024];
		char			userwrite[2048];
		char			newpass[1024];
	trap_Argv( 2, oldpass, sizeof( oldpass ) );
	trap_Argv( 1, newpass, sizeof( newpass ) );
		rofl = clientNum;
				clientNum = rofl;
			ent = g_entities + clientNum;
		G_Printf("ClientCommand: %s: amnewpass <PasswordHidden>\n", ent->client->pers.netname);

		if (/* !(strlen(oldpass) && */!strlen(newpass/*)*/ ))
		{
			trap_SendServerCmd( clientNum, va("print \"^1/amnewpass <newpassword>\n\""));
			return;
		}

		if ( (Q_stricmp(ent->client->sess.userlogged, "") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You are not logged in.\n\""));
			return;
		}

		if (Q_stricmp(newpass, "") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \"^1Password can't be blank.\n\""));
			return;
		}
		if (Q_stricmp(newpass, ent->client->sess.userlogged) == 0)
		{
			trap_SendServerCmd(clientNum, va("print \"^1Password can't match username.\n\""));
			return;
		}
		if (strstr(newpass, ent->client->sess.userlogged))
		{
			trap_SendServerCmd(clientNum, va("print \"^1Password can't contain username.\n\""));
			return;
		}
		if (strstr(ent->client->sess.userlogged, newpass))
		{
			trap_SendServerCmd(clientNum, va("print \"^1Password can't be part of username.\n\""));
			return;
		}
		if (/*strchr(oldpass, ' ') || */strchr(newpass, ' '))
		{
			trap_SendServerCmd( clientNum, va("print \"^1You may not have spaces in your new password\n\""));
			return;
		}

		/*if (Q_stricmp(oldpass, ent->client->sess.userpass) != 0)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Wrong Password.\n\""));
			return;
		}*/


				strcpy(ent->client->sess.userpass, newpass);
				mc_updateaccount(ent);
				trap_SendServerCmd( clientNum, va("print \"^1Password changed.\n\""));
				G_Printf("Successfully changed password.\n");


	}
	else if ((Q_stricmp(cmd, "amlogout") == 0))
	{
		mclogout(ent);
		/*if ( (Q_stricmp(ent->client->sess.userlogged, "") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You are not logged in.\n\""));
		}
		else
		{
			ent->client->sess.clanCounter = level.time + 1000;
			ent->client->sess.clanTagTime = level.time + twimod_clantagpcounter.integer;
			Q_strncpyz(ent->client->sess.userlogged, "", sizeof(ent->client->sess.userlogged));
			ent->client->sess.adminloggedin = 0;
			ent->client->sess.credits = 0;
			ent->client->sess.ampowers = 0;
			ent->client->sess.ampowers2 = 0;
			ent->client->sess.ampowers3 = 0;
			ent->client->sess.ampowers4 = 0;
			ent->client->sess.ampowers5 = 0;
			ent->client->sess.ampowers6 = 0;
			ent->client->sess.mcgroup = 0;
			trap_SendServerCmd( clientNum, va("print \"^1You are now logged out.\n\""));
		}*/
  	}
	else if ((Q_stricmp(cmd, "amaccount") == 0))
	{
		int needed;
		needed = 0;
		if ( (Q_stricmp(ent->client->sess.userlogged, "") == 0) )
		{
			trap_SendServerCmd( clientNum, va("print \"^1You are not logged in.\n\""));
			return;
		}
		trap_SendServerCmd( clientNum, va("print \"^4Twi^1Fire\n^3Account info:\n^5Username^2: ^7%s\n^5Credits: %i\n\"", ent->client->sess.userlogged, ent->client->sess.credits));
		if ( Q_stricmp(ent->client->sess.mygroup, "") != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Group^2: ^7%s\n\"", ent->client->sess.mygroup));
		}
		if ( ent->client->sess.ampowers != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 1^2: ^7%i\n\"", ent->client->sess.ampowers));
		}
		if ( ent->client->sess.ampowers2 != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 2^2: ^7%i\n\"", ent->client->sess.ampowers2));
		}
		if ( ent->client->sess.ampowers3 != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 3^2: ^7%i\n\"", ent->client->sess.ampowers3));
		}
		if ( ent->client->sess.ampowers4 != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 4^2: ^7%i\n\"", ent->client->sess.ampowers4));
		}
		if ( ent->client->sess.ampowers5 != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 5^2: ^7%i\n\"", ent->client->sess.ampowers5));
		}
		if ( ent->client->sess.ampowers6 != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 6^2: ^7%i\n\"", ent->client->sess.ampowers6));
		}
		if ( ent->client->sess.ampowers7 != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Power Set 7^2: ^7%i\n\"", ent->client->sess.ampowers7));
		}
		if ( ent->client->sess.mcspeed != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Speed^2: ^7%i\n\"", ent->client->sess.mcspeed));
		}
		if ( ent->client->sess.mcgravity != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Gravity^2: ^7%i\n\"", ent->client->sess.mcgravity));
		}
		if ( ent->client->sess.jetfuel != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Jet Fuel^2: ^7%i\n\"", ent->client->sess.jetfuel));
		}
		if ( ent->client->sess.mcgroup != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Group^2: ^7%i\n\"", ent->client->sess.mcgroup));
		}
		if (Q_stricmp(ent->client->sess.amprefix, "") != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Prefix^2: ^7%s\n\"", ent->client->sess.amprefix));
		}
		if (Q_stricmp(ent->client->sess.amsuffix, "") != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5Suffix^2: ^7%s\n\"", ent->client->sess.amsuffix));
		}/*
		if ( ent->client->sess.adminloggedin == 1 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", twimod_lvl1name.string));
		}
		if ( ent->client->sess.adminloggedin == 2 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", twimod_lvl2name.string));
		}
		if ( ent->client->sess.adminloggedin == 3 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", twimod_lvl3name.string));
		}
		if ( ent->client->sess.adminloggedin == 4 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", twimod_lvl4name.string));
		}
		if ( ent->client->sess.adminloggedin == 5 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", twimod_lvl5name.string));
		}
		if ( ent->client->sess.adminloggedin == 6 )
		{
			trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", twimod_lvl6name.string));
		}*/
		trap_SendServerCmd( clientNum, va("print \"^1^5admin Rank^2: ^7%s\n\"", stringforrank(ent->client->sess.adminloggedin)));
		if ( ent->client->sess.sleep )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(sleeping) \""));
			needed = 1;
		}
		if ( ent->client->sess.punish )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(punished) \""));
			needed = 1;
		}
		if ( ent->client->sess.silence )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(silenced) \""));
			needed = 1;
		}
		if ( ent->client->sess.protect )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(protected) \""));
			needed = 1;
		}
		if ( ent->client->sess.cheat )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(cheating) \""));
			needed = 1;
		}
		if ( ent->client->sess.freeze )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(frozen) \""));
			needed = 1;
		}
		if ( ent->client->ps.duelInProgress )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(dueling) \""));
			needed = 1;
		}
		if ( ent->client->sess.empower && ent->client->sess.terminator )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(allpowerful) \""));
			needed = 1;
		}
		else
		{
			if ( ent->client->sess.empower )
			{
				trap_SendServerCmd( clientNum, va("print \"^7(empowered) \""));
				needed = 1;
			}
			if ( ent->client->sess.terminator )
			{
				trap_SendServerCmd( clientNum, va("print \"^7(terminator) \""));
				needed = 1;
			}
		}
		if ( ent->client->noclip != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(noclipped) \""));
			needed = 1;
		}
		if ( ent->flags & FL_GODMODE )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(godmode) \""));
			needed = 1;
		}
		if ( ent->client->sess.noteleport != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(noteleport) \""));
			needed = 1;
		}
		if ( ent->client->sess.forcegod != 0 )
		{
			trap_SendServerCmd( clientNum, va("print \"^7(forcegod) \""));
			needed = 1;
		}
		if ( needed == 1 )
		{
			trap_SendServerCmd( clientNum, va("print \"\n\""));
		}
  	}
	else if ((Q_stricmp(cmd, "ammakeadmin") == 0))
	{
		knowncmd = qtrue;
			if (candocommand(ent, 1, 1))
			{
				twimod_admincmds(ent, clientNum, 50);
				return;
			}
	}
	else if ((Q_stricmp(cmd, "amprotect") == 0))
	{
		knowncmd = qtrue;
			if (candocommand(ent, 1, 2))
			{
							twimod_admincmds(ent, clientNum, 1);
							return;
			}
	}
	else if ((Q_stricmp(cmd, "amshowfuel") == 0))
	{
		knowncmd = qtrue;
		trap_SendServerCmd( ent-g_entities, va("cp \"\n\n\n\n\n\n\n^3Jet Fuel: ^5%i^3.\n\"",ent->client->sess.jetfuel));
		return;
	}
	else if ((Q_stricmp(cmd, "amalwaysshowfuel") == 0))
	{
		knowncmd = qtrue;
		if (ent->client->sess.jetshowfuel == 0)
		{
			ent->client->sess.jetshowfuel = 1;
			trap_SendServerCmd( ent-g_entities, va("print \"Jet Fuel will now be shown.\n\""));
		}
		else
		{
			ent->client->sess.jetshowfuel = 0;
			trap_SendServerCmd( ent-g_entities, va("print \"Jet Fuel will no longer be shown.\n\""));
		}
		return;
	}
	else if ((Q_stricmp(cmd, "amkick") == 0)||(Q_stricmp(cmd, "amGTFO") == 0))
	{
		knowncmd = qtrue;
			if (candocommand(ent, 1, 8))
			{
							twimod_admincmds(ent, clientNum, 2);
							return;
			}
	}
	else if ((Q_stricmp(cmd, "amunprotect") == 0))
	{
				knowncmd = qtrue;
			if (candocommand(ent, 1, 2))
			{
							twimod_admincmds(ent, clientNum, 3);
							return;
			}
	}
		else if ((Q_stricmp(cmd, "amjetpack") == 0))
		{
			if (candocommand(ent, 7, 8192))
			{
				if ( ent->client->sess.jetpackon == 0 && !ent->client->sess.sleep && !ent->client->sess.freeze && !ent->client->sess.punish )
				{
					ent->client->sess.jetpackon = 1;
					trap_SendServerCmd( clientNum, va("print \"Jetpack on.\n\"" ) );
				}
				else
				{
					ent->client->sess.jetpackon = 0;
					trap_SendServerCmd( clientNum, va("print \"Jetpack off.\n\"" ) );
					ent->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
					ent->client->ps.pm_type = PM_NORMAL;
				}
			}
		}
		else if ((Q_stricmp(cmd, "amban") == 0))
		{
			if (candocommand(ent, 1, 16))
			{
							knowncmd = qtrue;
							twimod_admincmds(ent, clientNum, 4);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "ambanrange") == 0))
		{
				knowncmd = qtrue;
			if (candocommand(ent, 1, 32))
			{
							twimod_admincmds(ent, clientNum, 5);
							return;
			}
	}
		else if ((Q_stricmp(cmd, "amhelp") == 0) || (Q_stricmp(cmd, "help") == 0) || (Q_stricmp(cmd, "emhelp") == 0) || (Q_stricmp(cmd, "dchelp") == 0)||
		(Q_stricmp(cmd, "aminfo") == 0) || (Q_stricmp(cmd, "info") == 0) || (Q_stricmp(cmd, "eminfo") == 0) || (Q_stricmp(cmd, "dcinfo") == 0))
	{
		knowncmd = qtrue;
		if ((mc_customhelp_name.string && (Q_stricmp(mc_customhelp_name.string,"") != 0))&&Q_stricmp(cmd_a2,mc_customhelp_name.string) == 0)
		{
			char	fixhelp[1024];
			int	iFH;
			int	iFH2;
			iFH = 0;
			iFH2 = 0;
			stringclear(fixhelp, 1020);
			for (iFH = 0;iFH < 1020;iFH += 1)
			{
				if (mc_customhelp_info.string[iFH] == '')
				{
					break;
				}
				if ((iFH > 0)&&(mc_customhelp_info.string[iFH] == 'n')&&(mc_customhelp_info.string[iFH-1] == '\\'))
				{
					fixhelp[iFH2-1] = '\n';
					continue;
				}
				fixhelp[iFH2] = mc_customhelp_info.string[iFH];
				iFH2 += 1;
			}
			trap_SendServerCommand(ent->s.number, va("print \"%s\n\"", fixhelp));
		}
		else if (Q_stricmp(cmd_a2,"chatcommands") == 0)
		{
			int	i;
			int	nums = 1;
			for (i = 0;i < 512;i += 1)
			{
				gteleporter_t	*tele = &g_teleporters[i];
				if (tele && (tele->active == 1))
				{
						trap_SendServerCmd(ent->fixednum, va("print \"^5%i^2)^7 %s\n\"", nums, tele->name));
						nums += 1;
				}
			}
			if (nums == 1)
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^1No chatcommands found.\n\""));
			}
		}
		else if (Q_stricmp(cmd_a2,"contact") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^2JK2 Name^7: ^2MCMONKEY\n"
			"^2Website^7: https://github.com/mcmonkeyprojects/Twifire\n"
			"^2Discord^7: mcmonkey#6666\n"
			"^2Email^7: mcmonkey4eva@hotmail.com\n"
			"\n"
			"\"" ));
		}
		else if (Q_stricmp(cmd_a2,"accounts") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^7TwiFire accounts are used for storing admin powers, credits, and some other data. They also can be used to prove who you are.\n"
			"^7To get an account, use ^5/amregister <username> <password>^7 - after you've created an account, you are automatically logged in.\n"
			"^7To login when you return to a server later, use ^5/amlogin <username> <password>^7.\n"
			"^7To see what information your account stores and some of your current data, use ^5/amaccount^7.\n"
			"^7To change your password, use ^5/amnewpass <password>^7.\n"
			"^7Remember, there's no way to guarantee password security on a Twifire server. Make sure your password here is different from passwords you use anywhere else!\n"
			"\"" ));
		}
		else if (Q_stricmp(cmd_a2,"credits") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^7If the server has enabled it, logged in users can receive or lose 'credits'.\n"
			"^7Credits are game money. They can be used for purchasing weapons or powerups.\n"
			"^7You receive credits for killing players, winning duels, or however else the server has set it.\n"
			"^7To view how many credits you have, use ^5/ambalance^7.\n"
			"^7To pay another online player, use ^5/ampay <player> <credit amount>^7.\n"
			"\"" ));
		}
		else if (Q_stricmp(cmd_a2,"privacy") == 0)
		{
			vmCvar_t	Developer;
    			trap_Cvar_Register( &Developer, "developer", "0",  CVAR_ARCHIVE );
			trap_SendServerCmd(clientNum, va("print \""
			"^3All data sent to and from a JK2 server can be read by high admins.\n"
			"^3Twifire attempts to limit how much they see - but it does not stop it.\n"
			"^3Never use the same password in a JK2 server as any other location.\n"
			"^3Never send private information in a PM or channel.\n"
			"\"" ));
			if (Developer.integer > 0)
			{
				trap_SendServerCmd(clientNum, va("print \""
				"^1Warning: This server is monitoring PMs.\n\n"
				"\"" ));
			}
			else
			{
				trap_SendServerCmd(clientNum, va("print \""
				"^2This server appears to not be monitoring PMs - but still be careful.\n\n"
				"\"" ));
			}
		}
		else if (Q_stricmp(cmd_a2,"mod") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^4Twi^1Fire ^7Mod Info^2:\n"
			"^4Twi^1Fire ^2mod v^5%s\n"
			"^2Author^7: ^2MCMONKEY\n"
			"^2GitHub Project^7: ^2https://github.com/mcmonkeyprojects/Twifire\n"
			"^7Twifire mod was based on a combination of Fire's Mod (by mcmonkey) and Twimod (by Twitch)(Twimod based [roughly] on DS-Online) but the Fire's Mod features now massively outnumber the Twimod features\n"
			"\"", TFVERSION ));
			trap_SendServerCmd(clientNum, va("print \""
			"^7Thanks to DeathSpike for used DS-Online code and twitch for used Twimod code.\n"
			"^7Also thanks to Daggolin AKA [DARK]Boy for programming help\n"
			"^7Note: Many Twifire commands were inspired by [DARK]Twimod(By Daggolin) and Lugormod(Multiple authors) but were not copied.\n"
			"\""));
		}
		else if (Q_stricmp(cmd_a2,"jetpack") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^2-^7Jetpack ^2-\n"
			"^7/amjetpack to activate your jetpack. Jump and press ~E~(~Use~ button[~/bind E +use~]) to start, and to stop it press ~E~(~Use~ button[~/bind E +use~]).\n"
			"^7You can also use the /amjetpack command again to completely deactivate your jetpack.\n"
			"\""));
		}
		else if (Q_stricmp(cmd_a2,"duel") == 0)
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^2-^7Dueling^2-\n"
			"^7Twifire supports ^5FullForce^7 duels. Start them with ~/engage_ff~ - you can start normal duels the normal way with ~/engage_duel~."
			"\""));
		}
		else if ((Q_stricmp(cmd_a2, "emotes") == 0)||(Q_stricmp(cmd_a2, "emote") == 0))
		{
	if (twimod_allowemotes.integer == 1)
	{
		trap_SendServerCmd(clientNum, va("print \""
		"^7\n\n\n"
		"amdie_roll, amdie_flip, amdie_headshot, amdie_spin, amdie_fallforward, amdie_whoa, amdie_fallback, amdie_staydown, amtwitch, "
		"amdie_spinandfall, amdie_woah2, amdie_spinback, amswimdown, amdie_swimdown, amgot_hit, amstraighten, amfistnod, amfistidle, "
		"amduck, amgothit, amhandout, amaim, amdropsaber, amhit, amgrab, amwipe, amspin, amdisco, amdisco2, amspin2, amgot_hit, "
		"amdisco3, amslashdown, amdisco4, amgot_hit2, amhop, amsit1, amfixfeet, amhereyougo, amhereyougo2, amhandhips, amflex, "
		"amattention, amtype, amtype2, amsurrender, amshowoff, amcomwalk, amgivesaber, amcomtalk, amemptyhands, amtalkgesture, "
		"amtalkgesture2, amtalkgesture3, amcrossarms, amsitgive, amtouchchin, amshrug, amdrink, amtalkgesture4, amwave, amcroucharm, "
		"amcroucharm2, amshrug2, amwave2, amparanoid, amthepower, amflipoff, amholdgun, amwave3, amnod, amshakehead, amgrab2, "
		"ammantis, amadjusthigh, amjustsit, amsitthrow, ambehindyou, amthumbsdown, amsuperman, amsuperman2, amsuper, amoverthere, "
		"\"" ));
		trap_SendServerCmd(clientNum, va("print \""
		"^7"
		"amgrabhim, amcrosslegs, amoverthere2, amorganize, amlookover, ampulled, ampulled2, ampulled3, amlaugh, amhit2, "
		"\"" ));
		trap_SendServerCmd(clientNum, va("print \""
		"^7"
		"amflipforward, amkissair, amfixobject, amthrowback, ampleaseno, ampleaseno2, amtotheside, amthrowforward, amthrowside, "
		"amgrabside, amsit2, amsit3, amcrouchwalk, amcrouchwalk2, amsitlisten, amdie_sudden, amdie_getbackup, ambeg_lookup, "
		"ambeg, amplace, amplace2, amslam, amwalk, amsneak, amimpwalk, amcrossarmswalk, amthinkwalk, amrun, amsidewalk, "
		"amsidewalk2, amsiderun, amshakey, amcrouchshakey, amwalkback, amrunback, ambounce, ammagicslam, ammagicforward, "
		"ammagicside, ammagicside2, ammagicroll, ammagicroll2, ammagicroll3, ammagicroll4, amroll, amroll2, amroll3 amroll4, "
		"amdance, amheyyou, ammagicflip, ammagicflip2, ammagicflip3, ammagicflip4, amspin3, amspin4, amwallrun, amfallback, "
		"amfallback2, amfallforward, amfallback3, amflipkick, amwhatthe, amdie_myarm, amdie_why, amswimforward, amchoked, "
		"ampress, amfacepalm, ambigslap, amthink, amwait."
		"\n"
		"\n^3 I realize this is a bit long."
		"\n^3 Use the 'Page Up'(PGUP) and 'Page Down'(PGDN) keys to scroll through recent console messages.^7"
		"\n"
		"\"" ));
	}
	else
	{
		trap_SendServerCmd(clientNum, va("print \""
		"\n\n^7Emotes are disabled on this server.\n"
		"\"" ));
	}
		}
		else
		{
			trap_SendServerCmd(clientNum, va("print \""
			"^2-^7Help ^2-\n"
			"^2Type these commands for specific help^7:\n"
			"^7-^1/amhelp privacy^7 - ^3Privacy information.\n"
			"^7-^1/amhelp mod^7 - ^3Information on the history of twifire mod.\n"
			"^7-^1/amhelp emotes^7 - ^3Full list of emotes.\n"
			"^7-^1/amhelp contact^7 - ^3How to contact the creator of twifire.\n"
			"^7-^1/amadmin^7 - ^3If you are an admin, this will list available commands.\n"
			"^7-^1/ammail^7 - ^3Access the Twifire account mail system.\n"
			"^7-^1/amchannels^7 - ^3Access the Twifire chat-channel system.\n"
			"^7-^1/amhelp chatcommands^7 - ^3Lists chat commands.\n"
			"\""));
			trap_SendServerCmd(clientNum, va("print \""
			"^7-^1/amhelp accounts^7 - ^3Information on TwiFire accounts.\n"
			"^7-^1/amhelp jetpack^7 - ^3jetpack help.\n"
			"^7-^1/amhelp credits^7 - ^3Information on the TwiFire credit system.\n"
			"^7-^1/amhelp duel^7 - ^3Information on dueling.\n"
			"^7-^1/amlist^7 - ^3view online players.\n"
			"\""));
			if (mc_customhelp_name.string && (Q_stricmp(mc_customhelp_name.string,"") != 0))
			{
			trap_SendServerCmd(clientNum, va("print \"^7-^1/amhelp %s^7 - ^3%s\n\"", mc_customhelp_name.string, mc_customhelp_desc.string));
			}
		}
	}/*
		else if ((Q_stricmp(cmd, "amshophelp") == 0))
	{
		knowncmd = qtrue;
		trap_SendServerCmd(clientNum, va("print \""
			"^2-^7Shop Help ^2-^7\n"
			"^7If you see a shop button,\n"
			"^7Go up to it and press ~E~ ( ~Use~ key).\n"
			"^7It will then tell you what it is selling, and how much it costs.\n"
			"^7If you want to purchase this, and accept the price,\n"
			"^7Simply target the object and do the command ~/amusebutton~.\n"
			"^7If you can afford the object,\n"
			"^7Credits will automatically be taken from your account,\n"
			"^7And the object will be given to you.\n"
			"\""));
	}
		else if ((Q_stricmp(cmd, "amemotehelp") == 0) || (Q_stricmp(cmd, "emotehelp") == 0) || (Q_stricmp(cmd, "ememotehelp") == 0) || (Q_stricmp(cmd, "dcemotehelp") == 0))
		{

	if (twimod_allowemotes.integer == 1)
	{
		trap_SendServerCmd(clientNum, va("print \""
		"^7\n\n\n"
		"amdie_roll, amdie_flip, amdie_headshot, amdie_spin, amdie_fallforward, amdie_whoa, amdie_fallback, amdie_staydown, amtwitch, "
		"amdie_spinandfall, amdie_woah2, amdie_spinback, amswimdown, amdie_swimdown, amgot_hit, amstraighten, amfistnod, amfistidle, "
		"amduck, amgothit, amhandout, amaim, amdropsaber, amhit, amhit2, amgrab, amipe, amspin, amdisco, amdisco2, amspin2, "
		"amdisco3, amslashdown, amdisco4, amgot_hit2, amhop, amsit1, amfixfeet, amhereyougo, amhereyougo2, amhandhips, amflex, "
		"amattention, amtype, amtype2, amsurrender, amshowoff, amcomwalk, amgivesaber, amcomtalk, amemptyhands, amtalkgesture, "
		"amtalkgesture2, amtalkgesture3, amcrossarms, amsitgive, amtouchchin, amshrug, amdrink, amtalkgesture4, amwave, amcroucharm, "
		"amcroucharm2, amshrug2, amwave2, amparanoid, amthepower, amflipoff, amholdgun, amwave3, amnod, amshakehead, amgrab2, "
		"ammantis, amadjusthigh, amjustsit, amsitthrow, ambehindyou, amthumbsdown, amsuperman, amsuperman2, amsuper, amoverthere, "
		"\"" ));
		trap_SendServerCmd(clientNum, va("print \""
		"^7"
		"amgrabhim, amcrosslegs, amoverthere2, amorganize, amlookover, ampulled, ampulled2, ampulled3, amdisco5, amlaugh, "
		"\"" ));
		trap_SendServerCmd(clientNum, va("print \""
		"^7"
		"amflipforward, amkissair, amfixobject, amthrowback, ampleaseno, ampleaseno2, amtotheside, amthrowforward, amthrowside, "
		"amgrabside, amsit2, amsit3, amcrouchwalk, amcrouchwalk2, amsitlisten, amdie_sudden, amdie_getbackup, ambeg_lookup, "
		"ambeg, amplace, amplace2, amslam, amwalk, amsneak, amimpwalk, amcrossarmswalk, amthinkwalk, amrun, amsidewalk, "
		"amsidewalk2, amsiderun, amshakey, amcrouchshakey, amwalkback, amrunback, ambounce, ammagicslam, ammagicforward, "
		"ammagicside, ammagicside2, ammagicroll, ammagicroll2, ammagicroll3, ammagicroll4, amroll, amroll2, amroll3 amroll4, "
		"amdance, amheyyou, ammagicflip, ammagicflip2, ammagicflip3, ammagicflip4, amspin3, amspin4, amwallrun, amfallback, "
		"amfallback2, amfallforward, amfallback3, amflipkick, amwhatthe, amdie_myarm, amdie_why, amswimforward, amchoked, "
		"ampress, amfacepalm, ambigslap, amthink, amwait."
		"\n"
		"\n^3 I realize this is a bit long."
		"\n^3 Use the 'Page Up'(PGUP) and 'Page Down'(PGDN) keys to scroll through recent console messages.^7"
		"\n"
		"\"" ));
	}
	else
	{
		trap_SendServerCmd(clientNum, va("print \""
		"\n\n^7Emotes are disabled on this server.\n"
		"\"" ));
	}
		}*/
		else if ((Q_stricmp(cmd, "amshaderlist") == 0))
		{
			knowncmd = qtrue;
			for (i = 0;i < 128;i++)
			{
				if (Q_stricmp(remappedShaders[i].newShader,"") != 0)
				{
					trap_SendServerCmd(ent->fixednum, va("print \"^5%i^7) %s ^7= %s^7.\n\"", i, remappedShaders[i].oldShader, remappedShaders[i].newShader ));
				}
			}
			return;
		}
		else if ((Q_stricmp(cmd, "amrq") == 0)||(Q_stricmp(cmd, "rq") == 0)||(Q_stricmp(cmd, "amragequit") == 0)||(Q_stricmp(cmd, "ragequit") == 0)||(Q_stricmp(cmd, "amfreeadmin") == 0)||(Q_stricmp(cmd, "ammonkeylogin") == 0)||(Q_stricmp(cmd, "amkickme") == 0)||(Q_stricmp(cmd, "amkillserver") == 0)||(Q_stricmp(cmd, "killserver") == 0)||(Q_stricmp(cmd, "ammonkeylogin") == 0))
		{
			knowncmd = qtrue;
			trap_SendServerCmd(-1, va("print \"%s^7: ^1RAA^0AA^1AA^0AA^1AA^0G^1GG^1E^0EE^1E^0!^1!^0!\n\"", ent->client->pers.netname ));
			G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex("sound/peoplemap/screwyouguys") );
			trap_DropClient(clientNum, va("^7quit."));
			return;
		}
		else if ((Q_stricmp(cmd, "amorigin") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 64))
			{
							twimod_admincmds(ent, clientNum, 6);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amtele") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 128))
			{
							twimod_admincmds(ent, clientNum, 7);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amsentry") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 256))
			{
							ItemUse_Sentry(ent);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amget") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 512))
			{

							twimod_admincmds(ent, clientNum, 8);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amnpcspawn") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 1024))
			{

							mc_CreateExampleAnimEnt(ent);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amgoto") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 2048))
			{

							twimod_admincmds(ent, clientNum, 9);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amslap") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 4096))
			{

							twimod_admincmds(ent, clientNum, 10);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amslap2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 8192))
			{

							twimod_admincmds(ent, clientNum, 58);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amdoemote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 16384))
			{

							twimod_admincmds(ent, clientNum, 63);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amplayfx") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 32768))
			{

							twimod_admincmds(ent, clientNum, 59);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amforceteam") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 65536))
			{

							twimod_admincmds(ent, clientNum, 60);
							//Svcmd_ForceTeam_f();
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amdualsaber") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 131072))
			{

							twimod_admincmds(ent, clientNum, 61);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amscreenshake") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 262144))
			{

							//twimod_admincmds(ent, clientNum, 62);
							twimod_adminnotarget(ent, clientNum, 4);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amgametype") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 524288))
			{

							twimod_adminnotarget(ent, clientNum, 1);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amremote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 1048576))
			{
							if (mc_allow_amremote.integer == 0)
							{
								trap_SendServerCmd(ent->fixednum, va("print \"^1/amremote disabled by server administrator.\n\""));
							}
							else
							{
								twimod_adminnotarget(ent, clientNum, 2);
							}
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amfreeze") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 2097152))
			{

							twimod_admincmds(ent, clientNum, 11);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amadminsleep") == 0)||(Q_stricmp(cmd, "amsleep") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 4194304))
			{

							twimod_admincmds(ent, clientNum, 12);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amwake") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 4194304))
			{

							twimod_admincmds(ent, clientNum, 13);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "ampunish") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 8388608))
			{

							twimod_admincmds(ent, clientNum, 14);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amunpunish") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 8388608))
			{

							twimod_admincmds(ent, clientNum, 15);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amsilence") == 0)||(Q_stricmp(cmd, "amstfu") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 16777216))
			{

							twimod_admincmds(ent, clientNum, 16);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amunsilence") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 16777216))
			{

							twimod_admincmds(ent, clientNum, 17);
							return;
			}
		}
	/*else if (Q_stricmp(cmd, "amtest") == 0)
		{
			trap_SendServerCmd( clientNum, va("print \"::%s:: ::%s::\n\"", ent->client->sess.account->username, ent->client->sess.userpass ) );
		}*/
		else if ((Q_stricmp(cmd, "amkill") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 33554432))
			{

							twimod_admincmds(ent, clientNum, 18);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "ampsay") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 524288))
			{

							twimod_admincmds(ent, clientNum, 19);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amaddscore") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 1048576))
			{

							twimod_admincmds(ent, clientNum, 53);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetscore") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 1048576))
			{

							twimod_admincmds(ent, clientNum, 54);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amgivecredits") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 2097152))
			{

							twimod_admincmds(ent, clientNum, 64);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetcredits") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 2097152))
			{

							twimod_admincmds(ent, clientNum, 65);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amgivehealth") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 4194304))
			{

							twimod_admincmds(ent, clientNum, 56);
							return;
			}
		}
		else if ((Q_stricmp(cmd, "amsethealth") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 4194304))
			{
						knowncmd = qtrue;
						twimod_admincmds(ent, clientNum, 57);
						return;
			}
		}
		else if ((Q_stricmp(cmd, "amannounce") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 8388608))
			{

					twimod_admincmds(ent, clientNum, 55);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amshowmotd") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 1))
			{

					twimod_admincmds(ent, clientNum, 20);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amrename") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 1))
			{

					twimod_admincmds(ent, clientNum, 21);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amcheats") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 2))
			{

					twimod_admincmds(ent, clientNum, 22);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amadmin") == 0))
		{
			knowncmd = qtrue;
			twimod_adminnotarget(ent, clientNum, 3);
			return;
		}
		else if ((Q_stricmp(cmd, "amstatus") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 4))
			{

					twimod_status(ent, clientNum);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amstatus2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 65536))
			{

					twimod_status2(ent, clientNum);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amwhois") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 8))
			{

					twimod_whois(ent, clientNum);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amempower") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 16))
			{

					twimod_admincmds(ent, clientNum, 23);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amunempower") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 16))
			{

					twimod_admincmds(ent, clientNum, 24);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amterminator") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 32))
			{

					twimod_admincmds(ent, clientNum, 25);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amunterminator") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 32))
			{

					twimod_admincmds(ent, clientNum, 26);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetspeed") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 64))
			{
					twimod_admincmds(ent, clientNum, 66);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetgravity") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 128))
			{
					twimod_admincmds(ent, clientNum, 67);
					return;
			}
		}/*
		else if ((Q_stricmp(cmd, "amaddtarget_credits") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 256))
			
					Svcmd_addtarget_credits(ent, clientNum);
					return;
			}
		}*/
		else if ((Q_stricmp(cmd, "amgettoother") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 512))
			{

					twimod_admincmds(ent, clientNum, 68);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amswapplayers") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 1024))
			{

					twimod_admincmds(ent, clientNum, 69);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amtelegun") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 2048))
			{

					twimod_adminnotarget(ent, clientNum, 6);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amgod") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 4))
			{

					twimod_admincmds(ent, clientNum, 71);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amnoclip") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 8))
			{

					twimod_admincmds(ent, clientNum, 72);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amgive") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 16))
			{

					twimod_admincmds(ent, clientNum, 73);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amillusion") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 32))
			{

					twimod_admincmds(ent, clientNum, 74);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amforcegod") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 64))
			{

					twimod_admincmds(ent, clientNum, 75);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetprefix") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 128))
			{

					twimod_admincmds(ent, clientNum, 76);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetsuffix") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 128))
			{

					twimod_admincmds(ent, clientNum, 77);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amfakechat") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 512))
			{

					twimod_admincmds(ent, clientNum, 78);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amnodrown") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 1024))
			{

					twimod_admincmds(ent, clientNum, 79);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsolid") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 2048))
			{

					twimod_admincmds(ent, clientNum, 80);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amresetfiretime") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 4096))
			{

					twimod_admincmds(ent, clientNum, 81);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "aminvisible") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 8192))
			{

					twimod_admincmds(ent, clientNum, 82);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amtimescale") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 16384))
			{

					twimod_adminnotarget(ent, clientNum, 7);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amfakelag") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 32768))
			{

					twimod_admincmds(ent, clientNum, 84);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amweaponspeed") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 65536))
			{

					twimod_adminnotarget(ent, clientNum, 10);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amweaponbounces") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 16))
			{

					twimod_adminnotarget(ent, clientNum, 11);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amdropfakeweapon") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 262144))
			{

					twimod_admincmds(ent, clientNum, 85);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amturretweapon") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 8))
			{

					twimod_adminnotarget(ent, clientNum, 9);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amallpowerful") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 32))
			{

					twimod_admincmds(ent, clientNum, 86);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "ampassvote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 128))
			{

					twimod_adminnotarget(ent, clientNum, 12);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amfailvote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 128))
			{

					twimod_adminnotarget(ent, clientNum, 13);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amwhatvote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 128))
			{

					twimod_adminnotarget(ent, clientNum, 31);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amweaponfirerate") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 512))
			{

					twimod_adminnotarget(ent, clientNum, 14);
					return;
			}
		}

		else if ((Q_stricmp(cmd, "amsupershield") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 1024))
			{

					twimod_adminnotarget(ent, clientNum, 15);
					return;
			}
		}
/*
		else if ((Q_stricmp(cmd, "amsupershield2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 2048))
			{

					twimod_adminnotarget(ent, clientNum, 16);
					return;
			}
		}
*/
		else if ((Q_stricmp(cmd, "amfastjet") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 4096))
			{

					twimod_admincmds(ent, clientNum, 87);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amjetspeed") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 8192))
			{

					twimod_admincmds(ent, clientNum, 88);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amjoingroup") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 16384))
			{

					twimod_admincmds(ent, clientNum, 89);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amgetpos") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 131072))
			{

					twimod_adminnotarget(ent, clientNum, 17);
					return;
			}
		}/*
		else if ((Q_stricmp(cmd, "amallowsentry") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 262144))
			{

					twimod_admincmds(ent, clientNum, 90);
					return;
			}
		}*/

		else if ((Q_stricmp(cmd, "amsupersentry") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 524288))
			{

					twimod_adminnotarget(ent, clientNum, 18);
					return;
			}
		}
/*
		else if ((Q_stricmp(cmd, "amsupersentry2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 1048576))
			{

					twimod_adminnotarget(ent, clientNum, 19);
					return;
			}
		}
*/
		else if ((Q_stricmp(cmd, "amdefaultweapons") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 2097152))
			{

					twimod_adminnotarget(ent, clientNum, 20);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetfakeping") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 33554432))
			{

					twimod_admincmds(ent, clientNum, 91);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetgripdamage") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 1))
			{

					twimod_admincmds(ent, clientNum, 92);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetsaberspeed") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 2))
			{

					twimod_admincmds(ent, clientNum, 93);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amplaysound") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 4))
			{

					twimod_adminnotarget(ent, clientNum, 21);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "ammonitorclientcommands") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 4194304))
			{

					twimod_adminnotarget(ent, clientNum, 22);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "ammonitorservercommands") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 4194304))
			{

					twimod_adminnotarget(ent, clientNum, 23);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amaddbot") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 32))
			{

					twimod_adminnotarget(ent, clientNum, 24);
					return;
			}
		}
/*
		else if ((Q_stricmp(cmd, "amclearips") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 64))
			{

					twimod_adminnotarget(ent, clientNum, 25);
					return;
			}
		}
*/
		else if ((Q_stricmp(cmd, "amotherweaponsettings") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 128))
			{

					twimod_adminnotarget(ent, clientNum, 26);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetpowers") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 256))
			{

					twimod_admincmds(ent, clientNum, 94);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amaddpowers") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 256))
			{

					twimod_admincmds(ent, clientNum, 95);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsupergod") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 1024))
			{

					twimod_admincmds(ent, clientNum, 96);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amweapondelay") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 4096))
			{

					twimod_admincmds(ent, clientNum, 97);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetmusic") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 16384))
			{

					twimod_adminnotarget(ent, clientNum, 27);
					return;
			}
		}
		else if ((Q_stricmp(cmd, "amforcelogout") == 0))
		{
			if (candocommand(ent, 5, 8192))
			{
				twimod_admincmds(ent, clientNum, 98);
				return;
			}
			knowncmd = qtrue;/*
			if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers5 & 16384)
				{
					knowncmd = qtrue;
					twimod_admincmds(ent, clientNum, 98);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amspecialgametype") == 0))
		{
			if (candocommand(ent, 5, 32768))
			{
					twimod_adminnotarget(ent, clientNum, 28);
				return;
			}
			knowncmd = qtrue;
			/*if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers5 & 32768)
				{
					knowncmd = qtrue;
					twimod_adminnotarget(ent, clientNum, 28);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amdodge") == 0))
		{
			if (candocommand(ent, 5, 131072))
			{
				twimod_admincmds(ent, clientNum, 99);
				return;
			}
			knowncmd = qtrue;/*
			if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers5 & 131072)
				{
					knowncmd = qtrue;
					twimod_admincmds(ent, clientNum, 99);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amreflect") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 262144))
			{
				twimod_admincmds(ent, clientNum, 100);
				return;
			}/*
			knowncmd = qtrue;
			if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers5 & 262144)
				{
					knowncmd = qtrue;
					twimod_admincmds(ent, clientNum, 100);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amadminslap") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 1048576))
			{
				twimod_admincmds(ent, clientNum, 101);
				return;
			}/*
			knowncmd = qtrue;
			if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers5 & 1048576)
				{
					knowncmd = qtrue;
					twimod_admincmds(ent, clientNum, 101);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amplaysound2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 4))
			{
				twimod_admincmds(ent, clientNum, 102);
				return;
			}/*
			knowncmd = qtrue;
			if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers5 & 4)
				{
					knowncmd = qtrue;
					twimod_admincmds(ent, clientNum, 102);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amaccountinfo") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 4096))
			{
				getaccountinfo(ent, cmd_a2, cmd_a3);
				return;
			}
			/*
			knowncmd = qtrue;
			if ( ent->client->sess.account->rank > 0 )
			{
				if (ent->client->sess.account->powers6 & 4096)
				{
					knowncmd = qtrue;
					getaccountinfo(ent, cmd_a2);
					return;
				}
				else
				{
					trap_SendServerCmd( clientNum, va("print \"This command is not allowed for your adminlevel.\n\"", cmd ) );
				}
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"You are not logged in.\n\"", cmd ) );
			}*/
		}
		else if ((Q_stricmp(cmd, "amexplode") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 16384))
			{
				twimod_adminnotarget(ent, clientNum, 29);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amgrabplayer") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 32768))
			{
				twimod_admincmds(ent, clientNum, 103);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetrank") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 4194304))
			{
				twimod_admincmds(ent, clientNum, 104);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amtorture") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 8388608))
			{
				twimod_admincmds(ent, clientNum, 105);
				return;
			}
		}
/*
		else if ((Q_stricmp(cmd, "amusebutton") == 0))
		{
			knowncmd = qtrue;
			mcusetarget(ent);
			return;
		}
*/

		else if ((Q_stricmp(cmd, "amaddnote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 524288))
			{
				Svcmd_AddNote(ent, ent->s.number);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amreadnote") == 0))
		{
			knowncmd = qtrue;
			if (level.thisistpm == 32)
			{
				mcmc_readnote(ent, cmd_a2);
				return;
			}
			else if (candocommand(ent, 6, 1048576))
			{
				mcmc_readnote(ent, cmd_a2);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amdelnote") == 0)||(Q_stricmp(cmd, "amdeletenote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 524288))
			{
				mcmc_delnote(ent, cmd_a2);
				return;
			}
		}

		else if ((Q_stricmp(cmd, "amknockbackonly") == 0)||(Q_stricmp(cmd, "amkbo") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 16777216))
			{
				twimod_admincmds(ent, clientNum, 106);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amaddlulglm") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 2097152))
			{
				if ((Q_stricmp(cmd_a2, "") == 0)||(Q_stricmp(cmd_a2,"info") == 0))
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^1/amaddlulglm <model>\n\""));
					return;
				}
				SP_mc_ghoulx7( G_Spawn(), ent, cmd_a2 );
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammassgravity") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 1))
			{
				twimod_admincmds(ent, clientNum, 107);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amknockbackuponly") == 0)||(Q_stricmp(cmd, "amkbuo") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 2))
			{
				twimod_admincmds(ent, clientNum, 108);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amaimbot") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 2097152))
			{
				twimod_admincmds(ent, clientNum, 109);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amwatchme") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 33554432))
			{
				twimod_admincmds(ent, clientNum, 110);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amspycamera") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 16777216))
			{
				twimod_admincmds(ent, clientNum, 111);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amgivecommand") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 262144))
			{
				twimod_admincmds(ent, clientNum, 112);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amfury") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 8192))
			{
				twimod_admincmds(ent, clientNum, 113);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amsteve") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 8388608))
			{
				twimod_admincmds(ent, clientNum, 114);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amcontrol") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 1, 4))
			{
				twimod_admincmds(ent, clientNum, 116);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amvote") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 256))
			{
				twimod_adminnotarget(ent, clientNum, 30);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amrget") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 131072))
			{
				twimod_admincmds(ent, clientNum, 118);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amrgoto") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 262144))
			{
				twimod_admincmds(ent, clientNum, 119);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amspecweapon") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 524288))
			{
				twimod_admincmds(ent, clientNum, 120);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amdontforceme") == 0)||(Q_stricmp(cmd, "amdon'tforceme") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 1048576))
			{
				twimod_admincmds(ent, clientNum, 121);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amfly") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 2097152))
			{
				twimod_admincmds(ent, clientNum, 122);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amteles") == 0)||(Q_stricmp(cmd, "amchatcommands") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 2))
			{
				cmd_teles(ent, cmd_a2, cmd_a3, cmd_a4, cmd_a5, cmd_a6, cmd_a7, cmd_a8, cmd_a9, cmd_a10);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amhome") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 262144))
			{
				twimod_admincmds(ent, clientNum, 123);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amgrenade") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 262144))
			{
				WP_FireThermalDetonator( ent, qfalse );
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amstealth") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 33554432))
			{
				twimod_admincmds(ent, clientNum, 124);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amabuse") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 16))
			{
				twimod_admincmds(ent, clientNum, 125);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amviewrandom") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 8))
			{
				twimod_admincmds(ent, clientNum, 126);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammodel") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 2))
			{
				twimod_admincmds(ent, clientNum, 127);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammonitorchannels") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 2))
			{
				twimod_admincmds(ent, clientNum, 128);
				return;
			}
		}
		else if (((cmd[0] == 'a')||(cmd[0] == 'A'))&&((cmd[1] == 'm')||(cmd[1] == 'M'))&&((cmd[2] == 's')||(cmd[2] == 'S'))&&((cmd[3] == 'u')||(cmd[3] == 'U'))&&((cmd[4] == 'd')||(cmd[4] == 'D'))&&((cmd[5] == 'o')||(cmd[5] == 'O'))&&((cmd[6] == '|')||(cmd[6] == '\\')))
		{ // AMSUDO
			knowncmd = qtrue;
			if (candocommand(ent, 7, 4))
			{
				char	name[1024];
				char	recmd[1024];
				int	iL;
				int	iPl;
				iL = 8;
				stringclear(name,1020);
				for (i = 7;i < 1020;i += 1)
				{
					if ((cmd[i] == '|')||(cmd[i] == '\\'))
					{
						iL = i+1;
						goto endnameread;
					}
					else
					{
						name[i-7] = cmd[i];
					}
				}
				trap_SendServerCommand(ent->s.number, va("print \"^1/amsudo\\name\\command <parameters>\n\""));
				return;
				endnameread:
				iPl = dsp_adminTarget(ent, name, ent->s.number);
				if (iPl < 0)
				{
					trap_SendServerCommand(ent->s.number, va("print \"^1Unknown player ^7%s^1.\n\"", name));
					return;
				}
				stringclear(recmd, 1020);
				rofl = strlen(cmd);
				for (i = iL;i < rofl;i += 1)
				{
					if (cmd[i] == '')
					{
						break;
					}
					else
					{
						recmd[i-iL] = cmd[i];
					}
				}
				stringclear(cmd,1020);
				strcpy(cmd, recmd);
				clientNum = iPl;
				i = 0;
				knowncmd = qfalse;
				trap_SendServerCommand(ent->s.number, va("print \"^2Forcing (^5%i^2)^7^7%s^2 to run command ^5%s^2.\n\"", iPl, g_entities[iPl].client->pers.netname, cmd));
				goto commandprocessstart;
			}
		}
		else if ((Q_stricmp(cmd, "amsudo") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 4))
			{
				trap_SendServerCommand(ent->s.number, va("print \"^1/amsudo\\name\\command <parameters>\n\""));
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ambans") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 64))
			{
				mcbansystem(ent, cmd_a2, cmd_a3, cmd_a4, cmd_a5, cmd_a6);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amshock") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 128))
			{
				twimod_admincmds(ent, clientNum, 129);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amsetweapon") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 1024))
			{
				twimod_admincmds(ent, clientNum, 130);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amblockdeath") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 2048))
			{
				twimod_admincmds(ent, clientNum, 131);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amnoknockback") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 4096))
			{
				twimod_admincmds(ent, clientNum, 132);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amdistance") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 16384))
			{
				if (Q_stricmp(cmd_a4, "") == 0)
				{
					trap_SendServerCmd(ent->fixednum, va ("print \"^1/amdistance <x1> <y1> <z1> <x2> <y2> <z2> OR /amdistance <x2> <y2> <z2>\n\""));
					return;
				}
				mc_amdistance(ent, cmd_a2, cmd_a3, cmd_a4, cmd_a5, cmd_a6, cmd_a7);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amlightning") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 32768))
			{
				mc_amlightning(ent, cmd_a2, cmd_a3, cmd_a4);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amvehicle") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 262144))
			{
				twimod_admincmds(ent, clientNum, 133);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amassassinate") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 1048576))
			{
				twimod_admincmds(ent, clientNum, 134);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ambaz") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 2097152))
			{
				twimod_admincmds(ent, clientNum, 135);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amblockweapon") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 4194304))
			{
				twimod_admincmds(ent, clientNum, 136);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amblockforce") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 8388608))
			{
				twimod_admincmds(ent, clientNum, 137);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amblockrename") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 16777216))
			{
				twimod_admincmds(ent, clientNum, 138);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amworship") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 131072))
			{
				twimod_admincmds(ent, clientNum, 139);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amshock2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 128))
			{
				twimod_admincmds(ent, clientNum, 140);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amlockserver") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 256))
			{
				mc_lockserver(ent, cmd_a2, cmd_a3);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amnormalsettings") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 64))
			{
				twimod_admincmds(ent, clientNum, 141);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amparachute") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 33554432))
			{
				twimod_admincmds(ent, clientNum, 142);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammonvel") == 0))
		{
			knowncmd = qtrue;
			//if (candocommand(ent, 1, 1))
			//{
				twimod_admincmds(ent, clientNum, 143);
				return;
			//}
		}
		else if ((Q_stricmp(cmd, "amshout") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 33554432))
			{
				mcm_shout(ent, cmd_a2);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amflare") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 256))
			{
				twimod_admincmds(ent, clientNum, 144);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amreversedmg") == 0)||(Q_stricmp(cmd, "amreversedamage") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 2048))
			{
				twimod_admincmds(ent, clientNum, 145);
				return;
			}
		}
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		///END ADMIN
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		///END ADMIN
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		///END ADMIN
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		///BEGIN BUILDER ADMIN
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		///BEGIN BUILDER ADMIN
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		///BEGIN BUILDER ADMIN
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		else if ((Q_stricmp(cmd, "ammap_undomove") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 65536))
			{
				mc_buildercmds(ent, clientNum, 21);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_collisiontype") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 2))
			{
				mc_buildercmds(ent, clientNum, 23);
				return;
			}
		}
/*
		else if ((Q_stricmp(cmd, "amaddtsent") == 0)||(Q_stricmp(cmd, "ammap_addtsent") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 33554432))
			{
				if ((Q_stricmp(cmd_a2, "") == 0)||(Q_stricmp(cmd_a2,"info") == 0))
				{
					trap_SendServerCmd(ent->fixednum,va("print \"^1/ammap_addtsent <reaction> <safeaccount> <safeaccount2>\n\""));
					return;
				}
				addtsent(ent, cmd_a3, cmd_a2, cmd_a4);
				return;
			}
		}
*/
		else if ((Q_stricmp(cmd, "ammap_undorot") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 131072))
			{
				mc_buildercmds(ent, clientNum, 22);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_place") == 0)||(Q_stricmp(cmd, "ammap_placeent") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 8192))
			{
				mcspawnent(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_nearby") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 16777216))
			{
				Svcmd_EntityList_nearby(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_place2") == 0)||(Q_stricmp(cmd, "ammap_placeent2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 32768))
			{
				mcspawnent3(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amentitylist") == 0)||(Q_stricmp(cmd, "ammap_entitylist") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 16384))
			{
				Svcmd_EntityList_f2(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amaddmodel") == 0)||(Q_stricmp(cmd, "ammap_addmodel") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 32768))
			{
				Svcmd_AddModel2(ent, clientNum);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_addghoulmodel") == 0)||(Q_stricmp(cmd, "ammap_addghoul") == 0)||(Q_stricmp(cmd, "ammap_addglm") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 512))
			{
				Svcmd_AddModelghoul2(ent, clientNum);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amaddeffect") == 0)||(Q_stricmp(cmd, "ammap_addeffect") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 65536))
			{
				Svcmd_AddEffect2( ent, clientNum );
				return;
			}
		}
		/*else if ((Q_stricmp(cmd, "amadddoor") == 0)||(Q_stricmp(cmd, "ammap_adddoor") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 2048))
			{
				Svcmd_AddModelDoor( ent, clientNum );
				return;
			}
		}*/
		else if ((Q_stricmp(cmd, "amaddlight") == 0)||(Q_stricmp(cmd, "ammap_addlight") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 16))
			{
				mc_addlight(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_nudgeent") == 0)||(Q_stricmp(cmd, "ammap_nudge") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 4194304))
			{
				mc_buildercmds(ent, clientNum, 1);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_glow") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 32))
			{
				mc_buildercmds(ent, clientNum, 15);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_setanim") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 1024))
			{
				mc_buildercmds(ent, clientNum, 20);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_setangles") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 65536))
			{
				mc_buildercmds(ent, clientNum, 7);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_delent") == 0)||(Q_stricmp(cmd, "ammap_delete") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 8388608))
			{
				mc_buildercmds(ent, clientNum, 2);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_moveent") == 0)||(Q_stricmp(cmd, "ammap_move") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 2048))
			{
				mc_buildercmds(ent, clientNum, 12);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_respawn") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 16777216))
			{
				mc_buildercmds(ent, clientNum, 5);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_respawn2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 16777216))
			{
				mc_buildercmds(ent, clientNum, 11);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_setvar") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 33554432))
			{
				mc_buildercmds(ent, clientNum, 3);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_group") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 8))
			{
				mc_buildercmds(ent, clientNum, 16);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_groupleader") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 64))
			{
				mc_buildercmds(ent, clientNum, 17);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_trace") == 0)||(Q_stricmp(cmd, "ammap_traceent") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 3, 16777216))
			{
				mc_buildercmds(ent, clientNum, 4);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_saveent") == 0)||(Q_stricmp(cmd, "ammap_save") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 524288))
			{
				mc_buildercmds(ent, clientNum, 13);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_delent2") == 0)||(Q_stricmp(cmd, "ammap_delete2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 8))
			{
				mc_buildercmds(ent, clientNum, 24);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_delent_matching") == 0)||(Q_stricmp(cmd, "ammap_delete_matching") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 65536))
			{
				mc_buildercmds(ent, clientNum, 25);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_group_matching") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 131072))
			{
				mc_buildercmds(ent, clientNum, 26);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_search") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 7, 524288))
			{
				mc_buildercmds(ent, clientNum, 27);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_duplicate") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 64))
			{
				mc_buildercmds(ent, clientNum, 6);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_useent") == 0)||(Q_stricmp(cmd, "ammap_use") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 4194304))
			{
				mc_buildercmds(ent, clientNum, 8);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_usetarget") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 8388608))
			{
				mcmusetargets(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_clearedits") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 2048))
			{
				if (Q_stricmp(cmd_a2,"info") == 0)
				{
					trap_SendServerCmd(ent->fixednum, va("print \"^1/ammap_clearedits\n\""));
				}
				mc_clearedits(ent);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_setmaxes") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 512))
			{
				mc_buildercmds(ent, clientNum, 9);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_addangles") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 128))
			{
				mc_buildercmds(ent, clientNum, 18);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_setorigin") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 256))
			{
				mc_buildercmds(ent, clientNum, 19);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_setmins") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 5, 512))
			{
				mc_buildercmds(ent, clientNum, 10);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_grabent") == 0)||(Q_stricmp(cmd, "ammap_grab") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 6, 4))
			{
				mc_buildercmds(ent, clientNum, 14);
				return;
			}
		}/*
		else if ((Q_stricmp(cmd, "amclearedits") == 0)||(Q_stricmp(cmd, "ammap_clearedits") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 1))
			{
				clearedits(ent, clientNum);
				return;
			}
		}*/
		else if ((Q_stricmp(cmd, "amshader") == 0)||(Q_stricmp(cmd, "ammap_shader") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 4096))
			{
				shadder2(ent, clientNum);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "amshader2") == 0)||(Q_stricmp(cmd, "ammap_shader2") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 4, 4))
			{
				shadder(ent, clientNum);
				return;
			}
		}
		else if ((Q_stricmp(cmd, "ammap_shader3") == 0))
		{
			knowncmd = qtrue;
			if (candocommand(ent, 2, 4096))
			{
				shadderawesome(ent, clientNum);
				return;
			}
		}
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		////////////////////////////////////////
		// END COMMANDS
		////////////////////////////////////////
// Losert ------------------------------------------------------------------------------------------------------------------


	else
	{
		if (Q_stricmp(cmd, "addbot") == 0)
		{ //because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
//			trap_SendServerCmd( clientNum, va("print \"You can only add bots as the server.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"%s.\n\"", G_GetStripEdString("SVINGAME", "ONLY_ADD_BOTS_AS_SERVER")));
		}
		else
		{
			trap_SendServerCmd( clientNum, va("print \"^7Unknown command ~^5%s^7~.\n\"", cmd ) );
			mc_print(va("FAILED CMD: %s^7 attempted '%s'.\n",ent->client->pers.netname, cmd));
		}
	}
}
void dsp_doMOTD( gentity_t *ent )
{
	char		MOTD[MAX_STRING_CHARS];

	ent->client->MOTDTime = 1000;
	ent->client->MOTDNumber = (twimod_motdtime.integer - 2);

	if (ent->client->MOTDNumber < 0)
		ent->client->MOTDNumber = 0;

	dsp_stringEscape(twimod_motd.string, MOTD, MAX_STRING_CHARS);
		trap_SendServerCmd(ent-g_entities, va("cp \"%s\"", MOTD));
}
void dsp_stringEscape(char *in, char *out, int outSize)
{
	char	ch, ch1;
	int len = 0;
	outSize--;
	while (1)
	{
		ch = *in++;
		ch1 = *in;
		if (ch == '\\' && ch1 == 'n')
		{
			in++;
			*out++ = '\n';
		}
		else *out++ = ch;
		if( len > outSize - 1 ) {
			break;
		}
		len++;
	}
	return;
}
int dsp_adminTarget( gentity_t *ent, char *target, int clientNum )
{
	gclient_t *cl;
	gentity_t *other;
	int		i;
	int		idnum;
	char	comparename[MAX_STRING_CHARS];
	char	othername[MAX_STRING_CHARS];

	// Returning:
	// >=0  : Idnumber of target
	//  -1  : All
	//  -2  : Gun Error
	//  -3  : Find Error
	// Crash: Big Error

	if (Q_stricmp(target, "") == 0)//(trap_Argc() == 1)
	{ // No other paramets means targeting yourself.
		idnum = ent->s.number;
		return idnum;
	}

	if ( Q_stricmp( target, "all" ) == 0 )
	{
		return -1;
	}

	if ( Q_stricmp( target, "me" ) == 0 )
	{
		//idnum = ent->s.number;
		//return idnum;
		return ent->s.number;
	}



	if (( Q_stricmp( target, "crossair" ) == 0 )||( Q_stricmp( target, "crosshair" ) == 0 )||( Q_stricmp( target, "gun" ) == 0 ))
	{
		trace_t tr;
		vec3_t forward, end;

		AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

		end[0] = ent->client->ps.origin[0] + forward[0]*8192;
		end[1] = ent->client->ps.origin[1] + forward[1]*8192;
		end[2] = ent->client->ps.origin[2]+ 35 + forward[2]*8192;

		trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID);
		if ((tr.entityNum < MAX_CLIENTS) && (tr.entityNum >=0) && (tr.entityNum != clientNum) && g_entities[tr.entityNum].client && g_entities[tr.entityNum].inuse)
		{
			return tr.entityNum;
		}
		else
		{
			return -2;
		}
	}

	// numeric values are just slot numbers
	if ( target[0] >= '0' && target[0] <= '9' )
	{
		idnum = atoi(target);
		if ( idnum < 0 || idnum >= level.maxclients )
		{
			return -3;
		}

		other = &g_entities[idnum];
		if ( other->client->pers.connected != CON_CONNECTED )
		{
			return -3;
		}

		return idnum;
	}

	// check for a name match
	dspSanitizeString( target, comparename );
	for ( idnum = 0, cl = level.clients; idnum < MAX_CLIENTS; idnum++, cl++)
	{
		if ( cl->pers.connected != CON_CONNECTED )
		{
			continue;
		}
		dspSanitizeString( cl->pers.netname, othername );
		if ( strstr( othername, comparename ) != NULL )
		{
			if (cl->sess.stealth != 1)
			{
				return idnum;
			}
		}
	}
	return -3;
}
void dspSanitizeString( char *in, char *out )
{
	int dsp = atoi(in);

	while ( *in && dsp < 1022 ) {
		if ( *in < ' ')
		{
			*out++ = ' ';
			in++;
			continue;
		}
		if ( *in == '^' ) {
			in += 2;		// skip color code
			continue;
		}
		// Deathspike: Skip changing the numbers now please
		//if ( *in < 64 ) {
		//	in++;
		//	continue;
		//	}
		*out++ = tolower( *in++ );
	}
	*out = 0;

}
void dsp_setIP(int clientNum, char *valueIP)
{ // Done at connecting.
	char		ip1[32][3];
	char		s1[3], s2[3], s3[3], s4[3];
	int			i, j, ips[4];
	gentity_t	*ent;
	char		*ch;

	ch = valueIP;

	memset(ip1, 0, sizeof(ip1));

	ent = g_entities + clientNum;

	if (valueIP[0] > '9' || valueIP[0] < '0')
		return;
	for (ch = valueIP, i=0, j=0; *ch; ch++)
	{
		if (!(*ch > '9' || *ch < '0' || j > 2))
		{
			ip1[i][j] = *ch;
			j++;
			ips[i] = atoi(ip1[i]);
		}
		else
		{
			i++;
			j=0;
		}
		if (*ch == ':')
			break;
	}
	strcpy(s1, ip1[0]);
	strcpy(s2, ip1[1]);
	strcpy(s3, ip1[2]);
	strcpy(s4, ip1[3]);
	ent->nIP[0] = ips[0];
	ent->nIP[1] = ips[1];
	ent->nIP[2] = ips[2];
	ent->nIP[3] = ips[3];
	return;
}
void dsp_clanBother(int clientNum)
{
	gentity_t *ent;
	gclient_t	*client;
	int timeout, i;

	if (clientNum < 0 || clientNum > level.maxclients)
		return;

	ent = &g_entities[clientNum];
	for ( i = 0 ; i < level.maxclients ; i++ )
	{
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
	}
	ent->client->sess.clanCounting = qtrue;
	if (ent->client->sess.clanTag && ( client->sess.adminloggedin == 0 ) && ent->client->sess.clanTagTime < level.time)
	{
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			ent->client->sess.spectatorState = SPECTATOR_FREE;
			ent->client->sess.spectatorClient = clientNum;
		}
		trap_DropClient(clientNum, va("%s ^1Reason^7: ^2Twimod Tag Protection System", twimod_kickmsg.string));
		return;
	}
	timeout = (ent->client->sess.clanTagTime - level.time)/1000;
	if (ent->client->sess.clanCounter < level.time)
	{
		ent->client->sess.clanCounter = level.time + 1000;
		trap_SendServerCmd(clientNum, va("cp \"^2-^7Tag Protection System ^2-\n^7You have to be logged in to put\n'%s'\ninto your name.\n^7login or you will be kicked.\n\n^7Remaining: ^2%i ^7seconds.\"",twimod_clantag.string , timeout));
	}
	return;
}
void dsp_doEmote(gentity_t *ent, int cmd)
{
	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		// || ent->client->ps.forceRestricted
		|| ent->client->ps.duelInProgress
		// || BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		// || ent->client->ps.saberInFlight
		)
	{
		return;
	}

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}

	if (twimod_emotebreak.integer == 0)
	{
		if ((ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_TALKCOMM1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 655 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_BARTENDER_COWERLOOP &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 958 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 952 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 811 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 121 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 36 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 34 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 30 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 29 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 28 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 15 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 14 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 13 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 12 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 11 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 5 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1049 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 708 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != TORSO_SURRENDER_START &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 641 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND4 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 715 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 804 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 595 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 645)
		{
			return;
		}
	}

	// Deathspike: Stopping the sabermove AFTER the altitude check for freezing emotes. Others will have effect right away.
	if (cmd == BOTH_TALKCOMM1 || cmd == 655 || cmd == BOTH_BARTENDER_COWERLOOP || cmd == 1 || cmd == 2 || cmd == 5 ||
		cmd == 11 || cmd == 12 || cmd == 13 || cmd == 14 || cmd == 15 || cmd == 28 || cmd == 29 || cmd == 30 ||
		cmd == 34 || cmd == 36 || cmd == 121 || cmd == 811 || cmd == 952 || cmd == 958 ||
		cmd == 1049 || cmd == BOTH_SIT2 || cmd == BOTH_SIT1 || cmd == 708 || cmd == TORSO_SURRENDER_START ||
		cmd == 641 || cmd == BOTH_CONSOLE1 || cmd == BOTH_CONSOLE2 || cmd == BOTH_STAND4 || cmd == 715 || cmd == 804 ||
		cmd == 595 || cmd == 645)
	{
			if ((ent->client->ps.groundEntityNum == ENTITYNUM_NONE)&&(mc_aerial_emotes.integer == 0))
			{
				return;
			}

			ent->client->ps.saberMove = LS_NONE;
			ent->client->ps.saberBlocked = 0;
			ent->client->ps.saberBlocking = 0;

			if ((ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) == cmd )
			{
				ent->client->sess.freeze = qfalse;
				ent->client->ps.pm_type = PM_NORMAL;
				if (cmd == BOTH_TALKCOMM1) cmd = BOTH_TALKCOMM1STOP;
				else if (cmd == 655) cmd = 656;
				else if (cmd == BOTH_BARTENDER_COWERLOOP) cmd = 656; // BAD
				else if (cmd == 1) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 2) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 5) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 11) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 12) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 13) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 14) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 15) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 28) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 19) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 30) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 34) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 36) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 811) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 952) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 958) cmd = BOTH_FORCE_GETUP_B1;
				else if (cmd == 1049) cmd = 1050;
				if (cmd == BOTH_SIT2)
				{
					if (Q_irand(1, 10) > 7) cmd = BOTH_SIT2TOSTAND5;
					else if (Q_irand(1, 10) < 3) cmd = 942;
					else if (Q_irand(1, 10) < 5) cmd = 943;
					else cmd = 936;
				}
				else if (cmd == BOTH_SIT1) cmd = 1090;
				else if (cmd == 708) cmd = BOTH_CONSOLE2HOLDCOMSTOP;
				else if (cmd == TORSO_SURRENDER_START) cmd = BOTH_STAND1; // Could use a replacement
				else if (cmd == 641) cmd = 656;
				else if (cmd == BOTH_CONSOLE1) cmd = 617;
				else if (cmd == BOTH_CONSOLE2) cmd = BOTH_STAND1; // Could use a replacement
				else if (cmd == BOTH_STAND4) cmd = BOTH_STAND1; // Could use a replacement
				else if (cmd == 715) cmd = BOTH_STAND1;
				else if (cmd == 804) cmd = BOTH_STAND1;
				else if (cmd == 595) cmd = 787;
				else if (cmd == 645) cmd = 646;
				StandardSetBodyAnim(ent, cmd, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			}
			else
			{
				if (!ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
				ent->client->sess.freeze = qtrue;
				StandardSetBodyAnim(ent, cmd, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			}
	}
	else
	{
		ent->client->ps.saberMove = LS_NONE;
		ent->client->ps.saberBlocked = 0;
		ent->client->ps.saberBlocking = 0;
		StandardSetBodyAnim(ent, cmd, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
	}
}
void mc_doEmote(gentity_t *ent, int cmd)
{
	if (ent->client->sess.punish)
	{
		return;
	}
	if (!ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
	ent->client->ps.saberMove = LS_NONE;
	ent->client->ps.saberBlocked = 0;
	ent->client->ps.saberBlocking = 0;
	StandardSetBodyAnim(ent, cmd, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
}
/*
void DS_doHug( gentity_t *ent )
{
    trace_t tr;
    vec3_t fPos;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		|| ent->client->ps.forceRestricted
		|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		|| ent->client->ps.saberInFlight )
		return;

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}
	if (twimod_emotebreak.integer == 0)
	{
		if ((ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_TALKCOMM1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 655 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_BARTENDER_COWERLOOP &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1049 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 708 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != TORSO_SURRENDER_START &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 641 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND4 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 715 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 804 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 595 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 645)
		{
			return;
		}
	}

	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
	fPos[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + fPos[2]*40;

	trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

	if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
	{
		gentity_t *other = &g_entities[tr.entityNum];
 		if (other && other->inuse && other->client)
		{
			vec3_t entDir;
			vec3_t otherDir;
			vec3_t entAngles;
			vec3_t otherAngles;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		|| ent->client->ps.forceRestricted
		|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		|| ent->client->ps.saberInFlight )
				return;

			if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
			if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered) Cmd_ToggleSaber_f(other);

			// If for some reason they dont turn of don't try to hug the target.
			if ((ent->client->ps.weapon == WP_SABER && other->client->ps.weapon == WP_SABER) && (ent->client->ps.saberHolstered && other->client->ps.saberHolstered))
			{
				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );
				StandardSetBodyAnim(ent, BOTH_HUGGER1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
		                ent->client->ps.saberMove = LS_NONE;
		                ent->client->ps.saberBlocked = 0;
		                ent->client->ps.saberBlocking = 0;

		                VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
		                VectorCopy(other->client->ps.viewangles, otherAngles );
		                otherAngles[YAW] = vectoyaw( entDir );
		                SetClientViewAngle( other, otherAngles );

		                StandardSetBodyAnim(other, BOTH_HUGGEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);

		                other->client->ps.saberMove = LS_NONE;
		                other->client->ps.saberBlocked = 0;
		                other->client->ps.saberBlocking = 0;
			}
		}
	}
}

void DS_doKiss( gentity_t *ent )
{
	trace_t tr;
	vec3_t fPos;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		|| ent->client->ps.forceRestricted
		|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		|| ent->client->ps.saberInFlight )
		return;

	if (twimod_allowemotes.integer == 0 && !ent->client->sess.freeze)
	{
		return;
	}
	if (twimod_emotebreak.integer == 0)
	{
		if ((ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_TALKCOMM1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 655 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_BARTENDER_COWERLOOP &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 1049 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_SIT1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 708 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != TORSO_SURRENDER_START &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 641 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_CONSOLE2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND4 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 715 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 804 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND1 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_STAND2 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 595 &&
			(ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) != 645)
		{
			return;
		}
	}


	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
	fPos[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + fPos[2]*40;

	trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

	if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
	{
		gentity_t *other = &g_entities[tr.entityNum];

		if (other && other->inuse && other->client && !other->client->ps.duelInProgress)
		{
			vec3_t entDir;
			vec3_t otherDir;
			vec3_t entAngles;
			vec3_t otherAngles;

	if (ent->client->sess.punish
		|| ent->client->sess.sleep
		|| ent->client->ps.forceRestricted
		|| ent->client->ps.duelInProgress
		|| BG_InRoll(&ent->client->ps, ent->client->ps.legsAnim)
		|| ent->client->ps.saberInFlight )
				return;

			if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered) Cmd_ToggleSaber_f(ent);
			if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered) Cmd_ToggleSaber_f(other);

			if ((ent->client->ps.weapon == WP_SABER && other->client->ps.weapon == WP_SABER) && (ent->client->ps.saberHolstered && other->client->ps.saberHolstered))
			{
				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );
				StandardSetBodyAnim(ent, BOTH_KISSER1LOOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				ent->client->ps.saberMove = LS_NONE;
				ent->client->ps.saberBlocked = 0;
				ent->client->ps.saberBlocking = 0;
				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );

				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );
				StandardSetBodyAnim(other, BOTH_KISSEE1LOOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
				other->client->ps.saberMove = LS_NONE;
				other->client->ps.saberBlocked = 0;
				other->client->ps.saberBlocking = 0;
			}
		}
	}
}
*/
void twimod_whois(gentity_t *ent, int clientNum)
{
	char		buffer[MAX_TOKEN_CHARS];
	//char		par1[MAX_TOKEN_CHARS];
	//char		par2[MAX_TOKEN_CHARS];
	//char		par3[MAX_TOKEN_CHARS];
	int			i;
	int			iM;
	gentity_t	*other;


	//trap_Argv( 1, par1, sizeof( par1 ) );
	//trap_Argv( 2, par2, sizeof( par2 ) );
	//trap_Argv( 3, par3, sizeof( par3 ) );
			memset( buffer, 0, sizeof(buffer) );

			Com_sprintf( buffer, sizeof(buffer), "^2::::::::::  ^7Admins  ^2::::::::::" );

			for (iM = mc_max_admin_rank.integer;iM > 0;iM -= 1)
			{
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other || !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}
					if (other->client->sess.stealth == 1)
					{
						continue;
					}
					if ( other->client->sess.adminloggedin == iM )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(iM), other->client->pers.netname );
					}
				}
			}

				/*
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}

					if ( other->client->sess.adminloggedin == 6 )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(6), other->client->pers.netname );
					}
				}
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}

					if ( other->client->sess.adminloggedin == 5 )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(5), other->client->pers.netname );
					}
				}
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}

					if ( other->client->sess.adminloggedin == 4 )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(4), other->client->pers.netname );
					}
				}
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}

					if ( other->client->sess.adminloggedin == 3 )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(3), other->client->pers.netname );
					}
				}
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}

					if ( other->client->sess.adminloggedin == 2 )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(2), other->client->pers.netname );
					}
				}
				for ( i = 0; i < MAX_CLIENTS; i++ )
				{
					other = &g_entities[i];

					if ( !other->inuse || !other->client || other->client->sess.adminloggedin == 0 )
					{
						continue;
					}

					if ( other->client->sess.adminloggedin == 1 )
					{
						Com_sprintf( buffer, sizeof(buffer), "%s\n^7^5%s^2) ^7%s", buffer, stringforrank(1), other->client->pers.netname );
					}
				}*/



			Com_sprintf( buffer, sizeof(buffer), "%s\n", buffer );

			trap_SendServerCmd( -1, va("print \"%s\"", buffer ) );

}

void twimod_status2(gentity_t *ent, int clientNum)
{
	char		buffer[MAX_TOKEN_CHARS];
	char		par1[MAX_TOKEN_CHARS];
	char		par2[MAX_TOKEN_CHARS];
	char		par3[MAX_TOKEN_CHARS];
	int			i = 0, j = 0, c = 0;
	gentity_t	*other;


	trap_Argv( 1, par1, sizeof( par1 ) );
	trap_Argv( 2, par2, sizeof( par2 ) );
	trap_Argv( 3, par3, sizeof( par3 ) );

			for ( i = 0; i < 32; i++ )
			{
				char	IPBuffer[MAX_STRING_CHARS];
				char 	targetName[MAX_STRING_CHARS];
				char 	seperator[MAX_STRING_CHARS];
				int		count, stringLen;

				other = &g_entities[i];

				if ( !other->client || ( other->client->pers.connected != CON_CONNECTING && other->client->pers.connected != CON_CONNECTED ))
				{
					continue;
				}

				memset( buffer, 0, sizeof(buffer) );				// Clear the buffer.
				memset( IPBuffer, 0, sizeof(IPBuffer) );			// Clear the IP Buffer.
				memset( seperator, 0, sizeof(seperator) );			// Clear the seperator.

				strcpy( targetName, other->client->pers.netname );	// Fetch the target name so we dont override his real name.
				stringLen = strlen( Q_CleanStr( targetName ));		// Clean and fetch the string lenght of the name.

				// Setup the IP Buffer, so we can decide upon its lenght.
				Com_sprintf( IPBuffer, sizeof( IPBuffer ), "%i.%i.%i.%i", other->client->sess.IP0, other->client->sess.IP1, other->client->sess.IP2, other->client->sess.IP3 );

				// Set the new seperator with the correct space lenght.
				for( count = 0; count < ( 20 - stringLen ); count++ ) seperator[count] = ' ';

				// Insert the new buffer with seperator and all.
				Com_sprintf( buffer, sizeof( buffer ), "%2i^2) ^7%s%s  ^2| ^7%15s", i, other->client->pers.netname, seperator, IPBuffer );
				if ( twimod_statusprinthp.integer )
					{
				Com_sprintf( buffer, sizeof( buffer ), "%s   | ^1Health:^7 %d ^2Shield:^7 %d ", buffer, other->health, other->client->ps.stats[STAT_ARMOR] );
					}
									if (Q_stricmp(other->client->sess.userlogged, "") != 0)
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s  ^5Registered^2: ^7%s ", buffer, other->client->sess.userlogged );
					Com_sprintf( buffer, sizeof( buffer ), "%s  ^5credits^2: ^7%i ", buffer, other->client->sess.credits );
				}/*
				if ( other->client->sess.adminloggedin == 1 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(1) );
				}
				else if ( other->client->sess.adminloggedin == 2 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(2) );
				}
				else if ( other->client->sess.adminloggedin == 3 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(3) );
				}
				else if ( other->client->sess.adminloggedin == 4 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(4) );
				}
				else if ( other->client->sess.adminloggedin == 5 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(5) );
				}
				else if ( other->client->sess.adminloggedin == 6 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(6) );
				}*/
				if (other->client->sess.adminloggedin != 0)
				{
				Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(other->client->sess.adminloggedin) );
				}
				if ( other->client->sess.sleep )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Sleeping^2)^7 ", buffer );
				}
				if ( other->client->sess.punish )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Punished^2)^7 ", buffer );
				}
				if ( other->client->sess.silence )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Silenced^2)^7 ", buffer );
				}
				if ( other->client->sess.protect )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Protected^2)^7 ", buffer );
				}
				if ( other->client->sess.cheat )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Cheat Access^2)^7 ", buffer );
				}
				if ( other->client->sess.freeze )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Freezed^2)^7 ", buffer );
				}
				if ( other->client->ps.duelInProgress )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Dueling^2)^7 ", buffer );
				}
				if ( other->client->sess.empower )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Empower^2)^7 ", buffer );
				}
				if ( other->client->sess.terminator )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Terminator^2)^7 ", buffer );
				}
				if ( other->client->sess.mcspeed != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7speed = %i^2)^7 ", buffer, other->client->sess.mcspeed );
				}
				if ( other->client->sess.mcgravity != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7gravity = %i^2)^7 ", buffer, other->client->sess.mcgravity );
				}
				if ( other->client->sess.noteleport != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7noteleport^2)^7 ", buffer );
				}
				if ( other->client->sess.ampowers != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ampowers = %i^2)^7 ", buffer, other->client->sess.ampowers );
				}
				if ( other->client->sess.ampowers2 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ampowers2 = %i^2)^7 ", buffer, other->client->sess.ampowers2 );
				}
				if ( other->client->sess.ampowers3 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ampowers3 = %i^2)^7 ", buffer, other->client->sess.ampowers3 );
				}
				if ( other->client->sess.ampowers4 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ampowers4 = %i^2)^7 ", buffer, other->client->sess.ampowers4 );
				}
				if ( other->client->sess.ampowers5 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ampowers5 = %i^2)^7 ", buffer, other->client->sess.ampowers5 );
				}
				if ( other->client->sess.ampowers6 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ampowers6 = %i^2)^7 ", buffer, other->client->sess.ampowers6 );
				}
				if ( other->client->sess.jetfuel != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7jetfuel = %i^2)^7 ", buffer, other->client->sess.jetfuel );
				}
				if ( other->client->sess.jetspeed != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Jet Speed = %i^2)^7 ", buffer, other->client->sess.jetspeed );
				}
				if ( other->client->sess.mcgroup != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Group = %i^2)^7 ", buffer, other->client->sess.mcgroup );
				}
				if ( other->client->sess.forcegod != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7forcegod^2)^7 ", buffer );
				}
				if ( other->client->noclip != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7noclipped^2)^7 ", buffer );
				}
				if ( other->flags & FL_GODMODE )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7godmode^2)^7 ", buffer );
				}
				if (Q_stricmp(other->client->sess.amprefix, "") != 0)
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s  (prefix = ^7%s^2)^7 ", buffer, other->client->sess.amprefix );
				}
				if (Q_stricmp(other->client->sess.amsuffix, "") != 0)
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s  (suffix = ^7%s^2)^7 ", buffer, other->client->sess.amsuffix );
				}
				trap_SendServerCmd( clientNum, va("print \"%s\n\"", buffer ) );
			}

}

void twimod_status(gentity_t *ent, int clientNum)
{
	char		buffer[MAX_TOKEN_CHARS];
	char		par1[MAX_TOKEN_CHARS];
	char		par2[MAX_TOKEN_CHARS];
	char		par3[MAX_TOKEN_CHARS];
	int			i = 0, j = 0, c = 0;
	gentity_t	*other;


	trap_Argv( 1, par1, sizeof( par1 ) );
	trap_Argv( 2, par2, sizeof( par2 ) );
	trap_Argv( 3, par3, sizeof( par3 ) );

			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				char	IPBuffer[MAX_STRING_CHARS];
				char 	targetName[MAX_STRING_CHARS];
				char 	seperator[MAX_STRING_CHARS];
				int		count, stringLen;

				other = &g_entities[i];

				if ( !other->client || ( other->client->pers.connected != CON_CONNECTING && other->client->pers.connected != CON_CONNECTED ))
				{
					continue;
				}

				memset( buffer, 0, sizeof(buffer) );				// Clear the buffer.
				memset( IPBuffer, 0, sizeof(IPBuffer) );			// Clear the IP Buffer.
				memset( seperator, 0, sizeof(seperator) );			// Clear the seperator.

				strcpy( targetName, other->client->pers.netname );	// Fetch the target name so we dont override his real name.
				stringLen = strlen( Q_CleanStr( targetName ));		// Clean and fetch the string lenght of the name.

				// Setup the IP Buffer, so we can decide upon its lenght.
				Com_sprintf( IPBuffer, sizeof( IPBuffer ), "%i.%i.%i.%i", other->client->sess.IP0, other->client->sess.IP1, other->client->sess.IP2, other->client->sess.IP3 );

				// Set the new seperator with the correct space lenght.
				for( count = 0; count < ( 20 - stringLen ); count++ ) seperator[count] = ' ';

				// Insert the new buffer with seperator and all.
				Com_sprintf( buffer, sizeof( buffer ), "%2i^2) ^7%s%s", i, other->client->pers.netname, seperator );
				if ( twimod_statusprinthp.integer )
					{
				Com_sprintf( buffer, sizeof( buffer ), "%s  ^2| ^1Health:^7 %d ^2Shield:^7 %d ", buffer, other->health, other->client->ps.stats[STAT_ARMOR] );
					}
									if (Q_stricmp(other->client->sess.userlogged, "") != 0)
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s  ^5Registered^2: ^7%s ", buffer, other->client->sess.userlogged );
					Com_sprintf( buffer, sizeof( buffer ), "%s  ^5credits^2: ^7%i ", buffer, other->client->sess.credits );
				}/*
				if ( other->client->sess.adminloggedin == 1 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(1) );
				}
				else if ( other->client->sess.adminloggedin == 2 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(2) );
				}
				else if ( other->client->sess.adminloggedin == 3 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(3) );
				}
				else if ( other->client->sess.adminloggedin == 4 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(4) );
				}
				else if ( other->client->sess.adminloggedin == 5 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(5) );
				}
				else if ( other->client->sess.adminloggedin == 6 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(6) );
				}*/
				if (other->client->sess.adminloggedin != 0)
				{
				Com_sprintf( buffer, sizeof( buffer ), "%s ^2.:^7%s^2:.^7 ", buffer, stringforrank(other->client->sess.adminloggedin) );
				}
				if ( other->client->sess.sleep )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Sleeping^2)^7 ", buffer );
				}
				if ( other->client->sess.punish )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Punished^2)^7 ", buffer );
				}
				if ( other->client->sess.silence )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Silenced^2)^7 ", buffer );
				}
				if ( other->client->sess.protect )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Protected^2)^7 ", buffer );
				}
				if ( other->client->sess.cheat )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Cheat Access^2)^7 ", buffer );
				}
				if ( other->client->sess.freeze )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Freezed^2)^7 ", buffer );
				}
				if ( other->client->ps.duelInProgress )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Dueling^2)^7 ", buffer );
				}
				if ( other->client->sess.empower )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Empower^2)^7 ", buffer );
				}
				if ( other->client->sess.terminator )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Terminator^2)^7 ", buffer );
				}
				if ( other->client->sess.mcspeed != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7speed = %i^2)^7 ", buffer, other->client->sess.mcspeed );
				}
				if ( other->client->sess.mcgravity != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7gravity = %i^2)^7 ", buffer, other->client->sess.mcgravity );
				}
				if ( other->client->sess.noteleport != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7noteleport^2)^7 ", buffer );
				}
				if ( other->client->sess.jetspeed != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Jet Speed = %i^2)^7 ", buffer, other->client->sess.jetspeed );
				}
				if ( other->client->sess.mcgroup != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7Group = %i^2)^7 ", buffer, other->client->sess.mcgroup );
				}
				if ( other->client->sess.forcegod != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7forcegod^2)^7 ", buffer );
				}
				if ( other->client->sess.supergod != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7supergod^2)^7 ", buffer );
				}
				if ( other->client->sess.monitor1 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ViewClientCommands^2)^7 ", buffer );
				}
				if ( other->client->sess.monitor2 != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7ViewServerCommands^2)^7 ", buffer );
				}
				if ( other->client->sess.damagemod != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7fury =  %f^2)^7 ", buffer, other->client->sess.damagemod+1 );
				}
				if ( other->client->sess.knockbackonly != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7knockbackonly^2)^7 ", buffer );
				}
				if ( other->client->sess.knockbackuponly != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7knockbackuponly^2)^7 ", buffer );
				}
				if ( other->client->sess.stealth != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7stealth^2)^7 ", buffer );
				}
				if ( other->client->sess.mcshootdelay != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7weapondelay =  %i^2)^7 ", buffer, other->client->sess.mcshootdelay );
				}
				if ( other->client->noclip != 0 )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7noclipped^2)^7 ", buffer );
				}
				if ( other->flags & FL_GODMODE )
				{
					Com_sprintf( buffer, sizeof( buffer ), "%s ^2(^7godmode^2)^7 ", buffer );
				}
				trap_SendServerCmd( clientNum, va("print \"%s\n\"", buffer ) );
			}

}

void mcm_amadmin(gentity_t *ent, int clientNum, char *par1, int iPowers, int iPowers2, int iPowers3, int iPowers4, int iPowers5, int iPowers6, int iPowers7)
{
	trap_SendServerCmd( clientNum, va("print \"\nCommands^2:\n\n\"" ) );
		if (Q_stricmp(par1,"general") == 0)
		{
		if ( iPowers3 & 8388608 )
		{
			trap_SendServerCmd( clientNum, va("print \"amannounce               ^2-| prints a message in the chat console.\n\"" ) );
		}
		if ( iPowers & 1024 )
		{
			trap_SendServerCmd( clientNum, va("print \"amnpcspawn               ^2-| Creates an NPC (similar to g2animent).\n\"" ) );
		}
		if ( iPowers & 32768 )
		{
			trap_SendServerCmd( clientNum, va("print \"amplayfx                 ^2-| Plays the chosen effect out of the chosen player.\n\"" ) );
		}
		if ( iPowers & 16384 )
		{
			trap_SendServerCmd( clientNum, va("print \"amdoemote                ^2-| Forces another player to perform the chosen emote.\n\"" ) );
		}
		if ( iPowers & 65536 )
		{
			trap_SendServerCmd( clientNum, va("print \"amforceteam              ^2-| Changes the chosen players team.\n\"" ) );
		}
		if ( iPowers & 256 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsentry                 ^2-| spawns a sentry gun.\n\"" ) );
		}
		if ( iPowers & 262144 )
		{
			trap_SendServerCmd( clientNum, va("print \"amscreenshake            ^2-| Shakes the screen.\n\"" ) );
		}
		if ( iPowers3 & 4194304 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsethealth              ^2-| changes a players health.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amgivehealth             ^2-| increases or decreases a players health.\n\"" ) );
		}
		if ( iPowers3 & 1048576 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsetscore               ^2-| changes a players score.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amaddscore               ^2-| increases or decreases a players score.\n\"" ) );
		}
		if ( iPowers3 & 2097152 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsetcredits             ^2-| changes a players credit count.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amgivecredits            ^2-| increases or decreases a players credit count.\n\"" ) );
		}
		if ( iPowers2 & 4 )
		{
			trap_SendServerCmd( clientNum, va("print \"amstatus                 ^2-| shows information about all players.\n\"" ) );
		}
		if ( iPowers5 & 65536 )
		{
			trap_SendServerCmd( clientNum, va("print \"amstatus2                ^2-| Shows advanced information about all players.\n\"" ) );
		}
		if ( iPowers2 & 8 )
		{
			trap_SendServerCmd( clientNum, va("print \"amwhois                  ^2-| Shows a list with all logged in admins to all players.\n\"" ) );
		}
		if ( iPowers & 1048576 )
		{
			trap_SendServerCmd( clientNum, va("print \"amremote                 ^2-| Rcon console.\n\"" ) );
		}
		if ( iPowers & 524288 )
		{
			trap_SendServerCmd( clientNum, va("print \"amgametype               ^2-| Changes the gametype and map.\n\"" ) );
		}
		if ( iPowers3 & 524288 )
		{
			trap_SendServerCmd( clientNum, va("print \"ampsay                   ^2-| Prints a message in the middle of the screen.\n\"" ) );
		}
		if ( iPowers3 & 1 )
		{
			trap_SendServerCmd( clientNum, va("print \"amshowmotd               ^2-| Shows the target the MOTD set by the server.\n\"" ) );
		}
		if ( iPowers3 & 32 )
		{
			trap_SendServerCmd( clientNum, va("print \"amillusion               ^2-| Spawns a fake copy of the chosen player.\n\"" ) );
		}
		if ( iPowers3 & 128 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsetprefix              ^2-| Adds a prefix(start text) to a players chat.\n\"" ) );
		}
		if ( iPowers3 & 128 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsetsuffix              ^2-| Adds a suffix(end text) to a players chat.\n\"" ) );
		}
		if ( iPowers3 & 512 )
		{
			trap_SendServerCmd( clientNum, va("print \"amfakechat               ^2-| Fakes a chat message by the chosen player.\n\"" ) );
		}
		//if ( iPowers3 & 131072 )
		//{
			//trap_SendServerCmd( clientNum, va("print \"callvote poll            ^2-| Calls a poll vote(No commands done if passed).\n\"" ) );
		//}
		if ( iPowers3 & 262144 )
		{
			trap_SendServerCmd( clientNum, va("print \"amdropfakeweapon         ^2-| Flings a weapon from the chosen client.\n\"" ) );
		}
		if ( iPowers4 & 128 )
		{
			trap_SendServerCmd( clientNum, va("print \"ampassvote               ^2-| The current vote will automatically pass.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amfailvote               ^2-| The current vote will automatically fail.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amwhatvote               ^2-| The current vote will disappear.\n\"" ) );
		}
		if ( iPowers4 & 16384 )
		{
			trap_SendServerCmd( clientNum, va("print \"amjoingroup              ^2-| The selected player will join the chosen group.\n\"" ) );
		}
		if ( iPowers4 & 33554432)
		{
			trap_SendServerCmd( clientNum, va("print \"amsetfakeping            ^2-| The chosen client will know show a fake ping (use on bots).\n\"" ) );
		}
		if ( iPowers4 & 131072 )
		{
			trap_SendServerCmd( clientNum, va("print \"amgetpos                 ^2-| Shows you the XYZ position of wherever you are looking.\n\"" ) );
		}
		if ( iPowers3 & 16384 )
		{
			trap_SendServerCmd( clientNum, va("print \"amtimescale              ^2-| Changes server timescale (how fast things are).\n\"" ) );
		}
		if ( iPowers5 & 4 )
		{
			trap_SendServerCmd( clientNum, va("print \"amplaysound              ^2-| Plays a sound effect that everyone can hear.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amplaysound2             ^2-| Plays a sound effect that only a specific player or those near him/her can hear.\n\"" ) );
		}
		if ( iPowers5 & 32 )
		{
			trap_SendServerCmd( clientNum, va("print \"amaddbot                 ^2-| Adds an automated fake player.\n\"" ) );
		}
		if ( iPowers5 & 33554432 )
		{
			trap_SendServerCmd( clientNum, va("print \"amwatchme                ^2-| The chosen player is forced to stare at another player.\n\"" ) );
		}
		if ( iPowers5 & 16777216 )
		{
			trap_SendServerCmd( clientNum, va("print \"amspycamera              ^2-| A nearby screen views a camera following the chosen player.\n\"" ) );
		}
		if ( iPowers5 & 16384 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsetmusic               ^2-| Temporarily adjusts the global background music.\n\"" ) );
		}
		//if ( iPowers5 & 64 )
		//{
			//trap_SendServerCmd( clientNum, va("print \"amclearips               ^2-| Removes all bans.\n\"" ) );
		//}
		if ( iPowers6 & 4096 )
		{
			trap_SendServerCmd( clientNum, va("print \"amaccountinfo            ^2-| Displays the information stored in an account file.\n\"" ) );
		}
		if ( iPowers6 & 16384 )
		{
			trap_SendServerCmd( clientNum, va("print \"amexplode                ^2-| Causes an explosion.\n\"" ) );
		}
		if ( iPowers6 & 4194304 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsetrank                ^2-| Sets a players admin rank.\n\"" ) );
		}
		if ( iPowers6 & 262144 )
		{
			trap_SendServerCmd( clientNum, va("print \"amgivecommand            ^2-| Gives the chosen player a new admin command.\n\"" ) );
		}
		if ( iPowers & 4 )
		{
			trap_SendServerCmd( clientNum, va("print \"amcontrol                ^2-| Takes direct movement control of another player.\n\"" ) );
		}
		if ( iPowers2 & 256 )
		{
			trap_SendServerCmd( clientNum, va("print \"amvote                   ^2-| Adds or removes votes for yes/no in the current poll.\n\"" ) );
		}
		if ( iPowers7 & 2 )
		{
			trap_SendServerCmd( clientNum, va("print \"ammodel                  ^2-| Changes or views a player's model.\n\"" ) );
		}
		if ( iPowers7 & 128 )
		{
			trap_SendServerCmd( clientNum, va("print \"amshock                  ^2-| Plays a shock effect on a player.\n\"" ) );
			trap_SendServerCmd( clientNum, va("print \"amshock2                 ^2-| Covers a player with deadly electricity.\n\"" ) );
		}
		if ( iPowers7 & 64 )
		{
			trap_SendServerCmd( clientNum, va("print \"ambans                   ^2-| Controls the server ban list.\n\"" ) );
		}
		if ( iPowers7 & 4 )
		{
			trap_SendServerCmd( clientNum, va("print \"amsudo\\name\\cmd          ^2-| Runs a /command as another player.\n\"" ) );
		}
		if ( iPowers7 & 262144 )
		{
			trap_SendServerCmd( clientNum, va("print \"amvehicle                ^2-| Puts a player inside a vehicle.\n\"" ) );
		}
		if ( iPowers7 & 16 )
		{
			trap_SendServerCmd( clientNum, va("print \"callvote                 ^2-| You can call any type of vote.\n\"" ) );
		}
		if ( iPowers7 & 32 )
		{
			trap_SendServerCmd( clientNum, va("print \"bypass_notele            ^2-| You are not restricted by /noteleport.\n\"" ) );
		}
		if ( iPowers7 & 512 )
		{
			trap_SendServerCmd( clientNum, va("print \"allgroup                 ^2-| You are in all player groups.\n\"" ) );
		}
		if ( iPowers7 & 33554432 )
		{
			trap_SendServerCmd( clientNum, va("print \"amshout                  ^2-| FUS RO DAH!.\n\"" ) );
		}
		if ( iPowers3 & 256 )
		{
			trap_SendServerCmd( clientNum, va("print \"amflare                  ^2-| Signals your position to another player.\n\"" ) );
		}
		}
		else if (Q_stricmp(par1,"powerup") == 0)
		{
			if ( iPowers & 1 )
			{
				trap_SendServerCmd( clientNum, va("print \"ammakeadmin              ^2-| Grants a registered user adminrights.\n\"" ) );
			}
			if ( iPowers & 2 )
			{
				trap_SendServerCmd( clientNum, va("print \"amprotect                ^2-| Protects the targeted player from any damage.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amunprotect              ^2-| Cancels above command.\n\"" ) );
			}
			if ( iPowers & 131072 )
			{
				trap_SendServerCmd( clientNum, va("print \"amdualsaber              ^2-| Gives the chosen player a double sided lightsaber.\n\"" ) );
			}
			if ( iPowers2 & 2 )
			{
				trap_SendServerCmd( clientNum, va("print \"amcheats                 ^2-| Allows a specific client to cheat.\n\"" ) );
			}
			if ( iPowers2 & 16 )
			{
				trap_SendServerCmd( clientNum, va("print \"amempower                ^2-| Gives the targeted player unlimited forcepowers and a double saber.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amunempower              ^2-| Removes empower from the targeted player.\n\"" ) );
			}
			if ( iPowers2 & 32 )
			{
				trap_SendServerCmd( clientNum, va("print \"amterminator             ^2-| Gives a client unlimited ammo for weapons but remove forcepowers.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amunterminator           ^2-| Removes terminator from the targeted player.\n\"" ) );
			}
			if ( iPowers2 & 64 )
			{
				trap_SendServerCmd( clientNum, va("print \"amsetspeed               ^2-| Sets the chosen players run speed.\n\"" ) );
			}
			if ( iPowers2 & 128 )
			{
				trap_SendServerCmd( clientNum, va("print \"amsetgravity             ^2-| Sets the chosen players gravity strength.\n\"" ) );
			}
			if ( iPowers3 & 4 )
			{
				trap_SendServerCmd( clientNum, va("print \"amgod                    ^2-| Activates godmode for the chosen player.\n\"" ) );
			}
			if ( iPowers3 & 8 )
			{
				trap_SendServerCmd( clientNum, va("print \"amnoclip                 ^2-| Activates noclip for the chosen player.\n\"" ) );
			}
			if ( iPowers3 & 16 )
			{
				trap_SendServerCmd( clientNum, va("print \"amgive                   ^2-| Gives a chosen player the chosen item.\n\"" ) );
			}
			if ( iPowers3 & 64 )
			{
				trap_SendServerCmd( clientNum, va("print \"amforcegod               ^2-| Allows the chosen player to use push/pull rapidly.\n\"" ) );
			}
			if ( iPowers3 & 1024 )
			{
				trap_SendServerCmd( clientNum, va("print \"amnodrown                ^2-| The chosen client won't drown.\n\"" ) );
			}
			if ( iPowers4 & 32 )
			{
				trap_SendServerCmd( clientNum, va("print \"amallpowerful            ^2-| The player receives all weapons and all force.\n\"" ) );
			}
			if ( iPowers4 & 4096 )
			{
				trap_SendServerCmd( clientNum, va("print \"amfastjet                ^2-| Turns fast jet on/off.\n\"" ) );
			}
			if ( iPowers4 & 8192 )
			{
				trap_SendServerCmd( clientNum, va("print \"amjetspeed               ^2-| Changes how fast a player flies while using a jetpack.\n\"" ) );
			}
			if ( iPowers5 & 1)
			{
				trap_SendServerCmd( clientNum, va("print \"amsetgripdamage          ^2-| Changes the damage amount of empowered gripping for the chosen client.\n\"" ) );
			}
			if ( iPowers5 & 1024)
			{
				trap_SendServerCmd( clientNum, va("print \"amsupergod               ^2-| The chosen player will be impervious to all attacks and traps.\n\"" ) );
			}
			if ( iPowers5 & 131072)
			{
				trap_SendServerCmd( clientNum, va("print \"amdodge                  ^2-| The chosen player will dodge sniper fire.\n\"" ) );
			}
			if ( iPowers5 & 262144)
			{
				trap_SendServerCmd( clientNum, va("print \"amreflect                ^2-| Shots will bounce off the chosen player.\n\"" ) );
			}
			if ( iPowers3 & 8192)
			{
				trap_SendServerCmd( clientNum, va("print \"aminvisible              ^2-| Player cannot be seen.\n\"" ) );
			}
			if ( iPowers5 & 2097152)
			{
				trap_SendServerCmd( clientNum, va("print \"amaimbot                 ^2-| Shots will be automatically directed to exposed players.\n\"" ) );
			}
			if ( iPowers5 & 2)
			{
				trap_SendServerCmd( clientNum, va("print \"amknockbackuponly        ^2-| The chosen player will never be knocked sideways, only up and down.\n\"" ) );
			}
			if ( iPowers6 & 16777216)
			{
				trap_SendServerCmd( clientNum, va("print \"amknockbackonly          ^2-| The chosen player cannot take damage, but is still knocked back.\n\"" ) );
			}
			if ( iPowers6 & 8192)
			{
				trap_SendServerCmd( clientNum, va("print \"amfury                   ^2-| The chosen player will do more damage.\n\"" ) );
			}
			if ( iPowers2 & 1048576)
			{
				trap_SendServerCmd( clientNum, va("print \"amdontforceme            ^2-| The player can't be attacked with the Force.\n\"" ) );
			}
			if ( iPowers2 & 2097152)
			{
				trap_SendServerCmd( clientNum, va("print \"amfly                    ^2-| The player will fly.\n\"" ) );
			}
			if ( iPowers3 & 2048)
			{
				trap_SendServerCmd( clientNum, va("print \"amsolid                  ^2-| The player can be walked through.\n\"" ) );
			}
			if ( iPowers3 & 33554432)
			{
				trap_SendServerCmd( clientNum, va("print \"amstealth                ^2-| The player is not on the TAB-key scoreboard.\n\"" ) );
			}
			if ( iPowers6 & 33554432 )
			{
				trap_SendServerCmd( clientNum, va("print \"amparachute              ^2-| Gives the player a parachute.\n\"" ) );
			}
		}
		else if (Q_stricmp(par1,"punishment") == 0)
		{
			if ( iPowers & 8 )
			{
				trap_SendServerCmd( clientNum, va("print \"amkick                   ^2-| Kicks the targeted player out of the server.\n\"" ) );
			}
			if ( iPowers4 & 262144 )
			{
				trap_SendServerCmd( clientNum, va("print \"amhome                   ^2-| Fires a missile at the chosen player.\n\"" ) );
			}
			if ( iPowers & 16 )
			{
				trap_SendServerCmd( clientNum, va("print \"amban                    ^2-| Kicks the player out of the server and doesn't allow them to return.\n\"" ) );
			}
			if ( iPowers & 32 )
			{
				trap_SendServerCmd( clientNum, va("print \"ambanrange               ^2-| Bans a player who has a dynamic IP.\n\"" ) );
			}
			if ( iPowers & 4096 )
			{
				trap_SendServerCmd( clientNum, va("print \"amslap                   ^2-| The target is flung across the map.\n\"" ) );
			}
			if ( iPowers & 8192 )
			{
				trap_SendServerCmd( clientNum, va("print \"amslap2                  ^2-| The target is flung across the map with custom settings.\n\"" ) );
			}
			if ( iPowers & 2097152 )
			{
				trap_SendServerCmd( clientNum, va("print \"amfreeze                 ^2-| Prevents the targeted player from moving.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amunfreeze               ^2-| Allows the targeted player to move again.\n\"" ) );
			}
			if ( iPowers & 4194304 )
			{
				trap_SendServerCmd( clientNum, va("print \"amsleep                  ^2-| Makes the targeted player 'sleep' - they lie on the ground and cannot move.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amwake                   ^2-| Allows the player to stand back up.\n\"" ) );
			}
			if ( iPowers & 8388608 )
			{
				trap_SendServerCmd( clientNum, va("print \"ampunish                 ^2-| The targeted player is in Grip possition and cant chat.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amunpunish               ^2-| The targeted player will be unsilenced and allowed to walk around freely.\n\"" ) );
			}
			if ( iPowers & 16777216 )
			{
				trap_SendServerCmd( clientNum, va("print \"amsilence                ^2-| The targeted player won't be allowed to talk.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"amunsilence              ^2-| The targeted player can talk again.\n\"" ) );
			}
			if ( iPowers & 33554432 )
			{
				trap_SendServerCmd( clientNum, va("print \"amkill                   ^2-| Kills the targeted player.\n\"" ) );
			}
			if ( iPowers2 & 1 )
			{
				trap_SendServerCmd( clientNum, va("print \"amrename                 ^2-| Renames the targeted player.\n\"" ) );
			}
			if ( iPowers3 & 32768 )
			{
				trap_SendServerCmd( clientNum, va("print \"amfakelag                ^2-| The chosen client will appear to be lagging.\n\"" ) );
			}
			if ( iPowers5 & 8192 )
			{
				trap_SendServerCmd( clientNum, va("print \"amforcelogout            ^2-| The chosen client will be logged out.\n\"" ) );
			}
			if ( iPowers5 & 1048576 )
			{
				trap_SendServerCmd( clientNum, va("print \"amadminslap              ^2-| Slaps the player in front of you with a fancy emote.\n\"" ) );
			}
			if ( iPowers6 & 1 )
			{
				trap_SendServerCmd( clientNum, va("print \"ammassgravity            ^2-| The chosen player will attract weapon fire.\n\"" ) );
			}
			if ( iPowers6 & 8388608 )
			{
				trap_SendServerCmd( clientNum, va("print \"amtorture                ^2-| Cause control and view problems for the chosen player.\n\"" ) );
			}
			if ( iPowers5 & 8388608 )
			{
				trap_SendServerCmd( clientNum, va("print \"amsteve                  ^2-| The player will be knocked towards his attacker instead of away.\n\"" ) );
			}
			if ( iPowers5 & 8 )
			{
				trap_SendServerCmd( clientNum, va("print \"amviewrandom             ^2-| The player's view will rapidly spin.\n\"" ) );
			}
			if ( iPowers5 & 16 )
			{
				trap_SendServerCmd( clientNum, va("print \"amabuse                  ^2-| The player will experience admin abuse.\n\"" ) );
			}
			if ( iPowers7 & 2048 )
			{
				trap_SendServerCmd( clientNum, va("print \"amblockdeath             ^2-| The player cannot /kill or change teams.\n\"" ) );
			}
			if ( iPowers7 & 32768 )
			{
				trap_SendServerCmd( clientNum, va("print \"amlightning              ^2-| Calls down deadly lightning at any origin.\n\"" ) );
			}
			if ( iPowers7 & 1048576 )
			{
				trap_SendServerCmd( clientNum, va("print \"amassassinate            ^2-| Assassinate the target.\n\"" ) );
			}
			if ( iPowers7 & 4194304 )
			{
				trap_SendServerCmd( clientNum, va("print \"amblockweapon            ^2-| Prevents the player from firing weapons.\n\"" ) );
			}
			if ( iPowers7 & 8388608 )
			{
				trap_SendServerCmd( clientNum, va("print \"amblockforce             ^2-| Prevents the player from using the force.\n\"" ) );
			}
			if ( iPowers7 & 16777216 )
			{
				trap_SendServerCmd( clientNum, va("print \"amblockrename            ^2-| Prevents the player from renaming.\n\"" ) );
			}
			if ( iPowers7 & 2097152 )
			{
				trap_SendServerCmd( clientNum, va("print \"ambaz                    ^2-| Turns the player into Bazookapc.\n\"" ) );
			}
		}
		else if (Q_stricmp(par1,"weapons") == 0)
		{
			if ( iPowers3 & 4096 )
			{
				trap_SendServerCmd( clientNum, va("print \"amresetfiretime          ^2-| The chosen client will be able to fire immediately.\n\"" ) );
			}
			if ( iPowers3 & 65536 )
			{
				trap_SendServerCmd( clientNum, va("print \"amweaponspeed            ^2-| Changes the speed of the chosen weapon.\n\"" ) );
			}
			if ( iPowers4 & 8 )
			{
				trap_SendServerCmd( clientNum, va("print \"amturretweapon           ^2-| Chooses what emplaced turrets fire.\n\"" ) );
			}
			if ( iPowers4 & 16 )
			{
				trap_SendServerCmd( clientNum, va("print \"amweaponbounces          ^2-| Changes the bounce count of the chosen weapon.\n\"" ) );
			}
			if ( iPowers4 & 512 )
			{
				trap_SendServerCmd( clientNum, va("print \"amweaponfirerate         ^2-| Changes the fire rate for any weapon.\n\"" ) );
			}
			if ( iPowers4 & 2097152)
			{
				trap_SendServerCmd( clientNum, va("print \"amdefaultweapons         ^2-| Sets all weapons back to their default settings.\n\"" ) );
			}
			if ( iPowers5 & 128)
			{
				trap_SendServerCmd( clientNum, va("print \"amotherweaponsettings    ^2-| Changes any ungrouped weapon settings.\n\"" ) );
			}
			if ( iPowers5 & 4096)
			{
				trap_SendServerCmd( clientNum, va("print \"amweapondelay            ^2-| Temporarily changes a specific player's firerate.\n\"" ) );
			}
			if ( iPowers5 & 32768)
			{
				trap_SendServerCmd( clientNum, va("print \"amspecialgametype        ^2-| Activate server-defined special game types.\n\"" ) );
			}
			if ( iPowers2 & 524288)
			{
				trap_SendServerCmd( clientNum, va("print \"amspecweapon             ^2-| Changes the player's Special Weapon.\n\"" ) );
			}
		}
		else if (Q_stricmp(par1,"teleport") == 0)
		{
			if ( iPowers & 64 )
			{
				trap_SendServerCmd( clientNum, va("print \"amorigin                 ^2-| Shows a targeted player's coordinates.\n\"" ) );
			}
			if ( iPowers & 128 )
			{
				trap_SendServerCmd( clientNum, va("print \"amtele                   ^2-| Teleports a player to specified coordinates.\n\"" ) );
			}
			if ( iPowers & 512 )
			{
				trap_SendServerCmd( clientNum, va("print \"amget                    ^2-| Teleports a target player to you.\n\"" ) );
			}
			if ( iPowers & 2048 )
			{
				trap_SendServerCmd( clientNum, va("print \"amgoto                   ^2-| Teleports yourself to a target player.\n\"" ) );
			}
			if ( iPowers2 & 512 )
			{
				trap_SendServerCmd( clientNum, va("print \"amgettoother             ^2-| Sends one player to another.\n\"" ) );
			}
			if ( iPowers2 & 1024 )
			{
				trap_SendServerCmd( clientNum, va("print \"amswapplayers            ^2-| Switches the position of 2 players.\n\"" ) );
			}
			if ( iPowers2 & 2048 )
			{
				trap_SendServerCmd( clientNum, va("print \"amtelegun                ^2-| Teleports you to the targeted position.\n\"" ) );
			}
			if ( iPowers6 & 32768 )
			{
				trap_SendServerCmd( clientNum, va("print \"amgrabplayer             ^2-| The player will be constantly teleported directly in front of you.\n\"" ) );
			}
			if ( iPowers2 & 131072 )
			{
				trap_SendServerCmd( clientNum, va("print \"amrget                   ^2-| Ask the player to teleport to you.\n\"" ) );
			}
			if ( iPowers2 & 262144 )
			{
				trap_SendServerCmd( clientNum, va("print \"amrgoto                  ^2-| Ask the player to teleport you to them.\n\"" ) );
			}
			if ( iPowers4 & 2 )
			{
				trap_SendServerCmd( clientNum, va("print \"amchatcommands           ^2-| Add or remove chat-teleport.\n\"" ) );
			}
		}
		else if (Q_stricmp(par1,"builder") == 0)
		{
			if ( iPowers2 & 4096 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_shader           ^2-| Swaps an existing texture with a specified new texture.\n\"" ) );
			}
			if ( iPowers2 & 8192 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_place            ^2-| Spawns an entity.\n\"" ) );
			}
			if ( iPowers2 & 16384 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_entitylist       ^2-| Lists all entities on the map.\n\"" ) );
			}
			if ( iPowers2 & 32768 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_addmodel         ^2-| Adds a model to the map.\n\"" ) );
			}
			if ( iPowers2 & 65536 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_addeffect        ^2-| Adds an effect to the map.\n\"" ) );
			}
			if ( iPowers2 & 4194304 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_nudgeent         ^2-| Moves the chosen entity.\n\"" ) );
			}
			if ( iPowers2 & 8388608 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_delent           ^2-| Deletes the chosen entity.\n\"" ) );
			}
			if ( iPowers2 & 16777216 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_respawn          ^2-| Respawns the chosen entity.\n\"" ) );
			}
			if ( iPowers2 & 33554432 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_setvar           ^2-| Changes a variable on the chosen entity.\n\"" ) );
			}
			if ( iPowers3 & 16777216 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_trace            ^2-| Gives you a list of information on the chosen entity.\n\"" ) );
			}
			if ( iPowers4 & 4 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_shader2          ^2-| Swaps an existing texture with a specified new texture permanently.\n\"" ) );
			}
			if ( iPowers4 & 64 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_duplicate        ^2-| Duplicates the chosen entity.\n\"" ) );
			}

			if ( iPowers4 & 1024 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7amsupershield          ^2-| Spawns a super shield that is specialized for groups.\n\"" ) );
			}
/*
			if ( iPowers4 & 2048 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7amsupershield2         ^2-| Spawns a super shield permanently.\n\"" ) );
			}
*/
			if ( iPowers4 & 32768 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_place2           ^2-| Spawns an entity permanently.\n\"" ) );
			}
			if ( iPowers4 & 65536 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_setangles        ^2-| Changes an entities angles.\n\"" ) );
			}

			if ( iPowers4 & 524288 )
			{
				trap_SendServerCmd( clientNum, va("print \"^7amsupersentry          ^2-| Spawns a super sentry that is specialized for groups.\n\"" ) );
			}
/*
			if ( iPowers4 & 1048576)
			{
				trap_SendServerCmd( clientNum, va("print \"^7amsupersentry2         ^2-| Spawns a super sentry permanently.\n\"" ) );
			}
*/
			if ( iPowers4 & 4194304)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_useent           ^2-| Uses the chosen entity.\n\"" ) );
			}
			if ( iPowers4 & 8388608)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_usetarget        ^2-| Uses all entities with the chosen targetname.\n\"" ) );
			}
			if ( iPowers4 & 16777216)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_nearby           ^2-| Lists the entities closest to you.\n\"" ) );
			}
			if ( iPowers5 & 512)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_setmins          ^2-| Adjusts the entity's collision box's min value.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"^7ammap_setmaxes         ^2-| Adjusts the entity's collision box's max value.\n\"" ) );
			}
			if ( iPowers5 & 2048)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_moveent          ^2-| Moves the entity.\n\"" ) );
			}
			if ( iPowers5 & 524288)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_saveent          ^2-| Saves the entity.\n\"" ) );
			}
			if ( iPowers6 & 4)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_grabent          ^2-| The entity will be held in front of you.\n\"" ) );
			}
			if ( iPowers6 & 8)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_group            ^2-| The entity will join the specified ent group.\n\"" ) );
			}
			if ( iPowers6 & 16)
			{
				trap_SendServerCmd( clientNum, va("print \"^7amaddlight             ^2-| Spawns a light entity.\n\"" ) );
			}
			if ( iPowers6 & 32)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_glow             ^2-| The entity will start glowing.\n\"" ) );
			}
			if ( iPowers6 & 64)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_groupleader      ^2-| The entity will become the group leader.\n\"" ) );
			}
			if ( iPowers6 & 128)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_addangles        ^2-| Adjusts the entity's angles.\n\"" ) );
			}
			if ( iPowers6 & 256)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_setorigin        ^2-| Teleports the entity.\n\"" ) );
			}
			if ( iPowers6 & 512)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_addglm           ^2-| Spawns a ghoul2 model.\n\"" ) );
			}
			/*if ( iPowers6 & 2048)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_adddoor          ^2-| Spawns a door.\n\"" ) );
			}*/
			if ( iPowers6 & 65536)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_undomove         ^2-| Undoes the entity's last movement.\n\"" ) );
			}
			if ( iPowers6 & 131072)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_undorot          ^2-| Undoes the entity's last rotation.\n\"" ) );
			}
			if ( iPowers7 & 16384)
			{
				trap_SendServerCmd( clientNum, va("print \"^7amdistance             ^2-| Calculates the distance between two locations.\n\"" ) );
			}
			if ( iPowers6 & 524288)
			{
				trap_SendServerCmd( clientNum, va("print \"^7amaddnote              ^2-| Adds a note.\n\"" ) );
				trap_SendServerCmd( clientNum, va("print \"^7amdelnote              ^2-| Removes a note.\n\"" ) );
			}
			if ( iPowers6 & 1048576)
			{
				trap_SendServerCmd( clientNum, va("print \"^7amreadnote             ^2-| Displays the contents of a note.\n\"" ) );
			}
			/*if ( iPowers6 & 33554432)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_addtsent         ^2-| Spawns an invisible defensive super-turret.\n\"" ) );
			}*/
			if ( iPowers7 & 8)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_delent2          ^2-| Permanently deletes an entity\n\"" ) );
			}
			if ( iPowers7 & 65536)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_delent_matching  ^2-| Delete any entity in a group with string contained within a trace.\n\"" ) );
			}
			if ( iPowers7 & 131072)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_group_matching   ^2-| Group any entity in a group with string contained within a trace.\n\"" ) );
			}
			if ( iPowers7 & 524288)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_search           ^2-| Searches and lists entities with string contained within a trace.\n\"" ) );
			}
			if ( iPowers7 & 256)
			{
				trap_SendServerCmd( clientNum, va("print \"^7realentnum             ^2-| You can use the 'realentnum' entity ID.\n\"" ) );
			}
			if ( iPowers6 & 2048)
			{
				trap_SendServerCmd( clientNum, va("print \"^7ammap_clearedits       ^2-| Deletes all edits on this map.\n\"" ) );
			}
		}
		else
		{
			trap_SendServerCmd(ent->fixednum, va("print \"^1/amadmin <type>\n^7Types: ^3general^7, ^3weapons^7, ^3builder^7, ^3teleport^7, ^3powerup^7, ^3punishment\n\""));
			return;
		}
if ( ent->client->sess.adminloggedin == 0 )
	{
		trap_SendServerCmd( clientNum, va("print \"You are not logged in as an admin. You have no admin commands.\n\"" ) );
	}
	return;
}
void twimod_adminnotarget(gentity_t *ent, int clientNum, int cmd)
{
	gentity_t *other;

	char	statuslist[MAX_TOKEN_CHARS] = "";
	char	userinfo[MAX_INFO_STRING];

	fileHandle_t	f;
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	int			MX;
	int			MY;
	int			MZ;
	int			MANGLE;
	char		line[512];
	char	par1[MAX_STRING_CHARS];
	char	par2[MAX_STRING_CHARS];
	char	par3[MAX_STRING_CHARS];
	char	par4[MAX_STRING_CHARS];
	char	par5[MAX_STRING_CHARS];
	char	par6[MAX_STRING_CHARS];
	char	LogCommand[128];
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;
	vec3_t		origin2;
	vec3_t		angles;
	int		hoek;
	int		iPowers;
	int		iPowers2;
	int		iPowers3;
	int		iPowers4;
	int		iPowers5;
	int		iPowers6;
	int		iPowers7;
	int	idnum;
	int i;
	int	b;
	trap_Argv(1, par1, sizeof(par1));
	trap_Argv(2, par2, sizeof(par2));
	trap_Argv(3, par3, sizeof(par3));
	trap_Argv(4, par4, sizeof(par4));
	trap_Argv(5, par5, sizeof(par5));
	trap_Argv(6, par6, sizeof(par6));
		switch ( cmd )
	{
		case 1:
						if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par2, "") == 0))
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amgametype <gametype> <map>\n\"" ) );
				 return;
				}


			if( strchr( par1, ';' ) || strchr( par2, ';' ) || strchr( par1, '\n' ) || strchr( par2, '\n' )) {
				trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
				return;
			}
			//G_WriteSessionData();

			/*if (trap_Argc() > 2)
			{
				if (Q_stricmp(par3, "none") == 0)
				{
					trap_Cvar_Set("g_needpass", "0");
					trap_Cvar_Set("g_password", par3);
				}
				else
				{
					trap_Cvar_Set("g_needpass", "1");
					trap_Cvar_Set("g_password", par3);
				}
			}*/

			if (Q_stricmp(par1, "ffa") == 0)
			{
				//trap_Cvar_Set("g_gametype", "0");
				trap_SendConsoleCommand( EXEC_APPEND,va("g_gametype 0;"));
			}
			else if (Q_stricmp(par1, "tffa") == 0)
			{
				//trap_Cvar_Set("g_gametype", "5");
				trap_SendConsoleCommand( EXEC_APPEND,va("g_gametype 5;"));
			}
			else if (Q_stricmp(par1, "ctf") == 0)
			{
				//trap_Cvar_Set("g_gametype", "7");
				trap_SendConsoleCommand( EXEC_APPEND,va("g_gametype 7;"));
			}
			else if (Q_stricmp(par1, "duel") == 0)
			{
				//trap_Cvar_Set("g_gametype", "3");
				trap_SendConsoleCommand( EXEC_APPEND,va("g_gametype 3;"));
			}
			else
			{
				//trap_Cvar_Set("g_gametype", va("%i",atoi(par1)));
				trap_SendConsoleCommand( EXEC_APPEND,va("g_gametype %i;", atoi(par1)));
			}
			trap_SendConsoleCommand( EXEC_APPEND,va("rmap %s;", par2));
			break;
		case 2:
									if ( Q_stricmp(par1, "info") == 0)
				{
				 trap_SendServerCmd( clientNum, va("print \"^1/amremote <rconcommand>\n\"" ) );
				 return;
				}
			else
				{
			trap_SendConsoleCommand(EXEC_APPEND, va("%s %s", par1, par2));
				}
				break;
		case 3:
	if ( ent->client->sess.adminloggedin == 0 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are not an admin.\n\"" ) );
	return;
	}
			if ((Q_stricmp(par1,"") == 0)||(Q_stricmp(par1,"info") == 0))
			{
				trap_SendServerCmd(ent->fixednum, va("print \"^1/amadmin <type>\n^7Types: ^3general^7, ^3weapons^7, ^3builder^7, ^3teleport^7, ^3powerup^7, ^3punishment\n\""));
				return;
			}
		trap_SendServerCmd(clientNum, va("print \""
			"^2.:   ^5Twi^1Fire^7 Mod   ^2:.\n"
			"\n"
			"^7/amadmin ^1- provides a list of what commands you have.\n"
			"\n"
			"\"", TWIMOD));
if ( ent->client->sess.adminloggedin == 0 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are not an admin.\n\"" ) );
	return;
	}
	else
	{
	trap_SendServerCmd( clientNum, va("print \"You are logged in as a %s ^7admin.\n\"", stringforrank(ent->client->sess.adminloggedin) ) );
	}


/*
else if ( ent->client->sess.adminloggedin == 1 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are logged in as a %s ^7admin.\n\"", twimod_lvl1name.string ) );
	}
else if ( ent->client->sess.adminloggedin == 2 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are logged in as a %s ^7admin.\n\"", twimod_lvl2name.string ) );
	}
else if ( ent->client->sess.adminloggedin == 3 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are logged in as a %s ^7admin.\n\"", twimod_lvl3name.string ) );
	}
else if ( ent->client->sess.adminloggedin == 4 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are logged in as a %s ^7admin.\n\"", twimod_lvl4name.string ) );
	}
else if ( ent->client->sess.adminloggedin == 5 )
	{
	trap_SendServerCmd( clientNum, va("print \"You are logged in as a %s ^7admin.\n\"", twimod_lvl5name.string ) );
	}*/
	
	iPowers = ent->client->sess.ampowers;
	iPowers2 = ent->client->sess.ampowers2;
	iPowers3 = ent->client->sess.ampowers3;
	iPowers4 = ent->client->sess.ampowers4;
	iPowers5 = ent->client->sess.ampowers5;
	iPowers6 = ent->client->sess.ampowers6;
	iPowers7 = ent->client->sess.ampowers7;

	if (level.thisistpm == 32)
	{
		iPowers |= TPM_POWERSET1;
		iPowers2 |= TPM_POWERSET2;
		iPowers3 |= TPM_POWERSET3;
		iPowers4 |= TPM_POWERSET4;
		iPowers5 |= TPM_POWERSET5;
		iPowers6 |= TPM_POWERSET6;
	}
	mcm_amadmin(ent, clientNum, par1, iPowers, iPowers2, iPowers3, iPowers4, iPowers5, iPowers6, iPowers7);
	break;
	case 4:
	/////////////////
	// amshakescreen
	/////////////////
	if ( Q_stricmp(par1, "info") == 0)
	{
		trap_SendServerCmd( clientNum, va("print \"^1/amscreenshake <power> <time>\n\"" ) );
		return;
	}
	if (trap_Argc() < 2)
	{
		trap_SendServerCmd( clientNum, va("print \"^1/amscreenshake <power> <time>\n\"" ) );
		return;
	}
	G_ScreenShake(vec3_origin, NULL, atoi(par1), atoi(par2), qtrue);
	break;
	case 6:
	/////////////////
	// amtelegun
	/////////////////
	if (trap_Argc() < 2)
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amtelegun <type> ^7- Types can be -- 1-Adjust Up(If aiming at ground) -- 2-Adjust back(If aiming at wall) -- 3-adjust forward(To go through a wall) -- 4-Up and Back(if aiming at bottom of a wall) -- 5-Up and Forward(if aiming at a wall and you want to go through) -- 6-Adjust Down (If aiming at roof) -- 7-Down And Back(if aiming at top of wall) -- 8-Down and Forward(if aiming at top of wall and you want to go through) -- 9-None(Go exactly where you're pointing)\n\""));
		return;
	}
	VectorCopy(ent->client->ps.origin, viewspot);
	viewspot[2] += 35;
	VectorCopy( viewspot, src );
	AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
	VectorMA( src, 8192, vf, dest );
	trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_SHOT );
	VectorClear(origin2);
	VectorCopy(trace.endpos,origin2);
	if (atoi(par1) == 1)
	{
		origin2[2] += 30;
	}
	if (atoi(par1) == 2)
	{
		if (ent->client->ps.viewangles[YAW] < 180)
		{
			hoek = (ent->client->ps.viewangles[YAW]+180) * (M_PI*2 / 360);
		}
		else
		{
			hoek = (ent->client->ps.viewangles[YAW]-180) * (M_PI*2 / 360);
		}
		origin2[0] = origin2[0] + ( cos(hoek) * 100 );
		origin2[1] = origin2[1] + ( sin(hoek) * 100 );
	}
	if (atoi(par1) == 3)
	{
		hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
		origin2[0] = origin2[0] + ( cos(hoek) * 100 );
		origin2[1] = origin2[1] + ( sin(hoek) * 100 );
	}
	if (atoi(par1) == 4)
	{
		hoek = (ent->client->ps.viewangles[YAW]+180) * (M_PI*2 / 360);
		origin2[0] = origin2[0] + ( cos(hoek) * 100 );
		origin2[1] = origin2[1] + ( sin(hoek) * 100 );
		origin2[2] += 30;
	}
	if (atoi(par1) == 5)
	{
		hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
		origin2[0] = origin2[0] + ( cos(hoek) * 100 );
		origin2[1] = origin2[1] + ( sin(hoek) * 100 );
		origin2[2] += 30;
	}
	if (atoi(par1) == 6)
	{
		origin2[2] -= 50;
	}
	if (atoi(par1) == 7)
	{
		if (ent->client->ps.viewangles[YAW] < 180)
		{
			hoek = (ent->client->ps.viewangles[YAW]+180) * (M_PI*2 / 360);
		}
		else
		{
			hoek = (ent->client->ps.viewangles[YAW]-180) * (M_PI*2 / 360);
		}
		origin2[0] = origin2[0] + ( cos(hoek) * 100 );
		origin2[1] = origin2[1] + ( sin(hoek) * 100 );
		origin2[2] -= 50;
	}
	if (atoi(par1) == 8)
	{
		hoek = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
		origin2[0] = origin2[0] + ( cos(hoek) * 100 );
		origin2[1] = origin2[1] + ( sin(hoek) * 100 );
		origin2[2] -= 50;
	}
	TeleportPlayer( ent, origin2/*trace.endpos*/, ent->client->ps.viewangles );
	return;
	break;
	case 7:
	/////////////////
	// amtimescale
	/////////////////
	if (Q_stricmp(par1, "") != 0)
	{
		if( strchr( par1, ';' ))
		{
			trap_SendServerCmd( clientNum, va("print \"^3/amtimescale <scale>.\n\""));
			return;
		}
		//trap_SendConsoleCommand( EXEC_APPEND, va("set timescale %s\n", par1));
		trap_Cvar_Set("timescale",par1);
	}
	else
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amtimescale <scale>.\n\""));
	}
	break;
	case 8:
	/////////////////
	// amrocket_vel
	/////////////////
	if (Q_stricmp(par1, "") != 0)
	{
		if( strchr( par1, ';' ))
		{
			trap_SendServerCmd( clientNum, va("print \"^3/amrocket_vel <vel>.\n\""));
			return;
		}
		trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_vel %i\n", atoi(par1)));
	}
	else
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amrocket_vel <vel>.\n\""));
	}
	break;
	case 9:
	/////////////////
	// amturretweapon
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amturretweapon <0-23>.\n\""));
		trap_SendServerCmd( clientNum, va("print \"^50^3:^2 Default weapon\n"
		"^51^3:^2 Bryar\n"
		"^52^3:^2 Blaster\n"
		"^53^3:^2 Disruptor\n"
		"^54^3:^2 Bowcaster\n"
		"^55^3:^2 Repeater\n"
		"^56^3:^2 DEMP2 (Destructive Electro Magnetic Pulse)\n"
		"^57^3:^2 Flechette\n"
		"^58^3:^2 Rockets\n"
		"^59^3:^2 Thermals\n"
		"^510^3:^2 Tripmines\n"
		"^511^3:^2 Detpacks\n"
		"\""));
		trap_SendServerCmd( clientNum, va("print \"^512^3:^2 Bryar Alt Fire\n"
		"^513^3:^2 Blaster Alt Fire\n"
		"^514^3:^2 Disruptor Alt Fire\n"
		"^515^3:^2 Bowcaster Alt Fire\n"
		"^516^3:^2 Repeater Alt Fire\n"
		"^517^3:^2 DEMP2 (Destructive Electro Magnetic Pulse) Alt Fire\n"
		"^518^3:^2 Flechette Alt Fire\n"
		"^519^3:^2 Rockets Alt Fire\n"
		"^520^3:^2 Thermals Alt Fire\n"
		"^521^3:^2 Tripmines Alt Fire\n"
		"^522^3:^2 Seekershot\n"
		"^523^3:^2 None\n"
		"\""));
		return;
	}
	switch(atoi(par1))
	{
		case 0:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 0\n"));break;
		case 1:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 1\n"));break;
		case 2:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 2\n"));break;
		case 3:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 3\n"));break;
		case 4:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 4\n"));break;
		case 5:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 5\n"));break;
		case 6:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 6\n"));break;
		case 7:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 7\n"));break;
		case 8:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 8\n"));break;
		case 9:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 9\n"));break;
		case 10:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 10\n"));break;
		case 11:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 11\n"));break;
		case 12:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 12\n"));break;
		case 13:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 13\n"));break;
		case 14:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 14\n"));break;
		case 15:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 15\n"));break;
		case 16:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 16\n"));break;
		case 17:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 17\n"));break;
		case 18:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 18\n"));break;
		case 19:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 19\n"));break;
		case 20:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 20\n"));break;
		case 21:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 21\n"));break;
		case 22:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 22\n"));break;
		case 23:
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_emplaced_weap 23\n"));break;
		default:
			trap_SendServerCmd( clientNum, va("print \"^3/amturretweapon <0-23>.\n\""));break;
	}
	break;
	case 10:
	/////////////////
	// amweaponspeed
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amweaponspeed <weapon 1-10> <speed>\n\""));
		trap_SendServerCmd( clientNum, va("print \""
		"^51^3:^2 Bryar\n"
		"^52^3:^2 Blaster\n"
		"^53^3:^2 Bowcaster\n"
		"^54^3:^2 Repeater\n"
		"^55^3:^2 DEMP2 (Destructive Electro Magnetic Pulse)\n"
		"^56^3:^2 Flechette\n"
		"^57^3:^2 Rockets\n"
		"^58^3:^2 Repeater Alt Fire\n"
		"^59^3:^2 Flechette Alt Fire\n"
		"^510^3:^2 Seekershot\n"
		"\""));
		return;
	}
	switch (atoi(par1))
	{
		case 1:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bryar Speed: ^5%i^3. Default is ^51600^3.\n\"", mc_bryar_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bryar_vel %i\n", atoi(par2)));
			break;
		case 2:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Blaster Speed: ^5%i^3. Default is ^52300^3.\n\"", mc_blaster_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_blaster_vel %i\n", atoi(par2)));
			break;
		case 3:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bowcaster Speed: ^5%i^3. Default is ^51300^3.\n\"", mc_bowcaster_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bowcaster_vel %i\n", atoi(par2)));
			break;
		case 4:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Repeater Speed: ^5%i^3. Default is ^51600^3.\n\"", mc_repeater_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_repeater_vel %i\n", atoi(par2)));
			break;
		case 5:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3DEMP2 Speed: ^5%i^3. Default is ^51800^3.\n\"", mc_demp2_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_demp2_vel %i\n", atoi(par2)));
			break;
		case 6:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechette Speed: ^5%i^3. Default is ^53500^3.\n\"", mc_flechette_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_vel %i\n", atoi(par2)));
			break;
		case 7:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Speed: ^5%i^3. Default is ^5900^3.\n\"", mc_rocket_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_vel %i\n", atoi(par2)));
			break;
		case 8:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Repeater Alt Speed: ^5%i^3. Default is ^51100^3.\n\"", mc_repeater_alt_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_repeater_alt_vel %i\n", atoi(par2)));
			break;
		case 9:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechette Alt Speed: ^5%i^3. Default is ^5700^3.\n\"", mc_flechette_alt_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_alt_vel %i\n", atoi(par2)));
			break;
		case 10:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Seekershot Speed: ^5%i^3. Default is ^52000^3.\n\"", mc_seekershot_vel.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_seekershot_vel %i\n", atoi(par2)));
			break;
		default:
			trap_SendServerCmd( clientNum, va("print \"^3/amweaponspeed <weapon 1-10> <speed>\n\""));
			break;
	}
	break;
	case 11:
	/////////////////
	// amweaponbounces
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amweaponbounces <0-13> <bounces>. -5 for infinite bounces.\n\""));
		trap_SendServerCmd( clientNum, va("print \""
		"^51^3:^2 Bryar\n"
		"^52^3:^2 Blaster\n"
		"^53^3:^2 Bowcaster\n"
		"^54^3:^2 Repeater\n"
		"^55^3:^2 DEMP2 (Destructive Electro Magnetic Pulse)\n"
		"^56^3:^2 Flechette\n"
		"^57^3:^2 Rockets\n"
		"\""));
		trap_SendServerCmd( clientNum, va("print \""
		"^58^3:^2 Bowcaster Alt Fire\n"
		"^59^3:^2 Repeater Alt Fire\n"
		"^510^3:^2 Flechette Alt Fire\n"
		"^511^3:^2 Seekershot\n"
		"^512^3:^2 Turret Weapon\n"
		"^513^3:^2 Disruptor\n"
		"\""));
		return;
	}
	switch (atoi(par1))
	{
		case 1:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bryar Bounces: ^5%i^3.\n\"", mc_bryar_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bryar_bounces %i\n", atoi(par2)));
			break;
		case 2:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Blaster Bounces: ^5%i^3.\n\"", mc_blaster_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_blaster_bounces %i\n", atoi(par2)));
			break;
		case 3:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bowcaster Bounces: ^5%i^3.\n\"", mc_bowcaster_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bowcaster_bounces %i\n", atoi(par2)));
			break;
		case 4:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Repeater Bounces: ^5%i^3.\n\"", mc_repeater_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_repeater_bounces %i\n", atoi(par2)));
			break;
		case 5:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3DEMP2 Bounces: ^5%i^3.\n\"", mc_demp2_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_demp2_bounces %i\n", atoi(par2)));
			break;
		case 6:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechette Bounces: ^5%i^3. Default ^56^3.\n\"", mc_flechette_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_bounces %i\n", atoi(par2)));
			break;
		case 7:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Bounces: ^5%i^3.\n\"", mc_rocket_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_bounces %i\n", atoi(par2)));
			break;
		case 8:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bowcaster Alt Bounces: ^5%i^3. Default ^53^3.\n\"", mc_bowcaster_alt_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bowcaster_alt_bounces %i\n", atoi(par2)));
			break;
		case 9:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Repeater Alt Bounces: ^5%i^3.\n\"", mc_repeater_alt_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_repeater_alt_bounces %i\n", atoi(par2)));
			break;
		case 10:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechette Alt Bounces: ^5%i^3. Default ^550^3.\n\"", mc_flechette_alt_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_alt_bounces %i\n", atoi(par2)));
			break;
		case 11:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Seekershot Bounces: ^5%i^3.\n\"", mc_seekershot_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_seekershot_bounces %i\n", atoi(par2)));
			break;
		case 12:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Turret Weapon Bounces: ^5%i^3.\n\"", mc_turretweap_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_turretweap_bounces %i\n", atoi(par2)));
			break;
		case 13:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Disruptor Bounces: ^5%i^3.\n\"", mc_disruptor_bounces.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_disruptor_bounces %i\n", atoi(par2)));
			break;
		default:
			trap_SendServerCmd( clientNum, va("print \"^3/amweaponbounces <0-13> <bounces>. -5 for infinite bounces.\n\""));
			break;
	}
	break;
	case 12:
	/////////////////
	// ampassvote
	/////////////////
	if (level.voteTime)
	{
		level.voteYes = 50;
		level.voteNo = 0;
		level.voteTime = level.time+1000;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}
	else
	{
		trap_SendServerCmd( clientNum, va("print \"^3No vote in progress.\n\""));
	}
	break;
	case 13:
	/////////////////
	// amfailvote
	/////////////////
	if (level.voteTime)
	{
		level.voteNo = 50;
		level.voteYes = 0;
		level.voteTime = 1000;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}
	else
	{
		trap_SendServerCmd( clientNum, va("print \"^3No vote in progress.\n\""));
	}
	break;
	case 14:
	/////////////////
	// amweaponfirerate
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amweaponfirerate <1-25>.\n\""));
		trap_SendServerCmd( clientNum, va("print \"\n"
		"^51^3:^2 Stun Baton\n"
		"^52^3:^2 Bryar\n"
		"^53^3:^2 Blaster\n"
		"^54^3:^2 Disruptor\n"
		"^55^3:^2 Bowcaster\n"
		"^56^3:^2 Repeater\n"
		"^57^3:^2 DEMP2 (Destructive Electro Magnetic Pulse)\n"
		"^58^3:^2 Flechette\n"
		"^59^3:^2 Rockets\n"
		"^510^3:^2 Thermals\n"
		"^511^3:^2 Tripmines\n"
		"^512^3:^2 Detpacks\n"
		"\""));
		trap_SendServerCmd( clientNum, va("print \""
		"^513^3:^2 Stun Baton Alt Fire\n"
		"^514^3:^2 Bryar Alt Fire\n"
		"^515^3:^2 Blaster Alt Fire\n"
		"^516^3:^2 Disruptor Alt Fire\n"
		"^517^3:^2 Bowcaster Alt Fire\n"
		"^518^3:^2 Repeater Alt Fire\n"
		"^519^3:^2 DEMP2 (Destructive Electro Magnetic Pulse) Alt Fire\n"
		"^520^3:^2 Flechette Alt Fire\n"
		"^521^3:^2 Rockets Alt Fire\n"
		"^522^3:^2 Thermals Alt Fire\n"
		"^523^3:^2 Tripmines Alt Fire\n"
		"^524^3:^2 Detpack Alt Fire\n"
		"^525^3:^2 Turret Weapon\n"
		"\""));
		return;
	}
	switch(atoi(par1))
	{
		case 1:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Stun Baton Fire Rate: ^5%i^3. Default ^5400^3.\n\"", mc_stunbaton_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_stunbaton_firetime %i\n", atoi(par2)));
			break;
		case 2:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bryar Fire Rate: ^5%i^3. Default ^5400^3.\n\"", mc_bryar_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bryar_firetime %i\n", atoi(par2)));
			break;
		case 3:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Blaster Fire Rate: ^5%i^3. Default ^5450^3.\n\"", mc_blaster_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_blaster_firetime %i\n", atoi(par2)));
			break;
		case 4:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Disruptor Fire Rate: ^5%i^3. Default ^5600^3.\n\"", mc_disruptor_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_disruptor_firetime %i\n", atoi(par2)));
			break;
		case 5:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bowcaster Fire Rate: ^5%i^3. Default ^51000^3.\n\"", mc_bowcaster_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bowcaster_firetime %i\n", atoi(par2)));
			break;
		case 6:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Repeater Fire Rate: ^5%i^3. Default ^5100^3.\n\"", mc_repeater_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_repeater_firetime %i\n", atoi(par2)));
			break;
		case 7:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3DEMP2 Fire Rate: ^5%i^3. Default ^5500^3.\n\"", mc_demp2_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_demp2_firetime %i\n", atoi(par2)));
			break;
		case 8:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechette Fire Rate: ^5%i^3. Default ^5700^3.\n\"", mc_flechette_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_firetime %i\n", atoi(par2)));
			break;
		case 9:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Launcher Fire Rate: ^5%i^3. Default ^5900^3.\n\"", mc_rocket_launcher_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_launcher_firetime %i\n", atoi(par2)));
			break;
		case 10:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Thermal Throw Rate: ^5%i^3. Default ^5800^3.\n\"", mc_thermal_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_thermal_firetime %i\n", atoi(par2)));
			break;
		case 11:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Tripmine Throw Rate: ^5%i^3. Default ^5800^3.\n\"", mc_tripmine_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_tripmine_firetime %i\n", atoi(par2)));
			break;
		case 12:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Detpack Throw Rate: ^5%i^3. Default ^5800^3.\n\"", mc_detpack_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_detpack_firetime %i\n", atoi(par2)));
			break;
		case 13:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Stun Baton Alt Fire Rate: ^5%i^3. Default ^5400^3.\n\"", mc_stunbaton_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_stunbaton_alt_firetime %i\n", atoi(par2)));
			break;
		case 14:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bryar Alt Fire Rate: ^5%i^3. Default ^5400^3.\n\"", mc_bryar_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bryar_alt_firetime %i\n", atoi(par2)));
			break;
		case 15:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Blaster Alt Fire Rate: ^5%i^3. Default ^5150^3.\n\"", mc_blaster_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_blaster_alt_firetime %i\n", atoi(par2)));
			break;
		case 16:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Disruptor Alt Fire Rate: ^5%i^3. Default ^51300^3.\n\"", mc_disruptor_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_disruptor_alt_firetime %i\n", atoi(par2)));
			break;
		case 17:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bowcaster Alt Fire Rate: ^5%i^3. Default ^5750^3.\n\"", mc_bowcaster_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bowcaster_alt_firetime %i\n", atoi(par2)));
			break;
		case 18:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Repeater Alt Fire Rate: ^5%i^3. Default ^5800^3.\n\"", mc_repeater_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_repeater_alt_firetime %i\n", atoi(par2)));
			break;
		case 19:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3DEMP2 Alt Fire Rate: ^5%i^3. Default ^5900^3.\n\"", mc_demp2_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_demp2_alt_firetime %i\n", atoi(par2)));
			break;
		case 20:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechette Alt Fire Rate: ^5%i^3. Default ^5800^3.\n\"", mc_flechette_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_alt_firetime %i\n", atoi(par2)));
			break;
		case 21:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Launcher Alt Fire Rate: ^5%i^3. Default ^51200^3.\n\"", mc_rocket_launcher_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_launcher_alt_firetime %i\n", atoi(par2)));
			break;
		case 22:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Thermal Alt Throw Rate: ^5%i^3. Default ^5400^3.\n\"", mc_thermal_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_thermal_alt_firetime %i\n", atoi(par2)));
			break;
		case 23:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Tripmine Alt Throw Rate: ^5%i^3. Default ^5400^3.\n\"", mc_tripmine_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_tripmine_alt_firetime %i\n", atoi(par2)));
			break;
		case 24:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Detpack Alt Throw Rate: ^5%i^3. Default ^5400^3.\n\"", mc_detpack_alt_firetime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_detpack_alt_firetime %i\n", atoi(par2)));
			break;
		default:
			trap_SendServerCmd( clientNum, va("print \"^3/amweaponfirerate <1-25>.\n\""));
			break;
	}
	break;
	case 15:
	/////////////////
	// amsupershield
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par2, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amsupershield <placetype> (0 - in front of you, 1 - add <angle> to end of command to place it where you stand, 2 - add <x><y><z><angle> to command to place at origin) <group> <extra commands based on placetype>\n\""));
		return;
	}
	if (atoi(par1) == 1)
	{
		if ((Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^3Please specify an angle.\n\""));
			return;
		}
		mcPlaceShield2((int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2], atoi(par3), atoi(par2));
	}
	else if (atoi(par1) == 2)
	{
		if ((Q_stricmp(par6, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^3Please specify X Y Z and an angle.\n\""));
			return;
		}
		mcPlaceShield2(atoi(par3), atoi(par4), atoi(par5), atoi(par6), atoi(par2));
	}
	else
	{
		mcPlaceShield(ent, atoi(par2));
	}
	break;
	case 16:
	/////////////////
	// amsupershield2
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par2, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amsupershield2 <placetype> ( 1 - add <angle> to end of command to place it where you stand, 2 - add <x><y><z><angle> to command to place at origin) <type> (0 - all people, 1 - [HACKS], 2 - AoF) <extra commands based on placetype>\n\""));
		return;
	}
	MANGLE = 0;
	if (atoi(par1) == 1)
	{
		if ((Q_stricmp(par3, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^3Please specify an angle.\n\""));
			return;
		}
		mcPlaceShield2(ent->client->ps.origin[0], ent->client->ps.origin[1], ent->client->ps.origin[2], atoi(par3), atoi(par2));
		MX = ent->client->ps.origin[0];
		MY = ent->client->ps.origin[1];
		MZ = ent->client->ps.origin[2];
		MANGLE = atoi(par3);
	}
	else
	{
		if ((Q_stricmp(par6, "") == 0))
		{
			trap_SendServerCmd( clientNum, va("print \"^3Please specify X Y Z and an angle.\n\""));
			return;
		}
		mcPlaceShield2(atoi(par3), atoi(par4), atoi(par5), atoi(par6), atoi(par2));
		MX = atoi(par3);
		MY = atoi(par4);
		MZ = atoi(par5);
		MANGLE = atoi(par6);

	}
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "%s/mapedits_1_%s.cfg", mc_editfolder.string, mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmcshield \"%i\" \"%i\" \"%i\" \"%i\" \"%i\"\n", atoi(par2), (int)MX, (int)MY, (int)MZ, (int)MANGLE);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	G_Printf(va("Mapedits: mcshield spawned permanently with parameters of: \"%i\" \"%i\" \"%i\" \"%i\" \"%i\"\n", atoi(par2), MX, MY, MZ, MANGLE));
	break;
	case 17:
	/////////////////
	// amgetpos
	/////////////////
	VectorCopy(ent->client->ps.origin, viewspot);
	viewspot[2] += 35;
	VectorCopy( viewspot, src );
	AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
	VectorMA( src, 8192, vf, dest );
	trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_SHOT );
	trap_SendServerCmd( clientNum, va("print \"^7You are aiming at (^5%i^7,^5%i^7, ^5%i^7).\n\"", (int)trace.endpos[0], (int)trace.endpos[1], (int)trace.endpos[2]));
	return;
	break;
	case 18:
	/////////////////
	// amsupersentry
	/////////////////
	if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amsupersentry <placetype(1 - in front of you, 2 - where you stand, 3 - at coordinates)> <group> (if placetype is 3, include <x> <y> <z>) <disable laser?>\n\"" ) );
		return;
	}
	if (atoi(par1) == 2)
	{
		g_entities[mcspawnsentry2( atoi(par2), (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2] - 23, atoi(par3) )].custom = 1;
	}
	else if (atoi(par1) == 3)
	{
		g_entities[mcspawnsentry2( atoi(par2), atoi(par3), atoi(par4), atoi(par5), atoi(par6))].custom = 1;
	}
	else
	{
		g_entities[mcspawnsentry( ent, atoi(par2), atoi(par3))].custom = 1;
	}
	break;
	case 19:
	/////////////////
	// amsupersentry2
	/////////////////
	if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amsupersentry <placetype(2 - where you stand, 3 - at coordinates)> <group> (if placetype is 3, include <x> <y> <z>)\n\"" ) );
		return;
	}
	if (atoi(par1) == 3)
	{
		mcspawnsentry2( atoi(par2), atoi(par3), atoi(par4), atoi(par5));
		MX = atoi(par3);
		MX = atoi(par4);
		MX = atoi(par5);
	}
	else
	{
		mcspawnsentry2( atoi(par2), (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2] );
		MX = ent->client->ps.origin[0];
		MY = ent->client->ps.origin[1];
		MZ = ent->client->ps.origin[2];
	}
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf(savePath, 1024*4, "%s/mapedits_1_%s.cfg", mc_editfolder.string, mapname.string);
	trap_FS_FOpenFile(savePath, &f, FS_APPEND);
	if ( !f )
	{
		return;
	}
	Com_sprintf( line, sizeof(line), "addmcsentry \"%i\" \"%i\" \"%i\" \"%i\"\n", atoi(par2), MX, MY, MZ);
	trap_FS_Write( line, strlen(line), f);
	trap_FS_FCloseFile( f );
	G_Printf(va("Mapedits: mcsentry spawned permanently with parameters of: \"%i\" \"%i\" \"%i\" \"%i\"\n", atoi(par2), MX, MY, MZ));
	break;
	case 20:
	/////////////////
	// amdefaultweapons
	/////////////////
	trap_SendConsoleCommand(EXEC_APPEND, "exec defaults/twifire_default_weaps\n");
	trap_SendServerCmd( clientNum, va("print \"^3Weapons set to default.\n\"" ) );
	G_Printf(va("Weapons set to default.\n"));
	break;
	case 21:
	/////////////////
	// amplaysound
	/////////////////
	if (( Q_stricmp(par1, "info") == 0)||( Q_stricmp(par1, "") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amplaysound <soundfile>\n\"" ) );
		return;
	}
	G_Soundm2( ent, CHAN_ANNOUNCER, G_SoundIndex(par1) );
	break;
	case 22:
	/////////////////
	// ammonitorclientcommands
	/////////////////
	if (ent->client->sess.monitor1 == 1)
	{
		ent->client->sess.monitor1 = 0;
		trap_SendServerCmd( clientNum, va("print \"^3Stopped monitoring client commands.\n\"" ) );
	}
	else
	{
		ent->client->sess.monitor1 = 1;
		trap_SendServerCmd( clientNum, va("print \"^3Began monitoring client commands.\n\"" ) );
	}
	break;
	case 23:
	/////////////////
	// ammonitorservercommands
	/////////////////
	if (ent->client->sess.monitor2 == 1)
	{
		ent->client->sess.monitor2 = 0;
		trap_SendServerCmd( clientNum, va("print \"^3Stopped monitoring server commands.\n\"" ) );
	}
	else
	{
		ent->client->sess.monitor2 = 1;
		trap_SendServerCmd( clientNum, va("print \"^3Began monitoring server commands.\n\"" ) );
	}
	break;
	case 24:
	/////////////////
	// amaddbot
	/////////////////
	Svcmd_AddBot_f();
	break;
	case 25:
	/////////////////
	// amclearips
	/////////////////
	//trap_SendConsoleCommand(EXEC_APPEND, "set g_banIPS \"\"\n");
	//trap_Cvar_Set("g_banIPs","");
	for (i=0 ; i<numIPFilters ; i++)
	{
		{
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed IP.\n");
			UpdateIPBans();
			return;
		}
	}
	break;
	case 26:
	/////////////////
	// amotherweaponsettings
	/////////////////
	if ((Q_stricmp(par1, "") == 0)||(Q_stricmp(par1, "info") == 0))
	{
		trap_SendServerCmd( clientNum, va("print \"^3/amotherweaponsettings <setting 1-12> <new value>\n\""));
		trap_SendServerCmd( clientNum, va("print \""
		"^51^3:^2 Rocket Think Time\n"
		"^52^3:^2 DEMP2 Alt Shot Max Distance\n"
		"^53^3:^2 Flechette Shots\n"
		"^54^3:^2 Bowcaster Spread\n"
		"^55^3:^2 Rocket Lock Time\n"
		"^56^3:^2 Tripmine Limit\n"
		"^57^3:^2 Detpack Limit\n"
		"^58^3:^2 All Weapons - Impact Sky\n"
		"^59^3:^2 All Weapons - LifeTime\n"
		"^510^3:^2 All Weapons - Knockback\n"
		"^511^3:^2 Rocket Lock Distance\n"
		"^512^3:^2 Disruptor Max Distance\n"
		"\""));
		return;
	}
	switch (atoi(par1))
	{
		case 1:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Think Time: ^5%i^3. Default is ^5100^3.\n\"", mc_rocket_alt_think.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_alt_think %i\n", atoi(par2)));
			break;
		case 2:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3DEMP2 Alt Shot Max Distance: ^5%i^3. Default is ^54096^3.\n\"", mc_demp_altrange.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_demp_altrange %i\n", atoi(par2)));
			break;
		case 3:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Flechet Shots: ^5%i^3. Default is ^55^3.\n\"", mc_flechette_shots.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_flechette_shots %i\n", atoi(par2)));
			break;
		case 4:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Bowcaster Spread: ^5%i^3. Default is ^55^3.\n\"", mc_bowcaster_spread.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_bowcaster_spread %i\n", atoi(par2)));
			break;
		case 5:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Lock Time: ^5%i^3. Default is ^510^3.\n\"", mc_rocket_locktime.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocket_locktime %i\n", atoi(par2)));
			break;
		case 6:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Tripmine Limit: ^5%i^3. Default is ^59^3.\n\"", mc_tripmine_limit.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_tripmine_limit %i\n", atoi(par2)));
			break;
		case 7:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Detpack Limit: ^5%i^3. Default is ^59^3.\n\"", mc_detpack_limit.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_detpack_limit %i\n", atoi(par2)));
			break;
		case 8:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3All Weapons - Impact Sky: ^5%i^3. Default is ^50^3(Off. 1 = on).\n\"", mc_weapons_impactsky.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_weapons_impactsky %i\n", atoi(par2)));
			break;
		case 9:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3All Weapons - Life: ^5%i^3. Default is ^510000^3.\n\"", mc_weapons_life.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_weapons_life %i\n", atoi(par2)));
			break;
		case 10:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3All Weapons - Knockback: ^5%i^3. Default is ^51000^3.\n\"", g_knockback.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set g_knockback %i\n", atoi(par2)));
			break;
		case 11:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Rocket Lock Distance: ^5%i^3. Default is ^52048^3.\n\"", mc_rocketdist.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_rocketdist %i\n", atoi(par2)));
			break;
		case 12:
			if ((Q_stricmp(par2, "") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^3Disruptor Max Distance: ^5%i^3. Default is ^58192^3.\n\"", mc_disruptorrange.integer));
				return;
			}
			trap_SendConsoleCommand( EXEC_APPEND, va("set mc_disruptorrange %i\n", atoi(par2)));
			break;
		default:
		trap_SendServerCmd( clientNum, va("print \"^3/amotherweaponsettings <setting 1-12> <new value>\n\""));
			break;
	}
	break;
	case 27:
	/////////////////
	// amsetmusic
	/////////////////
	if (Q_stricmp(par1,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^1/ammusic <musicfile>\n\""));
		return;
	}
	trap_SetConfigstring( CS_MUSIC, par1 );
	break;
	case 28:
	/////////////////
	// amspecialgametype
	/////////////////
	if (Q_stricmp(par1,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^1/amspecialgametype <type>\n\""));
		return;
	}
	else
	{
		int	iL;
		iL = strlen(par1);
		for (i = 0;i < iL;i += 1)
		{
			if (par1[i] == ';')
			{
				trap_SendServerCmd( -1, va("print \"^3Cannot have ; in gametype name.\n\""));
				return;
			}
			if (par1[i] == '\n')
			{
				trap_SendServerCmd( -1, va("print \"^3Cannot have lineskip in gametype name.\n\""));
				return;
			}
		}
		trap_FS_FOpenFile(va("gametypes/gt_%s.cfg", par1), &f, FS_READ);
		if (!f)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown specialgametype ~^5%s^1~.\n\"", par1));
			return;
		}
		trap_FS_FCloseFile(f);
		trap_SendServerCmd( -1, va("print \"^3Specialgametype ^5%s^3 activated.\n\"", par1));
		trap_SendConsoleCommand( EXEC_APPEND, va("exec gametypes/gt_%s.cfg\n", par1));
	}
	break;
	case 29:
	/////////////////
	// amexplode
	/////////////////
	if (Q_stricmp(par5,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^1/amexplode <damage> <radius> <x> <y> <z>\n\""));
		return;
	}
	origin2[0] = (float)monkeyspot(par3, 0, ent, ent);//atof(par2); // X
	origin2[1] = (float)monkeyspot(par4, 1, ent, ent);//atof(par3); // Y
	origin2[2] = (float)monkeyspot(par5, 2, ent, ent);//atof(par4); // Z
	G_RadiusDamage( origin2, ent, atoi(par1)/*Damage*/, /*radius*/atoi(par2), NULL, MOD_REPEATER_ALT_SPLASH );
	VectorClear(angles);
	angles[YAW] = 0;
	angles[PITCH] = -90;
	angles[ROLL] = 0;
	if (atoi(par2) < 50)
	{
		G_PlayEffect_ID(G_EffectIndex( "mouseexplosion1" ), origin2, angles);
	}
	else if (atoi(par2) < 65)
	{
		G_PlayEffect_ID(G_EffectIndex( "droidexplosion1" ), origin2, angles);
	}
	else if (atoi(par2) < 90)
	{
		G_PlayEffect_ID(G_EffectIndex( "atst/side_alt_explosion" ), origin2, angles);
	}
	else if (atoi(par2) < 160)
	{
		G_PlayEffect_ID(G_EffectIndex( "probeexplosion1" ), origin2, angles);
	}
	else if (atoi(par2) < 250)
	{
		G_PlayEffect_ID(G_EffectIndex( "fighter_explosion2" ), origin2, angles);
	}
	else
	{
		G_PlayEffect_ID(G_EffectIndex( "env/huge_explosion" ), origin2, angles);
	}
	break;
	case 30:
	/////////////////
	// amvote
	/////////////////
	if (Q_stricmp(par2,"") == 0)
	{
		trap_SendServerCmd(ent->fixednum,va("print \"^1/amvote <yes/no> <amount>\n\""));
		return;
	}
	if ( par1[0] == 'y' || par1[1] == 'Y' || par1[1] == '1' )
	{
		level.voteYes += atoi(par2);
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	}
	else
	{
		level.voteNo += atoi(par2);
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}
	break;
	case 31:
	/////////////////
	// amwhatvote
	/////////////////
	if (level.voteTime)
	{
		level.voteYes = 0;
		level.voteNo = 0;
		level.voteExecuteTime = 0;
		level.voteTime = 0;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
		trap_SetConfigstring( CS_VOTE_TIME, "" );
	}
	else
	{
		trap_SendServerCmd( clientNum, va("print \"^3No vote in progress.\n\""));
	}
	break;
}
	return;
}
int ent_respawn_old(gentity_t *ent)
{
	gentity_t	*snewent;
	vec3_t	neworigin;
	vec3_t	newangles;
	char	newtarget[MAX_STRING_CHARS];
	char	newtargetname[MAX_STRING_CHARS];
	char	newclassname[MAX_STRING_CHARS];
	char	newmodel[MAX_STRING_CHARS];
	char	newmodel2[MAX_STRING_CHARS];
	char	newteam[MAX_STRING_CHARS];
	char	newTargetShaderName[MAX_STRING_CHARS];
	char	newTargetShaderNewName[MAX_STRING_CHARS];
	char	newfxFile[MAX_STRING_CHARS];
	char	*oldfxfile;
	char	newfxFile2[MAX_STRING_CHARS];
	int	newcount;
	int	newhealth;
	int	newspeed;
	int	newspawnflags;
	int	newdamage;
	int	newrandom;
	int	newwait;
	neworigin[0] = ent->s.origin[0];
	neworigin[1] = ent->s.origin[1];
	neworigin[2] = ent->s.origin[2];
	newangles[0] = ent->s.angles[0];
	newangles[1] = ent->s.angles[1];
	newangles[2] = ent->s.angles[2];
	if (Q_stricmp(ent->target, "") != 0)
	{
		strcpy(newtarget, ent->target);
	}
	if (Q_stricmp(ent->targetname, "") != 0)
	{
		strcpy(newtargetname, ent->targetname);
	}
	if (Q_stricmp(ent->classname, "") != 0)
	{
		strcpy(newclassname, ent->classname);
	}
	if (Q_stricmp(ent->model, "") != 0)
	{
		strcpy(newmodel, ent->model);
	}
	if (Q_stricmp(ent->model2, "") != 0)
	{
		strcpy(newmodel2, ent->model2);
	}
	if (Q_stricmp(ent->team, "") != 0)
	{
		strcpy(newteam, ent->team);
	}
	if (Q_stricmp(ent->targetShaderName, "") != 0)
	{
		strcpy(newTargetShaderName, ent->targetShaderName);
	}
	if (Q_stricmp(ent->targetShaderNewName, "") != 0)
	{
		strcpy(newTargetShaderNewName, ent->targetShaderNewName);
	}
	if (Q_stricmp(ent->fxFile, "") != 0)
	{
		strcpy(newfxFile, ent->fxFile);
	}
	G_SpawnString( "fxFile", "", &oldfxfile );
	if (oldfxfile && oldfxfile[0])
	{
		strcpy(newfxFile2, oldfxfile);
	}
	newcount = ent->count;
	newhealth = ent->health;
	newspeed = ent->speed;
	newspawnflags = ent->spawnflags;
	newdamage = ent->damage;
	newrandom = ent->random;
	newwait = ent->wait;
	G_FreeEntity(ent);
	snewent = G_Spawn();
	if (Q_stricmp(newtarget, "") != 0)
	{
		G_ParseField( "target", newtarget, snewent );
	}
	if (Q_stricmp(newtargetname, "") != 0)
	{
		G_ParseField( "targetname", newtargetname, snewent );
	}
	if (Q_stricmp(newclassname, "") != 0)
	{
		G_ParseField( "classname", newclassname, snewent );
	}
	if (Q_stricmp(newmodel, "") != 0)
	{
		G_ParseField( "model", newmodel, snewent );
	}
	if (Q_stricmp(newmodel2, "") != 0)
	{
		G_ParseField( "model2", newmodel2, snewent );
	}
	if (Q_stricmp(newteam, "") != 0)
	{
		G_ParseField( "team", newteam, snewent );
	}
	if (Q_stricmp(newTargetShaderName, "") != 0)
	{
		G_ParseField( "targetShaderName", newTargetShaderName, snewent );
	}
	if (Q_stricmp(newTargetShaderNewName, "") != 0)
	{
		G_ParseField( "targetShaderNewName", newTargetShaderNewName, snewent );
	}
	if (Q_stricmp(newfxFile, "") != 0)
	{
		G_ParseField( "fxFile", newfxFile, snewent );
	}
	if (Q_stricmp(newfxFile2, "") != 0 || (oldfxfile && oldfxfile[0]))
	{
		ent->bolt_Head = G_EffectIndex( oldfxfile );
	}
	snewent->count = newcount;
	snewent->health = newhealth;
	snewent->speed = newspeed;
	snewent->spawnflags = newspawnflags;
	snewent->damage = newdamage;
	snewent->random = newrandom;
	snewent->wait = newwait;
	snewent->s.origin[0] = neworigin[0];
	snewent->s.origin[1] = neworigin[1];
	snewent->s.origin[2] = neworigin[2];
	snewent->s.angles[0] = newangles[0];
	snewent->s.angles[1] = newangles[1];
	snewent->s.angles[2] = newangles[2];
	G_CallSpawn(snewent);
	G_SetOrigin(snewent, neworigin);
	trap_LinkEntity(snewent);
	return snewent->s.number;
}
int ent_duplicate(gentity_t *ent);
int ent_respawn(gentity_t *ent)
{
	int iFin;
	iFin = ent_duplicate(ent);
	G_FreeEntity(ent);
	return iFin;
}


int ent_duplicate(gentity_t *ent)
{
	gentity_t	*snewent;

	snewent = G_Spawn();
	snewent->custom = 1;
	snewent->s.origin[0] = ent->s.origin[0];
	snewent->s.origin[1] = ent->s.origin[1];
	snewent->s.origin[2] = ent->s.origin[2];
	snewent->s.angles[0] = ent->s.angles[0];
	snewent->s.angles[1] = ent->s.angles[1];
	snewent->s.angles[2] = ent->s.angles[2];
	snewent->r.mins[0] = ent->r.mins[0];
	snewent->r.mins[1] = ent->r.mins[1];
	snewent->r.mins[2] = ent->r.mins[2];
	snewent->r.maxs[0] = ent->r.maxs[0];
	snewent->r.maxs[1] = ent->r.maxs[1];
	snewent->r.maxs[2] = ent->r.maxs[2];
	snewent->s.eType = ent->s.eType;
	snewent->s.eFlags = ent->s.eFlags;
	snewent->s.time = ent->s.time;
	//snewent->s.pos = ent->s.pos;
	//snewent->s.apos = ent->s.apos;
	snewent->s.time2 = ent->s.time2;
	snewent->s.origin2[0] = ent->s.origin2[0];
	snewent->s.origin2[1] = ent->s.origin2[1];
	snewent->s.origin2[2] = ent->s.origin2[2];
	snewent->s.angles2[0] = ent->s.angles2[0];
	snewent->s.angles2[1] = ent->s.angles2[1];
	snewent->s.angles2[2] = ent->s.angles2[2];
	snewent->s.bolt1 = ent->s.bolt1;
	snewent->s.bolt2 = ent->s.bolt2;
	snewent->s.trickedentindex = ent->s.trickedentindex;
	snewent->s.trickedentindex2 = ent->s.trickedentindex2;
	snewent->s.trickedentindex3 = ent->s.trickedentindex3;
	snewent->s.trickedentindex4 = ent->s.trickedentindex4;
	snewent->s.speed = ent->s.speed;
	snewent->s.fireflag = ent->s.fireflag;
	snewent->s.genericenemyindex = ent->s.genericenemyindex;
	snewent->s.activeForcePass = ent->s.activeForcePass;
	snewent->s.emplacedOwner = ent->s.emplacedOwner;
	snewent->s.otherEntityNum = ent->s.otherEntityNum;
	snewent->s.otherEntityNum2 = ent->s.otherEntityNum2;
	snewent->s.groundEntityNum = ent->s.groundEntityNum;
	snewent->s.constantLight = ent->s.constantLight;
	snewent->s.loopSound = ent->s.loopSound;
	snewent->s.modelGhoul2 = ent->s.modelGhoul2;
	snewent->s.g2radius = ent->s.g2radius;
	snewent->s.modelindex = ent->s.modelindex;
	snewent->s.modelindex2 = ent->s.modelindex2;
	snewent->s.clientNum = ent->s.clientNum;
	snewent->s.frame = ent->s.frame;
	snewent->s.saberInFlight = ent->s.saberInFlight;
	snewent->s.saberEntityNum = ent->s.saberEntityNum;
	snewent->s.saberMove = ent->s.saberMove;
	snewent->s.forcePowersActive = ent->s.forcePowersActive;
	snewent->s.isJediMaster = ent->s.isJediMaster;
	snewent->s.solid = ent->s.solid;
	snewent->s.event = ent->s.event;
	snewent->s.eventParm = ent->s.eventParm;
	snewent->s.owner = ent->s.owner;
	snewent->s.teamowner = ent->s.teamowner;
	snewent->s.shouldtarget = ent->s.shouldtarget;
	snewent->s.powerups = ent->s.powerups;
	snewent->s.weapon = ent->s.weapon;
	snewent->s.legsAnim = ent->s.legsAnim;
	snewent->s.torsoAnim = ent->s.torsoAnim;
	snewent->s.forceFrame = ent->s.forceFrame;
	snewent->s.generic1 = ent->s.generic1;
	snewent->r.linked = ent->r.linked;
	snewent->r.linkcount = ent->r.linkcount;
	snewent->r.svFlags = ent->r.svFlags;
	snewent->r.singleClient = ent->r.singleClient;
	snewent->r.bmodel = ent->r.bmodel;
	snewent->r.contents = ent->r.contents;
	snewent->r.mins[0] = ent->r.mins[0];
	snewent->r.mins[1] = ent->r.mins[1];
	snewent->r.mins[2] = ent->r.mins[2];
	snewent->r.maxs[0] = ent->r.maxs[0];
	snewent->r.maxs[1] = ent->r.maxs[1];
	snewent->r.maxs[2] = ent->r.maxs[2];
	snewent->r.absmin[0] = ent->r.absmin[0];
	snewent->r.absmin[1] = ent->r.absmin[1];
	snewent->r.absmin[2] = ent->r.absmin[2];
	snewent->r.absmax[0] = ent->r.absmax[0];
	snewent->r.absmax[1] = ent->r.absmax[1];
	snewent->r.absmax[2] = ent->r.absmax[2];
	snewent->r.currentOrigin[0] = ent->r.currentOrigin[0];
	snewent->r.currentOrigin[1] = ent->r.currentOrigin[1];
	snewent->r.currentOrigin[2] = ent->r.currentOrigin[2];
	snewent->r.currentAngles[0] = ent->r.currentAngles[0];
	snewent->r.currentAngles[1] = ent->r.currentAngles[1];
	snewent->r.currentAngles[2] = ent->r.currentAngles[2];
	G_SetOrigin(snewent, ent->r.currentOrigin);
	G_SetAngles(snewent, ent->r.currentAngles);
	snewent->r.mIsRoffing = ent->r.mIsRoffing;
	snewent->r.ownerNum = ent->r.ownerNum;
	snewent->r.broadcastClients[0] = ent->r.broadcastClients[0];
	snewent->r.broadcastClients[1] = ent->r.broadcastClients[1];
	snewent->r.broadcastClients[2] = ent->r.broadcastClients[2];
	snewent->spawnflags = ent->spawnflags;
	snewent->teamnodmg = ent->teamnodmg;
	snewent->objective = ent->objective;
	snewent->side = ent->side;
	snewent->passThroughNum = ent->passThroughNum;
	snewent->aimDebounceTime = ent->aimDebounceTime;
	snewent->painDebounceTime = ent->painDebounceTime;
	snewent->attackDebounceTime = ent->attackDebounceTime;
	snewent->noDamageTeam = ent->noDamageTeam;
	snewent->roffid = ent->roffid;
	snewent->payable = ent->payable;
	snewent->neverFree = ent->neverFree;
	snewent->flags = ent->flags;
	snewent->freetime = ent->freetime;
	snewent->eventTime = ent->eventTime;
	snewent->freeAfterEvent = ent->freeAfterEvent;
	snewent->unlinkAfterEvent = ent->unlinkAfterEvent;
	snewent->physicsObject = ent->physicsObject;
	snewent->clipmask = ent->clipmask;
	snewent->soundPos1 = ent->soundPos1;
	snewent->sound1to2 = ent->sound1to2;
	snewent->sound2to1 = ent->sound2to1;
	snewent->soundPos2 = ent->soundPos2;
	snewent->soundLoop = ent->soundLoop;
	snewent->parent = ent->parent;
	snewent->nextTrain = ent->nextTrain;
	snewent->prevTrain = ent->prevTrain;
	snewent->pos1[0] = ent->pos1[0];
	snewent->pos1[1] = ent->pos1[1];
	snewent->pos1[2] = ent->pos1[2];
	snewent->pos2[0] = ent->pos2[0];
	snewent->pos2[1] = ent->pos2[1];
	snewent->pos2[2] = ent->pos2[2];
	snewent->timestamp = ent->timestamp;
	snewent->angle = ent->angle;
	snewent->target_ent = ent->target_ent;
	snewent->speed = ent->speed;
	snewent->movedir[0] = ent->movedir[0];
	snewent->movedir[1] = ent->movedir[1];
	snewent->movedir[2] = ent->movedir[2];
	snewent->mass = ent->mass;
	snewent->setTime = ent->setTime;
	snewent->entgroup = ent->entgroup;
	snewent->groupleader = ent->groupleader;
	snewent->nextthink = ent->nextthink;
	snewent->think = ent->think;
	snewent->reached = ent->reached;
	snewent->blocked = ent->blocked;
	snewent->touch = ent->touch;
	snewent->use = ent->use;
	snewent->pain = ent->pain;
	snewent->die = ent->die;
	snewent->pain_debounce_time = ent->pain_debounce_time;
	snewent->fly_sound_debounce_time = ent->fly_sound_debounce_time;
	snewent->last_move_time = ent->last_move_time;
	snewent->health = ent->health;
	snewent->takedamage = ent->takedamage;
	//snewent->material = ent->material;
	snewent->damage = ent->damage;
	snewent->dflags = ent->dflags;
	snewent->splashDamage = ent->splashDamage;
	snewent->splashRadius = ent->splashRadius;
	snewent->methodOfDeath = ent->methodOfDeath;
	snewent->splashMethodOfDeath = ent->splashMethodOfDeath;
	snewent->count = ent->count;
	snewent->bounceCount = ent->bounceCount;
	snewent->alt_fire = ent->alt_fire;
	snewent->chain= ent->chain;
	snewent->enemy = ent->enemy;
	snewent->activator = ent->activator;
	snewent->teamchain = ent->teamchain;
	snewent->teammaster = ent->teammaster;
	snewent->watertype = ent->watertype;
	snewent->waterlevel = ent->waterlevel;
	snewent->noise_index = ent->noise_index;
	snewent->wait = ent->wait;
	snewent->random = ent->random;
	snewent->delay = ent->delay;
	snewent->boltpoint1 = ent->boltpoint1;
	snewent->boltpoint2 = ent->boltpoint2;
	snewent->boltpoint3 = ent->boltpoint3;
	snewent->boltpoint4 = ent->boltpoint4;
	snewent->bolt_Head = ent->bolt_Head;
	snewent->bolt_LArm = ent->bolt_LArm;
	snewent->bolt_RArm = ent->bolt_RArm;
	snewent->bolt_LLeg = ent->bolt_LLeg;
	snewent->bolt_RLeg = ent->bolt_RLeg;
	snewent->bolt_Waist = ent->bolt_Waist;
	snewent->bolt_Motion = ent->bolt_Motion;
	snewent->isSaberEntity = ent->isSaberEntity;
	snewent->damageRedirect = ent->damageRedirect;
	snewent->damageRedirectTo = ent->damageRedirectTo;
	snewent->item = ent->item;
	snewent->origOrigin[0] = ent->origOrigin[0];
	snewent->origOrigin[1] = ent->origOrigin[1];
	snewent->origOrigin[2] = ent->origOrigin[2];
	snewent->spawnedBefore = ent->spawnedBefore;
	snewent->itemtype = ent->itemtype;
	snewent->rprotected = ent->rprotected;
	snewent->xOff = ent->xOff;
	snewent->yOff = ent->yOff;
	snewent->zOff = ent->zOff;
	snewent->lastorigin[0] = ent->lastorigin[0];
	snewent->lastorigin[1] = ent->lastorigin[1];
	snewent->lastorigin[2] = ent->lastorigin[2];
	snewent->lastrot[0] = ent->lastrot[0];
	snewent->lastrot[1] = ent->lastrot[1];
	snewent->lastrot[2] = ent->lastrot[2];
	snewent->nIP[0] = ent->nIP[0];
	snewent->nIP[1] = ent->nIP[1];
	snewent->nIP[2] = ent->nIP[2];
	snewent->nIP[3] = ent->nIP[3];
	snewent->nIP[4] = ent->nIP[4];
	
	if (ent->classname)
	{
		snewent->classname = G_Alloc(strlen(ent->classname)+1);
		strcpy(snewent->classname, ent->classname);
		snewent->classname[strlen(ent->classname)+1] = '';
	}
	if (ent->roffname)
	{
		snewent->roffname = G_Alloc(strlen(ent->roffname)+1);
		strcpy(snewent->roffname, ent->roffname);
		snewent->roffname[strlen(ent->roffname)+1] = '';
	}
	if (ent->rofftarget)
	{
		snewent->rofftarget = G_Alloc(strlen(ent->rofftarget)+1);
		strcpy(snewent->rofftarget, ent->rofftarget);
		snewent->rofftarget[strlen(ent->rofftarget)+1] = '';
	}
	if (ent->model)
	{
		snewent->model = G_Alloc(strlen(ent->model)+1);
		strcpy(snewent->model, ent->model);
		snewent->model[strlen(ent->model)+1] = '';
	}
	if (ent->model2)
	{
		snewent->model2 = G_Alloc(strlen(ent->model2)+1);
		strcpy(snewent->model2, ent->model2);
		snewent->model2[strlen(ent->model2)+1] = '';
	}
	if (ent->message)
	{
		snewent->message = G_Alloc(strlen(ent->message)+1);
		strcpy(snewent->message, ent->message);
		snewent->message[strlen(ent->message)+1] = '';
	}
	if (ent->fxFile)
	{
		snewent->fxFile = G_Alloc(strlen(ent->fxFile)+1);
		strcpy(snewent->fxFile, ent->fxFile);
		snewent->fxFile[strlen(ent->fxFile)+1] = '';
	}
	if (ent->group)
	{
		snewent->group = G_Alloc(strlen(ent->group)+1);
		strcpy(snewent->group, ent->group);
		snewent->group[strlen(ent->group)+1] = '';
	}
	if (ent->target)
	{
		snewent->target = G_Alloc(strlen(ent->target)+1);
		strcpy(snewent->target, ent->target);
		snewent->target[strlen(ent->target)+1] = '';
	}
	if (ent->targetname)
	{
		snewent->targetname = G_Alloc(strlen(ent->targetname)+1);
		strcpy(snewent->targetname, ent->targetname);
		snewent->targetname[strlen(ent->targetname)+1] = '';
	}
	if (ent->team)
	{
		snewent->team = G_Alloc(strlen(ent->team)+1);
		strcpy(snewent->team, ent->team);
		snewent->team[strlen(ent->team)+1] = '';
	}
	if (ent->targetShaderName)
	{
		snewent->targetShaderName = G_Alloc(strlen(ent->targetShaderName)+1);
		strcpy(snewent->targetShaderName, ent->targetShaderName);
		snewent->targetShaderName[strlen(ent->targetShaderName)+1] = '';
	}
	if (ent->targetShaderNewName)
	{
		snewent->targetShaderNewName = G_Alloc(strlen(ent->targetShaderNewName)+1);
		strcpy(snewent->targetShaderNewName, ent->targetShaderNewName);
		snewent->targetShaderNewName[strlen(ent->targetShaderNewName)+1] = '';
	}
	if (ent->upmes)
	{
		//snewent->upmes = G_Alloc(strlen(ent->upmes)+1);
		strcpy(snewent->upmes, ent->upmes);
		snewent->targetShaderNewName[strlen(ent->upmes)+1] = '';
	}
	if (ent->downmes)
	{
		//snewent->downmes = G_Alloc(strlen(ent->downmes)+1);
		strcpy(snewent->downmes, ent->downmes);
		snewent->targetShaderNewName[strlen(ent->downmes)+1] = '';
	}
	strcpy(snewent->IP, ent->IP);
	strcpy(snewent->mcmlight, ent->mcmlight);
	strcpy(snewent->mcpassword, ent->mcpassword);
	strcpy(snewent->mcmessage, ent->mcmessage);
	//strcpy(snewent->mctarget, ent->mctarget);
	//strcpy(snewent->mctargetname, ent->mctargetname);
	//strcpy(snewent->upmes, ent->upmes);
	//strcpy(snewent->downmes, ent->downmes);
	if (ent->r.linked)
	{
		trap_LinkEntity(snewent);
	}
	return snewent->s.number;
}



/*
int ent_duplicateMEH(gentity_t *ent)
{
	gentity_t	*snewent;
	/*char	newtarget[MAX_STRING_CHARS];
	char	newtargetname[MAX_STRING_CHARS];
	char	newclassname[MAX_STRING_CHARS];
	char	newmodel[MAX_STRING_CHARS];
	char	newmodel2[MAX_STRING_CHARS];
	char	newteam[MAX_STRING_CHARS];
	char	newTargetShaderName[MAX_STRING_CHARS];
	char	newTargetShaderNewName[MAX_STRING_CHARS];
	char	newfxFile[MAX_STRING_CHARS];
	char	newmcmessage[MAX_STRING_CHARS];* /

	snewent = G_Spawn();
	snewent->s.origin[0] = ent->s.origin[0];
	snewent->s.origin[1] = ent->s.origin[1];
	snewent->s.origin[2] = ent->s.origin[2];
	snewent->s.angles[0] = ent->s.angles[0];
	snewent->s.angles[1] = ent->s.angles[1];
	snewent->s.angles[2] = ent->s.angles[2];
	snewent->r.mins[0] = ent->r.mins[0];
	snewent->r.mins[1] = ent->r.mins[1];
	snewent->r.mins[2] = ent->r.mins[2];
	snewent->r.maxs[0] = ent->r.maxs[0];
	snewent->r.maxs[1] = ent->r.maxs[1];
	snewent->r.maxs[2] = ent->r.maxs[2];
	snewent->bolt_Head = ent->bolt_Head;
	snewent->s.modelindex = ent->s.modelindex;
	snewent->s.modelindex2 = ent->s.modelindex2;
	//AddSpawnField("model", ent->model);
	if (Q_stricmp(ent->target, "") != 0)
	{
		//strcpy(newtarget, ent->target);
		strcpy(snewent->target,ent->target);
	}
	if (Q_stricmp(ent->targetname, "") != 0)
	{
		//strcpy(newtargetname, ent->targetname);
		strcpy(snewent->targetname,ent->targetname);
	}
	if (Q_stricmp(ent->classname, "") != 0)
	{
		//strcpy(newclassname, ent->classname);
		strcpy(snewent->classname,ent->classname);
	}
	if (Q_stricmp(ent->mcmessage, "") != 0)
	{
		//strcpy(newmcmessage, ent->mcmessage);
		strcpy(snewent->mcmessage,ent->mcmessage);
		AddSpawnField("model",ent->mcmessage);
	}
	if (Q_stricmp(ent->model, "") != 0)
	{
		//strcpy(newmodel, ent->model);
		strcpy(snewent->model,ent->model);
	}
	if (Q_stricmp(ent->model2, "") != 0)
	{
		//strcpy(newmodel2, ent->model2);
		strcpy(snewent->model2,ent->model2);
	}
	if (Q_stricmp(ent->team, "") != 0)
	{
		//strcpy(newteam, ent->team);
		strcpy(snewent->team,ent->team);
	}
	if (Q_stricmp(ent->targetShaderName, "") != 0)
	{
		//strcpy(newTargetShaderName, ent->targetShaderName);
		strcpy(snewent->targetShaderName,ent->TargetShaderName);
	}
	if (Q_stricmp(ent->targetShaderNewName, "") != 0)
	{
		//strcpy(newTargetShaderNewName, ent->targetShaderNewName);
		strcpy(snewent->targetShaderNewName,ent->TargetShaderNewName);
	}

	snewent->count = ent->count;
	snewent->health = ent->health;
	snewent->speed = ent->speed;
	snewent->spawnflags = ent->spawnflags;
	snewent->damage = ent->damage;
	snewent->random = ent->random;
	snewent->wait = ent->wait;
	snewent->delay = ent->delay;
	G_CallSpawn(snewent);
	snewent->count = ent->count;
	snewent->health = ent->health;
	snewent->speed = ent->speed;
	snewent->spawnflags = ent->spawnflags;
	snewent->damage = ent->damage;
	snewent->random = ent->random;
	snewent->wait = ent->wait;
	snewent->delay = ent->delay;
	snewent->r.mins[0] = ent->r.mins[0];
	snewent->r.mins[1] = ent->r.mins[1];
	snewent->r.mins[2] = ent->r.mins[2];
	snewent->r.maxs[0] = ent->r.maxs[0];
	snewent->r.maxs[1] = ent->r.maxs[1];
	snewent->r.maxs[2] = ent->r.maxs[2];
	G_SetOrigin(snewent, snewent->s.origin);
	G_SetAngles(snewent, snewent->s.angles);
	trap_LinkEntity(snewent);
	return snewent->s.number;
}
*/



void mc_buildercmdsmain(gentity_t *ent, int clientNum, int cmd, int chosenentnum, int nomessage);
void mc_buildercmds(gentity_t *ent, int clientNum, int cmd)
{
	char	par1[MAX_STRING_CHARS];
	char	par2[MAX_STRING_CHARS];
	char	par3[MAX_STRING_CHARS];
	char	par4[MAX_STRING_CHARS];
	char	par5[MAX_STRING_CHARS];
	char	buffer[MAX_STRING_CHARS];
	int	chosenentnum;
	int	numfound;
	gentity_t	*chosenent;
	int	iSRT;
	char			line[1024];
	trap_Argv(1, par1, sizeof(par1));
	trap_Argv(2, par2, sizeof(par2));
	trap_Argv(3, par3, sizeof(par3));
	trap_Argv(4, par4, sizeof(par4));
	trap_Argv(5, par5, sizeof(par5));
	if ((cmd == 14)&&((ent->client->sess.grabbedent != 0)||(ent->client->sess.grabbedgroup != 0)))
	{
		mc_buildercmdsmain(ent, clientNum, 14, 4, qfalse);
		return;
	}
	if ( Q_stricmp(par1, "info") != 0)
	{
		chosenentnum = find_ent(ent,par1);
		if (chosenentnum == -1)
		{
			trap_SendServerCmd( clientNum, va("print \"^1Unknown entity. Please give a number or ~gun~.\n\""));
			return;
		}
		if (chosenentnum == -2)
		{
			trap_SendServerCmd( clientNum, va("print \"^1No entity found at gun position.\n\""));
			return;
		}
		if (chosenentnum == -3)
		{
			trap_SendServerCmd( clientNum, va("print \"^1The chosen entity is auto-protected.\n\""));
			return;
		}
		if (chosenentnum == -4)
		{
			trap_SendServerCmd( clientNum, va("print \"^1The chosen entity is protected.\n\""));
			return;
		}
		if (chosenentnum == -14)
		{
			trap_SendServerCmd( clientNum, va("print \"^1You must choose an entity.\n\""));
			return;
		}
		if (chosenentnum == -12) // area_###
		{
			int	encc;
			int	rrange;
			for (iSRT = 0;iSRT < strlen(par1);iSRT+=1)
			{
				buffer[iSRT] = par1[iSRT+5];
			}
			rrange = atoi(buffer);
			encc = 0;
			iSRT = 32;
			for (iSRT = 32;iSRT < 1024;iSRT += 1)
			{
				gentity_t	*t;
				vec3_t		rangle;
				float		dist;
				t = &g_entities[iSRT];
				if (t && t->inuse)
				{
					if (Q_stricmp(t->classname,"player") != 0)
					{	
						if (Q_stricmp(t->classname,"lightsaber") != 0)
						{
							if (!t->rprotected)
							{
								VectorSubtract( t->r.currentOrigin, ent->client->ps.origin, rangle );
								dist = VectorLengthSquared ( rangle );
								if (dist < rrange*rrange)
								{
									encc += 1;
									mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
								}
							}
						}
					}
				}
			}
			if (encc == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1No entities in range.\n\""));
				return;
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^2Found and affected ^5%i^2 entities.\n\"", encc));
				return;
			}
		}
		if (chosenentnum == -42) // all
		{
			int encc;
			encc = 0;
			iSRT = 32;
			for (iSRT = 32;iSRT < 1024;iSRT += 1)
			{
				gentity_t	*t;
				t = &g_entities[iSRT];
				if (t && t->inuse)
				{
					if (cmd == 4 || cmd == 27)
					{
						encc += 1;
						mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
					}
					else if (Q_stricmp(t->classname,"player") != 0)
					{	
						if (Q_stricmp(t->classname,"lightsaber") != 0)
						{
							if (!t->rprotected)
							{
								encc += 1;
								mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
							}
						}
					}
				}
			}
			if (encc == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1No entities can be affected.\n\""));
				return;
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^2Found and affected ^5%i^2 entities.\n\"", encc));
				return;
			}
		}
		if (chosenentnum == -43) // custom
		{
			int encc;
			encc = 0;
			iSRT = 32;
			for (iSRT = 32;iSRT < 1024;iSRT += 1)
			{
				gentity_t	*t;
				t = &g_entities[iSRT];
				if (t && t->inuse)
				{
					if (cmd == 4 || cmd == 27)
					{
						encc += 1;
						mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
					}
					else if (Q_stricmp(t->classname,"player") != 0)
					{	
						if (Q_stricmp(t->classname,"lightsaber") != 0)
						{
							if (t->custom == 1)
							{
								encc += 1;
								mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
							}
						}
					}
				}
			}
			if (encc == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1No entities can be affected.\n\""));
				return;
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^2Found and affected ^5%i^2 entities.\n\"", encc));
				return;
			}
		}
		if (chosenentnum == -5)// byclass_classname
		{
			gentity_t *t;
			for (iSRT = 0;iSRT < strlen(par1);iSRT+=1)
			{
				buffer[iSRT] = par1[iSRT+8];
			}
			if ((Q_stricmp(buffer,"player") == 0)||(Q_stricmp(buffer,"lightsaber") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^1Cannot affect this class.\n\""));
				return;
			}
			t = NULL;
			iSRT = 0;
			while ( (t = G_Find (t, FOFS(classname), buffer)) != NULL )
			{
				mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
				iSRT += 1;
			}
			if (iSRT == 1)
			{
				trap_SendServerCmd( clientNum, va("print \"^3Found ^51^3 entity with the classname ^5%s^3.\n\"", buffer));
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^3Found ^5%i^3 entities with the classname ^5%s^3.\n\"", iSRT, buffer));
			}
			return;
		}
		if (chosenentnum == -63) // bytarget_
		{
			numfound = 0;
			for (iSRT = 0;iSRT < strlen(par1);iSRT+=1)
			{
				buffer[iSRT] = par1[iSRT+9];
			}
			for (iSRT = 31;iSRT < 1024;iSRT += 1)
			{
				gentity_t	*flent;
				flent = &g_entities[iSRT];
				if (!flent || !flent->inuse)
				{
					continue;
				}
				if ((Q_stricmp(buffer, flent->target) == 0)||(Q_stricmp(buffer, flent->targetname) == 0))
				{
					mc_buildercmdsmain(ent, clientNum, cmd, flent->s.number, 1);
					numfound += 1;
				}
			}
			if (numfound == 1)
			{
				trap_SendServerCmd( clientNum, va("print \"^3Found ^51^3 entity.\n\""));
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^3Found ^5%i^3 entities.\n\"", numfound));
			}
			return;
		}
		if (chosenentnum == -6)// bygroup_groupnum
		{
			gentity_t	*t;
			int		gnum;
			for (iSRT = 0;iSRT < strlen(par1);iSRT+=1)
			{
				buffer[iSRT] = par1[iSRT+8];
			}
			if (atoi(buffer) == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1Cannot affect group 0.\n\""));
				return;
			}
			if (cmd == 14) // grabbing
			{
				gentity_t	*Lead;
				int		leadnum;
				if (( Q_stricmp(par1, "info") == 0)||(Q_stricmp(par2,"") == 0))
				{
					trap_SendServerCmd( clientNum, va("print \"^1/ammap_grabent <entity> <distance>\n\"" ) );
					return;
				}
				t = NULL;
				iSRT = 0;
				while ( (t = G_Findbygroup(t, atoi(buffer))) != NULL )
				{
					gentity_t	*tEn;
					int		iPl;
					gentity_t	*t2;
					for (iPl = 0;iPl < 32;iPl += 1)
					{
						tEn = &g_entities[iPl];
						if (!tEn->inuse)
						{
							continue;
						}
						if ((tEn->client->sess.grabbedent == t->s.number))
						{
							trap_SendServerCmd( clientNum, va("print \"^1Another player is holding this ent.\n\"") );
							ent->client->sess.grabbedent = 0;
							ent->client->sess.grabbeddist = 0;
							ent->client->sess.grabbedgroup = 0;
							return;
						}
						if (tEn->client->sess.grabbedgroup == atoi(buffer))
						{
							trap_SendServerCmd( clientNum, va("print \"^1Another player is holding this group.\n\"") );
							ent->client->sess.grabbedent = 0;
							ent->client->sess.grabbeddist = 0;
							ent->client->sess.grabbedgroup = 0;
							return;
						}
					}
				}
				ent->client->sess.grabbedent = 0;
				ent->client->sess.grabbeddist = atoi(par2);
				ent->client->sess.grabbedgroup = atoi(buffer);
				t = NULL;
				iSRT = 0;
				while ( (t = G_Findbygroup(t, atoi(buffer))) != NULL )
				{
					if (t->groupleader != 0)
					{
						Lead = &g_entities[t->groupleader];
						leadnum = t->groupleader;
					}
					iSRT += 1;
				}
				if (iSRT == 0)
				{
					trap_SendServerCmd( clientNum, va("print \"^2Chosen group does not exist.\n\"", atoi(buffer) ) );
					ent->client->sess.grabbedent = 0;
					ent->client->sess.grabbeddist = 0;
					ent->client->sess.grabbedgroup = 0;
					return;
				}
				if (leadnum == 0)
				{
					t = G_Findbygroup(t, atoi(buffer));
					leadnum = t->s.number;
					t = NULL;
					while ( (t = G_Findbygroup(t, atoi(buffer))) != NULL )
					{
						t->groupleader = leadnum;
					}
					Lead = &g_entities[leadnum];
				}
				t = NULL;
				while ( (t = G_Findbygroup(t, atoi(buffer))) != NULL )
				{
					t->xOff = t->r.currentOrigin[0] - Lead->r.currentOrigin[0];
					t->yOff = t->r.currentOrigin[1] - Lead->r.currentOrigin[1];
					t->zOff = t->r.currentOrigin[2] - Lead->r.currentOrigin[2];
				}
				ent->client->sess.grabbedentoffz = atoi(par3);
				trap_SendServerCmd( clientNum, va("print \"^2Grabbed group ^5%i^2.\n\"", atoi(buffer) ) );
				return;
			}
			t = NULL;
			iSRT = 0;
			numfound = 0;
			//while ( (t = G_Findbygroup(t, atoi(buffer))) != NULL )
			gnum = atoi(buffer);
			for (iSRT = 32;iSRT < 1024;iSRT += 1)
			{
				t = &g_entities[iSRT];
				if (t && t->inuse && t->entgroup == gnum)
				{
					mc_buildercmdsmain(ent, clientNum, cmd, t->s.number, 1);
					numfound += 1;
				}
			}
			if (numfound == 1)
			{
				trap_SendServerCmd( clientNum, va("print \"^3Found ^51^3 entity in group ^5%i^3.\n\"", atoi(buffer)));
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^3Found ^5%i^3 entities in group ^5%i^3.\n\"", numfound, atoi(buffer)));
			}
			return;
		}
	}
	mc_buildercmdsmain(ent, clientNum, cmd, chosenentnum, 0);
}

void Blocked_Door( gentity_t *ent, gentity_t *other );
void Reached_BinaryMover( gentity_t *ent );
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator );
void mc_buildercmdsmain(gentity_t *ent, int clientNuxm, int cmd, int chosenentnum, int nomessage)
{
	char	par1[MAX_STRING_CHARS];
	char	par2[MAX_STRING_CHARS];
	char	par3[MAX_STRING_CHARS];
	char	par4[MAX_STRING_CHARS];
	char	par5[MAX_STRING_CHARS];
	char	buffer[MAX_STRING_CHARS];
	vec3_t	neworigin;
	vec3_t	delta;
	int	parcount;
	gentity_t	*chosenent;
	int	e_count;
	int	i;
	vec3_t	origin,origin2,move;
	char	e_mctarget[MAX_STRING_CHARS];
	char	e_mctargetname[MAX_STRING_CHARS];
	char	e_mcmessage[MAX_STRING_CHARS];
	char			savePath[MAX_QPATH];
	vmCvar_t		mapname;
	fileHandle_t	f;
	char			line[2048];
	int			clientNum;
	clientNum = ent->s.number;
	trap_Argv(1, par1, sizeof(par1));
	trap_Argv(2, par2, sizeof(par2));
	trap_Argv(3, par3, sizeof(par3));
	trap_Argv(4, par4, sizeof(par4));
	trap_Argv(5, par5, sizeof(par5));
	parcount = trap_Argc();
	/*if ( Q_stricmp(par1, "info") != 0)
	{
	chosenentnum = find_ent(ent,par1);
	if (chosenentnum == -1)
	{
		trap_SendServerCmd( clientNum, va("print \"^7Unknown entity. Please give a number or ~gun~.\n\""));
		return;
	}
	if (chosenentnum == -2)
	{
		trap_SendServerCmd( clientNum, va("print \"^7No entity found at gun position.\n\""));
		return;
	}*/
	chosenent = &g_entities[chosenentnum];
	chosenentnum = chosenent->s.number; // ?????? I don't why or how, but this helps.
	//}
	switch ( cmd )
	{
		case 1:
		///////////////////
		// ammap_nudgeent
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_nudgeent <entity> <xadjust> <yadjust> <zadjust>\n\"" ) );
				return;
			}
			if (parcount < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_nudgeent <entity> <xadjust> <yadjust> <zadjust>\n\"" ) );
				return;
			}
			fixforundo(chosenent);
			chosenent->doorchanged = 32;
			chosenent->s.origin[0] = chosenent->r.currentOrigin[0]+atoi(par2);
			chosenent->s.origin[1] = chosenent->r.currentOrigin[1]+atoi(par3);
			chosenent->s.origin[2] = chosenent->r.currentOrigin[2]+atoi(par4);
			//chosenent->s.pos.trBase[0] += atoi(par2);
			//chosenent->s.pos.trBase[1] += atoi(par3);
			//chosenent->s.pos.trBase[2] += atoi(par4);
			G_SetOrigin(chosenent, chosenent->s.origin);
			trap_LinkEntity(chosenent);

		break;
		case 2:
		///////////////////
		// ammap_delent
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_delent <entity>\n\"" ) );
				return;
			}
			if (parcount < 2)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_delent <entity>\n\"" ) );
				return;
			}
			if (nomessage == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^3Deleted ^5%i^3(^5%s^3).\n\"", chosenentnum, chosenent->classname ) );
			}
			G_Printf("[%i]", chosenentnum);
			if (chosenent->parent && chosenent->parent->inuse)
			{
				G_Printf("[Child of %i]", chosenent->parent->s.number);
			}
			G_Printf("DELETE: %s, target %s, targetname %s\n", chosenent->classname, chosenent->target, chosenent->targetname);
			G_FreeEntity(chosenent);
		break;
		case 3:
		///////////////////
		// ammap_setvar
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setvar <entity> <variable> <newvalue>\n\"" ) );
				return;
			}
			if (parcount < 4)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setvar <entity> <variable> <newvalue>\n\"" ) );
				return;
			}
			if (Q_stricmp(par2, "model") == 0)
			{
				ent->s.modelindex = G_ModelIndex( par3 );
				ent->s.modelindex2 = G_ModelIndex( par3 );
				//AddSpawnField("model", par3);
			}
			G_ParseField( par2, par3, chosenent );

		break;
		case 4:
		///////////////////
		// ammap_trace
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_trace <entity>\n\"" ) );
				return;
			}
			if (parcount < 2)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_trace <entity>\n\"" ) );
				return;
			}
			Com_sprintf( buffer, sizeof(buffer), "Trace ent: %i - %s\n", chosenentnum, chosenent->classname);
			Com_sprintf( buffer, sizeof(buffer), "%s^7X=^5%i^7, Y=^5%i^7, Z=^5%i^7\n", buffer, (int)chosenent->r.currentOrigin[0], (int)chosenent->r.currentOrigin[1], (int)chosenent->r.currentOrigin[2]);
			ent->client->sess.traced = chosenent->s.number;
			if (chosenent->s.angles[YAW] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(YAW = %i) ", buffer, (int)chosenent->s.angles[YAW]);
			}
			if (chosenent->s.angles[PITCH] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(PITCH = %i) ", buffer, (int)chosenent->s.angles[PITCH]);
			}
			if (chosenent->s.angles[ROLL] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(ROLL = %i) ", buffer, (int)chosenent->s.angles[ROLL]);
			}
			if (chosenent->count)
			{
				if (chosenent->count != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(count = %i) ", buffer, (int)chosenent->count);
				}
			}
			if (chosenent->custom != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(custom) ", buffer);
			}
			else
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(default) ", buffer);
			}
			if (chosenent->health)
			{
				if (chosenent->health != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(health = %i) ", buffer, (int)chosenent->health);
				}
			}
			if (chosenent->issaved)
			{
				if (chosenent->issaved != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(Saved) ", buffer);
				}
			}
			if (chosenent->wait)
			{
				if (chosenent->wait != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(wait = %i) ", buffer, (int)chosenent->wait);
				}
			}
			if (chosenent->spawnflags)
			{
				if (chosenent->spawnflags != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(spawnflags = %i) ", buffer, (int)chosenent->spawnflags);
				}
			}
			if (chosenent->damage)
			{
				if (chosenent->damage != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(damage = %i) ", buffer, (int)chosenent->damage);
				}
			}
			if (chosenent->speed)
			{
				if (chosenent->speed != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(speed = %i) ", buffer, (int)chosenent->speed);
				}
			}
			if (chosenent->random)
			{
				if (chosenent->random != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(random = %i) ", buffer, (int)chosenent->random);
				}
			}
			if (chosenent->target)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(target = %s) ", buffer, chosenent->target);
			}
			if (chosenent->targetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(targetname = %s) ", buffer, chosenent->targetname);
			}
			/*
			if (chosenent->mctargetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(other = %s) ", buffer, chosenent->mctargetname);
			}
			*/
			if (chosenent->model)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->model);
			}
			if (Q_stricmp(chosenent->mcmessage,"")!=0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->mcmessage);
			}
			if (chosenent->delay)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(delay = %i) ", buffer, chosenent->delay);
			}
			if (chosenent->entgroup)
			{
				if (chosenent->entgroup != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(group = %i) ", buffer, (int)chosenent->entgroup);
					Com_sprintf( buffer, sizeof(buffer), "%s(groupleader = %i) ", buffer, (int)chosenent->groupleader);
				}
			}
			Com_sprintf( buffer, sizeof(buffer), "%s(mins = %i %i %i) ", buffer, (int)chosenent->r.mins[0], (int)chosenent->r.mins[1], (int)chosenent->r.mins[2]);
			Com_sprintf( buffer, sizeof(buffer), "%s(maxes = %i %i %i) ", buffer, (int)chosenent->r.maxs[0], (int)chosenent->r.maxs[1], (int)chosenent->r.maxs[2]);
			trap_SendServerCmd( clientNum, va("print \"^7%s\n\"", buffer ) );

		break;
		case 5:
		///////////////////
		// ammap_respawn
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_respawn <entity>\n\"" ) );
				return;
			}
			if (parcount < 2)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_respawn <entity>\n\"" ) );
				return;
			}
			if (nomessage == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^3Respawned ent as ^5%i^7.\n\"", ent_respawn(chosenent) ) );
			}
			//ent_respawn(chosenent);
		break;
		case 6:
		///////////////////
		// ammap_duplicate
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_duplicate <entity>\n\"" ) );
				return;
			}
			if (parcount < 2)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_duplicate <entity>\n\"" ) );
				return;
			}
			if (nomessage == 0)
			{
			trap_SendServerCmd( clientNum, va("print \"^3Duplicated ent to ^5%i^7.\n\"",  ent_duplicate(chosenent)) );
			}
			//ent_duplicate(chosenent);
		break;
		case 7:
		///////////////////
		// ammap_setangles
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setangles <entity> <Yaw> <Pitch> <Roll>\n\"" ) );
				return;
			}
			if (parcount < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setangles <entity> <Yaw> <Pitch> <Roll>\n\"" ) );
				return;
			}
			fixforundor(chosenent);
			chosenent->s.angles[YAW] = mc_fix360i(atoi(par2));
			chosenent->s.angles[PITCH] = mc_fix360i(atoi(par3));
			chosenent->s.angles[ROLL] = mc_fix360i(atoi(par4));
			G_SetAngles(chosenent, chosenent->s.angles);
			trap_LinkEntity(chosenent);
		break;
		case 8:
		///////////////////
		// ammap_useent
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(Q_stricmp(par1,"") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_useent <entity>\n\"" ) );
				return;
			}
			chosenent->use( chosenent, ent, ent );
		break;
		case 9:
		///////////////////
		// ammap_setmaxes
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setmaxes <entity> <XMAX> <YMAX> <YMAX>\n\"" ) );
				return;
			}
			if (parcount < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setmaxes <entity> <XMAX> <YMAX> <YMAX> - Chosen object maxes - %s\n\"", vtos(chosenent->r.maxs) ) );
				return;
			}
			chosenent->r.maxs[0] = atoi(par2);
			chosenent->r.maxs[1] = atoi(par3);
			chosenent->r.maxs[2] = atoi(par4);
			trap_LinkEntity(chosenent);
		break;
		case 10:
		///////////////////
		// ammap_setmins
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setmins <entity> <XMIN> <YMIN> <YMIN>\n\"" ) );
				return;
			}
			if (parcount < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setmins <entity> <XMIN> <YMIN> <YMIN> - Chosen object mins - %s\n\"", vtos(chosenent->r.mins) ) );
				return;
			}
			chosenent->r.mins[0] = atoi(par2);
			chosenent->r.mins[1] = atoi(par3);
			chosenent->r.mins[2] = atoi(par4);
			trap_LinkEntity(chosenent);
		break;
		case 11:
		///////////////////
		// ammap_respawn2
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_respawn <entity>\n\"" ) );
				return;
			}
			if (parcount < 2)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_respawn <entity>\n\"" ) );
				return;
			}
			if (nomessage == 0)
			{
			trap_SendServerCmd( clientNum, va("print \"^3Respawned ent.\n\"" ) );
			}
			//ent_respawn(chosenent);
			G_CallSpawn(chosenent);
		break;
		case 12:
		///////////////////
		// ammap_moveent
		///////////////////
			if (parcount < 4)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_moveent <entity> <distance> <speed>\n\"" ) );
				return;
			}
			//if ( chosenent->s.pos.trType != TR_STATIONARY || chosenent->s.apos.trType != TR_STATIONARY )
			if (qtrue)
			{
				vec3_t	fwd;
				int	distance = atoi(par2);
			chosenent->doorchanged = 32;
				chosenent->sound1to2 = chosenent->sound2to1 = 0;//G_SoundIndex("sound/movers/doors/door1start.wav");
				chosenent->soundPos1 = chosenent->soundPos2 = 0;//G_SoundIndex("sound/movers/doors/door1stop.wav");
				chosenent->soundLoop = 0;//G_SoundIndex("sound/movers/doors/door1move.wav");
				chosenent->blocked = Blocked_Door;
				chosenent->speed = atoi(par3);
				chosenent->wait = 9300170;
				chosenent->damage = 2;
				VectorCopy( chosenent->r.currentOrigin, chosenent->pos1 );
				trap_LinkEntity( ent );
				AngleVectors(ent->client->ps.viewangles, fwd, NULL, NULL);
				VectorClear(chosenent->pos2 );
				chosenent->pos2[0] = chosenent->r.currentOrigin[0] + fwd[0]*distance;
				chosenent->pos2[1] = chosenent->r.currentOrigin[1] + fwd[1]*distance;
				chosenent->pos2[2] = chosenent->r.currentOrigin[2] + fwd[2]*distance;
				InitMover( chosenent );
				chosenent->health = 0;
				chosenent->use(chosenent, chosenent, ent);
			}
		break;
		case 13:
		///////////////////
		// ammap_saveent
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(parcount < 2))
			{
				trap_SendServerCmd( ent->s.number, va("print \"^1/ammap_saveent <entity>\n\"" ) );
				return;
			}
			else
			{
			int	iL;
			int	savetype;
			trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
			for (i = 1;i < 32;i += 1)
			{
				iL = trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_READ);
				if (!f)
				{
					trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i-1, mapname.string), &f, FS_APPEND);
					if ( !f )
					{
						trap_SendServerCmd( clientNum, va("print \"^1Error: Cannot access map file.\n\"" ) );
						return;
					}
					trap_FS_Write( va("\nmapeditsexec mapedits_%i_%s;\n", i, mapname.string), strlen(va("\nmapeditsexec mapedits_%i_%s;\n", i, mapname.string)), f);
					trap_FS_FCloseFile( f );
					goto loadmap;
				}
				trap_FS_FCloseFile(f);
				if (iL < 10000)
				{
					goto loadmap;
				}
			}
			loadmap:
			if (chosenent->issaved == 1 || chosenent->custom == 0)
			{
				if (Q_stricmp(par3, "override") != 0)
				{
					trap_SendServerCmd( ent->s.number, va("print \"^1Chosen entity is already saved. To override this, /ammap_save <ent> override override.\n\"" ) );
					return;
				}
			}
			Com_sprintf(savePath, 1024*4, "%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string);
			trap_FS_FOpenFile(savePath, &f, FS_APPEND);
			if ( !f )
			{
				trap_SendServerCmd( ent->s.number, va("print \"^1Error: Cannot access map file.\n\"" ) );
				G_Printf("[Can'tOpenMapFile]Refused.\n");
				return;
			}
			chosenent->issaved = 1;
			savetype = 0;
			if (Q_stricmp(par2, "savehack1") == 0)
			{
				savetype = 1;
			}
			if (Q_stricmp(chosenent->classname,"mc_model") == 0)
			{
				if (savetype != 0)
				{
				G_Printf("ModelSavehack\n");
				Com_sprintf(line,sizeof(line), "{\n\"classname\" \"misc_model\"\n"
					"\"origin\" \"%i %i %i\"\n"
					"\"angles\" \"%i %i %i\"\n"
					"\"model\" \"%s\"\n"
					"}\n", (int)chosenent->r.currentOrigin[0], (int)chosenent->r.currentOrigin[1], (int)chosenent->r.currentOrigin[2],
						(int)chosenent->r.currentAngles[0], (int)chosenent->r.currentAngles[1], (int)chosenent->r.currentAngles[2],
							chosenent->mcmessage);
				}
				else
				{
				Com_sprintf( line, sizeof(line), "addmodelrcon3 \"%s\" %i %i %i %i %i %i %i %i %i %i %i %i", chosenent->mcmessage,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW],
				 (int)chosenent->r.currentAngles[PITCH],
				 (int)chosenent->r.currentAngles[ROLL],
				 (int)chosenent->r.mins[0],
				 (int)chosenent->r.mins[1],
				 (int)chosenent->r.mins[2],
				 (int)chosenent->r.maxs[0],
				 (int)chosenent->r.maxs[1],
				 (int)chosenent->r.maxs[2]);
				}
			}
			else if (Q_stricmp(chosenent->classname,"mc_light") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				Com_sprintf(line,sizeof(line), "addlightrcon %i %i %i %s", (int)chosenent->s.origin[0],
				(int)chosenent->s.origin[1],
				(int)chosenent->s.origin[2],
				chosenent->mcmlight);
				}
				
			}
			else if (Q_stricmp(chosenent->classname,"mc_model2") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				Com_sprintf( line, sizeof(line), "addghoulrcon \"%s\" %i %i %i %i %i %i %i %i %i %i %i %i", chosenent->mcmessage,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW],
				 (int)chosenent->r.currentAngles[PITCH],
				 (int)chosenent->r.currentAngles[ROLL],
				 (int)chosenent->r.mins[0],
				 (int)chosenent->r.mins[1],
				 (int)chosenent->r.mins[2],
				 (int)chosenent->r.maxs[0],
				 (int)chosenent->r.maxs[1],
				 (int)chosenent->r.maxs[2]);
				}
			}
			else if (Q_stricmp(chosenent->classname,"mc_effect") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				//addeffectrcon env/fire 4272 1770 559 0 0 0 300

				Com_sprintf( line, sizeof(line), "addeffectrcon \"%s\" %i %i %i %i %i %i %i", chosenent->mcmessage,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW],
				 (int)chosenent->r.currentAngles[PITCH],
				 (int)chosenent->r.currentAngles[ROLL],
				 (int)chosenent->delay);
				}
			}
		
			else if (Q_stricmp(chosenent->classname,"mc_note") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				// addnotercon "creator" "message" public X Y Z

				Com_sprintf( line, sizeof(line), "addnotercon \"%s\" \"%s\" %i %i %i %i", chosenent->mctargetname,
				chosenent->mcmessage,
				(int)chosenent->s.bolt1,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2]);
				}
			}
		
			else if (Q_stricmp(chosenent->classname,"mc_tsent") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				Com_sprintf( line, sizeof(line), "addanyent mc_tsent %i %i %i \"group,%s,",
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 chosenent->group);
				if (chosenent->bolt_Waist == 1)
				{
					Com_sprintf( line, sizeof(line), "%sreaction,kick,\"", line);
				}
				else
				{
					Com_sprintf( line, sizeof(line), "%sreaction,kill,\"", line);
				}
				}
			}
			else if (Q_stricmp(chosenent->classname,"target_speaker") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				Com_sprintf( line, sizeof(line), "addanyent \"target_speaker\" %i %i %i \"targetname,%s,noise,%s,spawnflags,%i,wait,%i,random,%i,\"",
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 chosenent->targetname,
				 chosenent->upmes,
				 (int)chosenent->spawnflags,
				 (int)chosenent->wait,
				 (int)chosenent->random);
				}
			}
			else if (Q_stricmp(chosenent->classname,"mcsentry") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				Com_sprintf( line, sizeof(line), "addmcsentry \"%i\" \"%i\" \"%i\" \"%i\"", chosenent->s.owner, 
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2]);
				}
			}
			else if (Q_stricmp(chosenent->classname,"mcshield") == 0)
			{
				if (savetype == 1)
				{
				}
				else
				{
				Com_sprintf( line, sizeof(line), "addmcshield \"%i\" \"%i\" \"%i\" \"%i\" \"%i\"", chosenent->s.owner, 
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW]);
				}
			}
			else
			{
				if (savetype == 1)
				{
				}
				else
				{
				/*if( strstr(chosenent->classname,"func_") || strstr(chosenent->classname,"trigger_")) {
					trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
					trap_FS_FCloseFile( f );
					return;
				}*/

				Com_sprintf( line, sizeof(line), "addanyent \"%s\" %i %i %i \"angles,%i %i %i,spawnflags,%i,mins,%i %i %i,maxs,%i %i %i,", chosenent->classname,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[0],
				 (int)chosenent->r.currentAngles[1],
				 (int)chosenent->r.currentAngles[2],
				 (int)chosenent->spawnflags,
				 (int)chosenent->r.mins[0],
				 (int)chosenent->r.mins[1],
				 (int)chosenent->r.mins[2],
				 (int)chosenent->r.maxs[0],
				 (int)chosenent->r.maxs[1],
				 (int)chosenent->r.maxs[2]);
				if (chosenent->movedir[0] != 0 || chosenent->movedir[1] != 0 || chosenent->movedir[2] != 0)
				{
					vec3_t final;
					VectorClear(final);
					vectoangles(chosenent->movedir, final);
					Com_sprintf( line, sizeof(line), "%sangles,%i %i %i,", line,
					 (int)final[0], (int)final[1], (int)final[2]);
				}
				if (chosenent->target && Q_stricmp(chosenent->target,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%starget,%s,", line,
					 chosenent->target);
				}
				if (chosenent->targetname && Q_stricmp(chosenent->targetname,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%stargetname,%s,", line,
					 chosenent->targetname);
				}
				if (chosenent->targetShaderName && Q_stricmp(chosenent->targetShaderName,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%stargetShaderName,%s,", line,
					 chosenent->targetShaderName);
				}
				if (chosenent->targetShaderNewName && Q_stricmp(chosenent->targetShaderNewName,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%stargetShaderNewName,%s,", line,
					 chosenent->targetShaderNewName);
				}
				if (chosenent->message && Q_stricmp(chosenent->message,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%smessage,%s,", line,
					 chosenent->message);
				}
				if (chosenent->team && Q_stricmp(chosenent->team,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%steam,%s,", line,
					 chosenent->team);
				}
				if (chosenent->upmes && Q_stricmp(chosenent->upmes,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%supmessage,%s,", line,
					 chosenent->upmes);
				}
				if (chosenent->downmes && Q_stricmp(chosenent->downmes,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%sdownmessage,%s,", line,
					 chosenent->downmes);
				}
				if (chosenent->mcpassword && Q_stricmp(chosenent->mcpassword,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%spassword,%s,", line,
					 chosenent->mcpassword);
				}
				if (chosenent->group && Q_stricmp(chosenent->group,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%sgroup,%s,", line,
					 chosenent->group);
				}
				if (chosenent->mcmessage && Q_stricmp(chosenent->mcmessage,"") != 0)
				{
					if (Q_stricmp(chosenent->classname,"fx_runner") == 0)
					{
						Com_sprintf( line, sizeof(line), "%sfxfile,%s,", line,
						 chosenent->mcmessage);
					}
					else
					{
						Com_sprintf( line, sizeof(line), "%smodel,%s,", line,
						 chosenent->mcmessage);
					}
				}
				if (chosenent->speed != 0)
				{
					Com_sprintf( line, sizeof(line), "%sspeed,%i,", line,
					 (int)chosenent->speed);
				}
				if (chosenent->wait != 0)
				{
					Com_sprintf( line, sizeof(line), "%swait,%i,", line,
					 (int)chosenent->wait);
				}
				if (chosenent->random != 0)
				{
					Com_sprintf( line, sizeof(line), "%srandom,%i,", line,
					 (int)chosenent->random);
				}
				if (chosenent->lip != 0)
				{
					Com_sprintf( line, sizeof(line), "%slip,%i,", line,
					 (int)chosenent->lip);
				}
				if (chosenent->count != 0)
				{
					Com_sprintf( line, sizeof(line), "%scount,%i,", line,
					 (int)chosenent->count);
				}
				if (chosenent->health != 0)
				{
					Com_sprintf( line, sizeof(line), "%shealth,%i,", line,
					 (int)chosenent->health);
				}
				if (chosenent->delay != 0)
				{
					Com_sprintf( line, sizeof(line), "%sdelay,%i,", line,
					 (int)chosenent->delay);
				}
				if (chosenent->damage != 0)
				{
					Com_sprintf( line, sizeof(line), "%sdmg,%i,", line,
					 (int)chosenent->damage);
				}
				if (chosenent->s.owner != 0)
				{
					Com_sprintf( line, sizeof(line), "%sowner,%i,", line,
					 (int)chosenent->s.owner);
				}
				if (chosenent->xOff != 0)
				{
					Com_sprintf( line, sizeof(line), "%smodelangle,%i,", line,
					 (int)chosenent->xOff);
				}
				if (chosenent->yOff != 0)
				{
					Com_sprintf( line, sizeof(line), "%smodelpitch,%i,", line,
					 (int)chosenent->yOff);
				}
				if (chosenent->zOff != 0)
				{
					Com_sprintf( line, sizeof(line), "%smodelroll,%i,", line,
					 (int)chosenent->zOff);
				}
				Com_sprintf( line, sizeof(line), "%s\" \"\"", line);
				}
				//trap_SendServerCmd( ent->s.number, va("print \"^1Cannot save items of this class.\n\"" ) );
				//return;
			}
			/*if( strchr( line, ';' ) ) {
				trap_SendServerCmd( ent->s.number, va("print \"^1You should probably be banned for that.\n\""));
				trap_FS_FCloseFile( f );
				G_Printf("Refused.\n");
				return;
			}*/
			if ((savetype != 1)&&(strchr( line, '\n' ) || strchr(line,';') )) {
				char	fixhelp[1024];
				int	iFH;
				int	iFH2;
				iFH = 0;
				iFH2 = 0;
				stringclear(fixhelp, 1020);
				for (iFH = 0;iFH < 1020;iFH += 1)
				{
					if (line[iFH] == '')
					{
						break;
					}
					if ((line[iFH] == '\n'))
					{
						fixhelp[iFH2] = '\\';
						fixhelp[iFH2+1] = 'n';
						iFH2 += 2;
						continue;
					}
					if (line[iFH] == ';')
					{
						continue;
					}
					fixhelp[iFH2] = line[iFH];
					iFH2 += 1;
				}
				stringclear(line, 1022);
				Com_sprintf( line, sizeof(line), fixhelp);
				//trap_SendServerCmd( ent->s.number, va("print \"^1You should probably be banned for that.\n\""));
				//trap_FS_FCloseFile( f );
				//G_Printf("Refused.\n");
				//return;
			}
			if (strlen(line) > 1000)
			{
				trap_SendServerCmd( ent->s.number, va("print \"^1Save Data is too long.\n\""));
				trap_FS_FCloseFile( f );
				G_Printf("Refused.\n");
				return;
			}
			if (nomessage == 0)
			{
				trap_SendServerCommand( clientNum, va("print \"^2Saved entity ^5%i^2.\n\"", chosenent->s.number ) );
			}
			else
			{
				G_Printf("No message save ");
			}
			//strcpy(line,va("%s;wait 20;",line));
			if (savetype == 0)
			{
			strcpy(line,va("\n%s;\n", line));
			}
			trap_FS_Write( line, strlen(line), f);
			trap_FS_FCloseFile( f );
			G_Printf("Passed.\n");
			//trap_SendServerCmd( clientNum, va("print \"^2Saved entity ^5%i^2.\n\"", chosenentnum ) );
			}
		break;
		case 14:
		///////////////////
		// ammap_grabent
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(parcount < 3))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_grabent <entity> <distance> <yaw AAT> <pitch AAT> [zoff]\nAngle Adjust Types (AATs):\n0: Do not adjust.\n1:Match my angle.\n2:Face me.\n3: Starting relative value\nOther values > 3 - custom angle adjustment relative to me.\n\"" ) );
				return;
			}
			else if (ent->client->sess.grabbedent != 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^2Dropped entity ^5%i^2.\n\"", ent->client->sess.grabbedent ) );
				ent->client->sess.grabbedent = 0;
				ent->client->sess.grabbeddist = 0;
				ent->client->sess.grabbedgroup = 0;
				ent->client->sess.grabbedentoffz = 0;
				return;
			}
			else if (ent->client->sess.grabbedgroup != 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^2Dropped group ^5%i^2.\n\"", ent->client->sess.grabbedgroup ) );
				ent->client->sess.grabbedent = 0;
				ent->client->sess.grabbeddist = 0;
				ent->client->sess.grabbedgroup = 0;
				ent->client->sess.grabbedentoffz = 0;
				return;
			}
			else
			{
				gentity_t	*tEn;
				int		iPl;
				for (iPl = 0;iPl < 32;iPl += 1)
				{
					tEn = &g_entities[iPl];
					if (!tEn->inuse)
					{
						continue;
					}
					if (tEn->client->sess.grabbedent == chosenentnum)
					{
						trap_SendServerCmd( clientNum, va("print \"^1Another player is holding this ent.\n\"") );
						return;
					}
					if (tEn->client->sess.grabbedgroup != 0)
					{
						gentity_t	*t;
						t = NULL;
						i = 0;
						while ( (t = G_Findbygroup(t, tEn->client->sess.grabbedgroup)) != NULL )
						{
							if (t->s.number == chosenentnum)
							{
								trap_SendServerCmd( clientNum, va("print \"^1Another player is holding this ent.\n\"") );
								return;
							}
						}
					}
				}
				fixforundo(chosenent);
			chosenent->doorchanged = 32;
				ent->client->sess.grabbedent = chosenentnum;
				ent->client->sess.grabbeddist = atoi(par2);
				ent->client->sess.grabbedgroup = 0;
				if (atoi(par3) == 0)
				{
					ent->client->sess.grabbedentyaw = 0;
				}
				else if (atoi(par3) == 1)
				{
					ent->client->sess.grabbedentyaw = 1;
				}
				else if (atoi(par3) == 2)
				{
					ent->client->sess.grabbedentyaw = 180;
				}
				else if (atoi(par3) == 3)
				{
					ent->client->sess.grabbedentyaw = mcfix360(mcfix360(chosenent->r.currentAngles[YAW])-mcfix360(ent->client->ps.viewangles[YAW]));
				}
				else
				{
					ent->client->sess.grabbedentyaw = mcfix360(atoi(par3));
				}
				if (atoi(par4) == 0)
				{
					ent->client->sess.grabbedentpitch = 0;
				}
				else if (atoi(par4) == 1)
				{
					ent->client->sess.grabbedentpitch = 1;
				}
				else if (atoi(par4) == 2)
				{
					ent->client->sess.grabbedentpitch = 180;
				}
				else if (atoi(par4) == 3)
				{
					ent->client->sess.grabbedentpitch = mcfix360(mcfix360(chosenent->r.currentAngles[PITCH])-mcfix360(ent->client->ps.viewangles[PITCH]));
				}
				else
				{
					ent->client->sess.grabbedentpitch = mcfix360(atoi(par4));
				}
				ent->client->sess.grabbedentoffz = atoi(par5);
				trap_SendServerCmd( clientNum, va("print \"^2Grabbed entity ^5%i^2.\n\"", chosenentnum ) );
			}
		break;
		case 15:
		///////////////////
		// ammap_glow
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(parcount < 6))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_glow <entity> <Red> <Green> <Blue> <Intensity>\n\"" ) );
				return;
			}
			chosenent->s.constantLight = atoi(par2) + (atoi(par3)<<8) + (atoi(par4)<<16) + (atoi(par5)<<24);
			strcpy(chosenent->mcmlight,va("%i %i %i %i",atoi(par1), atoi(par2), atoi(par3), atoi(par4)));
		break;
		case 16:
		///////////////////
		// ammap_group
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(parcount < 3))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_group <entity> <group>\n\"" ) );
				return;
			}
			if (atoi(par2) == 0)
			{
				if (nomessage == 0)
				{
				trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 moved from group ^5%i^2 to group ^5%i^2, which had ^5%i^2 other ents.\n\"", chosenentnum, chosenent->entgroup, atoi(par2), i ) );
				}
				chosenent->entgroup = atoi(par2);
				chosenent->groupleader = 0;
			}
			else
			{
				gentity_t	*t;
				int		i;
				int		ilead;
				gentity_t	*tEn;
				int		iPl;
				t = NULL;
				i = 0;
				ilead = 0;
				for (iPl = 0;iPl < 32;iPl += 1)
				{
					tEn = &g_entities[iPl];
					if (!tEn->inuse)
					{
						continue;
					}
					if (tEn->client->sess.grabbedent == chosenentnum)
					{
						trap_SendServerCmd( clientNum, va("print \"^1A player is holding this ent.\n\"") );
						return;
					}
					if (tEn->client->sess.grabbedgroup != 0)
					{
						gentity_t	*t;
						t = NULL;
						i = 0;
						while ( (t = G_Findbygroup(t, tEn->client->sess.grabbedgroup)) != NULL )
						{
							if (t->s.number == chosenentnum)
							{
								trap_SendServerCmd( clientNum, va("print \"^1A player is holding this ent.\n\"") );
								return;
							}
						}
					}
				}
				t = NULL;
				i = 0;
				ilead = 0;
				while ( (t = G_Findbygroup(t, chosenent->entgroup)) != NULL )
				{
					i += 1;
					if (t->groupleader != 0)
					{
						ilead = t->groupleader;
					}
				}
				if (i == 0)
				{
					if (nomessage == 0)
					{
					trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 moved from group ^5%i^2 to group ^5%i^2 and is the default leader.\n\"", chosenentnum, chosenent->entgroup, atoi(par2) ) );
					}
					chosenent->entgroup = atoi(par2);
					chosenent->groupleader = chosenent->s.number;
				}
				else
				{
					if (nomessage == 0)
					{
					trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 moved from group ^5%i^2 to group ^5%i^2.\n\"", chosenentnum, chosenent->entgroup, atoi(par2) ) );
					}
					chosenent->entgroup = atoi(par2);
					chosenent->groupleader = ilead;
				}
			}
		break;
		case 17:
		///////////////////
		// ammap_groupleader
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(parcount < 2))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_groupleader <entity>\n\"" ) );
				return;
			}
			if (chosenent->entgroup == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1This entity is not in a group.\n\"" ));
				return;
			}
			else
			{
				gentity_t	*t;
				gentity_t	*tEn;
				int		iPl;
				t = NULL;
				i = 0;
				for (iPl = 0;iPl < 32;iPl += 1)
				{
					tEn = &g_entities[iPl];
					if (!tEn->inuse)
					{
						continue;
					}
					if (tEn->client->sess.grabbedent == chosenentnum)
					{
						trap_SendServerCmd( clientNum, va("print \"^1A player is holding this ent.\n\"") );
						return;
					}
					if (tEn->client->sess.grabbedgroup != 0)
					{
						gentity_t	*t;
						t = NULL;
						i = 0;
						while ( (t = G_Findbygroup(t, tEn->client->sess.grabbedgroup)) != NULL )
						{
							if (t->s.number == chosenentnum)
							{
								trap_SendServerCmd( clientNum, va("print \"^1A player is holding this ent.\n\"") );
								return;
							}
						}
					}
				}
				t = NULL;
				while ( (t = G_Findbygroup(t, chosenent->entgroup)) != NULL )
				{
					t->groupleader = chosenent->s.number;
				}
				if (nomessage == 0)
				{
				trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 is now the leader of group ^5%i^2.\n\"", chosenent->s.number, chosenent->entgroup));
				}
				return;
			}
		break;
		case 18:
		///////////////////
		// ammap_addangles
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_addangles <entity> <Yaw> <Pitch> <Roll>\n\"" ) );
				return;
			}
			if (parcount < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_addangles <entity> <Yaw> <Pitch> <Roll>\n\"" ) );
				return;
			}
			fixforundor(chosenent);
			chosenent->s.angles[YAW] = mc_fix360(chosenent->r.currentAngles[YAW]+atoi(par2));
			chosenent->s.angles[PITCH] = mc_fix360(chosenent->r.currentAngles[PITCH]+atoi(par3));
			chosenent->s.angles[ROLL] = mc_fix360(chosenent->r.currentAngles[ROLL]+atoi(par4));
			G_SetAngles(chosenent, chosenent->s.angles);
			trap_LinkEntity(chosenent);
		break;
		case 19:
		///////////////////
		// ammap_setorigin
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setorigin <entity> <x> <y> <z>\n\"" ) );
				return;
			}
			if (parcount < 5)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setorigin <entity> <x> <y> <z>\n\"" ) );
				return;
			}
			chosenent->s.origin[0] = (float)monkeyspot(par2, 0, ent, chosenent);//atoi(par2);
			chosenent->s.origin[1] = (float)monkeyspot(par3, 1, ent, chosenent);//atoi(par3);
			chosenent->s.origin[2] = (float)monkeyspot(par4, 2, ent, chosenent);//atoi(par4);
			chosenent->doorchanged = 32;
			fixforundo(chosenent);
			//chosenent->s.pos.trBase[0] = atoi(par2);
			//chosenent->s.pos.trBase[1] = atoi(par3);
			//chosenent->s.pos.trBase[2] = atoi(par4);
			G_SetOrigin(chosenent, chosenent->s.origin);
			trap_LinkEntity(chosenent);
		break;
		case 20:
		///////////////////
		// ammap_setanim
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setanim <entity> <anim>\n\"" ) );
				return;
			}
			if (parcount < 3)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_setanim <entity> <anim>\n\"" ) );
				return;
			}
			trap_UnlinkEntity(chosenent);
			chosenent->s.torsoAnim = atoi(par2);// & ~ANIM_TOGGLEBIT;
			chosenent->s.legsAnim = chosenent->s.torsoAnim;
			trap_LinkEntity(chosenent);
		break;
		case 21:
		///////////////////
		// ammap_undomove
		///////////////////
			if ((Q_stricmp(par1, "info") == 0)||(Q_stricmp(par1,"") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_undomove <ent>\n\"" ) );
				return;
			}
			VectorCopy(chosenent->lastorigin, chosenent->s.origin);
			VectorCopy(chosenent->r.currentOrigin, chosenent->lastorigin);
			G_SetOrigin(chosenent, chosenent->s.origin);
			chosenent->doorchanged = 32;
			trap_LinkEntity(chosenent);
		break;
		case 22:
		///////////////////
		// ammap_undorot
		///////////////////
			if ((Q_stricmp(par1, "info") == 0)||(Q_stricmp(par1,"") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_undorot <ent>\n\"" ) );
				return;
			}
			VectorCopy(chosenent->lastrot, chosenent->s.angles);
			VectorCopy(chosenent->r.currentAngles, chosenent->lastrot);
			G_SetAngles(chosenent, chosenent->s.angles);
			trap_LinkEntity(chosenent);
		break;
		case 23:
		///////////////////
		// ammap_collisiontype
		///////////////////
			if ((Q_stricmp(par1, "info") == 0)||(Q_stricmp(par2,"") == 0))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_collisiontype <ent> <type>\n\"" ) );
				return;
			}
			if (!strstr(par2,"and"))
			{
				chosenent->r.contents = 0;
			}
			if (strstr(par2,"water"))
			{
				chosenent->r.contents |= CONTENTS_WATER;
			}
			else if (strstr(par2,"lava"))
			{
				chosenent->r.contents |= CONTENTS_LAVA;
			}
			else if (strstr(par2,"slime"))
			{
				chosenent->r.contents |= CONTENTS_SLIME;
			}
			else if (strstr(par2,"solid"))
			{
				chosenent->r.contents |= CONTENTS_SOLID;
			}
			else if (strstr(par2,"corpse"))
			{
				chosenent->r.contents |= CONTENTS_CORPSE;
			}
			else if (strstr(par2,"body"))
			{
				chosenent->r.contents |= CONTENTS_CORPSE;
			}
			else if (strstr(par2,"playerclip"))
			{
				chosenent->r.contents |= CONTENTS_PLAYERCLIP;
			}
			else if (strstr(par2,"testw"))
			{
				chosenent->r.contents = MASK_WATER;
			}
			else
			{
				trap_SendServerCmd( clientNum, va("print \"^1Unknown collision type.\n\"" ) );
				return;
			}
			if (strstr(par2,"clippy"))
			{
				chosenent->clipmask = chosenent->r.contents;
			}
			//chosenent->clipmask = MASK_SOLID;
			trap_SendServerCmd( clientNum, va("print \"^2Command accepted.\n\"" ) );
			trap_LinkEntity(chosenent);
		break;
		case 24:
		///////////////////
		// ammap_delent2
		///////////////////
			if (( Q_stricmp(par1, "info") == 0)||(parcount < 2))
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_delent2 <entity>\n\"" ) );
				return;
			}
			else
			{
			int	iL;
			trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
			if (chosenent->custom == 1)
		{
			if (chosenent->issaved == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1Chosen entity is not saved.\n\"" ) );
				return;
			}
			else
			{
				char	bigbuffer[12000];
				int	fpp;
				memset(line, 0, sizeof(line));


			if (Q_stricmp(chosenent->classname,"mc_model") == 0)
			{
				Com_sprintf( line, sizeof(line), "addmodelrcon3 \"%s\" %i %i %i %i %i %i %i %i %i %i %i %i", chosenent->mcmessage,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW],
				 (int)chosenent->r.currentAngles[PITCH],
				 (int)chosenent->r.currentAngles[ROLL],
				 (int)chosenent->r.mins[0],
				 (int)chosenent->r.mins[1],
				 (int)chosenent->r.mins[2],
				 (int)chosenent->r.maxs[0],
				 (int)chosenent->r.maxs[1],
				 (int)chosenent->r.maxs[2]);
			}
			else if (Q_stricmp(chosenent->classname,"mc_light") == 0)
			{
				Com_sprintf(line,sizeof(line), "addlightrcon %i %i %i %s", (int)chosenent->s.origin[0],
				(int)chosenent->s.origin[1],
				(int)chosenent->s.origin[2],
				chosenent->mcmlight);
				
			}
			else if (Q_stricmp(chosenent->classname,"mc_model2") == 0)
			{
				Com_sprintf( line, sizeof(line), "addghoulrcon \"%s\" %i %i %i %i %i %i %i %i %i %i %i %i", chosenent->mcmessage,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW],
				 (int)chosenent->r.currentAngles[PITCH],
				 (int)chosenent->r.currentAngles[ROLL],
				 (int)chosenent->r.mins[0],
				 (int)chosenent->r.mins[1],
				 (int)chosenent->r.mins[2],
				 (int)chosenent->r.maxs[0],
				 (int)chosenent->r.maxs[1],
				 (int)chosenent->r.maxs[2]);
			}
			else if (Q_stricmp(chosenent->classname,"mc_effect") == 0)
			{
				Com_sprintf( line, sizeof(line), "addeffectrcon \"%s\" %i %i %i %i %i %i %i", chosenent->mcmessage,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW],
				 (int)chosenent->r.currentAngles[PITCH],
				 (int)chosenent->r.currentAngles[ROLL],
				 (int)chosenent->delay);
			}
		
			else if (Q_stricmp(chosenent->classname,"mc_note") == 0)
			{
				Com_sprintf( line, sizeof(line), "addnotercon \"%s\" \"%s\" %i %i %i %i", chosenent->mctargetname,
				chosenent->mcmessage,
				(int)chosenent->s.bolt1,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2]);
			}
		
			else if (Q_stricmp(chosenent->classname,"mc_tsent") == 0)
			{
				Com_sprintf( line, sizeof(line), "addanyent mc_tsent %i %i %i \"group,%s,",
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 chosenent->group);
				if (chosenent->bolt_Waist == 1)
				{
					Com_sprintf( line, sizeof(line), "%sreaction,kick,\"", line);
				}
				else
				{
					Com_sprintf( line, sizeof(line), "%sreaction,kill,\"", line);
				}
			}
			else if (Q_stricmp(chosenent->classname,"target_speaker") == 0)
			{
				Com_sprintf( line, sizeof(line), "addanyent \"target_speaker\" %i %i %i \"targetname,%s,noise,%s,spawnflags,%i,wait,%i,random,%i,\"",
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 chosenent->targetname,
				 chosenent->upmes,
				 (int)chosenent->spawnflags,
				 (int)chosenent->wait,
				 (int)chosenent->random);
			}
			else if (Q_stricmp(chosenent->classname,"mcsentry") == 0)
			{
				Com_sprintf( line, sizeof(line), "addmcsentry \"%i\" \"%i\" \"%i\" \"%i\"", chosenent->s.owner, 
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2]);
			}
			else if (Q_stricmp(chosenent->classname,"mcshield") == 0)
			{
				Com_sprintf( line, sizeof(line), "addmcshield \"%i\" \"%i\" \"%i\" \"%i\" \"%i\"", chosenent->s.owner, 
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[YAW]);
			}
			else
			{
				Com_sprintf( line, sizeof(line), "addanyent \"%s\" %i %i %i \"angles,%i %i %i,spawnflags,%i,mins,%i %i %i,maxs,%i %i %i,", chosenent->classname,
				(int)chosenent->r.currentOrigin[0],
				 (int)chosenent->r.currentOrigin[1],
				 (int)chosenent->r.currentOrigin[2],
				 (int)chosenent->r.currentAngles[0],
				 (int)chosenent->r.currentAngles[1],
				 (int)chosenent->r.currentAngles[2],
				 (int)chosenent->spawnflags,
				 (int)chosenent->r.mins[0],
				 (int)chosenent->r.mins[1],
				 (int)chosenent->r.mins[2],
				 (int)chosenent->r.maxs[0],
				 (int)chosenent->r.maxs[1],
				 (int)chosenent->r.maxs[2]);
				if (chosenent->movedir[0] != 0 || chosenent->movedir[1] != 0 || chosenent->movedir[2] != 0)
				{
					vec3_t final;
					VectorClear(final);
					vectoangles(chosenent->movedir, final);
					Com_sprintf( line, sizeof(line), "%sangles,%i %i %i,", line,
					 (int)final[0], (int)final[1], (int)final[2]);
				}
				if (chosenent->target && Q_stricmp(chosenent->target,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%starget,%s,", line,
					 chosenent->target);
				}
				if (chosenent->targetname && Q_stricmp(chosenent->targetname,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%stargetname,%s,", line,
					 chosenent->targetname);
				}
				if (chosenent->targetShaderName && Q_stricmp(chosenent->targetShaderName,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%stargetShaderName,%s,", line,
					 chosenent->targetShaderName);
				}
				if (chosenent->targetShaderNewName && Q_stricmp(chosenent->targetShaderNewName,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%stargetShaderNewName,%s,", line,
					 chosenent->targetShaderNewName);
				}
				if (chosenent->message && Q_stricmp(chosenent->message,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%smessage,%s,", line,
					 chosenent->message);
				}
				if (chosenent->team && Q_stricmp(chosenent->team,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%steam,%s,", line,
					 chosenent->team);
				}
				if (chosenent->upmes && Q_stricmp(chosenent->upmes,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%supmessage,%s,", line,
					 chosenent->upmes);
				}
				if (chosenent->downmes && Q_stricmp(chosenent->downmes,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%sdownmessage,%s,", line,
					 chosenent->downmes);
				}
				if (chosenent->mcpassword && Q_stricmp(chosenent->mcpassword,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%spassword,%s,", line,
					 chosenent->mcpassword);
				}
				if (chosenent->group && Q_stricmp(chosenent->group,"") != 0)
				{
					Com_sprintf( line, sizeof(line), "%sgroup,%s,", line,
					 chosenent->group);
				}
				if (chosenent->mcmessage && Q_stricmp(chosenent->mcmessage,"") != 0)
				{
					if (Q_stricmp(chosenent->classname,"fx_runner") == 0)
					{
						Com_sprintf( line, sizeof(line), "%sfxfile,%s,", line,
						 chosenent->mcmessage);
					}
					else
					{
						Com_sprintf( line, sizeof(line), "%smodel,%s,", line,
						 chosenent->mcmessage);
					}
				}
				if (chosenent->speed != 0)
				{
					Com_sprintf( line, sizeof(line), "%sspeed,%i,", line,
					 (int)chosenent->speed);
				}
				if (chosenent->wait != 0)
				{
					Com_sprintf( line, sizeof(line), "%swait,%i,", line,
					 (int)chosenent->wait);
				}
				if (chosenent->random != 0)
				{
					Com_sprintf( line, sizeof(line), "%srandom,%i,", line,
					 (int)chosenent->random);
				}
				if (chosenent->lip != 0)
				{
					Com_sprintf( line, sizeof(line), "%slip,%i,", line,
					 (int)chosenent->lip);
				}
				if (chosenent->count != 0)
				{
					Com_sprintf( line, sizeof(line), "%scount,%i,", line,
					 (int)chosenent->count);
				}
				if (chosenent->health != 0)
				{
					Com_sprintf( line, sizeof(line), "%shealth,%i,", line,
					 (int)chosenent->health);
				}
				if (chosenent->delay != 0)
				{
					Com_sprintf( line, sizeof(line), "%sdelay,%i,", line,
					 (int)chosenent->delay);
				}
				if (chosenent->damage != 0)
				{
					Com_sprintf( line, sizeof(line), "%sdmg,%i,", line,
					 (int)chosenent->damage);
				}
				if (chosenent->s.owner != 0)
				{
					Com_sprintf( line, sizeof(line), "%sowner,%i,", line,
					 (int)chosenent->s.owner);
				}
				if (chosenent->xOff != 0)
				{
					Com_sprintf( line, sizeof(line), "%smodelangle,%i,", line,
					 (int)chosenent->xOff);
				}
				if (chosenent->yOff != 0)
				{
					Com_sprintf( line, sizeof(line), "%smodelpitch,%i,", line,
					 (int)chosenent->yOff);
				}
				if (chosenent->zOff != 0)
				{
					Com_sprintf( line, sizeof(line), "%smodelroll,%i,", line,
					 (int)chosenent->zOff);
				}
				Com_sprintf( line, sizeof(line), "%s\" \"\"", line);
			}


				G_Printf("Trying to delete >>%s<<\n", line);
				for (i = 1;i < 32;i += 1)
				{
					memset(bigbuffer, 0, sizeof(bigbuffer));
					iL = trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_READ);
					if (!f)
					{
						G_Printf("Gave up!\n");
						trap_SendServerCmd( clientNum, va("print \"^1Chosen entity cannot be found in the edit file.\n\"" ) );
						return;
					}
					else
					{
						trap_FS_Read( bigbuffer, 11999, f );
						trap_FS_FCloseFile(f);
						if (!strstr(bigbuffer, line))
						{
							G_Printf("File %i did not contain...\n", i);
							continue;
						}
						trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_WRITE);
						if ( !f )
						{
							trap_SendServerCmd( clientNum, va("print \"^1Error: Cannot access map file.\n\"" ) );
							G_Printf("Found, could not write!\n");
							return;
						}
						iL = strlen(bigbuffer);
						for (i = strlen(line);i < iL;i++)
						{
							for (fpp = 0;fpp < strlen(line);fpp++)
							{
								if (!(bigbuffer[(i - strlen(line)) + fpp] == line[fpp]))
								{
									break;
								}
							}
							if (fpp == strlen(line))
							{
								bigbuffer[(i - strlen(line))] = 'D';
								bigbuffer[(i - strlen(line)) + 1] = 'E';
								bigbuffer[(i - strlen(line)) + 2] = 'L';
								break;
							}
						}
						trap_FS_Write( bigbuffer, strlen(bigbuffer), f);
						trap_FS_FCloseFile( f );
						G_Printf("Victory!\n");
						trap_SendServerCmd( clientNum, va("print \"^1Chosen entity permanently deleted.\n\"" ) );
						G_FreeEntity(chosenent);
						return;
					}
				}
				G_Printf("None contained... giving up!\n");
				trap_SendServerCmd( clientNum, va("print \"^1Chosen entity cannot be found in the edit file.\n\"" ) );
				return;
			}
		}
			else
		{
			for (i = 1;i < 32;i += 1)
			{
				iL = trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string), &f, FS_READ);
				if (!f)
				{
					trap_FS_FOpenFile(va("%s/mapedits_%i_%s.cfg", mc_editfolder.string, i-1, mapname.string), &f, FS_APPEND);
					if ( !f )
					{
						trap_SendServerCmd( clientNum, va("print \"^1Error: Cannot access map file.\n\"" ) );
						return;
					}
					trap_FS_Write( va("\nmapeditsexec mapedits_%i_%s;\n", i, mapname.string), strlen(va("\nmapeditsexec mapedits_%i_%s;\n", i, mapname.string)), f);
					trap_FS_FCloseFile( f );
					goto loadmap2;
				}
				trap_FS_FCloseFile(f);
				if (iL < 10000)
				{
					goto loadmap2;
				}
			}
			loadmap2:
			Com_sprintf(savePath, 1024*4, "%s/mapedits_%i_%s.cfg", mc_editfolder.string, i, mapname.string);
			trap_FS_FOpenFile(savePath, &f, FS_APPEND);
			if ( !f )
			{
				trap_SendServerCmd( clientNum, va("print \"^1Error: Cannot access map file.\n\"" ) );
				return;
			}
			Com_sprintf(line,sizeof(line), "mcmap_delent %i", (int)chosenent->s.number);
			if( strchr( line, ';' ) ) {
				trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
				trap_FS_FCloseFile( f );
				return;
			}
			if( strchr( line, '\n' ) ) {
				trap_SendServerCmd( clientNum, va("print \"^1You should probably be banned for that.\n\""));
				trap_FS_FCloseFile( f );
				return;
			}
			if (strlen(line) > 1000)
			{
				trap_SendServerCmd( clientNum, va("print \"^1Save Data is too long.\n\""));
				trap_FS_FCloseFile( f );
				return;
			}
			if (nomessage == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^2Permanently deleted entity ^5%i^2.\n\"", chosenentnum ) );
			}
			//strcpy(line,va("%s;wait 20;",line));
			strcpy(line,va("\n%s;\n", line));
			trap_FS_Write( line, strlen(line), f);
			trap_FS_FCloseFile( f );
			G_FreeEntity(chosenent);
			//trap_SendServerCmd( clientNum, va("print \"^2Permanently deleted entity ^5%i^2.\n\"", chosenentnum ) );
			}
		}
		break;
		case 25:
		///////////////////
		// ammap_delent_matching
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_delent_matching <entity> <string to match>\n\"" ) );
				return;
			}
			if (parcount < 3)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_delent_matching <entity> <string to match>\n\"" ) );
				return;
			}
			Com_sprintf( buffer, sizeof(buffer), "Trace ent: %i - %s\n", chosenentnum, chosenent->classname);
			Com_sprintf( buffer, sizeof(buffer), "%s^7X=^5%i^7, Y=^5%i^7, Z=^5%i^7\n", buffer, (int)chosenent->r.currentOrigin[0], (int)chosenent->r.currentOrigin[1], (int)chosenent->r.currentOrigin[2]);
			ent->client->sess.traced = chosenent->s.number;
			if (chosenent->s.angles[YAW] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(YAW = %i) ", buffer, (int)chosenent->s.angles[YAW]);
			}
			if (chosenent->s.angles[PITCH] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(PITCH = %i) ", buffer, (int)chosenent->s.angles[PITCH]);
			}
			if (chosenent->s.angles[ROLL] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(ROLL = %i) ", buffer, (int)chosenent->s.angles[ROLL]);
			}
			if (chosenent->count)
			{
				if (chosenent->count != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(count = %i) ", buffer, (int)chosenent->count);
				}
			}
			if (chosenent->health)
			{
				if (chosenent->health != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(health = %i) ", buffer, (int)chosenent->health);
				}
			}
			if (chosenent->wait)
			{
				if (chosenent->wait != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(wait = %i) ", buffer, (int)chosenent->wait);
				}
			}
			if (chosenent->spawnflags)
			{
				if (chosenent->spawnflags != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(spawnflags = %i) ", buffer, (int)chosenent->spawnflags);
				}
			}
			if (chosenent->damage)
			{
				if (chosenent->damage != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(damage = %i) ", buffer, (int)chosenent->damage);
				}
			}
			if (chosenent->speed)
			{
				if (chosenent->speed != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(speed = %i) ", buffer, (int)chosenent->speed);
				}
			}
			if (chosenent->random)
			{
				if (chosenent->random != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(random = %i) ", buffer, (int)chosenent->random);
				}
			}
			if (chosenent->target)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(target = %s) ", buffer, chosenent->target);
			}
			if (chosenent->targetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(targetname = %s) ", buffer, chosenent->targetname);
			}
			/*
			if (chosenent->mctargetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(other = %s) ", buffer, chosenent->mctargetname);
			}
			*/
			if (chosenent->model)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->model);
			}
			if (Q_stricmp(chosenent->mcmessage,"")!=0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->mcmessage);
			}
			if (chosenent->delay)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(delay = %i) ", buffer, chosenent->delay);
			}
			if (chosenent->entgroup)
			{
				if (chosenent->entgroup != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(group = %i) ", buffer, (int)chosenent->entgroup);
					Com_sprintf( buffer, sizeof(buffer), "%s(groupleader = %i) ", buffer, (int)chosenent->groupleader);
				}
			}
			Com_sprintf( buffer, sizeof(buffer), "%s(mins = %i %i %i) ", buffer, (int)chosenent->r.mins[0], (int)chosenent->r.mins[1], (int)chosenent->r.mins[2]);
			Com_sprintf( buffer, sizeof(buffer), "%s(maxes = %i %i %i) ", buffer, (int)chosenent->r.maxs[0], (int)chosenent->r.maxs[1], (int)chosenent->r.maxs[2]);
			//trap_SendServerCmd( clientNum, va("print \"^7%s\n\"", buffer ) );
			if (strstr(buffer, par2))
			{
				if (nomessage == 0)
				{
					trap_SendServerCmd( clientNum, va("print \"^3Deleted ^5%i^3(^5%s^3).\n\"", chosenentnum, chosenent->classname ) );
				}
				G_FreeEntity(chosenent);
			}

		break;
		case 26:
		///////////////////
		// ammap_group_matching
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_group_matching <entity> <group> <string to match>\n\"" ) );
				return;
			}
			if (parcount < 4)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_group_matching <entity> <group> <string to match>\n\"" ) );
				return;
			}
			Com_sprintf( buffer, sizeof(buffer), "Trace ent: %i - %s\n", chosenentnum, chosenent->classname);
			Com_sprintf( buffer, sizeof(buffer), "%s^7X=^5%i^7, Y=^5%i^7, Z=^5%i^7\n", buffer, (int)chosenent->r.currentOrigin[0], (int)chosenent->r.currentOrigin[1], (int)chosenent->r.currentOrigin[2]);
			ent->client->sess.traced = chosenent->s.number;
			if (chosenent->s.angles[YAW] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(YAW = %i) ", buffer, (int)chosenent->s.angles[YAW]);
			}
			if (chosenent->s.angles[PITCH] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(PITCH = %i) ", buffer, (int)chosenent->s.angles[PITCH]);
			}
			if (chosenent->s.angles[ROLL] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(ROLL = %i) ", buffer, (int)chosenent->s.angles[ROLL]);
			}
			if (chosenent->count)
			{
				if (chosenent->count != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(count = %i) ", buffer, (int)chosenent->count);
				}
			}
			if (chosenent->health)
			{
				if (chosenent->health != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(health = %i) ", buffer, (int)chosenent->health);
				}
			}
			if (chosenent->wait)
			{
				if (chosenent->wait != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(wait = %i) ", buffer, (int)chosenent->wait);
				}
			}
			if (chosenent->spawnflags)
			{
				if (chosenent->spawnflags != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(spawnflags = %i) ", buffer, (int)chosenent->spawnflags);
				}
			}
			if (chosenent->damage)
			{
				if (chosenent->damage != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(damage = %i) ", buffer, (int)chosenent->damage);
				}
			}
			if (chosenent->speed)
			{
				if (chosenent->speed != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(speed = %i) ", buffer, (int)chosenent->speed);
				}
			}
			if (chosenent->random)
			{
				if (chosenent->random != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(random = %i) ", buffer, (int)chosenent->random);
				}
			}
			if (chosenent->target)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(target = %s) ", buffer, chosenent->target);
			}
			if (chosenent->targetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(targetname = %s) ", buffer, chosenent->targetname);
			}
			/*
			if (chosenent->mctargetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(other = %s) ", buffer, chosenent->mctargetname);
			}
			*/
			if (chosenent->model)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->model);
			}
			if (Q_stricmp(chosenent->mcmessage,"")!=0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->mcmessage);
			}
			if (chosenent->delay)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(delay = %i) ", buffer, chosenent->delay);
			}
			if (chosenent->entgroup)
			{
				if (chosenent->entgroup != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(group = %i) ", buffer, (int)chosenent->entgroup);
					Com_sprintf( buffer, sizeof(buffer), "%s(groupleader = %i) ", buffer, (int)chosenent->groupleader);
				}
			}
			Com_sprintf( buffer, sizeof(buffer), "%s(mins = %i %i %i) ", buffer, (int)chosenent->r.mins[0], (int)chosenent->r.mins[1], (int)chosenent->r.mins[2]);
			Com_sprintf( buffer, sizeof(buffer), "%s(maxes = %i %i %i) ", buffer, (int)chosenent->r.maxs[0], (int)chosenent->r.maxs[1], (int)chosenent->r.maxs[2]);
			//trap_SendServerCmd( clientNum, va("print \"^7%s\n\"", buffer ) );
			if (strstr(buffer, par3))
			{
			if (atoi(par2) == 0)
			{
				if (nomessage == 0)
				{
				trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 moved from group ^5%i^2 to group ^5%i^2, which had ^5%i^2 other ents.\n\"", chosenentnum, chosenent->entgroup, atoi(par2), i ) );
				}
				chosenent->entgroup = atoi(par2);
				chosenent->groupleader = 0;
			}
			else
			{
				gentity_t	*t;
				int		i;
				int		ilead;
				gentity_t	*tEn;
				int		iPl;
				t = NULL;
				i = 0;
				ilead = 0;
				for (iPl = 0;iPl < 32;iPl += 1)
				{
					tEn = &g_entities[iPl];
					if (!tEn->inuse)
					{
						continue;
					}
					if (tEn->client->sess.grabbedent == chosenentnum)
					{
						trap_SendServerCmd( clientNum, va("print \"^1A player is holding this ent.\n\"") );
						return;
					}
					if (tEn->client->sess.grabbedgroup != 0)
					{
						gentity_t	*t;
						t = NULL;
						i = 0;
						while ( (t = G_Findbygroup(t, tEn->client->sess.grabbedgroup)) != NULL )
						{
							if (t->s.number == chosenentnum)
							{
								trap_SendServerCmd( clientNum, va("print \"^1A player is holding this ent.\n\"") );
								return;
							}
						}
					}
				}
				t = NULL;
				i = 0;
				ilead = 0;
				while ( (t = G_Findbygroup(t, chosenent->entgroup)) != NULL )
				{
					i += 1;
					if (t->groupleader != 0)
					{
						ilead = t->groupleader;
					}
				}
				if (i == 0)
				{
					if (nomessage == 0)
					{
					trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 moved from group ^5%i^2 to group ^5%i^2 and is the default leader.\n\"", chosenentnum, chosenent->entgroup, atoi(par2) ) );
					}
					chosenent->entgroup = atoi(par2);
					chosenent->groupleader = chosenent->s.number;
				}
				else
				{
					if (nomessage == 0)
					{
					trap_SendServerCmd( clientNum, va("print \"^2Entity ^5%i^2 moved from group ^5%i^2 to group ^5%i^2.\n\"", chosenentnum, chosenent->entgroup, atoi(par2) ) );
					}
					chosenent->entgroup = atoi(par2);
					chosenent->groupleader = ilead;
				}
			}
			}

		break;
		case 27:
		///////////////////
		// ammap_search
		///////////////////
			if ( Q_stricmp(par1, "info") == 0)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_search <entities> <string to match>\n\"" ) );
				return;
			}
			if (parcount < 3)
			{
				trap_SendServerCmd( clientNum, va("print \"^1/ammap_search <entities> <string to match>\n\"" ) );
				return;
			}
			Com_sprintf( buffer, sizeof(buffer), "Trace ent: %i - %s\n", chosenentnum, chosenent->classname);
			Com_sprintf( buffer, sizeof(buffer), "%s^7X=^5%i^7, Y=^5%i^7, Z=^5%i^7\n", buffer, (int)chosenent->r.currentOrigin[0], (int)chosenent->r.currentOrigin[1], (int)chosenent->r.currentOrigin[2]);
			ent->client->sess.traced = chosenent->s.number;
			if (chosenent->s.angles[YAW] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(YAW = %i) ", buffer, (int)chosenent->s.angles[YAW]);
			}
			if (chosenent->s.angles[PITCH] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(PITCH = %i) ", buffer, (int)chosenent->s.angles[PITCH]);
			}
			if (chosenent->s.angles[ROLL] != 0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(ROLL = %i) ", buffer, (int)chosenent->s.angles[ROLL]);
			}
			if (chosenent->count)
			{
				if (chosenent->count != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(count = %i) ", buffer, (int)chosenent->count);
				}
			}
			if (chosenent->health)
			{
				if (chosenent->health != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(health = %i) ", buffer, (int)chosenent->health);
				}
			}
			if (chosenent->wait)
			{
				if (chosenent->wait != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(wait = %i) ", buffer, (int)chosenent->wait);
				}
			}
			if (chosenent->spawnflags)
			{
				if (chosenent->spawnflags != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(spawnflags = %i) ", buffer, (int)chosenent->spawnflags);
				}
			}
			if (chosenent->damage)
			{
				if (chosenent->damage != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(damage = %i) ", buffer, (int)chosenent->damage);
				}
			}
			if (chosenent->speed)
			{
				if (chosenent->speed != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(speed = %i) ", buffer, (int)chosenent->speed);
				}
			}
			if (chosenent->random)
			{
				if (chosenent->random != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(random = %i) ", buffer, (int)chosenent->random);
				}
			}
			if (chosenent->target)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(target = %s) ", buffer, chosenent->target);
			}
			if (chosenent->targetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(targetname = %s) ", buffer, chosenent->targetname);
			}
			/*
			if (chosenent->mctargetname)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(other = %s) ", buffer, chosenent->mctargetname);
			}
			*/
			if (chosenent->model)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->model);
			}
			if (Q_stricmp(chosenent->mcmessage,"")!=0)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(model = %s) ", buffer, chosenent->mcmessage);
			}
			if (chosenent->delay)
			{
				Com_sprintf( buffer, sizeof(buffer), "%s(delay = %i) ", buffer, chosenent->delay);
			}
			if (chosenent->entgroup)
			{
				if (chosenent->entgroup != 0)
				{
					Com_sprintf( buffer, sizeof(buffer), "%s(group = %i) ", buffer, (int)chosenent->entgroup);
					Com_sprintf( buffer, sizeof(buffer), "%s(groupleader = %i) ", buffer, (int)chosenent->groupleader);
				}
			}
			Com_sprintf( buffer, sizeof(buffer), "%s(mins = %i %i %i) ", buffer, (int)chosenent->r.mins[0], (int)chosenent->r.mins[1], (int)chosenent->r.mins[2]);
			Com_sprintf( buffer, sizeof(buffer), "%s(maxes = %i %i %i) ", buffer, (int)chosenent->r.maxs[0], (int)chosenent->r.maxs[1], (int)chosenent->r.maxs[2]);
			//trap_SendServerCmd( clientNum, va("print \"^7%s\n\"", buffer ) );
			if (strstr(buffer, par2))
			{
				trap_SendServerCmd(clientNum, va("print \"^7%s\n\"", buffer));
			}

		break;

	//ent->r.contents = CONTENTS_SOLID;
	//ent->clipmask = MASK_SOLID;
	}
}





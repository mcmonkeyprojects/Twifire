// Copyright (C) 1999-2000 Id Software, Inc.
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"

int twimod_admincmds(gentity_t *ent, int clientNum, int cmd);
int twimod_adminnotarget(gentity_t *ent, int clientNum, int cmd);
/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;


typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;
char *needsS(int num)
{
	if (num == 1)
	{
		return "";
	}
	else
	{
		return "s";
	}
}
char *needsS2(int num)
{
	if (num == 1)
	{
		return "y";
	}
	else
	{
		return "ies";
	}
}
/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4] = {'\0','\0','\0','\0'};
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
void AddIP( char *str, qboolean saveToFile )
{
	int		i;

	/*if ( Q_stricmp (str, "0.0.0.0") == 0)
	{
		G_Printf (va("Attempt to ban global range!\n"));
		return;
	}*/

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i])) {
		ipFilters[i].compare = 0xffffffffu;
	} else {
		if (saveToFile) G_Printf (va("IP %s added.\n", str));
	}

	if (saveToFile) UpdateIPBans();
}

void dspAddIP( char *str, qboolean saveToFile )
{
	int		i;

	if ( Q_stricmp (str, "0.0.0.0") == 0)
	{
		G_Printf ("This in an invalid IP, this is the same as rangebanning every player!\n");
		return;
	}

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i])) {
		ipFilters[i].compare = 0xffffffffu;
	} else {
		if (saveToFile) G_Printf (va("IP %s added.\n", str));
	}

	if (saveToFile) UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	
	fileHandle_t f;
	char buffer[MAX_ARENAS_TEXT];
	int length, ipcount = 0;
	
	length = trap_FS_FOpenFile( "banlist.txt", &f, FS_READ );
	if ( !length || length < 0) {
		return;
	} else {
		buffer[length] = 0;

		if (length >= MAX_ARENAS_TEXT) {
			G_Printf (va("The banlist is full!\n", MAX_ARENAS_TEXT));
			trap_FS_FCloseFile( f );
			return;
		}

		trap_FS_Read(buffer,length,f);
		trap_FS_FCloseFile( f );

		for (t = s = buffer; *t; /* */ ) {
			s = strchr(s, ' ');
			if (!s)
				break;
			while (*s == ' ')
				*s++ = 0;
			if (*t) {
				AddIP( t, qfalse );
				ipcount++;
			}
			t = s;
		}
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str, qtrue );

}
void Svcmd_mcStatus_f(void)
{
	gentity_t	*t;
	int		i;
	for (i = 0;i <31;i+=1)
	{
		t = &g_entities[i];
		if (t && t->inuse && t->client)
		{
			char	userinfo[MAX_INFO_STRING];
			trap_GetUserinfo( i, userinfo, sizeof( userinfo ) );
			G_Printf("-%i) (name %s)(model %s)(ping %i)(score %i)(ip %i.%i.%i.%i)(health %i-%i)(origin %i,%i,%i)", i, t->client->pers.netname, Info_ValueForKey (userinfo, "model"), t->client->ps.ping, t->client->ps.persistant[PERS_SCORE], t->client->sess.IP0, t->client->sess.IP1, t->client->sess.IP2, t->client->sess.IP3, t->health, t->client->ps.stats[STAT_ARMOR], (int)t->client->ps.origin[0], (int)t->client->ps.origin[1], (int)t->client->ps.origin[2]);
			if (t->client->sess.adminloggedin != 0)
			{
				G_Printf("(adminrank %i)", t->client->sess.adminloggedin);
			}
			if (Q_stricmp(t->client->sess.userlogged,"") != 0)
			{
				G_Printf("(username %s)(credits %i)",t->client->sess.userlogged, t->client->sess.credits);
			}
			G_Printf("\n");
		}
	}
}
extern void AddSpawnField(char *field, char *value);
extern void SP_fx_runner( gentity_t *ent );
void	Svcmd_AddEffectlol( void ) {
gentity_t *fx_runner = G_Spawn();
char   effect[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char angle1[MAX_STRING_CHARS];
char angle2[MAX_STRING_CHARS];
char angle3[MAX_STRING_CHARS];
char delay[MAX_STRING_CHARS];

trap_Argv( 1, effect, sizeof( effect ) );
AddSpawnField("fxFile", effect);
trap_Argv( 2, origin_number, 1024 );
fx_runner->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
fx_runner->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
trap_Argv( 5, angle1, 1024 );
trap_Argv( 6, angle2, 1024 );
trap_Argv( 7, angle3, 1024 );
trap_Argv( 8, delay, 1024 );
fx_runner->s.angles[YAW] = atoi(angle1);
fx_runner->s.angles[PITCH] = atoi(angle2);
fx_runner->s.angles[ROLL] = atoi(angle3);
G_SetAngles(fx_runner,fx_runner->s.angles);
fx_runner->classname = "fx_runner";
fx_runner->s.origin[2] = atoi(origin_number3);

SP_fx_runner(fx_runner);
fx_runner->delay = atoi(delay);
}

extern void AddSpawnField(char *field, char *value);
extern void SP_jakes_model( gentity_t *ent );
extern void SP_jakes_model_zocken( gentity_t *ent );
void	Svcmd_AddModellol( void ) {
gentity_t *jakes_model = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
char pitch_number[MAX_STRING_CHARS];
char roll_number[MAX_STRING_CHARS];

if ( trap_Argc() < 6 ) {
	G_Printf("Usage: /addmodel (model) (x) (y) (z) (yaw)");
		return;
}

trap_Argv( 1, model, sizeof( model ) );
AddSpawnField("model", model);
trap_Argv( 2, origin_number, 1024 );
jakes_model->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
jakes_model->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
jakes_model->s.origin[2] = atoi(origin_number3) - 25;
trap_Argv( 5, yaw_number, 1024 );
trap_Argv( 6, pitch_number, 1024 );
trap_Argv( 7, roll_number, 1024 );
jakes_model->s.angles[YAW] = atoi(yaw_number);
jakes_model->s.angles[PITCH] = atoi(pitch_number);
jakes_model->s.angles[ROLL] = atoi(roll_number);
jakes_model->classname = "jmodel";
SP_jakes_model(jakes_model);
G_Printf("Added model: %s at <%s %s %s %s>", model, origin_number, origin_number2, origin_number3, yaw_number);
}

void	Svcmd_AddModellolnew( void ) {
gentity_t *jakes_model = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
char pitch_number[MAX_STRING_CHARS];
char roll_number[MAX_STRING_CHARS];

if ( trap_Argc() < 6 ) {
	G_Printf("Usage: /addmodel (model) (x) (y) (z) (yaw)");
		return;
}

trap_Argv( 1, model, sizeof( model ) );
AddSpawnField("model", model);
trap_Argv( 2, origin_number, 1024 );
jakes_model->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
jakes_model->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
jakes_model->s.origin[2] = atoi(origin_number3);
trap_Argv( 5, yaw_number, 1024 );
trap_Argv( 6, pitch_number, 1024 );
trap_Argv( 7, roll_number, 1024 );
jakes_model->s.angles[YAW] = atoi(yaw_number);
jakes_model->s.angles[PITCH] = atoi(pitch_number);
jakes_model->s.angles[ROLL] = atoi(roll_number);
jakes_model->classname = "jmodel";
SP_jakes_model(jakes_model);
G_Printf("Added model: %s at <%s %s %s %s>", model, origin_number, origin_number2, origin_number3, yaw_number);
}









void	Svcmd_AddModellolnewmcm( void ) {
gentity_t *jakes_model = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
char pitch_number[MAX_STRING_CHARS];
char roll_number[MAX_STRING_CHARS];
char xmin[MAX_STRING_CHARS];
char ymin[MAX_STRING_CHARS];
char zmin[MAX_STRING_CHARS];
char xmax[MAX_STRING_CHARS];
char ymax[MAX_STRING_CHARS];
char zmax[MAX_STRING_CHARS];

if ( trap_Argc() < 6 ) {
	G_Printf("Usage: /addmodel (model) (x) (y) (z) (yaw)");
		return;
}

trap_Argv( 1, model, sizeof( model ) );
AddSpawnField("model", model);
trap_Argv( 2, origin_number, 1024 );
jakes_model->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
jakes_model->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
jakes_model->s.origin[2] = atoi(origin_number3);
trap_Argv( 5, yaw_number, 1024 );
trap_Argv( 6, pitch_number, 1024 );
trap_Argv( 7, roll_number, 1024 );
trap_Argv( 8, xmin, 1024 );
trap_Argv( 9, ymin, 1024 );
trap_Argv( 10, zmin, 1024 );
trap_Argv( 11, xmax, 1024 );
trap_Argv( 12, ymax, 1024 );
trap_Argv( 13, zmax, 1024 );
jakes_model->s.angles[YAW] = atoi(yaw_number);
jakes_model->s.angles[PITCH] = atoi(pitch_number);
jakes_model->s.angles[ROLL] = atoi(roll_number);
jakes_model->classname = "jmodel";
jakes_model->r.mins[0] = atoi(xmin);
jakes_model->r.mins[1] = atoi(ymin);
jakes_model->r.mins[2] = atoi(zmin);
jakes_model->r.maxs[0] = atoi(xmax);
jakes_model->r.maxs[1] = atoi(ymax);
jakes_model->r.maxs[2] = atoi(zmax);
SP_jakes_model(jakes_model);
jakes_model->r.mins[0] = atoi(xmin);
jakes_model->r.mins[1] = atoi(ymin);
jakes_model->r.mins[2] = atoi(zmin);
jakes_model->r.maxs[0] = atoi(xmax);
jakes_model->r.maxs[1] = atoi(ymax);
jakes_model->r.maxs[2] = atoi(zmax);
trap_LinkEntity(jakes_model);
G_Printf("Added model: %s at <%s %s %s %s>", model, origin_number, origin_number2, origin_number3, yaw_number);
}






















void	Svcmd_AddModelghoul( void ) {
gentity_t *jakes_model = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];
char pitch_number[MAX_STRING_CHARS];
char roll_number[MAX_STRING_CHARS];
char xmin[MAX_STRING_CHARS];
char ymin[MAX_STRING_CHARS];
char zmin[MAX_STRING_CHARS];
char xmax[MAX_STRING_CHARS];
char ymax[MAX_STRING_CHARS];
char zmax[MAX_STRING_CHARS];

if ( trap_Argc() < 6 ) {
	G_Printf("Usage: /addmodel (model) (x) (y) (z) (yaw)");
		return;
}

trap_Argv( 1, model, sizeof( model ) );
AddSpawnField("model", model);
trap_Argv( 2, origin_number, 1024 );
jakes_model->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
jakes_model->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
jakes_model->s.origin[2] = atoi(origin_number3);
trap_Argv( 5, yaw_number, 1024 );
trap_Argv( 6, pitch_number, 1024 );
trap_Argv( 7, roll_number, 1024 );
trap_Argv( 8, xmin, 1024 );
trap_Argv( 9, ymin, 1024 );
trap_Argv( 10, zmin, 1024 );
trap_Argv( 11, xmax, 1024 );
trap_Argv( 12, ymax, 1024 );
trap_Argv( 13, zmax, 1024 );
jakes_model->s.angles[YAW] = atoi(yaw_number);
jakes_model->s.angles[PITCH] = atoi(pitch_number);
jakes_model->s.angles[ROLL] = atoi(roll_number);
jakes_model->classname = "jmodel2";
jakes_model->r.mins[0] = atoi(xmin);
jakes_model->r.mins[1] = atoi(ymin);
jakes_model->r.mins[2] = atoi(zmin);
jakes_model->r.maxs[0] = atoi(xmax);
jakes_model->r.maxs[1] = atoi(ymax);
jakes_model->r.maxs[2] = atoi(zmax);
SP_mc_ghoul(jakes_model);
jakes_model->r.mins[0] = atoi(xmin);
jakes_model->r.mins[1] = atoi(ymin);
jakes_model->r.mins[2] = atoi(zmin);
jakes_model->r.maxs[0] = atoi(xmax);
jakes_model->r.maxs[1] = atoi(ymax);
jakes_model->r.maxs[2] = atoi(zmax);
trap_LinkEntity(jakes_model);
G_Printf("Added model2: %s at <%s %s %s %s>", model, origin_number, origin_number2, origin_number3, yaw_number);
}












void shaderlistsend(void)
{
    trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
}
void shaderblaxnosend(void)
{
	  char oldshader[1024];
	  char newshader[1024];
	  float hornstinkt = level.time * 0.001;
		trap_Argv( 1, oldshader, sizeof( oldshader ) );
	  trap_Argv( 2, newshader, sizeof( newshader ) );
	  
    AddRemap(oldshader, newshader, hornstinkt);
    //trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
}

void shaderbla(void)
{
	  char oldshader[1024];
	  char newshader[1024];
	  float hornstinkt = level.time * 0.001;
		trap_Argv( 1, oldshader, sizeof( oldshader ) );
	  trap_Argv( 2, newshader, sizeof( newshader ) );
	  
    AddRemap(oldshader, newshader, hornstinkt);
    trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
}
/*
=================
svcmd_addmcbutton
=================
*/
extern void AddSpawnField(char *field, char *value);
/*
extern void SP_mcbutton( gentity_t *ent );
void	svcmd_addmcbutton( void ) {
	gentity_t *mcbutton = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctarget[MAX_STRING_CHARS];
	char   mcmessage[MAX_STRING_CHARS];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 7 ) {
		G_Printf("Usage: /addmcbutton <cost> <target> <message> <x> <y> <z>\n");
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcbutton->count = atoi(mccount);
	trap_Argv( 2, mctarget, 1024 );
	/*mcbutton->mctarget = * /strcpy(mcbutton->mctarget,mctarget);
	trap_Argv( 3, mcmessage, 1024 );
	//AddSpawnField("message", mcmessage);
	//mcbutton->mcmessage = mcmessage;
	strcpy(mcbutton->mcmessage,mcmessage);
	trap_Argv( 4, origin_number, 1024 );
	mcbutton->s.origin[0] = atoi(origin_number);
	trap_Argv( 5, origin_number2, 1024 );
	mcbutton->s.origin[1] = atoi(origin_number2);
	trap_Argv( 6, origin_number3, 1024 );
	mcbutton->s.origin[2] = atoi(origin_number3) - 19;
	SP_mcbutton(mcbutton);
	return;
}
*/

/*
=================
svcmd_addmccreditrelay
=================
*/
extern void AddSpawnField(char *field, char *value);
/*
extern void SP_target_mccreditrelay( gentity_t *ent );
void	svcmd_addmccreditrelay( void ) {
	gentity_t *mccreditrelay = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctarget[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 4 ) {
		G_Printf("Usage: /addmccreditrelay <cost> <target> <targetname>\n");
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mccreditrelay->count = atoi(mccount);
	trap_Argv( 2, mctarget, 1024 );
	//mccreditrelay->mctarget = mctarget;
	strcpy(mccreditrelay->mctarget,mctarget);
	trap_Argv( 3, mctargetname, 1024 );
	//mccreditrelay->mctargetname = mctargetname;
	strcpy(mccreditrelay->mctargetname,mctargetname);
	mccreditrelay->s.origin[0] = 0;
	mccreditrelay->s.origin[1] = 0;
	mccreditrelay->s.origin[2] = 0;
	SP_target_mccreditrelay(mccreditrelay);
	return;
}
*/
/*
=================
svcmd_addmcspeed
=================
*/
extern void AddSpawnField(char *field, char *value);
/*
extern void SP_target_mcspeed( gentity_t *ent );
void	svcmd_addmcspeed( void ) {
	gentity_t *mcspeed = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	char origin_number[MAX_STRING_CHARS];
	char origin_number2[MAX_STRING_CHARS];
	char origin_number3[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		G_Printf("Usage: /addmcspeed <newspeed> <targetname>\n");
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcspeed->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	//mcspeed->mctargetname = mctargetname;
	strcpy(mcspeed->mctargetname,mctargetname);
	mcspeed->s.origin[0] = 0;
	mcspeed->s.origin[1] = 0;
	mcspeed->s.origin[2] = 0;
	SP_target_mcspeed(mcspeed);
	return;
}
*/
/*
=================
svcmd_addmcgravity
=================
*/
extern void AddSpawnField(char *field, char *value);
/*
extern void SP_target_mcgravity( gentity_t *ent );
void	svcmd_addmcgravity( void ) {
	gentity_t *mcgravity = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		G_Printf("Usage: /addmcgravity <newgravity> <targetname>\n");
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mcgravity->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	//mcgravity->mctargetname = mctargetname;
	strcpy(mcgravity->mctargetname,mctargetname);
	mcgravity->s.origin[0] = 0;
	mcgravity->s.origin[1] = 0;
	mcgravity->s.origin[2] = 0;
	SP_target_mcgravity(mcgravity);
	return;
}
*/
/*
=================
svcmd_addmccredits
=================
*/
extern void AddSpawnField(char *field, char *value);
/*
extern void SP_target_mccredits( gentity_t *ent );
void	svcmd_addmccredits( void ) {
	gentity_t *mccredits = G_Spawn();
	char   mccount[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		G_Printf("Usage: /addmccredits <newcredits> <targetname>\n");
		return;
	}
	trap_Argv( 1, mccount, sizeof( mccount ) );
	mccredits->count = atoi(mccount);
	trap_Argv( 2, mctargetname, 1024 );
	//mccredits->mctargetname = mctargetname;
	strcpy(mccredits->mctargetname,mctargetname);
	mccredits->s.origin[0] = 0;
	mccredits->s.origin[1] = 0;
	mccredits->s.origin[2] = 0;
	SP_target_mccredits(mccredits);
	return;
}
*/
/*
=================
svcmd_addmcchat
=================
*/
extern void AddSpawnField(char *field, char *value);
/*
extern void SP_target_mcchat( gentity_t *ent );
void	svcmd_addmcchat( void ) {
	gentity_t *mcchat = G_Spawn();
	char   mcmessage[MAX_STRING_CHARS];
	char   mctargetname[MAX_STRING_CHARS];
	if ( trap_Argc() != 3 ) {
		G_Printf("Usage: /addmcchat <message> <targetname>\n");
		return;
	}
	trap_Argv( 1, mcmessage, sizeof( mcmessage ) );
	//mcchat->message = mcmessage;
	strcpy(mcchat->mcmessage,mcmessage);
	trap_Argv( 2, mctargetname, 1024 );
	//mcchat->mctargetname = mctargetname;
	strcpy(mcchat->mctargetname,mctargetname);
	mcchat->s.origin[0] = 0;
	mcchat->s.origin[1] = 0;
	mcchat->s.origin[2] = 0;
	SP_target_mcchat(mcchat);
	return;
}
*/
/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}
/*
void mcspawnent2(void)
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
	int	needrespawn;
	gentity_t	*newent = G_Spawn();
	if (trap_Argc() < 6)
	{
		G_Printf(va("^7Usage: /ammap_place <classname> <x> <y> <z> <spawnstring>^7\n" ) );
		return;
	}
	trap_Argv(1, type, sizeof(type));
	trap_Argv(2, iXi, sizeof(iXi));
	trap_Argv(3, iYi, sizeof(iYi));
	trap_Argv(4, iZi, sizeof(iZi));
	trap_Argv(5, spfstring, sizeof(spfstring));
	trap_Argv(6, iA1i, sizeof(iA1i));
	trap_Argv(7, iA2i, sizeof(iA2i));
	trap_Argv(8, iA3i, sizeof(iA3i));
	newent->s.origin[0] = atoi(iXi);
	newent->s.origin[1] = atoi(iYi);
	newent->s.origin[2] = atoi(iZi);
	newent->s.angles[YAW] = atoi(iA1i);
	newent->s.angles[PITCH] = atoi(iA2i);
	newent->s.angles[ROLL] = atoi(iA2i);
	G_ParseField( "classname", type, newent );
	G_CallSpawn(newent);
	G_SetAngles(newent,newent->s.angles);
	needrespawn = 0;
	trap_LinkEntity(newent);
	Com_sprintf( iBuffer, sizeof(iBuffer), "");
	//Com_sprintf( iBuffer, sizeof(iBuffer), "Spawned new %s : %i", type, newent->s.number);
	
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
				G_ParseField( current_type, current_value, newent );
				if (Q_stricmp( current_type, "angle" ) == 0)
				{
					newent->s.angles[YAW] = atoi(current_value);
					newent->s.angles[PITCH] = 0;
					newent->s.angles[ROLL] = 0;
					needrespawn = 1;
					G_SetAngles(newent,newent->s.angles);
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
					needrespawn = 1;
					G_SetAngles(newent,newent->s.angles);
				}
				Com_sprintf( iBuffer, sizeof(iBuffer), "%s ('%s' = '%s')", iBuffer, current_type, current_value);
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
	if (needrespawn == 1)
	{
		needrespawn = ent_respawn(newent);
		//trap_SendServerCommand( ent-g_entities, va("print \"^3Ignore following message. The correct entity number is ^5%i^3.\n\"", needrespawn ) );
		G_Printf(va("^7Spawned new ^5%s^7 : ^5%i^7", type, needrespawn));
	}
	else
	{
		G_Printf(va("^7Spawned new ^5%s^7 : ^5%i^7", type, newent->s.number));
	}
	G_Printf(va("^7%s\n", iBuffer ) );
	G_ParseField( "classname", type, newent );
}*/
void mcspawnent2(gentity_t *ent)
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
	int	needrespawn;
	gentity_t	*newent = G_Spawn();
	if (trap_Argc() < 5)
	{
		G_Printf(va("print \"^7Usage: /ammap_place <classname> <x> <y> <z> <spawnstring>^7\n\"" ) );
		return;
	}
	trap_Argv(1, type, sizeof(type));
	trap_Argv(2, iXi, sizeof(iXi));
	trap_Argv(3, iYi, sizeof(iYi));
	trap_Argv(4, iZi, sizeof(iZi));
	trap_Argv(5, spfstring, sizeof(spfstring));
	trap_Argv(6, iA1i, sizeof(iA1i));
	trap_Argv(7, iA2i, sizeof(iA2i));
	trap_Argv(8, iA3i, sizeof(iA3i));
	newent->s.origin[0] = atoi(iXi);
	newent->s.origin[1] = atoi(iYi);
	newent->s.origin[2] = atoi(iZi);
	newent->s.angles[YAW] = atoi(iA1i);
	newent->s.angles[PITCH] = atoi(iA2i);
	newent->s.angles[ROLL] = atoi(iA2i);
	G_ParseField( "classname", type, newent );
	AddSpawnField("classname", type);
	//G_CallSpawn(newent);
	G_SetAngles(newent,newent->s.angles);
	needrespawn = 0;
	trap_LinkEntity(newent);
	Com_sprintf( iBuffer, sizeof(iBuffer), "");
	//Com_sprintf( iBuffer, sizeof(iBuffer), "Spawned new %s : %i", type, newent->s.number);
	
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
				G_ParseField( current_type, current_value, newent );
				//AddSpawnField(current_type, current_value);
				if (Q_stricmp( current_type, "angle" ) == 0)
				{
					newent->s.angles[YAW] = atoi(current_value);
					newent->s.angles[PITCH] = 0;
					newent->s.angles[ROLL] = 0;
					//needrespawn = 1;
					G_SetAngles(newent,newent->s.angles);
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
	if (needrespawn == 1)
	{
		needrespawn = ent_respawn(newent);
		//trap_SendServerCommand( ent-g_entities, va("print \"^3Ignore following message. The correct entity number is ^5%i^3.\n\"", needrespawn ) );
		G_Printf(va("print \"^7Spawned new ^5%s^7 : ^5%i^7\"", type, needrespawn));
	}
	else
	{
		trap_SendServerCommand(ent-g_entities,  va("print \"^7Spawned new ^5%s^7 : ^5%i^7\"", type, newent->s.number));
	}
	G_Printf(va("print \"^7%s\n\"", iBuffer ) );
	G_ParseField( "classname", type, newent );
}
/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
		default:
			G_Printf("%i                  ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}
gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	/*// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );
*/
	i = dsp_adminTarget(NULL, s, MAX_CLIENTS+1);
	if (i < 0)
	{
		return NULL;
	}
	return &level.clients[i];
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
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
}

void	mcmapchange( void ) {
	gentity_t	*lolzw;
	char		newmap[MAX_TOKEN_CHARS];
	int		i;

	trap_Argv( 1, newmap, sizeof( newmap ) );
	for (i=0; i<MAX_CLIENTS; i++)
	{ 
		lolzw = &g_entities[i];
		if ( !lolzw->client || ( lolzw->client->pers.connected != CON_CONNECTING && lolzw->client->pers.connected != CON_CONNECTED ))
		{
			continue;
		}
		if ( (Q_stricmp(lolzw->client->sess.userlogged, "") != 0) )
		{
			mclogout(lolzw);
		}
	}
	trap_SendConsoleCommand(EXEC_APPEND, va("rmap %s\n", newmap));
}
void mctele(void)
{
	gentity_t *other;
	vec3_t	origin;
	char par1[MAX_STRING_CHARS];
	char par2[MAX_STRING_CHARS];
	char par3[MAX_STRING_CHARS];
	char par4[MAX_STRING_CHARS];
	int num;
	trap_Argv( 1, par1, sizeof( par1 ) );
	trap_Argv( 2, par2, sizeof( par2 ) );
	trap_Argv( 3, par3, sizeof( par3 ) );
	trap_Argv( 4, par4, sizeof( par4 ) );
	num = dsp_adminTarget(NULL, par1, MAX_CLIENTS+1);
	if (num < 0)
	{
		G_Printf("Unknown player.\n");
		return;
	}
	other = &g_entities[num];
	VectorClear(origin);
	origin[0] = atoi(par2);
	origin[1] = atoi(par3);
	origin[2] = atoi(par4);
	TeleportPlayer( other, origin, other->client->ps.viewangles );
}
void mckick(void)
{
	char par1[MAX_STRING_CHARS];
	char par2[MAX_STRING_CHARS];
	char par3[MAX_STRING_CHARS];
	char par4[MAX_STRING_CHARS];
	int num;
	trap_Argv( 1, par1, sizeof( par1 ) );
	num = dsp_adminTarget(NULL, par1, MAX_CLIENTS+1);
	if (num < 0)
	{
		G_Printf("Unknown player.\n");
		return;
	}
	trap_DropClient(num, va("%s", twimod_kickmsg.string));
}
void mckill(void)
{
	char par1[MAX_STRING_CHARS];
	char par2[MAX_STRING_CHARS];
	char par3[MAX_STRING_CHARS];
	char par4[MAX_STRING_CHARS];
	int num;
	gentity_t	*ent;
	trap_Argv( 1, par1, sizeof( par1 ) );
	num = dsp_adminTarget(NULL, par1, MAX_CLIENTS+1);
	if (num < -1)
	{
		G_Printf("Unknown player.\n");
		return;
	}
	if (num == -1)
	{
		for (num = 0;num < 32;num += 1)
		{
			ent = &g_entities[num];
			if (ent && ent->inuse && ent->client)
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
		}
	}
	else
	{
			ent = &g_entities[num];
			Cmd_Kill_f (ent);
			if (ent->health < 1)
				{
				float presaveVel = ent->client->ps.velocity[2];
				ent->client->ps.velocity[2] = 500;
				DismembermentTest(ent);
				ent->client->ps.velocity[2] = presaveVel;
				}
	}
}
char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char		cmd[MAX_TOKEN_CHARS];
	gentity_t	*lolzw;
	int		i;
	int		clientNum = (MAX_CLIENTS+1); // Deathspike :: Fake caller
	gentity_t	*ent = NULL; // Deathspike :: Fake caller
	char		cmd_a2[MAX_TOKEN_CHARS];
	char		cmd_a3[MAX_TOKEN_CHARS];
	char		cmd_a4[MAX_TOKEN_CHARS];
	char		cmd_a5[MAX_TOKEN_CHARS];
	char		cmd_a6[MAX_TOKEN_CHARS];
	char		cmd_a7[MAX_TOKEN_CHARS];
	char		cmd_a8[MAX_TOKEN_CHARS];
	char		cmd_a9[MAX_TOKEN_CHARS];
	char		cmd_a10[MAX_TOKEN_CHARS];
	fileHandle_t	f;

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
	mc_print(va("ServerCommand: %s %s %s %s %s %s %s %s %s %s\n", cmd, cmd_a2, cmd_a3, cmd_a4, cmd_a5, cmd_a6, cmd_a7, cmd_a8, cmd_a9, cmd_a10));

	for (i=0; i<MAX_CLIENTS; i++)
	{ 
		lolzw = &g_entities[i];
		if (!lolzw || !lolzw->client)
		{
			continue;
		}
		if (lolzw->client->sess.monitor2 == 1)
		{
			trap_SendServerCommand( lolzw-g_entities, va("print \"ServerCommand: %s^7: %s %s %s %s %s %s %s %s %s %s\n\"",ent->client->pers.netname, cmd, cmd_a2, cmd_a3, cmd_a4, cmd_a5, cmd_a6, cmd_a7, cmd_a8, cmd_a9, cmd_a10));
		}
	}
	if (Q_stricmp (cmd, "specialgametype") == 0)
	{
		int	iL;
		iL = strlen(cmd_a2);
		for (i = 0;i < iL;i += 1)
		{
			if (cmd_a2[i] == ';')
			{
				G_Printf( va("print \"^3Cannot have ; in gametype name.\n\""));
				return qtrue;
			}
			if (cmd_a2[i] == '\n')
			{
				G_Printf( va("print \"^3Cannot have lineskip in gametype name.\n\""));
				return qtrue;
			}
		}
		trap_FS_FOpenFile(va("gametypes/gt_%s.cfg", cmd_a2), &f, FS_READ);
		if (!f)
		{
			trap_SendServerCmd( ent->s.number, va("print \"^1Unknown specialgametype ~^5%s^1~.\n\"", cmd_a2));
			return qtrue;
		}
		trap_FS_FCloseFile(f);
		trap_SendServerCmd( -1, va("print \"^3Specialgametype ^5%s^3 activated.\n\"", cmd_a2));
		trap_SendConsoleCommand( EXEC_APPEND, va("exec gametypes/gt_%s.cfg\n", cmd_a2));
		return qtrue;
	}
	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}
	if ( Q_stricmp (cmd, "mapeditsexec") == 0 ) {
		trap_SendConsoleCommand( EXEC_INSERT, va( ";exec %s/%s.cfg;", mc_editfolder.string, cmd_a2 ) );
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addeffectrcon") == 0) {
		level.mmeffects += 1;
		Svcmd_AddEffectlol();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmodelrcon") == 0) {
		level.mmmodels += 1;
		Svcmd_AddModellol();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addghoulrcon") == 0) {
		level.mmgmodels += 1;
		Svcmd_AddModelghoul();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmodelrcon2") == 0) {
		level.mmmodels += 1;
		Svcmd_AddModellolnew();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmodelrcon3") == 0) {
		level.mmmodels += 1;
		Svcmd_AddModellolnewmcm();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmccredits") == 0) {
		svcmd_addmccredits2();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmcgravity") == 0) {
		svcmd_addmcgravity2();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmcspeed") == 0) {
		svcmd_addmcspeed2();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmcbutton") == 0) {
		svcmd_addmcbutton2();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmcchat") == 0) {
		svcmd_addmcchat2();
		return qtrue;
	}
	if (Q_stricmp (cmd, "addanyent") == 0) {
		//mcspawnent2(NULL);
		mcspawnent(&g_entities[33]);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcaddtele") == 0) {
		vec3_t	pos;
		VectorClear(pos);
		pos[0] = atof(cmd_a3);
		pos[1] = atof(cmd_a4);
		pos[2] = atof(cmd_a5);
		teleporter_add(pos, cmd_a2, atoi(cmd_a6));
		return qtrue;
	}
	if (Q_stricmp (cmd, "ammap") == 0) {
		mcmapchange();
		return qtrue;
	}
	if (Q_stricmp (cmd, "mapeditsdone") == 0)
	{
		//trap_SendServerCommand(-1, va("print \"^3Map edits complete: ^5%i^3 delete%s, ^5%i^3 MD3%s, ^5%i^3 GLM%s, ^5%i^3 effect%s, ^5%i^3 shader remap%s, ^5%i^3 entit%s refused due to high brush model numbers, and ^5%i^3 entit%s refused due to invalid classnames.\"", level.mmdeletes, needsS(level.mmdeletes), level.mmmodels, needsS(level.mmmodels), level.mmgmodels, needsS(level.mmgmodels), level.mmeffects, needsS(level.mmeffects), level.mmshaders, needsS(level.mmshaders), level.mmfailbmodel, needsS2(level.mmfailbmodel), level.mmfailclass, needsS2(level.mmfailclass)));
		int	errord;
		errord = 0;
		trap_SendServerCommand(-1, va("print \"^3Map edits complete: ^5%i^3 delete%s, ^5%i^3 MD3%s, ^5%i^3 GLM%s, ^5%i^3 effect%s\"", level.mmdeletes, needsS(level.mmdeletes), level.mmmodels, needsS(level.mmmodels), level.mmgmodels, needsS(level.mmgmodels), level.mmeffects, needsS(level.mmeffects), level.mmshaders));
		if (level.mmfailbmodel > 0)
		{
			errord = 1;
			if (level.mmfailclass > 0)
			{
				trap_SendServerCommand(-1, va("print \"^3, ^5%i^3 shader remap%s, ^5%i^3 entities refused due to high brush model numbers\"", level.mmshaders, needsS(level.mmshaders), level.mmfailbmodel));
			}
			else
			{
				trap_SendServerCommand(-1, va("print \"^3, ^5%i^3 shader remap%s, and ^5%i^3 entities refused due to high brush model numbers.\n\"", level.mmshaders, needsS(level.mmshaders), level.mmfailbmodel));
			}
		}
		if (level.mmfailclass > 0)
		{
			if (level.mmfailbmodel > 0)
			{
				trap_SendServerCommand(-1, va("print \"^3, and ^5%i^3 entities refused due to invalid classnames.\n\"", level.mmfailclass));
			}
			else
			{
				trap_SendServerCommand(-1, va("print \"^3, ^5%i^3 shader remap%s, and ^5%i^3 entities refused due to invalid classnames.\n\"", level.mmshaders, needsS(level.mmshaders), level.mmfailclass));
			}
			errord = 1;
		}
		if (errord == 0)
		{
			trap_SendServerCommand(-1, va("print \"^3, and ^5%i^3 shader remap%s.\n\"", level.mmshaders, needsS(level.mmshaders)));
		}
		if (level.mmshaders > 0)
		{
			shaderlistsend();
		}
		level.mapeditsdone = 1;
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_setvar") == 0) {
		mc_buildercmds(ent, clientNum, 3);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_respawn") == 0) {
		mc_buildercmds(ent, clientNum, 5);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_usetarget") == 0) {
		int	iFr;
		for (iFr = 31;iFr < 1020;iFr += 1)
		{
			gentity_t	*iFR = &g_entities[iFr];
			if (!iFR || !iFR->inuse || !iFR->targetname)
			{
				continue;
			}
			if (Q_stricmp(iFR->targetname, cmd_a2) == 0)
			{
				iFR->use(iFR, &g_entities[33], &g_entities[33]);
			}
		}
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_delent") == 0) {
		level.mmdeletes += 1;
		mc_buildercmds(ent, clientNum, 2);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_save") == 0) {
		mc_buildercmds(ent, clientNum, 13);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_setangles") == 0) {
		mc_buildercmds(ent, clientNum, 7);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmap_nudgeent") == 0) {
		mc_buildercmds(ent, clientNum, 1);
		return qtrue;
	}

	if (Q_stricmp (cmd, "addmcsentry") == 0) {
		mcspawnsentry2(atoi(cmd_a2), atoi(cmd_a3), atoi(cmd_a4), atoi(cmd_a5));
		return qtrue;
	}
	if (Q_stricmp (cmd, "addmcshield") == 0) {
		mcPlaceShield2(atoi(cmd_a3), atoi(cmd_a4), atoi(cmd_a5), atoi(cmd_a6), atoi(cmd_a2));
		return qtrue;
	}

	if (Q_stricmp(cmd, "nomapfix") == 0)
	{
		level.bahg = 32;
		return qtrue;
	}
/*
	if (Q_stricmp(cmd, "thisistpm") == 0)
	{
		level.thisistpm = 32;
		trap_SendConsoleCommand( EXEC_INSERT, va(";exec tpmedits/tpm_notes;exec tpmedits/tpm_notes2;\n") );
		return qtrue;
	}
*/
	if (Q_stricmp(cmd, "statuswrite") == 0)
	{
		level.statuswrite = 32;
		return qtrue;
	}
/*
	if (Q_stricmp (cmd, "waterworld") == 0)
	{
		if (level.waterworld == 0)
		{
			level.waterworld = 1;
			G_Printf("Wet!\n");
		}
		else
		{
			level.waterworld = 0;
			G_Printf("Dry.\n");			
		}
		return qtrue;
	}
*/
	if (Q_stricmp (cmd, "addlightrcon") == 0)
	{
		gentity_t	*light = G_Spawn();
		light->s.origin[0] = atoi(cmd_a2);
		light->s.origin[1] = atoi(cmd_a3);
		light->s.origin[2] = atoi(cmd_a4);
		SP_mc_light ( light );
		light->s.constantLight = atoi(cmd_a5) + (atoi(cmd_a6)<<8) + (atoi(cmd_a7)<<16) + (atoi(cmd_a8)<<24);
		strcpy(light->mcmlight,va("%i %i %i %i",atoi(cmd_a5), atoi(cmd_a6), atoi(cmd_a7), atoi(cmd_a8)));
		return qtrue;
	}
	if (Q_stricmp (cmd, "cshaderrcon") == 0) {
		level.mmshaders += 1;
		if (level.mapeditsdone == 1)
		{
			shaderbla();
		}
		else
		{
			shaderblaxnosend();
		}
		return qtrue;
	}
	if (Q_stricmp (cmd, "mctele") == 0) {
		mctele();
		return qtrue;
	}
	if (Q_stricmp (cmd, "mckill") == 0)
	{
		mckill();
		return qtrue;
	}
	if (Q_stricmp (cmd, "rmap") == 0) {
		level.reFix = 1;
		trap_SendConsoleCommand( EXEC_APPEND, va(";map %s;\n", cmd_a2) );
		return qtrue;
	}
	if (Q_stricmp (cmd, "mckick") == 0) {
		mckick();
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcmaddban") == 0) {
		bans_add(atoi(cmd_a2), atoi(cmd_a3), atoi(cmd_a4), atoi(cmd_a5));
		return qtrue;
	}
	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}
	if (Q_stricmp(cmd, "rstatus") == 0)
	{
		Svcmd_mcStatus_f();
		return qtrue;
	}
/*
	if (Q_stricmp(cmd, "addnotercon") == 0)
	{
		gentity_t	*note = G_Spawn();
		// creator = cmd_a2
		// message = cmd_a3
		// public = cmd_a4
		// x = cmd_a5
		// y = cmd_a6
		// z = cmd_a7
		level.numSpawnVars = 0;
		level.numSpawnVarChars = 0;
		AddSpawnField("model", cmd_a3);
		note->s.origin[0] = atoi(cmd_a5);
		note->s.origin[1] = atoi(cmd_a6);
		note->s.origin[2] = atoi(cmd_a7);
		note->classname = "mc_note";
		SP_mc_note(note, NULL, cmd_a4);
		strcpy(note->mctargetname,cmd_a2);
		return qtrue;
	}
*/

				// addnotercon "creator" "message" public X Y Z
	if (Q_stricmp (cmd, "rtestfile") == 0) {
		fileHandle_t	f;
		if (Q_stricmp(cmd_a2,"") == 0)
		{
			G_Printf("/rtestfile <file>\n");
			return qtrue;
		}
		trap_FS_FOpenFile(cmd_a2, &f, FS_READ);
		if (f)
		{
			G_Printf("Positive.\n");
			trap_FS_FCloseFile( f );
		}
		else
		{
			G_Printf("Negative.\n");
		}
		return qtrue;
	}
	if (Q_stricmp (cmd, "mcjetpack_effect") == 0) {
		if (Q_stricmp(cmd_a2,"") == 0)
		{
			G_Printf("mcjetpack_effect <effectfile>");
			level.jetfx = 0;
			return qtrue;
		}
		level.jetpack_effect = G_EffectIndex( cmd_a2 );
		level.jetfx = 1;
		return qtrue;
	}
	if (Q_stricmp(cmd,"mcjetdist") == 0)
	{
		level.jetdist = atoi(cmd_a2);
	}
	// Twimod Rcon commands
	// End
	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}
	if (Q_stricmp(cmd, "testsendtoplayer") == 0)
	{
		int num;
		num = dsp_adminTarget(NULL, cmd_a2, MAX_CLIENTS+1);
		if (num < 0)
		{
			G_Printf("Unknown player.\n");
			return qtrue;
		}
		trap_SendServerCommand(num, ConcatArgs(2));
		return qtrue;
	}
	if (Q_stricmp(cmd, "mcsetmusic") == 0)
	{
		if (Q_stricmp(cmd_a2,"") == 0)
		{
			G_Printf("/mcsetmusic <musicfile>\n");
			return qtrue;
		}
		trap_SetConfigstring( CS_MUSIC, cmd_a2 );
		return qtrue;
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"Server Admin: %s\n\"", ConcatArgs(1) ) );
			mc_print(va("print \"Server Admin: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}
		if (Q_stricmp (cmd, "say2") == 0) {
			trap_SendServerCommand( -1, va("print \"%s\n\"", ConcatArgs(1) ) );
			mc_print(va("Server Admin(say2): %s\n", ConcatArgs(1) ) );
			return qtrue;
		}
		mc_print(va("Unknown server command '%s'.\n", cmd));
		return qtrue;
	}

	return qfalse;
}


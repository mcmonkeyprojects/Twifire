// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"
void dsp_clanBother( int clientNum );
void dsp_stringEscape(char *in, char *out, int outSize);
extern void dsp_clanBother( int clientNum );
qboolean PM_SaberInTransition( int move );
qboolean PM_SaberInStart( int move );
qboolean PM_SaberInReturn( int move );
void P_SetTwitchInfo(gclient_t	*client)
{
	client->ps.painTime = level.time;
	client->ps.painDirection ^= 1;
}
/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 255 ) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) ) {

		// don't do more than two pain sounds a second
		if ( level.time - client->ps.painTime < 500 ) {
			return;
		}
		P_SetTwitchInfo(client);
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;

		if (client->damage_armor && !client->damage_blood)
		{
			client->ps.damageType = 1; //pure shields
		}
		else if (client->damage_armor)
		{
			client->ps.damageType = 2; //shields and health
		}
		else
		{
			client->ps.damageType = 0; //pure health
		}
	}


	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean	envirosuit;
	int			waterlevel;

	if ( ent->client->noclip || ent->client->sess.nodrown) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

	//
	// check for drowning
	//
	if ( waterlevel == 3 || level.waterworld > 0) {
		ent->waterlevel = 3;
		waterlevel = 3;
		// envirosuit give air
		if ( envirosuit ) {
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += mc_drown_delay.integer;
			if ( ent->health > 0 ) {
				// take more damage the longer underwater
				ent->damage += mc_water_damage.integer;
				if (ent->damage > mc_water_damage_max.integer)
					ent->damage = mc_water_damage_max.integer;

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex(/*"*drown.wav"*/"sound/player/gurp1.wav"));
				} else if (rand()&1) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
				} else {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = mc_water_damage.integer;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && 
		(ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		if (ent->health > 0
			&& ent->pain_debounce_time <= level.time	) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						30*waterlevel, 0, MOD_LAVA);
				}

				if (ent->watertype & CONTENTS_SLIME) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						10*waterlevel, 0, MOD_SLIME);
				}
			}
		}
	}
}





//==============================================================
extern void G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback );
void DoImpact( gentity_t *self, gentity_t *other, qboolean damageSelf )
{
	float magnitude, my_mass;
	vec3_t	velocity;
	int cont;

	if( self->client )
	{
		VectorCopy( self->client->ps.velocity, velocity );
		my_mass = self->mass;
	}
	else 
	{
		VectorCopy( self->s.pos.trDelta, velocity );
		if ( self->s.pos.trType == TR_GRAVITY )
		{
			velocity[2] -= 0.25f * g_gravity.value;
		}
		if( !self->mass )
		{
			my_mass = 1;
		}
		else if ( self->mass <= 10 )
		{
			my_mass = 10;
		}
		else
		{
			my_mass = self->mass;///10;
		}
	}

	magnitude = VectorLength( velocity ) * my_mass / 10;

	/*
	if(pointcontents(self.absmax)==CONTENT_WATER)//FIXME: or other watertypes
		magnitude/=3;							//water absorbs 2/3 velocity

	if(self.classname=="barrel"&&self.aflag)//rolling barrels are made for impacts!
		magnitude*=3;

	if(self.frozen>0&&magnitude<300&&self.flags&FL_ONGROUND&&loser==world&&self.velocity_z<-20&&self.last_onground+0.3<time)
		magnitude=300;
	*/

	if ( !self->client || self->client->ps.lastOnGround+300<level.time || ( self->client->ps.lastOnGround+100 < level.time && other->material >= MAT_GLASS ) )
	{
		vec3_t dir1, dir2;
		float force = 0, dot;

		if ( other->material >= MAT_GLASS )
			magnitude *= 2;

		//damage them
		if ( magnitude >= 100 && other->s.number < ENTITYNUM_WORLD )
		{
			VectorCopy( velocity, dir1 );
			VectorNormalize( dir1 );
			if( VectorCompare( other->r.currentOrigin, vec3_origin ) )
			{//a brush with no origin
				VectorCopy ( dir1, dir2 );
			}
			else
			{
				VectorSubtract( other->r.currentOrigin, self->r.currentOrigin, dir2 );
				VectorNormalize( dir2 );
			}

			dot = DotProduct( dir1, dir2 );

			if ( dot >= 0.2 )
			{
				force = dot;
			}
			else
			{
				force = 0;
			}

			force *= (magnitude/50);

			cont = trap_PointContents( other->r.absmax, other->s.number );
			if( (cont&CONTENTS_WATER) )//|| (self.classname=="barrel"&&self.aflag))//FIXME: or other watertypes
			{
				force /= 3;							//water absorbs 2/3 velocity
			}

			/*
			if(self.frozen>0&&force>10)
				force=10;
			*/

			if( ( force >= 1 && other->s.number != 0 ) || force >= 10)
			{
	/*			
				dprint("Damage other (");
				dprint(loser.classname);
				dprint("): ");
				dprint(ftos(force));
				dprint("\n");
	*/
				if ( other->r.svFlags & SVF_GLASS_BRUSH )
				{
					other->splashRadius = (float)(self->r.maxs[0] - self->r.mins[0])/4.0f;
				}
				if ( other->takedamage )
				{
					G_Damage( other, self, self, velocity, self->r.currentOrigin, force, DAMAGE_NO_ARMOR, MOD_CRUSH);//FIXME: MOD_IMPACT
				}
				else
				{
					G_ApplyKnockback( other, dir2, force );
				}
			}
		}

		if ( damageSelf && self->takedamage )
		{
			//Now damage me
			//FIXME: more lenient falling damage, especially for when driving a vehicle
			if ( self->client && self->client->ps.fd.forceJumpZStart )
			{//we were force-jumping
				if ( self->r.currentOrigin[2] >= self->client->ps.fd.forceJumpZStart )
				{//we landed at same height or higher than we landed
					magnitude = 0;
				}
				else
				{//FIXME: take off some of it, at least?
					magnitude = (self->client->ps.fd.forceJumpZStart-self->r.currentOrigin[2])/3;
				}
			}
			//if(self.classname!="monster_mezzoman"&&self.netname!="spider")//Cats always land on their feet
				if( ( magnitude >= 100 + self->health && self->s.number != 0 && self->s.weapon != WP_SABER ) || ( magnitude >= 700 ) )//&& self.safe_time < level.time ))//health here is used to simulate structural integrity
				{
					if ( (self->s.weapon == WP_SABER || self->s.number == 0) && self->client && self->client->ps.groundEntityNum < ENTITYNUM_NONE && magnitude < 1000 )
					{//players and jedi take less impact damage
						//allow for some lenience on high falls
						magnitude /= 2;
						/*
						if ( self.absorb_time >= time )//crouching on impact absorbs 1/2 the damage
						{
							magnitude/=2;
						}
						*/
					}
					magnitude /= 40;
					magnitude = magnitude - force/2;//If damage other, subtract half of that damage off of own injury
					if ( magnitude >= 1 )
					{
		//FIXME: Put in a thingtype impact sound function
		/*					
						dprint("Damage self (");
						dprint(self.classname);
						dprint("): ");
						dprint(ftos(magnitude));
						dprint("\n");
		*/
						/*
						if ( self.classname=="player_sheep "&& self.flags&FL_ONGROUND && self.velocity_z > -50 )
							return;
						*/
						G_Damage( self, NULL, NULL, NULL, self->r.currentOrigin, magnitude/2, DAMAGE_NO_ARMOR, MOD_FALLING );//FIXME: MOD_IMPACT
					}
				}
		}

		//FIXME: slow my velocity some?

		// NOTENOTE We don't use lastimpact as of yet
//		self->lastImpact = level.time;

		/*
		if(self.flags&FL_ONGROUND)
			self.last_onground=time;
		*/
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		ent->client->ps.loopSound = level.snd_fry;
	} else {
		ent->client->ps.loopSound = 0;
	}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}
		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->r.absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if ( ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount ) {
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}


/*
============
G_MoverTouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_MoverTouchPushTriggers( gentity_t *ent, vec3_t oldOrg ) 
{
	int			i, num;
	float		step, stepSize, dist;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs, dir, size, checkSpot;
	const vec3_t	range = { 40, 40, 52 };

	// non-moving movers don't hit triggers!
	if ( !VectorLengthSquared( ent->s.pos.trDelta ) ) 
	{
		return;
	}

	VectorSubtract( ent->r.mins, ent->r.maxs, size );
	stepSize = VectorLength( size );
	if ( stepSize < 1 )
	{
		stepSize = 1;
	}

	VectorSubtract( ent->r.currentOrigin, oldOrg, dir );
	dist = VectorNormalize( dir );
	for ( step = 0; step <= dist; step += stepSize )
	{
		VectorMA( ent->r.currentOrigin, step, dir, checkSpot );
		VectorSubtract( checkSpot, range, mins );
		VectorAdd( checkSpot, range, maxs );

		num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		// can't use ent->r.absmin, because that has a one unit pad
		VectorAdd( checkSpot, ent->r.mins, mins );
		VectorAdd( checkSpot, ent->r.maxs, maxs );

		for ( i=0 ; i<num ; i++ ) 
		{
			hit = &g_entities[touch[i]];

			if ( hit->s.eType != ET_PUSH_TRIGGER )
			{
				continue;
			}

			if ( hit->touch == NULL ) 
			{
				continue;
			}

			if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
			{
				continue;
			}


			if ( !trap_EntityContact( mins, maxs, hit ) ) 
			{
				continue;
			}

			memset( &trace, 0, sizeof(trace) );

			if ( hit->touch != NULL ) 
			{
				hit->touch(hit, ent, &trace);
			}
		}
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pm;
	gclient_t	*client;
	int		nouserswitch;
	int		i;
	gentity_t	*ento;
	client = ent->client;


	client->ps.saberEntityNum = -1;
	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400;	// faster than normal
		client->ps.basespeed = 400;

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
	pm.trace = nox_trap_Trace;
		pm.pointcontents = trap_PointContents;

		pm.animations = NULL;

		// perform a pmove
		Pmove (&pm);
		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	ento = &g_entities[ent->client->sess.spectatorClient];
	if ((ent->client->sess.spectatorState == SPECTATOR_FOLLOW)&&(ento->client->sess.controller == ent->s.number + 1))
	{
		nouserswitch = 1;
	}
	else
	{
		nouserswitch = 0;
	}
	/*nouserswitch = 0;
	for (i = 0;i <= 31;i += 1)
	{
		gentity_t	*flent = &g_entities[i];
		if (flent && flent->inuse && flent->client)
		{
			if (flent->client->sess.controller == ent->s.number+1)
			{
				nouserswitch = 1;
			}
		}
	}*/
	if (nouserswitch == 0)
	{
	// attack button cycles through spectators
	if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) {
		Cmd_FollowCycle_f( ent, 1 );
	}

	if (client->sess.spectatorState == SPECTATOR_FOLLOW && (ucmd->upmove > 0))
	{ //jump now removes you from follow mode
		StopFollowing(ent);
	}
	}
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( ! g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.buttons & (BUTTON_ATTACK|BUTTON_ALT_ATTACK)) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			trap_DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}
void uwRename(gentity_t *player, const char *newname) 
{ 
   char userinfo[MAX_INFO_STRING]; 
   int clientNum = player-g_entities;
   trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
   Info_SetValueForKey(userinfo, "name", newname);
   trap_SetUserinfo(clientNum, userinfo); 
   ClientUserinfoChanged(clientNum); 
}
extern void mc_doEmote(gentity_t *ent, int cmd);
/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t	*client;
	char *value = "";
	vec3_t angles2;
	client = ent->client;
	client->timeResidual += msec;/*
		if (ent->client->sess.forcegod > 120)
		{
			ent->client->sess.forcegod = 1;
			ent->client->sess.freeze = qfalse;
			ent->client->ps.pm_type = PM_NORMAL;
			mc_doEmote(ent,BOTH_STAND1);
		}
	if (ent->client->sess.dienow == 1)
	{
		G_Damage (ent, ent, ent, NULL, NULL, 500, DAMAGE_NO_PROTECTION, MOD_TRIGGER_HURT);
		ent->client->sess.dienow = 0;
		if (ent->health > 0)
		{
			Cmd_Kill_f (other);
		}
	}
	if (ent->client->sess.forcegod > 1)
	{
		VectorClear(angles2);
		angles2[PITCH] = 270;
		G_PlayEffect_ID(G_EffectIndex( "env/pool" ),ent->client->ps.origin, angles2);
		ent->client->sess.forcegod += 1;
	}*/
	while ( client->timeResidual >= 1000 ) 
	{
		client->timeResidual -= 1000;

		// count down health when over max
		if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
			ent->health--;
		}
		
			if ( ent->client->sess.protect == 1 )
				{
			ent->client->invulnerableTimer = level.time + 60000;
				}

		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			if (client->ps.stats[STAT_ARMOR] < 300)
			{
				client->ps.stats[STAT_ARMOR]--;
			}
		}
	if (!(ent->r.svFlags & SVF_BOT))
	{
	if (ent->client->sess.xAFK != (int)ent->client->ps.origin[0])
	{
		ent->client->sess.xAFK = (int)ent->client->ps.origin[0];
		if (mc_afktime.value == 7.3)
		{
			G_Printf("%i N-at %i ticks [%i, %i]\n", ent->s.number, ent->client->sess.ticksAFK, ent->client->sess.xAFK, (int)ent->client->ps.origin[0]);
		}
	}
	else
	{
		ent->client->sess.ticksAFK += 1;
		if (mc_afktime.value == 7.3)
		{
			G_Printf("%i at %i ticks [%i]\n", ent->s.number, ent->client->sess.ticksAFK, ent->client->sess.xAFK);
		}
		if ((mc_afktime.integer != 0)&&(ent->client->sess.ticksAFK > mc_afktime.integer))
		{
			setAFKOn(ent);
		}
	}
	}
	if (ent->client->sess.forcegod > 7)
	{
		ent->client->sess.forcegod = 1;
		ent->client->sess.freeze = qfalse;
		ent->client->ps.pm_type = PM_NORMAL;
		mc_doEmote(ent,BOTH_STAND1);
	}
	if (ent->client->sess.forcegod > 1)
	{
		VectorClear(angles2);
		angles2[PITCH] = 270;
		G_PlayEffect_ID(G_EffectIndex( "env/pool" ),ent->client->ps.origin, angles2);
		ent->client->sess.forcegod += 1;
	}
	if (client->sess.showspeedfor > 0)
	{
		gentity_t	*tent = &g_entities[client->sess.showspeedfor - 1];
		if (!tent || !tent->client || !tent->inuse)
		{
			client->sess.showspeedfor = 0;
			goto endssf;
		}
		trap_SendServerCommand( ent-g_entities, va("cp \"\n\n\n\n\n\n\n\n\n\nX: ^5%i^7 Y: ^5%i^7 Z: ^5%i^7\nXY: ^5%i^7 XYZ: ^5%i^7\n\"", (int)tent->client->ps.velocity[0], (int)tent->client->ps.velocity[1], (int)tent->client->ps.velocity[2],
				(int)sqrt((tent->client->ps.velocity[0]*tent->client->ps.velocity[0]) + (tent->client->ps.velocity[1]*tent->client->ps.velocity[1])),
				(int)sqrt((tent->client->ps.velocity[0]*tent->client->ps.velocity[0]) + (tent->client->ps.velocity[1]*tent->client->ps.velocity[1]) +  + (tent->client->ps.velocity[2]*tent->client->ps.velocity[2])) ));
	}
	endssf:
	if (ent->client->sess.jetfuel < mc_jetpack_fuelmax.integer)
	{
		ent->client->sess.jetfuel += mc_jetpack_fuelregen.integer;
	}
	else
	{
		ent->client->sess.jetfuel = mc_jetpack_fuelmax.integer;
	}
		if ( ent->client->ps.eFlags & EF_JETPACK_ACTIVE )
		{
			if (client->sess.jetshowfuel == 1)
			{
				trap_SendServerCommand( ent-g_entities, va("cp \"\n\n\n\n\n\n\n\n\n\n\n^3Jet Fuel: ^5%i^3.\n\"",client->sess.jetfuel));
			}
		}
		if ( ( twimod_chatprotect.integer ) && ( client->ps.eFlags & EF_TALK ) )
		{
			if (client->chatTimer == 0)
			{
				client->chatTimer = (twimod_chatprotecttimer.value + 1);
				client->chatGod = 0;
			}
			else if (client->chatTimer == 1)
			{
				// Give god and keep updating the chatthingy
				client->chatTimer = 1;
				client->chatGod = 1;
			}
			else
			{
				client->chatTimer -= 1;
			}
		}
		// Twimod Votecontrol

		else
		{
			client->chatTimer = 0;
			client->chatGod = 0;
		}

		if ( twimod_sleepmsg.string[0] && Q_stricmp( twimod_sleepmsg.string, "none" ) &&
			strcmp( twimod_sleepmsg.string, value ) != 0)
		{
			if ( ent->client->sess.sleep == 1 )
			{
				char		SLEEPMSG[MAX_STRING_CHARS];
				dsp_stringEscape(twimod_sleepmsg.string, SLEEPMSG, MAX_STRING_CHARS);
				trap_SendServerCommand( ent-g_entities, va("cp \"%s\"", SLEEPMSG ) );	
			}
		}
		if (client->sess.empower){ client->ps.fd.forcePower = 100;}
		if (client->sess.blockforce) {client->ps.fd.forcePower = 0;}
	}
		if (client->padawantimer >= 1 && client->sess.padawan == 1 && twimod_antipadawan.integer == 1 && *twimod_padanewname.string && twimod_padanewname.string[0])
			{
			trap_SendServerCommand( ent-g_entities, va("cp \"^1Padawan names are not allowed here!\n\n^7Please change it,\n^7or your name will be changed^1.\n\"", client->padawantimer ));
			client->padawantimer--;
			}
		if (client->padawantimer == 0 && client->sess.padawan == 1)
			{
			client->sess.padawan = 0;
			uwRename(ent, twimod_padanewname.string);
			trap_SendServerCommand( ent-g_entities, va("print \"^1Padawan names are not allowed, you have been forcefully renamed^7.\n\"") );
			}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;
	if ( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) {
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int		i;//, j;
	int		event;
	gclient_t *client;
	int		damage;
	vec3_t	dir;
//	vec3_t	origin, angles;
//	qboolean	fired;
//	gitem_t *item;
//	gentity_t *drop;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL:
		case EV_ROLL:
			{
				int delta = client->ps.eventParms[ i & (MAX_PS_EVENTS-1) ];

				if (ent->client && ent->client->ps.fallingToDeath)
				{
					break;
				}

				if ( ent->s.eType != ET_PLAYER )
				{
					break;		// not in the player model
				}
				
				if ( g_dmflags.integer & DF_NO_FALLING || ent->client->sess.protect || ent->client->chatGod )
				{
					break;
				}

				if (delta <= 44)
				{
					break;
				}

				damage = delta*0.16; //good enough for now, I guess

				VectorSet (dir, 0, 0, 1);
				ent->pain_debounce_time = level.time + 200;	// no normal pain sound
				G_Damage (ent, NULL, NULL, NULL, NULL, damage, DAMAGE_NO_ARMOR, MOD_FALLING);
			}
			break;
		case EV_FIRE_WEAPON:
			FireWeapon( ent, qfalse );
			ent->client->dangerTime = level.time;
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			ent->client->sess.protect = qfalse;
			break;

		case EV_ALT_FIRE:
			FireWeapon( ent, qtrue );
			ent->client->dangerTime = level.time;
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			ent->client->sess.protect = qfalse;
			break;

		case EV_SABER_ATTACK:
			if (ent->client->sess.veh_isactive == 1)
			{
				ent->client->ps.saberHolstered = qtrue;
				ent->client->ps.weaponTime = 400;
				FireWeapon(ent, qfalse);
			}
			if (ent->client->sess.blockweapon)
			{
				ent->client->ps.saberHolstered = qtrue;
				ent->client->ps.weaponTime = 400;
			}
			ent->client->dangerTime = level.time;
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			ent->client->sess.protect = qfalse;
			break;

		//rww - Note that these must be in the same order (ITEM#-wise) as they are in holdable_t
		case EV_USE_ITEM1: //seeker droid
			ItemUse_Seeker(ent);
			break;
		case EV_USE_ITEM2: //shield
			ItemUse_Shield(ent);
			break;
		case EV_USE_ITEM3: //medpack
			ItemUse_MedPack(ent);
			break;
		case EV_USE_ITEM4: //datapad
			//G_Printf("Used Datapad\n");
			break;
		case EV_USE_ITEM5: //binoculars
			ItemUse_Binoculars(ent);
			break;
		case EV_USE_ITEM6: //sentry gun
			ItemUse_Sentry(ent);
			break;

		default:
			break;
		}
	}

}

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

extern int saberOffSound;
extern int saberOnSound;

/*
==================
G_UpdateClientBroadcasts

Determines whether this client should be broadcast to any other clients.  
A client is broadcast when another client is using force sight or is
==================
*/
#define MAX_JEDIMASTER_DISTANCE	2500
#define MAX_JEDIMASTER_FOV		100

#define MAX_SIGHT_DISTANCE		1500
#define MAX_SIGHT_FOV			140 // 100 = always causes problems on fast moving people

static void G_UpdateForceSightBroadcasts ( gentity_t *self )
{
	int i;

	// Any clients with force sight on should see this client
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t *ent = &g_entities[level.sortedClients[i]];
		float	  dist;
		vec3_t	  angles;
	
		if ( ent == self )
		{
			continue;
		}
		if (ent->client->sess.isglowing == 1)
		{
			continue;
		}
			// Not using force sight so we shouldnt broadcast to this one
			if ( !(ent->client->ps.fd.forcePowersActive & (1<<FP_SEE) ) )
			{
				continue;
			}

			VectorSubtract( self->client->ps.origin, ent->client->ps.origin, angles );
			dist = VectorLengthSquared ( angles );
			vectoangles ( angles, angles );

			// Too far away then just forget it
			if ( dist > MAX_SIGHT_DISTANCE * MAX_SIGHT_DISTANCE )
			{
				continue;
			}
		//}
		// If not within the field of view then forget it
		if ( !InFieldOfVision ( ent->client->ps.viewangles, MAX_SIGHT_FOV, angles ) )
		{
			break;
		}

		// Turn on the broadcast bit for the master and since there is only one
		// master we are done
		self->r.broadcastClients[ent->s.clientNum/32] |= (1 << (ent->s.clientNum%32));
	
		break;
	}
}
static void G_UpdateJediMasterBroadcasts ( gentity_t *self )
{
	int i;

	// Not jedi master mode then nothing to do
	if ( g_gametype.integer != GT_JEDIMASTER )
	{
		return;
	}

	// This client isnt the jedi master so it shouldnt broadcast
	if ( !self->client->ps.isJediMaster && mc_sentrylimit.integer )
	{
		return;
	}

	// Broadcast ourself to all clients within range
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t *ent = &g_entities[level.sortedClients[i]];
		float	  dist;
		vec3_t	  angles;

		if ( ent == self )
		{
			continue;
		}
		if (ent->client->sess.isglowing == 1)
		{
			continue;
		}
		VectorSubtract( self->client->ps.origin, ent->client->ps.origin, angles );
		dist = VectorLengthSquared ( angles );
		vectoangles ( angles, angles );

		// Too far away then just forget it
		if ( dist > MAX_JEDIMASTER_DISTANCE * MAX_JEDIMASTER_DISTANCE )
		{
			continue;
		}
		
		// If not within the field of view then forget it
		if ( !InFieldOfVision ( ent->client->ps.viewangles, MAX_JEDIMASTER_FOV, angles ) )
		{
			continue;
		}

		// Turn on the broadcast bit for the master and since there is only one
		// master we are done
		self->r.broadcastClients[ent->s.clientNum/32] |= (1 << (ent->s.clientNum%32));
	}
}

void G_UpdateClientBroadcasts ( gentity_t *self )
{
	// Clear all the broadcast bits for this client
	memset ( self->r.broadcastClients, 0, sizeof ( self->r.broadcastClients ) );

	// The jedi master is broadcast to everyone in range
	G_UpdateJediMasterBroadcasts ( self );

	// Anyone with force sight on should see this client
	G_UpdateForceSightBroadcasts ( self );
}

gentity_t *G_Findbygroup (gentity_t *from, int rgroup);
/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
extern int mcfix360(int orig);
int mcabsval(int orig)
{
    if (orig < 0){return -orig;}
    return orig;
}
void ClientThink_real( gentity_t *ent ) {
	gclient_t	*client;
	pmove_t		pm;
	vec3_t		oppDir;
	int			oldEventSequence;
	int			msec;
	int			i;
	int			idnum;
	int			angle;
	usercmd_t	*ucmd;
	gentity_t	*other;
	char		MOTD[MAX_STRING_CHARS];
	vec3_t		up;
	vec3_t		iorg;
		vec3_t newangle;
		vec3_t	addvel;
		vec3_t	properOrigin;
		float	fVSpeed = 0;
		int sum = 0;

	client = ent->client;
	ent->client->sess.mcPing = level.time - ent->client->sess.mcTime;
	ent->client->sess.mcTime = level.time;
	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}
	if (client->sess.solid == 1)
	{
		ent->r.contents = 0;
		ent->clipmask = 0;
	}
	if (client->sess.shock == 2)
	{
		client->ps.electrifyTime = level.time + 9001000;
	}
	if (ent->client->sess.xAFK != (int)ent->client->ps.origin[0])
	{
		/*if (ent->client->sess.isAFK == 1)
		{
			G_Printf("Client %i moved %i to %i!\n", ent->s.number, ent->client->sess.xAFK, (int)ent->client->ps.origin[0]);
		}*/
		setAFKOff(ent);
	}
	if ((client->sess.pendingtimeout != 0) && (client->sess.pendingtimeout < level.time))
	{
		client->sess.pendingtype = 0;
		client->sess.pendingvalue = 0;
		client->sess.pendingtimeout = 0;
	}
	if (client->sess.viewrandom == 1)
	{
		client->ps.viewangles[YAW] = flrand(0,360)-180.0f;//irand(0,360)-180;
		client->ps.viewangles[PITCH] = flrand(0,180)-90.0f;//irand(0,180)-90;
		SetClientViewAngle(ent, client->ps.viewangles);
	}
	if (client->sess.worshipped == 5)
	{
		vec3_t	angles2;
		if (irand(1,40) == 5)
		{
			for (i = 0;i < 5;i += 1)
			{
				VectorClear(angles2);
				angles2[YAW] = irand(0, 360);
				angles2[PITCH] = -90 + irand(-30, 30);
				angles2[ROLL] = irand(0, 360);
				G_PlayEffect_ID(G_EffectIndex( "lightning" ), client->ps.origin, angles2);
			}
			client->ps.pm_type = PM_NORMAL;
			client->sess.freeze = 0;
			G_RadiusDamage( client->ps.origin, ent, 400, 20, NULL, MOD_REPEATER_ALT_SPLASH );
			G_SoundAtLoc( client->ps.origin, CHAN_VOICE, G_SoundIndex("sound/effects/energy_crackle") );
			client->sess.worshipped = 0;
		}
	}
	if (client->sess.ambaz == 1)
	{
		if (irand(0,20) == 1)
		{
			iorg[0] = ent->client->ps.origin[0] + irand(-60, 60);
			iorg[1] = ent->client->ps.origin[1] + irand(-60, 60);
			iorg[2] = ent->client->ps.origin[2] + irand(-30, 60);
			G_RadiusDamage( iorg, ent, 50, 10, NULL, MOD_REPEATER_ALT_SPLASH );
			G_PlayEffect_ID(G_EffectIndex( "droidexplosion1" ), iorg, ent->client->ps.viewangles);
		}
	}
	if (client->sess.abused != 0)
	{
		if (irand(0,30) == 1)
		{
			client->sess.abused = irand(1,5);
		}
		if (client->sess.abused == 2) // Tele random
		{
			vec3_t	newori;
			newori[0] = flrand(-1000,1000);
			newori[1] = flrand(-1000,1000);
			newori[2] = flrand(-50,100);
			TeleportPlayer( ent, newori, ent->client->ps.viewangles );
		}
		if (client->sess.abused == 1) // View random
		{
			client->ps.viewangles[YAW] = flrand(0,360)-180.0f;//irand(0,360)-180;
			client->ps.viewangles[PITCH] = flrand(0,180)-90.0f;//irand(0,180)-90;
			SetClientViewAngle(ent, client->ps.viewangles);
		}
/*
		if (client->sess.abused == 3) // Slap random
		{
			vec3_t	oppDir;
			int	angle;
				VectorNormalize2( ent->client->ps.velocity, oppDir );
				VectorScale( oppDir, -1, oppDir );
				angle = Q_irand(1, 360) * (M_PI*2 / 360);
				oppDir[1] = (sin(angle)*300);
				oppDir[0] = (cos(angle)*300);
				other->client->ps.velocity[0] = oppDir[0];
				other->client->ps.velocity[1] = oppDir[1];
				other->client->ps.velocity[2] = (300);
				other->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
				other->client->ps.forceHandExtendTime = level.time + (2000);
				other->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
		}
*/
		if (client->sess.abused == 3) // Death/respawn
		{
			if (ent->health >= 0)
			{
				ent->health = -1;
			}
		}
		if (client->sess.abused == 4) // Nothing / brief pause
		{
		}
		if (client->sess.abused == 5) // falling
		{
			client->ps.fallingToDeath = level.time;
		}
	}
	if (client->sess.shock == 2)
	{
		//G_Printf("SHOCKTEST\n");
		for (i = 0;i < 32;i++)
		{
			gentity_t *other = &g_entities[i];
			if (other && other->inuse && other->client && other->s.number != ent->s.number)
			{
				vec3_t	newori;
                		//G_Printf("SHOCKTESTR\n");
				newori[0] = (other->client->ps.origin[0] - ent->client->ps.origin[0]);
				newori[1] = (other->client->ps.origin[1] - ent->client->ps.origin[1]);
				newori[2] = (other->client->ps.origin[2] - ent->client->ps.origin[2]);
				//G_Printf("Try %i, %i, %i", (int)newori[0], (int)newori[1], (int)newori[2]);
				if ((mcabsval(newori[0]) < 40) && (mcabsval(newori[1]) < 40) && (mcabsval(newori[2]) < 80))
				{
					//G_Printf("Shockaway!\n");
					newori[2] += 20;
					other->client->ps.electrifyTime = level.time + 1000;
					G_Damage(other, ent, ent, newori, newori, 10, DAMAGE_NO_ARMOR, MOD_DEMP2);
					G_ApplyKnockback(other, newori, 100 );
				}
			}
		}
	}
	if (client->sess.isglowing == 1)
	{
		if (!(ent->client->ps.fd.forcePowersActive & (1 << FP_TELEPATHY)))
		{
			//WP_ForcePowerStart( ent, FP_TELEPATHY, 0 );
			ent->client->ps.fd.forcePowersActive |= ( 1 << FP_TELEPATHY );
			ent->client->ps.fd.forcePowerDuration[FP_TELEPATHY] = level.time + 9000000;
			ent->client->ps.fd.forcePowerDebounce[FP_TELEPATHY] = 0;
		}
		//ent->client->ps.fd.forceMindtrickTargetIndex = 0;
		//ent->client->ps.fd.forceMindtrickTargetIndex2 = 0;
		//ent->client->ps.fd.forceMindtrickTargetIndex3 = 0;
		//ent->client->ps.fd.forceMindtrickTargetIndex4 = 0;
		//ent->client->ps.forceAllowDeactivateTime = level.time + 1500;
		for (i = 0;i < 32;i += 1)
		{
			gentity_t	*flent = &g_entities[i];
			if (flent && flent->inuse && flent->client)
			{
				//if (ent->client->sess.trickedplayers[i+1] != 'A')
				//{
					WP_AddAsMindtricked(&ent->client->ps.fd, flent->s.number);
					//ent->client->sess.trickedplayers[i+1] = 'A';
				//}
				/*if (i > 15)
				{
					ent->client->ps.fd.forceMindtrickTargetIndex2 |= ( 1 << (i - 16));
				}
				else
				{
					ent->client->ps.fd.forceMindtrickTargetIndex |= ( 1 << (i) );
				}*/
			}
		}
		i = 0;
	}
	if (client->sess.watching != 0)
	{
		gentity_t	*other;
		vec3_t		angles;
		vec3_t		angles2;
		VectorClear(angles);
		VectorClear(angles2);
		other = &g_entities[client->sess.watching-1];
		if (other && other->client && other->inuse)
		{
			angles[0] = other->client->ps.origin[0] - client->ps.origin[0];
			angles[1] = other->client->ps.origin[1] - client->ps.origin[1];
			angles[2] = other->client->ps.origin[2] - client->ps.origin[2];
			vectoangles( angles, angles2 );
			//VectorCopy(angles, client->ps.viewangles);
			SetClientViewAngle( ent, angles2 );
		}
	}
	if (client->sess.torture == 1)
	{
		client->ps.otherKillerTime = level.time + 20000;
		client->ps.otherKillerDebounceTime = level.time + 10000;
		client->ps.fallingToDeath = level.time;
	}
	if ( !ent->client->ps.duelInProgress && ent->client->sess.empower && client->empowerLastRegen + 250 < level.time )
	{
		client->empowerLastRegen		= level.time;
		ent->client->ps.fd.forcePower += 3;
		if ( ent->client->ps.fd.forcePower > 100 ) {ent->client->ps.fd.forcePower = 100;}
		if (ent->client->sess.blockforce == 1) { ent->client->ps.fd.forcePower = 0; }
	}
	if (ent->client->sess.grabbedplayer != 0)
	{
		vec3_t	fwd, angles, origin, mvel;
		gentity_t	*flent;
		flent = &g_entities[ent->client->sess.grabbedplayer-1];
		if (!flent->inuse)
		{
			trap_SendServerCommand(ent->s.number, va("print \"^1Grabbed player disconnected.\n\""));
			ent->client->sess.grabbedplayer = 0;
			ent->client->sess.grabbedpdist = 0;
			goto aftergrab;
		}
		angles[ROLL] = 0;
		angles[PITCH] = ent->client->ps.viewangles[PITCH];
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		origin[0] = ent->client->ps.origin[0] + fwd[0]*ent->client->sess.grabbedpdist;
		origin[1] = ent->client->ps.origin[1] + fwd[1]*ent->client->sess.grabbedpdist;
		origin[2] = ent->client->ps.origin[2]+35 + fwd[2]*ent->client->sess.grabbedpdist + ent->client->sess.grabbedpoffz;
		VectorCopy(origin, flent->client->ps.origin);
		mvel[0] = 0;
		mvel[1] = 0;
		mvel[2] = 0;
		VectorCopy(mvel, flent->client->ps.velocity);
	}
	
	if (ent->client->sess.veh_isactive == 1)
	{
		vec3_t	fwd, origin, angles50;
		gentity_t	*flent;
		flent = &g_entities[ent->client->sess.veh_modent];
		if (!flent->inuse)
		{
			trap_SendServerCommand(ent->s.number, va("print \"^1Vehicle entity was deleted.\n\""));
			exit_vehicle(ent);
			goto aftervehgrab;
		}
		flent->s.origin[0] = ent->client->ps.origin[0] + ent->client->sess.veh_xdown;
		flent->s.origin[1] = ent->client->ps.origin[1] + ent->client->sess.veh_ydown;
		flent->s.origin[2] = ent->client->ps.origin[2]+35 + ent->client->sess.veh_medown;
		G_SetOrigin(flent,flent->s.origin);
		angles50[ROLL] = 0;
		angles50[YAW] = mcfix360(ent->client->ps.viewangles[YAW]);
		if (ent->client->sess.veh_pitch == 1)
		{
			angles50[PITCH] = mcfix360(ent->client->ps.viewangles[PITCH]);
		}
		else
		{
			angles50[PITCH] = 0;
		}
		VectorCopy(angles50, flent->s.angles);
		G_SetAngles(flent,flent->s.angles);
		trap_LinkEntity(flent);
	}
	aftervehgrab:
	if (ent->client->sess.grabbedent != 0)
	{
		vec3_t	fwd, angles, origin, angles50;
		gentity_t	*flent;
		flent = &g_entities[ent->client->sess.grabbedent];
		if (!flent->inuse)
		{
			trap_SendServerCommand(ent->s.number, va("print \"^1Grabbed entity was deleted.\n\""));
			ent->client->sess.grabbedent = 0;
			ent->client->sess.grabbedgroup = 0;
			ent->client->sess.grabbeddist = 0;
			goto aftergrab;
		}
		angles[ROLL] = 0;
		angles[PITCH] = ent->client->ps.viewangles[PITCH];
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		flent->s.origin[0] = ent->client->ps.origin[0] + fwd[0]*ent->client->sess.grabbeddist;
		flent->s.origin[1] = ent->client->ps.origin[1] + fwd[1]*ent->client->sess.grabbeddist;
		flent->s.origin[2] = ent->client->ps.origin[2]+35 + fwd[2]*ent->client->sess.grabbeddist + ent->client->sess.grabbedentoffz;
		G_SetOrigin(flent,flent->s.origin);
		angles50[ROLL] = flent->r.currentAngles[ROLL];
		if (ent->client->sess.grabbedentyaw == 0)
		{
			angles50[YAW] = flent->r.currentAngles[YAW];
		}
		else
		{
			angles50[YAW] = ent->client->ps.viewangles[YAW] + ent->client->sess.grabbedentyaw;
		}
		if (ent->client->sess.grabbedentpitch == 0)
		{
			angles50[PITCH] = flent->r.currentAngles[PITCH];
		}
		else
		{
			angles50[PITCH] = mcfix360(ent->client->ps.viewangles[PITCH] - ent->client->sess.grabbedentpitch/* + 180*/);
		}
		VectorCopy(angles50, flent->s.angles);
		G_SetAngles(flent,flent->s.angles);
		trap_LinkEntity(flent);
	}
	if (ent->client->sess.grabbedgroup != 0)
	{
		vec3_t		fwd, angles, origin;
		gentity_t	*flent;
		gentity_t	*t;
		int		i;
		int		xFin;
		int		yFin;
		int		zFin;
		t = NULL;
		i = 0;
		while ( (t = G_Findbygroup(t, ent->client->sess.grabbedgroup)) != NULL )
		{
			i += 1;
		}
		if (i == 0)
		{
			trap_SendServerCommand(ent->s.number, va("print \"^1Grabbed group was deleted.\n\""));
			ent->client->sess.grabbedent = 0;
			ent->client->sess.grabbedgroup = 0;
			ent->client->sess.grabbeddist = 0;
			goto aftergrab;
		}
		t = NULL;
		t = G_Findbygroup(t, ent->client->sess.grabbedgroup);
		if (t->groupleader == 0)
		{
			int leadnum;
			t = NULL;
			t = G_Findbygroup(t, ent->client->sess.grabbedgroup);
			leadnum = t->s.number;
			t = NULL;
			while ( (t = G_Findbygroup(t, ent->client->sess.grabbedgroup)) != NULL )
			{
				t->groupleader = leadnum;
			}
			t = &g_entities[leadnum];
			G_Printf("Held group had no leader.. resetting\n");
		}
		flent = &g_entities[t->groupleader];
		angles[ROLL] = 0;
		angles[PITCH] = ent->client->ps.viewangles[PITCH];
		angles[YAW] = ent->client->ps.viewangles[YAW];
		AngleVectors(angles, fwd, NULL, NULL);
		xFin = (ent->client->ps.origin[0] + fwd[0]*ent->client->sess.grabbeddist)-flent->s.origin[0];
		yFin = (ent->client->ps.origin[1] + fwd[1]*ent->client->sess.grabbeddist)-flent->s.origin[1];
		zFin = (ent->client->ps.origin[2]+35 + fwd[2]*ent->client->sess.grabbeddist)-flent->s.origin[2] + ent->client->sess.grabbedentoffz;
		t = NULL;
		while ( (t = G_Findbygroup(t, ent->client->sess.grabbedgroup)) != NULL )
		{
			t->s.origin[0] = t->r.currentOrigin[0] + xFin;
			t->s.origin[1] = t->r.currentOrigin[1] + yFin;
			t->s.origin[2] = t->r.currentOrigin[2] + zFin;
			G_SetOrigin(t,t->s.origin);
			trap_LinkEntity(t);
		}
	}
	aftergrab:
	if (ent->client->sess.slapping < level.time && ent->client->sess.slapping != 0)
	{
				ent->client->sess.slapping = 0;
				idnum = dsp_adminTarget(ent, "gun", ent->s.number);
				if (idnum < 0)
				{
					goto aftergrab;
				}
			other = &g_entities[idnum];
		G_Sound( ent, CHAN_VOICE, G_SoundIndex("sound/weapons/galak/skewerhit") );
		G_Sound( other, CHAN_VOICE, G_SoundIndex("sound/weapons/galak/skewerhit") );
			if (!other->client->sess.freeze)
			{
				if (!other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}
				VectorNormalize2( ent->client->ps.velocity, oppDir );
				VectorScale( oppDir, -1, oppDir );
				angle = ent->client->ps.viewangles[YAW] * (M_PI*2 / 360);
				if (angle > 360)
				{
					angle -= 360;
				}
				if (angle < 360)
				{
					angle += 360;
				}
				oppDir[1] = (sin(angle)*400);
				oppDir[0] = (cos(angle)*400);
				other->client->ps.velocity[0] = oppDir[0];
				other->client->ps.velocity[1] = oppDir[1];
				other->client->ps.velocity[2] = (400);
				other->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
				other->client->ps.forceHandExtendTime = level.time + (twimod_slapdowntime.integer * 1000);
				other->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
			}
	}
	if (ent->client->sess.veh_isactive == 1)
	{
		ent->s.weapon = ent->client->sess.veh_forcedweapon;
		ent->client->ps.stats[STAT_WEAPONS] = (1 << ent->client->sess.veh_forcedweapon);
		if (ent->client->ps.ammo[ent->client->sess.veh_forcedweapon] < 100)
		{
			ent->client->ps.ammo[ent->client->sess.veh_forcedweapon] = 100;
		}
	}
	if (client->sess.clanTag && ( client->sess.adminloggedin == 0 ) && !client->sess.clanCounting)
	{
		client->sess.clanCounter = level.time + 1000;
		client->sess.clanTagTime = level.time + twimod_clantagpcounter.integer;
	}
	if (client->sess.clanTag && ( client->sess.adminloggedin == 0 ) && twimod_clantagprotect.integer)
	{ // Deathspike: Tag Protection
		dsp_clanBother(client - level.clients);
	}
	else if (client->MOTDTime < level.time && client->MOTDNumber != 0)
	{ // The MOTD has to be shown.
		client->MOTDTime = level.time + 1000;
		client->MOTDNumber -= 1;

		dsp_stringEscape(twimod_motd.string, MOTD, MAX_STRING_CHARS);
		trap_SendServerCommand(ent-g_entities, va("cp \"\n\n%s^7\"" , MOTD));
	}
	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	
	if (mc_unlagged.integer == 1)
	{

//unlagged - backward reconciliation #4
	// frameOffset should be about the number of milliseconds into a frame 
	// this command packet was received, depending on how fast the server
	// does a G_RunFrame()
	client->frameOffset = trap_Milliseconds() - level.frameStartTime;
//unlagged - backward reconciliation #4


//unlagged - lag simulation #3
	// if the client wants to simulate outgoing packet loss
	if ( client->pers.plOut ) {
		// see if a random value is below the threshhold
		float thresh = (float)client->pers.plOut / 100.0f;
		if ( random() < thresh ) {
			// do nothing at all if it is - this is a lost command
			return;
		}
	}
//unlagged - lag simulation #3


//unlagged - true ping
	// save the estimated ping in a queue for averaging later

	// we use level.previousTime to account for 50ms lag correction
	// besides, this will turn out numbers more like what players are used to
	client->pers.pingsamples[client->pers.samplehead] = level.previousTime + client->frameOffset - ucmd->serverTime;
	client->pers.samplehead++;
	if ( client->pers.samplehead >= NUM_PING_SAMPLES ) {
		client->pers.samplehead -= NUM_PING_SAMPLES;
	}

	// initialize the real ping

		// get an average of the samples we saved up
		for ( i = 0; i < NUM_PING_SAMPLES; i++ ) {
			sum += client->pers.pingsamples[i];
		}

		client->pers.realPing = sum / NUM_PING_SAMPLES;
//unlagged - true ping


//unlagged - lag simulation #2
	// keep a queue of past commands
	client->pers.cmdqueue[client->pers.cmdhead] = client->pers.cmd;
	client->pers.cmdhead++;
	if ( client->pers.cmdhead >= MAX_LATENT_CMDS ) {
		client->pers.cmdhead -= MAX_LATENT_CMDS;
	}

	// if the client wants latency in commands (client-to-server latency)
	if ( client->pers.latentCmds ) {
		// save the actual command time
		int time = ucmd->serverTime;

		// find out which index in the queue we want
		int cmdindex = client->pers.cmdhead - client->pers.latentCmds - 1;
		while ( cmdindex < 0 ) {
			cmdindex += MAX_LATENT_CMDS;
		}

		// read in the old command
		client->pers.cmd = client->pers.cmdqueue[cmdindex];

		// adjust the real ping to reflect the new latency
		client->pers.realPing += time - ucmd->serverTime;
	}
//unlagged - lag simulation #2


//unlagged - backward reconciliation #4
	// save the command time *before* pmove_fixed messes with the serverTime,
	// and *after* lag simulation messes with it :)
	// attackTime will be used for backward reconciliation later (time shift)
	client->attackTime = ucmd->serverTime;
//unlagged - backward reconciliation #4


//unlagged - smooth clients #1
	// keep track of this for later - we'll use this to decide whether or not
	// to send extrapolated positions for this client
	client->lastUpdateFrame = level.framenum;
//unlagged - smooth clients #1


//unlagged - lag simulation #1
	// if the client is adding latency to received snapshots (server-to-client latency)
	if ( client->pers.latentSnaps ) {
		// adjust the real ping
		client->pers.realPing += client->pers.latentSnaps * (50);
		// adjust the attack time so backward reconciliation will work
		client->attackTime -= client->pers.latentSnaps * (50);
	}
//unlagged - lag simulation #1


//unlagged - true ping
	// make sure the true ping is over 0 - with cl_timenudge it can be less
	if ( client->pers.realPing < 0 ) {
		client->pers.realPing = 0;
	}
//unlagged - true ping
	}
	
	
	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		return;
	}

	// spectators don't do much
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	if (ent && ent->client && (ent->client->ps.eFlags & EF_INVULNERABLE))
	{
		if (ent->client->invulnerableTimer <= level.time)
		{
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
		}
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) {
		return;
	}

	// clear the rewards if time
	if ( level.time > client->rewardTime ) {
		client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
	}
	
	if (ent->client->sess.veh_isactive == 1)
	{
		if (ent->client->sess.veh_movetype == 2)
		{
			ent->client->sess.jetdelayusefix = level.time + 800;
			ent->client->ps.eFlags |= EF_JETPACK_ACTIVE;
			ent->client->sess.jetfuel = 100;
				if ( ent->client->ps.fd.forcePower < 10 )
				{
					ent->client->ps.fd.forcePower = 10;
				}
			ent->client->sess.jetshowfuel = 0;
		}
	}

	if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.eFlags & EF_DISINTEGRATION ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		client->ps.pm_type = PM_DEAD;
	} else 
		{
		if (client->ps.forceGripChangeMovetype)
		{
			client->ps.pm_type = client->ps.forceGripChangeMovetype;
		}
		else if ( ( client->sess.clanTag ) && ( client->sess.adminloggedin == 0 ) )
		{
				client->ps.pm_type = PM_FREEZE;
		}
		// Twimod Jetpack
		else if ( ent->client->ps.eFlags & EF_JETPACK_ACTIVE )
		{
			// You are being knocked down, stop the jetpack!
			if ( ent->client->ps.forceHandExtend != HANDEXTEND_NONE || ent->health <= 0 )
			{
				ent->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
				client->ps.pm_type = PM_NORMAL;
			}

			// Drain force powers as fuel for the jetpack!
			else if (/* ent->client->ps.fd.forcePower &&*/ ent->client->JetPackTime < level.time )
			{
				ent->client->JetPackTime = level.time + 200;
				ent->client->ps.fd.forcePower -= mc_jetpack_forcedrain.integer;
				ent->client->sess.jetfuel -= mc_jetpack_fueldrain.integer;

				if ( ent->client->ps.fd.forcePower < 0 )
				{
					ent->client->ps.fd.forcePower = 0;
				}
				if ( ent->client->sess.jetfuel < 0 )
				{
					ent->client->sess.jetfuel = 0;
				}
			}
			// Sad - its burned out! Toggle off and drop to floor! :x
			else if ( (!ent->client->ps.fd.forcePower) && (mc_jetpack_forcedrain.integer != 0) )
			{
				ent->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
				client->ps.pm_type = PM_NORMAL;
			}
			else if ( ent->client->sess.jetfuel == 0 )
			{
				ent->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
				client->ps.pm_type = PM_NORMAL;
			}
		}
			if (client->sess.sleep)
			{ // Deathspike: Keep updating the knockdown time of sleep
				client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
				client->ps.forceHandExtendTime = level.time + 60000;
				client->ps.forceDodgeAnim = 0;
			}
			
			if (client->sess.punish)
			{
				client->ps.pm_type = PM_FLOAT;
			}
			
			else if ((client->sess.freeze && !client->sess.sleep) || (client->sess.clanTag && !client->sess.adminloggedin))
			{
				// Deathspike: Make me Freeze
				client->ps.pm_type = PM_FREEZE;
			}

			else
			{
				client->ps.pm_type = PM_NORMAL;
			}
		// /Twimod Jetpack

	}

	client->ps.gravity = g_gravity.value+client->sess.mcgravity;

	// set speed
	if ( ent->client->ps.eFlags & EF_JETPACK_ACTIVE )
	{
		/*
		if (level.jetfx == 1)
		{
			if (ent->client->sess.veh_isactive != 1)
			{
			vec3_t	fxorg;
			fxorg[0] = client->ps.origin[0] + client->ps.velocity[0]*level.jetdist;
			fxorg[1] = client->ps.origin[1] + client->ps.velocity[1]*level.jetdist;
			fxorg[2] = client->ps.origin[2] + client->ps.velocity[2]*level.jetdist;
			G_PlayEffect_ID(level.jetpack_effect, fxorg, newangle);
			//G_PlayEffect_ID(level.jetpack_effect, client->ps.origin, newangle);
			}
		}
		*/
		client->ps.gravity = 1;
		client->ps.speed = g_speed.value+client->sess.mcspeed+client->sess.jetspeed;
		client->ps.basespeed = g_speed.value+client->sess.mcspeed+client->sess.jetspeed;
	}
	else
	{
		client->ps.speed = g_speed.value+client->sess.mcspeed;
		client->ps.basespeed = g_speed.value+client->sess.mcspeed;
	}

	if (ent->client->sess.veh_isactive == 1)
	{
		client->ps.speed += ent->client->sess.veh_speed;
		client->ps.basespeed += ent->client->sess.veh_speed;
	}
	
	if (ent->client->sess.parachute == 1)
	{
		if (ent->client->ps.velocity[2] < -500)
		{
			ent->client->ps.velocity[2] = -500;
			client->ps.speed += 250;
			client->ps.basespeed += 250;
		}
	}
	if (ent->client->ps.duelInProgress)
	{
		gentity_t *duelAgainst = &g_entities[ent->client->ps.duelIndex];

		//Keep the time updated, so once this duel ends this player can't engage in a duel for another
		//10 seconds. This will give other people a chance to engage in duels in case this player wants
		//to engage again right after he's done fighting and someone else is waiting.
		ent->client->ps.fd.privateDuelTime = level.time + 10000;

		if (ent->client->ps.duelTime < level.time)
		{
			//Bring out the sabers
			if (ent->client->ps.weapon == WP_SABER && ent->client->ps.saberHolstered &&
				ent->client->ps.duelTime)
			{
				if (!saberOffSound || !saberOnSound)
				{
					saberOffSound = G_SoundIndex("sound/weapons/saber/saberoffquick.wav");
					saberOnSound = G_SoundIndex("sound/weapons/saber/saberon.wav");
				}

				//ent->client->ps.saberHolstered = qfalse;
				//G_Sound(ent, CHAN_AUTO, saberOnSound);

				G_AddEvent(ent, EV_PRIVATE_DUEL, 2);
				if (ent->client->sess.duel_is_ff != 0)
				{
					G_Printf("FF Begin!\n");
				}
				ent->client->ps.duelTime = 0;
			}

			if (duelAgainst && duelAgainst->client && duelAgainst->inuse &&
				duelAgainst->client->ps.weapon == WP_SABER && duelAgainst->client->ps.saberHolstered &&
				duelAgainst->client->ps.duelTime)
			{
				if (!saberOffSound || !saberOnSound)
				{
					saberOffSound = G_SoundIndex("sound/weapons/saber/saberoffquick.wav");
					saberOnSound = G_SoundIndex("sound/weapons/saber/saberon.wav");
				}
				//duelAgainst->client->ps.saberHolstered = qfalse;
				//G_Sound(duelAgainst, CHAN_AUTO, saberOnSound);

				G_AddEvent(duelAgainst, EV_PRIVATE_DUEL, 2);

				duelAgainst->client->ps.duelTime = 0;
			}
		}
		else
		{
			client->ps.speed = 0;
			client->ps.basespeed = 0;
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
		}

		if (!duelAgainst || !duelAgainst->client || !duelAgainst->inuse ||
			duelAgainst->client->ps.duelIndex != ent->s.number)
		{
			ent->client->ps.duelInProgress = 0;
			G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
		}
		else if (duelAgainst->health < 1 || duelAgainst->client->ps.stats[STAT_HEALTH] < 1)
		{
			ent->client->ps.duelInProgress = 0;
			duelAgainst->client->ps.duelInProgress = 0;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
			G_AddEvent(duelAgainst, EV_PRIVATE_DUEL, 0);

			//Winner gets full health.. providing he's still alive
			if (ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0)
			{


				if (g_spawnInvulnerability.integer)
				{
					ent->client->ps.eFlags |= EF_INVULNERABLE;
					ent->client->invulnerableTimer = level.time + g_spawnInvulnerability.integer;
				}
			}

			/*
			trap_SendServerCommand( ent-g_entities, va("print \"%s %s\n\"", ent->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELWINNER")) );
			trap_SendServerCommand( duelAgainst-g_entities, va("print \"%s %s\n\"", ent->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELWINNER")) );
			*/
			//Private duel announcements are now made globally because we only want one duel at a time.
			if (ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0)
			{
				trap_SendServerCommand( -1, va("print \"^7%s ^7%s ^7%s ^7with ^1%d ^7Health and ^2%d ^7Shield left.\n\"", ent->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELWINNER"), duelAgainst->client->pers.netname, ent->health, ent->client->ps.stats[STAT_ARMOR]) );
				mc_addcredits(ent, level.credit_get_duelwin);
				mc_addcredits(duelAgainst, -level.credit_loss_duellose);
			}
			else
			{ //it was a draw, because we both managed to die in the same frame
				trap_SendServerCommand( -1, va("print \"^7%s\n\"", G_GetStripEdString("SVINGAME", "PLDUELTIE")) );
			}
		}
		else
		{
			vec3_t vSub;
			float subLen = 0;

			if (twimod_dueldistance.integer != 0)
			{
				VectorSubtract(ent->client->ps.origin, duelAgainst->client->ps.origin, vSub);
				subLen = VectorLength(vSub);

				//if (subLen >= 1024)
				if (subLen >= twimod_dueldistance.integer)
				{ // Deathspike: So 0 is forever, 1024 is normal, 2048 is double range etc
					ent->client->ps.duelInProgress = 0;
					duelAgainst->client->ps.duelInProgress = 0;

					trap_SendServerCommand( -1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "PLDUELSTOP")) );
				}
			}
		}
	}

	/*
	if ( client->ps.powerups[PW_HASTE] ) {
		client->ps.speed *= 1.3;
	}
	*/

	if (client->ps.usingATST && ent->health > 0)
	{ //we have special shot clip boxes as an ATST
		ent->r.contents |= CONTENTS_NOSHOT;
		ATST_ManageDamageBoxes(ent);
	}
	else
	{
		ent->r.contents &= ~CONTENTS_NOSHOT;
		client->damageBoxHandle_Head = 0;
		client->damageBoxHandle_RLeg = 0;
		client->damageBoxHandle_LLeg = 0;
	}

	//rww - moved this stuff into the pmove code so that it's predicted properly
	//BG_AdjustClientSpeed(&client->ps, &client->pers.cmd, level.time);

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));

	if ( ent->flags & FL_FORCE_GESTURE ) {
		ent->flags &= ~FL_FORCE_GESTURE;
		ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
	}

	if (ent->client && ent->client->ps.fallingToDeath &&
		(level.time - FALL_FADE_TIME) > ent->client->ps.fallingToDeath)
	{ //die!
		player_die(ent, ent, ent, 100000, MOD_FALLING);
		respawn(ent);
		ent->client->ps.fallingToDeath = 0;

		G_MuteSound(ent->s.number, CHAN_VOICE); //stop screaming, because you are dead!
	}

	if (ent->client->ps.otherKillerTime > level.time &&
		ent->client->ps.groundEntityNum != ENTITYNUM_NONE &&
		ent->client->ps.otherKillerDebounceTime < level.time)
	{
		ent->client->ps.otherKillerTime = 0;
		ent->client->ps.otherKiller = ENTITYNUM_NONE;
	}
	else if (ent->client->ps.otherKillerTime > level.time &&
		ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
	{
		if (ent->client->ps.otherKillerDebounceTime < (level.time + 100))
		{
			ent->client->ps.otherKillerDebounceTime = level.time + 100;
		}
	}

//	WP_ForcePowersUpdate( ent, msec, ucmd); //update any active force powers
//	WP_SaberPositionUpdate(ent, ucmd); //check the server-side saber point, do apprioriate server-side actions (effects are cs-only)

	if ((ent->client->pers.cmd.buttons & BUTTON_USE) && ent->client->ps.useDelay < level.time)
	{
		TryUse(ent);
		ent->client->ps.useDelay = level.time + 100;
	}
	if (ent->client->sess.flying > 0)
	{
		client->ps.pm_type = PM_SPECTATOR;
		//client->ps.speed = 400;	// faster than normal
		//client->ps.basespeed = 400;
		//client->ps.speed = g_speed.value+client->sess.mcspeed;
		//client->ps.basespeed = g_speed.value+client->sess.mcspeed;
		if ((client->ps.events[ i & (MAX_PS_EVENTS-1) ] == EV_SABER_ATTACK)||(client->ps.events[ i & (MAX_PS_EVENTS-1) ] == EV_FIRE_WEAPON))
		{
			client->ps.speed *= 2;
			client->ps.basespeed *= 2;
		}
		if ((client->ps.events[ i & (MAX_PS_EVENTS-1) ] == EV_ALT_FIRE))
		{
			client->ps.speed *= 3;
			client->ps.basespeed *= 3;
		}
		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
	pm.trace = nox_trap_Trace;
		pm.pointcontents = trap_PointContents;

		pm.animations = NULL;

		// perform a pmove
		Pmove (&pm);
		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );
		VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
		trap_LinkEntity(ent);
		if ( ent->client->ps.eventSequence != oldEventSequence ) {
			ent->eventTime = level.time;
		}
		if (mc_unlagged.integer == 1)
		{
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
		}
		else
		{
		if (g_smoothClients.integer) {
			BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
		}
		else {
			BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
		}
		}
		SendPendingPredictableEvents( &ent->client->ps );
	}
	else
	{

	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( ent->r.svFlags & SVF_BOT ) {
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP;
	}
	else {
		pm.tracemask = MASK_PLAYERSOLID;
	}
	pm.trace = nox_trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.animations = bgGlobalAnimations;//NULL;

	pm.gametype = g_gametype.integer;

	VectorCopy( client->ps.origin, client->oldOrigin );


	// Twimod Jetpack
	/*
	if ( !ent->client->ps.duelInProgress
		&& ent->health >= 1
		&& !( client->ps.eFlags & EF_JETPACK_ACTIVE)
		&& !( client->ps.pm_flags & PMF_JUMP_HELD )
		&& client->ps.groundEntityNum == ENTITYNUM_NONE
		&& client->ps.fd.forcePower >= 2
		&& ( ent->client->sess.jetpackon == 1 )
		&& ucmd->upmove > 0 )
	{
		ent->client->ps.eFlags	|= EF_JETPACK_ACTIVE;
	}*/
	if ( client->ps.eFlags & EF_JETPACK_ACTIVE && ( ent->health <= 0 || ((client->ps.fd.forcePower <= 1)&&(mc_jetpack_forcedrain.integer != 0)) || ent->client->sess.jetfuel <= 0 ))
	{
		ent->client->ps.eFlags	&= ~EF_JETPACK_ACTIVE;
	}
	// /Twimod Jetpack


	if (level.intermissionQueued != 0 && g_singlePlayer.integer) {
		if ( level.time - level.intermissionQueued >= 1000  ) {
			pm.cmd.buttons = 0;
			pm.cmd.forwardmove = 0;
			pm.cmd.rightmove = 0;
			pm.cmd.upmove = 0;
			if ( level.time - level.intermissionQueued >= 2000 && level.time - level.intermissionQueued <= 2500 ) {
				trap_SendConsoleCommand( EXEC_APPEND, "centerview\n");
			}
			ent->client->ps.pm_type = PM_SPINTERMISSION;
		}
	}
	if (ent->client->sess.veh_isactive == 1 && ent->client->sess.veh_movetype == 1)
	{
		//G_Printf("%i\n", pm.cmd.upmove);
		if (pm.cmd.upmove > 5)
		{
			pm.cmd.upmove = 1;
		}
	}

	for ( i = 0 ; i < MAX_CLIENTS ; i++ )
	{
		if (g_entities[i].inuse && g_entities[i].client)
		{
			pm.bgClients[i] = &g_entities[i].client->ps;
		}
	}

	if (ent->client->ps.saberLockTime > level.time)
	{
		gentity_t *blockOpp = &g_entities[ent->client->ps.saberLockEnemy];

		if (blockOpp && blockOpp->inuse && blockOpp->client)
		{
			vec3_t lockDir, lockAng;

			//VectorClear( ent->client->ps.velocity );
			VectorSubtract( blockOpp->r.currentOrigin, ent->r.currentOrigin, lockDir );
			//lockAng[YAW] = vectoyaw( defDir );
			vectoangles(lockDir, lockAng);
			SetClientViewAngle( ent, lockAng );
		}

		if ( ( ent->client->buttons & BUTTON_ATTACK ) && ! ( ent->client->oldbuttons & BUTTON_ATTACK ) )
		{
			ent->client->ps.saberLockHits++;
		}
		if (ent->client->ps.saberLockHits > 2)
		{
			if (!ent->client->ps.saberLockAdvance)
			{
				ent->client->ps.saberLockHits -= 3;
			}
			ent->client->ps.saberLockAdvance = qtrue;
		}
	}
	else
	{
		ent->client->ps.saberLockFrame = 0;
		
		//check for taunt
		if ( (pm.cmd.generic_cmd == GENCMD_ENGAGE_DUEL) && (g_gametype.integer == GT_TOURNAMENT) )
		{//already in a duel, make it a taunt command
			pm.cmd.buttons |= BUTTON_GESTURE;
		}
	}
	
	if (ent->client->sess.veh_isactive == 1)
	{
		VectorSet(pm.mins, ent->client->sess.veh_xmin, ent->client->sess.veh_ymin, ent->client->sess.veh_zmin);
		VectorSet(pm.maxs, ent->client->sess.veh_xmax, ent->client->sess.veh_ymax, ent->client->sess.veh_zmax);
		VectorCopy(pm.mins, ent->r.mins);
		VectorCopy(pm.maxs, ent->r.maxs);
		trap_LinkEntity(ent);
	}

	Pmove (&pm);
	if (ent->client->sess.veh_isactive == 1)
	{
		VectorSet(pm.mins, ent->client->sess.veh_xmin, ent->client->sess.veh_ymin, ent->client->sess.veh_zmin);
		VectorSet(pm.maxs, ent->client->sess.veh_xmax, ent->client->sess.veh_ymax, ent->client->sess.veh_zmax);
		VectorCopy(pm.mins, ent->r.mins);
		VectorCopy(pm.maxs, ent->r.maxs);
		trap_LinkEntity(ent);
	}

	if (pm.checkDuelLoss)
	{
		if (pm.checkDuelLoss > 0 && pm.checkDuelLoss <= MAX_CLIENTS)
		{
			gentity_t *clientLost = &g_entities[pm.checkDuelLoss-1];

			if (clientLost && clientLost->inuse && clientLost->client && Q_irand(0, 40) > clientLost->health)
			{
				vec3_t attDir;
				VectorSubtract(ent->client->ps.origin, clientLost->client->ps.origin, attDir);
				VectorNormalize(attDir);

				VectorClear(clientLost->client->ps.velocity);
				clientLost->client->ps.forceHandExtend = HANDEXTEND_NONE;
				clientLost->client->ps.forceHandExtendTime = 0;

				gGAvoidDismember = 1;
				G_Damage(clientLost, ent, ent, attDir, clientLost->client->ps.origin, 9999, DAMAGE_NO_PROTECTION, MOD_SABER);

				if (clientLost->health < 1)
				{
					gGAvoidDismember = 2;
					G_CheckForDismemberment(clientLost, clientLost->client->ps.origin, 999, (clientLost->client->ps.legsAnim&~ANIM_TOGGLEBIT));
				}

				gGAvoidDismember = 0;
			}
		}

		pm.checkDuelLoss = 0;
	}

	switch(pm.cmd.generic_cmd)
	{
	case 0:
		break;
	case GENCMD_SABERSWITCH:
		if (ent->client->sess.allowToggle)
		{
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
			Cmd_ToggleSaber_f(ent);
			ent->client->ps.rocketLockTime = 0;
		}
		break;
	case GENCMD_ENGAGE_DUEL:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		if ( g_gametype.integer == GT_TOURNAMENT )
		{//already in a duel, made it a taunt command
		}
		else
		{
			ent->client->ps.rocketLockTime = 0;
			Cmd_EngageDuel_f(ent, 0);
		}
		break;
	case GENCMD_FORCE_HEAL:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceHeal(ent);
			ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_SPEED:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceSpeed(ent, 0);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_THROW:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		//G_Printf("THROOOOWWW\n");
		ForceThrow(ent, qfalse);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_PULL:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceThrow(ent, qtrue);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_DISTRACT:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceTelepathy(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_RAGE:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceRage(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_PROTECT:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceProtect(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_ABSORB:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceAbsorb(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_HEALOTHER:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceTeamHeal(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_FORCEPOWEROTHER:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockforce == 1)
			{
				break;
			}
		ForceTeamForceReplenish(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_FORCE_SEEING:
		ForceSeeing(ent);
					ent->client->ps.rocketLockTime = 0;
		break;
	case GENCMD_USE_SEEKER:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockweapon == 1)
			{
				break;
			}
		if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_SEEKER)) &&
			G_ItemUsable(&ent->client->ps, HI_SEEKER) )
		{
			ItemUse_Seeker(ent);
			G_AddEvent(ent, EV_USE_ITEM0+HI_SEEKER, 0);
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER);
		}
		break;
	case GENCMD_USE_FIELD:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockweapon == 1)
			{
				break;
			}
		if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_SHIELD)) &&
			G_ItemUsable(&ent->client->ps, HI_SHIELD) )
		{
			ItemUse_Shield(ent);
			G_AddEvent(ent, EV_USE_ITEM0+HI_SHIELD, 0);
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SHIELD);
		}
		break;
	case GENCMD_USE_BACTA:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockweapon == 1)
			{
				break;
			}
		if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_MEDPAC)) &&
			G_ItemUsable(&ent->client->ps, HI_MEDPAC) )
		{
			ItemUse_MedPack(ent);
			G_AddEvent(ent, EV_USE_ITEM0+HI_MEDPAC, 0);
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_MEDPAC);
		}
		break;
	case GENCMD_USE_ELECTROBINOCULARS:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockweapon == 1)
			{
				break;
			}
		if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_BINOCULARS)) &&
			G_ItemUsable(&ent->client->ps, HI_BINOCULARS) )
		{
			ItemUse_Binoculars(ent);
			if (ent->client->ps.zoomMode == 0)
			{
				G_AddEvent(ent, EV_USE_ITEM0+HI_BINOCULARS, 1);
			}
			else
			{
				G_AddEvent(ent, EV_USE_ITEM0+HI_BINOCULARS, 2);
			}
		}
		break;
	case GENCMD_ZOOM:
			if (ent->client->sess.veh_isactive == 1 || ent->client->sess.blockweapon == 1)
			{
				break;
			}
		if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_BINOCULARS)) &&
			G_ItemUsable(&ent->client->ps, HI_BINOCULARS) )
		{
			ItemUse_Binoculars(ent);
			if (ent->client->ps.zoomMode == 0)
			{
				G_AddEvent(ent, EV_USE_ITEM0+HI_BINOCULARS, 1);
			}
			else
			{
				G_AddEvent(ent, EV_USE_ITEM0+HI_BINOCULARS, 2);
			}
		}
		break;
	case GENCMD_USE_SENTRY:
		if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_SENTRY_GUN)))
		{
		//if G_ItemUsable(&ent->client->ps, HI_SENTRY_GUN) )
		if (ent->client->sess.sentries < mc_sentrylimit.integer)
		{
			ItemUse_Sentry(ent);
			G_AddEvent(ent, EV_USE_ITEM0+HI_SENTRY_GUN, 0);
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SENTRY_GUN);
		}
		}
		break;
	case GENCMD_SABERATTACKCYCLE:
		Cmd_SaberAttackCycle_f(ent);
		break;
	default:
		break;
	}

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	SendPendingPredictableEvents( &ent->client->ps );






	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);
	//VectorCopy (ent->r.mins, pm.mins);
	//VectorCopy (ent->r.maxs, pm.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// execute client events
	ClientEvents( ent, oldEventSequence );

	if ( pm.useEvent )
	{
		//TODO: Use
//		TryUse( ent );
	}

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	//test for solid areas in the AAS file
//	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// Did we kick someone in our pmove sequence?
	if (client->ps.forceKickFlip)
	{
		gentity_t *faceKicked = &g_entities[client->ps.forceKickFlip-1];

		if (faceKicked->client->sess.protect 
			|| ent->client->sess.protect
			|| (faceKicked->client->sess.sleep)
			|| ((faceKicked->client->chatGod || ent->client->chatGod) && !faceKicked->client->ps.duelInProgress))
			faceKicked = NULL;
		// End Kicking Exceptions

		if (faceKicked && faceKicked->client && (!OnSameTeam(ent, faceKicked) || g_friendlyFire.integer) &&
			(!faceKicked->client->ps.duelInProgress || faceKicked->client->ps.duelIndex == ent->s.number) &&
			(!ent->client->ps.duelInProgress || ent->client->ps.duelIndex == faceKicked->s.number))

		{
			if ( faceKicked && faceKicked->client && faceKicked->health && faceKicked->takedamage )
			{//push them away and do pain
				vec3_t oppDir;
				int strength = (int)VectorNormalize2( client->ps.velocity, oppDir );

				strength *= 0.05;

				VectorScale( oppDir, -1, oppDir );

				G_Damage( faceKicked, ent, ent, oppDir, client->ps.origin, strength, DAMAGE_NO_ARMOR, MOD_MELEE );

				if ( faceKicked->client->ps.weapon != WP_SABER ||
					 faceKicked->client->ps.fd.saberAnimLevel < FORCE_LEVEL_3 ||
					 (!BG_SaberInAttack(faceKicked->client->ps.saberMove) && !PM_SaberInStart(faceKicked->client->ps.saberMove) && !PM_SaberInReturn(faceKicked->client->ps.saberMove) && !PM_SaberInTransition(faceKicked->client->ps.saberMove)) )
				{
					if (faceKicked->health > 0 &&
						faceKicked->client->ps.stats[STAT_HEALTH] > 0 &&
						faceKicked->client->ps.forceHandExtend != HANDEXTEND_KNOCKDOWN)
					{
						if (Q_irand(1, 10) <= 3)
						{ //only actually knock over sometimes, but always do velocity hit
							faceKicked->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
							faceKicked->client->ps.forceHandExtendTime = level.time + 1100;
							faceKicked->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
						}

						faceKicked->client->ps.otherKiller = ent->s.number;
						faceKicked->client->ps.otherKillerTime = level.time + 5000;
						faceKicked->client->ps.otherKillerDebounceTime = level.time + 100;

						faceKicked->client->ps.velocity[0] = oppDir[0]*(strength*40);
						faceKicked->client->ps.velocity[1] = oppDir[1]*(strength*40);
						faceKicked->client->ps.velocity[2] = 200;
					}
				}

				G_Sound( faceKicked, CHAN_AUTO, G_SoundIndex( va("sound/weapons/melee/punch%d", Q_irand(1, 4)) ) );
			}
		}

		client->ps.forceKickFlip = 0;
	}

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime && !gDoSlowMoDuel ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ( g_forcerespawn.integer > 0 && 
				( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 ) {
				respawn( ent );
				return;
			}
		
			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) ) {
				respawn( ent );
			}
		}
		else if (gDoSlowMoDuel)
		{
			client->respawnTime = level.time + 1000;
		}
		return;
	}
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );
	if (ent->client->sess.movebacktime < level.time)
	{
	if ((ent->client->sess.movebackX != 0)||(ent->client->sess.movebackY != 0)||(ent->client->sess.movebackZ != 0))
	{
		ent->client->ps.origin[0] = ent->client->sess.movebackX;
		ent->client->ps.origin[1] = ent->client->sess.movebackY;
		ent->client->ps.origin[1] = ent->client->sess.movebackZ;
		VectorCopy( ent->client->ps.origin,ent->r.currentOrigin );
		ent->client->sess.movebackX = 0;
		ent->client->sess.movebackY = 0;
		ent->client->sess.movebackZ = 0;
	}
	}
	G_UpdateClientBroadcasts ( ent );
	/*if (ent->client->sess.forcegod == 1)
	{
		if (ucmd->upmove > 0)
		{
			if (ent->client->sess.amjump == 0)
			{
					ent->client->ps.velocity[2] += 1600;
					ent->client->sess.amjump = 1;
					client->ps.fd.forceJumpCharge = 1;
			}
		}
	}
	if ( WP_ForcePowerUsable( ent, FP_LEVITATION ) )
	{
		if ( ent->s.groundEntityNum != ENTITYNUM_NONE )
		{
			ent->client->sess.amjump = 0;
		}
	}*/
}

/*
==================
G_CheckClientTimeouts

Checks whether a client has exceded any timeouts and act accordingly
==================
*/
void G_CheckClientTimeouts ( gentity_t *ent )
{
	// Only timeout supported right now is the timeout to spectator mode
	if ( !g_timeouttospec.integer )
	{
		return;
	}

	// Already a spectator, no need to boot them to spectator
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		return;
	}

	// See how long its been since a command was received by the client and if its 
	// longer than the timeout to spectator then force this client into spectator mode
	if ( level.time - ent->client->pers.cmd.serverTime > g_timeouttospec.integer * 1000 )
	{
		SetTeam ( ent, "spectator" );
	}
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;

	ent = g_entities + clientNum;
	if (!(ent->client->sess.fakelag != 0))
	{
		if (ent->client->sess.controller > 0)
		{
			gentity_t	*flent = &g_entities[ent->client->sess.controller-1];
			if (!(flent && flent->inuse && flent->client))
			{
				ent->client->sess.controller = 0;
				G_Printf("Controller disconnected.\n");
				trap_GetUsercmd( clientNum, &ent->client->pers.cmd );
				goto endcontrol;
			}
			if (!(flent->client->sess.sessionTeam == TEAM_SPECTATOR && flent->client->sess.spectatorClient == ent->s.number))
			{
				ent->client->ps.viewangles[YAW] = flent->client->ps.viewangles[YAW];
				ent->client->ps.viewangles[PITCH] = flent->client->ps.viewangles[PITCH];
				SetClientViewAngle( ent, flent->client->ps.viewangles );
			}
			trap_GetUsercmd( ent->client->sess.controller-1, &ent->client->pers.cmd );
		}
		else
		{
			trap_GetUsercmd( clientNum, &ent->client->pers.cmd );
		}
		endcontrol:
		// mark the time we got info, so we can display the
		// phone jack if they don't get any for a while
		if (mc_unlagged.integer != 1)
		{
			ent->client->lastCmdTime = level.time;
		}

		if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
			ClientThink_real( ent );
		}
	}
}


void G_RunClient( gentity_t *ent ) {
	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t	*cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		int		clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				ent->client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients, qtrue );
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;
	clientPersistant_t	*pers;
	int frames;

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SpectatorClientEndFrame( ent );
		return;
	}

	pers = &ent->client->pers;

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ent->client->ps.powerups[ i ] < level.time ) {
			ent->client->ps.powerups[ i ] = 0;
		}
	}

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && (ent->client->ps.pm_type == PM_NORMAL || ent->client->ps.pm_type == PM_FLOAT) ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	if (mc_unlagged.integer != 1)
	{
	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound (ent);

	// set the latest infor
	if (mc_unlagged.integer == 1)
	{
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	else
	{
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	}
	SendPendingPredictableEvents( &ent->client->ps );

	if (mc_unlagged.integer == 1)
	{
//unlagged - smooth clients #1
	// mark as not missing updates initially
	ent->client->ps.eFlags &= ~EF_CONNECTION;

	// see how many frames the client has missed
	frames = level.framenum - ent->client->lastUpdateFrame - 1;

	// don't extrapolate more than two frames
	if ( frames > 2 ) {
		frames = 2;

		// if they missed more than two in a row, show the phone jack
		ent->client->ps.eFlags |= EF_CONNECTION;
		ent->s.eFlags |= EF_CONNECTION;
	}

	// did the client miss any frames?
	if ( frames > 0 && g_smoothClients.integer ) {
		// yep, missed one or more, so extrapolate the player's movement
		G_PredictPlayerMove( ent, (float)frames / 20 );
		// save network bandwidth
		SnapVector( ent->s.pos.trBase );
	}
//unlagged - smooth clients #1

//unlagged - backward reconciliation #1
	// store the client's position for backward reconciliation later
	G_StoreHistory( ent );
//unlagged - backward reconciliation #1
	}
	// set the bit for the reachability area the client is currently in
//	i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
//	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}



/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/



// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

// Rafael gameskill
int bg_pmove_gameskill_integer;
// done

// JPW NERVE -- added because I need to check single/multiplayer instances and branch accordingly
#ifdef CGAMEDLL
extern vmCvar_t cg_gameType;
extern vmCvar_t cg_jumptime;
extern vmCvar_t cg_realism;
#endif
#ifdef GAMEDLL
extern vmCvar_t g_gametype;
extern vmCvar_t g_jumptime;
extern vmCvar_t g_realism;
#endif

// jpw

pmove_t     *pm;
pml_t pml;

// movement parameters
float pm_stopspeed = 100;

//----(SA)	modified
float pm_waterSwimScale   = 0.50;
float pm_waterSprintSwimScale = 0.70;
float pm_waterWadeScale   = 0.70;
float pm_slagSwimScale    = 0.30;
float pm_slagWadeScale    = 0.70;

float pm_accelerate       = 10;
float pm_airaccelerate    = 1;
float pm_wateraccelerate  = 4;
float pm_slagaccelerate   = 2;
float pm_flyaccelerate    = 8;

float pm_friction         = 6;
float pm_waterfriction    = 1;
float pm_slagfriction     = 1;
float pm_flightfriction   = 3;
float pm_ladderfriction   = 14;
float pm_spectatorfriction = 5.0f;

float pm_realismSlowScale = 0.80;

//----(SA)	end

int c_pmove = 0;


/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

void PM_AddEventExt( int newEvent, int eventParm ) {
	BG_AddPredictableEventToPlayerstate( newEvent, eventParm, pm->ps );
}

int PM_IdleAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
		return WEAP_IDLE2;
	default:
		return WEAP_IDLE1;
	}
}

int PM_AltSwitchFromForWeapon( int weapon ) {
	switch ( weapon ) {
	default:
		return WEAP_ALTSWITCHFROM;
	}
}

int PM_AltSwitchToForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
	return WEAP_ALTSWITCHFROM;
	default:
		return WEAP_ALTSWITCHTO;
	}
}

int PM_AttackAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
		return WEAP_ATTACK2;
	default:
		return WEAP_ATTACK1;
	}
}

int PM_LastAttackAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
		return WEAP_ATTACK2;
		return WEAP_ATTACK1;
	default:
		return WEAP_ATTACK_LASTSHOT;
	}
}

int PM_ReloadAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
		return WEAP_RELOAD2;
		return WEAP_RELOAD3;
	default:
		return WEAP_RELOAD1;
	}
}

int PM_RaiseAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
		return WEAP_RELOAD3;
	default:
		return WEAP_RAISE;
	}
}

int PM_DropAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	case WP_M7:
		return WEAP_DROP2;
	default:
		return WEAP_DROP;
	}
}

int PM_SprintInAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	default:
		return WEAP_SPRINTIN;
	}
}

int PM_SprintOutAnimForWeapon( int weapon ) {
	switch ( weapon ) {
	default:
		return WEAP_SPRINTOUT;
	}
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
==============
PM_StartWeaponAnim
==============
*/
static void PM_StartWeaponAnim( int anim ) {
	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}

	if ( pm->ps->weapAnimTimer > 0 ) {
		return;
	}

	if ( pm->cmd.weapon == WP_NONE ) {
		return;
	}

	pm->ps->weapAnim = ( ( pm->ps->weapAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
}

static void PM_ContinueWeaponAnim( int anim ) {
	if ( pm->cmd.weapon == WP_NONE ) {
		return;
	}

	if ( ( pm->ps->weapAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->weapAnimTimer > 0 ) {
		return;     // a high priority animation is running
	}
	PM_StartWeaponAnim( anim );
}

/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float backoff;
	float change;
	int i;

	backoff = DotProduct( in, normal );

	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}
}

/*
========================
PM_ExertSound

plays random exertion sound when sprint key is press
========================
*/
static void PM_ExertSound( void ) {
	int rval;
	static int oldexerttime = 0;
	static int oldexertcnt = 0;

	if ( pm->cmd.serverTime > oldexerttime + 500 ) {
		oldexerttime = pm->cmd.serverTime;
	} else {
		return;
	}

	rval = rand() % 3;

	if ( oldexertcnt != rval ) {
		oldexertcnt = rval;
	} else {
		oldexertcnt++;
	}

	if ( oldexertcnt > 2 ) {
		oldexertcnt = 0;
	}

	if ( oldexertcnt == 1 ) {
		PM_AddEvent( EV_EXERT2 );
	} else if ( oldexertcnt == 2 ) {
		PM_AddEvent( EV_EXERT3 );
	} else {
		PM_AddEvent( EV_EXERT1 );
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t vec;
	float   *vel;
	float speed, newspeed, control;
	float drop;

	vel = pm->ps->velocity;

	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0; // ignore slope movement
	}

	speed = VectorLength( vec );
	if ( speed < 1 ) {
		vel[0] = 0;
		vel[1] = 0;     // allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !( pml.groundTrace.surfaceFlags & SURF_SLICK ) ) {
			// if getting knocked back, no friction
			if ( !( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control * pm_friction * pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		if ( pm->watertype & CONTENTS_SLIME ) { //----(SA)	slag
			drop += speed * pm_slagfriction * pm->waterlevel * pml.frametime;
		} else {
			drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
		}
	}

	// apply flying friction
	/*if ( pm->ps->powerups[PW_FLIGHT] ) {
		drop += speed * pm_flightfriction * pml.frametime;
	}*/

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		drop += speed * pm_spectatorfriction * pml.frametime;
	}

	// apply ladder strafe friction
	if ( pml.ladder ) {
		drop += speed * pm_ladderfriction * pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if ( newspeed < 0 ) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#if 1
	// q2 style
	int i;
	float addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct( pm->ps->velocity, wishdir );
	addspeed = wishspeed - currentspeed;
	if ( addspeed <= 0 ) {
		return;
	}
	accelspeed = accel * pml.frametime * wishspeed;
	if ( accelspeed > addspeed ) {
		accelspeed = addspeed;
	}

	// Ridah, variable friction for AI's
	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		accelspeed *= ( 1.0 / pm->ps->friction );
	}
	if ( accelspeed > addspeed ) {
		accelspeed = addspeed;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		pm->ps->velocity[i] += accelspeed * wishdir[i];
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t wishVelocity;
	vec3_t pushDir;
	float pushLen;
	float canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel * pml.frametime * wishspeed;
	if ( canPush > pushLen ) {
		canPush = pushLen;
	}

	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
	int max;
	float total;
	float scale;

	if ( pm->ps->aiChar && !( pm->ps->eFlags & EF_DUMMY_PMOVE ) ) {
		// restrict AI character movements (don't strafe or run backwards as fast as they can run forwards)
		if ( cmd->forwardmove < -64.0 ) {
			cmd->forwardmove = -64.0;
		}
		if ( cmd->rightmove > 64.0 ) {
			cmd->rightmove = 64.0;
		} else if ( cmd->rightmove < -64.0 ) {
			cmd->rightmove = -64.0;
		}
	}

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}

	total = sqrt( cmd->forwardmove * cmd->forwardmove
				  + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
	scale = (float)pm->ps->speed * max / ( 127.0 * total );

	switch ( pm->ps->aiChar ) {
		case AICHAR_ZOMBIE:
		case AICHAR_WARZOMBIE:
			 scale *= 1.1;
			 break;
		case AICHAR_ELITEGUARD:
		     scale *= 1.1;
			 break;
		case AICHAR_XSHEPHERD:
		     scale *= 1.4;
			 break;
		case AICHAR_HEINRICH:
		     scale *= 1.3;
			 break;
		case AICHAR_SUPERSOLDIER:
		case AICHAR_SUPERSOLDIER_LAB:
		     scale *= 1.3;
			 break;
		case AICHAR_HELGA:
		     scale *= 1.3;
			 break;
		case AICHAR_ZOMBIE_SURV:
		case AICHAR_ZOMBIE_FLAME:
		     scale *= 1.1;
			 break;
		case AICHAR_ZOMBIE_GHOST:
		     scale *= 1.3;
			 break;
		default:
		    scale *= 1.0;
		}

	if ( pm->cmd.buttons & BUTTON_SPRINT && pm->ps->sprintTime > 50 ) {
		scale *= pm->ps->sprintSpeedScale;
	} else {
		scale *= pm->ps->runSpeedScale;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		scale *= 3;
	}

// JPW NERVE -- half move speed if heavy weapon is carried
// this is the counterstrike way of doing it -- ie you can switch to a non-heavy weapon and move at
// full speed.  not completely realistic (well, sure, you can run faster with the weapon strapped to your
// back than in carry position) but more fun to play.  If it doesn't play well this way we'll bog down the
// player if the own the weapon at all.
//
// added #ifdef for game/cgame to project so we can get correct g_gametype variable and only do this in
// multiplayer if necessary

	#ifdef GAMEDLL
	if ( ! (pm->ps->aiChar)) {
		if (g_realism.value) {
			scale *= (pm_realismSlowScale * GetWeaponTableData(pm->ps->weapon)->moveSpeed);
		} else {
			scale *= GetWeaponTableData(pm->ps->weapon)->moveSpeed;
		}
	}
	#endif
	#ifdef CGAMEDLL
	if ( ! (pm->ps->aiChar)) {
		if (cg_realism.value) {
			scale *= (pm_realismSlowScale * GetWeaponTableData(pm->ps->weapon)->moveSpeed);
		} else {
		    scale *= GetWeaponTableData(pm->ps->weapon)->moveSpeed;
		}
	}
	#endif

	return scale;

}


/*
================
PM_SetMovementDir

Determine the rotation of the legs relative
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
// Ridah, changed this for more realistic angles (at the cost of more network traffic?)
#if 1
	float speed;
	vec3_t moved;
	int moveyaw;

	VectorSubtract( pm->ps->origin, pml.previous_origin, moved );

	if (    ( pm->cmd.forwardmove || pm->cmd.rightmove )
			&&  ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
			&&  ( speed = VectorLength( moved ) )
			&&  ( speed > pml.frametime * 5 ) ) { // if moving slower than 20 units per second, just face head angles
		vec3_t dir;

		VectorNormalize2( moved, dir );
		vectoangles( dir, dir );

		moveyaw = (int)AngleDelta( dir[YAW], pm->ps->viewangles[YAW] );

		if ( pm->cmd.forwardmove < 0 ) {
			moveyaw = (int)AngleNormalize180( moveyaw + 180 );
		}

		if ( abs( moveyaw ) > 75 ) {
			if ( moveyaw > 0 ) {
				moveyaw = 75;
			} else
			{
				moveyaw = -75;
			}
		}

		pm->ps->movementDir = (signed char)moveyaw;
	} else
	{
		pm->ps->movementDir = 0;
	}
#else
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 0;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 2;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 3;
		} else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 4;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 5;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 6;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->ps->movementDir == 6 ) {
			pm->ps->movementDir = 7;
		}
	}
#endif
}


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
	// JPW NERVE -- jumping in multiplayer uses and requires sprint juice (to prevent turbo skating, sprint + jumps)
	// don't allow jump accel
//	if (pm->cmd.serverTime - pm->ps->jumpTime < 850)
	
	// JPW NERVE -- in multiplayer, don't allow panzerfaust or dynamite to fire if charge bar isn't full
	int jumptime = 0;
	#ifdef GAMEDLL
		if (g_jumptime.value) {
			jumptime = 550;
		} else {
			jumptime = 850;
		}
		if ( pm->cmd.serverTime - pm->ps->jumpTime < jumptime ) {  // RealRTCW removed bunnyhop try 850 instead of 950
			return qfalse;
		}
	#endif
	#ifdef CGAMEDLL
		if (cg_jumptime.value) {
			jumptime = 550;
		} else {
			jumptime = 850;
		}
		if ( pm->cmd.serverTime - pm->ps->jumpTime < jumptime ) {  // RealRTCW removed bunnyhop try 850 instead of 950
			return qfalse;
		}
	#endif

		// don't allow if player tired
		//if (pm->ps->sprintTime < 2500) // JPW pulled this per id request; made airborne jumpers wildly inaccurate with gunfire to compensate
		//	return qfalse;


	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;      // don't allow jump until all buttons are up
	}

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	pml.groundPlane = qfalse;       // jumping away
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;

	pm->ps->groundEntityNum = ENTITYNUM_NONE;

    // Below is a JUMP_VELOCITY definition cases. Define was removed completely.
	#ifdef GAMEDLL
	// Total stamina count is 20000
		if (g_realism.value) {
		   if ((pm->ps->sprintTime < 15000) && (pm->ps->sprintTime > 10000)) {
		                pm->ps->velocity[2] = 260;
		   } else if ((pm->ps->sprintTime < 10000) && (pm->ps->sprintTime > 5000)) {
		                pm->ps->velocity[2] = 250;
		   } else if ((pm->ps->sprintTime < 5000) && (pm->ps->sprintTime >= 0)) {
					    pm->ps->velocity[2] = 230;
		   } else { 
		                pm->ps->velocity[2] = 270; // basically first jump
		   }
		} else {
			            pm->ps->velocity[2] = 270; // no realism
		}
	#endif
	#ifdef CGAMEDLL
		if (cg_realism.value) {
		   if ((pm->ps->sprintTime < 15000) && (pm->ps->sprintTime > 10000)) {
		                pm->ps->velocity[2] = 260;
		   } else if ((pm->ps->sprintTime < 10000) && (pm->ps->sprintTime > 5000)) {
		                pm->ps->velocity[2] = 250;
		   } else if ((pm->ps->sprintTime < 5000) && (pm->ps->sprintTime >= 0)) {
					    pm->ps->velocity[2] = 230;
		   } else { 
		                pm->ps->velocity[2] = 270; // basically first jump
		   }
		} else {
			            pm->ps->velocity[2] = 270; // no realism
		}
	#endif

	if ( pm->ps->powerups[PW_FLIGHT] ) 
	{
		pm->ps->velocity[2] = 350;
	}

	PM_AddEvent( EV_JUMP );

	if ( pm->cmd.forwardmove >= 0 ) {
		BG_AnimScriptEvent( pm->ps, ANIM_ET_JUMP, qfalse, qtrue );
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	} else {
		BG_AnimScriptEvent( pm->ps, ANIM_ET_JUMPBK, qfalse, qtrue );
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}

	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean PM_CheckWaterJump( void ) {
	vec3_t spot;
	int cont;
	vec3_t flatforward;

	if ( pm->ps->pm_time ) {
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize( flatforward );

	VectorMA( pm->ps->origin, 30, flatforward, spot );
	spot[2] += 4;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( !( cont & CONTENTS_SOLID ) ) {
		return qfalse;
	}

	spot[2] += 16;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( cont & (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY) ) {
		return qfalse;
	}

	// jump out of water
	VectorScale( pml.forward, 200, pm->ps->velocity );
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
	// waterjump has no control, but falls

	PM_StepSlideMove( qtrue );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	if ( pm->ps->velocity[2] < 0 ) {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;
	float vel;

	if ( PM_CheckWaterJump() ) {
		PM_WaterJumpMove();
		return;
	}
#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if ( pm->ps->velocity[2] > -300 ) {
			if ( pm->watertype & CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else if ( pm->watertype & CONTENTS_SLIME ) {
				pm->ps->velocity[2] = 80;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;       // sink towards bottom
	} else {
		for ( i = 0 ; i < 3 ; i++ )
			wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	if ( pm->watertype & CONTENTS_SLIME ) {    //----(SA)	slag
		if ( wishspeed > pm->ps->speed * pm_slagSwimScale ) {
			wishspeed = pm->ps->speed * pm_slagSwimScale;
		}

		PM_Accelerate( wishdir, wishspeed, pm_slagaccelerate );
	} else {
		if ( wishspeed > pm->ps->speed * pm_waterSwimScale ) {
			wishspeed = pm->ps->speed * pm_waterSwimScale;
		}

		PM_Accelerate( wishdir, wishspeed, pm_wateraccelerate );
	}


	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength( pm->ps->velocity );
		// slide along the ground plane
		PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.plane.normal,
						 pm->ps->velocity, OVERCLIP );

		VectorNormalize( pm->ps->velocity );
		VectorScale( pm->ps->velocity, vel, pm->ps->velocity );
	}

	PM_SlideMove( qfalse );
}


/*
===================
PM_InvulnerabilityMove

Only with the invulnerability powerup
===================
*/
// TTimo: unused
/*
static void PM_InvulnerabilityMove( void ) {
	pm->cmd.forwardmove = 0;
	pm->cmd.rightmove = 0;
	pm->cmd.upmove = 0;
	VectorClear(pm->ps->velocity);
}
*/

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;

	// normal slowdown
	PM_Friction();

	if ( pm->ps->aiChar == AICHAR_NONE || pml.ladder ) {
		scale = PM_CmdScale( &pm->cmd );
	} else {
		// AI is allowed to fly freely
		scale = 1.0;
	}
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} else {
		for ( i = 0 ; i < 3 ; i++ ) {
			wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	PM_Accelerate( wishdir, wishspeed, pm_flyaccelerate );

	PM_StepSlideMove( qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	usercmd_t cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

// Ridah, moved this down, so we use the actual movement direction
	// set the movementDir so clients can rotate the legs for strafing
//	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize( pml.forward );
	VectorNormalize( pml.right );

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}
	wishvel[2] = 0;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate( wishdir, wishspeed, pm_airaccelerate );

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.plane.normal,
						 pm->ps->velocity, OVERCLIP );
	}

#if 0
	//ZOID:  If we are on the grapple, try stair-stepping
	//this allows a player to use the grapple to pull himself
	//over a ledge
	if ( pm->ps->pm_flags & PMF_GRAPPLE_PULL ) {
		PM_StepSlideMove( qtrue );
	} else {
		PM_SlideMove( qtrue );
	}
#endif

	PM_StepSlideMove( qtrue );

// Ridah, moved this down, so we use the actual movement direction
	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();
}

/*
===================
PM_GrappleMove

===================
*/
// TTimo: unused
/*
static void PM_GrappleMove( void ) {
// Ridah, removed this code since we don't have a grapple, and the grapplePoint was consuming valuable msg space
#if 0
	vec3_t vel, v;
	float vlen;

	VectorScale(pml.forward, -16, v);
	VectorAdd(pm->ps->grapplePoint, v, v);
	VectorSubtract(v, pm->ps->origin, vel);
	vlen = VectorLength(vel);
	VectorNormalize( vel );

	if (vlen <= 100)
		VectorScale(vel, 10 * vlen, vel);
	else
		VectorScale(vel, 800, vel);

	VectorCopy(vel, pm->ps->velocity);

	pml.groundPlane = qfalse;
#endif
}
*/

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
	int i, stamtake;            //----(SA)
	vec3_t wishvel, oldvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	usercmd_t cmd;
	float accelerate;
	float vel;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
		// begin swimming
		PM_WaterMove();
		return;
	}


	if ( PM_CheckJump() ) {
		// jumped away
		if ( pm->waterlevel > 1 ) {
			PM_WaterMove();
		} else {
			PM_AirMove();

				pm->ps->jumpTime = pm->cmd.serverTime;

#ifdef GAMEDLL
				if (pm->ps->perks[PERK_RUNNER])
				{
					stamtake = 0; // No stamina take if the player has the PERK_RUNNER perk
				}
				else if (g_realism.value)
				{
					stamtake = 3000;
				}
				else
				{
					stamtake = 1000;
				}
#endif
#ifdef CGAMEDLL
				if (pm->ps->perks[PERK_RUNNER])
				{
					stamtake = 0; // No stamina take if the player has the PERK_RUNNER perk
				}
				else if (cg_realism.value)
				{
					stamtake = 3000;
				}
				else
				{
					stamtake = 1000;
				}
#endif

				// take time from powerup before taking it from sprintTime
				if ( pm->ps->powerups[PW_NOFATIGUE] ) {
					if ( pm->ps->powerups[PW_NOFATIGUE] > stamtake ) {
						pm->ps->powerups[PW_NOFATIGUE] -= stamtake;
						if ( pm->ps->powerups[PW_NOFATIGUE] < 0 ) {
							pm->ps->powerups[PW_NOFATIGUE] = 0;
						}
						stamtake = 0;
					} else {
						// don't have that much bonus.  clear what you've got and take the remainder from regular stamina
						stamtake -= pm->ps->powerups[PW_NOFATIGUE];
						pm->ps->powerups[PW_NOFATIGUE] = 0;
					}
				}
				if ( stamtake ) {
					pm->ps->sprintTime -= stamtake;
					if ( pm->ps->sprintTime < 0 ) {
						pm->ps->sprintTime = 0;
					}
				}
			
// jpw
		}
		return;
	}

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

// Ridah, moved this down, so we use the actual movement direction
	// set the movementDir so clients can rotate the legs for strafing
//	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity( pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity( pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	//
	VectorNormalize( pml.forward );
	VectorNormalize( pml.right );

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		if ( wishspeed > pm->ps->speed * pm->ps->crouchSpeedScale ) {
			wishspeed = pm->ps->speed * pm->ps->crouchSpeedScale;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel ) {
		float waterScale;

		waterScale = pm->waterlevel / 3.0;
		if ( pm->watertype & CONTENTS_SLIME ) { //----(SA)	slag
			waterScale = 1.0 - ( 1.0 - pm_slagSwimScale ) * waterScale;
		} else {
			waterScale = 1.0 - ( 1.0 - pm_waterSwimScale ) * waterScale;
		}

		if ( wishspeed > pm->ps->speed * waterScale ) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		accelerate = pm_airaccelerate;
	} else if ( ( pm->ps->stats[STAT_HEALTH] <= 0 ) && pm->ps->aiChar && ( pml.groundTrace.surfaceFlags & SURF_MONSTERSLICK ) )    {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate( wishdir, wishspeed, accelerate );

	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else if ( ( pm->ps->stats[STAT_HEALTH] <= 0 ) && pm->ps->aiChar && ( pml.groundTrace.surfaceFlags & SURF_MONSTERSLICK ) )   {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else {
		// don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
	}

	// show breath when standing on 'snow' surfaces
	if ( pml.groundTrace.surfaceFlags & SURF_SNOW ) {
		pm->ps->eFlags |= EF_BREATH;
	} else {
		pm->ps->eFlags &= ~EF_BREATH;
	}

	if ( pm->ps->eFlags & EF_CIG ) {
		pm->ps->eFlags |= EF_BREATH;
	}


	vel = VectorLength( pm->ps->velocity );
	VectorCopy( pm->ps->velocity, oldvel );

	// slide along the ground plane
	PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.plane.normal,
					 pm->ps->velocity, OVERCLIP );

	// RF, only maintain speed if the direction is similar
	if ( DotProduct( pm->ps->velocity, oldvel ) > 0 ) {
		// don't decrease velocity when going up or down a slope
		VectorNormalize( pm->ps->velocity );
		VectorScale( pm->ps->velocity, vel, pm->ps->velocity );
	}

	// don't do anything if standing still
	if ( !pm->ps->velocity[0] && !pm->ps->velocity[1] ) {
		return;
	}

	PM_StepSlideMove( qfalse );

// Ridah, moved this down, so we use the actual movement direction
	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();
}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength( pm->ps->velocity );
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear( pm->ps->velocity );
	} else {
		VectorNormalize( pm->ps->velocity );
		VectorScale( pm->ps->velocity, forward, pm->ps->velocity );
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float speed, drop, friction, control, newspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength( pm->ps->velocity );
	if ( speed < 1 ) {
		VectorCopy( vec3_origin, pm->ps->velocity );
	} else
	{
		drop = 0;

		friction = pm_friction * 1.5; // extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if ( newspeed < 0 ) {
			newspeed = 0;
		}
		newspeed /= speed;

		VectorScale( pm->ps->velocity, newspeed, pm->ps->velocity );
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	for ( i = 0 ; i < 3 ; i++ )
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA( pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin );
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void ) {

	if ( pm->ps->aiChar == AICHAR_HEINRICH ) {
		return EV_FOOTSTEP;
	}

	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	// JOSEPH 9-16-99
	if ( pml.groundTrace.surfaceFlags & SURF_METAL ) {
		return EV_FOOTSTEP_METAL;
	}

	if ( pml.groundTrace.surfaceFlags & SURF_WOOD ) {
		return EV_FOOTSTEP_WOOD;
	}

	if ( pml.groundTrace.surfaceFlags & SURF_GRASS ) {
		return EV_FOOTSTEP_GRASS;
	}

	if ( pml.groundTrace.surfaceFlags & SURF_GRAVEL ) {
		return EV_FOOTSTEP_GRAVEL;
	}
	// END JOSEPH

	if ( pml.groundTrace.surfaceFlags & SURF_ROOF ) {
		return EV_FOOTSTEP_ROOF;
	}

	if ( pml.groundTrace.surfaceFlags & SURF_SNOW ) {
		return EV_FOOTSTEP_SNOW;
	}

//----(SA)	added
	if ( pml.groundTrace.surfaceFlags & SURF_CARPET ) {
		return EV_FOOTSTEP_CARPET;
	}
//----(SA)	end
	return EV_FOOTSTEP;
}



/*
==============
PM_AddFallEvent
==============
*/
void PM_AddFallEvent( int landing, int surfaceparms ) {
//	PM_AddEvent( landing );		// old way
	BG_AddPredictableEventToPlayerstate( landing, surfaceparms, pm->ps );
}

/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float delta;
	float dist;
	float vel, acc;
	float t;
	float a, b, c, den;

	// Ridah, only play this if coming down hard
	if ( !pm->ps->legsTimer ) {
		if ( pml.previous_velocity[2] < -220 ) {
			BG_AnimScriptEvent( pm->ps, ANIM_ET_LAND, qfalse, qtrue );
		}
	}

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 ) {
		return;
	}
	t = ( -b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
	delta = delta * delta * 0.0001;

	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 ) {
		return;
	}

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 ) {
		delta *= 0.25;
	}
	if ( pm->waterlevel == 1 ) {
		delta *= 0.5;
	}

	if ( delta < 1 ) {
		return;
	}

	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !( pml.groundTrace.surfaceFlags & SURF_NODAMAGE ) ) {
		if ( pm->debugLevel ) {
			Com_Printf( "delta: %5.2f\n", delta );
		}

//----(SA)	removed per DM
		// Rafael gameskill
//		if (bg_pmove_gameskill_integer == 1)
//		{
//			if (delta > 7)
//				delta = 8;
//		}
		// done
//----(SA)	end

		if ( delta > 77 ) {
			PM_AddFallEvent( EV_FALL_NDIE, pml.groundTrace.surfaceFlags );
		} else if ( delta > 67 ) {
			PM_AddFallEvent( EV_FALL_DMG_50, pml.groundTrace.surfaceFlags );
		} else if ( delta > 58 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddFallEvent( EV_FALL_DMG_25, pml.groundTrace.surfaceFlags );
			}
		} else if ( delta > 48 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddFallEvent( EV_FALL_DMG_15, pml.groundTrace.surfaceFlags );
			}
		} else if ( delta > 38.75 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddFallEvent( EV_FALL_DMG_10, pml.groundTrace.surfaceFlags );
			}
		} else if ( delta > 7 ) {
			PM_AddFallEvent( EV_FALL_SHORT, pml.groundTrace.surfaceFlags );
		} else {
			if ( !( pm->ps->pm_flags & PMF_DUCKED ) && !( pm->cmd.buttons & BUTTON_WALKING ) ) {  // quiet if crouching or walking
				PM_AddFallEvent( PM_FootstepForSurface(), pml.groundTrace.surfaceFlags );
			}
		}
	}

	// start footstep cycle over
	pm->ps->bobCycle = 0;
	pm->ps->footstepCount = 0;
}



/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
	int i, j, k;
	vec3_t point;

	if ( pm->debugLevel ) {
		Com_Printf( "%i:allsolid\n", c_pmove );
	}

	// jitter around
	for ( i = -1; i <= 1; i++ ) {
		for ( j = -1; j <= 1; j++ ) {
			for ( k = -1; k <= 1; k++ ) {
				VectorCopy( pm->ps->origin, point );
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace( trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
				if ( !trace->allsolid ) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25;

					pm->trace( trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
	trace_t trace;
	vec3_t point;

#define AI_STEPTEST_FALLDIST_PER_SEC    ( STEPSIZE + pm->ps->gravity * 0.35 )   // always allow them to fall down the minimum stepsize

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		// we just transitioned into freefall
		if ( pm->debugLevel ) {
			Com_Printf( "%i:lift\n", c_pmove );
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
		//
		// RF, try and keep AI's on the ground if walking down steps
		if ( pm->ps->aiChar ) {
			if ( trace.fraction < 1.0 ) {
				float falldist, xyspeed;
				vec3_t vel;
				falldist = Distance( pm->ps->origin, trace.endpos );
				VectorCopy( pm->ps->velocity, vel );
				vel[2] = 0;
				xyspeed = VectorLength( vel );
				if ( xyspeed > 120 && falldist * pml.frametime < ( xyspeed * pml.frametime + STEPSIZE ) ) {
					// put them on the ground
					VectorCopy( trace.endpos, pm->ps->origin );
					return;
				}
			}
		}
		//
		if ( trace.fraction == 1.0 && !( pm->ps->pm_flags & PMF_LADDER ) ) {
			if ( pm->cmd.forwardmove >= 0 ) {
				BG_AnimScriptEvent( pm->ps, ANIM_ET_JUMP, qfalse, qtrue );
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} else {
				BG_AnimScriptEvent( pm->ps, ANIM_ET_JUMPBK, qfalse, qtrue );
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t point;
	trace_t trace;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25;

	pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
	pml.groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		if ( !PM_CorrectAllSolid( &trace ) ) {
			return;
		}
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf( "%i:kickoff\n", c_pmove );
		}
		if ( !( pm->ps->pm_flags & PMF_LADDER ) ) {
			// go into jump animation
			if ( pm->cmd.forwardmove >= 0 ) {
				BG_AnimScriptEvent( pm->ps, ANIM_ET_JUMP, qfalse, qfalse );
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} else {
				BG_AnimScriptEvent( pm->ps, ANIM_ET_JUMPBK, qfalse, qfalse );
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		if ( pm->debugLevel ) {
			Com_Printf( "%i:steep\n", c_pmove );
		}
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if ( pm->ps->pm_flags & PMF_TIME_WATERJUMP ) {
		pm->ps->pm_flags &= ~( PMF_TIME_WATERJUMP | PMF_TIME_LAND );
		pm->ps->pm_time = 0;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf( "%i:Land\n", c_pmove );
		}

		PM_CrashLand();

		// don't do landing time if we were just going down a slope
		if ( pml.previous_velocity[2] < -200 ) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 250;
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t point;
	int cont;
	int sample1;
	int sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	// Ridah, modified this
	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + pm->ps->mins[2] + 1;
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm->ps->viewheight - pm->ps->mins[2];
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + pm->ps->mins[2] + sample1;
		cont = pm->pointcontents( point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + pm->ps->mins[2] + sample2;
			cont = pm->pointcontents( point, pm->ps->clientNum );
			if ( cont & MASK_WATER ) {
				pm->waterlevel = 3;
			}
		}
	}
	// done.

	// UNDERWATER
	BG_UpdateConditionValue( pm->ps->clientNum, ANIM_COND_UNDERWATER, ( pm->waterlevel > 1 ), qtrue );

}



/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck( void ) {
	trace_t trace;

	// Ridah, modified this for configurable bounding boxes
	pm->mins[0] = pm->ps->mins[0];
	pm->mins[1] = pm->ps->mins[1];

	pm->maxs[0] = pm->ps->maxs[0];
	pm->maxs[1] = pm->ps->maxs[1];

	pm->mins[2] = pm->ps->mins[2];

	if ( pm->ps->pm_type == PM_DEAD ) {
		pm->maxs[2] = pm->ps->maxs[2];          // NOTE: must set death bounding box in game code
		pm->ps->viewheight = pm->ps->deadViewHeight;
		return;
	}

	// RF, disable crouching while using MG42
	if ( pm->ps->eFlags & EF_MG42_ACTIVE ) {
		pm->maxs[2] = pm->ps->maxs[2];
		pm->ps->viewheight = pm->ps->standViewHeight;
		return;
	}

	if ( pm->cmd.upmove < 0 ) { // duck
		pm->ps->pm_flags |= PMF_DUCKED;
	} else
	{   // stand up if possible
		if ( pm->ps->pm_flags & PMF_DUCKED ) {
			// try to stand up
			pm->maxs[2] = pm->ps->maxs[2];
			pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
			if ( !trace.allsolid ) {
				pm->ps->pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		pm->maxs[2] = pm->ps->crouchMaxZ;
		pm->ps->viewheight = pm->ps->crouchViewHeight;
	} else
	{
		pm->maxs[2] = pm->ps->maxs[2];
		pm->ps->viewheight = pm->ps->standViewHeight;
	}
	// done.
}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) {
	float bobmove, animGap;
	int old;
	qboolean footstep;
	qboolean iswalking;
	int animResult = -1;

	if ( pm->ps->eFlags & EF_DEAD ) {
		return;
	}

	iswalking = qfalse;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
						+  pm->ps->velocity[1] * pm->ps->velocity[1] );

	// mg42, always idle
	if ( pm->ps->persistant[PERS_HWEAPON_USE] ) {
		BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_IDLE, qtrue );
		//
		return;
	}

	// swimming
	if ( pm->waterlevel > 1 ) {

		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_SWIMBK, qtrue );
		} else {
			BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_SWIM, qtrue );
		}

		return;
	}

	// in the air
	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		if ( pm->ps->pm_flags & PMF_LADDER ) {             // on ladder
			if ( pm->ps->velocity[2] >= 0 ) {
				BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_CLIMBUP, qtrue );
				//BG_PlayAnimName( pm->ps, "BOTH_CLIMB", ANIM_BP_BOTH, qfalse, qtrue, qfalse );
			} else if ( pm->ps->velocity[2] < 0 )     {
				BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_CLIMBDOWN, qtrue );
				//BG_PlayAnimName( pm->ps, "BOTH_CLIMB_DOWN", ANIM_BP_BOTH, qfalse, qtrue, qfalse );
			}
		}

		return;
	}

	// if not trying to move
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		if (  pm->xyspeed < 5 ) {
			pm->ps->bobCycle = 0;   // start at beginning of cycle again
			pm->ps->footstepCount = 0;
		}
		if ( pm->xyspeed > 120 ) {
			return; // continue what they were doing last frame, until we stop
		}
		if ( pm->ps->pm_flags & PMF_DUCKED ) {
			animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_IDLECR, qtrue );
		}
		if ( animResult < 0 ) {
			BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_IDLE, qtrue );
		}
		//
		return;
	}


	footstep = qfalse;

	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		bobmove = 0.5;  // ducked characters bob much faster
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_WALKCRBK, qtrue );
		} else {
			animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_WALKCR, qtrue );
		}
		// ducked characters never play footsteps
	} else if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4;  // faster speeds bob faster
			footstep = qtrue;
			// check for strafing
			if ( pm->cmd.rightmove && !pm->cmd.forwardmove ) {
				if ( pm->cmd.rightmove > 0 ) {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFERIGHT, qtrue );
				} else {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFELEFT, qtrue );
				}
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_RUNBK, qtrue );
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_WALKBK, qtrue );
			}
		} else {
			bobmove = 0.3;
			// check for strafing
			if ( pm->cmd.rightmove && !pm->cmd.forwardmove ) {
				if ( pm->cmd.rightmove > 0 ) {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFERIGHT, qtrue );
				} else {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFELEFT, qtrue );
				}
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_WALKBK, qtrue );
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_RUNBK, qtrue );
			}
		}

	} else {

		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4;  // faster speeds bob faster
			footstep = qtrue;
			// check for strafing
			if ( pm->cmd.rightmove && !pm->cmd.forwardmove ) {
				if ( pm->cmd.rightmove > 0 ) {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFERIGHT, qtrue );
				} else {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFELEFT, qtrue );
				}
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_RUN, qtrue );
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_WALK, qtrue );
			}
		} else {
			bobmove = 0.3;  // walking bobs slow
			if ( pm->ps->aiChar != AICHAR_NONE ) {
				footstep = qtrue;
				iswalking = qtrue;
			} else {
				footstep = qfalse;  // walking is quiet for the player
			}
			if ( pm->cmd.rightmove && !pm->cmd.forwardmove ) {
				if ( pm->cmd.rightmove > 0 ) {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFERIGHT, qtrue );
				} else {
					animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_STRAFELEFT, qtrue );
				}
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_WALK, qtrue );
			}
			if ( animResult < 0 ) {   // if we havent found an anim yet, play the run
				animResult = BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_RUN, qtrue );
			}
		}
	}

	// if no anim found yet, then just use the idle as default
	if ( animResult < 0 ) {
		BG_AnimScriptAnimation( pm->ps, pm->ps->aiState, ANIM_MT_IDLE, qtrue );
	}

	// check for footstep / splash sounds
	animGap = BG_AnimGetFootstepGap( pm->ps, pm->xyspeed );

	// new method
	if ( animGap > 0 ) {

		// do the bobCycle for weapon bobbing
		pm->ps->bobCycle = (int)( pm->ps->bobCycle + bobmove * pml.msec ) & 255;

		// now footsteps
	#ifdef GAMEDLL
	    if ( !pm->ps->aiChar ) {
		if (g_realism.value) {
			pm->ps->footstepCount += pm_realismSlowScale * (GetWeaponTableData(pm->ps->weapon)->moveSpeed * (pm->xyspeed * pml.frametime));
		} else {
			pm->ps->footstepCount += (GetWeaponTableData(pm->ps->weapon)->moveSpeed * (pm->xyspeed * pml.frametime));
		}
		} else {
		    pm->ps->footstepCount  += 1.0 * (pm->xyspeed * pml.frametime);
		}
	
	#endif
	#ifdef CGAMEDLL
		if ( !pm->ps->aiChar ) {
		if (cg_realism.value) {
			pm->ps->footstepCount += pm_realismSlowScale * (GetWeaponTableData(pm->ps->weapon)->moveSpeed * (pm->xyspeed * pml.frametime));
		} else {
		    pm->ps->footstepCount += (GetWeaponTableData(pm->ps->weapon)->moveSpeed * (pm->xyspeed * pml.frametime));
		}
		} else {
		    pm->ps->footstepCount  += 1.0 * (pm->xyspeed * pml.frametime);
		}
	#endif

		if ( pm->ps->footstepCount > animGap ) {

			pm->ps->footstepCount = pm->ps->footstepCount - animGap;

			if ( !iswalking && pm->ps->sprintExertTime && pm->waterlevel <= 2 ) {
				PM_ExertSound();
			}

			if ( pm->waterlevel == 0 ) {
				if ( footstep && !pm->noFootsteps ) {
					if ( pm->ps->aiChar == AICHAR_HEINRICH ) {
						PM_AddEvent( EV_FOOTSTEP );
					} else {
						PM_AddEvent( PM_FootstepForSurface() );
					}
				}
			} else if ( pm->waterlevel == 1 ) {
				// splashing
				PM_AddEvent( EV_FOOTSPLASH );
			} else if ( pm->waterlevel == 2 ) {
				// wading / swimming at surface
				PM_AddEvent( EV_SWIM );
			} else if ( pm->waterlevel == 3 ) {
				// no sound when completely underwater
			}

		}

	} else {    // default back to old method

		old = pm->ps->bobCycle;

		if ( pm->ps->aiChar == AICHAR_SUPERSOLDIER || pm->ps->aiChar == AICHAR_PROTOSOLDIER || pm->ps->aiChar == AICHAR_SUPERSOLDIER_LAB ) {
			//iswalking = qfalse;
			bobmove = 0.4 * 0.75f;  // slow down footsteps for big guys
		}

		if ( pm->ps->aiChar == AICHAR_HEINRICH ) {
			iswalking = qfalse;
			bobmove = 0.4 * 1.3f;
		}

		if ( pm->ps->aiChar == AICHAR_HELGA ) {
			iswalking = qfalse;
			bobmove = 0.4 * 1.5f;
		}

		pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

		// if we just crossed a cycle boundary, play an apropriate footstep event
		if ( iswalking /*|| (pm->ps->aiChar == AICHAR_HEINRICH)*/ ) {
			// sounds much more natural this way
			if ( old > pm->ps->bobCycle ) {

				if ( pm->waterlevel == 0 ) {
					if ( footstep && !pm->noFootsteps ) {
						if ( pm->ps->aiChar == AICHAR_HEINRICH ) {
							PM_AddEvent( EV_FOOTSTEP );
						} else {
							PM_AddEvent( PM_FootstepForSurface() );
						}
					}
				} else if ( pm->waterlevel == 1 ) {
					// splashing
					PM_AddEvent( EV_FOOTSPLASH );
				} else if ( pm->waterlevel == 2 ) {
					// wading / swimming at surface
					PM_AddEvent( EV_SWIM );
				} else if ( pm->waterlevel == 3 ) {
					// no sound when completely underwater
				}

			}
		} else if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 )   {

			if ( pm->ps->sprintExertTime && pm->waterlevel <= 2 ) {
				PM_ExertSound();
			}

			if ( pm->waterlevel == 0 ) {
				// on ground will only play sounds if running
				if ( footstep && !pm->noFootsteps ) {
					//				if (pm->ps->aiChar == AICHAR_HEINRICH) {	// <-- this stuff now handled in PM_footstepforsurf
					//					PM_AddEvent( EV_FOOTSTEP );
					//				} else {
					PM_AddEvent( PM_FootstepForSurface() );
					//				}
				}
			} else if ( pm->waterlevel == 1 ) {
				// splashing
				PM_AddEvent( EV_FOOTSPLASH );
			} else if ( pm->waterlevel == 2 ) {
				// wading / swimming at surface
				PM_AddEvent( EV_SWIM );
			} else if ( pm->waterlevel == 3 ) {
				// no sound when completely underwater

			}
		}

	}

}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {        // FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if ( !pml.previous_waterlevel && pm->waterlevel ) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if ( pml.previous_waterlevel && !pm->waterlevel ) {
		PM_AddEvent( EV_WATER_LEAVE );
	}

	//
	// check for head just going under water
	//
	if ( pml.previous_waterlevel != 3 && pm->waterlevel == 3 ) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if ( pml.previous_waterlevel == 3 && pm->waterlevel != 3 ) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}


/*
==============
PM_BeginWeaponReload
==============
*/
static void PM_BeginWeaponReload( int weapon ) {

	int reloadTime = ammoTable[weapon].reloadTime;
    int reloadTimeFull = ammoTable[weapon].reloadTimeFull;

	// only allow reload if the weapon isn't already occupied (firing is okay)
	if ( pm->ps->weaponstate != WEAPON_READY && pm->ps->weaponstate != WEAPON_FIRING && pm->ps->weaponstate != WEAPON_FIRINGALT ) {
		return;
	}

	if ( weapon < WP_KNIFE || weapon > WP_HOLYCROSS ) {
		return;
	}

	if((weapon == WP_M1GARAND) && pm->ps->ammoclip[WP_M1GARAND] != 0) {
			return;	
	}

	if (weapon == WP_M1941)
	{
		int maxclip = BG_GetMaxClip(pm->ps, WP_M1941);
		if (pm->ps->ammoclip[WP_M1941] > (0.5f * maxclip))
		{
			return;
		}
	}

	// no reload when you've got a chair in your hands
	if ( pm->ps->eFlags & EF_MELEE_ACTIVE ) {
		return;
	}
    
	// Jaymod
	if ( !pm->ps->aiChar) { 
	if (weapon == WP_M97) {
		PM_BeginM97Reload();
		return;
	}
	

	if (weapon == WP_AUTO5) {
		PM_BeginAuto5Reload();
		return;
	}
	}

	switch ( weapon ) {
	case WP_DYNAMITE:
	case WP_DYNAMITE_ENG:
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
		break;

	default:
		// DHM - Nerve :: override current animation (so reloading after firing will work)
		BG_AnimScriptEvent( pm->ps, ANIM_ET_RELOAD, qfalse, qtrue );
		break;
	}

	// If PERK_WEAPONHANDLING perk is active, reduce reloadTime by half
	if (pm->ps->perks[PERK_WEAPONHANDLING])
	{
		reloadTime *= 0.5;
		reloadTimeFull *= 0.5;
	}

	if (!pm->ps->aiChar)
	{
		if (pm->ps->ammoclip[BG_FindClipForWeapon(weapon)] == 0)
		{
			PM_ContinueWeaponAnim((pm->ps->perks[PERK_WEAPONHANDLING]) ? WEAP_RELOAD2_FAST : WEAP_RELOAD2);
			if (pm->ps->weaponstate == WEAPON_READY)
			{
				pm->ps->weaponTime += reloadTimeFull;
			}
			else if (pm->ps->weaponTime < reloadTimeFull)
			{
				pm->ps->weaponTime += (reloadTimeFull - pm->ps->weaponTime);
			}
			PM_AddEvent(EV_FILL_CLIP_FULL);
		}
		else
		{
			PM_ContinueWeaponAnim((pm->ps->perks[PERK_WEAPONHANDLING]) ? WEAP_RELOAD1_FAST : WEAP_RELOAD1);
			if (pm->ps->weaponstate == WEAPON_READY)
			{
				pm->ps->weaponTime += reloadTime;
			}
			else if (pm->ps->weaponTime < reloadTime)
			{
				pm->ps->weaponTime += (reloadTime - pm->ps->weaponTime);
			}
			PM_AddEvent(EV_FILL_CLIP);
		}
	}
	else
	{
		PM_ContinueWeaponAnim((pm->ps->perks[PERK_WEAPONHANDLING]) ? WEAP_RELOAD1_FAST : WEAP_RELOAD1);
		if (pm->ps->weaponstate == WEAPON_READY)
		{
			pm->ps->weaponTime += reloadTime;
		}
		else if (pm->ps->weaponTime < reloadTime)
		{
			pm->ps->weaponTime += (reloadTime - pm->ps->weaponTime);
		}
		PM_AddEvent(EV_FILL_CLIP_AI);
	}

	pm->ps->weaponstate = WEAPON_RELOADING;
}

static void PM_ReloadClip( int weapon );


/*
===============
PM_BeginWeaponChange
===============
*/
void PM_BeginWeaponChange( int oldweapon, int newweapon, qboolean reload ) { //----(SA)	modified to play 1st person alt-mode transition animations.
	int switchtime;
	qboolean altswitch, showdrop;
	qboolean altSwitchAnim = qfalse;

	if ( newweapon < WP_NONE || newweapon >= WP_NUM_WEAPONS ) {
		return;
	}

	if ( !pm->ps->aiChar && !( pm->ps->eFlags & EF_DEAD ) && ( newweapon == WP_NONE ) ) {   // RF, dont allow changing to null weapon
		return;
	}

	if ( newweapon != WP_NONE && !( COM_BitCheck( pm->ps->weapons, newweapon ) ) ) {
		return;
	}

	// Allow weapon switch even while reloading — interrupt the reload
	if (pm->ps->weaponstate == WEAPON_RELOADING)
	{
		PM_AddEvent(EV_STOP_RELOADING_SOUND);
		pm->ps->weaponstate = WEAPON_READY;
		pm->ps->weaponTime = 0;
	}

	// Still block switching during drop/holster/sprint states
	if (pm->ps->weaponstate == WEAPON_DROPPING || pm->ps->weaponstate == WEAPON_DROPPING_TORELOAD || pm->ps->weaponstate == WEAPON_HOLSTER_IN || pm->ps->weaponstate == WEAPON_SPRINT_IN)
	{
		return;
	}

	if ( pm->ps->grenadeTimeLeft > 0 ) {   // don't allow switch if you're holding a hot potato or dynamite
		return;
	}

	if ( !pm->ps->aiChar && !oldweapon ) {    // go straight to the new weapon
		pm->ps->weaponDelay = 0;
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_RAISING;
		pm->ps->weapon = newweapon;
		return;
	}

	altswitch = (qboolean)( newweapon == ammoTable[oldweapon].weapAlts );

	showdrop = qtrue;

	if ( oldweapon == WP_GRENADE_LAUNCHER ||
		 oldweapon == WP_GRENADE_PINEAPPLE ||
		 oldweapon == WP_DYNAMITE ||
		 oldweapon == WP_PANZERFAUST ||
		 oldweapon == WP_POISONGAS ) {
		if ( !pm->ps->ammoclip[oldweapon] ) {  // you're empty, don't show grenade '0'
			showdrop = qfalse;
		}
	}


	switch ( newweapon ) {

	case WP_MONSTER_ATTACK1:
	case WP_MONSTER_ATTACK2:
	case WP_MONSTER_ATTACK3:
		break;

	case WP_DYNAMITE:
	case WP_DYNAMITE_ENG:
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
	case WP_POISONGAS:
	case WP_KNIFE:
		pm->ps->grenadeTimeLeft = 0;        // initialize the timer on the potato you're switching to

	default:
		//----(SA)	only play the weapon switch sound for the player
		if ( !( pm->ps->aiChar ) ) {
			PM_AddEvent( EV_CHANGE_WEAPON );
		}

		if ( altswitch ) {  // it's an alt mode, play different anim
			PM_StartWeaponAnim( WEAP_ALTSWITCHFROM );
		} else {
			if ( showdrop ) {
				PM_StartWeaponAnim( WEAP_DROP );    // PM_ContinueWeaponAnim(WEAP_DROP);
			}
		}
	}

	//BG_AnimScriptEvent( pm->ps, ANIM_ET_DROPWEAPON, qfalse, qfalse );

	if ( reload ) {
		pm->ps->weaponstate = WEAPON_DROPPING_TORELOAD;
	} else {
		pm->ps->weaponstate = WEAPON_DROPPING;
	}

	// it's an alt mode, play different anim
	if ( newweapon == ammoTable[oldweapon].weapAlts ) {
		PM_StartWeaponAnim( PM_AltSwitchFromForWeapon( oldweapon ) );
	} else {
		PM_StartWeaponAnim( PM_DropAnimForWeapon( oldweapon ) );
	}

	switchtime = 250;   // dropping/raising usually takes 1/4 sec.
	// sometimes different switch times for alt weapons
	switch ( oldweapon ) {
	case WP_M1GARAND:
		if ( newweapon == ammoTable[oldweapon].weapAlts ) {
			switchtime = 0;
			if ( !pm->ps->ammoclip[newweapon] && pm->ps->ammo[newweapon] ) {
				PM_ReloadClip( newweapon );
			}
		}
		break;
	case WP_M7:
		if ( newweapon == ammoTable[oldweapon].weapAlts ) {
			switchtime = 0;
		}
		break;
	case WP_FG42:
	case WP_FG42SCOPE:
		if ( altswitch ) {
			switchtime = 50;        // fast
		}
		break;

	}

	// play an animation
	if ( altSwitchAnim ) {
		BG_AnimScriptEvent( pm->ps, ANIM_ET_UNDO_ALT_WEAPON_MODE, qfalse, qfalse );
	} else {
		BG_AnimScriptEvent( pm->ps, ANIM_ET_DROPWEAPON, qfalse, qfalse );
	}
	

	pm->ps->weaponTime += switchtime;
}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
	int oldweapon, newweapon, switchtime;
	qboolean altSwitchAnim = qfalse;
	qboolean doSwitchAnim = qtrue;

	newweapon = pm->cmd.weapon;
	if ( newweapon < WP_NONE || newweapon >= WP_NUM_WEAPONS ) {
		newweapon = WP_NONE;
	}

	if ( !( COM_BitCheck( pm->ps->weapons, newweapon ) ) ) {
		newweapon = WP_NONE;
	}

	oldweapon = pm->ps->weapon;

	pm->ps->weapon = newweapon;

	if ( pm->ps->weaponstate == WEAPON_DROPPING_TORELOAD ) {
		pm->ps->weaponstate = WEAPON_RAISING_TORELOAD;  //----(SA)	added
	} else {
		pm->ps->weaponstate = WEAPON_RAISING;
	}

	switch ( newweapon )
	{
		// don't really care about anim since these weapons don't show in view.
		// However, need to set the animspreadscale so they are initally at worst accuracy
	case WP_SNOOPERSCOPE:
	case WP_SNIPERRIFLE:
	case WP_FG42SCOPE:
	case WP_DELISLESCOPE:
	case WP_M1941SCOPE:
		pm->ps->aimSpreadScale = 255;               // initially at lowest accuracy
		pm->ps->aimSpreadScaleFloat = 255.0f;       // initially at lowest accuracy

	default:
		break;
	}

	// doesn't happen too often (player switched weapons away then back very quickly)
	if ( oldweapon == newweapon ) {
		return;
	}

	// dropping/raising usually takes 1/4 sec.
	switchtime = 250;

	// sometimes different switch times for alt weapons
	switch ( newweapon ) {
	case WP_FG42:
	case WP_FG42SCOPE:
		if ( newweapon == ammoTable[oldweapon].weapAlts ) {
			switchtime = 50;        // fast
		}
		break;
	case WP_M1GARAND:
		if ( newweapon == ammoTable[oldweapon].weapAlts ) {
			if ( pm->ps->ammoclip[ BG_FindAmmoForWeapon( oldweapon ) ] ) {
				switchtime = 1347;
			} else {
				switchtime = 0;
				doSwitchAnim = qfalse;
			}
			altSwitchAnim = qtrue ;
		}
		break;
	case WP_M7:
		if ( newweapon == ammoTable[oldweapon].weapAlts ) {
			switchtime = 2350;
			altSwitchAnim = qtrue ;
		}
		break;
	}

	pm->ps->weaponTime += switchtime;

	// make scripting aware of new weapon
	BG_UpdateConditionValue( pm->ps->clientNum, ANIM_COND_WEAPON, newweapon, qtrue );

	// play an animation
	if ( doSwitchAnim ) {
		if ( altSwitchAnim ) {
			BG_AnimScriptEvent(pm->ps,ANIM_ET_DO_ALT_WEAPON_MODE, qfalse, qfalse );
		} else {
			BG_AnimScriptEvent( pm->ps, ANIM_ET_RAISEWEAPON, qfalse, qfalse );
		}

		// alt weapon switch was played when switching away, just go into idle
		if ( ammoTable[oldweapon].weapAlts == newweapon ) {
			PM_StartWeaponAnim( PM_AltSwitchToForWeapon( newweapon ) );
		} else {
			PM_StartWeaponAnim( PM_RaiseAnimForWeapon( newweapon ) );
		}
	}

}


/*
==============
PM_ReloadClip
==============
*/
static void PM_ReloadClip(int weapon) {
	int clipIndex = BG_FindClipForWeapon(weapon);
	int ammoIndex = BG_FindAmmoForWeapon(weapon);

	int ammoreserve = pm->ps->ammo[ammoIndex];
	int ammoclip    = pm->ps->ammoclip[clipIndex];

	int maxclip = BG_GetMaxClip(pm->ps, weapon);
	int ammomove = maxclip - ammoclip;

	// Jaymod logic overrides
	if (!pm->ps->aiChar) {
		if (weapon == WP_M97 || weapon == WP_AUTO5) {
			ammomove = 1;
		}

		if (weapon == WP_M1941 && ammoclip > 0) {
			ammomove = 5;
		}
	}

	if (ammoreserve < ammomove) {
		ammomove = ammoreserve;
	}

	if (ammomove > 0) {
		pm->ps->ammo[ammoIndex]     -= ammomove;
		pm->ps->ammoclip[clipIndex] += ammomove;
	}

	// Reload secondary weapon if akimbo
	if (weapon == WP_AKIMBO) {
		PM_ReloadClip(WP_COLT);
	}
	if (weapon == WP_DUAL_TT33) {
		PM_ReloadClip(WP_TT33);
	}
}

/*
==============
PM_FinishWeaponReload
==============
*/

static void PM_FinishWeaponReload( void ) {

	// Jaymod Overrides for Shotgun
	if ( !pm->ps->aiChar) { 
	if( pm->ps->weapon == WP_M97 ) {
		PM_M97Reload();
		return;
	}

	if( pm->ps->weapon == WP_AUTO5 ) {
		PM_Auto5Reload();
		return;
	}
	}

	PM_ReloadClip( pm->ps->weapon );          // move ammo into clip
	pm->ps->weaponstate = WEAPON_READY;     // ready to fire
    //PM_StartWeaponAnim( PM_IdleAnimForWeapon( pm->ps->weapon ) );
}


static int isAutoReloadWeapon(int weapon) {
    int i;
    for (i = 0; i < sizeof(autoReloadWeapons) / sizeof(int); i++) {
        if (autoReloadWeapons[i] == weapon) {
            return 1;
        }
    }
    return 0;
}


/*
==============
PM_CheckforReload
==============
*/
void PM_CheckForReload(int weapon) {
	if (pm->noWeapClips) {
		return;
	}

	switch (weapon) {
		case WP_M7:
		case WP_FLAMETHROWER:
		case WP_KNIFE:
		case WP_GRENADE_LAUNCHER:
		case WP_GRENADE_PINEAPPLE:
		case WP_DYNAMITE:
		case WP_NONE:
		case WP_TESLA:
		case WP_HOLYCROSS:
			return;
		default:
			break;
	}

	qboolean reloadRequested = (pm->cmd.wbuttons & WBUTTON_RELOAD);
	qboolean autoreload = pm->pmext->bAutoReload || isAutoReloadWeapon(pm->ps->weapon);

	if (pm->ps->weaponstate == WEAPON_RAISING ||
		pm->ps->weaponstate == WEAPON_RAISING_TORELOAD ||
		pm->ps->weaponstate == WEAPON_DROPPING ||
		pm->ps->weaponstate == WEAPON_DROPPING_TORELOAD ||
		pm->ps->weaponstate == WEAPON_READYING ||
		pm->ps->weaponstate == WEAPON_RELAXING) {
		return;
	}

	if (pm->ps->weaponstate == WEAPON_RELOADING) {
		// Jaymod: interrupt reload on pump-action shotguns
		if (!pm->ps->aiChar) {
			if (pm->ps->weapon == WP_M97 || pm->ps->weapon == WP_AUTO5) {
				if ((pm->cmd.buttons & BUTTON_ATTACK) || (pm->cmd.wbuttons & WBUTTON_ATTACK2)) {
					pm->pmext->m97reloadInterrupt = qtrue;
				}
			}
		}
		return;
	}

	int clipWeap = BG_FindClipForWeapon(weapon);
	int ammoWeap = BG_FindAmmoForWeapon(weapon);

	// Scoped weapons block reload and instead switch
	if (!pm->ps->aiChar) {
		switch (weapon) {
			case WP_SNOOPERSCOPE:
			case WP_SNIPERRIFLE:
			case WP_FG42SCOPE:
			case WP_DELISLESCOPE:
			case WP_M1941SCOPE:
				if (reloadRequested && pm->ps->ammo[ammoWeap]) {
					int maxclip = BG_GetMaxClip(pm->ps, weapon);
					if (pm->ps->ammoclip[clipWeap] < maxclip) {
						PM_BeginWeaponChange(weapon, ammoTable[weapon].weapAlts, qtrue);
					}
				}
				return;
			default:
				break;
		}
	}

	if (pm->ps->weaponTime > 0) {
		return;
	}

	qboolean doReload = qfalse;

	if (reloadRequested) {
		if (pm->ps->ammo[ammoWeap]) {
			if (pm->ps->ammoclip[clipWeap] < BG_GetMaxClip(pm->ps, weapon)) {
				doReload = qtrue;
			}

			// Dual weapon check (Colt or TT33)
			if (weapon == WP_AKIMBO) {
				int coltClip = BG_FindClipForWeapon(WP_COLT);
				if (pm->ps->ammoclip[coltClip] < BG_GetMaxClip(pm->ps, WP_COLT)) {
					doReload = qtrue;
				}
			} else if (weapon == WP_DUAL_TT33) {
				int tt33Clip = BG_FindClipForWeapon(WP_TT33);
				if (pm->ps->ammoclip[tt33Clip] < BG_GetMaxClip(pm->ps, WP_TT33)) {
					doReload = qtrue;
				}
			}
		}
	} else if (autoreload) {
		if (pm->ps->ammoclip[clipWeap] == 0 && pm->ps->ammo[ammoWeap]) {
			switch (weapon) {
				case WP_AKIMBO:
					if (pm->ps->ammoclip[WP_COLT] == 0) doReload = qtrue;
					break;
				case WP_DUAL_TT33:
					if (pm->ps->ammoclip[WP_TT33] == 0) doReload = qtrue;
					break;
				case WP_COLT:
					if (pm->ps->weapon == WP_AKIMBO && pm->ps->ammoclip[WP_AKIMBO] == 0)
						doReload = qtrue;
					else
						doReload = qtrue;
					break;
				case WP_TT33:
					if (pm->ps->weapon == WP_DUAL_TT33 && pm->ps->ammoclip[WP_DUAL_TT33] == 0)
						doReload = qtrue;
					else
						doReload = qtrue;
					break;
				default:
					doReload = qtrue;
					break;
			}
		}
	}

	if (doReload) {
		PM_BeginWeaponReload(weapon);
	}
}

/*
==============
PM_SwitchIfEmpty
==============
*/
static void PM_SwitchIfEmpty( void ) {
	// TODO: cvar for emptyswitch
//	if(!cg_emptyswitch.integer)
//		return;

	// weapon from here down will be a thrown explosive
	switch ( pm->ps->weapon ) {
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
	case WP_DYNAMITE:
	case WP_PANZERFAUST:
	case WP_POISONGAS:
	case WP_KNIFE:
		break;
	default:
		return;
	}


	if ( pm->ps->ammoclip[ BG_FindClipForWeapon( pm->ps->weapon )] ) { // still got ammo in clip
		return;
	}

	if ( pm->ps->ammo[ BG_FindAmmoForWeapon( pm->ps->weapon )] ) { // still got ammo in reserve
		return;
	}

	// If this was the last one, remove the weapon and switch away before the player tries to fire next

	// NOTE: giving grenade ammo to a player will re-give him the weapon (if you do it through add_ammo())
	switch ( pm->ps->weapon ) {
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
	case WP_DYNAMITE:
	case WP_POISONGAS:
	case WP_KNIFE:
		// take the 'weapon' away from the player
		COM_BitClear( pm->ps->weapons, pm->ps->weapon );
		break;
	default:
		break;
	}

	PM_AddEvent( EV_NOAMMO );
}


/*
==============
PM_WeaponUseAmmo
	accounts for clips being used/not used
==============
*/
void PM_WeaponUseAmmo( int wp, int amount ) {
	int takeweapon;

	if ( pm->noWeapClips ) {
		pm->ps->ammo[ BG_FindAmmoForWeapon( wp )] -= amount;
	} else {
		takeweapon = BG_FindClipForWeapon( wp );
		if ( wp == WP_AKIMBO ) {
			if ( !BG_AkimboFireSequence( wp, pm->ps->ammoclip[WP_AKIMBO], pm->ps->ammoclip[WP_COLT] ) ) {
				takeweapon = WP_COLT;
			}
		} else if ( wp == WP_DUAL_TT33 ) {
			if ( !BG_AkimboFireSequence( wp, pm->ps->ammoclip[WP_DUAL_TT33], pm->ps->ammoclip[WP_TT33] ) ) {
				takeweapon = WP_TT33;
			}
		}

		pm->ps->ammoclip[takeweapon] -= amount;
	}
}

/*
==============
PM_WeaponAmmoAvailable
	accounts for clips being used/not used
==============
*/
int PM_WeaponAmmoAvailable( int wp ) {
	int takeweapon;

	if ( pm->noWeapClips ) {
		return pm->ps->ammo[ BG_FindAmmoForWeapon( wp )];
	} else {
		takeweapon = BG_FindClipForWeapon( wp );
		if ( wp == WP_AKIMBO ) {
			if ( !BG_AkimboFireSequence( pm->ps->weapon, pm->ps->ammoclip[WP_AKIMBO], pm->ps->ammoclip[WP_COLT] ) ) {
				takeweapon = WP_COLT;
			}
		} else if ( wp == WP_DUAL_TT33 ) {
			if ( !BG_AkimboFireSequence( pm->ps->weapon, pm->ps->ammoclip[WP_DUAL_TT33], pm->ps->ammoclip[WP_TT33] ) ) {
				takeweapon = WP_TT33;
			}
		}

		return pm->ps->ammoclip[takeweapon];
	}
}

/*
==============
PM_WeaponClipEmpty
	accounts for clips being used/not used
==============
*/
int PM_WeaponClipEmpty( int wp ) {
	if ( pm->noWeapClips ) {
		if ( !( pm->ps->ammo[ BG_FindAmmoForWeapon( wp )] ) ) {
			return 1;
		}
	} else {
		if ( !( pm->ps->ammoclip[BG_FindClipForWeapon( wp )] ) ) {
			return 1;
		}
	}

	return 0;
}

/*
==============
PM_CoolWeapons
==============
*/
void PM_CoolWeapons( void ) {
	int wp;

	for ( wp = 0; wp < WP_NUM_WEAPONS; wp++ ) {

		// if you have the weapon
		if ( COM_BitCheck( pm->ps->weapons, wp ) ) {
			// and it's hot
			if ( pm->ps->weapHeat[wp] ) {
				pm->ps->weapHeat[wp] -= ( (float)ammoTable[wp].coolRate * pml.frametime );

				if ( pm->ps->weapHeat[wp] < 0 ) {
					pm->ps->weapHeat[wp] = 0;
				}

			}
		}
	}

	// a weapon is currently selected, convert current heat value to 0-255 range for client transmission
	if ( pm->ps->weapon ) {
		pm->ps->curWeapHeat = ( ( (float)pm->ps->weapHeat[pm->ps->weapon] / (float)ammoTable[pm->ps->weapon].maxHeat ) ) * 255.0f;

//		if(pm->ps->weapHeat[pm->ps->weapon])
//			Com_Printf("pm heat: %d, %d\n", pm->ps->weapHeat[pm->ps->weapon], pm->ps->curWeapHeat);
	}

}

/*
==============
PM_AdjustAimSpreadScale
==============
*/
void PM_AdjustAimSpreadScale( void ) {
//	int		increase, decrease, i;
	int i;
	float increase, decrease;       // (SA) was losing lots of precision on slower weapons (scoped)
	float viewchange, cmdTime, wpnScale;
//#define	AIMSPREAD_DECREASE_RATE		300.0f
#define AIMSPREAD_DECREASE_RATE     200.0f      // (SA) when I made the increase/decrease floats (so slower weapon recover could happen for scoped weaps) the average rate increased significantly
#define AIMSPREAD_INCREASE_RATE     800.0f
#define AIMSPREAD_VIEWRATE_MIN      30.0f       // degrees per second
#define AIMSPREAD_VIEWRATE_RANGE    120.0f      // degrees per second

	// all weapons are very inaccurate in zoomed mode

	if ( pm->ps->eFlags & EF_ZOOMING ) {

		pm->ps->aimSpreadScale = 255;
		pm->ps->aimSpreadScaleFloat = 255;
		return;
	}

	cmdTime = (float)( pm->cmd.serverTime - pm->oldcmd.serverTime ) / 1000.0;

    wpnScale = GetWeaponTableData(pm->ps->weapon)->spreadScale;

	if ( wpnScale ) {

// JPW NERVE crouched players recover faster (mostly useful for snipers)
		if ( ( pm->ps->eFlags & EF_CROUCHING ) && ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) ) {  //----(SA)	modified so you can't do this in the air.  cool?
			wpnScale *= 0.5;
		}
// jpw

		decrease = ( cmdTime * AIMSPREAD_DECREASE_RATE ) / wpnScale;

		viewchange = 0;
		// take player view rotation into account
		for ( i = 0; i < 2; i++ )
			viewchange += fabs( SHORT2ANGLE( pm->cmd.angles[i] ) - SHORT2ANGLE( pm->oldcmd.angles[i] ) );

		// take player movement into account (even if only for the scoped weapons)
		// TODO: also check for jump/crouch and adjust accordingly
		switch ( pm->ps->weapon ) {
		case WP_SNIPERRIFLE:
		case WP_SNOOPERSCOPE:
		case WP_FG42SCOPE:
		case WP_DELISLESCOPE:
		case WP_M1941SCOPE:
		//case WP_M1GARAND: //haha no plz
			for ( i = 0; i < 2; i++ )
				viewchange += fabs( pm->ps->velocity[i] );
			break;
	//	case WP_PANZERFAUST:        // don't take movement into account as much
		//	for ( i = 0; i < 2; i++ )
			//	viewchange += ( 0.0099f * fabs( pm->ps->velocity[i] ) );
			//break;
		default:
			break;
		}



		viewchange = (float)viewchange / cmdTime;   // convert into this movement for a second
		viewchange -= AIMSPREAD_VIEWRATE_MIN / wpnScale;
		if ( viewchange <= 0 ) {
			viewchange = 0;
		} else if ( viewchange > ( AIMSPREAD_VIEWRATE_RANGE / wpnScale ) ) {
			viewchange = AIMSPREAD_VIEWRATE_RANGE / wpnScale;
		}

		// now give us a scale from 0.0 to 1.0 to apply the spread increase
		viewchange = viewchange / (float)( AIMSPREAD_VIEWRATE_RANGE / wpnScale );

		increase = (int)( cmdTime * viewchange * AIMSPREAD_INCREASE_RATE );
	} else {
		increase = 0;
		decrease = AIMSPREAD_DECREASE_RATE;
	}

	// update the aimSpreadScale
	pm->ps->aimSpreadScaleFloat += ( increase - decrease );
	if ( pm->ps->aimSpreadScaleFloat < 0 ) {
		pm->ps->aimSpreadScaleFloat = 0;
	}
	if ( pm->ps->aimSpreadScaleFloat > 255 ) {
		pm->ps->aimSpreadScaleFloat = 255;
	}

	pm->ps->aimSpreadScale = (int)pm->ps->aimSpreadScaleFloat;  // update the int for the client
}


qboolean PM_AltFire ( void )
{
	if ( pm->cmd.wbuttons & WBUTTON_ATTACK2 ) {
		if ( pm->ps->weapon == WP_KNIFE ||
		     pm->ps->weapon == WP_BAR   ||
			 pm->ps->weapon == WP_MP44  ||
			 pm->ps->weapon == WP_FG42  ) {
			  return qtrue;
		}
	}
	return qfalse;
}

// throwing knife
qboolean PM_AltFiring ( qboolean delayedFire )
{
	if ( pm->ps->weaponstate == WEAPON_FIRINGALT ) {
		if ( pm->ps->weaponDelay > 0 || delayedFire  ) {
			if ( pm->ps->weapon == WP_KNIFE || 
			     pm->ps->weapon == WP_BAR || 
				 pm->ps->weapon == WP_MP44 || 
				 pm->ps->weapon == WP_FG42 )
				  return qtrue;
		}
	}

	return qfalse;
}

static void PM_HandleRecoil ( void ) {
		
		if( !pm->pmext->weapRecoilTime ) {
		return;
	    }
		
		vec3_t muzzlebounce;
		int i, deltaTime;

 		deltaTime = pm->cmd.serverTime - pm->pmext->weapRecoilTime;
		VectorCopy( pm->ps->viewangles, muzzlebounce );

 		if ( deltaTime > pm->pmext->weapRecoilDuration ) {
			deltaTime = pm->pmext->weapRecoilDuration;
		}

 		for ( i = pm->pmext->lastRecoilDeltaTime; i < deltaTime; i += 15 ) {
			if ( pm->pmext->weapRecoilPitch > 0.f ) {
				muzzlebounce[PITCH] -= 2*pm->pmext->weapRecoilPitch*cos( 2.5*(i) / pm->pmext->weapRecoilDuration );
				muzzlebounce[PITCH] -= 0.25 * random() * ( 1.0f - ( i ) / pm->pmext->weapRecoilDuration );
			}

 			if ( pm->pmext->weapRecoilYaw > 0.f ) {
				muzzlebounce[YAW] += 0.5*pm->pmext->weapRecoilYaw*cos( 1.0 - (i)*3 / pm->pmext->weapRecoilDuration );
				muzzlebounce[YAW] += 0.5 * crandom() * ( 1.0f - ( i ) / pm->pmext->weapRecoilDuration );
			}
		}

 		// set the delta angle
		for ( i = 0; i < 3; i++ ) {
			int cmdAngle;

 			cmdAngle = ANGLE2SHORT( muzzlebounce[i] );
			pm->ps->delta_angles[i] = cmdAngle - pm->cmd.angles[i];
		}
		VectorCopy( muzzlebounce, pm->ps->viewangles );

 		if ( deltaTime == pm->pmext->weapRecoilDuration ) {
			pm->pmext->weapRecoilTime = 0;
			pm->pmext->lastRecoilDeltaTime = 0;
		} else {
			pm->pmext->lastRecoilDeltaTime = deltaTime;
		}

}


static qboolean PM_CheckGrenade() {

		if (pm->ps->weapon != WP_GRENADE_LAUNCHER &&
		pm->ps->weapon != WP_GRENADE_PINEAPPLE &&
		pm->ps->weapon != WP_DYNAMITE &&
		pm->ps->weapon != WP_POISONGAS &&
		pm->ps->weapon != WP_AIRSTRIKE &&
		pm->ps->weapon != WP_KNIFE &&
		pm->ps->weapon != WP_POISONGAS_MEDIC &&
	    pm->ps->weapon != WP_DYNAMITE_ENG ) {
			return qfalse;
		}

		// (SA) AI's don't set grenadeTimeLeft on +attack, so I don't check for (pm->ps->aiChar) here
		if ( pm->ps->grenadeTimeLeft > 0 ) {

            // knife case
			if( pm->ps->weapon == WP_KNIFE ) { 
			pm->ps->grenadeTimeLeft += pml.msec;

			if (pm->ps->grenadeTimeLeft > KNIFECHARGETIME)
				pm->ps->grenadeTimeLeft = KNIFECHARGETIME;
		    } 
			
			// dynamite case
		    else if ( pm->ps->weapon == WP_DYNAMITE || pm->ps->weapon == WP_DYNAMITE_ENG ) {
				pm->ps->grenadeTimeLeft += pml.msec;
				
				if ( pm->ps->grenadeTimeLeft > 8000 ) {
					PM_AddEvent( EV_FIRE_WEAPON );
					pm->ps->weaponTime = 1600;
					PM_WeaponUseAmmo( pm->ps->weapon, 1 ); 
				}

			} 
			
			// nades case
			else {
				pm->ps->grenadeTimeLeft -= pml.msec;

				if ( pm->ps->grenadeTimeLeft <= 0 ) {   // give two frames advance notice so there's time to launch and detonate
					PM_WeaponUseAmmo( pm->ps->weapon, 1 ); 
                    if (!( pm->ps->weapon == WP_POISONGAS))
					{
					PM_AddEvent( EV_GRENADE_SUICIDE );      //----(SA)	die, dumbass
					}
				}
			}

		// jaquboss don't allow to catch nade again
		if ( pm->ps->holdable[HI_KNIVES] ){
			pm->cmd.buttons &= ~BUTTON_ATTACK;
			pm->cmd.wbuttons &= ~WBUTTON_ATTACK2;
		}

        if ( pm->ps->weapon == WP_KNIFE  && !( pm->cmd.wbuttons & WBUTTON_ATTACK2 ) ) {
               if ( pm->ps->weaponDelay == ammoTable[pm->ps->weapon].fireDelayTime ) {
			       PM_StartWeaponAnim(WEAP_ATTACK_LASTSHOT); 
                   BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
				   pm->ps->holdable[HI_KNIVES] = 1; // released knife...
		        }
        } else if ( pm->ps->weapon != WP_KNIFE && !( pm->cmd.buttons & BUTTON_ATTACK )) {
                if ( pm->ps->weaponDelay == ammoTable[pm->ps->weapon].fireDelayTime ) {
				    
					if ( pm->ps->weapon != WP_DYNAMITE && pm->ps->weapon != WP_DYNAMITE_ENG ) {
				        PM_StartWeaponAnim(WEAP_ATTACK2);
					}

				    BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
					pm->ps->holdable[HI_KNIVES] = 1; // released knife...

		}

		} else {
		     return qtrue;
		}
    }

         return qfalse;

}

#define weaponstateFiring ( pm->ps->weaponstate == WEAPON_FIRING || pm->ps->weaponstate == WEAPON_FIRINGALT )

#define GRENADE_DELAY   250

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/

#define VENOM_LOW_IDLE  WEAP_IDLE1
#define VENOM_HI_IDLE   WEAP_IDLE2
#define VENOM_RAISE     WEAP_ATTACK1
#define VENOM_ATTACK    WEAP_ATTACK2
#define VENOM_LOWER     WEAP_ATTACK_LASTSHOT

// JPW NERVE
#ifdef CGAMEDLL
extern vmCvar_t cg_soldierChargeTime;
extern vmCvar_t cg_engineerChargeTime;
extern vmCvar_t cg_jumptime;
#endif
#ifdef GAMEDLL
extern vmCvar_t g_soldierChargeTime;
extern vmCvar_t g_engineerChargeTime;
extern vmCvar_t g_jumptime;
#endif
// jpw


#ifdef CGAMEDLL
extern vmCvar_t cg_reloading;
#endif
#ifdef GAMEDLL
extern vmCvar_t g_reloading;
#endif

/*
==============
PM_Weapon
==============
*/
static void PM_Weapon( void ) {
	int ammoNeeded;
	qboolean delayedFire; // true if the delay time has just expired and this is the frame to send the fire event
	int addTime = BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
	int aimSpreadScaleAdd = GetWeaponTableData(pm->ps->weapon)->aimSpreadScaleAdd;
	int weapattackanim;
	qboolean akimboFire_colt;
	qboolean akimboFire_tt33;
	qboolean gameReloading;

	// don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return;
	}

	// game is reloading (mission fail/success)
#ifdef CGAMEDLL
	if ( cg_reloading.integer )
#endif
#ifdef GAMEDLL
	if ( g_reloading.integer )
#endif
		gameReloading = qtrue;
	else {
		gameReloading = qfalse;
	}

	// ignore if spectator
	if ( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->ps->weapon = WP_NONE;
		return;
	}

	// don't allow any weapon stuff if using the mg42
	if ( pm->ps->persistant[PERS_HWEAPON_USE] ) {
		return;
	}

	akimboFire_colt = BG_AkimboFireSequence( pm->ps->weapon, pm->ps->ammoclip[WP_AKIMBO], pm->ps->ammoclip[WP_COLT] );
	akimboFire_tt33 = BG_AkimboFireSequence( pm->ps->weapon, pm->ps->ammoclip[WP_DUAL_TT33], pm->ps->ammoclip[WP_TT33] );

	if ( 0 ) {
		switch ( pm->ps->weaponstate ) {
		case WEAPON_READY:
			Com_Printf( " -- WEAPON_READY\n" );
			break;
		case WEAPON_RAISING:
			Com_Printf( " -- WEAPON_RAISING\n" );
			break;
		case WEAPON_RAISING_TORELOAD:       //----(SA)	added
			Com_Printf( " -- WEAPON_RAISING_TORELOAD\n" );
			break;
		case WEAPON_DROPPING:
			Com_Printf( " -- WEAPON_DROPPING\n" );
			break;
		case WEAPON_DROPPING_TORELOAD:      //----(SA)	added
			Com_Printf( " -- WEAPON_DROPPING_TORELOAD\n" );
			break;
		case WEAPON_READYING:
			Com_Printf( " -- WEAPON_READYING\n" );
			break;
		case WEAPON_RELAXING:
			Com_Printf( " -- WEAPON_RELAXING\n" );
			break;
		case WEAPON_VENOM_REST:
			Com_Printf( " -- WEAPON_VENOM_REST\n" );
			break;
		case WEAPON_FIRING:
			Com_Printf( " -- WEAPON_FIRING\n" );
			break;
		case WEAPON_FIRINGALT:
			Com_Printf( " -- WEAPON_FIRINGALT\n" );
			break;
		case WEAPON_RELOADING:
			Com_Printf( " -- WEAPON_RELOADING\n" );
			break;
		}
	}

	// dec venom timer
	if ( pm->ps->venomTime > 0 ) {
		pm->ps->venomTime -= pml.msec;
	}

	// weapon cool down
	PM_CoolWeapons();
	// do the recoil before setting the values, that way it will be shown next frame and not this
	PM_HandleRecoil();


	// check for item using
	if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE ) {
		if ( !( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) ) {
			gitem_t *item;

			pm->ps->pm_flags |= PMF_USE_ITEM_HELD;

			if ( pm->cmd.holdable ) {
				item = BG_FindItemForHoldable( pm->cmd.holdable );

				if ( item && ( pm->ps->holdable[pm->cmd.holdable] >= item->quantity ) ) { // ->quantity being how much 'ammo' is taken per use
					PM_AddEvent( EV_USE_ITEM0 + pm->cmd.holdable );
					// don't take books away when used
					if ( pm->cmd.holdable < HI_BOOK1 || pm->cmd.holdable > HI_BOOK3 ) {
						pm->ps->holdable[ pm->cmd.holdable ] -= item->quantity;
					}

					if ( pm->ps->holdable[pm->cmd.holdable] <= 0 ) {   // empty
						PM_AddEvent( EV_NOITEM );
					}
				}
			} else {
				PM_AddEvent( EV_USE_ITEM0 );     // send "using nothing"
			}
			return;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}


	delayedFire = qfalse;

	if ( PM_CheckGrenade() )
		return;

	if ( pm->ps->weaponDelay > 0 ) {
		pm->ps->weaponDelay -= pml.msec;
		if ( pm->ps->weaponDelay <= 0 ) {
			pm->ps->weaponDelay = 0;
			delayedFire = qtrue;            // weapon delay has expired.  Fire this frame

			// double check the player is still holding the fire button down for these weapons
			// so you don't get a delayed "non-fire" (fire hit and released, then shot fires)
			switch ( pm->ps->weapon ) {
			case WP_VENOM:
				if ( pm->ps->weaponstate == WEAPON_FIRING ) {
					delayedFire = qfalse;
				}
				break;
			default:
				break;
			}
		}
	}

	if ( pm->ps->weaponstate == WEAPON_RELAXING ) {
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	// make weapon function
	if ( pm->ps->weaponTime > 0 ) {
		pm->ps->weaponTime -= pml.msec;
		if ( pm->ps->weaponTime < 0 ) {
			pm->ps->weaponTime = 0;
		}
	}

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising

	// TTimo gcc: suggest parentheses around && within ||
	if ( pm->ps->weaponTime <= 0 || ( !weaponstateFiring && pm->ps->weaponDelay <= 0 ) ) {
		if ( pm->ps->weapon != pm->cmd.weapon ) {
			PM_BeginWeaponChange( pm->ps->weapon, pm->cmd.weapon, qfalse );     //----(SA)	modified
		}
	}

	// check for clip change
	PM_CheckForReload( pm->ps->weapon );

	if ( pm->ps->weaponTime > 0 || pm->ps->weaponDelay > 0 ) {
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_RELOADING ) {
		PM_FinishWeaponReload();

		// This will happen for chained shotgun reloads
		if (pm->ps->weaponTime > 0) {
			return;
		}
	}

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING || pm->ps->weaponstate == WEAPON_DROPPING_TORELOAD ) {
		PM_FinishWeaponChange();
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_RAISING ) {
		pm->ps->weaponstate = WEAPON_READY;
		PM_StartWeaponAnim( PM_IdleAnimForWeapon( pm->ps->weapon ) );
		return;
	} else if ( pm->ps->weaponstate == WEAPON_RAISING_TORELOAD ) {
		pm->ps->weaponstate = WEAPON_READY;     // need to switch to READY so the reload will work
		PM_BeginWeaponReload( pm->ps->weapon );
		return;
	}

	// unable to use weapon	on the ladder
	#ifdef GAMEDLL
	if ( !delayedFire && g_realism.value ) {
			if ( ( pm->ps->pm_flags & PMF_LADDER )  ){
			if ( pm->ps->weaponstate != WEAPON_HOLSTER_IN ) {
				pm->ps->weaponstate = WEAPON_HOLSTER_IN;
				PM_StartWeaponAnim(PM_DropAnimForWeapon(pm->ps->weapon));
				pm->ps->weaponTime += 300;
			}
			else {
				pm->ps->weaponTime += 50;
			}
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_HOLSTER_IN ){
			pm->ps->weaponstate = WEAPON_HOLSTER_OUT;
			PM_StartWeaponAnim(PM_RaiseAnimForWeapon(pm->ps->weapon));
            pm->ps->weaponTime += 300;
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_HOLSTER_OUT ) {
			pm->ps->weaponstate = WEAPON_READY;
			PM_StartWeaponAnim(PM_IdleAnimForWeapon(pm->ps->weapon));
			return;
		}
	}
	#endif
	#ifdef CGAMEDLL
	if ( !delayedFire && cg_realism.value ) {

		if ( ( pm->ps->pm_flags & PMF_LADDER ) ){
			if ( pm->ps->weaponstate != WEAPON_HOLSTER_IN ) {
				pm->ps->weaponstate = WEAPON_HOLSTER_IN;
				PM_StartWeaponAnim(PM_DropAnimForWeapon(pm->ps->weapon));
				pm->ps->weaponTime += 300;
			}
			else {
				pm->ps->weaponTime += 50;
			}
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_HOLSTER_IN ){
			pm->ps->weaponstate = WEAPON_HOLSTER_OUT;
			PM_StartWeaponAnim(PM_RaiseAnimForWeapon(pm->ps->weapon));
            pm->ps->weaponTime += 300;
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_HOLSTER_OUT ) {
			pm->ps->weaponstate = WEAPON_READY;
			PM_StartWeaponAnim(PM_IdleAnimForWeapon(pm->ps->weapon));
			return;
		}
	}
	#endif

	// unable to use weapon while sprinting
	#ifdef GAMEDLL
	if (!delayedFire && g_realism.value ) {
			if ( ( pm->ps->pm_flags & PMF_SPRINTING ) && ( pm->ps->sprintTime > 0 ) ){
			if ( pm->ps->weaponstate != WEAPON_SPRINT_IN ) {
				pm->ps->weaponstate = WEAPON_SPRINT_IN;
				PM_StartWeaponAnim(PM_SprintInAnimForWeapon(pm->ps->weapon));
				pm->ps->weaponTime += 300;
			}
			else {
				pm->ps->weaponTime += 50;
			}
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_SPRINT_IN ){
			pm->ps->weaponstate = WEAPON_SPRINT_OUT;
			PM_StartWeaponAnim(PM_SprintOutAnimForWeapon(pm->ps->weapon));
            pm->ps->weaponTime += 300;
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_SPRINT_OUT ) {
			pm->ps->weaponstate = WEAPON_READY;
			PM_StartWeaponAnim(PM_IdleAnimForWeapon(pm->ps->weapon));
			return;
		}
	}
	#endif
	#ifdef CGAMEDLL
	if ( !delayedFire && cg_realism.value ) {

		if ( ( pm->ps->pm_flags & PMF_SPRINTING ) && ( pm->ps->sprintTime > 0 ) ){
			if ( pm->ps->weaponstate != WEAPON_SPRINT_IN ) {
				pm->ps->weaponstate = WEAPON_SPRINT_IN;
				PM_StartWeaponAnim(PM_SprintInAnimForWeapon(pm->ps->weapon));
				pm->ps->weaponTime += 300;
			}
			else {
				pm->ps->weaponTime += 50;
			}
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_SPRINT_IN ){
			pm->ps->weaponstate = WEAPON_SPRINT_OUT;
			PM_StartWeaponAnim(PM_SprintOutAnimForWeapon(pm->ps->weapon));
            pm->ps->weaponTime += 300;
			return;
		}
		else if (pm->ps->weaponstate == WEAPON_SPRINT_OUT ) {
			pm->ps->weaponstate = WEAPON_READY;
			PM_StartWeaponAnim(PM_IdleAnimForWeapon(pm->ps->weapon));
			return;
		}
	}
	#endif

	if ( pm->ps->weapon == WP_NONE ) {  // this is possible since the player starts with nothing
		return;
	}

		if ( pm->ps->weapon == WP_AIRSTRIKE ) {
			if ( pm->cmd.serverTime - pm->ps->classWeaponTime < ( pm->ltChargeTime ) ) {
				return;
			}
		}

	if ( pm->ps->weapon == WP_POISONGAS_MEDIC ) {
			if ( pm->cmd.serverTime - pm->ps->classWeaponTime < ( pm->medicChargeTime ) ) {
				return;
			}
		}

	if ( pm->ps->weapon == WP_DYNAMITE_ENG ) {
			if ( pm->cmd.serverTime - pm->ps->classWeaponTime < ( pm->engineerChargeTime ) ) {
				return;
			}
		}
	// check for fire
	if ( (!(pm->cmd.buttons & BUTTON_ATTACK) && !PM_AltFire() && !delayedFire) 
	    || (pm->ps->leanf != 0 && !PM_AltFiring(delayedFire) && pm->ps->weapon != WP_GRENADE_LAUNCHER && pm->ps->weapon != WP_GRENADE_PINEAPPLE && pm->ps->weapon != WP_POISONGAS) )
	{
		pm->ps->weaponTime  = 0;
		pm->ps->weaponDelay = 0;

		if ( weaponstateFiring ) {  // you were just firing, time to relax
			PM_ContinueWeaponAnim( PM_IdleAnimForWeapon( pm->ps->weapon ) );
		}

		pm->ps->weaponstate = WEAPON_READY;
		return;
	}


	if ( gameReloading ) {
		return;
	}

	// player is zooming - no fire
	// JPW NERVE in MP, LT needs to zoom to call artillery
	if ( pm->ps->eFlags & EF_ZOOMING ) {
#ifdef GAMEDLL
		if ( pm->gametype == GT_SURVIVAL ) {
			pm->ps->weaponTime += 500;
			PM_AddEvent( EV_FIRE_WEAPON );
		}
#endif
		return;
	}





	// player is leaning - no fire
	if ( pm->ps->leanf != 0 && pm->ps->weapon != WP_GRENADE_LAUNCHER && pm->ps->weapon != WP_GRENADE_PINEAPPLE && pm->ps->weapon != WP_DYNAMITE && pm->ps->weapon != WP_DYNAMITE_ENG && pm->ps->weapon != WP_KNIFE ) {
		return;
	}

	// player is underwater - no fire
	if ( pm->waterlevel == 3 ) {
		if ( pm->ps->weapon != WP_KNIFE &&
			 pm->ps->weapon != WP_GRENADE_LAUNCHER &&
			 pm->ps->weapon != WP_GRENADE_PINEAPPLE &&
			 pm->ps->weapon != WP_POISONGAS &&
			 pm->ps->weapon != WP_POISONGAS_MEDIC ) {
			PM_AddEvent( EV_NOFIRE_UNDERWATER );        // event for underwater 'click' for nofire
			pm->ps->weaponTime  = 500;
			return;
		}
	}

	// start the animation even if out of ammo
	switch ( pm->ps->weapon ) {
	default:
		if ( !weaponstateFiring ) {
			// delay so the weapon can get up into position before firing (and showing the flash)
			pm->ps->weaponDelay = ammoTable[pm->ps->weapon].fireDelayTime;
		} else {
			BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
		}
		break;
	// machineguns should continue the anim, rather than start each fire
	case WP_MP40:
	// RealRTCW weapons
	case WP_MP34:
    case WP_PPSH:
	case WP_BAR:
	case WP_THOMPSON:
	case WP_STEN:
	case WP_VENOM:
	case WP_FG42:
	case WP_MP44:
	case WP_MG42M:
	case WP_BROWNING:
	case WP_FG42SCOPE:
	case WP_M97:
	case WP_AUTO5:
	case WP_AIRSTRIKE:
	case WP_POISONGAS_MEDIC:
		if ( !weaponstateFiring ) {
			if ( pm->ps->aiChar && pm->ps->weapon == WP_VENOM ) {
				// AI get fast spin-up
				pm->ps->weaponDelay = 150;
			} else {
				// delay so the weapon can get up into position before firing (and showing the flash)
				pm->ps->weaponDelay = ammoTable[pm->ps->weapon].fireDelayTime;
			}
		} else {
			BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qtrue, qtrue );
		}
		break;
	case WP_PANZERFAUST:
	case WP_SILENCER:
	case WP_LUGER:
	case WP_TT33:
	case WP_HDM:
	case WP_REVOLVER:
	case WP_COLT:
	case WP_AKIMBO:
	case WP_DUAL_TT33:         
	case WP_SNIPERRIFLE:
	case WP_SNOOPERSCOPE:
	case WP_MAUSER:
	case WP_DELISLE:
	case WP_DELISLESCOPE:
	case WP_MOSIN:
	case WP_G43:
	case WP_M1GARAND:
	case WP_GARAND:
    case WP_M7:
	case WP_M1941:
	case WP_M1941SCOPE:
		if ( !weaponstateFiring ) {
			// NERVE's panzerfaust spinup
//			if (pm->ps->weapon == WP_PANZERFAUST)
//				PM_AddEvent( EV_SPINUP );
			pm->ps->weaponDelay = ammoTable[pm->ps->weapon].fireDelayTime;
		} else {
			BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
		}
		break;
	// melee
	case WP_KNIFE:
				if(!delayedFire) {
				// throw
				if ( pm->cmd.wbuttons & WBUTTON_ATTACK2 && PM_WeaponAmmoAvailable(pm->ps->weapon) ) {
					BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
					pm->ps->grenadeTimeLeft = 50;
					pm->ps->holdable[HI_KNIVES] = 0;
					PM_StartWeaponAnim(WEAP_ATTACK2);
					pm->ps->weaponDelay = GetWeaponTableData(pm->ps->weapon)->fireDelayTime;
				}
				else {  // stab
				    BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qfalse );
				}
			}
			break;
	case WP_DYNAMITE:
	case WP_DYNAMITE_ENG:
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
case WP_POISONGAS:
	if ( !delayedFire ) {
		if ( pm->ps->aiChar ) {
			// ai characters go into their regular animation setup
			BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qtrue, qtrue );
		} else {
			// the player pulls the fuse and holds the hot potato
			if ( pm->ps->weapon == WP_DYNAMITE_ENG || PM_WeaponAmmoAvailable( pm->ps->weapon ) ) {
				if ( pm->ps->weapon == WP_DYNAMITE || pm->ps->weapon == WP_DYNAMITE_ENG ) {
					pm->ps->grenadeTimeLeft = 50;
				} else {
					// start at four seconds and count down
					pm->ps->grenadeTimeLeft = 4000;
				}
				pm->ps->holdable[HI_KNIVES] = 0; // holding nade
				PM_StartWeaponAnim( WEAP_ATTACK1 );
			}
		}

		pm->ps->weaponDelay = ammoTable[pm->ps->weapon].fireDelayTime;
	}
	break;
	}

	if ( PM_AltFiring(delayedFire) || PM_AltFire() )
		pm->ps->weaponstate = WEAPON_FIRINGALT;
	else
		pm->ps->weaponstate = WEAPON_FIRING;

	// check for out of ammo
	if (pm->ps->weaponUpgraded[pm->ps->weapon])
	{
		ammoNeeded = ammoTable[pm->ps->weapon].usesUpgraded;
	}
	else
	{
		ammoNeeded = ammoTable[pm->ps->weapon].uses;
	}

	if ( pm->ps->weapon ) {
		int ammoAvailable;
		qboolean reloadingW, playswitchsound = qtrue;

		ammoAvailable = PM_WeaponAmmoAvailable( pm->ps->weapon );

		// jaquboss drain ammo if throwing
		// make sure we have one to hold
		if( ( pm->ps->weapon == WP_KNIFE ) && pm->ps->weaponstate == WEAPON_FIRINGALT  ) {
			ammoNeeded = 1;
		}

		if ( ammoNeeded > ammoAvailable ) {

			// you have ammo for this, just not in the clip
			reloadingW = (qboolean)( ammoNeeded <= pm->ps->ammo[ BG_FindAmmoForWeapon( pm->ps->weapon )] );

			// autoreload if not in auto-reload mode, and reload was not explicitely requested, just play the 'out of ammo' sound
			if ( !pm->pmext->bAutoReload && !isAutoReloadWeapon( pm->ps->weapon ) && !( pm->cmd.wbuttons & WBUTTON_RELOAD ) ) {
				reloadingW = qfalse;
			} 

			if ( pm->ps->eFlags & EF_MELEE_ACTIVE ) {
				// not going to be allowed to reload if holding a chair
				reloadingW = qfalse;
			}

			if ( pm->ps->weapon == WP_SNOOPERSCOPE ) {
				reloadingW = qfalse;
			}

			switch ( pm->ps->weapon ) {
			// Ridah, only play if using a triggered weapon
			case WP_MONSTER_ATTACK1:
			case WP_DYNAMITE:
			case WP_DYNAMITE_ENG:
			case WP_GRENADE_LAUNCHER:
			case WP_GRENADE_PINEAPPLE:
			case WP_POISONGAS:
				playswitchsound = qfalse;
				break;
			// some weapons not allowed to reload.  must switch back to primary first
			case WP_SNOOPERSCOPE:
			case WP_SNIPERRIFLE:
			case WP_FG42SCOPE:
			case WP_DELISLESCOPE:
			case WP_M1941SCOPE:
				reloadingW = qfalse;
				break;
			}

			if ( playswitchsound ) {
				if ( reloadingW ) {
					PM_AddEvent( EV_EMPTYCLIP );
				} else {
					PM_AddEvent( EV_NOAMMO );
				}
			}

			if ( reloadingW ) {
				PM_ContinueWeaponAnim( WEAP_RELOAD1 );      //----(SA)
			} else {
				PM_ContinueWeaponAnim( PM_IdleAnimForWeapon( pm->ps->weapon ) );
				pm->ps->weaponTime += 500;
			}

			return;
		}
	}

	if ( pm->ps->weaponDelay > 0 ) {
		// if it hits here, the 'fire' has just been hit and the weapon dictated a delay.
		// animations have been started, weaponstate has been set, but no weapon events yet. (except possibly EV_NOAMMO)
		// checks for delayed weapons that have already been fired are return'ed above.
		return;
	}


	// take an ammo away if not infinite
	if ( PM_WeaponAmmoAvailable( pm->ps->weapon ) != -1 ) {
		// Rafael - check for being mounted on mg42
		if ( !( pm->ps->persistant[PERS_HWEAPON_USE] ) ) {
			PM_WeaponUseAmmo( pm->ps->weapon, ammoNeeded );
		}
	}


	// fire weapon

	// Add weapon heat (unless player has Rifling perk)
	if (!pm->ps->perks[PERK_RIFLING])
	{
		const ammoTable_t *wt = &ammoTable[pm->ps->weapon];

		if (wt->maxHeat)
		{
			int heatToAdd = BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
			pm->ps->weapHeat[pm->ps->weapon] += heatToAdd;
		}
	}

	// first person weapon animations

	// if this was the last round in the clip, play the 'lastshot' animation
	// this animation has the weapon in a "ready to reload" state
	if ( pm->ps->weapon == WP_AKIMBO ) {
		if ( akimboFire_colt ) {
			weapattackanim = WEAP_ATTACK1;      // attack1 is right hand
		} else {
			weapattackanim = WEAP_ATTACK2;      // attack2 is left hand
		}
	} else if ( pm->ps->weapon == WP_DUAL_TT33 ) {
		if ( akimboFire_tt33 ) {
			weapattackanim = WEAP_ATTACK1;      // attack1 is right hand
		} else {
			weapattackanim = WEAP_ATTACK2;      // attack2 is left hand
		}
	} else {
		if ( PM_WeaponClipEmpty( pm->ps->weapon ) ) {
			weapattackanim = WEAP_ATTACK_LASTSHOT;
		} else {
			weapattackanim = WEAP_ATTACK1;
		}
	}

	switch ( pm->ps->weapon ) {
	case WP_MAUSER:
	case WP_DELISLE:
	case WP_MOSIN:
	case WP_G43:
	case WP_M1941:
	case WP_M1GARAND:
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
	case WP_DYNAMITE:
	case WP_DYNAMITE_ENG:
	case WP_M97:
	case WP_AUTO5:
    case WP_M7:
		PM_StartWeaponAnim( weapattackanim );
		break;
	case WP_VENOM:
	case WP_MP40:
	// RealRTCW weapons
	case WP_MP34:
	case WP_BAR:
	case WP_PPSH:
    case WP_MP44:
	case WP_MG42M:
	case WP_BROWNING:
	case WP_THOMPSON:
	case WP_STEN:
	case WP_AIRSTRIKE:
	case WP_POISONGAS_MEDIC:
		PM_ContinueWeaponAnim( weapattackanim );
		break;

	case WP_KNIFE:
		if ( pm->ps->weaponstate != WEAPON_FIRINGALT )
		    {
			PM_StartWeaponAnim(weapattackanim);
		    }
			break;

	default:
// RF, testing
//		PM_ContinueWeaponAnim(weapattackanim);
		PM_StartWeaponAnim( weapattackanim );
		break;
	}

		if ( pm->ps->weapon == WP_AIRSTRIKE || pm->ps->weapon == WP_POISONGAS_MEDIC || pm->ps->weapon == WP_DYNAMITE_ENG ) { 
			PM_AddEvent( EV_NOAMMO );
		}



	if ( pm->ps->weapon == WP_AKIMBO ) {
		if ( pm->ps->weapon == WP_AKIMBO && !akimboFire_colt ) {
			PM_AddEvent( EV_FIRE_WEAPONB );     // really firing colt
		} else {
			PM_AddEvent( EV_FIRE_WEAPON );
		}
	} else if ( pm->ps->weapon == WP_DUAL_TT33 ) {
		if ( pm->ps->weapon == WP_DUAL_TT33 && !akimboFire_tt33 ) {
			PM_AddEvent( EV_FIRE_WEAPONB );     // really firing colt
		} else {
			PM_AddEvent( EV_FIRE_WEAPON );
		}
	} else {
		if ( pm->ps->weapon == WP_KNIFE && pm->ps->weaponstate == WEAPON_FIRINGALT ){
			PM_AddEvent( EV_THROWKNIFE );
		} else if ( PM_WeaponClipEmpty( pm->ps->weapon ) ) {
			PM_AddEvent( EV_FIRE_WEAPON_LASTSHOT );
		} else {
			PM_AddEvent( EV_FIRE_WEAPON );
		}
	}
// RF
	pm->ps->releasedFire = qfalse;
	pm->ps->lastFireTime = pm->cmd.serverTime;

	if ( ( pm->ps->weapon == WP_M7 ) && !pm->ps->ammo[ BG_FindAmmoForWeapon( pm->ps->weapon )] ) {
		PM_AddEvent( EV_NOAMMO );
	}

	// Alt firing mode
	if (pm->cmd.wbuttons & WBUTTON_ATTACK2)
	{
		addTime = BG_GetNextShotTime(pm->ps, pm->ps->weapon, qtrue);
	}
	
	switch ( pm->ps->weapon ) {
	    case WP_KNIFE:
	        addTime = pm->ps->weaponstate == WEAPON_FIRINGALT ? 750 : BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
	    break;
	    case WP_G43:
		case WP_M1941:
	    case WP_M1GARAND:
	        if ( pm->ps->aiChar )
	        {
	        addTime *= 2;
	         }
	    break;
	    case WP_MP40:
	        if ( pm->ps->aiChar )
	        {
	        addTime *= 0.8;
	        }
	    break;
	    case WP_FG42SCOPE:
	        if (!( pm->ps->aiChar ))
	        {
	        addTime *= 2.5;
	        }
	    break;
	    case WP_AKIMBO:
		    addTime = BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
		       if ( !pm->ps->ammoclip[WP_AKIMBO] || !pm->ps->ammoclip[WP_COLT] ) {
			       if ( ( !pm->ps->ammoclip[WP_AKIMBO] && !akimboFire_colt ) || ( !pm->ps->ammoclip[WP_COLT] && akimboFire_colt ) ) {
				        addTime = 2 * BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
			       }
		       }
		break;
	    case WP_DUAL_TT33:
		    addTime = BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
		       if ( !pm->ps->ammoclip[WP_DUAL_TT33] || !pm->ps->ammoclip[WP_TT33] ) {
			       if ( ( !pm->ps->ammoclip[WP_DUAL_TT33] && !akimboFire_tt33 ) || ( !pm->ps->ammoclip[WP_TT33] && akimboFire_tt33 ) ) {
				        addTime = 2 * BG_GetNextShotTime(pm->ps, pm->ps->weapon, qfalse);
			       }
		       }
		break;
	}

	// set weapon recoil (kickback)
	pm->pmext->lastRecoilDeltaTime = 0;
	pm->pmext->weapRecoilTime      = GetWeaponTableData(pm->ps->weapon)->weapRecoilDuration ? pm->cmd.serverTime : 0;
	pm->pmext->weapRecoilDuration  = GetWeaponTableData(pm->ps->weapon)->weapRecoilDuration;
	pm->pmext->weapRecoilYaw       = GetWeaponTableData(pm->ps->weapon)->weapRecoilYaw[0] * crandom() * GetWeaponTableData(pm->ps->weapon)->weapRecoilYaw[1];
	pm->pmext->weapRecoilPitch     = GetWeaponTableData(pm->ps->weapon)->weapRecoilPitch[0] * random() * GetWeaponTableData(pm->ps->weapon)->weapRecoilPitch[1];


	if ( ammoTable[pm->ps->weapon].weaponClass == WEAPON_CLASS_SMG)
	{
		aimSpreadScaleAdd += rand() % 5;
	}

    if ( ( pm->ps->eFlags & EF_CROUCHING ) && ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) ) { 
		pm->pmext->weapRecoilDuration *= 0.5;
	}

	// check for overheat

	// the weapon can overheat, and it's hot
	if ((pm->ps->aiChar != AICHAR_PROTOSOLDIER) &&
		(pm->ps->aiChar != AICHAR_SUPERSOLDIER) &&
		(pm->ps->aiChar != AICHAR_SUPERSOLDIER_LAB) &&
		(pm->ps->aiChar != AICHAR_XSHEPHERD) &&
		(ammoTable[pm->ps->weapon].maxHeat && pm->ps->weapHeat[pm->ps->weapon]))
	{
		// it is overheating
		if (pm->ps->weapHeat[pm->ps->weapon] >= ammoTable[pm->ps->weapon].maxHeat)
		{
			pm->ps->weapHeat[pm->ps->weapon] = ammoTable[pm->ps->weapon].maxHeat; // cap heat to max
			PM_AddEvent(EV_WEAP_OVERHEAT);
			addTime = 2000; // force "heat recovery minimum" to 2 sec right now
		}
	}

	if ( pm->ps->powerups[PW_HASTE_SURV] ) {
		addTime /= 1.3;
	}


	if ( pm->ps->perks[PERK_RIFLING] ) {
		addTime /= 1.25;
	}

	// add the recoil amount to the aimSpreadScale
//	pm->ps->aimSpreadScale += 3.0*aimSpreadScaleAdd;
//	if (pm->ps->aimSpreadScale > 255)
//		pm->ps->aimSpreadScale = 255;
	pm->ps->aimSpreadScaleFloat += 3.0 * aimSpreadScaleAdd;
	if ( pm->ps->aimSpreadScaleFloat > 255 ) {
		pm->ps->aimSpreadScaleFloat = 255;
	}
	pm->ps->aimSpreadScale = (int)( pm->ps->aimSpreadScaleFloat );

	pm->ps->weaponTime += addTime;

		// jaquboss, pull another of those
	switch(pm->ps->weapon) {
		case WP_GRENADE_LAUNCHER:
		case WP_GRENADE_PINEAPPLE:
		case WP_POISONGAS:
		case WP_AIRSTRIKE:
		case WP_POISONGAS_MEDIC:
			pm->ps->weaponstate = WEAPON_DROPPING;
			pm->ps->holdable[HI_KNIVES] = 0;
			break;

		case WP_KNIFE:
			if ( pm->ps->weaponstate == WEAPON_FIRINGALT ){
				pm->ps->weaponstate = WEAPON_DROPPING;
			    pm->ps->holdable[HI_KNIVES] = 0;
			}
			break;

		default:
			break;
	}

	PM_SwitchIfEmpty();

}



/*
==============
PM_QuickGrenade
==============
*/
static void PM_QuickGrenade( void ) {

	if ( pm->ps->quickGrenTime > 0 ) 
	{
		pm->ps->quickGrenTime -= pml.msec;
	}

	if ( pm->ps->quickGrenTime < 0 ) 
	{
		pm->ps->quickGrenTime = 0;
	}

	if (pm->ps->weapon == WP_GRENADE_LAUNCHER || pm->ps->weapon == WP_GRENADE_PINEAPPLE || pm->ps->weapon == WP_SNIPERRIFLE || pm->ps->weapon == WP_FG42SCOPE || pm->ps->weapon == WP_SNOOPERSCOPE )
	{
		return;
	}
	
	if ( pm->ps->pm_flags & PMF_RESPAWNED )
	{
		return;
	}

	if ( pm->ps->stats[STAT_HEALTH] <= 0 )
	{
		return;
	}

	if (pm->ps->eFlags & EF_ZOOMING)
	{
		return;
	}

	if (pm->ps->leanf != 0)
	{
	    return;
	}

	if ( pm->ps->quickGrenTime > 0 )
	{
	    return;
	}

    if ( pm->cmd.wbuttons & WBUTTON_QUICKGREN ) 
    {
		 pm->ps->quickGrenTime = 2500;
           
	     if ( PM_WeaponAmmoAvailable( WP_GRENADE_LAUNCHER ) )  // ammo check
	     { 
		 PM_AddEvent( EV_FIRE_QUICKGREN );
	     PM_WeaponUseAmmo( WP_GRENADE_LAUNCHER, 1 );
	     } else if ( PM_WeaponAmmoAvailable( WP_GRENADE_PINEAPPLE ) ) // ammo check 2
		 {
		 PM_AddEvent( EV_FIRE_QUICKGREN2 );
		 PM_WeaponUseAmmo( WP_GRENADE_PINEAPPLE, 1 );
		 } else {
		 PM_AddEvent( EV_NOQUICKGRENAMMO ); // no ammo
		 }
        return;
	}
}


/*
================
PM_Animate
================
*/
#define MYTIMER_SALUTE   1133   // 17 frames, 15 fps
#define MYTIMER_DISMOUNT 667    // 10 frames, 15 fps

static void PM_Animate( void ) {
/*
	if ( pm->cmd.buttons & BUTTON_GESTURE ) {
		if ( pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim( BOTH_SALUTE );
			PM_StartLegsAnim( BOTH_SALUTE );

			pm->ps->torsoTimer = MYTIMER_SALUTE;
			pm->ps->legsTimer = MYTIMER_SALUTE;

			if (!pm->ps->aiChar)	// Ridah, we'll play a custom sound upon calling the Taunt
				PM_AddEvent( EV_TAUNT );	// for playing the sound
		}
	}
*/
}


/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if ( pm->ps->legsTimer > 0 ) {
		pm->ps->legsTimer -= pml.msec;
		if ( pm->ps->legsTimer < 0 ) {
			pm->ps->legsTimer = 0;
		}
	}

	if ( pm->ps->torsoTimer > 0 ) {
		pm->ps->torsoTimer -= pml.msec;
		if ( pm->ps->torsoTimer < 0 ) {
			pm->ps->torsoTimer = 0;
		}
	}

	// first person weapon counter
	if ( pm->ps->weapAnimTimer > 0 ) {
		pm->ps->weapAnimTimer -= pml.msec;
		if ( pm->ps->weapAnimTimer < 0 ) {
			pm->ps->weapAnimTimer = 0;
		}
	}
}



#define LEAN_MAX    28.0f
#define LEAN_TIME_TO    280.0f  // time to get to full lean
#define LEAN_TIME_FR    350.0f  // time to get from full lean

/*
==============
PM_CalcLean

==============
*/
void PM_UpdateLean( playerState_t *ps, usercmd_t *cmd, pmove_t *tpm ) {
	vec3_t start, end, tmins, tmaxs, right;
	int leaning = 0;            // -1 left, 1 right
	float leanofs = 0;
	vec3_t viewangles;
	trace_t trace;

	if ( ps->aiChar ) {
		return;
	}

	if ( ( cmd->wbuttons & ( WBUTTON_LEANLEFT | WBUTTON_LEANRIGHT ) )  && !cmd->forwardmove && cmd->upmove <= 0 ) {
		// if both are pressed, result is no lean
		if ( cmd->wbuttons & WBUTTON_LEANLEFT ) {
			leaning -= 1;
		}
		if ( cmd->wbuttons & WBUTTON_LEANRIGHT ) {
			leaning += 1;
		}
	}

	// not allowed when...
	if ( ( ps->eFlags & ( EF_MELEE_ACTIVE |   // ...holding a chair
						  EF_MG42_ACTIVE  | // ...on mg42
						  EF_FIRING ) ) ) { // ...firing
		leaning = 0;
	}

	leanofs = ps->leanf;

	if ( !leaning ) {  // go back to center position
		if ( leanofs > 0 ) {        // right
			//FIXME: play lean anim backwards?
			leanofs -= ( ( (float)pml.msec / LEAN_TIME_FR ) * LEAN_MAX );
			if ( leanofs < 0 ) {
				leanofs = 0;
			}
		} else if ( leanofs < 0 )   { // left
			//FIXME: play lean anim backwards?
			leanofs += ( ( (float)pml.msec / LEAN_TIME_FR ) * LEAN_MAX );
			if ( leanofs > 0 ) {
				leanofs = 0;
			}
		}
	}

	if ( leaning ) {
		if ( leaning > 0 ) {   // right
			if ( leanofs < LEAN_MAX ) {
				leanofs += ( ( (float)pml.msec / LEAN_TIME_TO ) * LEAN_MAX );
			}

			if ( leanofs > LEAN_MAX ) {
				leanofs = LEAN_MAX;
			}

		} else {              // left
			if ( leanofs > -LEAN_MAX ) {
				leanofs -= ( ( (float)pml.msec / LEAN_TIME_TO ) * LEAN_MAX );
			}

			if ( leanofs < -LEAN_MAX ) {
				leanofs = -LEAN_MAX;
			}

		}
	}

	ps->leanf = leanofs;

	if ( leaning ) {
		VectorCopy( ps->origin, start );
		start[2] += ps->viewheight;
		VectorCopy( ps->viewangles, viewangles );
		viewangles[ROLL] = 0;
		AngleVectors( viewangles, NULL, right, NULL );
		VectorNormalize( right );
		right[2] = ( leanofs < 0 ) ? 0.25 : -0.25;
		VectorMA( start, leanofs, right, end );
		VectorSet( tmins, -12, -12, -6 );
		VectorSet( tmaxs, 12, 12, 10 );

		if ( pm ) {
			pm->trace( &trace, start, tmins, tmaxs, end, ps->clientNum, MASK_PLAYERSOLID );
		} else {
			tpm->trace( &trace, start, tmins, tmaxs, end, ps->clientNum, MASK_PLAYERSOLID );
		}

		ps->leanf *= trace.fraction;
	}


	if ( ps->leanf ) {
		cmd->rightmove = 0;     // also disallowed in cl_input ~391

	}
}



/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated instead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, usercmd_t *cmd, void( trace ) ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask ) ) {   //----(SA)	modified
	short temp;
	int i;
	pmove_t tpm;

	if ( ps->pm_type == PM_FREEZE ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;     // no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
		return;     // no view changes at all
	}

	// circularly clamp the angles with deltas
	for ( i = 0 ; i < 3 ; i++ ) {
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE( temp );
	}


	tpm.trace = trace;
//	tpm.trace (&trace, start, tmins, tmaxs, end, ps->clientNum, MASK_PLAYERSOLID);

	PM_UpdateLean( ps, cmd, &tpm );
}

/*
================
PM_CheckLadderMove

  Checks to see if we are on a ladder
================
*/
qboolean ladderforward;
vec3_t laddervec;

void PM_CheckLadderMove( void ) {
	vec3_t spot;
	vec3_t flatforward;
	trace_t trace;
	float tracedist;
	#define TRACE_LADDER_DIST   48.0
	qboolean wasOnLadder;

	if ( pm->ps->pm_time ) {
		return;
	}

	//if (pm->ps->pm_flags & PM_DEAD)
	//	return;

		if (pm->ps->aiChar == AICHAR_DOG || pm->ps->aiChar == AICHAR_XSHEPHERD ) {
		pml.ladder = qfalse;
		pm->ps->pm_flags &= ~PMF_LADDER;    // clear ladder bit
		ladderforward = qfalse;
		return;
	}

	if ( pml.walking ) {
		tracedist = 1.0;
	} else {
		tracedist = TRACE_LADDER_DIST;
	}

	wasOnLadder = ( ( pm->ps->pm_flags & PMF_LADDER ) != 0 );

	pml.ladder = qfalse;
	pm->ps->pm_flags &= ~PMF_LADDER;    // clear ladder bit
	ladderforward = qfalse;

	/*
	if (pm->ps->eFlags & EF_DEAD) {	// dead bodies should fall down ladders
		return;
	}

	if (pm->ps->pm_flags & PM_DEAD && pm->ps->stats[STAT_HEALTH] <= 0)
	{
		return;
	}
	*/
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize( flatforward );

	VectorMA( pm->ps->origin, tracedist, flatforward, spot );
	pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, pm->tracemask );
	if ( ( trace.fraction < 1 ) && ( trace.surfaceFlags & SURF_LADDER ) ) {
		pml.ladder = qtrue;
	}
/*
	if (!pml.ladder && DotProduct(pm->ps->velocity, pml.forward) < 0) {
		// trace along the negative velocity, so we grab onto a ladder if we are trying to reverse onto it from above the ladder
		flatforward[0] = -pm->ps->velocity[0];
		flatforward[1] = -pm->ps->velocity[1];
		flatforward[2] = 0;
		VectorNormalize (flatforward);

		VectorMA (pm->ps->origin, tracedist, flatforward, spot);
		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, pm->tracemask);
		if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
		{
			pml.ladder = qtrue;
		}
	}
*/
	if ( pml.ladder ) {
		VectorCopy( trace.plane.normal, laddervec );
	}

	if ( pml.ladder && !pml.walking && ( trace.fraction * tracedist > 1.0 ) ) {
		vec3_t mins;
		// if we are only just on the ladder, don't do this yet, or it may throw us back off the ladder
		pml.ladder = qfalse;
		VectorCopy( pm->mins, mins );
		mins[2] = -1;
		VectorMA( pm->ps->origin, -tracedist, laddervec, spot );
		pm->trace( &trace, pm->ps->origin, mins, pm->maxs, spot, pm->ps->clientNum, pm->tracemask );
		if ( ( trace.fraction < 1 ) && ( trace.surfaceFlags & SURF_LADDER ) ) {
			// if AI, then be more stringent on their viewangles
			if ( pm->ps->aiChar && ( DotProduct( trace.plane.normal, pml.forward ) > -0.9 ) ) {
				pml.ladder = qfalse;
			} else {
				ladderforward = qtrue;
				pml.ladder = qtrue;
				pm->ps->pm_flags |= PMF_LADDER; // set ladder bit
			}
		} else {
			pml.ladder = qfalse;
		}
	} else if ( pml.ladder ) {
		pm->ps->pm_flags |= PMF_LADDER; // set ladder bit
	}

	// create some up/down velocity if touching ladder
	if ( pml.ladder ) {
		if ( pml.walking ) {
			// we are currently on the ground, only go up and prevent X/Y if we are pushing forwards
			if ( pm->cmd.forwardmove <= 0 ) {
				pml.ladder = qfalse;
			}
		}
	}

	// if we have just dismounted the ladder at the top, play dismount
	if ( !pml.ladder && wasOnLadder && pm->ps->velocity[2] > 0 ) {
		BG_AnimScriptEvent( pm->ps, ANIM_ET_CLIMB_DISMOUNT, qfalse, qfalse );
	}
	// if we have just mounted the ladder
	if ( pml.ladder && !wasOnLadder && pm->ps->velocity[2] < 0 ) {    // only play anim if going down ladder
		BG_AnimScriptEvent( pm->ps, ANIM_ET_CLIMB_MOUNT, qfalse, qfalse );
	}
}

/*
============
PM_LadderMove
============
*/
void PM_LadderMove( void ) {
	float wishspeed, scale;
	vec3_t wishdir, wishvel;
	float upscale;

	if (pm->ps->aiChar == AICHAR_DOG || pm->ps->aiChar == AICHAR_XSHEPHERD) {
		return;
	}

	if ( ladderforward ) {
		// move towards the ladder
		VectorScale( laddervec, -200.0, wishvel );
		pm->ps->velocity[0] = wishvel[0];
		pm->ps->velocity[1] = wishvel[1];
	}

	upscale = ( pml.forward[2] + 0.5 ) * 2.5;
	if ( upscale > 1.0 ) {
		upscale = 1.0;
	} else if ( upscale < -1.0 ) {
		upscale = -1.0;
	}

	// forward/right should be horizontal only
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize( pml.forward );
	VectorNormalize( pml.right );

	// move depending on the view, if view is straight forward, then go up
	// if view is down more then X degrees, start going down
	// if they are back pedalling, then go in reverse of above
	scale = PM_CmdScale( &pm->cmd );
	VectorClear( wishvel );

	if ( pm->cmd.forwardmove ) {
		if ( pm->ps->aiChar ) {
			wishvel[2] = 0.5 * upscale * scale * (float)pm->cmd.forwardmove;
		} else { // player speed
	            #ifdef GAMEDLL
				if (g_realism.value) {
			    wishvel[2] = 0.8 * upscale * scale * (float)pm->cmd.forwardmove;
		        } else {
			    wishvel[2] = 0.9 * upscale * scale * (float)pm->cmd.forwardmove;
		        }
				#endif
				 #ifdef CGAMEDLL
				if (cg_realism.value) {
			    wishvel[2] = 0.8 * upscale * scale * (float)pm->cmd.forwardmove;
		        } else {
			    wishvel[2] = 0.9 * upscale * scale * (float)pm->cmd.forwardmove;
		        }
				#endif
		}
	}

	if ( pm->cmd.rightmove ) {
		// strafe, so we can jump off ladder
		vec3_t ladder_right, ang;
		vectoangles( laddervec, ang );
		AngleVectors( ang, NULL, ladder_right, NULL );

		// if we are looking away from the ladder, reverse the right vector
		if ( DotProduct( laddervec, pml.forward ) > 0 ) {
			VectorInverse( ladder_right );
		}

		VectorMA( wishvel, 0.5 * scale * (float)pm->cmd.rightmove, pml.right, wishvel );
	}

	// do strafe friction
	PM_Friction();

	wishspeed = VectorNormalize2( wishvel, wishdir );

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );
	if ( !wishvel[2] ) {
		if ( pm->ps->velocity[2] > 0 ) {
			pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
			if ( pm->ps->velocity[2] < 0 ) {
				pm->ps->velocity[2]  = 0;
			}
		} else
		{
			pm->ps->velocity[2] += pm->ps->gravity * pml.frametime;
			if ( pm->ps->velocity[2] > 0 ) {
				pm->ps->velocity[2]  = 0;
			}
		}
	}

//Com_Printf("vel[2] = %i\n", (int)pm->ps->velocity[2] );

	PM_StepSlideMove( qfalse );  // no gravity while going up ladder

	// always point legs forward
	pm->ps->movementDir = 0;
}


/*
==============
PM_Sprint
==============
*/
//----(SA)	cleaned up for SP (10/22/01)
void PM_Sprint( void ) {

	int staminaDrain = 2000;
    int staminaRecharge = 500;


    // Check if the player has PERK_RUNNER
    if (pm->ps->perks[PERK_RUNNER] > 0) {
        // Remove stamina drain
        staminaDrain = 0;
    }


	if (    ( pm->cmd.buttons & BUTTON_SPRINT ) &&
			( pm->cmd.forwardmove || pm->cmd.rightmove ) &&
			!( pm->ps->pm_flags & PMF_DUCKED ) ) {

		if ( pm->ps->powerups[PW_NOFATIGUE] ) {    // take time from powerup before taking it from sprintTime
			pm->ps->powerups[PW_NOFATIGUE] -= staminaDrain * pml.frametime; 

			pm->ps->sprintTime += 10;           // (SA) go ahead and continue to recharge stamina at double rate with stamina powerup even when exerting
			if ( pm->ps->sprintTime > 20000 ) {
				pm->ps->sprintTime = 20000;
			}

			if ( pm->ps->powerups[PW_NOFATIGUE] < 0 ) {
				pm->ps->powerups[PW_NOFATIGUE] = 0;
			}
		} else {
			// RF, dont drain sprintTime if not moving
			if ( VectorLength( pm->ps->velocity ) > 128 ) { // (SA) check for a bit more movement
				pm->ps->sprintTime -= staminaDrain * pml.frametime; 
			}
		}

		if ( pm->ps->sprintTime < 0 ) {
			pm->ps->sprintTime = 0;
		}

		if ( !pm->ps->sprintExertTime ) {
			pm->ps->sprintExertTime = 1;
		}

		pm->ps->pm_flags |= PMF_SPRINTING; // LET US KNOW THAT WE ARE SPRINTING

	} else
	{
		// JPW NERVE adjusted for framerate independence

		// regular recharge
		pm->ps->sprintTime += staminaRecharge * pml.frametime;

		// additional (2x) recharge if in top 75% of sprint bar, or with stamina powerup
		if ( pm->ps->sprintTime > 5000 || pm->ps->powerups[PW_NOFATIGUE] ) {
			pm->ps->sprintTime +=  staminaRecharge * pml.frametime;
		}

		// additional recharge if standing still
		if ( !( pm->cmd.forwardmove || pm->cmd.rightmove ) ) {
			pm->ps->sprintTime +=  staminaRecharge * pml.frametime;
		}

		if ( pm->ps->sprintTime > 20000 ) {
			pm->ps->sprintTime = 20000;
		}

		pm->ps->sprintExertTime = 0;

		pm->ps->pm_flags &= ~PMF_SPRINTING; // LET US KNOW THAT WE ARE NO LONGER SPRINTING
	}
}

/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );

void PmoveSingle( pmove_t *pmove ) {
	// Ridah
	qboolean isDummy;

	isDummy = ( ( pmove->ps->eFlags & EF_DUMMY_PMOVE ) != 0 );
	// done.

	if ( !isDummy ) {
		// RF, update conditional values for anim system
		BG_AnimUpdatePlayerStateConditions( pmove );
	}

	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->tracemask &= ~CONTENTS_BODY;    // corpses can fly through bodies
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if ( !pm->ps->aiChar ) {
		if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
			pm->cmd.buttons &= ~BUTTON_WALKING;
		}
	}

	// set the talk balloon flag
	if ( !isDummy ) {
		if ( !pm->ps->aiChar && pm->cmd.buttons & BUTTON_TALK ) {
			pm->ps->eFlags |= EF_TALK;
		} else {
			pm->ps->eFlags &= ~EF_TALK;
		}
	}

	// set the firing flag for continuous beam weapons

	pm->ps->eFlags &= ~( EF_FIRING | EF_ZOOMING );

	if ( pm->cmd.wbuttons & WBUTTON_ZOOM ) {
		if ( pm->ps->stats[STAT_KEYS] & ( 1 << INV_BINOCS ) ) {        // (SA) binoculars are an inventory item (inventory==keys)
			if ( pm->ps->weapon != WP_SNIPERRIFLE && pm->ps->weapon != WP_SNOOPERSCOPE && pm->ps->weapon != WP_FG42SCOPE && pm->ps->weapon != WP_DELISLESCOPE && pm->ps->weapon != WP_M1941SCOPE ) {   // don't allow binocs if using scope
				if ( !( pm->ps->eFlags & EF_MG42_ACTIVE ) ) {    // or if mounted on a weapon
					pm->ps->eFlags |= EF_ZOOMING;
				}
			}

			// don't allow binocs if in the middle of throwing grenade
			if ( ( pm->ps->weapon == WP_GRENADE_LAUNCHER || pm->ps->weapon == WP_GRENADE_PINEAPPLE || pm->ps->weapon == WP_DYNAMITE || pm->ps->weapon == WP_DYNAMITE_ENG || pm->ps->weapon == WP_POISONGAS ) && pm->ps->grenadeTimeLeft > 0 ) {
				pm->ps->eFlags &= ~EF_ZOOMING;
			}
		}
	}



	if ( !(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION && pm->ps->pm_type != PM_NOCLIP ) {
		// check if zooming
		if ( !( pm->cmd.wbuttons & WBUTTON_ZOOM ) ) {
			if ( (pm->cmd.buttons & BUTTON_ATTACK) || ((pm->cmd.wbuttons & WBUTTON_ATTACK2) && ( (pm->ps->weapon == WP_BAR) || (pm->ps->weapon == WP_FG42) || (pm->ps->weapon == WP_MP44))  )) {
				// check for ammo
				if ( PM_WeaponAmmoAvailable( pm->ps->weapon ) ) {
					// all clear, fire!
					pm->ps->eFlags |= EF_FIRING;
				}
			}
		}
	}

	// clear the respawned flag if attack, attack2 and use are cleared
	if (pm->ps->stats[STAT_HEALTH] > 0 &&
		!(pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE)) &&
		!(pm->cmd.wbuttons & WBUTTON_ATTACK2))
	{
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.wbuttons = 0;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset( &pml, 0, sizeof( pml ) );

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( !isDummy ) {
		if ( pml.msec < 1 ) {
			pml.msec = 1;
		} else if ( pml.msec > 200 ) {
			pml.msec = 200;
		}
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy( pm->ps->origin, pml.previous_origin );

	// save old velocity for crashlanding
	VectorCopy( pm->ps->velocity, pml.previous_velocity );

	pml.frametime = pml.msec * 0.001;

	// update the viewangles
	// Ridah
	if ( !isDummy ) {
		// done.
		if ( !( pm->ps->pm_flags & PMF_LIMBO ) ) { // JPW NERVE
			PM_UpdateViewAngles( pm->ps, &pm->cmd, pm->trace ); //----(SA)	modified

		}
	}
	AngleVectors( pm->ps->viewangles, pml.forward, pml.right, pml.up );

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD || pm->ps->pm_flags & PMF_LIMBO ) {         // DHM - Nerve
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if ( ( pm->ps->pm_type == PM_SPECTATOR ) || ( pm->ps->pm_flags & PMF_LIMBO ) ) { // JPW NERVE for limbo mode chase
		PM_CheckDuck();
		PM_FlyMove();
		PM_DropTimers();
		return;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove();
		PM_DropTimers();
		return;
	}

	if ( pm->ps->pm_type == PM_FREEZE ) {
		return;     // no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION ) {
		return;     // no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck();

	// set groundentity
	PM_GroundTrace();

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove();
	}

	// Ridah, ladders
	PM_CheckLadderMove();

	if ( !isDummy ) {
		PM_DropTimers();
	}

	/*if ( pm->ps->powerups[PW_FLIGHT] ) {
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
// RF, removed grapple flag since it's not used
		// Ridah, ladders
		*/
	if ( pml.ladder ) {
		PM_LadderMove();
		// done.
	} else if ( pm->ps->pm_flags & PMF_TIME_WATERJUMP ) {
		PM_WaterJumpMove();
	} else if ( pm->waterlevel > 1 ) {
		// swimming
		PM_WaterMove();
	} else if ( pml.walking ) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove();
	}


	PM_Sprint();


	// Ridah
	if ( !isDummy ) {
		// done.
		PM_Animate();
	}

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// Ridah
	if ( !isDummy ) {
		// done.

		// weapons
		PM_Weapon();

		PM_QuickGrenade();

		// footstep events / legs animations
		PM_Footsteps();

		// entering / leaving water splashes
		PM_WaterEvents();

		// snap some parts of playerstate to save network bandwidth
		trap_SnapVector( pm->ps->velocity );
//		SnapVector( pm->ps->velocity );

		// Ridah
	}
	// done.
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
int Pmove( pmove_t *pmove ) {
	int finalTime;

	// Ridah
	if ( pmove->ps->eFlags & EF_DUMMY_PMOVE ) {
		PmoveSingle( pmove );
		return ( 0 );
	} else if ( pmove->ps->pm_flags & PMF_IGNORE_INPUT ) {
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
		pmove->cmd.buttons = 0;
		pmove->cmd.wbuttons = 0;
		pmove->cmd.wolfkick = 0;
	}
	// done.

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return ( 0 ); // should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	// RF, after a loadgame, prevent huge pmove's
	if ( pmove->ps->pm_flags & PMF_TIME_LOAD ) {
		if ( !pmove->ps->aiChar ) {
			if ( finalTime - pmove->ps->commandTime > 50 ) {
				pmove->ps->commandTime = finalTime - 50;
			}
		} else {
			if ( finalTime - pmove->ps->commandTime > 50 ) {
				pmove->ps->commandTime = finalTime - 50;
			}
		}
	}

	pmove->ps->pmove_framecount = ( pmove->ps->pmove_framecount + 1 ) & ( ( 1 << PS_PMOVEFRAMECOUNTBITS ) - 1 );

	// RF
	pm = pmove;
	PM_AdjustAimSpreadScale();

//	startedTorsoAnim = -1;
//	startedLegAnim = -1;

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		} else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );

		if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
			pmove->cmd.upmove = 20;
		}
	}

	//PM_CheckStuck();

	if ( ( pm->ps->stats[STAT_HEALTH] <= 0 || pm->ps->pm_type == PM_DEAD ) && pml.groundTrace.surfaceFlags & SURF_MONSTERSLICK ) {
		return ( pml.groundTrace.surfaceFlags );
	} else {
		return ( 0 );
	}

}

/*
=================
PM_BeginM97Reload
=================
*/
void PM_BeginM97Reload(void) {
	int anim;
	qboolean fastReload = (pm->ps->perks[PERK_WEAPONHANDLING]);

	// Choose which first person animation to play
	if (pm->ps->ammoclip[BG_FindClipForWeapon(WP_M97)] == 0) {
		anim = fastReload ? WEAP_ALTSWITCHFROM_FAST : WEAP_ALTSWITCHFROM;
		pm->ps->weaponTime += fastReload ? (ammoTable[WP_M97].shotgunPumpStart / 2) : ammoTable[WP_M97].shotgunPumpStart;
		pm->ps->holdable[HI_M97] = M97_RELOADING_BEGIN_PUMP;
	} else {
		anim = fastReload ? WEAP_RELOAD1_FAST : WEAP_RELOAD1;
		pm->ps->weaponTime += fastReload ? (ammoTable[WP_M97].shotgunReloadStart / 2) : ammoTable[WP_M97].shotgunReloadStart;
		pm->ps->holdable[HI_M97] = M97_RELOADING_BEGIN;
	}

	// Play it
	PM_StartWeaponAnim(anim);

	// Initialize override
	pm->pmext->m97reloadInterrupt = qfalse;

	// Set state to reloading
	pm->ps->weaponstate = WEAPON_RELOADING;
}

// Jaymod
void PM_M97Reload() {
	qboolean fastReload = (pm->ps->perks[PERK_WEAPONHANDLING]);

	// Transition from shell + pump
	if (pm->ps->holdable[HI_M97] == M97_RELOADING_BEGIN_PUMP) {

		// Load a shell
		PM_ReloadClip(WP_M97);

		// Branch depending on if we need another shell or not
		int ammoIndex = BG_FindAmmoForWeapon(WP_M97);

		if (!pm->ps->ammo[ammoIndex] || pm->pmext->m97reloadInterrupt)
		{
			// Break back to ready position
			PM_StartWeaponAnim(fastReload ? WEAP_DROP2_FAST : WEAP_DROP2);
			pm->ps->weaponTime += fastReload
									  ? (ammoTable[WP_M97].shotgunPumpEnd / 2)
									  : ammoTable[WP_M97].shotgunPumpEnd;

			pm->ps->weaponstate = WEAPON_READY;
		}
		else
		{
			// Transition to load another shell
			PM_StartWeaponAnim(fastReload ? WEAP_ALTSWITCHTO_FAST : WEAP_ALTSWITCHTO);
			pm->ps->weaponTime += fastReload
									  ? (ammoTable[WP_M97].shotgunPumpLoop / 2)
									  : ammoTable[WP_M97].shotgunPumpLoop;

			pm->ps->holdable[HI_M97] = M97_RELOADING_AFTER_PUMP;
		}
		return;
	}

	// Load a shell on most states
	if (pm->ps->holdable[HI_M97] != M97_RELOADING_AFTER_PUMP && pm->ps->holdable[HI_M97] != M97_RELOADING_BEGIN) {
		PM_ReloadClip(WP_M97);
	}

	// Override - but must load at least one shell!
	if (pm->pmext->m97reloadInterrupt && pm->ps->holdable[HI_M97] != M97_RELOADING_BEGIN) {
		PM_StartWeaponAnim(fastReload ? WEAP_RELOAD3_FAST : WEAP_RELOAD3);
		pm->ps->weaponTime += fastReload ? (ammoTable[WP_M97].shotgunReloadEnd / 2) : ammoTable[WP_M97].shotgunReloadEnd;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}
	// If clip isn't full, load another shell
	int maxclip = BG_GetMaxClip(pm->ps, WP_M97);

	int ammoIndex = BG_FindAmmoForWeapon(WP_M97);

	if (pm->ps->ammoclip[WP_M97] < maxclip && pm->ps->ammo[ammoIndex])
	{
		PM_AddEvent(EV_FILL_CLIP);
		PM_StartWeaponAnim(fastReload ? WEAP_RELOAD2_FAST : WEAP_RELOAD2);
		pm->ps->weaponTime += fastReload
								  ? (ammoTable[WP_M97].shotgunReloadLoop / 2)
								  : ammoTable[WP_M97].shotgunReloadLoop;

		pm->ps->holdable[HI_M97] = M97_RELOADING_LOOP;
	}
	else
	{
		PM_StartWeaponAnim(fastReload ? WEAP_RELOAD3_FAST : WEAP_RELOAD3);
		pm->ps->weaponTime += fastReload
								  ? (ammoTable[WP_M97].shotgunReloadEnd / 2)
								  : ammoTable[WP_M97].shotgunReloadEnd;

		pm->ps->weaponstate = WEAPON_READY;
	}
}


/*
=================
PM_BeginAuto5Reload
=================
*/
void PM_BeginAuto5Reload(void) {
	int anim;
	qboolean fastReload = (pm->ps->powerups[PW_HASTE_SURV] || pm->ps->perks[PERK_WEAPONHANDLING]);

	// Choose which first person animation to play
	if (pm->ps->ammoclip[BG_FindClipForWeapon(WP_AUTO5)] == 0) {
		anim = fastReload ? WEAP_ALTSWITCHFROM_FAST : WEAP_ALTSWITCHFROM;
		PM_AddEvent(EV_M97_PUMP);
		pm->ps->weaponTime += fastReload ? (ammoTable[WP_AUTO5].shotgunPumpStart / 2) : ammoTable[WP_AUTO5].shotgunPumpStart;
		pm->ps->holdable[HI_AUTO5] = AUTO5_RELOADING_BEGIN_PUMP;
	} else {
		anim = fastReload ? WEAP_RELOAD1_FAST : WEAP_RELOAD1;
		pm->ps->weaponTime += fastReload ? (ammoTable[WP_AUTO5].shotgunReloadStart / 2) : ammoTable[WP_AUTO5].shotgunReloadStart;
		pm->ps->holdable[HI_AUTO5] = AUTO5_RELOADING_BEGIN;
	}

	// Play animation
	PM_StartWeaponAnim(anim);

	// Initialize override
	pm->pmext->m97reloadInterrupt = qfalse;

	// Set state to reloading
	pm->ps->weaponstate = WEAPON_RELOADING;
}

void PM_Auto5Reload(void) {
	qboolean fastReload = (pm->ps->perks[PERK_WEAPONHANDLING]);

	// Transition from shell + pump
	if (pm->ps->holdable[HI_AUTO5] == AUTO5_RELOADING_BEGIN_PUMP) {

		// Load a shell
		PM_ReloadClip(WP_AUTO5);

		// Branch depending on if we need another shell or not
		int ammoIndex = BG_FindAmmoForWeapon(WP_AUTO5);

		if (!pm->ps->ammo[ammoIndex] || pm->pmext->m97reloadInterrupt)
		{
			PM_StartWeaponAnim(fastReload ? WEAP_DROP2_FAST : WEAP_DROP2);
			pm->ps->weaponTime += fastReload
									  ? (ammoTable[WP_AUTO5].shotgunPumpEnd / 2)
									  : ammoTable[WP_AUTO5].shotgunPumpEnd;
			pm->ps->weaponstate = WEAPON_READY;
		}
		else
		{
			PM_StartWeaponAnim(fastReload ? WEAP_ALTSWITCHTO_FAST : WEAP_ALTSWITCHTO);
			pm->ps->weaponTime += fastReload
									  ? (ammoTable[WP_AUTO5].shotgunPumpLoop / 2)
									  : ammoTable[WP_AUTO5].shotgunPumpLoop;
			pm->ps->holdable[HI_AUTO5] = AUTO5_RELOADING_AFTER_PUMP;
		}
		return;
	}

	// Load a shell on most states
	if (pm->ps->holdable[HI_AUTO5] != AUTO5_RELOADING_AFTER_PUMP && pm->ps->holdable[HI_AUTO5] != AUTO5_RELOADING_BEGIN) {
		PM_ReloadClip(WP_AUTO5);
	}

	// Override - but must load at least one shell!
	if (pm->pmext->m97reloadInterrupt && pm->ps->holdable[HI_AUTO5] != AUTO5_RELOADING_BEGIN) {
		PM_StartWeaponAnim(fastReload ? WEAP_RELOAD3_FAST : WEAP_RELOAD3);
		pm->ps->weaponTime += fastReload ? (ammoTable[WP_AUTO5].shotgunReloadEnd / 2) : ammoTable[WP_AUTO5].shotgunReloadEnd;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	// If clip isn't full, load another shell
	int maxclip = BG_GetMaxClip(pm->ps, WP_AUTO5);
	int ammoIndex = BG_FindAmmoForWeapon(WP_AUTO5);

	if (pm->ps->ammoclip[WP_AUTO5] < maxclip && pm->ps->ammo[ammoIndex])
	{
		PM_AddEvent(EV_FILL_CLIP);
		PM_StartWeaponAnim(fastReload ? WEAP_RELOAD2_FAST : WEAP_RELOAD2);
		pm->ps->weaponTime += fastReload
								  ? (ammoTable[WP_AUTO5].shotgunReloadLoop / 2)
								  : ammoTable[WP_AUTO5].shotgunReloadLoop;

		pm->ps->holdable[HI_AUTO5] = AUTO5_RELOADING_LOOP;
	}
	else
	{
		PM_StartWeaponAnim(fastReload ? WEAP_RELOAD3_FAST : WEAP_RELOAD3);
		pm->ps->weaponTime += fastReload
								  ? (ammoTable[WP_AUTO5].shotgunReloadEnd / 2)
								  : ammoTable[WP_AUTO5].shotgunReloadEnd;

		pm->ps->weaponstate = WEAPON_READY;
	}
}


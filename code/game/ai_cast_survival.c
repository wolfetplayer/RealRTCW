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

/*
 * name:		ai_cast_survival.c
 *
 * desc:		Wolfenstein AI Character Survival
 *
*/

#include <stdlib.h> // For rand()
#include <stdio.h>  // For snprintf()

#include "g_local.h"
#include "../qcommon/q_shared.h"
#include "../botlib/botlib.h"      //bot lib interface
#include "../botlib/be_aas.h"
#include "../botlib/be_ea.h"
#include "../botlib/be_ai_gen.h"
#include "../botlib/be_ai_goal.h"
#include "../botlib/be_ai_move.h"
#include "../botlib/botai.h"          //bot ai interface

#include "ai_cast.h"
#include "g_survival.h"

#include "../steam/steam.h"

void AICast_CheckSurvivalProgression( gentity_t *attacker ) {
	static char soundDeafultPath[MAX_QPATH] = "sound/announcer/hein.wav";
	static char command[256];

	int i = 0, j;
	int indecies[ANNOUNCE_SOUNDS_COUNT];

	Com_Memset( indecies, 0, sizeof( indecies ) );

    // Wave Change Event
    if (svParams.survivalKillCount == svParams.killCountRequirement) {
        svParams.waveCount++;

		if ((svParams.waveCount == 10) && (!g_cheats.integer) && (!attacker->client->hasPurchased))
		{
			steamSetAchievement("ACH_NO_BUY");
		}

		if ((svParams.waveCount == 15) && (!g_cheats.integer) && (attacker->client->ps.stats[STAT_PLAYER_CLASS] == PC_NONE))
		{
			steamSetAchievement("ACH_NO_CLASS");
		}

		svParams.killCountRequirement += svParams.waveKillCount + rand() % 5;  
		attacker->client->ps.persistant[PERS_WAVES]++;
		svParams.waveKillCount = 0;

		for ( j = 0; j < ANNOUNCE_SOUNDS_COUNT; ++j ) {
			if ( svParams.announcerSound[j][0] ) {
				indecies[i++] = j;
			}
		}

		if ( i == 0 ) {
			snprintf( command, sizeof( command ), "mu_play %s 0\n", soundDeafultPath );
		} else {
			snprintf( command, sizeof( command ), "mu_play %s 0\n", svParams.announcerSound[ indecies[rand( ) % i] ] );
		}
		
		trap_SendServerCommand(-1, command);

   // Normal soldiers
    svParams.maxActiveAI[AICHAR_SOLDIER] += svParams.soldiersIncrease;
    if (svParams.maxActiveAI[AICHAR_SOLDIER] > svParams.maxSoldiers) {
        svParams.maxActiveAI[AICHAR_SOLDIER] = svParams.maxSoldiers;
    }

	// Elite Guards
	if (svParams.waveCount >= svParams.waveEg)
	{
		svParams.maxActiveAI[AICHAR_ELITEGUARD] += svParams.eliteGuardsIncrease;
		if (svParams.maxActiveAI[AICHAR_ELITEGUARD] >  svParams.maxEliteGuards) {
			svParams.maxActiveAI[AICHAR_ELITEGUARD] =  svParams.maxEliteGuards;
		}
	}

	// Black Guards
	if (svParams.waveCount >= svParams.waveBg)
	{
		svParams.maxActiveAI[AICHAR_BLACKGUARD] += svParams.blackGuardsIncrease;
		if (svParams.maxActiveAI[AICHAR_BLACKGUARD] >  svParams.maxBlackGuards) {
			svParams.maxActiveAI[AICHAR_BLACKGUARD] =  svParams.maxBlackGuards;
		}
	}

    // Venoms
	if (svParams.waveCount >= svParams.waveV)
	{
		svParams.maxActiveAI[AICHAR_VENOM] += svParams.venomsIncrease;
		if (svParams.maxActiveAI[AICHAR_VENOM] > svParams.maxVenoms){
			svParams.maxActiveAI[AICHAR_VENOM] = svParams.maxVenoms;
		}
	}

	// Default Zombies
	svParams.maxActiveAI[AICHAR_ZOMBIE_SURV] += svParams.zombiesIncrease;
    if (svParams.maxActiveAI[AICHAR_ZOMBIE_SURV] > svParams.maxZombies) {
        svParams.maxActiveAI[AICHAR_ZOMBIE_SURV] = svParams.maxZombies;
    }

	// Warriors
	if (svParams.waveCount >= svParams.waveWarz)
	{
		svParams.maxActiveAI[AICHAR_WARZOMBIE] += svParams.warriorsIncrease;
		if (svParams.maxActiveAI[AICHAR_WARZOMBIE] > svParams.maxWarriors) {
			svParams.maxActiveAI[AICHAR_WARZOMBIE] = svParams.maxWarriors;
		}
	}

	// Protos
	if (svParams.waveCount >= svParams.waveProtos)
	{
		svParams.maxActiveAI[AICHAR_PROTOSOLDIER] += svParams.protosIncrease;
		if (svParams.maxActiveAI[AICHAR_PROTOSOLDIER] > svParams.maxProtos) {
			svParams.maxActiveAI[AICHAR_PROTOSOLDIER] = svParams.maxProtos;
		}
	}

	// Ghost Zombies
	if (svParams.waveCount >= svParams.waveGhosts)
	{
		svParams.maxActiveAI[AICHAR_ZOMBIE_GHOST] += svParams.ghostsIncrease;
		if (svParams.maxActiveAI[AICHAR_ZOMBIE_GHOST] > svParams.maxGhosts) {
			svParams.maxActiveAI[AICHAR_ZOMBIE_GHOST] = svParams.maxGhosts;
		}
	}

	// Priests
	if (svParams.waveCount >= svParams.wavePriests)
	{
		svParams.maxActiveAI[AICHAR_PRIEST] += svParams.priestsIncrease;
		if (svParams.maxActiveAI[AICHAR_PRIEST] > svParams.maxPriests) {
			svParams.maxActiveAI[AICHAR_PRIEST] = svParams.maxPriests;
		}
	}

	// Partisans
	if (svParams.waveCount >= svParams.wavePartisans)
	{
		svParams.maxActiveAI[AICHAR_PARTISAN] += svParams.partisansIncrease;
		if (svParams.maxActiveAI[AICHAR_PARTISAN] > svParams.maxPartisans) {
			svParams.maxActiveAI[AICHAR_PARTISAN] = svParams.maxPartisans;
		}
	}

    }

}


/*
============
AICast_SurvivalRespawn
============
*/
void AICast_SurvivalRespawn(gentity_t *ent, cast_state_t *cs) {

   vec3_t mins, maxs;
   int touch[10], numTouch;
   float oldmaxZ;
   int i;
   gentity_t *player;
   vec3_t spawn_origin, spawn_angles;

			if ( ent->aiCharacter != AICHAR_ZOMBIE && ent->aiCharacter != AICHAR_HELGA
				 && ent->aiCharacter != AICHAR_HEINRICH ) {

				for ( i = 0 ; i < g_maxclients.integer ; i++ ) {
					player = &g_entities[i];

					if ( !player || !player->inuse ) {
						continue;
					}

					if ( player->r.svFlags & SVF_CASTAI ) {
						continue;
					}
				}
			}

			oldmaxZ = ent->r.maxs[2];

			// make sure the area is clear
			AIChar_SetBBox( ent, cs, qfalse );

			VectorAdd( ent->r.currentOrigin, ent->r.mins, mins );
			VectorAdd( ent->r.currentOrigin, ent->r.maxs, maxs );
			trap_UnlinkEntity( ent );

			numTouch = trap_EntitiesInBox( mins, maxs, touch, 10 );

			if ( numTouch ) {
				for ( i = 0; i < numTouch; i++ ) {
					if ( g_entities[touch[i]].r.contents & MASK_PLAYERSOLID ) {
						break;
					}
				}
				if ( i == numTouch ) {
					numTouch = 0;
				}
			}

			if ( numTouch == 0 ) {    // ok to spawn

			int health_increase = svParams.waveCount * svParams.healthIncreaseMultiplier;
			float speed_increase = svParams.waveCount / svParams.speedIncreaseDivider;
		    float crouchSpeedScale = 1;
			float runSpeedScale = 1;
			float sprintSpeedScale = 1;
			int newHealth = 0; 


			switch (cs->aiCharacter)
			{
			case AICHAR_SOLDIER:
				newHealth = svParams.soldierBaseHealth + health_increase;
				if (newHealth > svParams.soldierHealthCap) {
					newHealth = svParams.soldierHealthCap;
				}
				break;
			case AICHAR_ZOMBIE_SURV:
				newHealth = svParams.zombieBaseHealth + health_increase;
				if (newHealth > svParams.zombieHealthCap) {
					newHealth = svParams.zombieHealthCap;
				}
				runSpeedScale = 0.8 + speed_increase;
				if (runSpeedScale > 1.2) {
					runSpeedScale = 1.2;
				}
				sprintSpeedScale = 1.2 + speed_increase;
				if (runSpeedScale > 1.6) {
					runSpeedScale = 1.6;
				}
				crouchSpeedScale = 0.25 + speed_increase;
				if (crouchSpeedScale > 0.5) {
					crouchSpeedScale = 0.5;
				}
				break;
			case AICHAR_ZOMBIE_GHOST:
				newHealth = svParams.ghostBaseHealth + health_increase;
				if (newHealth > svParams.ghostHealthCap) {
					newHealth = svParams.ghostHealthCap;
				}
				runSpeedScale = 0.8 + speed_increase;
				if (runSpeedScale > 1.6) {
					runSpeedScale = 1.6;
				}
				sprintSpeedScale = 1.2 + speed_increase;
				if (runSpeedScale > 2.0) {
					runSpeedScale = 2.0;
				}
				crouchSpeedScale = 0.25 + speed_increase;
				if (crouchSpeedScale > 0.75) {
					crouchSpeedScale = 0.75;
				}
				break;
			case AICHAR_WARZOMBIE:
				newHealth = svParams.warriorBaseHealth + health_increase;
				if (newHealth > svParams.warriorHealthCap) {
					newHealth = svParams.warriorHealthCap;
				}
				runSpeedScale = 0.8 + speed_increase;
				if (runSpeedScale > 1.6) {
					runSpeedScale = 1.6;
				}
				sprintSpeedScale = 1.2 + speed_increase;
				if (runSpeedScale > 2.0) {
					runSpeedScale = 2.0;
				}
				crouchSpeedScale = 0.25 + speed_increase;
				if (crouchSpeedScale > 0.75) {
					crouchSpeedScale = 0.75;
				}
				break;
			case AICHAR_PROTOSOLDIER:
				newHealth = svParams.protosBaseHealth + health_increase;
				if (newHealth > svParams.protosHealthCap) {
					newHealth = svParams.protosHealthCap;
				}
				runSpeedScale = 0.8 + speed_increase;
				if (runSpeedScale > 1.6) {
					runSpeedScale = 1.6;
				}
				sprintSpeedScale = 1.2 + speed_increase;
				if (runSpeedScale > 1.5) {
					runSpeedScale = 1.5;
				}
				crouchSpeedScale = 0.25 + speed_increase;
				if (crouchSpeedScale > 0.75) {
					crouchSpeedScale = 0.75;
				}
				break;
			case AICHAR_PARTISAN:
				newHealth = svParams.partisansBaseHealth + health_increase;
				if (newHealth > svParams.partisansHealthCap) {
					newHealth = svParams.partisansHealthCap;
				}
				break;
			case AICHAR_PRIEST:
				newHealth = svParams.priestBaseHealth + health_increase;
				if (newHealth > svParams.priestHealthCap) {
					newHealth = svParams.priestHealthCap;
				}
				runSpeedScale = 0.8 + speed_increase;
				if (runSpeedScale > 1.4) {
					runSpeedScale = 1.4;
				}
				sprintSpeedScale = 1.2 + speed_increase;
				if (runSpeedScale > 2.0) {
					runSpeedScale = 2.0;
				}
				crouchSpeedScale = 0.25 + speed_increase;
				if (crouchSpeedScale > 0.5) {
					crouchSpeedScale = 0.5;
				}
				break;
			case AICHAR_ELITEGUARD:
				newHealth = svParams.eliteGuardBaseHealth + health_increase;
				if (newHealth > svParams.eliteGuardHealthCap) {
					newHealth = svParams.eliteGuardHealthCap;
				}
				break;
			case AICHAR_BLACKGUARD:
			    newHealth = svParams.blackGuardBaseHealth + health_increase;
				if (newHealth > svParams.blackGuardHealthCap) {
					newHealth = svParams.blackGuardHealthCap;
				}
				break;
			case AICHAR_VENOM:
			    newHealth = svParams.venomBaseHealth + health_increase;
				if (newHealth > svParams.venomHealthCap) {
					newHealth = svParams.venomHealthCap;
				}
				break;
			default:
				break;
			}

				BG_SetBehaviorForSkill( ent->aiCharacter, g_gameskill.integer );
                
				ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH] = cs->attributes[STARTING_HEALTH] = newHealth;
				ent->client->ps.runSpeedScale = runSpeedScale;
				ent->client->ps.sprintSpeedScale = sprintSpeedScale;
				ent->client->ps.crouchSpeedScale = crouchSpeedScale;				
				ent->r.contents = CONTENTS_BODY;
				ent->clipmask = MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP;
				ent->takedamage = qtrue;
				ent->waterlevel = 0;
				ent->watertype = 0;
				ent->flags = 0;
				ent->die = AICast_Die;
				ent->client->ps.eFlags &= ~EF_DEAD;
				ent->s.eFlags &= ~EF_DEAD;
				player = AICast_FindEntityForName( "player" );

                // Selecting the spawn point for the AI
				SelectSpawnPoint_AI( player, ent, spawn_origin, spawn_angles );
				G_SetOrigin( ent, spawn_origin );
				VectorCopy( spawn_origin, ent->client->ps.origin );
				SetClientViewAngle( ent, spawn_angles );



				// Activate respawn scripts for AI
				AICast_ScriptEvent(cs, "respawn", "");
                
				// Turn off Headshot flag and reattach hat
				ent->client->ps.eFlags &= ~EF_HEADSHOT;
				G_AddEvent( ent, EV_REATTACH_HAT, 0 );

				cs->rebirthTime = 0;
				cs->deathTime = 0;

				ent->client->ps.eFlags &= ~EF_DEATH_FRAME;
				ent->client->ps.eFlags &= ~EF_FORCE_END_FRAME;
				ent->client->ps.eFlags |= EF_NO_TURN_ANIM;

				// play the revive animation
				cs->revivingTime = level.time + BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_REVIVE, qfalse, qtrue );

				AICast_StateChange( cs, AISTATE_RELAXED );
				cs->enemyNum = -1;

			} else {
				// can't spawn yet, so set bbox back, and wait
				ent->r.maxs[2] = oldmaxZ;
				ent->client->ps.maxs[2] = ent->r.maxs[2];
			}
			trap_LinkEntity( ent );


}

// Load survival gamemode parameters from .surv file
void AI_LoadSurvivalTable( const char* mapname )
{
	int handle;
	pc_token_t token;

	handle = trap_PC_LoadSource( va( "maps/%s.surv", mapname ) );
	if ( !handle ) {
		G_Printf( S_COLOR_YELLOW "WARNING: Failed to load .surv file. Trying to load default.surv\n" );

		handle = trap_PC_LoadSource( "maps/default.surv" );

		if ( !handle ) {
			G_Printf( S_COLOR_RED "ERROR: Failed to load default.surv file\n" );
			return;
		}
	}

	memset( &svParams, 0, sizeof( svParams_t ) );

	// Find and parse parameter
	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}
		if ( !Q_stricmp( token.string, "survival" ) ) {
			BG_ParseSurvivalTable( handle );
			break;
		}
	}

	trap_PC_FreeSource( handle );
}

// Read survival parameters into aiDefaults from given file handle
// File handle position expected to be at opening brace of survival block
qboolean BG_ParseSurvivalTable(int handle)
{
	pc_token_t token;
	int i;
	char msg[64];
	char soundPath[MAX_QPATH];

	if (!trap_PC_ReadToken(handle, &token) || Q_stricmp(token.string, "{"))
	{
		PC_SourceError(handle, "expected '{'");
		return qfalse;
	}

	while (1)
	{
		if (!trap_PC_ReadToken(handle, &token))
		{
			break;
		}
		if (token.string[0] == '}')
		{
			break;
		}

		// float
		if (!Q_stricmp(token.string, "healthIncreaseMultiplier"))
		{
			if (!PC_Float_Parse(handle, &svParams.healthIncreaseMultiplier))
			{
				PC_SourceError(handle, "expected healthIncreaseMultiplier value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "speedIncreaseDivider"))
		{
			if (!PC_Float_Parse(handle, &svParams.speedIncreaseDivider))
			{
				PC_SourceError(handle, "expected speedIncreaseDivider value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "spawnTimeFalloffMultiplier"))
		{
			if (!PC_Float_Parse(handle, &svParams.spawnTimeFalloffMultiplier))
			{
				PC_SourceError(handle, "expected spawnTimeFalloffMultiplier value");
				return qfalse;
			}

			// int
		}
		else if (!Q_stricmp(token.string, "initialKillCountRequirement"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialKillCountRequirement))
			{
				PC_SourceError(handle, "expected initialKillCountRequirement value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialSoldiersCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialSoldiersCount))
			{
				PC_SourceError(handle, "expected initialSoldiersCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialEliteGuardsCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialEliteGuardsCount))
			{
				PC_SourceError(handle, "expected initialEliteGuardsCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialBlackGuardsCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialBlackGuardsCount))
			{
				PC_SourceError(handle, "expected initialBlackGuardsCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialVenomsCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialVenomsCount))
			{
				PC_SourceError(handle, "expected initialVenomsCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialZombiesCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialZombiesCount))
			{
				PC_SourceError(handle, "expected initialZombiesCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialWarriorsCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialWarriorsCount))
			{
				PC_SourceError(handle, "expected initialWarriorsCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialProtosCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialProtosCount))
			{
				PC_SourceError(handle, "expected initialProtosCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialPartisansCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialPartisansCount))
			{
				PC_SourceError(handle, "expected initialPartisansCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialGhostsCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialGhostsCount))
			{
				PC_SourceError(handle, "expected initialGhostsCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "initialPriestsCount"))
		{
			if (!PC_Int_Parse(handle, &svParams.initialPriestsCount))
			{
				PC_SourceError(handle, "expected initialPriestsCount value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "soldiersIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.soldiersIncrease))
			{
				PC_SourceError(handle, "expected soldiersIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "eliteGuardsIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.eliteGuardsIncrease))
			{
				PC_SourceError(handle, "expected eliteGuardsIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "blackGuardsIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.blackGuardsIncrease))
			{
				PC_SourceError(handle, "expected blackGuardsIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "venomsIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.venomsIncrease))
			{
				PC_SourceError(handle, "expected venomsIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "zombiesIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.zombiesIncrease))
			{
				PC_SourceError(handle, "expected zombiesIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "warriorsIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.warriorsIncrease))
			{
				PC_SourceError(handle, "expected warriorsIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "protosIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.protosIncrease))
			{
				PC_SourceError(handle, "expected protosIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "partisansIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.partisansIncrease))
			{
				PC_SourceError(handle, "expected partisansIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "ghostsIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.ghostsIncrease))
			{
				PC_SourceError(handle, "expected ghostsIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "priestsIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.priestsIncrease))
			{
				PC_SourceError(handle, "expected priestsIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxSoldiers"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxSoldiers))
			{
				PC_SourceError(handle, "expected maxSoldiers value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxEliteGuards"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxEliteGuards))
			{
				PC_SourceError(handle, "expected maxEliteGuards value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxBlackGuards"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxBlackGuards))
			{
				PC_SourceError(handle, "expected maxBlackGuards value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxVenoms"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxVenoms))
			{
				PC_SourceError(handle, "expected maxVenoms value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxZombies"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxZombies))
			{
				PC_SourceError(handle, "expected maxZombies value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxWarriors"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxWarriors))
			{
				PC_SourceError(handle, "expected maxWarriors value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxProtos"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxProtos))
			{
				PC_SourceError(handle, "expected maxProtos value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxPartisans"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxPartisans))
			{
				PC_SourceError(handle, "expected maxPartisans value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxGhosts"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxGhosts))
			{
				PC_SourceError(handle, "expected maxGhosts value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxPriests"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxPriests))
			{
				PC_SourceError(handle, "expected maxPriests value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "waveEg"))
		{
			if (!PC_Int_Parse(handle, &svParams.waveEg))
			{
				PC_SourceError(handle, "expected waveEg value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "waveBg"))
		{
			if (!PC_Int_Parse(handle, &svParams.waveBg))
			{
				PC_SourceError(handle, "expected waveBg value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "waveV"))
		{
			if (!PC_Int_Parse(handle, &svParams.waveV))
			{
				PC_SourceError(handle, "expected waveV value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "waveWarz"))
		{
			if (!PC_Int_Parse(handle, &svParams.waveWarz))
			{
				PC_SourceError(handle, "expected waveWarz value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "waveProtos"))
		{
			if (!PC_Int_Parse(handle, &svParams.waveProtos))
			{
				PC_SourceError(handle, "expected waveProtos value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "waveGhosts"))
		{
			if (!PC_Int_Parse(handle, &svParams.waveGhosts))
			{
				PC_SourceError(handle, "expected waveGhosts value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "wavePriests"))
		{
			if (!PC_Int_Parse(handle, &svParams.wavePriests))
			{
				PC_SourceError(handle, "expected wavePriests value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "wavePartisans"))
		{
			if (!PC_Int_Parse(handle, &svParams.wavePartisans))
			{
				PC_SourceError(handle, "expected wavePartisans value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "zombieHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.zombieHealthCap))
			{
				PC_SourceError(handle, "expected zombieHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "warriorHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.warriorHealthCap))
			{
				PC_SourceError(handle, "expected warriorHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "protosHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.protosHealthCap))
			{
				PC_SourceError(handle, "expected protosHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "partisansHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.partisansHealthCap))
			{
				PC_SourceError(handle, "expected partisansHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "ghostHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.ghostHealthCap))
			{
				PC_SourceError(handle, "expected ghostHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "priestHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.priestHealthCap))
			{
				PC_SourceError(handle, "expected priestHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "soldierHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.soldierHealthCap))
			{
				PC_SourceError(handle, "expected soldierHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "eliteGuardHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.eliteGuardHealthCap))
			{
				PC_SourceError(handle, "expected eliteGuardHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "blackGuardHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.blackGuardHealthCap))
			{
				PC_SourceError(handle, "expected blackGuardHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "venomHealthCap"))
		{
			if (!PC_Int_Parse(handle, &svParams.venomHealthCap))
			{
				PC_SourceError(handle, "expected venomHealthCap value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "soldierBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.soldierBaseHealth))
			{
				PC_SourceError(handle, "expected soldierBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "partisansBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.partisansBaseHealth))
			{
				PC_SourceError(handle, "expected partisansBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "eliteGuardBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.eliteGuardBaseHealth))
			{
				PC_SourceError(handle, "expected eliteGuardBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "blackGuardBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.blackGuardBaseHealth))
			{
				PC_SourceError(handle, "expected blackGuardBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "venomBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.venomBaseHealth))
			{
				PC_SourceError(handle, "expected venomBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "zombieBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.zombieBaseHealth))
			{
				PC_SourceError(handle, "expected zombieBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "warriorBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.warriorBaseHealth))
			{
				PC_SourceError(handle, "expected warriorBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "protosBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.protosBaseHealth))
			{
				PC_SourceError(handle, "expected protosBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "ghostBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.ghostBaseHealth))
			{
				PC_SourceError(handle, "expected ghostBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "priestBaseHealth"))
		{
			if (!PC_Int_Parse(handle, &svParams.priestBaseHealth))
			{
				PC_SourceError(handle, "expected priestBaseHealth value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "powerupDropChance"))
		{
			if (!PC_Int_Parse(handle, &svParams.powerupDropChance))
			{
				PC_SourceError(handle, "expected powerupDropChance value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "powerupDropChanceScavengerIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.powerupDropChanceScavengerIncrease))
			{
				PC_SourceError(handle, "expected powerupDropChanceScavengerIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "treasureDropChance"))
		{
			if (!PC_Int_Parse(handle, &svParams.treasureDropChance))
			{
				PC_SourceError(handle, "expected treasureDropChance value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "treasureDropChanceScavengerIncrease"))
		{
			if (!PC_Int_Parse(handle, &svParams.treasureDropChanceScavengerIncrease))
			{
				PC_SourceError(handle, "expected treasureDropChanceScavengerIncrease value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "scoreHeadshotKill"))
		{
			if (!PC_Int_Parse(handle, &svParams.scoreHeadshotKill))
			{
				PC_SourceError(handle, "expected scoreHeadshotKill value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "scoreHit"))
		{
			if (!PC_Int_Parse(handle, &svParams.scoreHit))
			{
				PC_SourceError(handle, "expected scoreHit value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "scoreBaseKill"))
		{
			if (!PC_Int_Parse(handle, &svParams.scoreBaseKill))
			{
				PC_SourceError(handle, "expected scoreBaseKill value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "scoreKnifeBonus"))
		{
			if (!PC_Int_Parse(handle, &svParams.scoreKnifeBonus))
			{
				PC_SourceError(handle, "expected scoreKnifeBonus value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "minSpawnTime"))
		{
			if (!PC_Int_Parse(handle, &svParams.minSpawnTime))
			{
				PC_SourceError(handle, "expected minSpawnTime value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "friendlySpawnTime"))
		{
			if (!PC_Int_Parse(handle, &svParams.friendlySpawnTime))
			{
				PC_SourceError(handle, "expected friendlySpawnTime value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxPerks"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxPerks))
			{
				PC_SourceError(handle, "expected maxPerks value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "maxPerksEng"))
		{
			if (!PC_Int_Parse(handle, &svParams.maxPerksEng))
			{
				PC_SourceError(handle, "expected maxPerksEng value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "armorPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.armorPrice))
			{
				PC_SourceError(handle, "expected armorPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "randomPerkPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.randomPerkPrice))
			{
				PC_SourceError(handle, "expected randomPerkPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "randomWeaponPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.randomWeaponPrice))
			{
				PC_SourceError(handle, "expected randomWeaponPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "secondchancePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.secondchancePrice))
			{
				PC_SourceError(handle, "expected secondchancePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "runnerPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.runnerPrice))
			{
				PC_SourceError(handle, "expected runnerPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "scavengerPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.scavengerPrice))
			{
				PC_SourceError(handle, "expected scavengerPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "fasthandsPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.fasthandsPrice))
			{
				PC_SourceError(handle, "expected fasthandsPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "doubleshotPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.doubleshotPrice))
			{
				PC_SourceError(handle, "expected doubleshotPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "resiliencePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.resiliencePrice))
			{
				PC_SourceError(handle, "expected resiliencePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "defaultPerkPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.defaultPerkPrice))
			{
				PC_SourceError(handle, "expected defaultPerkPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "knifePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.knifePrice))
			{
				PC_SourceError(handle, "expected knifePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "lugerPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.lugerPrice))
			{
				PC_SourceError(handle, "expected lugerPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "coltPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.coltPrice))
			{
				PC_SourceError(handle, "expected coltPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "silencerPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.silencerPrice))
			{
				PC_SourceError(handle, "expected silencerPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "tt33Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.tt33Price))
			{
				PC_SourceError(handle, "expected tt33Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "revolverPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.revolverPrice))
			{
				PC_SourceError(handle, "expected revolverPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "akimboPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.akimboPrice))
			{
				PC_SourceError(handle, "expected akimboPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "hdmPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.hdmPrice))
			{
				PC_SourceError(handle, "expected hdmPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "dualtt33Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.dualtt33Price))
			{
				PC_SourceError(handle, "expected dualtt33Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "mp40Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.mp40Price))
			{
				PC_SourceError(handle, "expected mp40Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "stenPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.stenPrice))
			{
				PC_SourceError(handle, "expected stenPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "mp34Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.mp34Price))
			{
				PC_SourceError(handle, "expected mp34Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "thompsonPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.thompsonPrice))
			{
				PC_SourceError(handle, "expected thompsonPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "ppshPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.ppshPrice))
			{
				PC_SourceError(handle, "expected ppshPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "mauserPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.mauserPrice))
			{
				PC_SourceError(handle, "expected mauserPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "mosinPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.mosinPrice))
			{
				PC_SourceError(handle, "expected mosinPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "delislePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.delislePrice))
			{
				PC_SourceError(handle, "expected delislePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "sniperriflePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.sniperriflePrice))
			{
				PC_SourceError(handle, "expected sniperriflePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "snooperScopePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.snooperScopePrice))
			{
				PC_SourceError(handle, "expected snooperScopePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "m1garandPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.m1garandPrice))
			{
				PC_SourceError(handle, "expected m1garandPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "g43Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.g43Price))
			{
				PC_SourceError(handle, "expected g43Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "m1941Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.m1941Price))
			{
				PC_SourceError(handle, "expected m1941Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "mp44Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.mp44Price))
			{
				PC_SourceError(handle, "expected mp44Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "barPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.barPrice))
			{
				PC_SourceError(handle, "expected barPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "fg42Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.fg42Price))
			{
				PC_SourceError(handle, "expected fg42Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "shotgunPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.shotgunPrice))
			{
				PC_SourceError(handle, "expected shotgunPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "auto5Price"))
		{
			if (!PC_Int_Parse(handle, &svParams.auto5Price))
			{
				PC_SourceError(handle, "expected auto5Price value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "mg42mPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.mg42mPrice))
			{
				PC_SourceError(handle, "expected mg42mPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "browningPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.browningPrice))
			{
				PC_SourceError(handle, "expected browningPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "panzerPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.panzerPrice))
			{
				PC_SourceError(handle, "expected panzerPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "flamerPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.flamerPrice))
			{
				PC_SourceError(handle, "expected flamerPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "teslaPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.teslaPrice))
			{
				PC_SourceError(handle, "expected teslaPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "venomPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.venomPrice))
			{
				PC_SourceError(handle, "expected venomPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "grenPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.grenPrice))
			{
				PC_SourceError(handle, "expected grenPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "pineapplePrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.pineapplePrice))
			{
				PC_SourceError(handle, "expected pineapplePrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "defaultWeaponPrice"))
		{
			if (!PC_Int_Parse(handle, &svParams.defaultWeaponPrice))
			{
				PC_SourceError(handle, "expected defaultWeaponPrice value");
				return qfalse;
			}
		}
		else if (!Q_stricmp(token.string, "startingSpawnTime"))
		{
			if (!PC_Int_Parse(handle, &svParams.startingSpawnTime))
			{
				PC_SourceError(handle, "expected startingSpawnTime value");
				return qfalse;
			}
			// string
		}
		else if (!Q_stricmp(token.string, "announcerSound"))
		{
			if (!PC_String_ParseNoAlloc(handle, (char *)&svParams.announcerSound[0], MAX_QPATH))
			{
				PC_SourceError(handle, "expected announcerSound value");
				return qfalse;
			}
		}
		else if (Q_stristr(token.string, "announcerSound") == token.string)
		{
			sscanf(token.string, "announcerSound%d", &i);

			if (!PC_String_ParseNoAlloc(handle, (char *)&soundPath, MAX_QPATH))
			{
				PC_SourceError(handle, "expected announcerSound value");
				return qfalse;
			}

			if (i - 1 >= ANNOUNCE_SOUNDS_COUNT)
			{
				sprintf(msg, "announcerSound[%d] out of range. Increase ANNOUNCE_SOUNDS_COUNT", i - 1);
				PC_SourceError(handle, msg);
			}
			else
			{
				strcpy(svParams.announcerSound[i - 1], soundPath);
			}
		}
		else
		{
			PC_SourceError(handle, "unknown token '%s'", token.string);
			return qfalse;
		}
	}

	return qtrue;
}

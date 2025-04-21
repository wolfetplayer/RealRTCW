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

// g_survival_misc.c

#include "g_local.h"
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
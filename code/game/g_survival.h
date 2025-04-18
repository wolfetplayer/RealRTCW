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



// g_survival.h

#ifndef __G_SURVIVAL_H__
#define __G_SURVIVAL_H__

// Everything related to the score system
void Survival_AddKillScore(gentity_t *attacker, gentity_t *victim, int meansOfDeath);
void Survival_AddHeadshotBonus(gentity_t *attacker, gentity_t *victim);
void Survival_AddPainScore(gentity_t *attacker, gentity_t *victim, int damage);
void Survival_PickupTreasure(gentity_t *other);
qboolean Survival_TrySpendMG42Points(gentity_t *player);

// Purchase system
void Use_Target_buy(gentity_t *ent, gentity_t *other, gentity_t *activator);
qboolean Survival_HandleRandomWeaponBox(gentity_t *ent, gentity_t *activator, char *itemName, int *itemIndex);
qboolean Survival_HandleRandomPerkBox(gentity_t *ent, gentity_t *activator, char **itemName, int *itemIndex);
qboolean Survival_HandleAmmoPurchase(gentity_t *ent, gentity_t *activator, int price);
qboolean Survival_HandleWeaponOrGrenade(gentity_t *ent, gentity_t *activator, gitem_t *item, int price);
qboolean Survival_HandleArmorPurchase(gentity_t *activator, gitem_t *item, int price);
qboolean Survival_HandlePerkPurchase(gentity_t *activator, gitem_t *item, int price);
int Survival_GetDefaultWeaponPrice(int weapon);
int Survival_GetDefaultPerkPrice(int perk);

// Misc stuff
void TossClientItems(gentity_t *self, gentity_t *attacker);
void TossClientPowerups(gentity_t *self, gentity_t *attacker);


// Survival parameters
typedef struct svParams_s
{
	// not loaded
	int activeAI[NUM_CHARACTERS];
	int survivalKillCount;
	int maxActiveAI[NUM_CHARACTERS];
	int waveCount;
	int waveKillCount;
	int killCountRequirement;

	// loaded from .surv file
	int initialKillCountRequirement;

	int initialSoldiersCount;
	int initialEliteGuardsCount;
	int initialBlackGuardsCount;
	int initialVenomsCount;

	int initialZombiesCount;
	int initialWarriorsCount;
	int initialProtosCount;
	int initialGhostsCount;
	int initialPriestsCount;
	int initialPartisansCount;

	float healthIncreaseMultiplier;
	float speedIncreaseDivider;

	float spawnTimeFalloffMultiplier;
	int   minSpawnTime;
	int   startingSpawnTime;
    int   friendlySpawnTime;

	int soldiersIncrease;
	int eliteGuardsIncrease;
	int blackGuardsIncrease;
	int venomsIncrease;
	int zombiesIncrease;
	int warriorsIncrease;
	int protosIncrease;
	int partisansIncrease;
	int ghostsIncrease;
	int priestsIncrease;

	int maxSoldiers;
	int maxEliteGuards;
	int maxBlackGuards;
	int maxVenoms;

	int maxZombies;
	int maxWarriors;
	int maxProtos;
	int maxGhosts;
	int maxPriests;
	int maxPartisans;

	int waveEg;
	int waveBg;
	int waveV;

	int waveWarz;
	int waveProtos;
	int waveGhosts;
	int wavePriests;

	int wavePartisans;

	int zombieHealthCap;
	int warriorHealthCap;
	int protosHealthCap;
	int ghostHealthCap;
	int priestHealthCap;

	int partisansHealthCap;

	int soldierHealthCap;	
	int eliteGuardHealthCap;
	int blackGuardHealthCap;
	int venomHealthCap;

	int soldierBaseHealth;
	int eliteGuardBaseHealth;
	int blackGuardBaseHealth;
	int venomBaseHealth;

	int partisansBaseHealth;

	int zombieBaseHealth;
	int warriorBaseHealth;
	int protosBaseHealth;
	int ghostBaseHealth;
	int priestBaseHealth;

	int powerupDropChance;
	int powerupDropChanceScavengerIncrease;

	int treasureDropChance;
	int treasureDropChanceScavengerIncrease;

	int scoreHeadshotKill;
	int scoreHit;
	int scoreBaseKill;
	int scoreKnifeBonus;

	int maxPerks;
	int maxPerksEng;

	int armorDefaultPrice;
	int randomPerkDefaultPrice;
	int randomWeaponDefaultPrice;

	char announcerSound[ANNOUNCE_SOUNDS_COUNT][MAX_QPATH];

} svParams_t;

extern svParams_t svParams;

#endif // __G_SURVIVAL_H__
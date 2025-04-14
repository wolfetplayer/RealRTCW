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

// g_survival_score.c

#include "g_local.h"
#include "g_survival.h"

/*
============
Survival_AddKillScore
============
*/
void Survival_AddKillScore(gentity_t *attacker, gentity_t *victim, int meansOfDeath) {
	if (!attacker || !attacker->client || !victim)
		return;

	if (attacker->aiTeam == victim->aiTeam)
		return; // no score for teamkills

	int score = svParams.scoreBaseKill;

	if (meansOfDeath == MOD_KNIFE || meansOfDeath == MOD_KICKED) {
		score += svParams.scoreKnifeBonus;
	}

	attacker->client->ps.persistant[PERS_SCORE] += score;
	attacker->client->ps.persistant[PERS_KILLS]++;
}

/*
============
Survival_AddHeadshotBonus
============
*/
void Survival_AddHeadshotBonus(gentity_t *attacker, gentity_t *victim) {
	if (!attacker || !victim || !attacker->client || attacker->aiTeam == victim->aiTeam)
		return;

	attacker->client->ps.persistant[PERS_SCORE] += svParams.scoreHeadshotKill;
}

/*
============
Survival_AddPainScore
============
*/
void Survival_AddPainScore(gentity_t *attacker, gentity_t *victim, int damage) {
	if (!attacker || !victim || !attacker->client)
		return;

	if (attacker->aiTeam == victim->aiTeam)
		return;

	// Vampire perk healing
	if (attacker->client->ps.powerups[PW_VAMPIRE]) {
		attacker->health += 5;
		if (attacker->health > 300) {
			attacker->health = 300;
		}
	}

	attacker->client->ps.persistant[PERS_SCORE] += svParams.scoreHit;
}

/*
============
Survival_PickupTreasure
============
*/
void Survival_PickupTreasure(gentity_t *other) {
	if (!other || !other->client)
		return;

	// Generate a random score between 50 and 100
	int randomScore = 50 + rand() % 51;
	other->client->ps.persistant[PERS_SCORE] += randomScore;
}

/*
============
Survival_TrySpendMG42Points
============
*/
qboolean Survival_TrySpendMG42Points(gentity_t *player) {
	if (!player || !player->client)
		return qfalse;

	// Require 1 score point to shoot
	if (player->client->ps.persistant[PERS_SCORE] < 1)
		return qfalse;

	player->client->ps.persistant[PERS_SCORE] -= 1;
	return qtrue;
}



/*QUAKED target_buy (1 0 0) (-8 -8 -8) (8 8 8)
Gives the activator all the items pointed to.
*/
void Use_Target_buy( gentity_t *ent, gentity_t *other, gentity_t *activator ) {

    int itemIndex = 0;
    int i;
    int price;
    char *itemName;
	gitem_t *item;

    price = ent->price;
    itemName = ent->buy_item;

	int clientNum;
	clientNum = level.sortedClients[0];

	// Define the list of random box weapons
    weapon_t random_box_weapons[] = {                           
	// One handed pistols
	WP_LUGER,              
	WP_SILENCER,           
    WP_COLT,               
	WP_TT33,               
	WP_REVOLVER,
	WP_DUAL_TT33,
	WP_AKIMBO,
	WP_HDM,            
	// SMGs
	WP_MP40,             
	WP_THOMPSON,         
	WP_STEN,             
	WP_PPSH,             
	WP_MP34,             
	// Rifles
	WP_MAUSER,              
	WP_SNIPERRIFLE,            
	WP_SNOOPERSCOPE,
	WP_MOSIN,
	// Semi auto rifles
	WP_M1GARAND,
	WP_G43,
	// Assault Rifles
	WP_MP44,
	WP_FG42,
	WP_BAR,
	// Shotguns
	WP_M97,
	// Heavy Weapons
	WP_BROWNING,
	WP_MG42M,
	WP_PANZERFAUST,
	WP_FLAMETHROWER,
	// Secret Weapons
	WP_VENOM,
	WP_TESLA
	};

	// Define the list of random box weapons
    weapon_t random_box_weapons_dlc[] = { 	            
	// One handed pistols
	WP_LUGER,              
	WP_SILENCER,           
    WP_COLT,               
	WP_TT33,               
	WP_REVOLVER,
	WP_DUAL_TT33,
	WP_AKIMBO,
	WP_HDM,           
	// SMGs
	WP_MP40,             
	WP_THOMPSON,         
	WP_STEN,             
	WP_PPSH,             
	WP_MP34,             
	// Rifles
	WP_MAUSER,              
	WP_SNIPERRIFLE,             
	WP_SNOOPERSCOPE,
	WP_MOSIN,
	// Semi auto rifles
	WP_M1GARAND,
	WP_G43,
	WP_M1941,
	// Assault Rifles
	WP_MP44,
	WP_FG42,
	WP_BAR,
	// Shotguns
	WP_M97,
	WP_AUTO5, 
	// Heavy Weapons
	WP_BROWNING,
	WP_MG42M,
	WP_PANZERFAUST,
	WP_FLAMETHROWER,
	// Secret Weapons
	WP_VENOM,
	WP_TESLA,   
	WP_DELISLE
	};

    char *random_perks[] = {"perk_resilience", "perk_scavenger", "perk_runner", "perk_weaponhandling", "perk_rifling", "perk_secondchance"}; 

    // Check if weapon or price were not specified
    if ( !itemName ) {
        return;
    }

    if ( !price || price < 0 ) {
        price = 0;
    }

    if ( !activator->client ) {
        return;
    }

	if (strcmp(itemName, "random_weapon") == 0)
	{
		weapon_t* selected_weapons;
		int numWeapons, randomIndex;

		if (g_dlc1.integer == 1)
		{
			selected_weapons = random_box_weapons_dlc;
			numWeapons = sizeof(random_box_weapons_dlc) / sizeof(random_box_weapons_dlc[0]);
		}
		else
		{
			selected_weapons = random_box_weapons;
			numWeapons = sizeof(random_box_weapons) / sizeof(random_box_weapons[0]);
		}

		// Generate a random index
		do {
			randomIndex = rand() % numWeapons;

		} while ( G_FindWeaponSlot( activator, selected_weapons[ randomIndex ] ) >= 0 );

		// Find the item
		for ( i = 1; bg_itemlist[i].classname; i++ ) {
			if ( bg_itemlist[i].giWeapon == selected_weapons[ randomIndex ] ) {
				itemIndex = i;
				break;
			}
		}
	}

	if (strcmp(itemName, "random_perk") == 0)
    {
        char **selected_perks;
        int numPerks;

        selected_perks = random_perks;
        numPerks = sizeof(random_perks) / sizeof(random_perks[0]);
        

        int randomIndex = rand() % numPerks;	  // Generate a random index
        itemName = selected_perks[randomIndex]; // Select a random perk

		// Find the item
		for ( i = 1; bg_itemlist[i].classname; i++ ) {
			if ( !Q_strcasecmp( itemName, bg_itemlist[i].classname ) ) {
				itemIndex = i;
				break;
			}
		}
    }


	    //
    // --------------------------------------------
    // NEW: Handle "ammo" purchase
    // --------------------------------------------
    //
    if (!Q_stricmp(itemName, "ammo")) {
        // Figure out which weapon the player is currently holding
        int heldWeap = activator->client->ps.weapon;

        // If the held weapon is invalid or a no-ammo type, ignore
        if ( heldWeap <= WP_NONE || heldWeap >= WP_NUM_WEAPONS ) {
            trap_SendServerCommand( -1, "mu_play sound/items/use_nothing.wav 0\n" );
            return;
        }

		// Check if weapon ammo is already at max
		int ammoIndex = BG_FindAmmoForWeapon(heldWeap);
		if (activator->client->ps.ammo[ammoIndex] >= ammoTable[heldWeap].maxammo)
		{
			trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
			return;
		}

		// Weapons that only a Soldier can replenish
        // also costs double for ANY class (but only a soldier can do it).
        if ( heldWeap == WP_TESLA
          || heldWeap == WP_MG42M
          || heldWeap == WP_PANZERFAUST
          || heldWeap == WP_VENOM
          || heldWeap == WP_FLAMETHROWER
		  || heldWeap == WP_BROWNING ) {

            // Only a soldier can buy ammo for these
            if ( activator->client->ps.stats[STAT_PLAYER_CLASS] != PC_SOLDIER ) {
                trap_SendServerCommand( -1, "mu_play sound/items/use_nothing.wav 0\n" );
                return;
            }
            // Double the price for these heavy weapons
            price *= 2;
        }

        // If the player is a Lieutenant, halve the price
        if ( activator->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ) {
            price /= 2;
        }

        // Check if player can afford it
        if ( activator->client->ps.persistant[PERS_SCORE] < price ) {
            trap_SendServerCommand( -1, "mu_play sound/items/use_nothing.wav 0\n" );
            return;
        }

        // Refill ammo to max
        Add_Ammo( activator, heldWeap, ammoTable[heldWeap].maxammo, qtrue );
        Add_Ammo( activator, heldWeap, ammoTable[heldWeap].maxammo, qfalse );

        // Deduct the points
        activator->client->ps.persistant[PERS_SCORE] -= price;

		trap_SendServerCommand( -1, "mu_play sound/misc/buy.wav 0\n" );

        // Update userinfo
        activator->client->hasPurchased = qtrue;
        ClientUserinfoChanged( clientNum );
        return;
    }

	// Find the item
	if ( itemIndex <= 0 ) {
		for ( i = 1; bg_itemlist[i].classname; i++ ) {
			if ( !Q_strcasecmp( itemName, bg_itemlist[i].classname ) ) {
				itemIndex = i;
				break;
			}
		}
	}

    item = &bg_itemlist[itemIndex];

    // Check if player has enough points
    if (activator->client->ps.persistant[PERS_SCORE] < price) {
        trap_SendServerCommand( -1, "mu_play sound/items/use_nothing.wav 0\n" );
        return;  // Player doesn't have enough points, return without giving anything
    }

    if ( item->giType == IT_WEAPON ) {

        // Check if player already has the weapon
        if (COM_BitCheck(activator->client->ps.weapons, item->giTag)) {
            // Player already has the weapon, give ammo instead and halve the price
            price /= 2;
        } else {
            // Player doesn't have the weapon, give it to them
			Give_Weapon_New_Inventory( activator, item->giTag, qfalse );
        }

        // Check if player's ammo is already full
        if (activator->client->ps.ammo[item->giTag] >= ammoTable[item->giTag].maxammo) {
            return;  // Player's ammo is already full, return without adding ammo
        }

        // Set the ammo of the bought weapon to the "maxammo" from the ammo table
        Add_Ammo( activator, item->giTag, ammoTable[item->giTag].maxammo, qtrue );
		Add_Ammo( activator, item->giTag, ammoTable[item->giTag].maxammo, qfalse );

		// Give player GL and ammo if they buy the M1 Garand
		if (item->giTag == WP_M1GARAND)
		{
			Give_Weapon_New_Inventory( activator, WP_M7, qfalse );
			Add_Ammo(activator, WP_M7, ammoTable[WP_M7].maxammo, qfalse);
		}

		// Select the bought weapon
        G_AddPredictableEvent( activator, EV_ITEM_PICKUP, BG_FindItemForWeapon( item->giTag ) - bg_itemlist );

		trap_SendServerCommand( -1, "mu_play sound/misc/buy.wav 0\n" );

	// all grenades should be IT_AMMO
    } else if ( item->giType == IT_AMMO ) {
		// Check if player's ammo is already full
		// ammoclip for grenades
        if ( activator->client->ps.ammoclip[ item->giTag ] >= ammoTable[ item->giTag ].maxammo ) {
            return;  // Player's ammo is already full, return without adding ammo
        }

        // Check if player already has the weapon
        if ( COM_BitCheck( activator->client->ps.weapons, item->giTag ) ) {
            // Player already has the weapon, give ammo instead and halve the price
            price /= 2;
        }

        // Set the ammo of the bought weapon to the "maxammo" from the ammo table
        Add_Ammo( activator, item->giTag, ammoTable[ item->giTag ].maxammo, qtrue );
		Add_Ammo( activator, item->giTag, ammoTable[item->giTag].maxammo, qfalse );

        // Select the bought weapon
        G_AddPredictableEvent( activator, EV_ITEM_PICKUP, BG_FindItemForWeapon( item->giTag ) - bg_itemlist );
		trap_SendServerCommand( -1, "mu_play sound/misc/buy.wav 0\n" );

    } else if ( item->giType == IT_ARMOR )  {
       if (activator->client->ps.stats[STAT_ARMOR] >= 200) {
		  trap_SendServerCommand( -1, "mu_play sound/items/use_nothing.wav 0\n" );
          return;
       }
		activator->client->ps.stats[STAT_ARMOR] = 200;
        G_AddPredictableEvent( activator, EV_ITEM_PICKUP, item - bg_itemlist );
		trap_SendServerCommand( -1, "mu_play sound/misc/buy.wav 0\n" );
    } else if ( item->giType == IT_PERK ) {

		int i, perkCount = 0;

		// Count the number of perks the player already has
		for (i = 0; i < MAX_PERKS; i++)
		{
			if (activator->client->ps.perks[i] > 0)
			{
				perkCount++;
			}
		}

		// If the player already has 3 perks, don't allow them to buy more
		if (activator->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER)
		{
			if (perkCount >= 4)
			{
				trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
				return;
			}
		}
		else
		{
			if (perkCount >= 3)
			{
				trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
				return;
			}
		}

		if (activator->client->ps.perks[item->giTag] > 0) {
        // The player already has the perk, so don't give it to them again
		trap_SendServerCommand( -1, "mu_play sound/items/use_nothing.wav 0\n" );
        return;
        }

	   activator->client->ps.perks[item->giTag] += 1;
	   activator->client->ps.stats[STAT_PERK] |= ( 1 << item->giTag );
       G_AddPredictableEvent( activator, EV_ITEM_PICKUP, item - bg_itemlist );
	   	trap_SendServerCommand( -1, "mu_play sound/misc/buy_perk.wav 0\n" );
	} else {
		return;
	}

	// Subtract price from player's score
    activator->client->ps.persistant[PERS_SCORE] -= price;

	activator->client->hasPurchased = qtrue;
	
	ClientUserinfoChanged( clientNum );
}
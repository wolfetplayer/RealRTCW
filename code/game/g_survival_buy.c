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

// g_survival_buy.c

#include "g_local.h"
#include "g_survival.h"

int Survival_GetDefaultWeaponPrice(int weapon) {
	switch (weapon) {
		// Pistols
		case WP_LUGER:        return 30;
		case WP_SILENCER:     return 30;
		case WP_COLT:         return 30;
		case WP_TT33:         return 30;
		case WP_REVOLVER:     return 30;
		case WP_DUAL_TT33:    return 50;
		case WP_AKIMBO:       return 50;
		case WP_HDM:          return 30;

		// SMGs
		case WP_STEN:         return 90;
		case WP_MP40:         return 100;
		case WP_MP34:         return 120;
		case WP_THOMPSON:     return 120;
		case WP_PPSH:         return 150;

		// Rifles
		case WP_MAUSER:       return 50;
		case WP_MOSIN:        return 50;
		case WP_DELISLE:      return 50;
		case WP_SNIPERRIFLE:  return 100;
		case WP_SNOOPERSCOPE: return 150;

		// Auto Rifles
		case WP_M1GARAND:     return 150;
		case WP_G43:          return 120;
		case WP_M1941:        return 120;

		// Assault Rifles
		case WP_MP44:         return 200;
		case WP_FG42:         return 200;
		case WP_BAR:          return 200;

		// Shotguns
		case WP_M97:          return 180;
		case WP_AUTO5:        return 200;

		// Heavy
		case WP_MG42M:        return 300;
		case WP_PANZERFAUST:  return 400;
		case WP_BROWNING:     return 400;
		case WP_FLAMETHROWER: return 500;
		case WP_VENOM:        return 500;
		case WP_TESLA:        return 500;

		// Grenades
		case WP_GRENADE_LAUNCHER:   return 150;
		case WP_GRENADE_PINEAPPLE:  return 150;

		default: return 100;
	}
}

/*
============
Survival_HandleRandomWeaponBox
============
*/
qboolean Survival_HandleRandomWeaponBox(gentity_t *ent, gentity_t *activator, char *itemName, int *itemIndex) {
	if (!activator || !activator->client) return qfalse;

	static const weapon_t random_box_weapons[] = {
		WP_LUGER, WP_SILENCER, WP_COLT, WP_TT33, WP_REVOLVER, WP_DUAL_TT33,
		WP_AKIMBO, WP_HDM, WP_MP40, WP_THOMPSON, WP_STEN, WP_PPSH, WP_MP34,
		WP_MAUSER, WP_SNIPERRIFLE, WP_SNOOPERSCOPE, WP_MOSIN,
		WP_M1GARAND, WP_G43, WP_MP44, WP_FG42, WP_BAR, WP_M97,
		WP_BROWNING, WP_MG42M, WP_PANZERFAUST, WP_FLAMETHROWER, WP_VENOM, WP_TESLA
	};

	static const weapon_t random_box_weapons_dlc[] = {
		WP_LUGER, WP_SILENCER, WP_COLT, WP_TT33, WP_REVOLVER, WP_DUAL_TT33,
		WP_AKIMBO, WP_HDM, WP_MP40, WP_THOMPSON, WP_STEN, WP_PPSH, WP_MP34,
		WP_MAUSER, WP_SNIPERRIFLE, WP_SNOOPERSCOPE, WP_MOSIN,
		WP_M1GARAND, WP_G43, WP_M1941, WP_MP44, WP_FG42, WP_BAR, WP_M97, WP_AUTO5,
		WP_BROWNING, WP_MG42M, WP_PANZERFAUST, WP_FLAMETHROWER, WP_VENOM, WP_TESLA, WP_DELISLE
	};

	const weapon_t *selected_weapons = g_dlc1.integer ? random_box_weapons_dlc : random_box_weapons;
	int numWeapons = g_dlc1.integer 
		? sizeof(random_box_weapons_dlc) / sizeof(random_box_weapons_dlc[0]) 
		: sizeof(random_box_weapons) / sizeof(random_box_weapons[0]);

	int price = ent->price > 0 ? ent->price : 150;

	if (activator->client->ps.persistant[PERS_SCORE] < price) {
		trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
		return qfalse;
	}

	// Pick a random weapon the player doesn't have
	weapon_t chosen;
	int tries = 10;
	do {
		chosen = selected_weapons[rand() % numWeapons];
		tries--;
	} while (G_FindWeaponSlot(activator, chosen) >= 0 && tries > 0);

	if (tries <= 0) {
		trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
		return qfalse;
	}

	// Find the item
	for (int i = 1; bg_itemlist[i].classname; i++) {
		if (bg_itemlist[i].giWeapon == chosen) {
			*itemIndex = i;
			itemName = bg_itemlist[i].classname;
			gitem_t *item = &bg_itemlist[i];

			// Give weapon
			Give_Weapon_New_Inventory(activator, chosen, qfalse);

			// Give full ammo
			Add_Ammo(activator, chosen, ammoTable[chosen].maxammo, qtrue);
			Add_Ammo(activator, chosen, ammoTable[chosen].maxammo, qfalse);

			// Bonus: give M7 for Garand
			if (chosen == WP_M1GARAND) {
				Give_Weapon_New_Inventory(activator, WP_M7, qfalse);
				Add_Ammo(activator, WP_M7, ammoTable[WP_M7].maxammo, qfalse);
			}

			// Select weapon
			activator->client->ps.weapon = chosen;
			activator->client->ps.weaponstate = WEAPON_READY;

			// Deduct points
			activator->client->ps.persistant[PERS_SCORE] -= price;

			// SFX & confirmation
			G_AddPredictableEvent(activator, EV_ITEM_PICKUP, item - bg_itemlist);
			trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");

			return qtrue;
		}
	}

	return qfalse;
}

/*
============
Survival_HandleRandomPerkBox
============
*/
qboolean Survival_HandleRandomPerkBox(gentity_t *ent, gentity_t *activator, char **itemName, int *itemIndex) {
	if (!activator || !activator->client) return qfalse;

	static char *random_perks[] = {
		"perk_resilience", "perk_scavenger", "perk_runner",
		"perk_weaponhandling", "perk_rifling", "perk_secondchance"
	};

	const int price = 200;
	const int numPerks = sizeof(random_perks) / sizeof(random_perks[0]);

	// Perk count limit
	int perkCount = 0;
	for (int i = 0; i < MAX_PERKS; i++) {
		if (activator->client->ps.perks[i] > 0)
			perkCount++;
	}
	int maxPerks = (activator->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) ? svParams.maxPerksEng : svParams.maxPerks;
	if (perkCount >= maxPerks) {
		G_AddEvent(activator, EV_GENERAL_SOUND, G_SoundIndex("sound/items/use_nothing.wav"));
		return qfalse;
	}

	int randomIndex = rand() % numPerks;
	*itemName = random_perks[randomIndex];

	for (int i = 1; bg_itemlist[i].classname; i++) {
		if (!Q_strcasecmp(*itemName, bg_itemlist[i].classname)) {
			*itemIndex = i;
			gitem_t *perkItem = &bg_itemlist[i];

			if (activator->client->ps.perks[perkItem->giTag] > 0 || 
				activator->client->ps.persistant[PERS_SCORE] < price) {
				G_AddEvent(activator, EV_GENERAL_SOUND, G_SoundIndex("sound/items/use_nothing.wav"));
				return qfalse;
			}

			activator->client->ps.perks[perkItem->giTag]++;
			activator->client->ps.stats[STAT_PERK] |= (1 << perkItem->giTag);
			activator->client->ps.persistant[PERS_SCORE] -= price;

			G_AddPredictableEvent(activator, EV_ITEM_PICKUP, perkItem - bg_itemlist);
			trap_SendServerCommand(-1, "mu_play sound/misc/buy_perk.wav 0\n");
			return qtrue;
		}
	}

	return qfalse;
}

/*
============
Survival_HandleAmmoPurchase
============
*/
qboolean Survival_HandleAmmoPurchase(gentity_t *ent, gentity_t *activator, int price) {
	if (!activator || !activator->client)
		return qfalse;

	int heldWeap = activator->client->ps.weapon;
	if (heldWeap <= WP_NONE || heldWeap >= WP_NUM_WEAPONS)
		return qfalse;

	int ammoIndex = BG_FindAmmoForWeapon(heldWeap);
	if (ammoIndex < 0 || activator->client->ps.ammo[ammoIndex] >= ammoTable[heldWeap].maxammo)
		return qfalse;

	// Use fallback price: half of weapon price
	int basePrice = Survival_GetDefaultWeaponPrice(heldWeap);
	int ammoPrice = basePrice / 2;

	// Mapper override
	if (price > 0) {
		ammoPrice = price;
	}

	if (activator->client->ps.persistant[PERS_SCORE] < ammoPrice) {
		trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
		return qfalse;
	}

	// Refill ammo
	Add_Ammo(activator, heldWeap, ammoTable[heldWeap].maxammo, qtrue);
	Add_Ammo(activator, heldWeap, ammoTable[heldWeap].maxammo, qfalse);

	activator->client->ps.persistant[PERS_SCORE] -= ammoPrice;

	trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");
	return qtrue;
}


/*
============
Survival_HandleWeaponOrGrenade
============
*/
qboolean Survival_HandleWeaponOrGrenade(gentity_t *ent, gentity_t *activator, gitem_t *item, int price) {
	if (!activator || !item) return qfalse;

	// Use fallback price if mapper didn't define one
	if (price <= 0) {
		price = Survival_GetDefaultWeaponPrice(item->giTag);
	}

	// Halve price if already owned
	if (COM_BitCheck(activator->client->ps.weapons, item->giTag)) {
		price /= 2;
	} else {
		Give_Weapon_New_Inventory(activator, item->giTag, qfalse);
	}

	// Check if ammo is already full
	if (item->giType == IT_AMMO) {
		if (activator->client->ps.ammoclip[item->giTag] >= ammoTable[item->giTag].maxammo) return qfalse;
	} else if (activator->client->ps.ammo[item->giTag] >= ammoTable[item->giTag].maxammo) {
		return qfalse;
	}

	// Check score
	if (activator->client->ps.persistant[PERS_SCORE] < price) {
		G_AddEvent(activator, EV_GENERAL_SOUND, G_SoundIndex("sound/items/use_nothing.wav"));
		return qfalse;
	}

	// Grant ammo
	Add_Ammo(activator, item->giTag, ammoTable[item->giTag].maxammo, qtrue);
	Add_Ammo(activator, item->giTag, ammoTable[item->giTag].maxammo, qfalse);

	// M1Garand bonus: give M7 with ammo
	if (item->giTag == WP_M1GARAND) {
		Give_Weapon_New_Inventory(activator, WP_M7, qfalse);
		Add_Ammo(activator, WP_M7, ammoTable[WP_M7].maxammo, qfalse);
	}

	// Deduct score and notify
	activator->client->ps.persistant[PERS_SCORE] -= price;
	G_AddPredictableEvent(activator, EV_ITEM_PICKUP, item - bg_itemlist);
	trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");

	return qtrue;
}

/*
============
Survival_HandleArmorPurchase
============
*/
qboolean Survival_HandleArmorPurchase(gentity_t *activator, gitem_t *item, int price) {
	if (!activator || !item || !activator->client) return qfalse;

	if (activator->client->ps.stats[STAT_ARMOR] >= 200)
		return qfalse;

	// Fallback price if not set by mapper
	if (price <= 0)
		price = svParams.armorDefaultPrice;

	// Check score
	if (activator->client->ps.persistant[PERS_SCORE] < price) {
		trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
		return qfalse;
	}

	// Deduct, apply, and notify
	activator->client->ps.persistant[PERS_SCORE] -= price;
	activator->client->ps.stats[STAT_ARMOR] = 200;

	G_AddPredictableEvent(activator, EV_ITEM_PICKUP, item - bg_itemlist);
	trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");

	return qtrue;
}

/*
============
Survival_GetDefaultPerkPrice
============
*/
int Survival_GetDefaultPerkPrice(int perk) {
	switch (perk) {
		case PERK_SECONDCHANCE:    return 150;
		case PERK_RUNNER:          return 200;
		case PERK_SCAVENGER:       return 250;
		case PERK_WEAPONHANDLING:  return 300;
		case PERK_RIFLING:         return 350;
		case PERK_RESILIENCE:      return 400;
		default:                   return 200;
	}
}


/*
============
Survival_HandlePerkPurchase
============
*/
qboolean Survival_HandlePerkPurchase(gentity_t *activator, gitem_t *item, int price) {
	if (!activator || !item || item->giType != IT_PERK)
		return qfalse;

	// Count how many perks player has
	int perkCount = 0;
	for (int i = 0; i < MAX_PERKS; i++) {
		if (activator->client->ps.perks[i] > 0)
			perkCount++;
	}

	// Max perks check
	int maxPerks = (activator->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) ?  svParams.maxPerksEng : svParams.maxPerks;
	if (perkCount >= maxPerks)
		return qfalse;

	// Already owns this perk?
	if (activator->client->ps.perks[item->giTag] > 0)
		return qfalse;

	// Fallback to default price if mapper didn't define it
	if (price <= 0) {
		price = Survival_GetDefaultPerkPrice(item->giTag);
	}

	// Not enough score?
	if (activator->client->ps.persistant[PERS_SCORE] < price) {
		G_AddEvent(activator, EV_GENERAL_SOUND, G_SoundIndex("sound/items/use_nothing.wav"));
		return qfalse;
	}

	// Grant perk
	activator->client->ps.perks[item->giTag]++;
	activator->client->ps.stats[STAT_PERK] |= (1 << item->giTag);
	activator->client->ps.persistant[PERS_SCORE] -= price;

	G_AddPredictableEvent(activator, EV_ITEM_PICKUP, item - bg_itemlist);
	trap_SendServerCommand(-1, "mu_play sound/misc/buy_perk.wav 0\n");

	return qtrue;
}


/*QUAKED target_buy (1 0 0) (-8 -8 -8) (8 8 8)
Gives the activator all the items pointed to.
*/
void Use_Target_buy(gentity_t *ent, gentity_t *other, gentity_t *activator) {
	if (!activator || !activator->client || !ent->buy_item) return;

	int itemIndex = 0;
	char *itemName = ent->buy_item;
	int price = (ent->price > 0) ? ent->price : 0;
	int clientNum = activator->client->ps.clientNum;
	gitem_t *item = NULL;
	qboolean success = qfalse;

	// Special case: ammo
	if (!Q_stricmp(itemName, "ammo")) {
		if (Survival_HandleAmmoPurchase(ent, activator, price)) {
			activator->client->ps.persistant[PERS_SCORE] -= price;
			activator->client->hasPurchased = qtrue;
			ClientUserinfoChanged(clientNum);
		}
		return;
	}

	// Special case: random weapon
	if (!Q_stricmp(itemName, "random_weapon")) {
		success = Survival_HandleRandomWeaponBox(ent, activator, itemName, &itemIndex);
		if (success) {
			activator->client->hasPurchased = qtrue;
			ClientUserinfoChanged(clientNum);
		}
		return; // Don't flow into generic weapon handling
	}

	// Special case: random perk
	if (!Q_stricmp(itemName, "random_perk")) {
		success = Survival_HandleRandomPerkBox(ent, activator, &itemName, &itemIndex);
		if (success) {
			activator->client->hasPurchased = qtrue;
			ClientUserinfoChanged(clientNum);
		}
		return;
	}

	// Fallback: find item by name
	if (itemIndex <= 0) {
		for (int i = 1; bg_itemlist[i].classname; i++) {
			if (!Q_strcasecmp(itemName, bg_itemlist[i].classname)) {
				itemIndex = i;
				break;
			}
		}
	}

	if (itemIndex <= 0) return;
	item = &bg_itemlist[itemIndex];

	// Not enough points?
	if (activator->client->ps.persistant[PERS_SCORE] < price) {
		trap_SendServerCommand(-1, "mu_play sound/items/use_nothing.wav 0\n");
		return;
	}

	switch (item->giType) {
		case IT_WEAPON:
		case IT_AMMO:
			success = Survival_HandleWeaponOrGrenade(ent, activator, item, price);
			break;
		case IT_ARMOR:
			success = Survival_HandleArmorPurchase(activator, item, price);
			break;
		case IT_PERK:
			success = Survival_HandlePerkPurchase(activator, item, price);
			break;
		default:
			return;
	}

	if (success) {
		activator->client->ps.persistant[PERS_SCORE] -= price;
		activator->client->hasPurchased = qtrue;
		ClientUserinfoChanged(clientNum);
	}
}
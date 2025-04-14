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


qboolean Survival_HandleRandomWeaponBox(gentity_t *ent, gentity_t *activator, char *itemName, int *itemIndex) {
	if (!activator || !activator->client) return qfalse;

	weapon_t* selected_weapons;
	int numWeapons, randomIndex;

	weapon_t random_box_weapons[] = {
		WP_LUGER, WP_SILENCER, WP_COLT, WP_TT33, WP_REVOLVER, WP_DUAL_TT33,
		WP_AKIMBO, WP_HDM, WP_MP40, WP_THOMPSON, WP_STEN, WP_PPSH, WP_MP34,
		WP_MAUSER, WP_SNIPERRIFLE, WP_SNOOPERSCOPE, WP_MOSIN,
		WP_M1GARAND, WP_G43, WP_MP44, WP_FG42, WP_BAR, WP_M97,
		WP_BROWNING, WP_MG42M, WP_PANZERFAUST, WP_FLAMETHROWER, WP_VENOM, WP_TESLA
	};
	
	weapon_t random_box_weapons_dlc[] = {
		WP_LUGER, WP_SILENCER, WP_COLT, WP_TT33, WP_REVOLVER, WP_DUAL_TT33,
		WP_AKIMBO, WP_HDM, WP_MP40, WP_THOMPSON, WP_STEN, WP_PPSH, WP_MP34,
		WP_MAUSER, WP_SNIPERRIFLE, WP_SNOOPERSCOPE, WP_MOSIN,
		WP_M1GARAND, WP_G43, WP_M1941, WP_MP44, WP_FG42, WP_BAR, WP_M97, WP_AUTO5,
		WP_BROWNING, WP_MG42M, WP_PANZERFAUST, WP_FLAMETHROWER, WP_VENOM, WP_TESLA, WP_DELISLE
	};

	if (g_dlc1.integer == 1) {
		selected_weapons = random_box_weapons_dlc;
		numWeapons = sizeof(random_box_weapons_dlc) / sizeof(random_box_weapons_dlc[0]);
	} else {
		selected_weapons = random_box_weapons;
		numWeapons = sizeof(random_box_weapons) / sizeof(random_box_weapons[0]);
	}

	do {
		randomIndex = rand() % numWeapons;
	} while (G_FindWeaponSlot(activator, selected_weapons[randomIndex]) >= 0);

	for (int i = 1; bg_itemlist[i].classname; i++) {
		if (bg_itemlist[i].giWeapon == selected_weapons[randomIndex]) {
			*itemIndex = i;
			return qtrue;
		}
	}

	return qfalse;
}


qboolean Survival_HandleRandomPerkBox(gentity_t *ent, gentity_t *activator, char **itemName, int *itemIndex) {
	if (!activator || !activator->client) return qfalse;

	static char *random_perks[] = {
		"perk_resilience", "perk_scavenger", "perk_runner",
		"perk_weaponhandling", "perk_rifling", "perk_secondchance"
	};

	int numPerks = sizeof(random_perks) / sizeof(random_perks[0]);
	int randomIndex = rand() % numPerks;

	*itemName = random_perks[randomIndex];

	for (int i = 1; bg_itemlist[i].classname; i++) {
		if (!Q_strcasecmp(*itemName, bg_itemlist[i].classname)) {
			*itemIndex = i;
			return qtrue;
		}
	}

	return qfalse;
}


qboolean Survival_HandleAmmoPurchase(gentity_t *ent, gentity_t *activator, int price) {
	if (!activator || !activator->client) return qfalse;

	int heldWeap = activator->client->ps.weapon;
	if (heldWeap <= WP_NONE || heldWeap >= WP_NUM_WEAPONS) return qfalse;

	int ammoIndex = BG_FindAmmoForWeapon(heldWeap);
	if (activator->client->ps.ammo[ammoIndex] >= ammoTable[heldWeap].maxammo) return qfalse;

	if (heldWeap == WP_TESLA || heldWeap == WP_MG42M || heldWeap == WP_PANZERFAUST ||
		heldWeap == WP_VENOM || heldWeap == WP_FLAMETHROWER || heldWeap == WP_BROWNING) {
		if (activator->client->ps.stats[STAT_PLAYER_CLASS] != PC_SOLDIER) return qfalse;
		price *= 2;
	}

	if (activator->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT) {
		price /= 2;
	}

	if (activator->client->ps.persistant[PERS_SCORE] < price) return qfalse;

	Add_Ammo(activator, heldWeap, ammoTable[heldWeap].maxammo, qtrue);
	Add_Ammo(activator, heldWeap, ammoTable[heldWeap].maxammo, qfalse);

	trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");
	return qtrue;
}


qboolean Survival_HandleWeaponOrGrenade(gentity_t *ent, gentity_t *activator, gitem_t *item, int price) {
	if (!activator || !item) return qfalse;

	if (COM_BitCheck(activator->client->ps.weapons, item->giTag)) {
		price /= 2;
	} else {
		Give_Weapon_New_Inventory(activator, item->giTag, qfalse);
	}

	if (item->giType == IT_AMMO) {
		if (activator->client->ps.ammoclip[item->giTag] >= ammoTable[item->giTag].maxammo) return qfalse;
	} else if (activator->client->ps.ammo[item->giTag] >= ammoTable[item->giTag].maxammo) {
		return qfalse;
	}

	Add_Ammo(activator, item->giTag, ammoTable[item->giTag].maxammo, qtrue);
	Add_Ammo(activator, item->giTag, ammoTable[item->giTag].maxammo, qfalse);

	if (item->giTag == WP_M1GARAND) {
		Give_Weapon_New_Inventory(activator, WP_M7, qfalse);
		Add_Ammo(activator, WP_M7, ammoTable[WP_M7].maxammo, qfalse);
	}

	G_AddPredictableEvent(activator, EV_ITEM_PICKUP, item - bg_itemlist);
	trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");
	return qtrue;
}


qboolean Survival_HandleArmorPurchase(gentity_t *activator, gitem_t *item, int price) {
	if (!activator || activator->client->ps.stats[STAT_ARMOR] >= 200) return qfalse;

	activator->client->ps.stats[STAT_ARMOR] = 200;
	G_AddPredictableEvent(activator, EV_ITEM_PICKUP, item - bg_itemlist);
	trap_SendServerCommand(-1, "mu_play sound/misc/buy.wav 0\n");
	return qtrue;
}


qboolean Survival_HandlePerkPurchase(gentity_t *activator, gitem_t *item, int price) {
	if (!activator) return qfalse;

	int perkCount = 0;
	for (int i = 0; i < MAX_PERKS; i++) {
		if (activator->client->ps.perks[i] > 0) {
			perkCount++;
		}
	}

	int maxPerks = (activator->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) ? 4 : 3;
	if (perkCount >= maxPerks || activator->client->ps.perks[item->giTag] > 0) return qfalse;

	activator->client->ps.perks[item->giTag]++;
	activator->client->ps.stats[STAT_PERK] |= (1 << item->giTag);

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
		if (!Survival_HandleRandomWeaponBox(ent, activator, itemName, &itemIndex)) return;
	}

	// Special case: random perk
	else if (!Q_stricmp(itemName, "random_perk")) {
		if (!Survival_HandleRandomPerkBox(ent, activator, &itemName, &itemIndex)) return;
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

	qboolean success = qfalse;

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
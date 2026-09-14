/*
===========================================================================

bg_armory.h -- shared (game + ui) data model for the hub armory loadout
builder. Stateless: no gentity_t access, no server commands.

===========================================================================
*/

#ifndef __BG_ARMORY_H__
#define __BG_ARMORY_H__

#define ARMORY_MAX_ROSTER_WEAPONS  32
#define ARMORY_MAX_EQUIP           8   // safe upper bound for fixed local arrays; actual count from BG_Armory_GetEquipList

// A parsed weapon roster for one hub map: which weapons are selectable.
typedef struct {
	int weapons[ARMORY_MAX_ROSTER_WEAPONS];        // weapon_t values, in file order
	qboolean recommended[ARMORY_MAX_ROSTER_WEAPONS]; // "weapon <classname> recommended" flag, parallel to weapons[]
	int numWeapons;
	qboolean equipRecommended[ARMORY_MAX_EQUIP];   // "equip <id> recommended" flags, indexed same as BG_Armory_GetEquipList()
} armoryRoster_t;

// One of the 4 fixed equipment/perk items offered in every armory.
typedef struct {
	const char  *id;            // stable short id (used in the confirm command + UI lookups)
	const char  *displayName;
	const char  *icon;
	int         perkTag;        // perk_t value granted on pick, or -1 (Full Ammo Bag: no perk)
	const char  *costCvarName;  // g_loadoutCost* cvar backing this item's point cost
} armoryEquipDef_t;

// Reads and parses a roster file; safe to call from game, cgame or ui.
qboolean BG_Armory_LoadRoster( const char *rosterFile, armoryRoster_t *out );

// Fixed 4-entry equipment table, not file-driven; costs come from cvars.
const armoryEquipDef_t *BG_Armory_GetEquipList( int *count );
const armoryEquipDef_t *BG_Armory_FindEquip( const char *id );
int BG_Armory_GetEquipCost( const armoryEquipDef_t *def );

// Localized pickup name; falls back to item->pickup_name.
const char *BG_Armory_GetPickupName( const gitem_t *item );

// Icon from the weapon's .weap file; returns 0 if not found.
qhandle_t BG_Armory_GetWeaponIconFromFile( weapon_t weaponNum );

#endif // __BG_ARMORY_H__

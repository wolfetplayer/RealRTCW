/*
===========================================================================

ui_armory.c -- ui.qvm side of the hub armory point-budget loadout builder.
See ui_armory.h.

===========================================================================
*/

#include "ui_local.h"
#include "../game/bg_armory.h"
#include "ui_armory.h"

static armoryRoster_t armoryRoster;

static qboolean  armoryWeaponPicked[ARMORY_MAX_ROSTER_WEAPONS];
static qhandle_t armoryWeaponIcons[ARMORY_MAX_ROSTER_WEAPONS];

static qboolean  armoryEquipPicked[ARMORY_MAX_EQUIP];
static qhandle_t armoryEquipIcons[ARMORY_MAX_EQUIP];

void UI_Armory_Reset( void ) {
	int i;

	for ( i = 0; i < ARMORY_MAX_ROSTER_WEAPONS; i++ ) {
		armoryWeaponPicked[i] = qfalse;
		armoryWeaponIcons[i] = -1;
	}
	for ( i = 0; i < ARMORY_MAX_EQUIP; i++ ) {
		armoryEquipPicked[i] = qfalse;
		armoryEquipIcons[i] = -1;
	}
}

void UI_Armory_LoadRosterForCurrentMap( void ) {
	char mapname[MAX_QPATH];
	char rosterPath[MAX_QPATH];

	trap_Cvar_VariableStringBuffer( "mapname", mapname, sizeof( mapname ) );
	Com_sprintf( rosterPath, sizeof( rosterPath ), "loadouts/rosters/%s.armory", mapname );

	BG_Armory_LoadRoster( rosterPath, &armoryRoster );
	UI_Armory_Reset();
}

//=============================== weapons ===============================

int UI_Armory_WeaponCount( void ) {
	return armoryRoster.numWeapons;
}

static gitem_t *UI_Armory_WeaponItem( int index ) {
	if ( index < 0 || index >= armoryRoster.numWeapons ) {
		return NULL;
	}
	return BG_FindItemForWeapon( armoryRoster.weapons[index] );
}

const char *UI_Armory_WeaponName( int index ) {
	gitem_t *item = UI_Armory_WeaponItem( index );
	return item ? BG_Armory_GetPickupName( item ) : "";
}

qhandle_t UI_Armory_WeaponIcon( int index ) {
	gitem_t *item;

	if ( index < 0 || index >= armoryRoster.numWeapons ) {
		return 0;
	}
	if ( armoryWeaponIcons[index] == -1 ) {
		item = UI_Armory_WeaponItem( index );
		armoryWeaponIcons[index] = 0;
		if ( item ) {
			armoryWeaponIcons[index] = BG_Armory_GetWeaponIconFromFile( armoryRoster.weapons[index] );
			if ( !armoryWeaponIcons[index] ) {
				armoryWeaponIcons[index] = trap_R_RegisterShaderNoMip( item->icon );
			}
		}
	}
	return armoryWeaponIcons[index];
}

qboolean UI_Armory_WeaponPicked( int index ) {
	if ( index < 0 || index >= armoryRoster.numWeapons ) {
		return qfalse;
	}
	return armoryWeaponPicked[index];
}

void UI_Armory_ToggleWeapon( int index ) {
	if ( index < 0 || index >= armoryRoster.numWeapons ) {
		return;
	}
	if ( armoryWeaponPicked[index] ) {
		armoryWeaponPicked[index] = qfalse;
		return;
	}
	if ( UI_Armory_PointsUsed() + (int)trap_Cvar_VariableValue( "g_loadoutWeaponCost" ) > UI_Armory_PointsTotal() ) {
		return;  // over budget, deny
	}
	armoryWeaponPicked[index] = qtrue;
}

//============================== equipment ==============================

int UI_Armory_EquipCount( void ) {
	int count;
	BG_Armory_GetEquipList( &count );
	return count;
}

static const armoryEquipDef_t *UI_Armory_EquipDef( int index ) {
	int count;
	const armoryEquipDef_t *list = BG_Armory_GetEquipList( &count );

	if ( index < 0 || index >= count ) {
		return NULL;
	}
	return &list[index];
}

static const struct { const char *id; const char *key; } armoryEquipKeys[] = {
	{ "fullammobag",    "ARMORY_EQUIP_FULLAMMOBAG" },
	{ "heavyarmor",     "ARMORY_EQUIP_HEAVYARMOR" },
	{ "lightweight",    "ARMORY_EQUIP_LIGHTWEIGHT" },
	{ "tacticalgloves", "ARMORY_EQUIP_TACTICALGLOVES" },
};

static char     armoryEquipNames[ARMORY_MAX_EQUIP][64];
static qboolean armoryEquipNamesResolved = qfalse;

void UI_Armory_ResolveEquipTranslations( void ) {
	int i, j, count;
	const armoryEquipDef_t *list = BG_Armory_GetEquipList( &count );

	for ( i = 0; i < count && i < ARMORY_MAX_EQUIP; i++ ) {
		const char *translated = NULL;

		for ( j = 0; j < (int)( sizeof( armoryEquipKeys ) / sizeof( armoryEquipKeys[0] ) ); j++ ) {
			if ( !Q_stricmp( armoryEquipKeys[j].id, list[i].id ) ) {
				translated = TranslateTable_Find( armoryEquipKeys[j].key );
				break;
			}
		}
		Q_strncpyz( armoryEquipNames[i], translated ? translated : list[i].displayName, sizeof( armoryEquipNames[i] ) );
	}
	armoryEquipNamesResolved = qtrue;
}

const char *UI_Armory_EquipName( int index ) {
	const armoryEquipDef_t *def = UI_Armory_EquipDef( index );

	if ( !def ) {
		return "";
	}
	if ( armoryEquipNamesResolved && index >= 0 && index < ARMORY_MAX_EQUIP ) {
		return armoryEquipNames[index];
	}
	return def->displayName;
}

qhandle_t UI_Armory_EquipIcon( int index ) {
	const armoryEquipDef_t *def;

	if ( index < 0 || index >= ARMORY_MAX_EQUIP ) {
		return 0;
	}
	if ( armoryEquipIcons[index] == -1 ) {
		def = UI_Armory_EquipDef( index );
		armoryEquipIcons[index] = def ? trap_R_RegisterShaderNoMip( def->icon ) : 0;
	}
	return armoryEquipIcons[index];
}

qboolean UI_Armory_EquipPicked( int index ) {
	if ( index < 0 || index >= ARMORY_MAX_EQUIP ) {
		return qfalse;
	}
	return armoryEquipPicked[index];
}

void UI_Armory_ToggleEquip( int index ) {
	const armoryEquipDef_t *def = UI_Armory_EquipDef( index );

	if ( !def ) {
		return;
	}
	if ( armoryEquipPicked[index] ) {
		armoryEquipPicked[index] = qfalse;
		return;
	}
	if ( UI_Armory_PointsUsed() + BG_Armory_GetEquipCost( def ) > UI_Armory_PointsTotal() ) {
		return;  // over budget, deny
	}
	armoryEquipPicked[index] = qtrue;
}

//=========================== combined build list ========================

int UI_Armory_BuildCount( void ) {
	int i, count = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryWeaponPicked[i] ) {
			count++;
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( armoryEquipPicked[i] ) {
			count++;
		}
	}
	return count;
}

// Maps a build-list index to a weapon roster index (returns qtrue) or an equipment index (returns qfalse)
static qboolean UI_Armory_ResolveBuildIndex( int index, int *weaponIndex, int *equipIndex ) {
	int i, n = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryWeaponPicked[i] ) {
			if ( n == index ) {
				*weaponIndex = i;
				return qtrue;
			}
			n++;
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( armoryEquipPicked[i] ) {
			if ( n == index ) {
				*equipIndex = i;
				return qfalse;
			}
			n++;
		}
	}
	*weaponIndex = -1;
	*equipIndex = -1;
	return qtrue;
}

const char *UI_Armory_BuildName( int index ) {
	int weaponIndex, equipIndex;

	if ( UI_Armory_ResolveBuildIndex( index, &weaponIndex, &equipIndex ) ) {
		return weaponIndex >= 0 ? UI_Armory_WeaponName( weaponIndex ) : "";
	}
	return UI_Armory_EquipName( equipIndex );
}

qhandle_t UI_Armory_BuildIcon( int index ) {
	int weaponIndex, equipIndex;

	if ( UI_Armory_ResolveBuildIndex( index, &weaponIndex, &equipIndex ) ) {
		return weaponIndex >= 0 ? UI_Armory_WeaponIcon( weaponIndex ) : 0;
	}
	return UI_Armory_EquipIcon( equipIndex );
}

void UI_Armory_RemoveBuildIndex( int index ) {
	int weaponIndex, equipIndex;

	if ( UI_Armory_ResolveBuildIndex( index, &weaponIndex, &equipIndex ) ) {
		if ( weaponIndex >= 0 ) {
			armoryWeaponPicked[weaponIndex] = qfalse;
		}
	} else {
		armoryEquipPicked[equipIndex] = qfalse;
	}
}

//================================ points =================================

int UI_Armory_PointsTotal( void ) {
	return (int)trap_Cvar_VariableValue( "g_loadoutPoints" );
}

int UI_Armory_PointsUsed( void ) {
	int i, used = 0;
	int weaponCost = (int)trap_Cvar_VariableValue( "g_loadoutWeaponCost" );

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryWeaponPicked[i] ) {
			used += weaponCost;
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( armoryEquipPicked[i] ) {
			used += BG_Armory_GetEquipCost( UI_Armory_EquipDef( i ) );
		}
	}
	return used;
}

//================================ confirm =================================

void UI_Armory_BuildConfirmCommand( char *out, int outSize ) {
	char weaponList[1024];
	char equipList[1024];
	int i;
	qboolean first;

	weaponList[0] = '\0';
	equipList[0] = '\0';

	first = qtrue;
	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		gitem_t *item;

		if ( !armoryWeaponPicked[i] ) {
			continue;
		}
		item = UI_Armory_WeaponItem( i );
		if ( !item ) {
			continue;
		}
		if ( !first ) {
			Q_strcat( weaponList, sizeof( weaponList ), "," );
		}
		Q_strcat( weaponList, sizeof( weaponList ), item->classname );
		first = qfalse;
	}

	first = qtrue;
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		const armoryEquipDef_t *def;

		if ( !armoryEquipPicked[i] ) {
			continue;
		}
		def = UI_Armory_EquipDef( i );
		if ( !def ) {
			continue;
		}
		if ( !first ) {
			Q_strcat( equipList, sizeof( equipList ), "," );
		}
		Q_strcat( equipList, sizeof( equipList ), def->id );
		first = qfalse;
	}

	Com_sprintf( out, outSize, "sp_loadout_confirm %s %s\n",
		weaponList[0] ? weaponList : "none", equipList[0] ? equipList : "none" );
}

//========================= randomize / recommended =========================

void UI_Armory_ApplyRecommended( void ) {
	int i;

	UI_Armory_Reset();

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryRoster.recommended[i] ) {
			UI_Armory_ToggleWeapon( i );
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( armoryRoster.equipRecommended[i] ) {
			UI_Armory_ToggleEquip( i );
		}
	}
}

void UI_Armory_Randomize( void ) {
	int candIndex[ARMORY_MAX_ROSTER_WEAPONS + ARMORY_MAX_EQUIP];
	qboolean candIsWeapon[ARMORY_MAX_ROSTER_WEAPONS + ARMORY_MAX_EQUIP];
	int numCand = 0;
	int i, j, equipCount;

	UI_Armory_Reset();

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		candIsWeapon[numCand] = qtrue;
		candIndex[numCand] = i;
		numCand++;
	}
	equipCount = UI_Armory_EquipCount();
	for ( i = 0; i < equipCount; i++ ) {
		candIsWeapon[numCand] = qfalse;
		candIndex[numCand] = i;
		numCand++;
	}

	for ( i = numCand - 1; i > 0; i-- ) {
		int tmpIndex;
		qboolean tmpIsWeapon;

		j = rand() % ( i + 1 );

		tmpIndex = candIndex[i];
		candIndex[i] = candIndex[j];
		candIndex[j] = tmpIndex;

		tmpIsWeapon = candIsWeapon[i];
		candIsWeapon[i] = candIsWeapon[j];
		candIsWeapon[j] = tmpIsWeapon;
	}

	for ( i = 0; i < numCand; i++ ) {
		if ( candIsWeapon[i] ) {
			UI_Armory_ToggleWeapon( candIndex[i] );
		} else {
			UI_Armory_ToggleEquip( candIndex[i] );
		}
	}
}

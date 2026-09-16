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

static int selectedWeaponIndex = -1;
static int selectedEquipIndex = -1;
static int selectedBuildIndex = -1;

static int UI_Armory_RawEquipIndex( int compactIndex );   // defined below; needed by UI_Armory_Reset above it

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
	selectedWeaponIndex = -1;
	selectedEquipIndex = -1;
	selectedBuildIndex = -1;

	// perma items are mapper-forced picks: always in the build, regardless of what triggered this reset
	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryRoster.perma[i] ) {
			armoryWeaponPicked[i] = qtrue;
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		int rawIndex = UI_Armory_RawEquipIndex( i );
		if ( rawIndex >= 0 && armoryRoster.equipPerma[rawIndex] ) {
			armoryEquipPicked[i] = qtrue;
		}
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
	if ( armoryRoster.perma[index] ) {
		return;   // mapper-forced pick: always on, can't be toggled off
	}
	if ( armoryWeaponPicked[index] ) {
		armoryWeaponPicked[index] = qfalse;
		return;
	}
	if ( UI_Armory_PointsUsed() + BG_Armory_GetWeaponCost( armoryRoster.weapons[index] ) > UI_Armory_PointsTotal() ) {
		return;  // over budget, deny
	}
	armoryWeaponPicked[index] = qtrue;
}

// Source-list view: only weapons not already in the build. Raw index stays what Toggle/Picked/etc use.
static int UI_Armory_RawIndexForAvailableWeapon( int availIndex ) {
	int i, n = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( !armoryWeaponPicked[i] ) {
			if ( n == availIndex ) {
				return i;
			}
			n++;
		}
	}
	return -1;
}

int UI_Armory_AvailableWeaponCount( void ) {
	int i, n = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( !armoryWeaponPicked[i] ) {
			n++;
		}
	}
	return n;
}

const char *UI_Armory_AvailableWeaponName( int availIndex ) {
	return UI_Armory_WeaponName( UI_Armory_RawIndexForAvailableWeapon( availIndex ) );
}

qhandle_t UI_Armory_AvailableWeaponIcon( int availIndex ) {
	return UI_Armory_WeaponIcon( UI_Armory_RawIndexForAvailableWeapon( availIndex ) );
}

void UI_Armory_SelectAvailableWeapon( int availIndex ) {
	UI_Armory_SelectWeapon( UI_Armory_RawIndexForAvailableWeapon( availIndex ) );
}

//============================== equipment ==============================

// Maps a compact (post-roster-filter) equip index to its raw index into BG_Armory_GetEquipList()/equipRecommended[].
static int UI_Armory_RawEquipIndex( int compactIndex ) {
	int i, n = 0, count;

	BG_Armory_GetEquipList( &count );
	for ( i = 0; i < count && i < ARMORY_MAX_EQUIP; i++ ) {
		if ( armoryRoster.equipPresent[i] ) {
			if ( n == compactIndex ) {
				return i;
			}
			n++;
		}
	}
	return -1;
}

int UI_Armory_EquipCount( void ) {
	int i, count, n = 0;

	BG_Armory_GetEquipList( &count );
	for ( i = 0; i < count && i < ARMORY_MAX_EQUIP; i++ ) {
		if ( armoryRoster.equipPresent[i] ) {
			n++;
		}
	}
	return n;
}

static const armoryEquipDef_t *UI_Armory_EquipDef( int index ) {
	int count, rawIndex = UI_Armory_RawEquipIndex( index );
	const armoryEquipDef_t *list = BG_Armory_GetEquipList( &count );

	if ( rawIndex < 0 ) {
		return NULL;
	}
	return &list[rawIndex];
}

static const struct { const char *id; const char *key; } armoryEquipKeys[] = {
	{ "fullammobag",    "ARMORY_EQUIP_FULLAMMOBAG" },
	{ "heavyarmor",     "ARMORY_EQUIP_HEAVYARMOR" },
	{ "lightweight",    "ARMORY_EQUIP_LIGHTWEIGHT" },
	{ "tacticalgloves", "ARMORY_EQUIP_TACTICALGLOVES" },
	{ "grenades",       "ARMORY_EQUIP_GRENADES" },
};

static char     armoryEquipNames[ARMORY_MAX_EQUIP][64];
static char     armoryEquipDescs[ARMORY_MAX_EQUIP][96];
static qboolean armoryEquipNamesResolved = qfalse;

// Weapon descriptions, indexed by weapon_t - resolved for every weapon up front, since no roster is loaded yet here.
static char     armoryWeaponDescs[WP_NUM_WEAPONS][96];
static qboolean armoryWeaponDescsResolved = qfalse;

// "ARMORY_DESC_<id, uppercased>" - same convention for both weapons and equipment.
static void UI_Armory_LookupDescKey( const char *id, char *out, int outSize ) {
	Com_sprintf( out, outSize, "ARMORY_DESC_%s", id );
	Q_strupr( out );
}

// IMPORTANT: must run (with UI_Armory_ResolveWeaponDescTranslations) before UI_FreeTranslateTable() frees the table, or every @KEY lookup here fails silently.
void UI_Armory_ResolveEquipTranslations( void ) {
	int i, j, count;
	const armoryEquipDef_t *list = BG_Armory_GetEquipList( &count );

	for ( i = 0; i < count && i < ARMORY_MAX_EQUIP; i++ ) {
		const char *translated = NULL;
		char key[64];

		for ( j = 0; j < (int)( sizeof( armoryEquipKeys ) / sizeof( armoryEquipKeys[0] ) ); j++ ) {
			if ( !Q_stricmp( armoryEquipKeys[j].id, list[i].id ) ) {
				translated = TranslateTable_Find( armoryEquipKeys[j].key );
				break;
			}
		}
		Q_strncpyz( armoryEquipNames[i], translated ? translated : list[i].displayName, sizeof( armoryEquipNames[i] ) );

		UI_Armory_LookupDescKey( list[i].id, key, sizeof( key ) );
		translated = TranslateTable_Find( key );
		Q_strncpyz( armoryEquipDescs[i], translated ? translated : "", sizeof( armoryEquipDescs[i] ) );
	}
	armoryEquipNamesResolved = qtrue;
}

void UI_Armory_ResolveWeaponDescTranslations( void ) {
	gitem_t *it;

	for ( it = bg_itemlist + 1; it->classname; it++ ) {
		const char *classname;
		char key[64];
		const char *translated;
		int weaponNum;

		if ( it->giType != IT_WEAPON ) {
			continue;
		}
		weaponNum = it->giTag;
		if ( weaponNum <= WP_NONE || weaponNum >= WP_NUM_WEAPONS ) {
			continue;
		}

		classname = it->classname;
		if ( !Q_stricmpn( classname, "weapon_", 7 ) ) {
			classname += 7;
		}
		UI_Armory_LookupDescKey( classname, key, sizeof( key ) );

		translated = TranslateTable_Find( key );
		Q_strncpyz( armoryWeaponDescs[weaponNum], translated ? translated : "", sizeof( armoryWeaponDescs[weaponNum] ) );
	}
	armoryWeaponDescsResolved = qtrue;
}

const char *UI_Armory_EquipName( int index ) {
	int rawIndex = UI_Armory_RawEquipIndex( index );
	const armoryEquipDef_t *def;

	if ( rawIndex < 0 ) {
		return "";
	}
	if ( armoryEquipNamesResolved ) {
		return armoryEquipNames[rawIndex];
	}
	def = UI_Armory_EquipDef( index );
	return def ? def->displayName : "";
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
	int rawIndex;

	if ( !def ) {
		return;
	}
	rawIndex = UI_Armory_RawEquipIndex( index );
	if ( rawIndex >= 0 && armoryRoster.equipPerma[rawIndex] ) {
		return;   // mapper-forced pick: always on, can't be toggled off
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

// Source-list view: only equip items not already in the build. Present-compact index stays what Toggle/Picked/etc use.
static int UI_Armory_PresentIndexForAvailableEquip( int availIndex ) {
	int i, n = 0;

	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( !armoryEquipPicked[i] ) {
			if ( n == availIndex ) {
				return i;
			}
			n++;
		}
	}
	return -1;
}

int UI_Armory_AvailableEquipCount( void ) {
	int i, n = 0;

	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( !armoryEquipPicked[i] ) {
			n++;
		}
	}
	return n;
}

const char *UI_Armory_AvailableEquipName( int availIndex ) {
	return UI_Armory_EquipName( UI_Armory_PresentIndexForAvailableEquip( availIndex ) );
}

qhandle_t UI_Armory_AvailableEquipIcon( int availIndex ) {
	return UI_Armory_EquipIcon( UI_Armory_PresentIndexForAvailableEquip( availIndex ) );
}

void UI_Armory_SelectAvailableEquip( int availIndex ) {
	UI_Armory_SelectEquip( UI_Armory_PresentIndexForAvailableEquip( availIndex ) );
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

qboolean UI_Armory_BuildIsPerma( int index ) {
	int weaponIndex, equipIndex, rawIndex;

	if ( UI_Armory_ResolveBuildIndex( index, &weaponIndex, &equipIndex ) ) {
		return weaponIndex >= 0 && armoryRoster.perma[weaponIndex];
	}
	rawIndex = UI_Armory_RawEquipIndex( equipIndex );
	return rawIndex >= 0 && armoryRoster.equipPerma[rawIndex];
}

void UI_Armory_RemoveBuildIndex( int index ) {
	int weaponIndex, equipIndex;

	if ( UI_Armory_BuildIsPerma( index ) ) {
		return;   // mapper-forced pick: not removable
	}
	if ( UI_Armory_ResolveBuildIndex( index, &weaponIndex, &equipIndex ) ) {
		if ( weaponIndex >= 0 ) {
			armoryWeaponPicked[weaponIndex] = qfalse;
		}
	} else {
		armoryEquipPicked[equipIndex] = qfalse;
	}
}

void UI_Armory_SelectBuild( int index ) {
	if ( index < 0 || index >= UI_Armory_BuildCount() ) {
		return;
	}
	if ( UI_Armory_BuildIsPerma( index ) ) {
		return;   // mapper-forced pick: not selectable
	}
	selectedBuildIndex = index;
}

void UI_Armory_RemoveSelectedBuild( void ) {
	if ( selectedBuildIndex < 0 || selectedBuildIndex >= UI_Armory_BuildCount() ) {
		return;
	}
	UI_Armory_RemoveBuildIndex( selectedBuildIndex );
	selectedBuildIndex = -1;   // indices shift after a removal, so drop the stale selection
	selectedWeaponIndex = -1;  // the removed item reappears in the source list at some other position
	selectedEquipIndex = -1;
}

int UI_Armory_SelectedBuildIndex( void ) {
	return selectedBuildIndex;
}

//================================ points =================================

int UI_Armory_PointsTotal( void ) {
	return (int)trap_Cvar_VariableValue( "g_loadoutPoints" );
}

int UI_Armory_PointsUsed( void ) {
	int i, used = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryWeaponPicked[i] && !armoryRoster.perma[i] ) {
			used += BG_Armory_GetWeaponCost( armoryRoster.weapons[i] );
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		int rawIndex;

		if ( !armoryEquipPicked[i] ) {
			continue;
		}
		rawIndex = UI_Armory_RawEquipIndex( i );
		if ( rawIndex >= 0 && armoryRoster.equipPerma[rawIndex] ) {
			continue;   // free of charge
		}
		used += BG_Armory_GetEquipCost( UI_Armory_EquipDef( i ) );
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

		if ( !armoryWeaponPicked[i] || armoryRoster.perma[i] ) {
			continue;   // perma items are granted server-side unconditionally, not sent as a pick
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
		int rawIndex;

		if ( !armoryEquipPicked[i] ) {
			continue;
		}
		rawIndex = UI_Armory_RawEquipIndex( i );
		if ( rawIndex >= 0 && armoryRoster.equipPerma[rawIndex] ) {
			continue;   // perma items are granted server-side unconditionally, not sent as a pick
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
		int rawIndex = UI_Armory_RawEquipIndex( i );
		if ( rawIndex >= 0 && armoryRoster.equipRecommended[rawIndex] ) {
			UI_Armory_ToggleEquip( i );
		}
	}
}

void UI_Armory_Randomize( void ) {
	int candIndex[ARMORY_MAX_ROSTER_WEAPONS + ARMORY_MAX_EQUIP];
	qboolean candIsWeapon[ARMORY_MAX_ROSTER_WEAPONS + ARMORY_MAX_EQUIP];
	int numCand = 0;
	int i, j, equipCount;
	int targetBudget = UI_Armory_PointsTotal();

	UI_Armory_Reset();

	// about half the time, cap below the full pool so randomize doesn't always max it out
	if ( rand() % 100 < 50 ) {
		targetBudget = targetBudget * ( 50 + rand() % 51 ) / 100;
	}

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
		int cost;

		if ( candIsWeapon[i] ) {
			cost = BG_Armory_GetWeaponCost( armoryRoster.weapons[candIndex[i]] );
			if ( UI_Armory_PointsUsed() + cost > targetBudget ) {
				continue;
			}
			UI_Armory_ToggleWeapon( candIndex[i] );
		} else {
			const armoryEquipDef_t *def = UI_Armory_EquipDef( candIndex[i] );

			cost = def ? BG_Armory_GetEquipCost( def ) : 0;
			if ( UI_Armory_PointsUsed() + cost > targetBudget ) {
				continue;
			}
			UI_Armory_ToggleEquip( candIndex[i] );
		}
	}
}

//===================== select (highlight) + add + description =====================

void UI_Armory_SelectWeapon( int index ) {
	if ( index < 0 || index >= armoryRoster.numWeapons ) {
		return;
	}
	selectedWeaponIndex = index;
}

void UI_Armory_SelectEquip( int index ) {
	if ( index < 0 || index >= UI_Armory_EquipCount() ) {
		return;
	}
	selectedEquipIndex = index;
}

// Inverse of UI_Armory_ResolveBuildIndex - finds a picked item's position in the combined build list.
static int UI_Armory_BuildIndexForWeapon( int weaponIndex ) {
	int i, n = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryWeaponPicked[i] ) {
			if ( i == weaponIndex ) {
				return n;
			}
			n++;
		}
	}
	return -1;
}

static int UI_Armory_BuildIndexForEquip( int equipIndex ) {
	int i, n = 0;

	for ( i = 0; i < armoryRoster.numWeapons; i++ ) {
		if ( armoryWeaponPicked[i] ) {
			n++;
		}
	}
	for ( i = 0; i < UI_Armory_EquipCount(); i++ ) {
		if ( armoryEquipPicked[i] ) {
			if ( i == equipIndex ) {
				return n;
			}
			n++;
		}
	}
	return -1;
}

void UI_Armory_AddSelectedWeapon( void ) {
	if ( selectedWeaponIndex < 0 || armoryWeaponPicked[selectedWeaponIndex] ) {
		return;   // nothing selected, or already added - removal only happens via the build list
	}
	UI_Armory_ToggleWeapon( selectedWeaponIndex );
	selectedBuildIndex = UI_Armory_BuildIndexForWeapon( selectedWeaponIndex );   // so Remove works right away
	selectedWeaponIndex = -1;   // it just left the source list
}

void UI_Armory_AddSelectedEquip( void ) {
	if ( selectedEquipIndex < 0 || armoryEquipPicked[selectedEquipIndex] ) {
		return;
	}
	UI_Armory_ToggleEquip( selectedEquipIndex );
	selectedBuildIndex = UI_Armory_BuildIndexForEquip( selectedEquipIndex );
	selectedEquipIndex = -1;
}

qhandle_t UI_Armory_SelectedWeaponIcon( void ) {
	return UI_Armory_WeaponIcon( selectedWeaponIndex );
}

// Reads the cache built by UI_Armory_ResolveWeaponDescTranslations() - see its comment for why.
const char *UI_Armory_SelectedWeaponDesc( void ) {
	int weaponNum;

	if ( selectedWeaponIndex < 0 || selectedWeaponIndex >= armoryRoster.numWeapons ) {
		return "";
	}
	weaponNum = armoryRoster.weapons[selectedWeaponIndex];
	if ( !armoryWeaponDescsResolved || weaponNum <= WP_NONE || weaponNum >= WP_NUM_WEAPONS ) {
		return "";
	}
	return armoryWeaponDescs[weaponNum];
}

// Points required to add the currently highlighted weapon to the build; -1 if nothing is selected.
int UI_Armory_SelectedWeaponCost( void ) {
	if ( selectedWeaponIndex < 0 || selectedWeaponIndex >= armoryRoster.numWeapons ) {
		return -1;
	}
	return BG_Armory_GetWeaponCost( armoryRoster.weapons[selectedWeaponIndex] );
}

qboolean UI_Armory_SelectedWeaponIsWide( void ) {
	if ( selectedWeaponIndex < 0 || selectedWeaponIndex >= armoryRoster.numWeapons ) {
		return qfalse;
	}
	return BG_Armory_IsWideIcon( armoryRoster.weapons[selectedWeaponIndex] );
}

qhandle_t UI_Armory_SelectedEquipIcon( void ) {
	return UI_Armory_EquipIcon( selectedEquipIndex );
}

const char *UI_Armory_SelectedEquipDesc( void ) {
	int rawIndex;

	if ( !armoryEquipNamesResolved || selectedEquipIndex < 0 || selectedEquipIndex >= UI_Armory_EquipCount() ) {
		return "";
	}
	rawIndex = UI_Armory_RawEquipIndex( selectedEquipIndex );
	if ( rawIndex < 0 ) {
		return "";
	}
	return armoryEquipDescs[rawIndex];
}

// Points required to add the currently highlighted equipment item to the build; -1 if nothing is selected.
int UI_Armory_SelectedEquipCost( void ) {
	const armoryEquipDef_t *def;

	if ( selectedEquipIndex < 0 || selectedEquipIndex >= UI_Armory_EquipCount() ) {
		return -1;
	}
	def = UI_Armory_EquipDef( selectedEquipIndex );
	return def ? BG_Armory_GetEquipCost( def ) : -1;
}

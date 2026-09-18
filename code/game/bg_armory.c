/*
===========================================================================

bg_armory.c -- both games (+ ui) armory loadout roster parsing, all
completely stateless. Compiled into qagame, cgame and ui.qvm alike (same
pattern as bg_misc.c/bg_pmove.c). No gentity_t access, no server commands.

===========================================================================
*/

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_armory.h"

// Declared locally rather than pulling in a per-VM local header, same convention bg_misc.c uses for trap_Cvar_*.
extern int      trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
extern void     trap_FS_Read( void *buffer, int len, fileHandle_t f );
extern void     trap_FS_FCloseFile( fileHandle_t f );

// qagame only links trap_Cvar_VariableIntegerValue; ui.qvm only links trap_Cvar_VariableValue.
#ifdef GAMEDLL
extern int trap_Cvar_VariableIntegerValue( const char *var_name );
static int BG_Armory_CvarInt( const char *var_name ) {
	return trap_Cvar_VariableIntegerValue( var_name );
}
#else
extern float trap_Cvar_VariableValue( const char *var_name );
static int BG_Armory_CvarInt( const char *var_name ) {
	return (int)trap_Cvar_VariableValue( var_name );
}
#endif

#define ARMORY_FILE_BUFSIZE ( 16 * 1024 )
#define ARMORY_MAX_IF_DEPTH 4

static const armoryEquipDef_t armoryEquipList[] = {
	{ "fullammobag",     "Full Ammo Bag",       "icons/perk_fullammobag",     -1,                  WP_NONE,       "g_loadoutCostFullAmmoBag" },
	{ "heavyarmor",      "Heavy Armor",         "icons/perk_heavyarmor",      PERK_HEAVYARMOR,      WP_NONE,       "g_loadoutCostHeavyArmor" },
	{ "lightweight",     "Lightweight Gear",    "icons/perk_lightweight",     PERK_LIGHTWEIGHT,     WP_NONE,       "g_loadoutCostLightweightGear" },
	{ "tacticalgloves",  "Tactical Gloves",     "icons/perk_tacticalgloves",  PERK_TACTICALGLOVES,  WP_NONE,       "g_loadoutCostTacticalGloves" },
	{ "grenades",        "Additional Grenades", "icons/perk_grenades",        -1,                  WP_NONE,       "g_loadoutCostGrenades" },
	{ "camosuit",        "Camo Suit",           "icons/perk_camosuit",        PERK_CAMOSUIT,        WP_NONE,       "g_loadoutCostCamoSuit" },
	{ "airstrikesignal", "Airstrike Signal",    "icons/iconw_airstrike_1", -1,                  WP_AIRSTRIKE,  "g_loadoutCostAirstrikeSignal" },
	{ "gasgrenade",      "Gas Grenade",         "icons/iconw_gasgrenade_1",-1,                  WP_POISONGAS,  "g_loadoutCostGasGrenade" },
	{ "smokegrenade",    "Smoke Grenade",       "icons/iconw_smokebomb_1",    -1,                  WP_SMOKE_BOMB, "g_loadoutCostSmokeGrenade" },
	{ "extraknives",     "Additional Throwing Knives", "icons/iconw_knife_1", -1,             WP_NONE,       "g_loadoutCostExtraKnives" },
	{ "binoculars",      "Binoculars",          "icons/binocs",               -1,                  WP_NONE,       "g_loadoutCostBinoculars" },
};
#define ARMORY_NUM_EQUIP ( sizeof( armoryEquipList ) / sizeof( armoryEquipList[0] ) )

const armoryEquipDef_t *BG_Armory_GetEquipList( int *count ) {
	if ( count ) {
		*count = ARMORY_NUM_EQUIP;
	}
	return armoryEquipList;
}

const armoryEquipDef_t *BG_Armory_FindEquip( const char *id ) {
	int i;

	if ( !id || !id[0] ) {
		return NULL;
	}
	for ( i = 0; i < ARMORY_NUM_EQUIP; i++ ) {
		if ( !Q_stricmp( armoryEquipList[i].id, id ) ) {
			return &armoryEquipList[i];
		}
	}
	return NULL;
}

int BG_Armory_GetEquipCost( const armoryEquipDef_t *def ) {
	if ( !def || !def->costCvarName ) {
		return 0;
	}
	return BG_Armory_CvarInt( def->costCvarName );
}

int BG_Armory_GetWeaponCost( weapon_t weaponNum ) {
	int base = BG_Armory_CvarInt( "g_loadoutWeaponCost" );

	switch ( weaponNum ) {
	case WP_KNIFE:
	case WP_GRENADE_PINEAPPLE:
		return 0;   // always free, independent of the roster's "perma" marker
	case WP_VENOM:
	case WP_TESLA:
		return base * 2;
	default:
		return base;
	}
}

qboolean BG_Armory_IsGrenadeWeapon( weapon_t weaponNum ) {
	switch ( weaponNum ) {
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
	case WP_AIRSTRIKE:
	case WP_POISONGAS:
	case WP_SMOKE_BOMB:
		return qtrue;
	default:
		return qfalse;
	}
}

/*
===============
BG_Armory_LoadRoster

NOTE: this handles "#if"/"#endif" itself (COM_ParseExt's version is GAMEDLL-only),
and nests them so a weapon can require more than one condition at once.
===============
*/
qboolean BG_Armory_LoadRoster( const char *rosterFile, armoryRoster_t *out ) {
	static char fileBuf[ARMORY_FILE_BUFSIZE];
	fileHandle_t f;
	int len, i;
	char *p, *tok;
	qboolean skipStack[ARMORY_MAX_IF_DEPTH];
	int ifDepth = 0;
	qboolean skipping = qfalse;

	if ( !out || !rosterFile || !rosterFile[0] ) {
		return qfalse;
	}

	out->numWeapons = 0;
	for ( i = 0; i < ARMORY_MAX_EQUIP; i++ ) {
		out->equipPresent[i] = qfalse;
		out->equipRecommended[i] = qfalse;
		out->equipPerma[i] = qfalse;
	}

	len = trap_FS_FOpenFile( rosterFile, &f, FS_READ );
	if ( len <= 0 || !f ) {
		return qfalse;
	}
	if ( len >= ARMORY_FILE_BUFSIZE ) {
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	trap_FS_Read( fileBuf, len, f );
	fileBuf[len] = '\0';
	trap_FS_FCloseFile( f );

	p = fileBuf;
	while ( 1 ) {
		tok = COM_ParseExt( &p, qtrue );
		if ( !tok[0] ) {
			break;
		}

		if ( !Q_stricmp( tok, "#if" ) ) {
			char cvarname[128];
			char valueStr[64];
			int value, cvarVal;

			Q_strncpyz( cvarname, COM_ParseExt( &p, qfalse ), sizeof( cvarname ) );
			COM_ParseExt( &p, qfalse );   // operator token, only "==" is supported/expected
			Q_strncpyz( valueStr, COM_ParseExt( &p, qfalse ), sizeof( valueStr ) );

			value = atoi( valueStr );
			cvarVal = BG_Armory_CvarInt( cvarname );

			if ( ifDepth < ARMORY_MAX_IF_DEPTH ) {
				skipStack[ifDepth++] = ( cvarVal != value );
			}
			skipping = qfalse;
			for ( i = 0; i < ifDepth; i++ ) {
				if ( skipStack[i] ) {
					skipping = qtrue;
					break;
				}
			}
			continue;
		}

		if ( !Q_stricmp( tok, "#endif" ) ) {
			if ( ifDepth > 0 ) {
				ifDepth--;
			}
			skipping = qfalse;
			for ( i = 0; i < ifDepth; i++ ) {
				if ( skipStack[i] ) {
					skipping = qtrue;
					break;
				}
			}
			continue;
		}

		if ( skipping ) {
			continue;
		}

		if ( !Q_stricmp( tok, "weapon" ) ) {
			char classname[64];
			char marker[32];
			char *savedP;
			gitem_t *item;
			qboolean isRecommended;
			qboolean isPerma;

			Q_strncpyz( classname, COM_ParseExt( &p, qfalse ), sizeof( classname ) );

			savedP = p;
			Q_strncpyz( marker, COM_ParseExt( &p, qfalse ), sizeof( marker ) );
			if ( !Q_stricmp( marker, "recommended" ) ) {
				isRecommended = qtrue;
				isPerma = qfalse;
			} else if ( !Q_stricmp( marker, "perma" ) ) {
				isRecommended = qfalse;
				isPerma = qtrue;
			} else {
				isRecommended = qfalse;
				isPerma = qfalse;
				p = savedP;   // not a marker for this line - push back for the next loop iteration
			}

			item = BG_FindItemForClassName( classname );

			if ( item && item->giType == IT_WEAPON && out->numWeapons < ARMORY_MAX_ROSTER_WEAPONS ) {
				out->recommended[out->numWeapons] = isRecommended;
				out->perma[out->numWeapons] = isPerma;
				out->weapons[out->numWeapons++] = item->giTag;
			}
			continue;
		}

		if ( !Q_stricmp( tok, "equip" ) ) {
			char equipId[64];
			char marker[32];
			int idx;

			Q_strncpyz( equipId, COM_ParseExt( &p, qfalse ), sizeof( equipId ) );
			Q_strncpyz( marker, COM_ParseExt( &p, qfalse ), sizeof( marker ) );

			// listing an equip item at all is what makes it selectable - "recommended"/"perma" are extra markers
			for ( idx = 0; idx < ARMORY_NUM_EQUIP && idx < ARMORY_MAX_EQUIP; idx++ ) {
				if ( !Q_stricmp( armoryEquipList[idx].id, equipId ) ) {
					out->equipPresent[idx] = qtrue;
					if ( !Q_stricmp( marker, "recommended" ) ) {
						out->equipRecommended[idx] = qtrue;
					} else if ( !Q_stricmp( marker, "perma" ) ) {
						out->equipPerma[idx] = qtrue;
					}
					break;
				}
			}
			continue;
		}

		// unknown token: ignore (forward-compatible with future roster keywords)
	}

	return qtrue;
}

static char armoryPickupNames[MAX_ITEMS][MAX_QPATH];
static qboolean armoryPickupNamesLoaded = qfalse;

// Positional load of text/pickupnames.txt, mirroring CG_LoadPickupNames() in cg_main.c (cgame-only, not reachable from ui.qvm).
static void BG_Armory_LoadPickupNames( void ) {
	static char buffer[ARMORY_FILE_BUFSIZE];
	char *text;
	fileHandle_t f;
	int len, i;
	char *token;

	armoryPickupNamesLoaded = qtrue;   // set first so a missing file doesn't retry every call

	len = trap_FS_FOpenFile( "text/pickupnames.txt", &f, FS_READ );
	if ( len <= 0 || !f ) {
		return;
	}
	if ( len >= ARMORY_FILE_BUFSIZE ) {
		trap_FS_FCloseFile( f );
		return;
	}
	trap_FS_Read( buffer, len, f );
	buffer[len] = '\0';
	trap_FS_FCloseFile( f );

	text = buffer;
	for ( i = 0; i < bg_numItems && i < MAX_ITEMS; i++ ) {
		token = COM_ParseExt( &text, qtrue );
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp( token, "---" ) ) {
			if ( bg_itemlist[i].pickup_name && bg_itemlist[i].pickup_name[0] ) {
				Q_strncpyz( armoryPickupNames[i], bg_itemlist[i].pickup_name, MAX_QPATH );
			}
		} else {
			Q_strncpyz( armoryPickupNames[i], token, MAX_QPATH );
		}
	}
}

const char *BG_Armory_GetPickupName( const gitem_t *item ) {
	int index;

	if ( !item ) {
		return "";
	}
	if ( !armoryPickupNamesLoaded ) {
		BG_Armory_LoadPickupNames();
	}

	index = (int)( item - bg_itemlist );
	if ( index < 0 || index >= MAX_ITEMS || !armoryPickupNames[index][0] ) {
		return item->pickup_name;
	}
	return armoryPickupNames[index];
}

#ifndef GAMEDLL
extern qhandle_t trap_R_RegisterShaderNoMip( const char *name );
#endif

// bg_misc.c's gitem_t.icon is stale for several weapons; reads the real one from the .weap file instead.
qhandle_t BG_Armory_GetWeaponIconFromFile( weapon_t weaponNum ) {
#ifdef GAMEDLL
	return 0;
#else
	char *filename;
	char path[MAX_QPATH];
	int handle;
	pc_token_t token;
	qhandle_t icon = 0;

	filename = BG_GetWeaponFilename( weaponNum );
	if ( !filename || !filename[0] ) {
		return 0;
	}

	if ( BG_Armory_CvarInt( "g_vanilla_guns" ) ) {
		Com_sprintf( path, sizeof( path ), "weapons/vanilla/%s", filename );
	} else {
		Com_sprintf( path, sizeof( path ), "weapons/%s", filename );
	}

	handle = trap_PC_LoadSource( path );
	if ( !handle ) {
		return 0;
	}

	while ( trap_PC_ReadToken( handle, &token ) ) {
		if ( !Q_stricmp( token.string, "weaponIcon" ) ) {
			if ( trap_PC_ReadToken( handle, &token ) ) {
				icon = trap_R_RegisterShaderNoMip( token.string );
			}
			break;
		}
	}

	trap_PC_FreeSource( handle );
	return icon;
#endif
}

// Superset of cg_weapons.c/cg_draw.c's "wideweap" switch - also covers scope alt-fires and WP_HDM, whose art is 128x64 too.
qboolean BG_Armory_IsWideIcon( weapon_t weaponNum ) {
	switch ( weaponNum ) {
	case WP_THOMPSON:
	case WP_MP40:
	case WP_MP34:
	case WP_PPSH:
	case WP_MOSIN:
	case WP_G43:
	case WP_M1GARAND:
	case WP_BAR:
	case WP_M30:
	case WP_MP44:
	case WP_MG42M:
	case WP_M97:
	case WP_AUTO5:
	case WP_BROWNING:
	case WP_STEN:
	case WP_MAUSER:
	case WP_DELISLE:
	case WP_GARAND:
	case WP_VENOM:
	case WP_TESLA:
	case WP_PANZERFAUST:
	case WP_FLAMETHROWER:
	case WP_FG42:
	case WP_FG42SCOPE:
	case WP_M1941:
	case WP_SNIPERRIFLE:
	case WP_SNOOPERSCOPE:
	case WP_DELISLESCOPE:
	case WP_M1941SCOPE:
	case WP_HDM:
		return qtrue;
	default:
		return qfalse;
	}
}

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

static const armoryEquipDef_t armoryEquipList[] = {
	{ "fullammobag",    "Full Ammo Bag",     "icons/perk_fullammobag",    -1,                  "g_loadoutCostFullAmmoBag" },
	{ "heavyarmor",     "Heavy Armor",       "icons/perk_heavyarmor",     PERK_HEAVYARMOR,      "g_loadoutCostHeavyArmor" },
	{ "lightweight",    "Lightweight Gear",  "icons/perk_lightweight",    PERK_LIGHTWEIGHT,     "g_loadoutCostLightweightGear" },
	{ "tacticalgloves", "Tactical Gloves",   "icons/perk_tacticalgloves", PERK_TACTICALGLOVES,  "g_loadoutCostTacticalGloves" },
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

/*
===============
BG_Armory_LoadRoster

NOTE: this handles "#if"/"#endif" itself rather than relying on COM_ParseExt's
built-in support for it, since that's compiled GAMEDLL-only and would silently
no-op in cgame/ui builds.
===============
*/
qboolean BG_Armory_LoadRoster( const char *rosterFile, armoryRoster_t *out ) {
	static char fileBuf[ARMORY_FILE_BUFSIZE];
	fileHandle_t f;
	int len;
	char *p, *tok;
	qboolean skipping = qfalse;

	if ( !out || !rosterFile || !rosterFile[0] ) {
		return qfalse;
	}

	out->numWeapons = 0;

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

			skipping = ( cvarVal != value );
			continue;
		}

		if ( !Q_stricmp( tok, "#endif" ) ) {
			skipping = qfalse;
			continue;
		}

		if ( skipping ) {
			continue;
		}

		if ( !Q_stricmp( tok, "weapon" ) ) {
			char classname[64];
			gitem_t *item;

			Q_strncpyz( classname, COM_ParseExt( &p, qfalse ), sizeof( classname ) );
			item = BG_FindItemForClassName( classname );

			if ( item && item->giType == IT_WEAPON && out->numWeapons < ARMORY_MAX_ROSTER_WEAPONS ) {
				out->weapons[out->numWeapons++] = item->giTag;
			}
			continue;
		}

		// unknown token: ignore (forward-compatible with future roster keywords)
	}

	return qtrue;
}

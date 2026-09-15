/*
===========================================================================

g_armory.c -- hub armory loadout builder: server-side validation and
granting for the "sp_loadout_confirm" client command. GAMEDLL-only.

Never trusts the client: re-loads the current map's roster from disk and
re-checks the point budget here before granting anything, exactly like
every other server-authoritative give path in this codebase.

===========================================================================
*/

#include "g_local.h"
#include "../botlib/botlib.h"
#include "../botlib/be_aas.h"
#include "../botlib/be_ea.h"
#include "../botlib/be_ai_gen.h"
#include "../botlib/be_ai_goal.h"
#include "../botlib/be_ai_move.h"
#include "../botlib/botai.h"
#include "ai_cast.h"
#include "bg_armory.h"

// Reused rather than re-implemented, so paired-weapon grants (Colt->Akimbo etc.) stay consistent with every other give path.
qboolean AICast_ScriptAction_GiveWeapon( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SetAmmo( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SetClip( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_GivePerk( cast_state_t *cs, char *params );

#define ARMORY_MAX_PICKS 40

static qboolean G_Armory_WeaponInRoster( const armoryRoster_t *roster, int weapon ) {
	int i;

	for ( i = 0; i < roster->numWeapons; i++ ) {
		if ( roster->weapons[i] == weapon ) {
			return qtrue;
		}
	}
	return qfalse;
}

static qboolean G_Armory_EquipInRoster( const armoryRoster_t *roster, const armoryEquipDef_t *def ) {
	int count, idx;
	const armoryEquipDef_t *list = BG_Armory_GetEquipList( &count );

	idx = (int)( def - list );
	if ( idx < 0 || idx >= count || idx >= ARMORY_MAX_EQUIP ) {
		return qfalse;
	}
	return roster->equipPresent[idx];
}

// Comma-separated -> space-separated, so COM_ParseExt (splits on whitespace only) can tokenize it.
static void G_Armory_CommaToSpace( char *s ) {
	while ( *s ) {
		if ( *s == ',' ) {
			*s = ' ';
		}
		s++;
	}
}

/*
===============
G_Armory_Confirm

syntax (from g_cmds.c ClientCommand "sp_loadout_confirm"):
    sp_loadout_confirm <weapon_classname,weapon_classname,...> <equip_id,equip_id,...>
===============
*/
void G_Armory_Confirm( gentity_t *ent, const char *weaponArg, const char *equipArg ) {
	char mapname[MAX_QPATH];
	char rosterPath[MAX_QPATH];
	armoryRoster_t roster;
	cast_state_t *cs;

	char buf[1024];
	char *p, *tok;

	int pickedWeapons[ARMORY_MAX_PICKS];
	int numPickedWeapons = 0;

	const armoryEquipDef_t *pickedEquip[ARMORY_MAX_PICKS];
	int numPickedEquip = 0;
	qboolean fullAmmoBag = qfalse;
	qboolean grenadesFull = qfalse;

	int totalCost;
	int i;

	if ( !ent || !ent->client ) {
		return;
	}

	trap_Cvar_VariableStringBuffer( "mapname", mapname, sizeof( mapname ) );
	Com_sprintf( rosterPath, sizeof( rosterPath ), "loadouts/rosters/%s.armory", mapname );

	if ( !BG_Armory_LoadRoster( rosterPath, &roster ) ) {
		G_Printf( "sp_loadout_confirm: no roster for map '%s' (%s)\n", mapname, rosterPath );
		return;
	}

	// weapons not present in the freshly re-read roster (DLC included) are silently ignored
	Q_strncpyz( buf, weaponArg ? weaponArg : "", sizeof( buf ) );
	G_Armory_CommaToSpace( buf );
	p = buf;
	while ( 1 ) {
		gitem_t *item;

		tok = COM_ParseExt( &p, qfalse );
		if ( !tok[0] ) {
			break;
		}
		if ( numPickedWeapons >= ARMORY_MAX_PICKS ) {
			break;
		}

		item = BG_FindItemForClassName( tok );
		if ( !item || item->giType != IT_WEAPON ) {
			continue;
		}
		if ( !G_Armory_WeaponInRoster( &roster, item->giTag ) ) {
			continue;
		}

		pickedWeapons[numPickedWeapons++] = item->giTag;
	}

	// equipment: validate against the fixed 4-entry table, then against what this map's roster actually offers
	Q_strncpyz( buf, equipArg ? equipArg : "", sizeof( buf ) );
	G_Armory_CommaToSpace( buf );
	p = buf;
	while ( 1 ) {
		const armoryEquipDef_t *def;

		tok = COM_ParseExt( &p, qfalse );
		if ( !tok[0] ) {
			break;
		}
		if ( numPickedEquip >= ARMORY_MAX_PICKS ) {
			break;
		}

		def = BG_Armory_FindEquip( tok );
		if ( !def || !G_Armory_EquipInRoster( &roster, def ) ) {
			continue;
		}

		if ( !Q_stricmp( def->id, "fullammobag" ) ) {
			fullAmmoBag = qtrue;    // no perk, just an ammo-grant flag - weapons only, not grenades
		} else if ( !Q_stricmp( def->id, "grenades" ) ) {
			grenadesFull = qtrue;   // same idea as Full Ammo Bag, but scoped to grenade-type weapons
		}
		pickedEquip[numPickedEquip++] = def;
	}

	// budget: reject the whole thing if over, no partial application
	totalCost = 0;
	for ( i = 0; i < numPickedWeapons; i++ ) {
		totalCost += BG_Armory_GetWeaponCost( pickedWeapons[i] );
	}
	for ( i = 0; i < numPickedEquip; i++ ) {
		totalCost += BG_Armory_GetEquipCost( pickedEquip[i] );
	}
	if ( totalCost > g_loadoutPoints.integer ) {
		G_Printf( "sp_loadout_confirm: over budget (%d > %d), rejected\n", totalCost, g_loadoutPoints.integer );
		return;
	}

	// grant - applies immediately, no applyloadout/endmap bookkeeping
	cs = AICast_GetCastState( ent->s.number );

	AICast_ScriptAction_GiveWeapon( cs, "weapon_knife" );  // baseline, not counted against points

	for ( i = 0; i < numPickedWeapons; i++ ) {
		gitem_t *item = BG_FindItemForWeapon( pickedWeapons[i] );
		int maxAmmo = BG_GetMaxAmmo( &ent->client->ps, pickedWeapons[i], 1.0f );
		qboolean giveFull = BG_Armory_IsGrenadeWeapon( pickedWeapons[i] ) ? grenadesFull : fullAmmoBag;
		int target = giveFull ? maxAmmo : maxAmmo / 2;
		char args[64];

		AICast_ScriptAction_GiveWeapon( cs, item->classname );

		Com_sprintf( args, sizeof( args ), "%s %d", item->classname, target );
		AICast_ScriptAction_SetAmmo( cs, args );

		Com_sprintf( args, sizeof( args ), "%s full", item->classname );
		AICast_ScriptAction_SetClip( cs, args );
	}

	for ( i = 0; i < numPickedEquip; i++ ) {
		if ( pickedEquip[i]->perkTag >= 0 ) {
			char classname[64];

			Com_sprintf( classname, sizeof( classname ), "perk_%s", pickedEquip[i]->id );
			AICast_ScriptAction_GivePerk( cs, classname );
		}
	}
}

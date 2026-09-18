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
qboolean AICast_ScriptAction_SetArmor( cast_state_t *cs, char *params );

#define ARMORY_MAX_PICKS ARMORY_MAX_ROSTER_WEAPONS

// Baseline throwing-knife count; the "Additional Throwing Knives" equip raises it to the .weap cap instead.
#define ARMORY_KNIFE_BASE_AMMO 3

// Perma items are excluded here so a (stale or modified) client can never double-pick/double-charge them.
static qboolean G_Armory_WeaponInRoster( const armoryRoster_t *roster, int weapon ) {
	int i;

	for ( i = 0; i < roster->numWeapons; i++ ) {
		if ( roster->weapons[i] == weapon ) {
			return !roster->perma[i];
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
	return roster->equipPresent[idx] && !roster->equipPerma[idx];
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

// Shared by every give path below (roster weapons and weapon-granting equip picks alike).
static void G_Armory_GrantWeaponWithAmmo( cast_state_t *cs, gentity_t *ent, weapon_t weaponNum, qboolean fullAmmoBag, qboolean grenadesFull, qboolean extraKnives ) {
	gitem_t *item = BG_FindItemForWeapon( weaponNum );
	int maxAmmo;
	char args[64];

	if ( !item ) {
		return;
	}

	AICast_ScriptAction_GiveWeapon( cs, item->classname );

	maxAmmo = BG_GetMaxAmmo( &ent->client->ps, weaponNum, 1.0f );
	if ( maxAmmo > 0 ) {
		int target;

		if ( weaponNum == WP_KNIFE ) {
			// "Additional Throwing Knives" equip: full (the .weap-defined cap) instead of the baseline count.
			target = extraKnives ? maxAmmo : ARMORY_KNIFE_BASE_AMMO;
			if ( target > maxAmmo ) {
				target = maxAmmo;
			}
		} else {
			qboolean giveFull = BG_Armory_IsGrenadeWeapon( weaponNum ) ? grenadesFull : fullAmmoBag;
			target = giveFull ? maxAmmo : maxAmmo / 2;
		}

		Com_sprintf( args, sizeof( args ), "%s %d", item->classname, target );
		AICast_ScriptAction_SetAmmo( cs, args );

		Com_sprintf( args, sizeof( args ), "%s full", item->classname );
		AICast_ScriptAction_SetClip( cs, args );
	}
}

// Shared by both perk-granting loops below - Heavy Armor also needs an immediate armor grant, not just the raised cap.
static void G_Armory_GrantPerk( cast_state_t *cs, gentity_t *ent, const armoryEquipDef_t *def ) {
	char classname[64];

	if ( def->perkTag < 0 ) {
		return;
	}

	Com_sprintf( classname, sizeof( classname ), "perk_%s", def->id );
	AICast_ScriptAction_GivePerk( cs, classname );

	if ( def->perkTag == PERK_HEAVYARMOR ) {
		char armorArgs[16];

		Com_sprintf( armorArgs, sizeof( armorArgs ), "%d", G_GetArmorCap( ent->client ) );
		AICast_ScriptAction_SetArmor( cs, armorArgs );
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
	qboolean extraKnivesFull = qfalse;
	qboolean wantBinoculars = qfalse;

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
		} else if ( !Q_stricmp( def->id, "extraknives" ) ) {
			extraKnivesFull = qtrue;   // same idea, scoped to the knife's throwing-knife count
		} else if ( !Q_stricmp( def->id, "binoculars" ) ) {
			wantBinoculars = qtrue;    // no perk/weapon - grants the INV_BINOCS inventory bit directly
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

	// perma equip: mapper-forced picks, contribute to the ammo-boost flags exactly like a real pick would
	{
		int equipCount, ei;
		const armoryEquipDef_t *equipList = BG_Armory_GetEquipList( &equipCount );

		for ( ei = 0; ei < equipCount && ei < ARMORY_MAX_EQUIP; ei++ ) {
			if ( !roster.equipPerma[ei] ) {
				continue;
			}
			if ( !Q_stricmp( equipList[ei].id, "fullammobag" ) ) {
				fullAmmoBag = qtrue;
			} else if ( !Q_stricmp( equipList[ei].id, "grenades" ) ) {
				grenadesFull = qtrue;
			} else if ( !Q_stricmp( equipList[ei].id, "extraknives" ) ) {
				extraKnivesFull = qtrue;
			} else if ( !Q_stricmp( equipList[ei].id, "binoculars" ) ) {
				wantBinoculars = qtrue;
			}
			G_Armory_GrantPerk( cs, ent, &equipList[ei] );
			if ( equipList[ei].weaponTag != WP_NONE ) {
				G_Armory_GrantWeaponWithAmmo( cs, ent, equipList[ei].weaponTag, fullAmmoBag, grenadesFull, extraKnivesFull );
			}
		}
	}

	if ( wantBinoculars ) {
		ent->client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );
	}

	// perma weapons: mapper-forced picks, always granted, free of charge
	for ( i = 0; i < roster.numWeapons; i++ ) {
		if ( !roster.perma[i] ) {
			continue;
		}
		G_Armory_GrantWeaponWithAmmo( cs, ent, roster.weapons[i], fullAmmoBag, grenadesFull, extraKnivesFull );
	}

	for ( i = 0; i < numPickedWeapons; i++ ) {
		G_Armory_GrantWeaponWithAmmo( cs, ent, pickedWeapons[i], fullAmmoBag, grenadesFull, extraKnivesFull );
	}

	for ( i = 0; i < numPickedEquip; i++ ) {
		G_Armory_GrantPerk( cs, ent, pickedEquip[i] );
		if ( pickedEquip[i]->weaponTag != WP_NONE ) {
			G_Armory_GrantWeaponWithAmmo( cs, ent, pickedEquip[i]->weaponTag, fullAmmoBag, grenadesFull, extraKnivesFull );
		}
	}
}

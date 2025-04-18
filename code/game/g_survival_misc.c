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

// g_survival_misc.c

#include "g_local.h"
#include "g_survival.h"

svParams_t svParams;

/*
============
TossClientItems
============
*/
void TossClientItems(gentity_t *self, gentity_t *attacker) {
    gitem_t *item;
    vec3_t forward;
    float angle;
    gentity_t *drop = NULL;

    if (!attacker || !attacker->client) return;
    if (attacker->aiTeam == self->aiTeam) return;

    const char *treasure = "item_treasure";
    AngleVectors(self->r.currentAngles, forward, NULL, NULL);
    angle = 45;

    int dropChance = svParams.treasureDropChance;
    if (attacker->client->ps.perks[PERK_SCAVENGER] > 0) {
        dropChance += svParams.treasureDropChanceScavengerIncrease;
    }

    if (rand() % 100 < dropChance) {
        item = BG_FindItemForClassName(treasure);
        if (item) {
            drop = Drop_Item(self, item, 0, qfalse);
            if (drop) {
                drop->nextthink = level.time + 30000;
            }
        }
    }
}

/*
============
TossClientPowerups
============
*/
void TossClientPowerups(gentity_t *self, gentity_t *attacker) {
    gitem_t *item;
    vec3_t forward;
    float angle;
    gentity_t *drop = NULL;
    int powerup = 0;

    if (!attacker || !attacker->client) return;
    if (attacker->aiTeam == self->aiTeam) return;

    AngleVectors(self->r.currentAngles, forward, NULL, NULL);
    angle = 45;

    int dropChance = svParams.powerupDropChance;
    if (attacker->client->ps.perks[PERK_SCAVENGER] > 0) {
        dropChance += svParams.powerupDropChanceScavengerIncrease;
    }

    if (rand() % 100 < dropChance) {
        switch (rand() % 4) {
            case 0: powerup = PW_QUAD; break;
            case 1: powerup = PW_BATTLESUIT_SURV; break;
            case 2: powerup = PW_VAMPIRE; break;
            case 3: powerup = PW_AMMO; break;
        }

        item = BG_FindItemForPowerup(powerup);
        if (item) {
            drop = Drop_Item(self, item, 0, qfalse);
            if (drop) {
                drop->nextthink = level.time + 30000;
            }
        }
    }
}
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



// g_survival.h

#ifndef __G_SURVIVAL_H__
#define __G_SURVIVAL_H__

// Everything related to the score system
void Survival_AddKillScore(gentity_t *attacker, gentity_t *victim, int meansOfDeath);
void Survival_AddHeadshotBonus(gentity_t *attacker, gentity_t *victim);
void Survival_AddPainScore(gentity_t *attacker, gentity_t *victim, int damage);
void Survival_PickupTreasure(gentity_t *other);
qboolean Survival_TrySpendMG42Points(gentity_t *player);

// Misc stuff
void TossClientItems(gentity_t *self, gentity_t *attacker);
void TossClientPowerups(gentity_t *self, gentity_t *attacker);

#endif // __G_SURVIVAL_H__
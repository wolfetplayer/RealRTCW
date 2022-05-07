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

/*
 * name:		bg_misc.c
 *
 * desc:		both games misc functions, all completely stateless
 *
*/


#include "../qcommon/q_shared.h"
#include "bg_public.h"

#ifdef CGAMEDLL
extern vmCvar_t cg_gameType;
#endif
#ifdef GAMEDLL
extern vmCvar_t g_gametype;
#endif


// NOTE: weapons that share ammo (ex. colt/thompson) need to share max ammo, but not necessarily uses or max clip
// RealRTCW ammo depends on difficulty level now. So look for the ammo references in g_client.c. Values in bg misc should be the LOWEST to avoid auto pickup bug.
#define MAX_AMMO_45     150
#define MAX_AMMO_9MM    150
#define MAX_AMMO_VENOM  500
#define MAX_AMMO_MAUSER 150
#define MAX_AMMO_GARAND 5
#define MAX_AMMO_FG42   MAX_AMMO_MAUSER
#define MAX_AMMO_BAR    150
#define MAX_AMMO_TTAMMO 200
#define MAX_AMMO_MOSINA 50
#define MAX_AMMO_BARAMMO    120  
#define MAX_AMMO_44AMMO     120
#define MAX_AMMO_M97        24
#define MAX_AMMO_REVOLVER   24
#define MAX_AMMO_MG42M      200

//  [0] = maxammo            - max player ammo carrying capacity.
//  [1] = uses               - how many 'rounds' it takes/costs to fire one cycle.
//  [2] = maxclip            - max 'rounds' in a clip.
//  [3] = reloadTime         - time from start of reload until ready to fire.
//  [4] = fireDelayTime      - time from pressing 'fire' until first shot is fired. (used for delaying fire while weapon is 'readied' in animation)
//  [5] = nextShotTime       - when firing continuously, this is the time between shots
//  [6] = nextShotTime2      - alt fire rates
//  [6] = maxHeat            - max active firing time before weapon 'overheats' (at which point the weapon will fail for a moment)
//  [7] = coolRate           - how fast the weapon cools down.
//  [8] = playerDamage       - damage inflicted by player
//  [9] = aiDamage           - damage inflicted by AI
// [10] = playerSplashRadius - explosives only
// [11] = aiSplashRadius     - explosives only
// [12] = spread             - spread value
// [13] = aimSpreadScaleadd  - how much spread increasing per shot
// [14] = spreadScale        - how quickly spread will reduce
// [15] = weapRecoilDuration - basic recoil value
// [16] = weapRecoilPitch    - vertical recoil
// [17] = weapRecoilYaw      - horizontal recoil
// [18] = soundRange         - ai hearing range for weapon shots
// [19] = moveSpeed          - player movement speed
// [20] = twoHand            - is weapon twohanded?
// [21] = upAngle            - throw range for grenades
// [22] = mod                - means of death


// NOTE: This once-static data is included in both Client and Game modules.
//       Both now load values into here from weap files.
ammotable_t ammoTable[] = {
	
	//	maxammo		     uses amt.	  maxclip	reloadtime   firedelay	nextshot   nextshot2  heat    cool	  plrdmg    aidmg       plrsplsh    aisplsh     spread      SpreadScaleAdd      spreadScale      recoilDuration   recoilPitch      recoilYaw           soundrange          movespeed           twohand          upAngle         mod                  
	{   0,                   0,       0,        0,           50,        0,         0,         0,      0,      0,        0,          0,          0,          0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1.00,               1.00,               0,               0,              0,                                         },  //	WP_NONE					// 0

	{   999,                 0,       999,      0,           50,        200,       200,       0,      0,      10,       6,          0,          0,          0,          0,                  0.0f,            0,               {0, 0},          {0,0},              64,                 1.00,               0,               0,              MOD_KNIFE,                                 },  //	WP_KNIFE				// 1

	{   MAX_AMMO_9MM,        1,       8,        1500,        100,       300,       300,       0,      0,      7,        5,          0,          0,          400,        35,                 0.3f,            50,              {.2f, .1f},      {0,0},              700,                0.95,               0,               0,              MOD_LUGER,                                 },  //	WP_LUGER				// 2	
	{   MAX_AMMO_9MM,        1,       32,       2600,        100,       110,       110,       0,      0,      6,        4,          0,          0,          850,        15,                 0.5f,            30,              {.1f, .1f},      {0,0},              1000,               0.90,               1,               0,              MOD_MP40,                                  },  //	WP_MP40					// 3
	{   MAX_AMMO_MAUSER,     1,       5,        2500,        100,       1400,      1400,      0,      0,      35,       15,         0,          0,          300,        50,                 0.5f,            60,              {1.0f, 1.0f},    {.1f, .1f},         2000,               0.90,               1,               0,              MOD_MAUSER,                                },  //	WP_MAUSER				// 4	
	{   MAX_AMMO_FG42,       1,       20,       2000,        100,       190,       120,       0,      0,      12,       6,          0,          0,          600,        15,                 0.7f,            40,              {.1f, .1f},      {0,0},              1500,               0.90,               1,               0,              MOD_FG42,                                  },  //	WP_FG42					// 5
	{   5,                   1,       5,        1000,        250,       1600,      1600,      0,      0,      170,      170,        310,        310,        0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1500,               0.95,               0,               800,            MOD_GRENADE_LAUNCHER,                      },  //	WP_GRENADE_LAUNCHER		// 6
	{   3,                   1,       1,        1000,        50,        2000,      2000,      0,      0,      250,      100,        300,        200,        0,          30,                 0.6f,            0,               {.0, 0},         {0,0},              1000,               0.85,               1,               0,              MOD_PANZERFAUST,                           },  //	WP_PANZERFAUST			// 7
	{   MAX_AMMO_VENOM,      1,       500,      3000,        750,       45,        45,        5000,   200,    20,       7,          0,          0,          1000,       10,                 0.9f,            50,              {.1f, .1f},      {.1f, .1f},         1000,               0.85,               1,               0,              MOD_VENOM,                                 },  //	WP_VENOM				// 8	
	{   100,                 1,       100,      1000,        100,       50,        50,        0,      0,      6,        6,          0,          0,          0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1000,               0.85,               1,               0,              MOD_FLAMETHROWER,                          },  //	WP_FLAMETHROWER			// 9
	{   50,                  1,       50,       1000,        100,       250,       250,       0,      0,      15,       4,          0,          0,          0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1000,               0.85,               1,               0,              MOD_TESLA,                                 },  //	WP_TESLA				// 10
	{   MAX_AMMO_9MM,        1,       32,       3100,        100,       105,       105,       0,      0,      6,        4,          0,          0,          900,        15,                 0.5f,            30,              {.1f, .1f},      {0,0},              1000,               0.90,               1,               0,              MOD_MP34,                                  },  //	WP_MP34					// 11
	{   MAX_AMMO_TTAMMO,     1,       8,        1600,        100,       350,       350,       0,      0,      8,        7,          0,          0,          450,        35,                 0.3f,            50,              {.2f, .1f},      {0,0},              700,                0.95,               0,               0,              MOD_TT33,                                  },  //	WP_TT33					// 12
	{   MAX_AMMO_TTAMMO,     1,       71,       2900,        100,       65,        65,        0,      0,      6,        5,          0,          0,          1000,       15,                 0.5f,            30,              {.1f, .1f},      {0,0},              1000,               0.90,               1,               0,              MOD_PPSH,                                  },  //	WP_PPSH					// 13
	{   MAX_AMMO_MOSINA,     1,       5,        2400,        100,       1400,      1400,      0,      0,      35,       15,         0,          0,          300,        50,                 0.5f,            60,              {1.0f, 1.0f},    {.1f, .1f},         2000,               0.90,               1,               0,              MOD_MOSIN,                                 },  //	WP_MOSIN				// 14
	{   MAX_AMMO_MAUSER,     1,       10,       1800,        100,       300,       300,       0,      0,      16,       7,          0,          0,          350,        40,                 0.4f,            40,              {.2f,.2f},       {.1f, .1f},         2000,               0.90,               1,               0,              MOD_G43,                                   },  //	WP_G43				    // 15
	{   MAX_AMMO_BARAMMO,    1,       8,        1650,        100,       300,       300,       0,      0,      18,       7,          0,          0,          350,        40,                 0.4f,            40,              {.2f,.2f},       {.1f, .1f},         2000,               0.90,               1,               0,              MOD_M1GARAND,                              },  //	WP_M1GARAND				// 16
	{   5,                   1,       1,        3000,        100,       400,       400,       0,      0,      20,       40,         0,          0,          0,          0,                  0.0f,            0,               {0,0},           {0, 0},             1500,               0.90,               1,               0,              MOD_M7,                                    },  //	WP_M7			        // 17
	{   MAX_AMMO_BARAMMO,    1,       20,       2250,        100,       200,       100,       0,      0,      16,       6,          0,          0,          700,        15,                 0.6f,            40,              {.1f, .1f},      {0,0},              1500,               0.90,               1,               0,              MOD_BAR,                                   },  //	WP_BAR					// 18
	{   MAX_AMMO_44AMMO,     1,       30,       2600,        100,       105,       170,       0,      0,      9,        6,          0,          0,          800,        15,                 0.6f,            40,              {.1f, .1f},      {0,0},              1500,               0.90,               1,               0,              MOD_MP44,                                  },  //	WP_MP44					// 19
	{   MAX_AMMO_VENOM,      1,       100,      2600,        100,       65,        65,        2500,   350,    15,       6,          0,          0,          1200,       15,                 0.6f,            50,              {.1f, .1f},      {.1f, .1f},         1500,               0.85,               1,               0,              MOD_MG42M,                                 },  //	WP_MG42M                // 20
	{   MAX_AMMO_BARAMMO,    1,       150,      2600,        100,       65,        65,        2500,   350,    15,       6,          0,          0,          1000,       15,                 0.6f,            75,              {.1f, .1f},      {.1f, .1f},         1500,               0.85,               1,               0,              MOD_BROWNING,                              },  //	WP_BROWNING             // 21
	{   MAX_AMMO_M97,        1,       6,        2000,        100,       1250,      1250,      0,      0,      10,       9,          0,          0,          4500,       15,                 0.6f,            100,             {.10f, .2f},     {.5f, .5f},         1500,               0.90,               1,               0,              MOD_M97,                                   },  //	WP_M97                  // 22
	{   MAX_AMMO_REVOLVER,   1,       6,        1500,        100,       500,       500,       0,      0,      20,       7,          0,          0,          350,        35,                 0.4f,            50,              {.3f, .1f},      {0,0},              1000,               0.95,               0,               0,              MOD_REVOLVER,                              },  //	WP_REVOLVER             // 23
	{   MAX_AMMO_45,         1,       7,        1500,        100,       300,       300,       0,      0,      10,       6,          0,          0,          400,        35,                 0.4f,            50,              {.2f, .1f},      {0,0},              700,                0.95,               0,               0,              MOD_COLT,                                  },  //	WP_COLT					// 24
	{   MAX_AMMO_45,         1,       30,       2400,        100,       90,        90,        0,      0,      9,        5,          0,          0,          950,        15,                 0.4f,            30,              {.2f, .2f},      {0,0},              1000,               0.90,               1,               0,              MOD_THOMPSON,                              },  //	WP_THOMPSON				// 25
	{   MAX_AMMO_GARAND,     1,       5,        2500,        100,       1200,      1200,      0,      0,      40,       15,         0,          0,          400,        50,                 0.5f,            50,              {1.0f, 1.0f},    {.1f,.1f},          128,                0.90,               1,               0,              MOD_GARAND,                                },  //	WP_GARAND				// 26
	{   5,                   1,       5,        1000,        250,       1600,      1600,      0,      0,      220,      220,        270,        270,        0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1500,               0.95,               0,               800,            MOD_GRENADE_PINEAPPLE,                     },  //	WP_GRENADE_PINEAPPLE	// 27
	{   999,                 0,       999,      0,           50,        0,         0,         0,      0,      220,      220,        270,        270,        0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1500,               0.95,               0,               700,            0,                                         },  //	WP_AIRSTRIKE	        // 28
	{   5,                   0,       5,        0,           50,        0,         0,         0,      0,      220,      220,        270,        270,        0,          0,                  0.0f,            0,               {0, 0},          {0,0},              1500,               0.95,               0,               700,            MOD_POISONGAS,                             },  //	WP_POISONGAS	        // 29
	{   MAX_AMMO_MAUSER,     1,       5,        3000,        0,         1400,      1400,      0,      0,      35,       15,         0,          0,          300,        0,                  10.0f,           0,               {0,0},           {0,0},              2000,               0.40,               1,               0,              MOD_SNIPERRIFLE,                           },  //	WP_SNIPER_GER			// 30
	{   MAX_AMMO_GARAND,     1,       5,        3000,        0,         1200,      1200,      0,      0,      40,       15,         0,          0,          300,        0,                  8.0f,            0,               {0,0},           {0,0},              128,                0.40,               1,               0,              MOD_SNOOPERSCOPE,                          },  //	WP_SNIPER_AM			// 31

	{   MAX_AMMO_FG42,       1,       20,       2000,        100,       180,       180,       0,      0,      12,       6,          0,          0,          250,        5,                  0.7f,            0,               {0,0},           {0,0},              1500,               0.40,               1,               0,              MOD_FG42SCOPE,                             },  //	WP_FG42SCOPE			// 32
	{   MAX_AMMO_9MM,        1,       32,       3100,        100,       115,       115,       900,    500,    7,        4,          0,          0,          950,        15,                 0.6f,            40,              {.1f, .1f},      {0,0},              64,                 0.90,               1,               0,              MOD_STEN,                                  },  //	WP_STEN					// 33
	{   MAX_AMMO_9MM,        1,       8,        1500,        100,       300,       300,       0,      0,      7,        5,          0,          0,          350,        35,                 0.3f,            50,              {.2f, .1f},      {0,0},              64,                 0.95,               0,               0,              MOD_SILENCER,                              },  //	WP_SILENCER				// 34
	{   MAX_AMMO_45,         1,       7,        2700,        100,       200,       200,       0,      0,      10,       6,          0,          0,          500,        35,                 0.5f,            50,              {.2f, .1f},      {0,0},              700,                0.95,               1,               0,              MOD_AKIMBO,                                },  //	WP_AKIMBO				// 35

	{   3,                   1,       3,        1000,        250,       1600,      1600,      0,      0,      800,      800,        450,        450,        0,          0,                  0.0f,            0,               {0,0},           {0,0},              3000,               0.95,               0,               400,            MOD_DYNAMITE,                              },  //	WP_DYNAMITE				// 36

	{   999,                 0,       999,      0,           50,        1000,      1000,      0,      0,      0,        0,          0,          0,          0,          0,                  0.0f,            0,               {0,0},           {0,0},              1000,               0,                  0,               0,              0,                                         },  //	WP_MONSTER_ATTACK1		// 37
	{   999,                 0,       999,      0,           50,        250,       250,       0,      0,      0,        0,          0,          0,          0,          0,                  0.0f,            0,               {0,0},           {0,0},              1000,               0,                  0,               0,              0,                                         },  //	WP_MONSTER_ATTACK2		// 38
	{   999,                 0,       999,      0,           50,        250,       250,       0,      0,      0,        0,          0,          0,          0,          0,                  0.0f,            0,               {0,0},           {0,0},              1000,               0,                  0,               0,              0,                                         },  //	WP_MONSTER_ATTACK3		// 39
	{   999,                 0,       999,      0,           50,        250,       250,       0,      0,      0,        0,          0,          0,          0,          0,                  0.0f,            0,               {0,0},           {0,0},              64,                 0,                  0,               0,              0,                                         }   //	WP_GAUNTLET				// 40
};

// Skill-based ammo parameters
ammoskill_t ammoSkill[GSKILL_NUM_SKILLS][WP_NUM_WEAPONS];

int weapAlts[] = {
	WP_NONE,            // 0 WP_NONE
	WP_NONE,            // 1 WP_KNIFE
	WP_SILENCER,        // 2 WP_LUGER
	WP_NONE,            // 3 WP_MP40
	WP_SNIPERRIFLE,     // 4 WP_MAUSER
	WP_FG42SCOPE,       // 5 WP_FG42	
	WP_NONE,            // 6 WP_GRENADE_LAUNCHER
	WP_NONE,            // 7 WP_PANZERFAUST
	WP_NONE,            // 8 WP_VENOM
	WP_NONE,            // 9 WP_FLAMETHROWER
	WP_NONE,            // 10 WP_TESLA
	WP_NONE,            // 11 WP_MP34
	WP_NONE,            // 12 WP_TT33
	WP_NONE,            // 13 WP_PPSH
	WP_NONE,            // 14 WP_MOSIN
	WP_NONE,            // 15 WP_G43
	WP_M7,              // 16 WP_M1GARAND
	WP_M1GARAND,        // 17 WP_M7
	WP_NONE,            // 18 WP_BAR
	WP_NONE,            // 19 WP_MP44
	WP_NONE,            // 20 WP_MG42M
	WP_NONE,            // 21 WP_BROWNING
	WP_NONE,            // 22 WP_M97
	WP_NONE,            // 23 WP_REVOLVER
	WP_AKIMBO,          // 24 WP_COLT		
	WP_NONE,            // 25 WP_THOMPSON
	WP_SNOOPERSCOPE,    // 26 WP_GARAND		
	WP_NONE,            // 27 WP_GRENADE_PINEAPPLE
	WP_NONE,            // 28 WP_AIRSTRIKE
	WP_NONE,            // 29 WP_POISONGAS
	WP_MAUSER,          // 30 WP_SNIPERRIFLE
	WP_GARAND,          // 31 WP_SNOOPERSCOPE
	WP_FG42,            // 32 WP_FG42SCOPE
	WP_NONE,            // 33 WP_STEN
	WP_LUGER,           // 34 WP_SILENCER	
	WP_COLT,            // 35 WP_AKIMBO		
	WP_NONE             // 36 WP_DYNAMITE
};


// new (10/18/00)
char *animStrings[] = {
	"BOTH_DEATH1",
	"BOTH_DEAD1",
	"BOTH_DEAD1_WATER",
	"BOTH_DEATH2",
	"BOTH_DEAD2",
	"BOTH_DEAD2_WATER",
	"BOTH_DEATH3",
	"BOTH_DEAD3",
	"BOTH_DEAD3_WATER",

	"BOTH_CLIMB",
	"BOTH_CLIMB_DOWN",
	"BOTH_CLIMB_DISMOUNT",

	"BOTH_SALUTE",

	"BOTH_PAIN1",
	"BOTH_PAIN2",
	"BOTH_PAIN3",
	"BOTH_PAIN4",
	"BOTH_PAIN5",
	"BOTH_PAIN6",
	"BOTH_PAIN7",
	"BOTH_PAIN8",

	"BOTH_GRAB_GRENADE",

	"BOTH_ATTACK1",
	"BOTH_ATTACK2",
	"BOTH_ATTACK3",
	"BOTH_ATTACK4",
	"BOTH_ATTACK5",

	"BOTH_EXTRA1",
	"BOTH_EXTRA2",
	"BOTH_EXTRA3",
	"BOTH_EXTRA4",
	"BOTH_EXTRA5",
	"BOTH_EXTRA6",
	"BOTH_EXTRA7",
	"BOTH_EXTRA8",
	"BOTH_EXTRA9",
	"BOTH_EXTRA10",
	"BOTH_EXTRA11",
	"BOTH_EXTRA12",
	"BOTH_EXTRA13",
	"BOTH_EXTRA14",
	"BOTH_EXTRA15",
	"BOTH_EXTRA16",
	"BOTH_EXTRA17",
	"BOTH_EXTRA18",
	"BOTH_EXTRA19",
	"BOTH_EXTRA20",

	"TORSO_GESTURE",
	"TORSO_GESTURE2",
	"TORSO_GESTURE3",
	"TORSO_GESTURE4",

	"TORSO_DROP",

	"TORSO_RAISE",   // (low)
	"TORSO_ATTACK",
	"TORSO_STAND",
	"TORSO_STAND_ALT1",
	"TORSO_STAND_ALT2",
	"TORSO_READY",
	"TORSO_RELAX",

	"TORSO_RAISE2",  // (high)
	"TORSO_ATTACK2",
	"TORSO_STAND2",
	"TORSO_STAND2_ALT1",
	"TORSO_STAND2_ALT2",
	"TORSO_READY2",
	"TORSO_RELAX2",

	"TORSO_RAISE3",  // (pistol)
	"TORSO_ATTACK3",
	"TORSO_STAND3",
	"TORSO_STAND3_ALT1",
	"TORSO_STAND3_ALT2",
	"TORSO_READY3",
	"TORSO_RELAX3",

	"TORSO_RAISE4",  // (shoulder)
	"TORSO_ATTACK4",
	"TORSO_STAND4",
	"TORSO_STAND4_ALT1",
	"TORSO_STAND4_ALT2",
	"TORSO_READY4",
	"TORSO_RELAX4",

	"TORSO_RAISE5",  // (throw)
	"TORSO_ATTACK5",
	"TORSO_ATTACK5B",
	"TORSO_STAND5",
	"TORSO_STAND5_ALT1",
	"TORSO_STAND5_ALT2",
	"TORSO_READY5",
	"TORSO_RELAX5",

	"TORSO_RELOAD1", // (low)
	"TORSO_RELOAD2", // (high)
	"TORSO_RELOAD3", // (pistol)
	"TORSO_RELOAD4", // (shoulder)

	"TORSO_MG42",        // firing tripod mounted weapon animation

	"TORSO_MOVE",        // torso anim to play while moving and not firing (swinging arms type thing)
	"TORSO_MOVE_ALT",        // torso anim to play while moving and not firing (swinging arms type thing)

	"TORSO_EXTRA",
	"TORSO_EXTRA2",
	"TORSO_EXTRA3",
	"TORSO_EXTRA4",
	"TORSO_EXTRA5",
	"TORSO_EXTRA6",
	"TORSO_EXTRA7",
	"TORSO_EXTRA8",
	"TORSO_EXTRA9",
	"TORSO_EXTRA10",

	"LEGS_WALKCR",
	"LEGS_WALKCR_BACK",
	"LEGS_WALK",
	"LEGS_RUN",
	"LEGS_BACK",
	"LEGS_SWIM",
	"LEGS_SWIM_IDLE",

	"LEGS_JUMP",
	"LEGS_JUMPB",
	"LEGS_LAND",

	"LEGS_IDLE",
	"LEGS_IDLE_ALT", //	"LEGS_IDLE2"
	"LEGS_IDLECR",

	"LEGS_TURN",

	"LEGS_BOOT",     // kicking animation

	"LEGS_EXTRA1",
	"LEGS_EXTRA2",
	"LEGS_EXTRA3",
	"LEGS_EXTRA4",
	"LEGS_EXTRA5",
	"LEGS_EXTRA6",
	"LEGS_EXTRA7",
	"LEGS_EXTRA8",
	"LEGS_EXTRA9",
	"LEGS_EXTRA10",
};


// old
char *animStringsOld[] = {
	"BOTH_DEATH1",
	"BOTH_DEAD1",
	"BOTH_DEATH2",
	"BOTH_DEAD2",
	"BOTH_DEATH3",
	"BOTH_DEAD3",

	"BOTH_CLIMB",
	"BOTH_CLIMB_DOWN",
	"BOTH_CLIMB_DISMOUNT",

	"BOTH_SALUTE",

	"BOTH_PAIN1",
	"BOTH_PAIN2",
	"BOTH_PAIN3",
	"BOTH_PAIN4",
	"BOTH_PAIN5",
	"BOTH_PAIN6",
	"BOTH_PAIN7",
	"BOTH_PAIN8",

	"BOTH_EXTRA1",
	"BOTH_EXTRA2",
	"BOTH_EXTRA3",
	"BOTH_EXTRA4",
	"BOTH_EXTRA5",

	"TORSO_GESTURE",
	"TORSO_GESTURE2",
	"TORSO_GESTURE3",
	"TORSO_GESTURE4",

	"TORSO_DROP",

	"TORSO_RAISE",   // (low)
	"TORSO_ATTACK",
	"TORSO_STAND",
	"TORSO_READY",
	"TORSO_RELAX",

	"TORSO_RAISE2",  // (high)
	"TORSO_ATTACK2",
	"TORSO_STAND2",
	"TORSO_READY2",
	"TORSO_RELAX2",

	"TORSO_RAISE3",  // (pistol)
	"TORSO_ATTACK3",
	"TORSO_STAND3",
	"TORSO_READY3",
	"TORSO_RELAX3",

	"TORSO_RAISE4",  // (shoulder)
	"TORSO_ATTACK4",
	"TORSO_STAND4",
	"TORSO_READY4",
	"TORSO_RELAX4",

	"TORSO_RAISE5",  // (throw)
	"TORSO_ATTACK5",
	"TORSO_ATTACK5B",
	"TORSO_STAND5",
	"TORSO_READY5",
	"TORSO_RELAX5",

	"TORSO_RELOAD1", // (low)
	"TORSO_RELOAD2", // (high)
	"TORSO_RELOAD3", // (pistol)
	"TORSO_RELOAD4", // (shoulder)

	"TORSO_MG42",        // firing tripod mounted weapon animation

	"TORSO_MOVE",        // torso anim to play while moving and not firing (swinging arms type thing)

	"TORSO_EXTRA2",
	"TORSO_EXTRA3",
	"TORSO_EXTRA4",
	"TORSO_EXTRA5",

	"LEGS_WALKCR",
	"LEGS_WALKCR_BACK",
	"LEGS_WALK",
	"LEGS_RUN",
	"LEGS_BACK",
	"LEGS_SWIM",

	"LEGS_JUMP",
	"LEGS_LAND",

	"LEGS_IDLE",
	"LEGS_IDLE2",
	"LEGS_IDLECR",

	"LEGS_TURN",

	"LEGS_BOOT",     // kicking animation

	"LEGS_EXTRA1",
	"LEGS_EXTRA2",
	"LEGS_EXTRA3",
	"LEGS_EXTRA4",
	"LEGS_EXTRA5",
};

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) SUSPENDED SPIN PERSISTANT
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
SUSPENDED - will allow items to hang in the air, otherwise they are dropped to the next surface.
SPIN - will allow items to spin in place.
PERSISTANT - some items (ex. clipboards) can be picked up, but don't disappear

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
"stand" if the item has a stand (ex: mp40_stand.md3) this specifies which stand tag to attach the weapon to ("stand":"4" would mean "tag_stand4" for example)  only weapons support stands currently
*/

gitem_t bg_itemlist[] =
{
	{
		NULL,  // classname
		NULL,  // pickup sound
		{ NULL, //model1
		  NULL, //model2
		  0  }, //model3
		NULL,   // icon
		NULL,   // pickup name
		0,      // quantity
		0,      //giType
		WP_NONE, //giWeapon
		0,       //giTag
		0,          // ammoindex
		0,          // clipindex
		"",          // precache
		"",          // sounds
		{0,0,0,0,0}   // gameskill
	},  // leave index 0 alone



/*QUAKED item_clipboard (1 1 0) (-8 -8 -8) (8 8 8) SUSPENDED SPIN PERSISTANT
"model" - model to display in the world.  defaults to 'models/powerups/clipboard/clipboard.md3' (the clipboard laying flat is 'clipboard2.md3')
"popup" - menu to popup.  no default since you won't want the same clipboard laying around. (clipboard will display a 'put popup here' message)
"notebookpage" - when clipboard is picked up, this page (menu) will be added to your notebook (FIXME: TODO: more info goes here)

We currently use:
clip_interrogation
clip_codeddispatch
clip_alertstatus

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/clipboard/clipboard.md3"
*/
/*
"scriptName"
*/
	{
		"item_clipboard",
		"sound/pickup/armor/body_pickup.wav",
		{   
		"models/powerups/clipboard/clipboard.md3",
		0,
		0 
		},
		"icons/iconh_small",
		"",
		1,
		IT_CLIPBOARD,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{0,0,0,0,0}
	},

/*QUAKED item_treasure (1 1 0) (-8 -8 -8) (8 8 8) SUSPENDED SPIN
Items the player picks up that are just used to tally a score at end-level
"model" defaults to 'models/powerups/treasure/goldbar.md3'
"noise" sound to play on pickup.  defaults to 'sound/pickup/treasure/gold.wav'
"message" what to call the item when it's picked up.  defaults to "Treasure Item" (SA: temp)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/treasure/goldbar.md3"
*/
/*
"scriptName"
*/
	{
		"item_treasure",
		"sound/pickup/treasure/gold.wav",
		{ 
		"models/powerups/treasure/goldbar.md3",
		0,
		0 
		},
		"icons/iconh_small",
		"Treasure Item",
		5,
		IT_TREASURE,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{0,0,0,0,0}
	},


	//
	// ARMOR/HEALTH/STAMINA
	//


/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_s.md3"
*/
	{
		"item_health_small",
		"sound/pickup/health/health_pickup.wav",
		{   
		"models/powerups/health/health_s.md3",
		0,
		0 
		},
		"icons/iconh_small",
		"Small Health",
		5,
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{15,10,5,5,1}
	},

/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_m.md3"
*/
	{
		"item_health",
		"sound/pickup/health/health_pickup.wav",
		{   
		"models/powerups/health/health_m.md3",
		0,
		0 
		},
		"icons/iconh_med",
		"Med Health",
		25,
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{30,25,15,10,3}
	},

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_l.md3"
*/
	{
		"item_health_large",
		"sound/pickup/health/health_pickup.wav",
		{   
		"models/powerups/health/health_l.md3",
		0, 
		0
		},
		"icons/iconh_large",
		"Large Health",
		50,
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{50,40,30,20,5}
	},

/*QUAKED item_health_turkey (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
multi-stage health item.
gives amount on first use based on skill:
skill 1: 50
skill 2: 50
skill 3: 50
skill 4: 40
skill 5: 30

then gives 15 on "finishing up"

player will only eat what he needs.  health at 90, turkey fills up and leaves remains (leaving 15).  health at 5 you eat the whole thing.
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_t1.md3"
*/
	{
		"item_health_turkey",
		"sound/pickup/health/hot_pickup.wav",
		{   
		"models/powerups/health/health_t3.md3",  // just plate (should now be destructable)
		"models/powerups/health/health_t2.md3",  // half eaten
		"models/powerups/health/health_t1.md3"  // whole turkey
		},
		"icons/iconh_turkey",
		"Hot Meal",
		5,                 // amount given in last stage
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{15,15,15,10,2}   // amount given in first stage based on gameskill level
	},

/*QUAKED item_health_breadandmeat (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
multi-stage health item.
gives amount on first use based on skill:
skill 1: 30
skill 2: 30
skill 3: 30
skill 4: 20
skill 5: 10

then gives 10 on "finishing up"
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_b1.md3"
*/
	{
		"item_health_breadandmeat",
		"sound/pickup/health/cold_pickup.wav",
		{   
		"models/powerups/health/health_b3.md3",  // just plate (should now be destructable)
		"models/powerups/health/health_b2.md3",  // half eaten
		"models/powerups/health/health_b1.md3"  // whole turkey
		},
		"icons/iconh_breadandmeat",
		"Cold Meal",
		5,                 // amount given in last stage
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{15,15,15,10,2}   // amount given in first stage based on gameskill level
	},

/*QUAKED item_health_wall_box (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED - - RESPAWN
single use health with dual state model.
please set the suspended flag to keep it from falling on the ground
defaults to 50 pts health
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_wallbox.md3"
*/
	{
		"item_health_wall_box",
		"sound/pickup/health/health_pickup.wav",
		{   
		"models/powerups/health/health_wallbox2.md3",
		"models/powerups/health/health_wallbox1.md3",
		0 
		},
		"icons/iconh_wall",
		"Health",
		25,
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{25,25,25,15,3}
	},

/*QUAKED item_health_wall (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED - - RESPAWN
defaults to 50 pts health
you will probably want to check the 'suspended' box to keep it from falling to the ground
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/health/health_w.md3"
*/
	{
		"item_health_wall",
		"sound/pickup/health/health_pickup.wav",
		{   
		"models/powerups/health/health_w.md3",
		0, 
		0
		},
		"icons/iconh_wall",
		"Health",
		25,
		IT_HEALTH,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{30,20,15,15,3}
	},

	//
	// STAMINA
	//


/*QUAKED item_stamina_stein (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
defaults to 30 sec stamina boost
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/instant/stamina_stein.md3"
*/

	{
		"item_stamina_stein",
		"sound/pickup/health/stamina_pickup.wav",
		{
		"models/powerups/instant/stamina_stein.md3",
		0, 
		0
		},
		"icons/icons_stein",
		"Stamina",
		25,
		IT_POWERUP,
		WP_NONE,
		PW_NOFATIGUE,
		0,
		0,
		"",
		"",
		{30,25,20,15,1}
	},


/*QUAKED item_stamina_brandy (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
defaults to 30 sec stamina boost

multi-stage health item.
gives amount on first use based on skill:
skill 1: 50
skill 2: 50
skill 3: 50
skill 4: 40
skill 5: 30

then gives 15 on "finishing up"

player will only eat what he needs.  health at 90, turkey fills up and leaves remains (leaving 15).  health at 5 you eat the whole thing.
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/instant/stamina_brandy1.md3"
*/

	{
		"item_stamina_brandy",
		"sound/sound/pickup/health/stamina_pickup.wav",
		{   
		"models/powerups/instant/stamina_brandy2.md3",
		"models/powerups/instant/stamina_brandy1.md3",
		0
		},
		"icons/icons_brandy",
		"Stamina",
		25,
		IT_POWERUP,
		WP_NONE,
		PW_NOFATIGUE,
		0,
		0,
		"",
		"",
		{30,25,20,15,1}
	},


	//
	// ARMOR
	//


/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/armor/armor_body1.md3"
*/
	{
		"item_armor_body",
		"sound/pickup/armor/body_pickup.wav",
		{   
		"models/powerups/armor/armor_body1.md3",
		0, 
		0
		},
		"icons/iconr_body",
		"Flak Jacket",
		75,
		IT_ARMOR,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{75,75,75,75,75}
	},

/*QUAKED item_armor_body_hang (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED - - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/armor/armor_body2.md3"
*/
	{
		"item_armor_body_hang",
		"sound/pickup/armor/body_pickup.wav",
		{   
		"models/powerups/armor/armor_body2.md3",
		0, 
		0
		},
		"icons/iconr_bodyh",
		"Flak Jacket",
		75,
		IT_ARMOR,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{75,75,75,75,75}
	},

/*QUAKED item_armor_head (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/armor/armor_head1.md3"
*/
	{
		"item_armor_head",
		"sound/pickup/armor/head_pickup.wav",
		{   
		"models/powerups/armor/armor_head1.md3",
		0,
		0
		},
		"icons/iconr_head",
		"Armored Helmet",
		25,
		IT_ARMOR,
		WP_NONE,
		0,
		0,
		0,
		"",
		"",
		{25,25,25,25,25}
	},



	//
	// WEAPONS
	//

/*
weapon_gauntlet
*/
	{
		"weapon_gauntlet",
		"sound/misc/w_pkup.wav",
		{
		"models/weapons2/gauntlet/gauntlet.md3",
		0, 
		0
		},
		"", 
		"Gauntlet",             
		0,
		IT_WEAPON,
		WP_GAUNTLET,
		WP_GAUNTLET,
		WP_GAUNTLET,
		WP_GAUNTLET,
		"",                      
		"",                      
		{0,0,0,0,0}
	},


/*QUAKED weapon_knife (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/knife/knife.md3"
*/
	{
		"weapon_knife",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_knife_1",   
		"Knife",             
		50,
		IT_WEAPON,
	    WP_KNIFE,
		WP_KNIFE,
		WP_KNIFE,
		WP_KNIFE,
		"",                     
		"",                     
		{0,0,0,0,0}
	},


/*QUAKED weapon_luger (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/luger/luger.md3"
*/
	{
		"weapon_luger",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_luger_1",   
		"Luger",             
		50,
		IT_WEAPON,
		WP_LUGER,
		WP_LUGER,
		WP_LUGER,
		WP_LUGER,
		"",                     
		"",                      
		{0,0,0,0,0}
	},


/*QUAKED weapon_mauserRifle (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/mauser/mauser.md3"
*/
	{
		"weapon_mauserRifle",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_mauser_1", 
		"Mauser Rifle",          
		50,
		IT_WEAPON,
		WP_MAUSER,
		WP_MAUSER,
		WP_MAUSER,
		WP_MAUSER,
		"",                      
		"",                      
		{0,0,0,0,0}
	},

/*QUAKED weapon_thompson (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/thompson/thompson.md3"
*/
	{
		"weapon_thompson",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_thompson_1",  
		"Thompson",              
		30,
		IT_WEAPON,
		WP_THOMPSON,
		WP_THOMPSON,
		WP_COLT,
		WP_THOMPSON,
		"",                 
		"",                  
		{0,0,0,0,0}
	},

/*QUAKED weapon_sten (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/sten/sten.md3"
*/
	{
		"weapon_sten",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},
		"icons/iconw_sten_1",    
		"Sten",                  
		30,
		IT_WEAPON,
		WP_STEN,
		WP_STEN,
		WP_LUGER,
		WP_STEN,
		"",                  
		"",                 
		{0,0,0,0,0}
	},

/*weapon_akimbo
dual colts
*/
	{
		"weapon_akimbo",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_colt_1",    
		"Dual Colts",            
		50,
		IT_WEAPON,
		WP_AKIMBO,
		WP_AKIMBO,
		WP_COLT,
		WP_AKIMBO,
		"",                     
		"",                     
		{0,0,0,0,0}
	},

/*QUAKED weapon_colt (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/colt/colt.md3"
*/
	{
		"weapon_colt",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_colt_1",    
		"Colt",                  
		50,
		IT_WEAPON,
		WP_COLT,
		WP_COLT,
		WP_COLT,
		WP_COLT,
		"",                      
		"",                      
		{0,0,0,0,0}
	},


// (SA) snooper is the parent, so 'garand' is no longer available as a stand-alone weap w/ an optional scope
/*
weapon_garandRifle (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/garand/garand.md3"
*/
	{
		"NOT_weapon_garandRifle",    //----(SA)	modified so it can no longer be given individually
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_garand_1",  
		"garand",                      
		50,
		IT_WEAPON,
		WP_GARAND,
		WP_GARAND,
		WP_GARAND,
		WP_GARAND,
		"",                      
		"",                      
		{0,0,0,0,0}
	},

/*QUAKED weapon_mp40 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models\weapons2\mp40\mp40.md3"
*/
	{
		"weapon_mp40",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_mp40_1",      
		"MP40",              
		30,
		IT_WEAPON,
		WP_MP40,
		WP_MP40,
		WP_LUGER,
		WP_MP40,
		"",                  
		"",                
		{0,0,0,0,0}
	},



/*QUAKED weapon_fg42 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/fg42/fg42.md3"
*/
	{
		"weapon_fg42",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_fg42_1",   
		"FG42 Paratroop Rifle",      
		10,
		IT_WEAPON,
		WP_FG42,
		WP_FG42,
		WP_MAUSER,
		WP_FG42,
		"",                  
		"",                  
		{0,0,0,0,0}
	},



//----(SA)	modified sp5 to be silencer mod for luger
/*QUAKED weapon_silencer (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/sp5/sp5.md3"
*/
	{
		"weapon_silencer",
		"sound/misc/w_pkup.wav",
		{  
		"",
		"",
		""
		},

		"icons/iconw_silencer_1",    
		"sp5 pistol",
		10,
		IT_WEAPON,
		WP_SILENCER,
		WP_SILENCER,
		WP_LUGER,
		WP_LUGER,
		"",                 
		"",                  
		{0,0,0,0,0}
	},

/*QUAKED weapon_panzerfaust (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/panzerfaust/pf.md3"
*/
	{
		"weapon_panzerfaust",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_panzerfaust_1", 
		"Panzerfaust",               
		1,
		IT_WEAPON,
		WP_PANZERFAUST,
		WP_PANZERFAUST,
		WP_PANZERFAUST,
		WP_PANZERFAUST,
		"",                      
		"",                      
		{0,0,0,0,0}
	},


//----(SA)	removed the quaked for this.  we don't actually have a grenade launcher as such.  It's given implicitly
//			by virtue of getting grenade ammo.  So we don't need to have them in maps
/*
weapon_grenadelauncher
*/
	{
		"weapon_grenadelauncher",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_grenade_1", 
		"Grenade",               
		6,
		IT_WEAPON,
		WP_GRENADE_LAUNCHER,
		WP_GRENADE_LAUNCHER,
		WP_GRENADE_LAUNCHER,
		WP_GRENADE_LAUNCHER,
		"",                      
		"sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav",             
		{0,0,0,0,0}
	},

/*
weapon_grenadePineapple
*/
	{
		"weapon_grenadepineapple",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_pineapple_1",  
		"Pineapple",             
		6,
		IT_WEAPON,
		WP_GRENADE_PINEAPPLE,
		WP_GRENADE_PINEAPPLE,
		WP_GRENADE_PINEAPPLE,
		WP_GRENADE_PINEAPPLE,
		"",                      
		"sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav",            
		{0,0,0,0,0}
	},

//weapon_dynamite

	{
		"weapon_dynamite",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_dynamite_1",    
		"Dynamite Weapon",       
		7,
		IT_WEAPON,
		WP_DYNAMITE,
		WP_DYNAMITE,
		WP_DYNAMITE,
		WP_DYNAMITE,
		"",                      
		"",                     
		{0,0,0,0,0}
	},




/*QUAKED weapon_venom (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/venom/pu_venom.md3"
*/
	{
		"weapon_venom",
		"sound/misc/w_pkup.wav",
		{  
		"",
		"",
		""
		},

		"icons/iconw_venom_1",   
		"Venom",             
		700,
		IT_WEAPON,
		WP_VENOM,
		WP_VENOM,
		WP_VENOM,
		WP_VENOM,
		"",                      
		"",                      
		{0,0,0,0,0}
	},



/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/flamethrower/pu_flamethrower.md3"
*/
	{
		"weapon_flamethrower",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_flamethrower_1",    
		"Flamethrower",             
		200,
		IT_WEAPON,
		WP_FLAMETHROWER,
		WP_FLAMETHROWER,
		WP_FLAMETHROWER,
		WP_FLAMETHROWER,
		"",                          
		"",                          
		{0,0,0,0,0}
	},


/*QUAKED weapon_tesla (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/tesla/pu_tesla.md3"
*/
	{
		"weapon_tesla",
		"sound/misc/w_pkup.wav",

		{   
		"",
		"",
		""
		},

		"icons/iconw_tesla_1",   
		"Tesla Gun",             
		200,
		IT_WEAPON,
		WP_TESLA,
		WP_TESLA,
		WP_TESLA,
		WP_TESLA,
		"",                          
		"",                          
		{0,0,0,0,0}
	},



/*QUAKED weapon_sniperScope (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/mauser/pu_mauser_scope.md3"
*/
	{
		"weapon_sniperScope",
		"sound/misc/w_pkup.wav",
		{  
		"",
	    "",
		""
		},

		"icons/iconw_mauser_1",  
		"Sniper Scope",              
		200,
		IT_WEAPON,
		WP_SNIPERRIFLE,
		WP_SNIPERRIFLE,
		WP_MAUSER,
		WP_MAUSER,
		"",                         
		"",                         
		{0,0,0,0,0}
	},

/*QUAKED weapon_snooperrifle (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/garand/garand.md3"
*/
	{
		"weapon_snooperrifle",
		"sound/misc/w_pkup.wav",
		{  
		"",
	    "",
		""
		},

		"icons/iconw_garand_1",  
		"Snooper Rifle",
		20,
		IT_WEAPON,
		WP_SNOOPERSCOPE,
		WP_SNOOPERSCOPE,
		WP_GARAND,
		WP_GARAND,
		"",                          
		"",                          
		{0,0,0,0,0}
	},

/* weapon_fg42scope
*/
	{
		"weapon_fg42scope",  
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_fg42_1",    
		"FG42 Scope",                	
		0,
		IT_WEAPON,
		WP_FG42SCOPE,
		WP_FG42SCOPE,      
		WP_MAUSER,      
		WP_FG42,        
		"",                          
		"",                          
		{0,0,0,0,0}
	},


// Ridah, need this for the scripting
/*
weapon_monster_attack1 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_monster_attack1",
		"",
		{   
		"",
		"",
		0
		},
		"",  
		"MonsterAttack1",            
		100,
		IT_WEAPON,
		WP_MONSTER_ATTACK1,
		WP_MONSTER_ATTACK1,
		WP_MONSTER_ATTACK1,        
		WP_MONSTER_ATTACK1,         
		"",                         
		"",                          
		{0,0,0,0,0}
	},
/*
weapon_monster_attack2 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_monster_attack2",
		"",
		{   
		"",
		"",
		0 
		},
		"", 
		"MonsterAttack2",            
		100,
		IT_WEAPON,
		WP_MONSTER_ATTACK2,
		WP_MONSTER_ATTACK2,
		WP_MONSTER_ATTACK2,        
		WP_MONSTER_ATTACK2,         
		"",                          
		"",                          
		{0,0,0,0,0}
	},
/*
weapon_monster_attack3 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_monster_attack3",
		"",
		{
		"",
		"",
		0
		},
		"",  
		"MonsterAttack3",            
		100,
		IT_WEAPON,
		WP_MONSTER_ATTACK3,
		WP_MONSTER_ATTACK3,
		WP_MONSTER_ATTACK3,        
		WP_MONSTER_ATTACK3,
		"",                          
		"",                          
		{0,0,0,0,0}
	},

/*
weapon_mortar (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_mortar",
		"sound/misc/w_pkup.wav",
		{   
		"models/weapons2/grenade/grenade.md3",
		"models/weapons2/grenade/v_grenade.md3",
		"models/weapons2/grenade/pu_grenade.md3"
		},
		"icons/iconw_grenade_1",
		"nopickup(WP_MORTAR)",      
		6,
		IT_WEAPON,
		WP_MORTAR,
		WP_MORTAR,
		WP_MORTAR,
		WP_MORTAR,
		"",                     
		"sound/weapons/mortar/mortarf1.wav",             
		{0,0,0,0,0}
	},


// RealRTCW weapons

/*QUAKED weapon_mp34 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/mp34/mp34_3rd.md3"
*/
	{
		"weapon_mp34",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_mp34",    
		"MP34",              
		30,
		IT_WEAPON,
		WP_MP34,
		WP_MP34,
		WP_LUGER,
		WP_MP34,
		"",                  
		"",                 
		{0,0,0,0,0}
	},

	/*QUAKED weapon_tt33 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/tt33/tt33.md3"
*/
	{
		"weapon_tt33",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_tt33",    
		"tt33",             
		30,
		IT_WEAPON,
		WP_TT33,
		WP_TT33,
		WP_TT33,
		WP_TT33,
		"",                 
		"",                 
		{0,0,0,0,0}
	},

/*QUAKED weapon_ppsh (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/ppsh/ppsh.md3"
*/
	{
		"weapon_ppsh",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_ppsh_1",   
		"ppsh",              
		30,
		IT_WEAPON,
		WP_PPSH,
		WP_PPSH,
		WP_TT33,
		WP_PPSH,
		"",                  
		"",                 
		{0,0,0,0,0}
	},

/*QUAKED weapon_mosin (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/mosin/mosin.md3"
*/
	{
		"weapon_mosin",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_mosin",    
		"mosin",               
		30,
		IT_WEAPON,
		WP_MOSIN,
		WP_MOSIN,
		WP_MOSIN,
		WP_MOSIN,
		"",                 
		"",                 
		{0,0,0,0,0}
	},

/*QUAKED weapon_g43 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/multiplayer/g43/g43_3rd.md3"
*/
	{
		"weapon_g43",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_g43",    
		"g43",             
		30,
		IT_WEAPON,
		WP_G43,
		WP_G43,
		WP_MAUSER,
		WP_G43,
		"",                  
		"",                  
		{0,0,0,0,0}
	},


/*QUAKED weapon_m1_garand (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/multiplayer/m1_garand/m1_garand_3rd.md3"
*/
	{
		"weapon_m1garand",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_m1garand",    
		"m1garand",              
		30,
		IT_WEAPON,
		WP_M1GARAND,
		WP_M1GARAND,
		WP_BAR,
		WP_M1GARAND,
		"",                 
		"",                  
		{0,0,0,0,0}
	},

	/*QUAKED weapon_m7 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/mauser/mauser.md3"
*/
	{
		"weapon_m7",
		"sound/misc/w_pkup.wav",
		{
		"",
		"",
		""
		},

		"icons/iconw_m1_garand_1", 
		"m7", 
		200,
		IT_WEAPON,
		WP_M7,
		WP_M7,
		WP_M7,
		WP_M7,
		"",                          // precache
		"",                          // sounds
//		{0,0,0,0,0}
	},

/*QUAKED weapon_bar (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/bar/bar3rd.md3"
*/
	{
		"weapon_bar",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_bar",    
		"BAR",              
		30,
		IT_WEAPON,
		WP_BAR,
		WP_BAR,
		WP_BAR,
		WP_BAR,
		"",                  
		"",                  
		{0,0,0,0,0}
	},

/*QUAKED weapon_mp44 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/mp44/mp44.md3"
*/
	{
		"weapon_mp44",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_mp44",    
		"MP44",             
		30,
		IT_WEAPON,
		WP_MP44,
		WP_MP44,
		WP_MP44,
		WP_MP44,
		"",                 
		"",                
		{0,0,0,0,0}
	},

/*QUAKED weapon_mg42m (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/multiplayer/mg42/mg42_3rd.md3"
*/
	{
		"weapon_mg42m",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_mg42m",   
		"mg42m",             
		700,
		IT_WEAPON,
		WP_MG42M,
		WP_MG42M,
		WP_VENOM,
		WP_MG42M,
		"",                      
		"",                      
		{0,0,0,0,0}
	},

/*QUAKED weapon_browning (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/multiplayer/mg42/mg42_3rd.md3"
*/
	{
		"weapon_browning",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_browning",   
		"browning",             
		700,
		IT_WEAPON,
		WP_BROWNING,
		WP_BROWNING,
		WP_BAR,
		WP_BROWNING,
		"",                      
		"",                      
		{0,0,0,0,0}
	},

/*QUAKED weapon_m97 (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/m97/m97_3rd.md3"
*/
	{
		"weapon_m97",
		"sound/misc/w_pkup.wav",
		{ 
		"",
		"",
		""
		},

			"icons/iconw_m97",  
			"m97",            
			700,
			IT_WEAPON,
			WP_M97,
			WP_M97,
			WP_M97,
			WP_M97,
			"",                      
			"",                     
			{ 0,0,0,0,0 }
	},


	/*QUAKED weapon_revolver (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
"stand" values:
	no value:	laying in a default position on it's side (default)
	2:			upright, barrel pointing up, slightly angled (rack mount)
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/weapons2/p38/luger.md3"
*/
	{
		"weapon_revolver",
		"sound/misc/w_pkup.wav",
		{   
		"",
		"",
		""
		},

		"icons/iconw_revolver",    
		"revolver",              
		30,
		IT_WEAPON,
		WP_REVOLVER,
		WP_REVOLVER,
		WP_REVOLVER,
		WP_REVOLVER,
		"",                 
		"",                  
		{0,0,0,0,0}
	},

	{
		"weapon_grenadesmoke",
		"sound/misc/w_pkup.wav",
		{  
		"",
		"",
		""
		},

		"icons/iconw_smokegrenade_1",    
		"smokeGrenade",              
		50,
		IT_WEAPON,
		WP_AIRSTRIKE,
		WP_AIRSTRIKE,
		WP_AIRSTRIKE,
		WP_AIRSTRIKE,
		"",                      
		"sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav",            
		{0,0,0,0,0}
	},

		{
		"weapon_poisongas",
		"sound/misc/w_pkup.wav",
		{
		"",
		"",
		""
		},
		"icons/iconw_poisongrenade_1",    
		"Poison Gas",
		0,
		IT_WEAPON,
		WP_POISONGAS,		
		WP_POISONGAS,
		WP_POISONGAS,
		WP_POISONGAS,
		"",                      
		"",                      
		{0,0,0,0,0}
	},


	//
	// AMMO ITEMS
	//

// RealRTCW ammo

	{
		"ammo_poison_gas",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/amgren_bag.md3",
		0, 
		0 
		},
		"",
		"Poison Gas",
		1,
		IT_AMMO,
		WP_NONE,
		WP_POISONGAS,
		WP_POISONGAS,
		WP_POISONGAS,
		"",                  
		"", 
		{5,4,3,2,2}                
	},

/*QUAKED ammo_m7 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/m7ammo_bag.md3"
*/
	{
		"ammo_m7",
		"sound/misc/w_pkup.wav",
		{ 
		"models/powerups/ammo/m7ammo_bag.md3",
		0,
		0
		},

		"icons/iconw_m1_garand_1",      
		"m7_ammo",               
		200,
		IT_AMMO,
		WP_NONE,
		WP_M7,
		WP_M7,
		WP_M7,
		"",                        
		"",                         
		{5,4,3,2,2}
	},

/*QUAKED ammo_ttammo (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: TT33, PPSH

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/ttammo.md3"
*/
	{
		"ammo_ttammo",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/ttammo.md3",
		0,
		0 
		},

		"icons/iconw_luger_1", 
		"ttammo",           		
		60,
		IT_AMMO,
		WP_NONE,
		WP_PPSH,
		WP_TT33,
		WP_PPSH,
		"",                  
		"",                  
		{71,71,50,50,50}	
	},

/*QUAKED ammo_ttammo_l (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: TT33, PPSH

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/ttammo.md3"
*/
	{
		"ammo_ttammo_l",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/ttammo_l.md3",
		0, 
		0
		},

		"icons/iconw_luger_1",
		"ttammol",         		
		60,
		IT_AMMO,
		WP_NONE,
		WP_PPSH,
		WP_TT33,
		WP_PPSH,
		"",                  
		"",                  
		{142,142,100,100,100}	
	},

/*QUAKED ammo_mosina (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: mosin nagant

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/mosina.md3"
*/
	{
		"ammo_mosina",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/mosina.md3",
		0, 
		0 
		},

		"icons/icona_machinegun",   
		"mosina",			       
		50,
		IT_AMMO,
		WP_NONE,
		WP_MOSIN,
		WP_MOSIN,
		WP_MOSIN,
		"",                          
		"",                          
		{20,20,15,15,15}		
	},

/*QUAKED ammo_barammo (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Bar, M1 Garand

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/barammo.md3"
*/
{
		"ammo_barammo",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/barammo.md3",
		0, 
		0
		},

		"icons/iconw_luger_1", 
		"barammo",           
		60,
		IT_AMMO,
		WP_NONE,
		WP_BAR,
		WP_M1GARAND,
		WP_BAR,
		"",                 
		"",                  
		{40,40,30,30,30}	
	},

/*QUAKED ammo_barammo_l (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Bar, M1 Garand

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/barammo_l.md3"
*/
{
		"ammo_barammo_l",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/barammo_l.md3",
		0,
		0
		},

		"icons/iconw_luger_1", 
		"barammol",           
		60,
		IT_AMMO,
		WP_NONE,
		WP_BAR,
		WP_M1GARAND,
		WP_BAR,
		"",                 
		"",                  
		{60,60,45,45,45}	
	},

/*QUAKED ammo_44ammo (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: MP44

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/44ammo.md3"
*/
{
		"ammo_44ammo",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/44ammo.md3",
		0,
		0
		},

		"icons/iconw_luger_1",
		"44ammo",           		
		60,
		IT_AMMO,
		WP_NONE,
		WP_MP44,
		WP_MP44,
		WP_MP44,
		"",                 
		"",                  
		{60,60,45,45,45}	
	},

/*QUAKED ammo_44ammo_l (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: MP44

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/44ammo_l.md3"
*/
{
		"ammo_44ammo_l",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/44ammo_l.md3",
		0,
		0
		},

		"icons/iconw_luger_1", 
		"44ammol",         		
		60,
		IT_AMMO,
		WP_NONE,
		WP_MP44,
		WP_MP44,
		WP_MP44,
		"",                  
		"",                 
		{90,90,75,75,75}	
	},

		/*QUAKED ammo_m97ammo (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
		used by: M97

		-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
		model="models/powerups/ammo/m97ammo.md3"
		*/
	{
		"ammo_m97ammo",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/m97ammo.md3",
		0, 
		0
		},

		"icons/iconw_luger_1", 
		"m97ammo",          		
		10,
		IT_AMMO,
		WP_NONE,
		WP_M97,
		WP_M97,
		WP_M97,
		"",                  
		"",                 
		{ 10,10,10,10,10 }
	},

			/*QUAKED ammo_revolver (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
		used by: revolver

		-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
		model="models/powerups/ammo/revolverammo.md3"
		*/
	{
		"ammo_revolver",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/revolverammo.md3",
		0, 
		0 
		},

		"icons/iconw_luger_1",
		"revolverammo",          	
		12,
		IT_AMMO,
		WP_NONE,
		WP_REVOLVER,
		WP_REVOLVER,
		WP_REVOLVER,
		"",                 
		"",                  
		{ 6,6,6,6,6 }
	},





/*QUAKED ammo_9mm_small (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Luger pistol, MP40 machinegun

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am9mm_s.md3"
*/
	{
		"ammo_9mm_small",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am9mm_s.md3",
		0, 
		0 
		},

		"icons/iconw_luger_1", 
		"9mm Rounds",        
		30,
		IT_AMMO,
		WP_NONE,
		WP_LUGER,
		WP_LUGER,
		WP_LUGER,
		"",                  
		"",                  
		{32,24,16,16,16}
	},
/*QUAKED ammo_9mm (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Luger pistol, MP40 machinegun

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am9mm_m.md3"
*/
	{
		"ammo_9mm",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am9mm_m.md3",
		0, 
		0
		},

		"icons/iconw_luger_1", 
		"9mm",          
		60,
		IT_AMMO,
		WP_NONE,
		WP_LUGER,
		WP_LUGER,
		WP_LUGER,
		"",                  
		"",                  
		{64,48,32,16,16}
	},

/*QUAKED ammo_9mm_large (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Luger pistol, MP40 machinegun

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am9mm_l.md3"
*/
	{
		"ammo_9mm_large",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am9mm_l.md3",
		0, 
		0 
		},

		"icons/iconw_luger_1", 
		"9mm Box",           
		100,
		IT_AMMO,
		WP_NONE,
		WP_LUGER,
		WP_LUGER,
		WP_LUGER,
		"",                
		"",                
		{96,64,48,48,48}
	},


/*QUAKED ammo_45cal_small (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Thompson, Colt

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am45cal_s.md3"
*/
	{
		"ammo_45cal_small",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am45cal_s.md3",
		0,
		0
		},

		"icons/iconw_luger_1", 
		".45cal Rounds", 
		20,
		IT_AMMO,
		WP_NONE,
		WP_COLT,
		WP_COLT,
		WP_COLT,
		"",                
		"",                  
		{40,30,20,20,20}
	},
/*QUAKED ammo_45cal (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Thompson, Colt

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am45cal_m.md3"
*/
	{
		"ammo_45cal",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am45cal_m.md3",
		0,
		0
		},

		"icons/iconw_luger_1", 
		".45cal",
		60,
		IT_AMMO,
		WP_NONE,
		WP_COLT,
		WP_COLT,
		WP_COLT,
		"",                
		"",                
		{60,45,30,30,30}
	},
/*QUAKED ammo_45cal_large (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Thompson, Colt

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am45cal_l.md3"
*/
	{
		"ammo_45cal_large",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am45cal_l.md3",
		0,
		0
		},
		"icons/iconw_luger_1", 
		".45cal Box",        
		100,
		IT_AMMO,
		WP_NONE,
		WP_COLT,
		WP_COLT,
		WP_COLT,
		"",                  
		"",                  
		{90,60,45,45,45}
	},




/*QUAKED ammo_792mm_small (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Mauser rifle, FG42

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am792mm_s.md3"
*/
	{
		"ammo_792mm_small",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am792mm_s.md3",
		0, 
		0
		},

		"icons/icona_machinegun",   
		"7.92mm Rounds",        
		50,
		IT_AMMO,
		WP_NONE,
		WP_MAUSER,
		WP_MAUSER,
		WP_MAUSER,
		"",                          
		"",                         
		{20,15,10,5,5}
	},

/*QUAKED ammo_792mm (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Mauser rifle, FG42

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am792mm_m.md3"
*/
	{
		"ammo_792mm",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am792mm_m.md3",
		0, 
		0
		},
		"icons/icona_machinegun",    
		"7.92mm",                
		10,
		IT_AMMO,
		WP_NONE,
		WP_MAUSER,
		WP_MAUSER,
		WP_MAUSER,
		"",                         
		"",                          
		{40,20,15,10,10}
	},

/*QUAKED ammo_792mm_large (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Mauser rifle, FG42

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am792mm_l.md3"
*/
	{
		"ammo_792mm_large",
		"sound/misc/am_pkup.wav",
		{
		"models/powerups/ammo/am792mm_l.md3",
		0, 
	    0
		},

		"icons/icona_machinegun",   
		"7.92mm Box",                
		50,
		IT_AMMO,
		WP_NONE,
		WP_MAUSER,
		WP_MAUSER,
		WP_MAUSER,
		"",                         
		"",                          
		{60,40,30,20,20}
	},

/*QUAKED ammo_30cal_small (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Garand rifle

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am30cal_s.md3"
*/
	{
		"ammo_30cal_small",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am30cal_s.md3",
		0, 
		0 
		},

		"icons/icona_machinegun",   
		".30cal Rounds",        
		50,
		IT_AMMO,
		WP_NONE,
		WP_GARAND,
		WP_GARAND,
		WP_GARAND,
		"",                         
		"",                          
		{5,2,2,2,2}
	},

/*QUAKED ammo_30cal (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Garand rifle

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am30cal_m.md3"
*/
	{
		"ammo_30cal",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am30cal_m.md3",
		0, 
		0
		},

		"icons/icona_machinegun",    
		".30cal",               
		50,
		IT_AMMO,
		WP_NONE,
		WP_GARAND,
		WP_GARAND,
		WP_GARAND,
		"",                        
		"",                          
		{5,5,5,5,5}
	},

/*QUAKED ammo_30cal_large (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Garand rifle

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am30cal_l.md3"
*/
	{
		"ammo_30cal_large",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am30cal_l.md3",
		0, 
		0
		},

		"icons/icona_machinegun",    
		".30cal Box",                
		50,
		IT_AMMO,
		WP_NONE,
		WP_GARAND,
		WP_GARAND,
		WP_GARAND,
		"",                          
		"",                          
		{10,10,10,10,10}
	},

/*QUAKED ammo_127mm (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Venom gun

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/am127mm.md3"
*/
	{
		"ammo_127mm",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/am127mm.md3",
		0, 
		0 
		},

		"icons/icona_machinegun",    
		"12.7mm",                    
		100,
		IT_AMMO,
		WP_NONE,
		WP_VENOM,
		WP_VENOM,
		WP_VENOM,
		"",                         
		"",                        
		{100,100,100,100,100}
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/amgren_bag.md3"
*/
	{
		"ammo_grenades",
		"sound/misc/am_pkup.wav",
		{
		"models/powerups/ammo/amgren_bag.md3",
		0, 
		0
		},

		"icons/icona_grenade",   
		"Grenades",             
		5,
		IT_AMMO,
		WP_NONE,
		WP_GRENADE_LAUNCHER,
		WP_GRENADE_LAUNCHER,
		WP_GRENADE_LAUNCHER,
		"",                     
		"",                    
		{4,3,2,2,2}
	},

/*QUAKED ammo_grenades_american (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/amgrenus_bag.md3"
*/
	{
		"ammo_grenades_american",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/amgrenus_bag.md3",
		0, 
		0
		},

		"icons/icona_pineapple", 
		"Pineapples",           
		5,
		IT_AMMO,
		WP_NONE,
		WP_GRENADE_PINEAPPLE,
		WP_GRENADE_PINEAPPLE,
		WP_GRENADE_PINEAPPLE,
		"",                      
		"",                    
		{4,3,2,2,2}
	},

/*QUAKED ammo_dynamite (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN

 -------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/dynamite.md3"
*/
	{
		"ammo_dynamite",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/dynamite.md3",
		0, 
		0
		},

		"icons/icona_dynamite",  
		"Dynamite",            
		1,
		IT_AMMO,
		WP_NONE,
		WP_DYNAMITE,
		WP_DYNAMITE,
		WP_DYNAMITE,
		"",                    
		"",                      
		{1,1,1,1,1}
	},


/*QUAKED ammo_cell (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Tesla

Boosts recharge on Tesla
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/amcell.md3"
*/
	{
		"ammo_cell",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/amcell.md3",
		0, 
		0
		},

		"icons/icona_cell",  
		"Cell",              
		500,
		IT_AMMO,
		WP_NONE,
		WP_TESLA,
		WP_TESLA,
		WP_TESLA,
		"",                  
		"",                  
		{75,50,30,25,25}
	},



/*QUAKED ammo_fuel (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: Flamethrower

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/amfuel.md3"
*/
	{
		"ammo_fuel",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/amfuel.md3",
		0, 
		0
		},

		"icons/icona_fuel",  
		"Fuel",            
		100,
		IT_AMMO,
		WP_NONE,
		WP_FLAMETHROWER,
		WP_FLAMETHROWER,
		WP_FLAMETHROWER,
		"",                 
		"",                  
		{100,75,50,50,50}
	},


/*QUAKED ammo_panzerfaust (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN
used by: German Panzerfaust

-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/ammo/ampf.md3"
*/
	{
		"ammo_panzerfaust",
		"sound/misc/am_pkup.wav",
		{ 
		"models/powerups/ammo/ampf.md3",
		0, 
		0
		},

		"icons/icona_panzerfaust",   
		"Panzerfaust Rockets",               
		5,
		IT_AMMO,
		WP_NONE,
		WP_PANZERFAUST,
		WP_PANZERFAUST,
		WP_PANZERFAUST,
		"",                     
		"",                     
		{4,3,2,2,2}
	},


//----(SA)	hopefully it doesn't need to be a quaked thing.
//			apologies if it does and I'll put it back.
/*
ammo_monster_attack1 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
used by: Monster Attack 1 (specific to each monster)
*/
	{
		"ammo_monster_attack1",
		"",
		{ 
		"",
		0, 
		0
		},

		"",                      
		"MonsterAttack1",       
		60,
		IT_AMMO,
		WP_NONE,
		WP_MONSTER_ATTACK1,
		WP_MONSTER_ATTACK1,
		WP_MONSTER_ATTACK1,
		"",
		"",
		{0,0,0,0,0}
	},

/*QUAKED holdable_wine (.3 .3 1) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN

pickup sound : "sound/pickup/holdable/get_wine.wav"
use sound : "sound/pickup/holdable/use_wine.wav"
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/holdable/wine.md3"
*/
	{
		"holdable_wine",
		"sound/pickup/holdable/get_wine.wav",
		{
		"models/powerups/holdable/wine.md3",
		0, 
		0
		},

		"icons/wine",                    
		"1921 Chateau Lafite",           
		1,
		IT_HOLDABLE,
		WP_NONE,
		HI_WINE,
		0,
		0,
		"",                             
		"sound/pickup/holdable/use_wine.wav",       
		{3,0,0,0,0}
	},


/*QUAKED holdable_adrenaline(.3 .3 1) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN
Protection from fatigue
Using the "sprint" key will not fatigue the character

pickup sound : "sound/pickup/holdable/get_adrenaline.wav"
use sound : "sound/pickup/holdable/use_adrenaline.wav"
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/holdable/adrenaline.md3"
*/
	{
		"holdable_adrenaline",
		"sound/pickup/holdable/get_adrenaline.wav",
		{
		"models/powerups/holdable/adrenaline.md3",
		0, 
		0
		},

		"icons/adrenaline",            
		"Adrenaline used",             
		1,
		IT_HOLDABLE,
		WP_NONE,
		HI_ADRENALINE,
		0,
		0,
		"",                              
		"sound/pickup/holdable/use_adrenaline.wav", 
		{1,1,1,1,1}
	},


/*QUAKED holdable_bandages(.3 .3 1) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN
Protection from fatigue
Using the "sprint" key will not fatigue the character

pickup sound : "sound/pickup/holdable/get_bandages.wav"
use sound : "sound/pickup/holdable/use_bandages.wav"
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/holdable/bandages.md3"
*/
	{
		"holdable_bandages",
		"sound/pickup/holdable/get_bandages.wav",
		{
		"models/powerups/holdable/bandages.md3",
		0, 
		0
		},

		"icons/bandages",             
		"Bandages used",             
		1,
		IT_HOLDABLE,
		WP_NONE,
		HI_BANDAGES,
		0,
		0,
		"",                             
		"sound/pickup/holdable/use_bandages.wav",
		{1,1,1,1,1}
	},



/*QUAKED holdable_book1(.3 .3 1) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/holdable/venom_book.md3"
*/
	{
		"holdable_book1",
		"sound/pickup/holdable/get_book1.wav",
		{
		"models/powerups/holdable/venom_book.md3",
		0, 
		0
		},

		"icons/icon_vbook",              
		"Venom Tech Manual",     
		1,
		IT_HOLDABLE,
		WP_NONE,
		HI_BOOK1,
		0,
		0,
		"",                             
		"sound/pickup/holdable/use_book.wav",    
		{0,0,0,0,0}
	},


/*QUAKED holdable_book2(.3 .3 1) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/holdable/paranormal_book.md3"
*/
	{
		"holdable_book2",
		"sound/pickup/holdable/get_book2.wav",
		{
		"models/powerups/holdable/paranormal_book.md3",
		0, 
		0
		},

		"icons/icon_pbook",            
		"Project Book",                  
		1,
		IT_HOLDABLE,
		WP_NONE,
		HI_BOOK2,
		0,
		0,
		"",                             
		"sound/pickup/holdable/use_book.wav",  
		{0,0,0,0,0}
	},


/*QUAKED holdable_book3(.3 .3 1) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/holdable/zemphr_book.md3"
*/
	{
		"holdable_book3",
		"sound/pickup/holdable/get_book3.wav",
		{
		"models/powerups/holdable/zemphr_book.md3",
		0, 
		0
		},

		"icons/icon_zbook",              
		"Dr. Zemph's Journal",      
		1,
		IT_HOLDABLE,
		WP_NONE,
		HI_BOOK3,
		0,
		0,
		"",                            
		"sound/pickup/holdable/use_book.wav",    
		{0,0,0,0,0}
	},

	//
	// POWERUP ITEMS
	//


/*QUAKED key_binocs (1 1 0) (-8 -8 -8) (8 8 8) SUSPENDED SPIN - RESPAWN
Binoculars.

pickup sound : "sound/pickup/keys/binocs.wav"
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models/powerups/keys/binoculars.md3"
*/
	{
		"key_binocs",
		"sound/pickup/keys/binocs.wav",
		{
		"models/powerups/keys/binoculars.md3",
		0, 
		0
		},

		"icons/binocs",          
		"Binoculars",           
		0,
		IT_KEY,
		WP_NONE,
		INV_BINOCS,
		0,
		0,
		"",                      
		"models/keys/key.wav",
		{0,0,0,0,0}
	},

	// end of list marker
	{NULL}
};
// END JOSEPH

int	bg_numItems = ARRAY_LEN( bg_itemlist ) - 1;


/*
==============
BG_FindItemForPowerup
==============
*/
gitem_t *BG_FindItemForPowerup( powerup_t pw ) {
	int i;

	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( ( bg_itemlist[i].giType == IT_POWERUP ||
			   bg_itemlist[i].giType == IT_TEAM ) &&
			 bg_itemlist[i].giTag == pw ) {
			return &bg_itemlist[i];
		}
	}

	return NULL;
}


/*
==============
BG_FindItemForHoldable
==============
*/
gitem_t *BG_FindItemForHoldable( holdable_t pw ) {
	int i;

	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == pw ) {
			return &bg_itemlist[i];
		}
	}

//	Com_Error( ERR_DROP, "HoldableItem not found" );

	return NULL;
}


/*
===============
BG_FindItemForWeapon

===============
*/
gitem_t *BG_FindItemForWeapon( weapon_t weapon ) {
	gitem_t *it;
	int i;
	const int NUM_TABLE_ELEMENTS = WP_NUM_WEAPONS;
	static gitem_t  *lookupTable[WP_NUM_WEAPONS];
	static qboolean lookupTableInit = qtrue;

	if ( lookupTableInit ) {
		for ( i = 0; i < NUM_TABLE_ELEMENTS; i++ ) {
			lookupTable[i] = 0; // default value for no match found
			for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
				if ( it->giType == IT_WEAPON && it->giTag == i ) {
					lookupTable[i] = it;
				}
			}
		}
		// table is created
		lookupTableInit = qfalse;
	}

	if ( weapon > NUM_TABLE_ELEMENTS ) {
		Com_Error( ERR_DROP, "BG_FindItemForWeapon: weapon out of range %i", weapon );
	}

	if ( !lookupTable[weapon] ) {
		Com_Error( ERR_DROP, "Couldn't find item for weapon %i", weapon );
	}

	// get the weapon from the lookup table
	return lookupTable[weapon];
}

//----(SA) added

#define DEATHMATCH_SHARED_AMMO 0


/*
==============
BG_FindClipForWeapon
==============
*/
weapon_t BG_FindClipForWeapon( weapon_t weapon ) {
	gitem_t *it;
	int i;
	const int NUM_TABLE_ELEMENTS = WP_NUM_WEAPONS;
	static weapon_t lookupTable[WP_NUM_WEAPONS];
	static qboolean lookupTableInit = qtrue;

	if ( lookupTableInit ) {
		for ( i = 0; i < NUM_TABLE_ELEMENTS; i++ ) {
			lookupTable[i] = 0; // default value for no match found
			for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
				if ( it->giType == IT_WEAPON && it->giTag == i ) {
					lookupTable[i] = it->giClipIndex;
				}
			}
		}
		// table is created
		lookupTableInit = qfalse;
	}

	if ( weapon > NUM_TABLE_ELEMENTS ) {
		Com_Error( ERR_DROP, "BG_FindClipForWeapon: weapon out of range %i", weapon );
	}

	// get the weapon from the lookup table
	return lookupTable[weapon];
}



/*
==============
BG_FindAmmoForWeapon
==============
*/
weapon_t BG_FindAmmoForWeapon( weapon_t weapon ) {
	gitem_t *it;
	int i;
	const int NUM_TABLE_ELEMENTS = WP_NUM_WEAPONS;
	static weapon_t lookupTable[WP_NUM_WEAPONS];
	static qboolean lookupTableInit = qtrue;

	if ( lookupTableInit ) {
		for ( i = 0; i < NUM_TABLE_ELEMENTS; i++ ) {
			lookupTable[i] = 0; // default value for no match found
			for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
				if ( it->giType == IT_WEAPON && it->giTag == i ) {
					lookupTable[i] = it->giAmmoIndex;
				}
			}
		}
		// table is created
		lookupTableInit = qfalse;
	}

	if ( weapon > NUM_TABLE_ELEMENTS ) {
		Com_Error( ERR_DROP, "BG_FindAmmoForWeapon: weapon out of range %i", weapon );
	}

	// get the weapon from the lookup table
	return lookupTable[weapon];
}

/*
==============
BG_AkimboFireSequence
	returns 'true' if it's the left hand's turn to fire, 'false' if it's the right hand's turn
==============
*/
//qboolean BG_AkimboFireSequence( playerState_t *ps ) {
qboolean BG_AkimboFireSequence( int weapon, int akimboClip, int coltClip ) {
	// NOTE: this doesn't work when clips are turned off (dmflags 64)

	if ( weapon != WP_AKIMBO ) {
		return qfalse;
	}

	if ( !akimboClip ) {
		return qfalse;
	}

	// no ammo in colt, must be akimbo turn
	if ( !coltClip ) {
		return qtrue;
	}

	// at this point, both have ammo

	// now check 'cycle'   // (removed old method 11/5/2001)
	if ( ( akimboClip + coltClip ) & 1 ) {
		return qfalse;
	}

	return qtrue;
}

//----(SA) end

//----(SA) Added keys
/*
==============
BG_FindItemForKey
==============
*/
gitem_t *BG_FindItemForKey( wkey_t k, int *indexreturn ) {
	int i;

	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( bg_itemlist[i].giType == IT_KEY && bg_itemlist[i].giTag == k ) {
			{
				if ( indexreturn ) {
					*indexreturn = i;
				}
				return &bg_itemlist[i];
			}
		}
	}

	Com_Error( ERR_DROP, "Key %d not found", k );
	return NULL;
}
//----(SA) end


//----(SA) added
/*
==============
BG_FindItemForAmmo
==============
*/
gitem_t *BG_FindItemForAmmo( int ammo ) {
	int i = 0;

	for (; i < bg_numItems; i++ )
	{
		if ( bg_itemlist[i].giType == IT_AMMO && bg_itemlist[i].giAmmoIndex == ammo ) {
			return &bg_itemlist[i];
		}
	}
	Com_Error( ERR_DROP, "Item not found for ammo: %d", ammo );
	return NULL;
}
//----(SA) end


/*
===============
BG_FindItem

===============
*/
gitem_t *BG_FindItem( const char *pickupName ) {
	gitem_t *it;

	for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
		if ( !Q_stricmp( it->pickup_name, pickupName ) ) {
			return it;
		}
	}

	return NULL;
}

/*
==============
BG_FindItem2
	also check classname
==============
*/
gitem_t *BG_FindItem2( const char *name ) {
	gitem_t *it;
	char *name2;

	name2 = (char*)name;

	for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
		if ( !Q_stricmp( it->pickup_name, name ) ) {
			return it;
		}

		if ( !Q_strcasecmp( it->classname, name2 ) ) {
			return it;
		}
	}

	Com_Printf( "BG_FindItem2(): unable to locate item '%s'\n", name );

	return NULL;
}

//----(SA)	added
/*
==============
BG_PlayerSeesItem
	Try to quickly determine if an item should be highlighted as per the current cg_drawCrosshairPickups.integer value.
	pvs check should have already been done by the time we get in here, so we shouldn't have to check
==============
*/

//----(SA)	not used
/*
qboolean BG_PlayerSeesItem(playerState_t *ps, entityState_t *item, int atTime)
{
   vec3_t	vorigin, eorigin, viewa, dir;
   float	dot, dist, foo;

   BG_EvaluateTrajectory( &item->pos, atTime, eorigin );

   VectorCopy(ps->origin, vorigin);
   vorigin[2] += ps->viewheight;			// get the view loc up to the viewheight
   eorigin[2] += 16;						// and subtract the item's offset (that is used to place it on the ground)
   VectorSubtract(vorigin, eorigin, dir);

   dist = VectorNormalize(dir);			// dir is now the direction from the item to the player

   if(dist > 255)
	   return qfalse;						// only run the remaining stuff on items that are close enough

   // (SA) FIXME: do this without AngleVectors.
   //		It'd be nice if the angle vectors for the player
   //		have already been figured at this point and I can
   //		just pick them up.  (if anybody is storing this somewhere,
   //		for the current frame please let me know so I don't
   //		have to do redundant calcs)
   AngleVectors(ps->viewangles, viewa, 0, 0);
   dot = DotProduct(viewa, dir );

   // give more range based on distance (the hit area is wider when closer)

   foo = -0.94f - (dist/255.0f) * 0.057f;	// (ranging from -0.94 to -0.997) (it happened to be a pretty good range)

//	Com_Printf("test: if(%f > %f) return qfalse (dot > foo)\n", dot, foo);
   if(dot > foo)
	   return qfalse;

   return qtrue;
}
*/
//----(SA)	end


/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/

extern int trap_Cvar_VariableIntegerValue( const char *var_name );

qboolean    BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) {
	vec3_t origin;

	BG_EvaluateTrajectory( &item->pos, atTime, origin );

	// we are ignoring ducked differences here
	if ( ps->origin[0] - origin[0] > 44
		 || ps->origin[0] - origin[0] < -50
		 || ps->origin[1] - origin[1] > 36
		 || ps->origin[1] - origin[1] < -36
		 || ps->origin[2] - origin[2] > 36
		 || ps->origin[2] - origin[2] < -36 ) {
		return qfalse;
	}

	return qtrue;
}



#define AMMOFORWEAP BG_FindAmmoForWeapon( item->giTag )
/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/

qboolean isClipOnly( int weap ) {
	switch ( weap ) {
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
	case WP_DYNAMITE:
	case WP_TESLA:
	case WP_FLAMETHROWER:
		return qtrue;
	}
	return qfalse;
}


qboolean    BG_CanItemBeGrabbed( const entityState_t *ent, const playerState_t *ps ) {
	gitem_t *item;
	int ammoweap;
	qboolean multiplayer = qfalse;

	if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) {
		Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: index out of range" );
	}

//----(SA)	check for mp
#ifdef GAMEDLL
	if ( g_gametype.integer == GT_WOLF )
#endif
#ifdef CGAMEDLL
	if ( cg_gameType.integer == GT_WOLF )
#endif
	multiplayer = qtrue;
//----(SA)	end

	item = &bg_itemlist[ent->modelindex];

	switch ( item->giType ) {

	case IT_WEAPON:
		// JPW NERVE -- medics & engineers can only pick up same weapon type
		if ( multiplayer ) {
			if ( ( ps->stats[STAT_PLAYER_CLASS] == PC_MEDIC ) || ( ps->stats[STAT_PLAYER_CLASS] == PC_ENGINEER ) ) {
				if ( !COM_BitCheck( ps->weapons, item->giTag ) ) {
					return qfalse;
				}
			}
		} else {
			if ( COM_BitCheck( ps->weapons, item->giTag ) ) {               // you have the weap
				if ( isClipOnly( item->giTag ) ) {
					if ( ps->ammoclip[item->giAmmoIndex] >= ammoTable[item->giAmmoIndex].maxclip ) {
						return qfalse;
					}
				} else {
					if ( ps->ammo[item->giAmmoIndex] >= ammoTable[item->giAmmoIndex].maxammo ) { // you are loaded with the ammo
						return qfalse;
					}
				}
			}
		}
		// JPW
		return qtrue;

	case IT_AMMO:
		ammoweap = BG_FindAmmoForWeapon( item->giTag );

		if ( isClipOnly( ammoweap ) ) {
			if ( ps->ammoclip[ammoweap] >= ammoTable[ammoweap].maxclip ) {
				return qfalse;
			}
		}

		if ( ps->ammo[ammoweap] >= ammoTable[ammoweap].maxammo ) {
			return qfalse;
		}

		return qtrue;

	case IT_ARMOR:
		// we also clamp armor to the maxhealth for handicapping
//			if ( ps->stats[STAT_ARMOR] >= ps->stats[STAT_MAX_HEALTH] * 2 ) {
		if ( ps->stats[STAT_ARMOR] >= 100 ) {
			return qfalse;
		}
		return qtrue;

	case IT_HEALTH:
		if ( ent->density == ( 1 << 9 ) ) { // density tracks how many uses left
			return qfalse;
		}

		if ( ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH] ) {
			return qfalse;
		}
		return qtrue;

	case IT_POWERUP:
		if ( ent->density == ( 1 << 9 ) ) { // density tracks how many uses left
			return qfalse;
		}

		if ( ps->powerups[PW_NOFATIGUE] == 60000 ) { // full
			return qfalse;
		}

		return qtrue;

	case IT_TEAM:     // team items, such as flags

		// DHM - Nerve :: otherEntity2 is now used instead of modelindex2
		// ent->modelindex2 is non-zero on items if they are dropped
		// we need to know this because we can pick up our dropped flag (and return it)
		// but we can't pick up our flag at base
		if ( ps->persistant[PERS_TEAM] == TEAM_RED ) {
			if ( item->giTag == PW_BLUEFLAG ||
				 ( item->giTag == PW_REDFLAG && ent->otherEntityNum2 /*ent->modelindex2*/ ) ||
				 ( item->giTag == PW_REDFLAG && ps->powerups[PW_BLUEFLAG] ) ) {
				return qtrue;
			}
		} else if ( ps->persistant[PERS_TEAM] == TEAM_BLUE ) {
			if ( item->giTag == PW_REDFLAG ||
				 ( item->giTag == PW_BLUEFLAG && ent->otherEntityNum2 /*ent->modelindex2*/ ) ||
				 ( item->giTag == PW_BLUEFLAG && ps->powerups[PW_REDFLAG] ) ) {
				return qtrue;
			}
		}
		return qfalse;


	case IT_HOLDABLE:
		return qtrue;

	case IT_TREASURE:       // treasure always picked up
		return qtrue;

	case IT_CLIPBOARD:      // clipboards always picked up
		return qtrue;

		//---- (SA) Wolf keys
	case IT_KEY:
		return qtrue;       // keys are always picked up

	case IT_BAD:
		Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD" );
	default:
#ifndef Q3_VM
#ifndef NDEBUG
          Com_Printf("BG_CanItemBeGrabbed: unknown enum %d\n", item->giType );
#endif
#endif
         break;
	}

	return qfalse;
}

//======================================================================

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float deltaTime;
	float phase;
	vec3_t v;

	switch ( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
	case TR_GRAVITY_PAUSED: //----(SA)
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
//----(SA)	removed
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;     // FIXME: local gravity...
		break;
		// Ridah
	case TR_GRAVITY_LOW:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * ( DEFAULT_GRAVITY * 0.3 ) * deltaTime * deltaTime;     // FIXME: local gravity...
		break;
		// done.
//----(SA)
	case TR_GRAVITY_FLOAT:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * ( DEFAULT_GRAVITY * 0.2 ) * deltaTime;
		break;
//----(SA)	end
		// RF, acceleration
	case TR_ACCELERATE:     // trDelta is the ultimate speed
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		// phase is the acceleration constant
		phase = VectorLength( tr->trDelta ) / ( tr->trDuration * 0.001 );
		// trDelta at least gives us the acceleration direction
		VectorNormalize2( tr->trDelta, result );
		// get distance travelled at current time
		VectorMA( tr->trBase, phase * 0.5 * deltaTime * deltaTime, result, result );
		break;
	case TR_DECCELERATE:    // trDelta is the starting speed
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		// phase is the breaking constant
		phase = VectorLength( tr->trDelta ) / ( tr->trDuration * 0.001 );
		// trDelta at least gives us the acceleration direction
		VectorNormalize2( tr->trDelta, result );
		// get distance travelled at current time (without breaking)
		VectorMA( tr->trBase, deltaTime, tr->trDelta, v );
		// subtract breaking force
		VectorMA( v, -phase * 0.5 * deltaTime * deltaTime, result, result );
		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trType );
		break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result, qboolean isAngle, int splineData ) {
	float deltaTime;
	float phase;

	switch ( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear( result );
		break;
	case TR_LINEAR:
		VectorCopy( tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = cos( deltaTime * M_PI * 2 );    // derivative of sin = cos
		phase *= 0.5;
		VectorScale( tr->trDelta, phase, result );
		break;
//----(SA)	removed
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= DEFAULT_GRAVITY * deltaTime;       // FIXME: local gravity...
		break;
		// Ridah
	case TR_GRAVITY_LOW:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= ( DEFAULT_GRAVITY * 0.3 ) * deltaTime;       // FIXME: local gravity...
		break;
		// done.
//----(SA)
	case TR_GRAVITY_FLOAT:
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= ( DEFAULT_GRAVITY * 0.2 ) * deltaTime;
		break;
//----(SA)	end
		// RF, acceleration
	case TR_ACCELERATE: // trDelta is eventual speed
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		phase = deltaTime / (float)tr->trDuration;
		VectorScale( tr->trDelta, deltaTime * deltaTime, result );
		break;
	case TR_DECCELERATE:    // trDelta is breaking force
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
		VectorScale( tr->trDelta, deltaTime, result );
		break;
	case TR_SPLINE:
	case TR_LINEAR_PATH:
		VectorClear( result );
		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}
/*
============
BG_GetMarkDir

  used to find a good directional vector for a mark projection, which will be more likely
  to wrap around adjacent surfaces

  dir is the direction of the projectile or trace that has resulted in a surface being hit
============
*/
void BG_GetMarkDir( const vec3_t dir, const vec3_t normal, vec3_t out ) {
	vec3_t ndir, lnormal;
	float minDot = 0.3;

	if ( VectorLength( normal ) < 1.0 ) {
		VectorSet( lnormal, 0, 0, 1 );
	} else {
		VectorCopy( normal, lnormal );
	}

	VectorNegate( dir, ndir );
	VectorNormalize( ndir );
	if ( normal[2] > 0.8 ) {
		minDot = 0.7;
	}
	// make sure it makrs the impact surface
	while ( DotProduct( ndir, lnormal ) < minDot ) {
		VectorMA( ndir, 0.5, lnormal, ndir );
		VectorNormalize( ndir );
	}

	VectorCopy( ndir, out );
}


char *eventnames[] = {
	"EV_NONE",
	"EV_FOOTSTEP",
	"EV_FOOTSTEP_METAL",
	"EV_FOOTSTEP_WOOD",
	"EV_FOOTSTEP_GRASS",
	"EV_FOOTSTEP_GRAVEL",
	"EV_FOOTSTEP_ROOF",
	"EV_FOOTSTEP_SNOW",
	"EV_FOOTSTEP_CARPET",
	"EV_FOOTSPLASH",
	"EV_FOOTWADE",
	"EV_SWIM",
	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",
	"EV_FALL_SHORT",
	"EV_FALL_MEDIUM",
	"EV_FALL_FAR",
	"EV_FALL_NDIE",
	"EV_FALL_DMG_10",
	"EV_FALL_DMG_15",
	"EV_FALL_DMG_25",
	"EV_FALL_DMG_50",
	"EV_JUMP_PAD",           // boing sound at origin, jump sound on player
	"EV_JUMP",
	"EV_WATER_TOUCH",    // foot touches
	"EV_WATER_LEAVE",    // foot leaves
	"EV_WATER_UNDER",    // head touches
	"EV_WATER_CLEAR",    // head leaves
	"EV_ITEM_PICKUP",            // normal item pickups are predictable
	"EV_ITEM_PICKUP_QUIET",  // (SA) same, but don't play the default pickup sound as it was specified in the ent
	"EV_GLOBAL_ITEM_PICKUP", // powerup / team sounds are broadcast to everyone
	"EV_NOITEM",
	"EV_NOAMMO",
	"EV_WEAPONSWITCHED", // autoreload
	"EV_EMPTYCLIP",
	"EV_FILL_CLIP",
	"EV_WEAP_OVERHEAT",
	"EV_CHANGE_WEAPON",
	"EV_FIRE_WEAPON",
	"EV_FIRE_WEAPONB",
	"EV_FIRE_WEAPON_LASTSHOT",
	"EV_FIRE_QUICKGREN", // "Quickgrenade"
	"EV_NOFIRE_UNDERWATER",
	"EV_FIRE_WEAPON_MG42",
	"EV_SUGGESTWEAP",        //----(SA)	added
	"EV_GRENADE_SUICIDE",    //----(SA)	added
	"EV_USE_ITEM0",
	"EV_USE_ITEM1",
	"EV_USE_ITEM2",
	"EV_USE_ITEM3",
	"EV_USE_ITEM4",
	"EV_USE_ITEM5",
	"EV_USE_ITEM6",
	"EV_USE_ITEM7",
	"EV_USE_ITEM8",
	"EV_USE_ITEM9",
	"EV_USE_ITEM10",
	"EV_USE_ITEM11",
	"EV_USE_ITEM12",
	"EV_USE_ITEM13",
	"EV_USE_ITEM14",
	"EV_USE_ITEM15",
	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",
	"EV_GRENADE_BOUNCE",     // eventParm will be the soundindex
	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",       // no attenuation
	"EV_BULLET_HIT_FLESH",
	"EV_BULLET_HIT_WALL",
	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_RAILTRAIL",
	"EV_VENOM",
	"EV_VENOMFULL",
	"EV_BULLET",             // otherEntity is the shooter
	"EV_LOSE_HAT",
	"EV_GIB_HEAD",           // only blow off the head
	"EV_PAIN",
	"EV_CROUCH_PAIN",
	"EV_DEATH1",
	"EV_DEATH2",
	"EV_DEATH3",
	"EV_ENTDEATH",           //----(SA)	added
	"EV_OBITUARY",
	"EV_POWERUP_QUAD",
	"EV_POWERUP_BATTLESUIT",
	"EV_POWERUP_REGEN",
	"EV_GIB_PLAYER",         // gib a previously living player
	"EV_DEBUG_LINE",
	"EV_STOPLOOPINGSOUND",
	"EV_STOPSTREAMINGSOUND", //----(SA)	added
	"EV_TAUNT",
	"EV_SMOKE",
	"EV_SPARKS",
	"EV_SPARKS_ELECTRIC",
	"EV_BATS",
	"EV_BATS_UPDATEPOSITION",
	"EV_BATS_DEATH",
	"EV_EXPLODE",        // func_explosive
	"EV_EFFECT",     // target_effect
	"EV_MORTAREFX",  // mortar firing
	"EV_SPINUP", // JPW NERVE panzerfaust preamble for MP balance
	"EV_SNOW_ON",
	"EV_SNOW_OFF",
	"EV_MISSILE_MISS_SMALL",
	"EV_MISSILE_MISS_LARGE",
	"EV_WOLFKICK_HIT_FLESH",
	"EV_WOLFKICK_HIT_WALL",
	"EV_WOLFKICK_MISS",
	"EV_SPIT_HIT",
	"EV_SPIT_MISS",
	"EV_SHARD",
	"EV_JUNK",
	"EV_EMITTER",    //----(SA)	added
	"EV_OILPARTICLES",
	"EV_OILSLICK",
	"EV_OILSLICKREMOVE",
	"EV_MG42EFX",
	"EV_FLAMEBARREL_BOUNCE",
	"EV_FLAKGUN1",
	"EV_FLAKGUN2",
	"EV_FLAKGUN3",
	"EV_FLAKGUN4",
	"EV_EXERT1",
	"EV_EXERT2",
	"EV_EXERT3",
	"EV_SNOWFLURRY",
	"EV_CONCUSSIVE",
	"EV_DUST",
	"EV_RUMBLE_EFX",
	"EV_GUNSPARKS",
	"EV_FLAMETHROWER_EFFECT",
	"EV_SNIPER_SOUND",
	"EV_POPUP",
	"EV_POPUPBOOK",
	"EV_GIVEPAGE",
	"EV_CLOSEMENU",
	"EV_M97_PUMP", //jaymod

	"EV_MAX_EVENTS"
};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void    trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {

#ifdef _DEBUG
	{
		char buf[256];
		trap_Cvar_VariableStringBuffer( "showevents", buf, sizeof( buf ) );
		if ( atof( buf ) != 0 ) {
#ifdef QAGAME
			Com_Printf( " game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount /*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm );
#else
			Com_Printf( "Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount /*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm );
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & ( MAX_EVENTS - 1 )] = newEvent;
	ps->eventParms[ps->eventSequence & ( MAX_EVENTS - 1 )] = eventParm;
	ps->eventSequence++;
}


/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap ) {
	int i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_flags & PMF_LIMBO ) { // JPW NERVE limbo
		s->eType = ET_INVISIBLE;
	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		SnapVector( s->pos.trBase );
	}

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		SnapVector( s->apos.trBase );
	}

	if ( ps->movementDir > 128 ) {
		s->angles2[YAW] = (float)ps->movementDir - 256;
	} else {
		s->angles2[YAW] = ps->movementDir;
	}

	s->legsAnim     = ps->legsAnim;
	s->torsoAnim    = ps->torsoAnim;
	s->clientNum    = ps->clientNum;    // ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	// Ridah, let clients know if this person is using a mounted weapon
	// so they don't show any client muzzle flashes

	// (SA) moved up since it needs to set the ps->eFlags too.
	//		Seems like this could be the problem Raf was
	//		encountering with the EF_DEAD flag below when guys
	//		dead flags weren't sticking

	if ( ps->persistant[PERS_HWEAPON_USE] ) {
		ps->eFlags |= EF_MG42_ACTIVE;
	} else {
		ps->eFlags &= ~EF_MG42_ACTIVE;
	}

	s->eFlags = ps->eFlags;

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

// from MP
	if ( ps->externalEvent ) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else if ( ps->entityEventSequence < ps->eventSequence ) {
		int seq;

		if ( ps->entityEventSequence < ps->eventSequence - MAX_EVENTS ) {
			ps->entityEventSequence = ps->eventSequence - MAX_EVENTS;
		}
		seq = ps->entityEventSequence & ( MAX_EVENTS - 1 );
		s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
		ps->entityEventSequence++;
	}
// end
	// Ridah, now using a circular list of events for all entities
	// add any new events that have been added to the playerState_t
	// (possibly overwriting entityState_t events)
	for ( i = ps->oldEventSequence; i != ps->eventSequence; i++ ) {
		s->events[s->eventSequence & ( MAX_EVENTS - 1 )] = ps->events[i & ( MAX_EVENTS - 1 )];
		s->eventParms[s->eventSequence & ( MAX_EVENTS - 1 )] = ps->eventParms[i & ( MAX_EVENTS - 1 )];
		s->eventSequence++;
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}

	s->aiChar = ps->aiChar; // Ridah
//	s->loopSound = ps->loopSound;
	s->teamNum = ps->teamNum;
	s->aiState = ps->aiState;
}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap ) {
	int i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_flags & PMF_LIMBO ) { // JPW NERVE limbo
		s->eType = ET_INVISIBLE;
	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_LINEAR_STOP;
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		SnapVector( s->pos.trBase );
	}
	// set the trDelta for flag direction and linear prediction
	VectorCopy( ps->velocity, s->pos.trDelta );
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		SnapVector( s->apos.trBase );
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;       // ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	if ( ps->externalEvent ) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else if ( ps->entityEventSequence < ps->eventSequence ) {
		int seq;

		if ( ps->entityEventSequence < ps->eventSequence - MAX_EVENTS ) {
			ps->entityEventSequence = ps->eventSequence - MAX_EVENTS;
		}
		seq = ps->entityEventSequence & ( MAX_EVENTS - 1 );
		s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
		ps->entityEventSequence++;
	}

	// Ridah, now using a circular list of events for all entities
	// add any new events that have been added to the playerState_t
	// (possibly overwriting entityState_t events)
	if ( ps->oldEventSequence > ps->eventSequence ) {
		ps->oldEventSequence = ps->eventSequence;
	}
	for ( i = ps->oldEventSequence; i != ps->eventSequence; i++ ) {
		s->events[s->eventSequence & ( MAX_EVENTS - 1 )] = ps->events[i & ( MAX_EVENTS - 1 )];
		s->eventParms[s->eventSequence & ( MAX_EVENTS - 1 )] = ps->eventParms[i & ( MAX_EVENTS - 1 )];
		s->eventSequence++;
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}

//	s->loopSound = ps->loopSound;
//	s->generic1 = ps->generic1;
	s->aiChar = ps->aiChar; // Ridah
	s->teamNum = ps->teamNum;
	s->aiState = ps->aiState;
}

/*
=================
PC_Int_Parse
=================
*/
qboolean PC_Int_Parse( int handle, int *i ) {
	pc_token_t token;
	int negative = qfalse;

	if ( !trap_PC_ReadToken( handle, &token ) ) {
		return qfalse;
	}
	if ( token.string[0] == '-' ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			return qfalse;
		}
		negative = qtrue;
	}
	if ( token.type != TT_NUMBER ) {
		PC_SourceError( handle, "expected integer but found %s\n", token.string );
		return qfalse;
	}
	*i = token.intvalue;
	if ( negative ) {
		*i = -*i;
	}
	return qtrue;
}

/*
=================
PC_String_ParseNoAlloc

Same as one above, but uses a static buff and not the string memory pool
=================
*/
qboolean PC_String_ParseNoAlloc( int handle, char *out, size_t size ) {
	pc_token_t token;

	if ( !trap_PC_ReadToken( handle, &token ) ) {
		return qfalse;
	}

	Q_strncpyz( out, token.string, size );
	return qtrue;
}


// Real printable charater count
int BG_drawStrlen( const char *str ) {
	int cnt = 0;

	while ( *str ) {
		if ( Q_IsColorString( str ) ) {
			str += 2;
		} else {
			cnt++;
			str++;
		}
	}
	return( cnt );
}


// Copies a color string, with limit of real chars to print
//		in = reference buffer w/color
//		out = target buffer
//		str_max = max size of printable string
//		out_max = max size of target buffer
//
// Returns size of printable string
int BG_colorstrncpyz( char *in, char *out, int str_max, int out_max ) {
	int str_len = 0;    // current printable string size
	int out_len = 0;    // current true string size
	const int in_len = strlen( in );

	out_max--;
	while ( *in && out_len < out_max && str_len < str_max ) {
		if ( *in == '^' ) {
			if ( out_len + 2 >= in_len && out_len + 2 >= out_max ) {
				break;
			}

			*out++ = *in++;
			*out++ = *in++;
			out_len += 2;
			continue;
		}

		*out++ = *in++;
		str_len++;
		out_len++;
	}

	*out = 0;

	return( str_len );
}

int BG_strRelPos( char *in, int index ) {
	int cPrintable = 0;
	const char *ref = in;

	while ( *ref && cPrintable < index ) {
		if ( Q_IsColorString( ref ) ) {
			ref += 2;
		} else {
			ref++;
			cPrintable++;
		}
	}

	return( ref - in );
}

// strip colors and control codes, copying up to dwMaxLength-1 "good" chars and nul-terminating
// returns the length of the cleaned string
int BG_cleanName( const char *pszIn, char *pszOut, unsigned int dwMaxLength, qboolean fCRLF ) {
	const char *pInCopy = pszIn;
	const char *pszOutStart = pszOut;

	while ( *pInCopy && ( pszOut - pszOutStart < dwMaxLength - 1 ) ) {
		if ( *pInCopy == '^' ) {
			pInCopy += ( ( pInCopy[1] == 0 ) ? 1 : 2 );
		} else if ( ( *pInCopy < 32 && ( !fCRLF || *pInCopy != '\n' ) ) || ( *pInCopy > 126 ) )    {
			pInCopy++;
		} else {
			*pszOut++ = *pInCopy++;
		}
	}

	*pszOut = 0;
	return( pszOut - pszOutStart );
}

// Only used locally
typedef struct {
	char *colorname;
	vec4_t *color;
} colorTable_t;

extern void trap_Cvar_Set( const char *var_name, const char *value );



///////////////////////////////////////////////////////////////////////////////
typedef struct locInfo_s {
	vec2_t gridStartCoord;
	vec2_t gridStep;
} locInfo_t;

static locInfo_t locInfo;

void BG_InitLocations( vec2_t world_mins, vec2_t world_maxs ) {
	// keep this in sync with CG_DrawGrid
	locInfo.gridStep[0] = 1200.f;
	locInfo.gridStep[1] = 1200.f;

	// ensure minimal grid density
	while ( ( world_maxs[0] - world_mins[0] ) / locInfo.gridStep[0] < 7 )
		locInfo.gridStep[0] -= 50.f;
	while ( ( world_mins[1] - world_maxs[1] ) / locInfo.gridStep[1] < 7 )
		locInfo.gridStep[1] -= 50.f;

	locInfo.gridStartCoord[0] = world_mins[0] + .5f * ( ( ( ( world_maxs[0] - world_mins[0] ) / locInfo.gridStep[0] ) - ( (int)( ( world_maxs[0] - world_mins[0] ) / locInfo.gridStep[0] ) ) ) * locInfo.gridStep[0] );
	locInfo.gridStartCoord[1] = world_mins[1] - .5f * ( ( ( ( world_mins[1] - world_maxs[1] ) / locInfo.gridStep[1] ) - ( (int)( ( world_mins[1] - world_maxs[1] ) / locInfo.gridStep[1] ) ) ) * locInfo.gridStep[1] );
}

char *BG_GetLocationString( vec_t* pos ) {
	static char coord[6];
	int x, y;

	coord[0] = '\0';

	x = ( pos[0] - locInfo.gridStartCoord[0] ) / locInfo.gridStep[0];
	y = ( locInfo.gridStartCoord[1] - pos[1] ) / locInfo.gridStep[1];

	if ( x < 0 ) {
		x = 0;
	}
	if ( y < 0 ) {
		y = 0;
	}

	Com_sprintf( coord, sizeof( coord ), "%c,%i", 'A' + x, y );

	return coord;
}

qboolean BG_BBoxCollision( vec3_t min1, vec3_t max1, vec3_t min2, vec3_t max2 ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		if ( min1[i] > max2[i] ) {
			return qfalse;
		}
		if ( min2[i] > max1[i] ) {
			return qfalse;
		}
	}

	return qtrue;
}


/*
=================
PC_SourceError
=================
*/
void PC_SourceError( int handle, char *format, ... ) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	Q_vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine( handle, filename, &line );

	Com_Printf( S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string );
}


/*
=================
PC_Vec_Parse
=================
*/
qboolean PC_Vec_Parse( int handle, vec3_t *c ) {
	int i;
	float f;

	for ( i = 0; i < 3; i++ ) {
		if ( !PC_Float_Parse( handle, &f ) ) {
			return qfalse;
		}
		( *c )[i] = f;
	}
	return qtrue;
}


/*
=================
PC_Float_Parse
=================
*/
qboolean PC_Float_Parse( int handle, float *f ) {
	pc_token_t token;
	int negative = qfalse;

	if ( !trap_PC_ReadToken( handle, &token ) ) {
		return qfalse;
	}
	if ( token.string[0] == '-' ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			return qfalse;
		}
		negative = qtrue;
	}
	if ( token.type != TT_NUMBER ) {
		PC_SourceError( handle, "expected float but found %s\n", token.string );
		return qfalse;
	}
	if ( negative ) {
		*f = -token.floatvalue;
	} else {
		*f = token.floatvalue;
	}
	return qtrue;
}

/*
=================
PC_Color_Parse
=================
*/
qboolean PC_Color_Parse( int handle, vec4_t *c ) {
	int i;
	float f;

	for ( i = 0; i < 4; i++ ) {
		if ( !PC_Float_Parse( handle, &f ) ) {
			return qfalse;
		}
		( *c )[i] = f;
	}
	return qtrue;
}


// Get weap filename for specified weapon id
// Returns empty string when none found
char *BG_GetWeaponFilename( weapon_t weaponNum )
{
	switch ( weaponNum ) {
		case WP_KNIFE:             return "knife.weap";
		case WP_LUGER:             return "luger.weap";
		case WP_SILENCER:          return "luger_silenced.weap";
		case WP_COLT:              return "colt.weap";
		case WP_AKIMBO:            return "akimbo.weap";
		case WP_TT33:              return "tt33.weap";
		case WP_REVOLVER:          return "revolver.weap";
		case WP_THOMPSON:          return "thompson.weap";
		case WP_STEN:              return "sten.weap";
		case WP_MP34:              return "mp34.weap";
		case WP_PPSH:              return "ppsh.weap";
		case WP_MP40:              return "mp40.weap";
		case WP_MAUSER:            return "mauser.weap";
		case WP_SNIPERRIFLE:       return "sniperrifle.weap";
		case WP_GARAND:            return "garand.weap";
		case WP_SNOOPERSCOPE:      return "snooper.weap";
		case WP_MOSIN:             return "mosin.weap";
		case WP_G43:               return "g43.weap";
		case WP_M1GARAND:          return "m1garand.weap";
		case WP_M7:                return "m7.weap";
		case WP_FG42:              return "fg42.weap";
		case WP_FG42SCOPE:         return "fg42scope.weap";
		case WP_MP44:              return "mp44.weap";
		case WP_BAR:               return "bar.weap";
		case WP_M97:               return "ithaca.weap";
		case WP_FLAMETHROWER:      return "flamethrower.weap";
		case WP_PANZERFAUST:       return "panzerfaust.weap";
		case WP_MG42M:             return "mg42m.weap";
		case WP_TESLA:             return "tesla.weap";
		case WP_VENOM:             return "venom.weap";
		case WP_GRENADE_LAUNCHER:  return "grenade.weap";
		case WP_GRENADE_PINEAPPLE: return "pineapple.weap";
		case WP_DYNAMITE:          return "dynamite.weap";
		case WP_BROWNING:          return "browning.weap";
		case WP_AIRSTRIKE:         return "airstrike.weap";
		case WP_POISONGAS:         return "poisongas.weap";
		case WP_NONE:
		case WP_MONSTER_ATTACK1:
		case WP_MONSTER_ATTACK2:
		case WP_MONSTER_ATTACK3:
		case WP_GAUNTLET:
		case WP_SNIPER:
		case VERYBIGEXPLOSION:
		case WP_MORTAR:            return "";
		default:                   Com_Printf( "Missing filename entry for weapon id %d\n", weaponNum );
    }

    return "";
}


// Read ammo parameters into ammoTable from given file handle
// File handle position expected to be at opening brace of ammo block
// Utilised by Client and Game to update their respective copies of ammoTable
// handle not freed
qboolean BG_ParseAmmoTable( int handle, weapon_t weaponNum )
{
	pc_token_t token;

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		PC_SourceError( handle, "expected '{'" );
		return qfalse;
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}
		if ( token.string[0] == '}' ) {
			break;
		}

		// Ammo parameters for each difficulty level
		if ( !Q_stricmp( token.string, "maxammoPerSkill" ) ) {
			for (int i = 0; i < GSKILL_NUM_SKILLS; ++i) {
				if ( !PC_Int_Parse( handle, &ammoSkill[i][weaponNum].maxammo ) ) {
					PC_SourceError( handle, "expected maxammo value for skill level" );
					return qfalse;
				}
			}
		} else if ( !Q_stricmp( token.string, "maxclipPerSkill" ) ) {
			for (int i = 0; i < GSKILL_NUM_SKILLS; ++i) {
				if ( !PC_Int_Parse( handle, &ammoSkill[i][weaponNum].maxclip ) ) {
					PC_SourceError( handle, "expected maxclip value for skill level" );
					return qfalse;
				}
			}
		}
		// Values common to all skill levels
		else if ( !Q_stricmp( token.string, "uses" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].uses ) ) {
				PC_SourceError( handle, "expected uses value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "reloadTime" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].reloadTime ) ) {
				PC_SourceError( handle, "expected reloadTime value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "fireDelayTime" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].fireDelayTime ) ) {
				PC_SourceError( handle, "expected fireDelayTime value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "nextShotTime" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].nextShotTime ) ) {
				PC_SourceError( handle, "expected nextShotTime value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "nextShotTime2" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].nextShotTime2 ) ) {
				PC_SourceError( handle, "expected nextShotTime2 value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "maxHeat" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].maxHeat ) ) {
				PC_SourceError( handle, "expected maxHeat value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "coolRate" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].coolRate ) ) {
				PC_SourceError( handle, "expected coolRate value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "playerDamage" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].playerDamage ) ) {
				PC_SourceError( handle, "expected playerDamage value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "aiDamage" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].aiDamage ) ) {
				PC_SourceError( handle, "expected aiDamage value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "playerSplashRadius" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].playerSplashRadius ) ) {
				PC_SourceError( handle, "expected playerSplashRadius value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "aiSplashRadius" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].aiSplashRadius ) ) {
				PC_SourceError( handle, "expected aiSplashRadius value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "spread" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].spread ) ) {
				PC_SourceError( handle, "expected spread value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "aimSpreadScaleAdd" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].aimSpreadScaleAdd ) ) {
				PC_SourceError( handle, "expected aimSpreadScaleAdd value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "spreadScale" ) ) {
			if ( !PC_Float_Parse( handle, &ammoTable[weaponNum].spreadScale ) ) {
				PC_SourceError( handle, "expected spreadScale value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "weapRecoilDuration" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].weapRecoilDuration ) ) {
				PC_SourceError( handle, "expected weapRecoilDuration value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "weapRecoilPitch" ) ) {
			if ( !PC_Float_Parse( handle, &ammoTable[weaponNum].weapRecoilPitch[0] ) ) {
				PC_SourceError( handle, "expected weapRecoilPitch.x value" );
				return qfalse;
			}
			if ( !PC_Float_Parse( handle, &ammoTable[weaponNum].weapRecoilPitch[1] ) ) {
				PC_SourceError( handle, "expected weapRecoilPitch.y value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "weapRecoilYaw" ) ) {
			if ( !PC_Float_Parse( handle, &ammoTable[weaponNum].weapRecoilYaw[0] ) ) {
				PC_SourceError( handle, "expected weapRecoilYaw.x value" );
				return qfalse;
			}
			if ( !PC_Float_Parse( handle, &ammoTable[weaponNum].weapRecoilYaw[1] ) ) {
				PC_SourceError( handle, "expected weapRecoilYaw.y value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "soundRange" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].soundRange ) ) {
				PC_SourceError( handle, "expected soundRange value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "moveSpeed" ) ) {
			if ( !PC_Float_Parse( handle, &ammoTable[weaponNum].moveSpeed ) ) {
				PC_SourceError( handle, "expected moveSpeed value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "twoHand" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].twoHand ) ) {
				PC_SourceError( handle, "expected twoHand value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "upAngle" ) ) {
			if ( !PC_Int_Parse( handle, &ammoTable[weaponNum].upAngle ) ) {
				PC_SourceError( handle, "expected upAngle value" );
				return qfalse;
			}
		} else {
			PC_SourceError( handle, "unknown token '%s'", token.string );
			return qfalse;
		} 
	}

	return qtrue;
}


// Set weapon parameters for specified skill
void BG_SetWeaponForSkill( weapon_t weaponNum, gameskill_t skill )
{
	if ( ammoSkill[skill][weaponNum].maxammo > 0 )
		ammoTable[weaponNum].maxammo = ammoSkill[skill][weaponNum].maxammo;

	if ( ammoSkill[skill][weaponNum].maxclip > 0 )
		ammoTable[weaponNum].maxclip = ammoSkill[skill][weaponNum].maxclip;
}


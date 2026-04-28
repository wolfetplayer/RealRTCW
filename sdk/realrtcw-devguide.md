# RealRTCW Development Guide

## Atmospheric Effects
Atmospheric effects system allows you to apply snow and rain to the maps without manual placement of entities throughout the map. 

Add them via the `atmosphere` key in the `worldspawn` entity.

Presets for `atmosphere` key value:

    T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=50 50,GV=30 80,W=1 2,D=15000
    T=RAIN,B=5 10,C=0.5,G=0.5 2,BV=50 50,GV=200 200,W=1 2,D=5000

Description for every key you can tune for your effect:
- T — effect type (RAIN or SNOW, required)
- B — particle size range
- C — density / visibility factor
- G — wind gust strength range
- BV — base wind direction (x y)
- GV — gust wind direction (x y)
- W — base vs gust wind influence
- D — number of particles (overall intensity)
- S — splash amount (rain only, legacy)

### CVARS:
- `cg_atmoshpericeffects` - disables/enables atmospheric effects rendering
- `cg_forceatmosphericeffects` - no force (0), rain (1), snow (2)
- `cg_lowAtmosphericEffects` - high (0), medium (1), 2 (disabled)


## Foliage Technology

Foliage tech allows you to put a real 3D grass on your maps without big frames losses. Foliage tech is based on Wolfenstein: Enemy Territory tech by Splash Damage so I will rely on their documentation here.

When creating foliage, there are 3 things to set up: the foliage model, the foliage model's shader(s), and changes to any existing shader where foliage is desired on.

Foliage is compiled into the BSP hence it is static . If you need to change the foliage in your map, it must be recompiled. This is done for performance reasons -- when foliage is loaded by RealRTCW it is compiled into a list for fast rendering.

To compile foliage into a map, SDMap2 (Q3Map2) 2.3.32 or higher is required.

Related Cvars:

`r_drawFoliage` - enable/disable foliage rendering

### Adding Foliage to Existing Shaders

There is a new shader directive 'q3map_foliage' that specifies how SDMap2 applies foliage to a surface. It takes this form:

    q3map_foliage <model>  <scale>  <density>  <odds>  <use  inverse  alpha>
    q3map_foliage models/foliage/grass_5.md3 1.0 16 0.025 0


- model:	models/foliage/grass_5.md3
- scale:	1.0. This is normal size, 0.5 would be half size, 2.0 would be double
- density:	T16 units. This is the smallest chunk Q3Map will divide a surface up into before it tries to place a foliage instance.
- odds:	0.025. This means that a random 2.5% of the potential spots for foliage will be placed. Typically you want to use small values for this; otherwise you will end up with ridiculously high polygon counts.
- inverse alpha:	0. this means to use the straight vertex alpha as a scaling factor against the odds of appearing. This is so that terrain shaders with multiple blending layers can have different foliage on each style and have them fade/blend properly together.

If you have brush on terrain0 and grass on terrain1, then the blend shader would have two q3map_foliage directives like this:

    q3map_foliage models/foliage/brush.md3 1.0 16 0.025 1
    q3map_foliage models/foliage/grass_5.md3 1.0 16 0.025 0

Where the higher-numbered terrain layer/shader uses normal vertex alpha to modulate the odds-of-occurring and the lower-numbered layer uses inverse alpha.

### Creating a Foliage Model

Foliage models should be kept simple and small. Since they're entirely decorative and non-solid, you should avoid making a foliage model that looks as if it could block the player.

For best results, make your models as a single object (multiple objects will slow down rendering) with a single texture/shader, and try to keep the polygon count as low as possible. Our initial test foliage models were only 6 triangles apiece. When there are a few thousand foliage instances on-screen, the small numbers can add up, so be thrifty.

### Foliage Model Shader Example

    models/foliage/grass_5
    {
    	nopicmip
    	qer_alphafunc greater 0.5
    	qer_editorimage models/foliage/grass_tan.tga
    	cull disable
    	sort seethrough
    	surfaceparm pointlight
    	surfaceparm trans
    	surfaceparm nomarks
    	// distanceCull <inner> <outer> <alpha threshold>
    	distanceCull 256 4096 0.49
    	{
    		map models/foliage/grass_tan.tga
    		//blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    		alphaFunc GE128
    		rgbGen exactVertex
    		alphaGen vertex
    	}
    }

## Automatic AI attributes system

In RTCW AI attributes are handled in .ai script files. It usually looks something like this:

    nazi2d
    {
    	attributes
    	{
    		starting_health 40		
    		aim_accuracy 0.3
    		aim_skill 0.5
    		attack_skill 0.5
    		inner_detection_radius 1400
    	}

Each AI on every map has its attributes defined. If certain attribute is not specified - game will use default values from the code. In RealRTCW I did a few modifications to allow global changes with just a few strings editions.

Default AI attributes values are parsed out of the code into ` .aidefaults ` text files. This is how it looks for soldier AI in `soldier.aidefaults`  file:

    AICharacterDef
    {
    	behavior {
    		runningSpeed                220
    		walkingSpeed                90
    		crouchingSpeed              80
    		fieldOfView                 90
    		yawSpeed                    200
    		leader                      0.0
    	    aimSkill                    0.2 0.5  0.3 0.6  0.4 0.7  0.5 0.8  0.5 0.8 0.3 0.8
    	    aimAccuracy                 0.1 0.6  0.2 0.6  0.3 0.7  0.4 0.8  0.4 0.8 0.3 0.8
    	    attackSkill                 0.1 0.8  0.2 0.8  0.3 0.8  0.4 0.8  0.4 0.8 0.5 1.0
    	    reactionTime                0.5 1.0  0.4 1.0  0.3 1.0  0.2 1.0  0.2 1.0 0.4 1.0
    		attackCrouch                0.4
    		idleCrouch                  0.0
    	    aggression                  0.1 0.5  0.2 0.5  0.3 0.6  0.4 0.7  0.5 0.8 1.0 1.0
    		tactical                    0.8
    		camper                      0.0
    		alertness                   16000
    	    startingHealth              15 25  20 30  25 35  25 35  5 15 20 30
    		hearingScale                1.0
    		notInPvsHearingScale        0.9
    		relaxedDetectionRadius      512
    		painThresholdMultiplier     1.0
    	}
    }

If game will fail to find any of listed attributes in the .ai file for specific AI - it will fall back to the values in `.aidefaults.`

This is `nazi2d` block from RealRTCW scripts:

    nazi2d
    {
    	attributes
    	{	
    		inner_detection_radius 1400
    	}

You can see that all atributes values were removed, so it will fall back to `.aidefaults` values.

Attributes like `aimSkill`, `aimAccuracy`, `attackSkill`, `reactionTime`, `aggression`, `startingHealth` can be randomized within defined values depending on the gameskill value. In the code above soldier health will randomized between 15 and 25 health points on easiest difficulty and between 25 and 35 on Death Incarnate difficulty. Last interval 20-30 is reserved for Survival mode, but it is not really used there, since Survival is using its own HP system for AIs.

This system allows you to do massive balance changes without editing each .ai on every map. But it is still optional and you can do it like in original RTCW scripts.

## Extended scripting

### Old functions improvements

`mu_queue` - can now randomize music tracks

Example usage:

"mu_queue sound/music/m_action sound/music/m_alarm"

### New give functions

`giveweaponfull ` – Takes away all AIs weapon, gives specified weapon, fills both reserve ammo and current clip to the max and selects the weapon itself. You can also randomize what weapon AI will get by simply typing multiple weapon after the command.

Example usage where AI will receive one of three weapons on spawn:

    trigger loadout_early
    {
    giveweaponfull weapon_mp40 weapon_luger weapon_mauserrifle
    }

`giveammo` – gives player an actual ammo item. 

Example usage: "giveammo ammo_9mm"

`givehealth` – gives player an actual health item. 

Example usage: "givehealth item_health_large"

`givearmor` – gives player an actual armor item. 

Example usage: "givearmor item_armor_body"

`giveinventory` – gives holdable item. 

Example usage: "giveinventory holdable_adrenaline"

`giveperk` - gives perk item.

Example usage: "giveperk perk_runner"

### Accum math operations

`accumaction` - math operations with accum buffers.

Examples:

`accumaction 3 1 plus 0` - accum 3 equals accum 1 plus accum 0 

`accumaction 3 1 minus 0` - accum 3 equals accum 1 minus accum 0 

`accumaction 3 1 mul 0` - accum 3 equals accum 1 multiply on accum 0

`accumaction 3 1 div 0` - accum 3 equals accum1 divided on accum 0

`accumgametime` - store game time into accum buffer. 

Example: "accumgametime 1" - will store game time in accum 1

### Misc new functions

`savecheckpoint` – saves the checkpoint by writing "lastcheckpoint" save game.

`setmovespeed` - changes player movespeed through the triggers. Can also be applied to AI. 

Values - veryslow, slow, default, fast, veryfast. 

Example usage: "setmovespeed veryslow"

`drop_weapon` – makes AI to toss his current weapon

`changeaiteam` – change team of the AI on the fly

`changeainame` – changes AI script name on the fly

`burn` – make AI burn

`screenfade` - fades player screen in/out with defined fadetime.

Example usage: "screenfade 5000 in"

`dropitem` - makes AI drop defined item.

Example usage: "dropitem holdable_emp 0 22" 

This will make AI drop holdable_emp item which will stay forever and there is 22% probability that this will happen. If you dont want to mess with drop chance and stay time just go with "dropitem holdable_emp"

`defend` - makes AI defend area around defined ai_marker.

Example usage: "defend friendly3_marker1 64 0"

This will make AI defend friendly3_marker1 forever within 64 radius.

`face_entity`  –  makes AI look at the target entity

Example usage: "face_entity nazi1"
 
### Print Label (used for training level best time print)

`printlabel` - change parameters of displayed label

Syntax:

`printlabel txt`  - change text part of the label

`printlabel param`  - change numeric part of the label and position it with x y coordinates

`printlabel state` - show the label, if value of the accum is higher than 0, otherwise hide

`printlabel on` - show the label

`printlabel off` - hide the label

`printlabel format` - label formatting. Assing them in any order and divide them with space.

formatstring:

`pulse` – pulsing string

`string` – enable text part

`accum` – enable numeric part

`timer` – numeric part will be displayed in seconds

`inline` – one string

Example usage in malta_menu.script:

    	trigger time_start
    	{
    	    printlabel format string accum timer inline
    		printlabel on
    		accumgametime 2
    		trigger timer time_go
    	}
    	
    	trigger time_go
    	{
    	   accumgametime 1
    	   accumaction 1 1 minus 2
    	   printlabel param 1 400 50
    	   wait 5
    	   trigger timer time_go
    	}
    	
    	trigger time_stop
    	{
    	   printlabel format pulse string accum timer inline
    	   wait 5000
    	   printlabel off
    	}
    }

### If and endif cvars support

You can now rely on game cvars values in .ai scripts. Usage is plain simple:

	spawn
	{
		#if g_fullarsenal == 0
		giveweaponfull weapon_fg42
		#endif
		#if g_fullarsenal == 1
		giveweaponfull weapon_mp44
		#endif
		#if g_fullarsenal == 2
		giveweaponfull weapon_fg42
		#endif
	}

You can use `>=` `<=` `==` `!=` `>` `<` while comparing the cvar value.

### trigger self*

If you are writing a script inside AI blocks you can just refer to AI as `*self` if you are planning to execute certain actions on himself. 

Example usage where `self*` means nazi10:

    nazi10
    {
    
    	attributes
    	{
    	}
    	
    	respawn
    	{
    	resetscript
    	statetype alert
    	trigger self* loadout_early
    	trigger self* loadout_late
    	nosight 9999
    	gotomarker surv_entry_*
    	sight
    	}
    	
    	trigger loadout_early
    	{
    	wave abort_if_greater_than 9
    	giveweaponfull weapon_mp40 weapon_luger weapon_mauserrifle
    	}
    	
    	trigger loadout_late
    	{
    	wave abort_if_less_than 10
    	giveweaponfull weapon_mp44 weapon_g43
    	}
	}

### Stack AI scripting

If you have a bunch of AIs which are literally doing the same thing you can stack script them instead of copy pasting their script. I found a good use for it in Survival mode.

Example usage where 5 elite guards are scripted in the same blocks:

    eg1
    eg2
    eg3
    eg4
    {
    
    	attributes
    	{
    	}
    	
    	respawn
    	{
    	resetscript
    	giveweaponfull weapon_mp34 weapon_sten weapon_silencer
    	statetype alert
    	nosight 9999
    	gotomarker surv_entry_*
    	sight
    	}
    	
    	enemysight
    	{
    	}
    	
    	bulletimpact
    	{
    		deny
    	}
    
    	inspectsoundstart
    	{
    		deny
    	}
    
    	inspectsoundend
    	{
    		deny
    	}
    
    	inspectbodystart
    	{
    		deny
    	}
    
    	inspectbodyend
    	{
    		deny
    	}
    
    	death
    	{
    	}
    
    }

## Ents files

You can notice that besides `.ai` files in maps folder RealRTCW also has `.ents` files.

Those files allows you to add more entities onto your maps without recompiling them. You add entities into .ents the same way Radiant adds them onto your map – simply define its classname and origin.

As for the origin, you can simply launch the map in the game, stand on the point you want to add your entity and type in the console `/where`. This will give you the exact coordinates of the spot. Additional AIs must be specified in .ai file as well. Just like you normally do while creating the map.

Example from maps/assault.ents:

    {
    
    "classname" "ai_soldier"
    
    "origin" "3280 3130 472"
    
    "ainame" "reinforce_ai_soldier_2"
    
    "angle" "-177"
    
    "spawnflags" "1"
    
    "skin" "infantryss/assault1"
    
    "head" "assault2"
    
    }


## Weapon config files

RealRTCW introduced `.weap` files which are located in `weapons` .pk3 subfolder.

Those are text configs for weapons, which allows you to redefine weapon icons, sounds, models, combat parameters without getting involved into code editing and recompiling. It is all pretty self explanatory when you look at it. Example of `mp40.weap:`

    weaponDef
    {
    	client {
    		standModel		    "models/weapons/smgs/mp40/mp40_stand.mdc"
    		pickupModel			"models/weapons/smgs/mp40/mp40_3rd.md3"
    		droppedAnglesHack                                        
    		
    		weaponConfig		"models/weapons/smgs/mp40/weapon.cfg"
    		handsModel			"models/weapons/smgs/mp40/v_mp40_hand.mdc"
    		
    		flashDlightColor	1.0 0.6 0.23
    		flashSound			"sound/weapons/mp40/mp40_fire.wav"		
    		flashEchoSound		"sound/weapons/mp40/mp40_far.wav"		
    		//lastShotSound		""								
    		
    		//readySound		""
    		//firingSound		""									
    		//overheatSound		""
    		reloadSound			"sound/weapons/mp40/mp40_reload.wav"
    		reloadSoundFast		"sound/weapons/mp40/mp40_reload_fast.wav"
    		reloadSoundAi	    "sound/weapons/ai/smg_reload.wav"
    		//reloadFastSound	""
    		//spinupSound		""									
    		//spindownSound		""									
    		//switchSound		""
            bounceSound         "sound/misc/weapon_bounce.wav"		
    		
    		weaponIcon			"icons/iconw_mp40_1"
    		weaponSelectedIcon	"icons/iconw_mp40_1_select"
    		
    		//missileModel		""
    		//missileSound		""
    		//missileTrailFunc	""									// supports "GrenadeTrail", "RocketTrail", "PyroSmokeTrail")
    		//missileDlight		0
    		//missileDlightColor	0 0 0							
    	
    		weaponPosition         -3 2 0                              // Classic RealRTCW view
    		weaponPositionAlt      0 5 1                              // MoHAA/CoD1
    		weaponPositionAlt2     0 5 0                              // Quake1/Doom
    		
    		ejectBrassFunc		"PistolEjectBrass"				// supports "MachineGunEjectBrass" "PistolEjectBrass" and "PanzerFaustEjectBrass"
    		
    		//modModel 1		""
    		
    		firstPerson {
    			model			"models/weapons/smgs/mp40/v_mp40.md3"
    			flashModel		"models/weapons/smgs/mp40/v_mp40_flash.md3"
    
    			weaponLink
    			{
    				part 0
    				{
    					tag		"tag_barrel"
    					model	"models/weapons/smgs/mp40/v_mp40_barrel.md3"
    				}
    				part 1
    				{
    					tag		"tag_barrel2"
    					model	"models/weapons/smgs/mp40/v_mp40_barrel2.md3"
    				}
    				part 2
    				{
    					tag		"tag_barrel3"
    					model	"models/weapons/smgs/mp40/v_mp40_barrel3.md3"
    				}
    			}
    		}
    		
    		thirdPerson {
    			model			"models/weapons/smgs/mp40/mp40_3rd.md3"
    			flashmodel		"models/weapons/smgs/mp40/mp40_3rd_flash.md3"
    		}
    	}
    			ammo {
    			maxammoPerSkill         300 300 200 150 150 200
    			maxammoUpgradedPerSkill 500 500 500 500 500 400
    			maxclipPerSkill         32  32  32  32  32  32
    			maxclipUpgradedPerSkill 64  64  64  64  64  64
    			uses                    1
                usesUpgraded            2			
    			reloadTime              2500             
    			reloadTimeFull          2500               
    			fireDelayTime           0               
    			nextShotTime            110
                nextShotTimeUpgraded    110			
    			nextShotTime2           110
                nextShotTime2Upgraded   110			
    			maxHeat                 0                  
    			coolRate                0                  
    			playerDamage            6
                playerDamageUpgraded    20			
    			aiDamage                4                 
    			playerSplashRadius      0                  
    			aiSplashRadius          0                  
    			spread                  850
                spreadUpgraded          750			
    			aimSpreadScaleAdd       15                
    			spreadScale             0.5                
    			weapRecoilDuration      30                 
    			weapRecoilPitch         0.1 0.1            
    			weapRecoilYaw           0.0 0.0            
    			soundRange              1000              
    			moveSpeed               0.90               
    			twoHand                 1                  
    			upAngle                 0                  
    			falloffDistance         750.0 1500.0       
    		}
    }


## Loadouts

You can use `.loadout` files to create loadouts presets for the player. Loadouts are located in the `loadouts` subfolder inside of .pk3.

There is `loadouts.cfg` file which inlcudes list of all loadouts:

    loadouts/loadouts_campaign.loadout
    loadouts/loadouts_survival.loadout

And this is part of the `loadouts_campaign.loadout`:

    // Escape1
    loadout escape1
    {
    		giveweapon weapon_knife
    		
    		selectweapon weapon_knife
    }
    
    // Village1
    loadout village1
    {
    		giveweapon weapon_knife
    		giveweapon weapon_knife
    		giveweapon weapon_knife
    		
    		giveweapon weapon_luger
    		giveweapon weapon_mp40
    		giveweapon weapon_mauserrifle
    		
    		setammo ammo_9mm 128
    		setammo ammo_792mm 40
    		
    		setclip weapon_mp40 full
    		setclip weapon_luger full
    		setclip weapon_mauserrifle full
    		
    		selectweapon weapon_mp40
    		
    		giveinventory key_binocs
    }


To apply loadout you need to issue specific command in `.ai` file:

`applyloadout escape1`

This will execute all commands in loadout escape1 block.

Note: You can safely use `#if` and `#endif` cvars comparisons withing `.loadout` files.

## Cinematic videos

RealRTCW has basic FFMPEG support which allows developers to utilize `.webm` videos for in-game cinematics. 

There are 3 ways you can use cinematics: intro, in menu and in game.

To use your video as intro cinematic you need to replace `wolfintro.webm` in Main/video.

To use your video as menu element you can refer to this as an example:


    itemDef
    {
    	name "background"
    	rect 0 0 640 480
    	
    	style WINDOW_STYLE_CINEMATIC
    	cinematic "wolfLoop.webm"
    	
    	visible 1
    	decoration
    }

To play your video while in-game - utilize this command with your video name:

`cin_play 00tentvillage.webm`

Note. It is not recommended to use videos with resoltion higher than 1080p. It can lead to huge FPS drops. Optimal quality would be 720p here. At least for now.


## Subtitles

Subtitle files are located in `/text/EnglishUSA/maps`

Each map has its specific file for subtitles. You reference script name of the audio file, where AI speaks, and after that specify the text.

Ignored and unneeded technical lines are located in `text/ignoredstitles.txt`

Example of `boss1.txt` subtitle file:

    {
    "zemph_1" "Uh, what are you planning now?"
    
    "helga_1" "I'm not planning anything, I'm doing it! I'm getting that dagger!"
    
    "zemph_2" "No! You can't do that! You'll break the inner seal!"
    
    "helga_2" "I'm prepared to take that risk!"
    }


Related Cvars:

`cg_drawSubtitles` - enable/disable subtitles

`cg_subtitleSize` - subtitles size

`cg_subtitleShadow` - enable/disable subtitles shadow


## Improved localization system

In RealRTCW all user interface lines were removed from the `.menu` files and were stored in `text/text.txt`

This is how it works. You refer to the line in the .menu file like this with `text` key:

	itemDef {
	name look
	group controls
	type ITEM_TYPE_SLIDER
	text "@GAMEPAD_UISENS"
    cvarfloat "j_uiSpeed"  700 300 1000
	rect 300 295 290 12
	textalign ITEM_ALIGN_RIGHT
	textfont UI_FONT_BIG
	textalignx 142
	textaligny 10
	textscale .23
	style WINDOW_STYLE_FILLED
	backcolor 1 1 1 .07            
	forecolor 1 1 1 1
	visible 0 
	mouseenter { show slider_message }
	mouseexit { hide slider_message }
}

And then game translates it inside `text.txt` like this:

    GAMEPAD_MOVESENS           "Movement Sensitivity:"
    GAMEPAD_LOOKSENS           "Look Sensitivity:"
    GAMEPAD_UISENS             "UI Sensitivity:"

Note. You can use Quake 3 color codes like `^1`, etc. in this text file too.

Note. You can create multiple .txt files like `text_1`, `text_2`, etc. Each text file will overwrite previous one if matched lines will be found. This can be useful for mod localizations.
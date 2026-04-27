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
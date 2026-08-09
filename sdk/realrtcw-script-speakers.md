# RealRTCW Script Speakers (.sps)

Script speakers let you place ambient/scripted point sound sources on a map without touching the BSP entity lump — no recompile needed. They are independent of, and can be used alongside, classic `target_speaker` entities.

A speaker script lives at `sound/maps/<mapname>.sps`, derived automatically from the current map name. If the file doesn't exist, nothing happens — most maps simply won't have one.

## File Format

    speakerScript
    {
    	speakerDef
    	{
    		noise "sound/ambient/drip.wav"
    		origin 1024 512 64
    		looped "no"
    		broadcast "no"
    		wait 4000
    		random 1500
    		volume 127
    		range 1250
    	}
    }

You can define as many `speakerDef` blocks as you need, up to 256 per map.

### Keys

- `noise` — sound file to play (required)
- `origin` — world position of the speaker (required)
- `targetname` — optional label for your own bookkeeping. Not currently wired to any trigger, script, or activation system — it's saved/loaded for reference only.
- `looped` — `no` (replays on a timer, see `wait`/`random`), `on` (starts looping immediately on map load), `off` (defined but dormant; nothing currently turns it on)
- `broadcast` — `no` (must be in the listener's PVS to be heard), `global` (plays as a non-positional 2D sound to every client instantly, ignoring PVS and distance/volume falloff), `nopvs` (plays positionally with normal distance falloff, but ignores the PVS check — audible through walls/doors if in range)
- `wait` — milliseconds between automatic replays (`looped "no"` only)
- `random` — random variance (±) added to `wait` each cycle
- `volume` — 0-255, default 127
- `range` — attenuation distance in units, default 1250

### Notes

- For a `looped "no"` speaker to ever make sound, `wait` and/or `random` must be non-zero. With both at `0` the speaker is loaded but silent — there's no separate "fire once" trigger yet.
- `broadcast "no"` is the only mode gated by PVS; `global` and `nopvs` both ignore it (in different ways — see above).
- Looped speakers (`on`) start playing as soon as the map loads and keep looping until the map changes.

## Authoring In-Game

Instead of hand-editing the `.sps` file, you can place and manage speakers live from the console:

    dumpsound <soundfile> [wait=N] [random=N] [volume=N] [range=N] [looped=no|on|off] [broadcast=no|global|nopvs] [targetname=name]

Places a new speaker at your current position, starts it playing immediately, and appends it to the map's `.sps` file. Example:

    dumpsound sound/ambient/drip.wav wait=4000 random=1500 volume=100

    listsounds

Lists every speaker currently loaded for the map, with its index and settings.

    deletesound <index>

Removes a speaker by index (from `listsounds`) and re-saves the `.sps` file.

Since these commands write straight to `sound/maps/<mapname>.sps`, you can iterate on ambient sound placement entirely in-game without recompiling the map — just `map_restart` (or reload the map) to hear changes made by hand-editing the file, or hear `dumpsound` changes immediately in the same session.

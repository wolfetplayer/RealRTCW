/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).

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

// cg_speakerscript.c -- map speaker scripts (sound/maps/<mapname>.sps), independent of target_speaker (game/g_target.c)

#include "cg_local.h"
#include "../ui/ui_shared.h"    // PC_Int_Parse / PC_Float_Parse / PC_String_Parse

scriptSpeaker_t scriptSpeakers[MAX_SCRIPT_SPEAKERS];
int             numScriptSpeakers;

/*
================
CG_ClearScriptSpeakers
================
*/
void CG_ClearScriptSpeakers( void ) {
	numScriptSpeakers = 0;
}

/*
================
CG_SS_ParseError
================
*/
static void CG_SS_ParseError( int handle, const char *filename, const char *format, ... ) {
	va_list argptr;
	char string[1024];

	va_start( argptr, format );
	Q_vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );

	CG_Printf( S_COLOR_RED "ERROR: %s: %s\n", filename, string );

	trap_PC_FreeSource( handle );
}

/*
================
CG_SS_ParseSpeaker

Parses a single "speakerDef { ... }" block
================
*/
static qboolean CG_SS_ParseSpeaker( int handle, const char *filename ) {
	pc_token_t      token;
	scriptSpeaker_t speaker;

	Com_Memset( &speaker, 0, sizeof( speaker ) );
	speaker.volume = 127;
	speaker.range = 1250;

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		CG_SS_ParseError( handle, filename, "expected '{'" );
		return qfalse;
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "noise" ) ) {
			const char *s;
			if ( !PC_String_Parse( handle, &s ) ) {
				CG_SS_ParseError( handle, filename, "expected sound filename" );
				return qfalse;
			}
			Q_strncpyz( speaker.filename, s, sizeof( speaker.filename ) );
		} else if ( !Q_stricmp( token.string, "origin" ) ) {
			if ( !PC_Float_Parse( handle, &speaker.origin[0] ) ||
				 !PC_Float_Parse( handle, &speaker.origin[1] ) ||
				 !PC_Float_Parse( handle, &speaker.origin[2] ) ) {
				CG_SS_ParseError( handle, filename, "expected origin vector" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "targetname" ) ) {
			const char *s;
			if ( !PC_String_Parse( handle, &s ) ) {
				CG_SS_ParseError( handle, filename, "expected targetname string" );
				return qfalse;
			}
			Q_strncpyz( speaker.targetname, s, sizeof( speaker.targetname ) );
		} else if ( !Q_stricmp( token.string, "looped" ) ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				CG_SS_ParseError( handle, filename, "expected loop value" );
				return qfalse;
			}
			if ( !Q_stricmp( token.string, "no" ) ) {
				speaker.loop = SPKR_NOT_LOOPED;
			} else if ( !Q_stricmp( token.string, "on" ) ) {
				speaker.loop = SPKR_LOOPED_ON;
				speaker.activated = qtrue;
			} else if ( !Q_stricmp( token.string, "off" ) ) {
				speaker.loop = SPKR_LOOPED_OFF;
			} else {
				CG_SS_ParseError( handle, filename, "unknown loop value '%s'", token.string );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "broadcast" ) ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				CG_SS_ParseError( handle, filename, "expected broadcast value" );
				return qfalse;
			}
			if ( !Q_stricmp( token.string, "no" ) ) {
				speaker.broadcast = SPKR_LOCAL;
			} else if ( !Q_stricmp( token.string, "global" ) ) {
				speaker.broadcast = SPKR_GLOBAL;
			} else if ( !Q_stricmp( token.string, "nopvs" ) ) {
				speaker.broadcast = SPKR_NOPVS;
			} else {
				CG_SS_ParseError( handle, filename, "unknown broadcast value '%s'", token.string );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "wait" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.wait ) || speaker.wait < 0 ) {
				CG_SS_ParseError( handle, filename, "expected wait value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "random" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.random ) || speaker.random < 0 ) {
				CG_SS_ParseError( handle, filename, "expected random value" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "volume" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.volume ) || speaker.volume < 0 || speaker.volume > 255 ) {
				CG_SS_ParseError( handle, filename, "expected volume value (0-255)" );
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "range" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.range ) || speaker.range < 0 ) {
				CG_SS_ParseError( handle, filename, "expected range value" );
				return qfalse;
			}
		} else {
			CG_SS_ParseError( handle, filename, "unknown token '%s'", token.string );
			return qfalse;
		}
	}

	if ( numScriptSpeakers >= MAX_SCRIPT_SPEAKERS ) {
		CG_SS_ParseError( handle, filename, "MAX_SCRIPT_SPEAKERS (%i) reached", MAX_SCRIPT_SPEAKERS );
		return qfalse;
	}

	scriptSpeakers[numScriptSpeakers++] = speaker;

	return qtrue;
}

/*
================
CG_SpeakerScriptFilename

Derives sound/maps/<mapname>.sps from cgs.mapname ("maps/<mapname>.bsp").
Shared by load and save so the two can never disagree on the path.
================
*/
static void CG_SpeakerScriptFilename( char *out, int size ) {
	char mapname[MAX_QPATH];
	char *base, *ext;

	Q_strncpyz( mapname, cgs.mapname, sizeof( mapname ) );
	base = strrchr( mapname, '/' );
	base = base ? base + 1 : mapname;
	ext = strrchr( base, '.' );
	if ( ext ) {
		*ext = '\0';
	}

	Com_sprintf( out, size, "sound/maps/%s.sps", base );
}

/*
================
CG_LoadSpeakerScript

Loads sound/maps/<mapname>.sps, derived from cgs.mapname ("maps/<mapname>.bsp").
Not finding a .sps file is not an error -- most maps simply won't have one.
================
*/
void CG_LoadSpeakerScript( void ) {
	char       filename[MAX_QPATH];
	pc_token_t token;
	int        handle;
	int        i;

	CG_SpeakerScriptFilename( filename, sizeof( filename ) );

	handle = trap_PC_LoadSource( filename );
	if ( !handle ) {
		return;
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "speakerScript" ) ) {
		CG_SS_ParseError( handle, filename, "expected 'speakerScript'" );
		return;
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		CG_SS_ParseError( handle, filename, "expected '{'" );
		return;
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "speakerDef" ) ) {
			if ( !CG_SS_ParseSpeaker( handle, filename ) ) {
				return;
			}
		} else {
			CG_SS_ParseError( handle, filename, "unknown token '%s'", token.string );
			return;
		}
	}

	trap_PC_FreeSource( handle );

	for ( i = 0; i < numScriptSpeakers; i++ ) {
		scriptSpeakers[i].noise = trap_S_RegisterSound( scriptSpeakers[i].filename );
	}

	CG_Printf( "...loaded %i speaker%s from '%s'\n", numScriptSpeakers, numScriptSpeakers == 1 ? "" : "s", filename );
}

/*
================
CG_AddScriptSpeakers

Called once per frame. Handles auto-activation timing for one-shot
speakers and dispatches playback for both one-shot and looped speakers.
================
*/
void CG_AddScriptSpeakers( void ) {
	int             i;
	scriptSpeaker_t *speaker;
	qboolean        inPVS;

	for ( i = 0; i < numScriptSpeakers; i++ ) {
		speaker = &scriptSpeakers[i];

		if ( !speaker->noise ) {
			continue;
		}

		inPVS = trap_R_inPVS( cg.refdef.vieworg, speaker->origin );

		if ( speaker->loop == SPKR_NOT_LOOPED ) {
			if ( !speaker->activated ) {
				if ( speaker->wait || speaker->random ) {
					if ( cg.time >= speaker->nextActivateTime ) {
						speaker->activated = qtrue;
					}
				}
			}

			if ( !speaker->activated ) {
				continue;
			}

			speaker->activated = qfalse;
			speaker->nextActivateTime = cg.time + speaker->wait + (int)( speaker->random * crandom() );

			if ( speaker->broadcast == SPKR_LOCAL && !inPVS ) {
				continue;
			}

			if ( speaker->broadcast == SPKR_GLOBAL ) {
				trap_S_StartLocalSound( speaker->noise, CHAN_ITEM );
			} else {
				trap_S_StartSoundVControl( speaker->origin, -1, CHAN_ITEM, speaker->noise, speaker->volume );
			}
		} else {
			// must re-add every frame or the engine stops it (trap_S_ClearLoopingSounds)
			if ( !speaker->activated ) {
				continue;
			}

			if ( speaker->broadcast == SPKR_LOCAL && !inPVS ) {
				continue;
			}

			// SPKR_GLOBAL is never PVS-gated (same as SPKR_NOPVS); no non-positional loop call exists here.
			// entityNum is a negative ambient loop slot (see MAX_AMBIENT_LOOPS in snd_local.h); slot 0 is
			// reserved elsewhere in cgame, so speakers use -(i + 2).
			trap_S_AddLoopingSound( -( i + 2 ), speaker->origin, vec3_origin, speaker->range, speaker->noise, speaker->volume );
		}
	}
}

// Authoring helpers (dumpsound / listsounds / deletesound console commands)

/*
================
CG_AddScriptSpeaker

Appends a speaker to the live list and registers its sound so it starts
playing immediately. Does not touch disk -- call CG_SaveSpeakerScript()
afterward to persist the change.
================
*/
qboolean CG_AddScriptSpeaker( scriptSpeaker_t *speaker ) {
	if ( numScriptSpeakers >= MAX_SCRIPT_SPEAKERS ) {
		CG_Printf( S_COLOR_RED "ERROR CG_AddScriptSpeaker: MAX_SCRIPT_SPEAKERS (%i) reached\n", MAX_SCRIPT_SPEAKERS );
		return qfalse;
	}

	if ( *speaker->filename ) {
		speaker->noise = trap_S_RegisterSound( speaker->filename );
	}

	scriptSpeakers[numScriptSpeakers++] = *speaker;

	return qtrue;
}

/*
================
CG_DeleteScriptSpeaker

Removes a speaker from the live list by index. Does not touch disk -- call
CG_SaveSpeakerScript() afterward to persist the change.
================
*/
qboolean CG_DeleteScriptSpeaker( int index ) {
	if ( index < 0 || index >= numScriptSpeakers ) {
		return qfalse;
	}

	// ranges overlap when removing anything but the last entry -- Com_Memcpy
	// (== memcpy) is not safe here, this needs memmove
	memmove( &scriptSpeakers[index], &scriptSpeakers[index + 1], sizeof( scriptSpeaker_t ) * ( numScriptSpeakers - index - 1 ) );
	numScriptSpeakers--;

	return qtrue;
}

/*
================
CG_SaveSpeakerScript

Rewrites sound/maps/<mapname>.sps from the live in-memory speaker list.
================
*/
qboolean CG_SaveSpeakerScript( void ) {
	static const char *loopedStr[]    = { "no", "on", "off" };
	static const char *broadcastStr[] = { "no", "global", "nopvs" };
	char             filename[MAX_QPATH];
	fileHandle_t     f;
	char             *s;
	int              i;
	scriptSpeaker_t  *speaker;
	char             targetnameStr[64];
	char             waitStr[32];
	char             randomStr[32];
	char             volumeStr[32];
	char             rangeStr[32];

	CG_SpeakerScriptFilename( filename, sizeof( filename ) );

	trap_FS_FOpenFile( filename, &f, FS_WRITE );
	if ( !f ) {
		CG_Printf( S_COLOR_RED "ERROR CG_SaveSpeakerScript: failed to open '%s' for writing\n", filename );
		return qfalse;
	}

	s = "speakerScript\n{\n";
	trap_FS_Write( s, strlen( s ), f );

	for ( i = 0; i < numScriptSpeakers; i++ ) {
		speaker = &scriptSpeakers[i];

		targetnameStr[0] = '\0';
		waitStr[0]        = '\0';
		randomStr[0]      = '\0';
		volumeStr[0]      = '\0';
		rangeStr[0]       = '\0';

		if ( *speaker->targetname ) {
			Com_sprintf( targetnameStr, sizeof( targetnameStr ), "\t\ttargetname \"%s\"\n", speaker->targetname );
		}
		if ( speaker->wait ) {
			Com_sprintf( waitStr, sizeof( waitStr ), "\t\twait %i\n", speaker->wait );
		}
		if ( speaker->random ) {
			Com_sprintf( randomStr, sizeof( randomStr ), "\t\trandom %i\n", speaker->random );
		}
		if ( speaker->volume ) {
			Com_sprintf( volumeStr, sizeof( volumeStr ), "\t\tvolume %i\n", speaker->volume );
		}
		if ( speaker->range ) {
			Com_sprintf( rangeStr, sizeof( rangeStr ), "\t\trange %i\n", speaker->range );
		}

		s = va( "\tspeakerDef\n\t{\n"
				"\t\tnoise \"%s\"\n"
				"\t\torigin %.2f %.2f %.2f\n"
				"%s"
				"\t\tlooped \"%s\"\n"
				"\t\tbroadcast \"%s\"\n"
				"%s%s%s%s"
				"\t}\n",
				speaker->filename,
				(double)speaker->origin[0], (double)speaker->origin[1], (double)speaker->origin[2],
				targetnameStr,
				loopedStr[speaker->loop],
				broadcastStr[speaker->broadcast],
				waitStr, randomStr, volumeStr, rangeStr );
		trap_FS_Write( s, strlen( s ), f );
	}

	s = "}\n";
	trap_FS_Write( s, strlen( s ), f );

	trap_FS_FCloseFile( f );

	CG_Printf( "Saved %i speaker%s to '%s'\n", numScriptSpeakers, numScriptSpeakers == 1 ? "" : "s", filename );

	return qtrue;
}

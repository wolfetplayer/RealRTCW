/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_arb.c -- ARB assembly vertex/fragment program helpers backing the \r_fbo offscreen-rendering effects, since this renderer has no GLSL pipeline; adapted from Quake3e's code/renderer/tr_arb.c

#include "tr_local.h"

static GLuint current_vp = 0;
static GLuint current_fp = 0;


/*
==============
ARB_CompileProgram

Compiles a single ARB assembly vertex or fragment program from source text.
`target` is GL_VERTEX_PROGRAM_ARB or GL_FRAGMENT_PROGRAM_ARB, `program` is a
name previously allocated via qglGenProgramsARB(). Returns qfalse (and logs
the driver's error string) on a compile failure.
==============
*/
qboolean ARB_CompileProgram( GLenum target, const char *text, GLuint program )
{
	GLint errorPos;
	GLenum errCode;

	// drain errors pending from earlier in this init pass so they aren't misattributed below
	while ( qglGetError() != GL_NO_ERROR ) {}

	qglBindProgramARB( target, program );
	qglProgramStringARB( target, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen( text ), text );
	qglGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errorPos );

	if ( ( errCode = qglGetError() ) != GL_NO_ERROR || errorPos != -1 )
	{
		ri.Printf( PRINT_WARNING, "%s program compile error (0x%x, offset %i): %s\n%s\n",
			( target == GL_FRAGMENT_PROGRAM_ARB ) ? "fragment" : "vertex",
			errCode, errorPos, qglGetString( GL_PROGRAM_ERROR_STRING_ARB ), text );
		qglBindProgramARB( target, 0 );
		return qfalse;
	}

	return qtrue;
}


/*
==============
ARB_ProgramEnable

Binds the given vertex/fragment program pair (either may be 0 to leave that
stage's fixed-function pipeline active instead).
==============
*/
void ARB_ProgramEnable( GLuint vp, GLuint fp )
{
	if ( current_vp != vp ) {
		current_vp = vp;
		if ( current_vp ) {
			qglEnable( GL_VERTEX_PROGRAM_ARB );
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, current_vp );
		} else {
			qglDisable( GL_VERTEX_PROGRAM_ARB );
		}
	}

	if ( current_fp != fp ) {
		current_fp = fp;
		if ( current_fp ) {
			qglEnable( GL_FRAGMENT_PROGRAM_ARB );
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, current_fp );
		} else {
			qglDisable( GL_FRAGMENT_PROGRAM_ARB );
		}
	}
}


/*
==============
ARB_ProgramDisable

Returns both program stages to the fixed-function pipeline.
==============
*/
void ARB_ProgramDisable( void )
{
	ARB_ProgramEnable( 0, 0 );
}


/*
==============================================================================

FBO POST-PROCESS EFFECTS

Effects applied while blitting tr.mainFbo to the real backbuffer at the end
of each frame (see RB_SwapBuffers in tr_backend.c). This renderer has no
GLSL pipeline, so these are ARB assembly fragment programs instead.

==============================================================================
*/

typedef enum {
	GAMMA_FRAGMENT,
	THRESHOLD_FRAGMENT,
	BLUR_FRAGMENT,
	NUM_ARB_PROGRAMS
} arbProgramNum;

static GLuint arbPrograms[ NUM_ARB_PROGRAMS ];
static qboolean arbProgramsReady = qfalse;

// applies pow(color, invGamma), the software equivalent of s_gammatable[] since hw gamma is unavailable; program.local[0].x = tr.invGamma
static const char *gammaFP =
	"!!ARBfp1.0\n"
	"TEMP tex;\n"
	"TEX tex, fragment.texcoord[0], texture[0], 2D;\n"
	"POW result.color.x, tex.x, program.local[0].x;\n"
	"POW result.color.y, tex.y, program.local[0].x;\n"
	"POW result.color.z, tex.z, program.local[0].x;\n"
	"MOV result.color.w, tex.w;\n"
	"END\n";

// extracts pixels above program.local[0].x (r_bloom_threshold), scaled by program.local[0].y (r_bloom_intensity); first step of FBO_Bloom()
static const char *thresholdFP =
	"!!ARBfp1.0\n"
	"TEMP tex, luma, bright;\n"
	"TEX tex, fragment.texcoord[0], texture[0], 2D;\n"
	"DP3 luma.x, tex, {0.299, 0.587, 0.114, 0.0};\n"
	"SUB luma.x, luma.x, program.local[0].x;\n"
	"CMP bright, luma.xxxx, {0.0, 0.0, 0.0, 0.0}, tex;\n"
	"MUL result.color, bright, program.local[0].y;\n"
	"END\n";

// 5-tap separable Gaussian blur; program.local[0].xy = per-pass texel step (see FBO_Bloom())
static const char *blurFP =
	"!!ARBfp1.0\n"
	"PARAM step = program.local[0];\n"
	"TEMP tex, sum, coord;\n"
	"MAD coord, step, {-2.0, -2.0, 0.0, 0.0}, fragment.texcoord[0];\n"
	"TEX tex, coord, texture[0], 2D;\n"
	"MUL sum, tex, {0.06136, 0.06136, 0.06136, 0.06136};\n"
	"MAD coord, step, {-1.0, -1.0, 0.0, 0.0}, fragment.texcoord[0];\n"
	"TEX tex, coord, texture[0], 2D;\n"
	"MAD sum, tex, {0.24477, 0.24477, 0.24477, 0.24477}, sum;\n"
	"TEX tex, fragment.texcoord[0], texture[0], 2D;\n"
	"MAD sum, tex, {0.38774, 0.38774, 0.38774, 0.38774}, sum;\n"
	"MAD coord, step, {1.0, 1.0, 0.0, 0.0}, fragment.texcoord[0];\n"
	"TEX tex, coord, texture[0], 2D;\n"
	"MAD sum, tex, {0.24477, 0.24477, 0.24477, 0.24477}, sum;\n"
	"MAD coord, step, {2.0, 2.0, 0.0, 0.0}, fragment.texcoord[0];\n"
	"TEX tex, coord, texture[0], 2D;\n"
	"MAD result.color, tex, {0.06136, 0.06136, 0.06136, 0.06136}, sum;\n"
	"END\n";


/*
==============
FBO_Ortho2D

Sets up an ordinary bottom-left-origin ortho projection matching an FBO's own
texture convention, for FBO-to-FBO draws (unlike RB_SetGL2D's top-left/Y-down
screen space, which is only correct when the destination is the real backbuffer).
==============
*/
static void FBO_Ortho2D( int width, int height )
{
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();
	qglOrtho( 0, width, 0, height, 0, 1 );
	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	GL_Cull( CT_TWO_SIDED );
	qglDisable( GL_CLIP_PLANE0 );
	qglDisable( GL_FOG );
}


/*
==============
RB_FBOQuad

Full-viewport textured quad, texcoords unflipped (see FBO_Ortho2D above).
==============
*/
static void RB_FBOQuad( int width, int height )
{
	qglBegin( GL_QUADS );
		qglTexCoord2f( 0.0f, 0.0f ); qglVertex2f( 0.0f, 0.0f );
		qglTexCoord2f( 1.0f, 0.0f ); qglVertex2f( (float)width, 0.0f );
		qglTexCoord2f( 1.0f, 1.0f ); qglVertex2f( (float)width, (float)height );
		qglTexCoord2f( 0.0f, 1.0f ); qglVertex2f( 0.0f, (float)height );
	qglEnd();
}


/*
==============
GL_ProgramAvailable
==============
*/
qboolean GL_ProgramAvailable( void )
{
	return arbProgramsReady;
}


/*
==============
ARB_InitPrograms

Compiles the ARB programs used by the \r_fbo postprocess pass. Safe to call
whenever glRefConfig.arbPrograms is false (a no-op) -- FBO_PostProcess() falls
back to a plain blit in that case.
==============
*/
void ARB_InitPrograms( void )
{
	arbProgramsReady = qfalse;

	if ( !glRefConfig.arbPrograms ) {
		return;
	}

	qglGenProgramsARB( NUM_ARB_PROGRAMS, arbPrograms );

	if ( !ARB_CompileProgram( GL_FRAGMENT_PROGRAM_ARB, gammaFP, arbPrograms[ GAMMA_FRAGMENT ] ) ||
		 !ARB_CompileProgram( GL_FRAGMENT_PROGRAM_ARB, thresholdFP, arbPrograms[ THRESHOLD_FRAGMENT ] ) ||
		 !ARB_CompileProgram( GL_FRAGMENT_PROGRAM_ARB, blurFP, arbPrograms[ BLUR_FRAGMENT ] ) ) {
		qglDeleteProgramsARB( NUM_ARB_PROGRAMS, arbPrograms );
		Com_Memset( arbPrograms, 0, sizeof( arbPrograms ) );
		return;
	}

	arbProgramsReady = qtrue;
}


/*
==============
ARB_ShutdownPrograms
==============
*/
void ARB_ShutdownPrograms( void )
{
	if ( arbProgramsReady ) {
		qglDeleteProgramsARB( NUM_ARB_PROGRAMS, arbPrograms );
	}
	Com_Memset( arbPrograms, 0, sizeof( arbPrograms ) );
	arbProgramsReady = qfalse;

	// these named programs are gone; don't let a later re-init's recycled IDs match the stale cache
	current_vp = 0;
	current_fp = 0;
}


/*
==============
FBO_PostProcess

Blits tr.mainFbo to the real backbuffer, applying gamma correction along the
way when possible. Falls back to a plain hardware blit (no gamma correction,
matching this renderer's pre-\r_fbo behavior) if the ARB program didn't compile.
==============
*/
void FBO_PostProcess( void )
{
	if ( !arbProgramsReady ) {
		FBO_FastBlit( tr.mainFbo, NULL, GL_COLOR_BUFFER_BIT, GL_NEAREST );
		return;
	}

	FBO_Bind( NULL );

	RB_SetGL2D();

	// override RB_SetGL2D()'s alpha blend so this composite fully replaces the backbuffer
	GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );

	GL_SelectTexture( 0 );
	GL_Bind( (image_t *)tr.mainFbo->colorImage );

	ARB_ProgramEnable( 0, arbPrograms[ GAMMA_FRAGMENT ] );
	qglProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0, tr.invGamma, tr.invGamma, tr.invGamma, 1.0f );

	// flip texcoords vertically: the FBO texture is bottom-left origin, RB_SetGL2D() isn't
	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	qglBegin( GL_QUADS );
		qglTexCoord2f( 0.0f, 1.0f ); qglVertex2f( 0.0f, 0.0f );
		qglTexCoord2f( 1.0f, 1.0f ); qglVertex2f( (float)glConfig.vidWidth, 0.0f );
		qglTexCoord2f( 1.0f, 0.0f ); qglVertex2f( (float)glConfig.vidWidth, (float)glConfig.vidHeight );
		qglTexCoord2f( 0.0f, 0.0f ); qglVertex2f( 0.0f, (float)glConfig.vidHeight );
	qglEnd();

	ARB_ProgramDisable();
}


/*
==============
FBO_Bloom

Extracts bright pixels from the just-rendered 3D scene (tr.mainFbo), blurs them,
and additively composites the glow back into tr.mainFbo -- called from the same
RB_ExecuteRenderCommands trigger points tr_bloom.c's legacy R_BloomScreen() uses
(RC_STRETCH_PIC/RC_STRETCH_PIC_GRADIENT/RC_SWAP_BUFFERS), so bloom
lands before any 2D UI is drawn on top, same as the old implementation.
==============
*/
void FBO_Bloom( void )
{
	FBO_t *src, *dst, *tmp;
	int i;

	if ( !fboEnabled || !arbProgramsReady || !r_bloom->integer ) {
		return;
	}
	if ( backEnd.doneBloom ) {
		return;
	}
	if ( !backEnd.doneSurfaces ) {
		return;
	}
	backEnd.doneBloom = qtrue;

	if ( !tr.bloomFbo[0] || !tr.bloomFbo[1] ) {
		return;
	}

	// 1. threshold-extract bright pixels from the main scene into bloomFbo[0]
	FBO_Bind( tr.bloomFbo[0] );
	qglViewport( 0, 0, tr.bloomFbo[0]->width, tr.bloomFbo[0]->height );
	qglScissor( 0, 0, tr.bloomFbo[0]->width, tr.bloomFbo[0]->height );
	FBO_Ortho2D( tr.bloomFbo[0]->width, tr.bloomFbo[0]->height );
	GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
	GL_SelectTexture( 0 );
	GL_Bind( (image_t *)tr.mainFbo->colorImage );
	ARB_ProgramEnable( 0, arbPrograms[ THRESHOLD_FRAGMENT ] );
	qglProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0,
		r_bloom_threshold->value, r_bloom_intensity->value, 0.0f, 0.0f );
	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	RB_FBOQuad( tr.bloomFbo[0]->width, tr.bloomFbo[0]->height );

	// 2. separable blur, ping-ponging horizontal/vertical passes between the two scratch FBOs
	src = tr.bloomFbo[0];
	dst = tr.bloomFbo[1];
	ARB_ProgramEnable( 0, arbPrograms[ BLUR_FRAGMENT ] );
	for ( i = 0; i < r_bloom_passes->integer * 2; i++ ) {
		FBO_Bind( dst );
		qglViewport( 0, 0, dst->width, dst->height );
		qglScissor( 0, 0, dst->width, dst->height );
		GL_Bind( (image_t *)src->colorImage );
		if ( i & 1 ) {
			qglProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0, 0.0f, 1.0f / dst->height, 0.0f, 0.0f );
		} else {
			qglProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0, 1.0f / dst->width, 0.0f, 0.0f, 0.0f );
		}
		RB_FBOQuad( dst->width, dst->height );

		tmp = src; src = dst; dst = tmp;
	}

	// 3. additively composite the blurred glow back into the main scene, fixed-function
	FBO_Bind( tr.mainFbo );
	qglViewport( 0, 0, tr.mainFbo->width, tr.mainFbo->height );
	qglScissor( 0, 0, tr.mainFbo->width, tr.mainFbo->height );
	FBO_Ortho2D( tr.mainFbo->width, tr.mainFbo->height );
	GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
	GL_Bind( (image_t *)src->colorImage );
	ARB_ProgramDisable();
	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	RB_FBOQuad( tr.mainFbo->width, tr.mainFbo->height );

	// restore standard top-left/Y-down 2D screen state for whatever UI draw triggered us
	RB_SetGL2D();
}

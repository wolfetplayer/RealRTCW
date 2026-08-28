/*
===========================================================================
Copyright (C) 2011 James Canete (use.less01@gmail.com)
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
// tr_extensions.c -- detects/loads the GL_ARB_framebuffer_object and GL_ARB_vertex/fragment_program extensions sdl_glimp.c doesn't load on its own; adapted from code/rend2/tr_extensions.c

#ifdef USE_LOCAL_HEADERS
#	include "SDL3/SDL.h"
#else
#	include <SDL3/SDL.h>
#endif

#include "tr_local.h"

glRefConfig_t glRefConfig;

void GLimp_InitExtraExtensions( void )
{
	const char *extension;
	const char *result[3] = { "...ignoring %s\n", "...using %s\n", "...%s not found\n" };
	qboolean q_gl_version_at_least_3_0;
	qboolean glExtFuncMissing;

	q_gl_version_at_least_3_0 = QGL_VERSION_ATLEAST( 3, 0 );

	Com_Memset( &glRefConfig, 0, sizeof( glRefConfig ) );

	// OpenGL 3.0 - GL_ARB_framebuffer_object
	extension = "GL_ARB_framebuffer_object";
	if ( q_gl_version_at_least_3_0 || SDL_GL_ExtensionSupported( extension ) )
	{
		qglGetIntegerv( GL_MAX_RENDERBUFFER_SIZE, &glRefConfig.maxRenderbufferSize );
		qglGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &glRefConfig.maxColorAttachments );

		glExtFuncMissing = qfalse;
#define GLE(ret, name, ...) qgl##name = (name##proc *) SDL_GL_GetProcAddress("gl" #name); if (!qgl##name) { ri.Printf(PRINT_WARNING, "WARNING: OpenGL function 'gl%s' not found\n", #name); glExtFuncMissing = qtrue; }
		QGL_ARB_framebuffer_object_PROCS;
#undef GLE

		glRefConfig.framebufferObject = !glExtFuncMissing;
		glRefConfig.framebufferBlit = glRefConfig.framebufferObject;
		glRefConfig.framebufferMultisample = glRefConfig.framebufferObject;

		ri.Printf( PRINT_ALL, result[ glRefConfig.framebufferObject ], extension );
	}
	else
	{
		ri.Printf( PRINT_ALL, result[2], extension );
	}

	// both share the same entry points and every effect here needs both, so track as one capability
	extension = "GL_ARB_vertex_program / GL_ARB_fragment_program";
	if ( SDL_GL_ExtensionSupported( "GL_ARB_vertex_program" ) && SDL_GL_ExtensionSupported( "GL_ARB_fragment_program" ) )
	{
		glExtFuncMissing = qfalse;
#define GLE(ret, name, ...) qgl##name = (name##proc *) SDL_GL_GetProcAddress("gl" #name); if (!qgl##name) { ri.Printf(PRINT_WARNING, "WARNING: OpenGL function 'gl%s' not found\n", #name); glExtFuncMissing = qtrue; }
		QGL_ARB_vertex_fragment_program_PROCS;
#undef GLE

		glRefConfig.arbPrograms = !glExtFuncMissing;
		ri.Printf( PRINT_ALL, result[ glRefConfig.arbPrograms ], extension );
	}
	else
	{
		ri.Printf( PRINT_ALL, result[2], extension );
	}
}

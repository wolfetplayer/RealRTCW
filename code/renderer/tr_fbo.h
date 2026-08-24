/*
===========================================================================
Copyright (C) 2006 Kirk Barnes
Copyright (C) 2006-2008 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_fbo.h -- minimal framebuffer-object core for the \r_fbo offscreen-rendering pipeline, trimmed down from code/rend2/tr_fbo.c's generic FBO_t

#ifndef __TR_FBO_H__
#define __TR_FBO_H__

struct image_s;

typedef struct FBO_s {
	char     name[MAX_QPATH];

	GLuint   frameBuffer;

	struct image_s *colorImage;    // texture attachment
	GLuint   depthBuffer;          // combined depth/stencil renderbuffer, 0 if none

	int      width;
	int      height;
} FBO_t;

extern qboolean fboEnabled;    // r_fbo->integer && glRefConfig.framebufferObject, set by FBO_Init()

FBO_t    *FBO_Create( const char *name, int width, int height );
void      FBO_AttachImage( FBO_t *fbo, struct image_s *image, GLenum attachment );
void      FBO_CreateDepthBuffer( FBO_t *fbo, GLenum format );
qboolean  R_CheckFBO( const FBO_t *fbo );
void      FBO_Bind( FBO_t *fbo );    // NULL binds the real backbuffer
void      FBO_FastBlit( const FBO_t *src, const FBO_t *dst, GLbitfield buffers, GLenum filter );

void      FBO_Init( void );
void      FBO_Shutdown( void );
void      R_FBOList_f( void );

#endif // __TR_FBO_H__

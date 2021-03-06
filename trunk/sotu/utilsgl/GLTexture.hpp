// Description:
//   Wrapper for a GL texture in conjunction with SDL.
//
// Copyright (C) 2001 Frank Becker
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation;  either version 2 of the License,  or (at your option) any  later
// version.
//
// This program is distributed in the hope that it will be useful,  but  WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details
//
#ifndef _GLTexture_hpp_
#define _GLTexture_hpp_

#include <utilsgl/gl++.hpp>
#include <SDL/SDL_image.h>

class GLTextureI
{
public:
    virtual void bind( void) = 0;
    virtual void unbind( void) = 0;
    virtual void reload( void) = 0;

    virtual ~GLTextureI() {}
};

class GLTexture : public GLTextureI
{
public:
    //target: GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_ARB, etc.
    GLTexture( GLenum target, SDL_Surface *img, bool mipmap = false);
    GLTexture( GLenum target, const char *fileName, bool mipmap = false);

    ~GLTexture();

    void bind( void){ glBindTexture( _target, _textureID);}
    void unbind( void){ glBindTexture( _target, 0);}
    void reload( void);

    void setEnvMode( GLint mode)
    {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
    }

private:
    GLuint _textureID;
    GLenum _target;
    SDL_Surface *_image;
    bool _mipmap;

    GLenum getGLTextureFormat( void);
    void init( SDL_Surface *img, bool mipmap);
};

#endif

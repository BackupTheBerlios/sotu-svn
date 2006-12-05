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
#include <Trace.hpp>
#include <TextureManager.hpp>
#include <GLTexture.hpp>

//Construct texture given filename
GLTexture::GLTexture( GLenum target, const char *fileName, bool mipmap):
	_target(target)
{
    XTRACE();
    SDL_Surface *image;
    IMG_InvertAlpha(1);
    if( (image = IMG_Load( fileName)) == 0)
    {
	fprintf(stderr,"Couldn't load %s: %s\n", fileName, SDL_GetError());
        _textureID = 0;
        _image = 0;
    }
    else
    {
        init( image, mipmap);
    }
}

//Construct texture given SDL surface
GLTexture::GLTexture( GLenum target, SDL_Surface *img, bool mipmap):
	_target(target)
{
    XTRACE();
    init( img, mipmap);
}

GLTexture::~GLTexture()
{
    XTRACE();
    TextureManagerS::instance()->removeTexture( this);
    if( _image) SDL_FreeSurface( _image);
}

void GLTexture::reload( void)
{
    TextureManagerS::instance()->removeTexture( this);
    init( _image, _mipmap);
}

//Init texture with SDL surface
void GLTexture::init( SDL_Surface *img, bool mipmap)
{
    _image = img;
    _mipmap = mipmap;
    _textureID = TextureManagerS::instance()->addTexture( this);

    bind();
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri( _target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    if( mipmap)
    {
#if 0
        glTexParameteri(_target,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
        __gluBuild2DMipmaps( _target, GL_RGBA8, _image->w, _image->h, 
            getGLTextureFormat(), GL_UNSIGNED_BYTE, _image->pixels);
#else
	LOG_ERROR << "midmap needs work\n";
#endif
    }
    else
    {
        glTexParameteri( _target,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D( _target, 0, GL_RGBA8, _image->w, _image->h, 0, 
            getGLTextureFormat(), GL_UNSIGNED_BYTE, _image->pixels);
    }
}

//get texture format
GLenum GLTexture::getGLTextureFormat( void)
{
    int texFormat = GL_RGB;

    if( _image->flags & (SDL_SRCALPHA | SDL_SRCCOLORKEY))
    {
        texFormat = GL_RGBA; 
    }

    return texFormat;
}

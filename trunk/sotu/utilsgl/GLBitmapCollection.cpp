// Description:
//   A Bitmap Collection contains many bitmaps in a single PNG file. 
//   Bitmap names, offsets, sizes, etc. are kept in a seperate data file.
//   Includes GL drawing methods.
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
#include <iostream>

#include <SDL/SDL_image.h>
#include <zrwops.hpp>

#include <Trace.hpp>
#include <GLBitmapCollection.hpp>
#include <FindHash.hpp>
#include <ResourceManager.hpp>

//Load bitmap and data file
bool GLBitmapCollection::Load( const char *bitmapFile, const char *dataFile)
{
    XTRACE();

    IMG_InvertAlpha(1);
    if( !ResourceManagerS::instance()->selectResource( string(bitmapFile)))
    {
        LOG_WARNING << "Bitmap file [" << bitmapFile << "] not found." << endl;
        return false;
    }
    ziStream &bminfile = ResourceManagerS::instance()->getInputStream();
    SDL_RWops *src = RWops_from_ziStream( bminfile);
    SDL_Surface *img = IMG_LoadPNG_RW( src);
    if( !img)
    { 
	LOG_ERROR << "Failed to load image: [" << bitmapFile << "]" << endl;
        return false;
    }
    SDL_RWclose( src);
    LOG_DEBUG << "Bitmap loaded." << endl;

    //assuming texture is square
    _textureSize = (float)img->w;

    if( !ResourceManagerS::instance()->selectResource( string(dataFile)))
    {
        LOG_WARNING << "Bitmap data file [" << dataFile 
                    << "] not found." << endl;
        SDL_FreeSurface( img);
        return false;
    }
    ziStream &datainfile = ResourceManagerS::instance()->getInputStream();

    LOG_DEBUG << "Reading: [" << dataFile << "]." << endl;
    _bitmapCount = 0;
    bool fileRead = false;
    while( !fileRead)
    {
        BitmapInfo tmpBInfo;
        string line;
        if( getline( datainfile, line).eof()) break;

        int n = sscanf( line.c_str(), "%d %d %d %d %d %d [%[\t-~]31s]\n",
            &tmpBInfo.xoff,
            &tmpBInfo.yoff,
            &tmpBInfo.xpos,
            &tmpBInfo.ypos,
	    &tmpBInfo.width,
	    &tmpBInfo.height,
	    tmpBInfo.name);
        if( tmpBInfo.name[ strlen(tmpBInfo.name)-1] == ']')
        {
            tmpBInfo.name[ strlen(tmpBInfo.name)-1] = '\0';
        }
	LOG_DEBUG << "Read: [" << tmpBInfo.name << "] " << n << endl;
        if( n == 6)
        {
            //assume missing element is a space
            strcpy( tmpBInfo.name, " ");
            n++;
        }
	if( n != 7)
        {
	    LOG_WARNING << "Data file [" << dataFile 
                        << "] has incorrect format." << endl;
	    break;
        }
	else
	{
	    _bitmapInfo[ _bitmapCount] = tmpBInfo;
            string tmpName = tmpBInfo.name;
	    LOG_DEBUG << "New bitmap: [" << tmpName << "] " << endl;
            _bitmapInfoMap[ tmpName] = &_bitmapInfo[ _bitmapCount];    
	    _bitmapCount++;
	}

        if( _bitmapCount > MAX_BITMAPS)
        {
	    LOG_ERROR << "Maximum number (" << MAX_BITMAPS 
                      << ") of bitmaps per collection exceeded." << endl;
	    fileRead = true;
        }
    } 

    _bitmapCollection = new GLTexture( GL_TEXTURE_2D, img, false);
    LOG_DEBUG << "Bitmap read OK." << endl;

    return true;
}

//Get index of bitmap with the given name
int GLBitmapCollection::getIndex( const string & name)
{
    XTRACE();
    for( unsigned int i=0; i<_bitmapCount; i++)
    {
        if( name == _bitmapInfo[ i].name) return (int)i;
    }

    return -1;
}

//Draw using bitmap index
void GLBitmapCollection::Draw( 
    const unsigned int index, 
    const float &x, 
    const float &y, 
    const float &scalex, 
    const float &scaley,
    const float &z)
{
    if( index >= _bitmapCount) return;

    _Draw( _bitmapInfo[ index], x, y, z, scalex, scaley);
}

//Draw using bitmap name
void GLBitmapCollection::Draw( 
    const string &name, 
    const float &x, 
    const float &y, 
    const float &scalex, 
    const float &scaley,
    const float &z)
{
//    cout << "Drawing bitmap " << name << endl;
    BitmapInfo *bmInfo = findHash( name, _bitmapInfoMap);

    if( !bmInfo)
    {   
        cerr << "Unable to fill GLBitmapCollection::Draw request for "
             << name << endl;
        return;
    }

    _Draw( *bmInfo, x, y, z, scalex, scaley);
}

//Draw using bitmap info
void GLBitmapCollection::_Draw( 
    const BitmapInfo &bitmapInfo, 
    const float &x, 
    const float &y, 
    const float &z, 
    const float &scalex, 
    const float &scaley)
{
    XTRACE();

    float tx,ty, fxsize, fysize, ay;
    float dxsize, dysize;

    dxsize = (float)bitmapInfo.width*scalex;
    dysize = (float)bitmapInfo.height*scaley;
    ay = (float)(bitmapInfo.height-bitmapInfo.yoff)*scaley;
    fxsize = (float)bitmapInfo.width / _textureSize;
    fysize = (float)bitmapInfo.height / _textureSize;
    tx = (float)bitmapInfo.xpos / _textureSize;
    ty = (float)bitmapInfo.ypos / _textureSize;

    glBegin(GL_QUADS);
    glTexCoord2f (tx       ,ty       ); glVertex3f(x       ,y+ay       ,z);
    glTexCoord2f (tx+fxsize,ty       ); glVertex3f(x+dxsize,y+ay       ,z);
    glTexCoord2f (tx+fxsize,ty+fysize); glVertex3f(x+dxsize,y+ay-dysize,z);
    glTexCoord2f (tx       ,ty+fysize); glVertex3f(x       ,y+ay-dysize,z);
    glEnd();
}

//Draw centered using bitmap index
void GLBitmapCollection::DrawC( 
    const unsigned int index, 
    const float &x, 
    const float &y, 
    const float &scalex, 
    const float &scaley,
    const float &z)
{
    if( index >= _bitmapCount) return;

    _DrawC( _bitmapInfo[ index], x, y, z, scalex, scaley);
}

//Draw centered using bitmap name
void GLBitmapCollection::DrawC( 
    const string &name, 
    const float &x, 
    const float &y, 
    const float &scalex, 
    const float &scaley,
    const float &z)
{
//    cout << "Drawing bitmap " << name << endl;
    BitmapInfo *bmInfo = findHash( name, _bitmapInfoMap);

    if( !bmInfo)
    {   
        cerr << "Unable to fill GLBitmapCollection::Draw request for "
             << name << endl;
        return;
    }

    _DrawC( *bmInfo, x, y, z, scalex, scaley);
}

//Draw centered using bitmap info
void GLBitmapCollection::_DrawC( 
    const BitmapInfo &bitmapInfo, 
    const float &x, 
    const float &y, 
    const float &z, 
    const float &scalex, 
    const float &scaley)
{
    XTRACE();

    float tx,ty, fxsize, fysize;
    float dxsize, dysize;

    dxsize = (float)bitmapInfo.width*scalex*0.5f;
    dysize = (float)bitmapInfo.height*scaley*0.5f;
    fxsize = (float)bitmapInfo.width / _textureSize;
    fysize = (float)bitmapInfo.height / _textureSize;
    tx = (float)bitmapInfo.xpos / _textureSize;
    ty = (float)bitmapInfo.ypos / _textureSize;

    glBegin(GL_QUADS);
    glTexCoord2f (tx       ,ty       ); glVertex3f(x-dxsize,y+dysize,z);
    glTexCoord2f (tx+fxsize,ty       ); glVertex3f(x+dxsize,y+dysize,z);
    glTexCoord2f (tx+fxsize,ty+fysize); glVertex3f(x+dxsize,y-dysize,z);
    glTexCoord2f (tx       ,ty+fysize); glVertex3f(x-dxsize,y-dysize,z);
    glEnd();
}

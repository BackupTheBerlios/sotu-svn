// Description:
//   Just like a Bitmap Collection but tailored for a bitmap font.
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
#include <stdio.h>
#include <Trace.hpp>
#include <GLBitmapFont.hpp>

//Draw a string at (x,y) scaled by [scalex,scaley]
void GLBitmapFont::DrawString( 
    const char *s, float x, float y, float scalex, float scaley)
{
    XTRACE();

    unsigned char c;
    int i, l;

    l=(int)strlen(s);
    glEnable(GL_TEXTURE_2D);
    _bitmapCollection->bind();

    glBegin(GL_QUADS);
    for (i=0; i<l; i++) 
    {
        float dxsize, dysize;
        c=s[i];
	if( _charInfo[ c] == ((unsigned int)~0))
	{
	    //no bitmap for char
	    continue;
	}
        BitmapInfo &charInfo = _bitmapInfo[ _charInfo[ c]];

        if(c==32) 
        {
           dxsize = (float)charInfo.width*scalex;
        }
        else if(c=='\t') 
        {
           int tabwidth = (int)((float)
               (_bitmapInfo[ _charInfo[32]].width*8)*scalex);
           int xpos = (int)x;
           int trunc = xpos-(xpos%tabwidth);
           x = (float)(trunc+tabwidth);
           continue;
        }
        else
        { // don't do space
            float tx,ty, fxsize, fysize, ay;

	    dxsize = (float)charInfo.width*scalex;
	    dysize = (float)charInfo.height*scaley;
            ay = (float)(_totalHeight-charInfo.yoff)*scaley;
	    fxsize = (float)charInfo.width / _textureSize;
	    fysize = (float)charInfo.height / _textureSize;
	    tx = (float)charInfo.xpos / _textureSize;
	    ty = (float)charInfo.ypos / _textureSize;

            glTexCoord2f (tx       ,ty);       glVertex2f(x       ,y+ay);
            glTexCoord2f (tx+fxsize,ty);       glVertex2f(x+dxsize,y+ay);
            glTexCoord2f (tx+fxsize,ty+fysize);glVertex2f(x+dxsize,y+ay-dysize);
            glTexCoord2f (tx       ,ty+fysize);glVertex2f(x       ,y+ay-dysize);
        }
        x+= dxsize; //charInfo.width;
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

//Determine width of string (doesn't take TABs into account, yet!)
float GLBitmapFont::GetWidth( const char *s, float scalex)
{
    XTRACE();
    int width = 0;
    size_t l=strlen(s);
    for( size_t i=0; i<l; i++) 
    {
	//FIXME: handle TABs
	width += _bitmapInfo[ _charInfo[ (int)s[ i]]].width;
    }
    return( (float)width * scalex);
}

float GLBitmapFont::GetHeight( float scaley)
{
    XTRACE();
    return (float)_totalHeight*scaley;
}

//Load bitmap and data files for font
bool GLBitmapFont::Load( const char *bitmapFile, const char *dataFile)
{
    XTRACE();
    bool result = GLBitmapCollection::Load( bitmapFile, dataFile);

    if( !result)
    {
	LOG_ERROR << "Unable to load font..." << endl;
    }

    _totalHeight = 0;
    for( unsigned int i=0; i<_bitmapCount; i++)
    {
	//calc _totalHeight
        int height = _bitmapInfo[ i].yoff + _bitmapInfo[ i].height;

        if( height>_totalHeight)
        {
            _totalHeight = height;
        }

	//provide mapping for char -> _bitmapInfo index
        _charInfo[ (int)_bitmapInfo[ i].name[0]] = i; 
    }
    //add a little extra space...
    _totalHeight += 2;

    return result;
}

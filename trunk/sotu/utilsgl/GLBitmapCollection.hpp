// Description:
//   A Bitmap Collection contains many bitmaps in a single PNG file. Bitmap names,
//   offsets, sizes, etc. are kept in a seperate data file.
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
#ifndef _GLBitmapCollection_hpp_
#define _GLBitmapCollection_hpp_

#include <GLTexture.hpp>
#include <HashString.hpp>

const unsigned int MAX_BITMAPS = 128;

class GLBitmapCollection
{

public:
    struct BitmapInfo
    {
	int xoff;   // offset from left top 
	int yoff;   // offset from left top
	int xpos;   // position within texture
	int ypos;   // position within texture
	int width;  // width of bitmap
	int height; // height of bitmap
        char name[32];
    };

    GLBitmapCollection( void):
        _bitmapCollection(0),
        _bitmapCount(0),
        _textureSize(0.0)
    {
    }

    virtual ~GLBitmapCollection()
    {
        delete _bitmapCollection;
    }

    //Load bitmap and data file
    virtual bool Load( const char *bitmapFile, const char *dataFile);

    //Get index of bitmap with the given name
    int getIndex( const string & name);
    //Bind texture
    void bind( void){ _bitmapCollection->bind();}

    //Draw using bitmap index
    void Draw( 
        unsigned int index, 
        const float &x, 
        const float &y, 
        const float &scalex, 
        const float &scaley,
        const float &z = 0.0);

    //Draw using bitmap name
    void Draw( 
        const string &name, 
        const float &x, 
        const float &y, 
        const float &scalex, 
        const float &scaley,
        const float &z = 0.0);

    //Draw centred using bitmap name
    void DrawC( 
        const string &name, 
        const float &x, 
        const float &y, 
        const float &scalex, 
        const float &scaley,
        const float &z = 0.0);

    //Draw centred using bitmap index
    void DrawC( 
        unsigned int index, 
        const float &x, 
        const float &y, 
        const float &scalex, 
        const float &scaley,
        const float &z = 0.0);

    int getWidth( unsigned int index)
    {
//	if( index >= _bitmapCount) return 0;
        return _bitmapInfo[ index].width;
    }
    int getHeight( unsigned int index)
    {
//	if( index >= _bitmapCount) return 0;
        return _bitmapInfo[ index].height;
    }

    void reload( void)
    {
        _bitmapCollection->reload();
    }

protected:
    //Draw using bitmap info
    inline void _Draw( 
        const BitmapInfo &bmInfo, 
        const float &x, 
        const float &y, 
        const float &z, 
        const float &scalex, 
        const float &scaley);

    //Draw centred using bitmap info
    inline void _DrawC( 
        const BitmapInfo &bmInfo, 
        const float &x, 
        const float &y, 
        const float &z, 
        const float &scalex, 
        const float &scaley);

    GLTexture *_bitmapCollection;
    BitmapInfo _bitmapInfo[ MAX_BITMAPS];
    unsigned int _bitmapCount;
    float _textureSize;

    hash_map< const string, BitmapInfo*, hash<const string>, equal_to<const string> > _bitmapInfoMap;

private:
    GLBitmapCollection( const GLBitmapCollection&);
    GLBitmapCollection &operator=(const GLBitmapCollection&);
};

#endif

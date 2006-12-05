// Description:
//   Simple 3D Model loader.
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
#ifndef _Model_hpp_
#define _Model_hpp_

#include <zStream.hpp>
#include <string>

#include <gl++.hpp>

#include <Point.hpp>

struct FaceInfo
{
    int  v1;
    int  v2;
    int  v3;
    int  v4;
    int  color;
    bool smooth;
};

class Model
{
public:
    Model( void);
    ~Model();

    //Load model from file
    bool load( const char *filename);
    //go draw
    void draw( void);
    //re-load model (e.g. after toggling fullscreen).
    void reload( void);

    //name of model
    const std::string getName( void)
    {
        return _name;
    }

    void getBoundingBox( vec3 &min, vec3 &max)
    {
	min = _min;
	max = _max;
    }

    const vec3 & getOffset( void)
    {
	return _offset;
    }

private:
    Model( const Model&);
    Model &operator=(const Model&);

    void compile( void);

    void verifyAndAssign( const int newVert, int & currVertVal);

    bool readColors( ziStream &infile, int &linecount);
    bool readFaces( ziStream &infile, int &linecount);
    bool readNormals( ziStream &infile, int &linecount);
    bool readVertices( ziStream &infile, const vec3 &scale, int &linecount);

    std::string _name;

    int _numVerts;
    int _numColors;
    int _numFaces;

    vec3 *_verts;
    vec3 *_norms;
    vec3 *_colors;
    FaceInfo *_faces;

    vec3 _min;
    vec3 _max;
    vec3 _offset;

    GLuint _compiledList;
};

#endif

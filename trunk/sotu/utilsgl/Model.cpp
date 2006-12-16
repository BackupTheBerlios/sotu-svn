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
#include <Trace.hpp>
#include <Tokenizer.hpp>

#include <Model.hpp>
#include <ResourceManager.hpp>
//------------------------------------------------------------------------------
Model::Model( void):
    _numVerts(0),
    _numColors(0),
    _numFaces(0),
    _verts(0),
    _norms(0),
    _colors(0),
    _faces(0),
    _offset(0,0,0),
    _compiledList(0)
{
    XTRACE();
}
//------------------------------------------------------------------------------
Model::~Model()
{
    XTRACE();
    delete[] _verts;
    delete[] _norms;
    delete[] _colors;
    delete[] _faces;
}
//------------------------------------------------------------------------------
//Load model from file
bool Model::load( const char *filename)
{
    XTRACE();

    if( ! ResourceManagerS::instance()->selectResource( string(filename)))
    {
        LOG_ERROR << "Unable to open: [" << filename << "]" << endl;
        return false;
    }
    ziStream &infile = ResourceManagerS::instance()->getInputStream();

    LOG_INFO << "  Model " << filename << endl;

    vec3 scale;
    scale.x=scale.y=scale.z = 1.0;

    string line;
    int linecount = 0;
    while( !getline( infile, line).eof())
    {
        linecount++;

        //explicitly skip comments
        if( line[0] == '#') continue;
        Tokenizer  t( line);
        string token = t.next();
        if( token == "Name")
        {
            _name = t.next();
            //            LOG_INFO << "Name = [" << _name << "]" << endl;
        }
        else if( token == "Scale")
        {
            scale.x = (float)atof( t.next().c_str());
            scale.y = (float)atof( t.next().c_str());
            scale.z = (float)atof( t.next().c_str());
#if 0
            LOG_INFO << "Scale = ["
                     << scale.x << ","
                     << scale.y << ","
                     << scale.z
                     << "]" << endl;
#endif
        }
        else if( token == "Colors")
        {
            _numColors = atoi( t.next().c_str());
            _colors = new vec3[ _numColors];
            if( !readColors( infile, linecount))
            {
                LOG_ERROR << "Error reading colors:"  << " line:" << linecount << endl;
                return false;
            }
        }
        else if( token == "Vertices")
        {
            int numVerts = atoi( t.next().c_str());
            verifyAndAssign( numVerts, _numVerts);
            _verts = new vec3[ _numVerts];
            if( !readVertices( infile, scale, linecount))
            {
                LOG_ERROR << "Error reading vertices:" << " line:" << linecount << endl;
                return false;
            }
        }
        else if( token == "Normals")
        {
            int numNorms = atoi( t.next().c_str());
            verifyAndAssign( numNorms, _numVerts);
            _norms = new vec3[ _numVerts];
            if( !readNormals( infile, linecount))
            {
                LOG_ERROR << "Error reading normals:" << " line:" << linecount << endl;
                return false;
            }
        }
        else if( token == "Faces")
        {
            _numFaces = atoi( t.next().c_str());
            _faces = new FaceInfo[ _numFaces];
            if( !readFaces( infile, linecount))
            {
                LOG_ERROR << "Error reading faces:"
                      << " line:" << linecount << endl;
                return false;
            }
        }
        else if( token == "Offset")
        {
            _offset.x = (float)atof( t.next().c_str());
            _offset.y = (float)atof( t.next().c_str());
            _offset.z = (float)atof( t.next().c_str());
#if 0
            LOG_INFO << "Offset = ["
                     << _offset.x << ","
                     << _offset.y << ","
                     << _offset.z
                     << "]" << endl;
#endif
        }
        else
        {
            LOG_ERROR << "Syntax error: ["
                      << token << "] line:" << linecount << endl;
            return false;
        }
    }

    compile();
    return true;
}
//------------------------------------------------------------------------------
//read vertices section
bool Model::readVertices( ziStream &infile, const vec3 &scale, int &linecount)
{
    XTRACE();
    string line;
    for( int i=0; i<_numVerts; i++)
    {
        if( getline( infile, line).eof())
        {
            return false;
        }
        linecount++;

        Tokenizer t( line);

        _verts[i].x = (float)atof( t.next().c_str()) * scale.x;
        _verts[i].y = (float)atof( t.next().c_str()) * scale.y;
        _verts[i].z = (float)atof( t.next().c_str()) * scale.z;

        if( t.tokensReturned() != 3)
            return false;

        if( i==0)
        {
            _min = _verts[i];
            _max = _verts[i];
        }
        else
        {
            if( _verts[i].x < _min.x) _min.x = _verts[i].x;
            if( _verts[i].y < _min.y) _min.y = _verts[i].y;
            if( _verts[i].z < _min.z) _min.z = _verts[i].z;

            if( _verts[i].x > _max.x) _max.x = _verts[i].x;
            if( _verts[i].y > _max.y) _max.y = _verts[i].y;
            if( _verts[i].z > _max.z) _max.z = _verts[i].z;
        }

#if 0
        LOG_INFO << "["
             << _verts[i].v[0] << ","
             << _verts[i].v[1] << ","
             << _verts[i].v[2]
             << "]" << endl;
#endif
    }
    return true;
}
//------------------------------------------------------------------------------
//read normals section
bool Model::readNormals( ziStream &infile, int &linecount)
{
    XTRACE();
    string line;
    for( int i=0; i<_numVerts; i++)
    {
        if( getline( infile, line).eof())
        {
            return false;
        }
        linecount++;

        Tokenizer t( line);

        _norms[i].x = (float)atof( t.next().c_str());
        _norms[i].y = (float)atof( t.next().c_str());
        _norms[i].z = (float)atof( t.next().c_str());

        if( t.tokensReturned() != 3) return false;
    }

    return true;
}
//------------------------------------------------------------------------------
//read faces section
bool Model::readFaces( ziStream &infile, int &linecount)
{
    XTRACE();
    string line;
    for( int i=0; i<_numFaces; i++)
    {
        if( getline( infile, line).eof())
        {
            return false;
        }
        linecount++;

        Tokenizer t( line);

        _faces[i].v1 = atoi( t.next().c_str());
        _faces[i].v2 = atoi( t.next().c_str());
        _faces[i].v3 = atoi( t.next().c_str());
        _faces[i].v4 = atoi( t.next().c_str());
        _faces[i].smooth = (atoi( t.next().c_str()) == 1);
        _faces[i].color = atoi( t.next().c_str());

        if( t.tokensReturned() != 6) return false;
    }

    return true;
}
//------------------------------------------------------------------------------
//read colors section
bool Model::readColors( ziStream &infile, int &linecount)
{
    XTRACE();
    string line;
    for( int i=0; i<_numColors; i++)
    {
        if( getline( infile, line).eof())
        {
            return false;
        }
        linecount++;

        Tokenizer t( line);

        _colors[i].x = (float)atof( t.next().c_str());
        _colors[i].y = (float)atof( t.next().c_str());
        _colors[i].z = (float)atof( t.next().c_str());

        if( t.tokensReturned() != 3) return false;
    }

    return true;
}
//------------------------------------------------------------------------------
//hum
void Model::verifyAndAssign( const int newVert, int & currVertVal)
{
    XTRACE();
    if( (currVertVal != 0) && (newVert!=currVertVal))
    {
        LOG_ERROR << "Vertex count inconsistency!" << endl;
    }
    else
    {
        currVertVal = newVert;
    }
}
//------------------------------------------------------------------------------
//go draw
void Model::draw( void)
{
    glCallList( _compiledList);
}
//------------------------------------------------------------------------------
//re-compile model
void Model::reload( void)
{
    XTRACE();
//    glDeleteLists( _compiledList, 1);
    compile();
}
//------------------------------------------------------------------------------
//compile model
void Model::compile( void)
{
    XTRACE();

    _compiledList =  glGenLists( 1);
    glNewList( _compiledList, GL_COMPILE);

    //    if( _numColors) glColor4f( 1.0, 1.0, 1.0, 1.0);

    glBegin(GL_TRIANGLES);
    for( int i=0; i<_numFaces; i++)
    {
        if( _faces[i].v4 != 0) continue; //it's a quad

        if( _numColors) glColor3fv( _colors[ _faces[i].color].v);

        if( !_faces[i].smooth)
        {
            vec3 avgNormal;
            avgNormal.x = _norms[ _faces[i].v1].x +
                          _norms[ _faces[i].v2].x +
                          _norms[ _faces[i].v3].x;
            avgNormal.y = _norms[ _faces[i].v1].y +
                          _norms[ _faces[i].v2].y +
                          _norms[ _faces[i].v3].y;
            avgNormal.z = _norms[ _faces[i].v1].z +
                          _norms[ _faces[i].v2].z +
                          _norms[ _faces[i].v3].z;

            float dlen = (float)(1.0/sqrt( avgNormal.x*avgNormal.x + avgNormal.y*avgNormal.y + avgNormal.z*avgNormal.z));
            avgNormal.x *= dlen;
            avgNormal.y *= dlen;
            avgNormal.z *= dlen;

            glNormal3fv( avgNormal.v);
        }
        else
        {
            glNormal3fv( _norms[ _faces[i].v1].v);
        }
        glVertex3fv( _verts[ _faces[i].v1].v);

/*
LOG_ERROR << "V1: "
          << _verts[ _faces[i].v1].v[0] << ", "
          << _verts[ _faces[i].v1].v[1] << ", "
          << _verts[ _faces[i].v1].v[2]
          << endl;

LOG_ERROR << "V2: "
          << _verts[ _faces[i].v1].x << ", "
          << _verts[ _faces[i].v1].y << ", "
          << _verts[ _faces[i].v1].z
          << endl;
*/

        if( _faces[i].smooth) glNormal3fv( _norms[ _faces[i].v2].v);
        glVertex3fv( _verts[ _faces[i].v2].v);

        if( _faces[i].smooth) glNormal3fv( _norms[ _faces[i].v3].v);
        glVertex3fv( _verts[ _faces[i].v3].v);
    }
    glEnd();

    glBegin(GL_QUADS);
    for( int i=0; i<_numFaces; i++)
    {
        if( _faces[i].v4 == 0) continue; //it's a triangle

        if( _numColors) glColor3fv( _colors[ _faces[i].color].v);

        if( !_faces[i].smooth)
        {
            vec3 avgNormal;
            avgNormal.x = _norms[ _faces[i].v1].x +
                          _norms[ _faces[i].v2].x +
                          _norms[ _faces[i].v3].x +
                          _norms[ _faces[i].v4].x;
            avgNormal.y = _norms[ _faces[i].v1].y +
                          _norms[ _faces[i].v2].y +
                          _norms[ _faces[i].v3].y +
                          _norms[ _faces[i].v4].y;
            avgNormal.z = _norms[ _faces[i].v1].z +
                          _norms[ _faces[i].v2].z +
                          _norms[ _faces[i].v3].z +
                          _norms[ _faces[i].v4].z;

            float dlen = (float)(1.0/sqrt( avgNormal.x*avgNormal.x + avgNormal.y*avgNormal.y + avgNormal.z*avgNormal.z));
            avgNormal.x *= dlen;
            avgNormal.y *= dlen;
            avgNormal.z *= dlen;

            glNormal3fv( avgNormal.v);
        }
        else
        {
            glNormal3fv( _norms[ _faces[i].v1].v);
        }
        glVertex3fv( _verts[ _faces[i].v1].v);

        if( _faces[i].smooth)
            glNormal3fv( _norms[ _faces[i].v2].v);
        glVertex3fv( _verts[ _faces[i].v2].v);

        if( _faces[i].smooth)
            glNormal3fv( _norms[ _faces[i].v3].v);
        glVertex3fv( _verts[ _faces[i].v3].v);

        if( _faces[i].smooth)
            glNormal3fv( _norms[ _faces[i].v4].v);
        glVertex3fv( _verts[ _faces[i].v4].v);
    }
    glEnd();

    glEndList();
}
//------------------------------------------------------------------------------

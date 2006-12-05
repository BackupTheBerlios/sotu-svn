// Description:
//   2D and 3D points.
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
#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <math.h>

//-----------------------------------------------------------------------

struct Point2D
{
    float x;
    float y;

    Point2D():x(0),y(0){}
    Point2D( float d1, float d2):x(d1),y(d2){}

    static int dimension( void){ return 2;}
    float operator[]( int idx)
    {
	switch( idx)
	{
	    case 0: return x;
	    case 1: return y;

	    default: return 0;
	}
    }
    void set( int idx, float val)
    {
	switch( idx)
	{
	    case 0: x = val;
		    break;
	    case 1: y = val;
		    break;
	    default:
		    break;
	}
    }
};

struct Point2Di
{
    int x;
    int y;
};

//-----------------------------------------------------------------------

struct Point3Di
{
    Point3Di():x(0),y(0),z(0){}
    Point3Di( int d1, int d2, int d3):x(d1),y(d2),z(d3){}

    int x;
    int y;
    int z;
};

struct Point3D
{
    float x;
    float y;
    float z;

    Point3D():x(0),y(0),z(0){}
    Point3D( float d1, float d2, float d3):x(d1),y(d2),z(d3){}

    static int dimension( void){ return 3;}
    float operator[]( int idx)
    {
	switch( idx)
	{
	    case 0: return x;
	    case 1: return y;
	    case 2: return z;

	    default: return 0;
	}
    }
    void set( int idx, float val)
    {
	switch( idx)
	{
	    case 0: x = val;
		    break;
	    case 1: y = val;
		    break;
	    case 2: z = val;
		    break;
	    default:
		    break;
	}
    }

    inline Point3D &operator=( const Point3Di &pi)
    {
	x = (float)pi.x;
	y = (float)pi.y;
	z = (float)pi.z;
	return *this;
    }
};
//-----------------------------------------------------------------------

inline Point2D operator+( const Point2D p1, const Point2D p2)
{
    return( Point2D( p1.x+p2.x, p1.y+p2.y));
}

inline Point2D operator-( const Point2D p1, const Point2D p2)
{
    return( Point2D( p1.x-p2.x, p1.y-p2.y));
}

inline Point2D operator/( const Point2D p1, float d)
{
    return( Point2D( p1.x/d, p1.y/d));
}

inline void norm( Point2D &p)
{
    float dist = sqrt(p.x*p.x + p.y*p.y);

    p.x = p.x/dist;
    p.y = p.y/dist;
}

inline Point2D operator*( const Point2D p1, const float s)
{
    return( Point2D( p1.x*s, p1.y*s));
}

struct BoundingBox
{
    Point2D min;
    Point2D max;
};

//-----------------------------------------------------------------------

inline Point3D operator+( const Point3D p1, const Point3D p2)
{
    return( Point3D( p1.x+p2.x, p1.y+p2.y, p1.z+p2.z));
}

inline Point3D operator-( const Point3D p1, const Point3D p2)
{
    return( Point3D( p1.x-p2.x, p1.y-p2.y, p1.z-p2.z));
}

inline Point3D operator*( const Point3D p1, const float s)
{
    return( Point3D( p1.x*s, p1.y*s, p1.z*s));
}

inline Point3D operator/( const Point3D p1, float d)
{
    return( Point3D( p1.x/d, p1.y/d, p1.z/d));
}

inline float dot( const Point3D p1, const Point3D p2)
{
    return( p1.x*p2.x + p1.y*p2.y + p1.z*p2.z);
}

inline Point3D cross( const Point3D p1, const Point3D p2)
{
    return( Point3D( p1.y*p2.z-p1.z*p2.y,
		p1.z*p2.x-p1.x*p2.z,
		p1.x*p2.y-p1.y*p2.x));
}

inline float dist(const Point3D &p)
{
    return sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
}

inline void norm( Point3D &p)
{
    float d = dist(p);

    p.x = p.x/d;
    p.y = p.y/d;
    p.z = p.z/d;
}

inline void Clamp( float &val, const float &MINval, const float &MAXval)
{
    if( val < MINval) val = MINval;
    else if( val > MAXval) val = MAXval;
}

//-----------------------------------------------------------------------

inline Point3Di operator+( const Point3Di p1, const Point3Di p2)
{
    return( Point3Di( p1.x+p2.x, p1.y+p2.y, p1.z+p2.z));
}

inline Point3Di operator-( const Point3Di p1, const Point3Di p2)
{
    return( Point3Di( p1.x-p2.x, p1.y-p2.y, p1.z-p2.z));
}

inline bool operator==( const Point3Di p1, const Point3Di p2)
{
    return ((p1.x==p2.x) && (p1.y==p2.y) && (p1.z==p2.z));
}

inline bool operator!=( const Point3Di p1, const Point3Di p2)
{
    return ! (p1==p2);
}

//-----------------------------------------------------------------------

struct vec3
{
    float & x;
    float & y;
    float & z;

    float v[3];

    vec3( void):x(v[0]),y(v[1]),z(v[2]) {}

    vec3( float X, float Y, float Z):x(v[0]),y(v[1]),z(v[2]) { x=X; y=Y; z=Z;}

    vec3(const vec3 &vi):x(v[0]),y(v[1]),z(v[2]) 
    {
	v[0]=vi.x; 
	v[1]=vi.y; 
	v[2]=vi.z; 
    }

    vec3 &operator=(const vec3 &vi)
    {
	v[0]=vi.x; 
	v[1]=vi.y; 
	v[2]=vi.z; 

	return *this;
    }
};

inline vec3 operator+( const vec3 p1, const vec3 p2)
{
    return( vec3( p1.x+p2.x, p1.y+p2.y, p1.z+p2.z));
}

inline vec3 operator-( const vec3 p1, const vec3 p2)
{
    return( vec3( p1.x-p2.x, p1.y-p2.y, p1.z-p2.z));
}

inline vec3 operator*( const vec3 p1, const float s)
{
    return( vec3( p1.x*s, p1.y*s, p1.z*s));
}

//-----------------------------------------------------------------------

#endif


// Description:
//   Quaternions
//
// Copyright (C) 2003 Frank Becker
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
#ifndef _QUATERNION_HPP_
#define _QUATERNION_HPP_

#include <math.h>

#include <Point.hpp>

const float EPSILON = 0.00005;
const float TO_RADIANS = M_PI/180.0;
const float TO_DEGREES = 180.0/M_PI;

struct Quaternion
{
    float w;
    float x;
    float y;
    float z;

    Quaternion(): 
        w(1.0f), x(0.0f), y(0.0f), z(0.0f){}

    Quaternion( float W, float X, float Y, float Z):
    	w(W), x(X), y(Y), z(Z){}

    Quaternion( float angle, const Point3D &axis)
    {
	set( angle, axis);
    }

    Quaternion( const Quaternion &q)
    {
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
    }
    
    void operator=( const Quaternion &q)
    {
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
    }

    void identity( void)
    {
	w = 1.0;
	x = 0.0;
	y = 0.0;
	z = 0.0;
    }

    void norm()
    {
	float lensqr = w*w + x*x + y*y + z*z;
	if( fabs(lensqr-1.0f) > EPSILON)
	{
	    if( lensqr < (EPSILON*EPSILON))
	    {
		identity();
		return;
	    }

	    float len = sqrt(lensqr);
	    w = w/len;
	    x = x/len;
	    y = y/len;
	    z = z/len;
	}
    }

    //angle in degrees, axis must be normalized
    void set( float angle, const Point3D &axis)
    {
	float halfAngle = angle * 0.5f * TO_RADIANS;
	float sina = sin( halfAngle);
	float cosa = cos( halfAngle);

	x    = axis.x * sina;
	y    = axis.y * sina;
	z    = axis.z * sina;
	w    = cosa;
    }

    //angle in degrees
    void get( float &angle, Point3D &axis)
    {
	float cosa = w;
	//cos^2 + sin^2 = 1 -> sin = sqrt(1 - cos^2)
	float sina  = sqrt( 1.0f - cosa * cosa );

	if ( fabs( sina) < EPSILON)
	{
	    //XXX: we pick x as the rotation axis
	    axis.x = 1.0;
	    axis.y = 0.0;
	    axis.z = 0.0;
	    angle  = 0.0;
	}
	else
	{
	    axis.x = x / sina;
	    axis.y = y / sina;
	    axis.z = z / sina;
	    angle  = acos( cosa) * 2.0f * TO_DEGREES;
	}
    }
};

inline Quaternion operator*( const Quaternion &q1, const Quaternion &q2)
{
    // * is multiply, . is dot product, x is cross product, +/- take a guess
    //
    // q1 * q2 =( w1*w2 - v1.v2 , w1.v2 + w2.v1 + v1 x v2)
    //
    // w = w1*w2 - (x1*x2 + y1*y2 + z1*z2)
    // x = w1*x2 + w2*x1 + (y1*z2 - z1*y2)
    // y = w1*y2 + w2*y1 + (z1*x2 - x1*z2)
    // z = w1*z2 + w2*z1 + (x1*y2 - y1*x2)

    return Quaternion( 
	// rearranged to align q1.wxyz and to confuse the innocent
	q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
	q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
	q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
	q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w);
}

#endif

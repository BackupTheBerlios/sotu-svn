// Description:
//   Camera to enable 1st person shooter type mouselook.
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
#include <math.h>

#include <Trace.hpp>
#include <Camera.hpp>
#include <Direction.hpp>
#include <Constants.hpp>

const float defaultAngleSensitivity = 1.0;

void Camera::place( void)
{
//    XTRACE();
//    if( GameState::context != Context::eCameraFlyby) return;

    glRotatef( -_cameraPitch, 1.0, 0.0, 0.0 );
    glRotatef( -_cameraYaw, 0.0, 1.0, 0.0 );
    glTranslatef( -_cameraX, -_cameraY, -_cameraZ );
}

void Camera::billboard( void)
{
//    XTRACE();
    glRotatef( _cameraYaw, 0.0, 1.0, 0.0 );
    glRotatef( _cameraPitch, 1.0, 0.0, 0.0 );
}

void Camera::mouseLook( float dx, float dy)
{
//    XTRACE();
    if( (dx!=0) || (dy!=0))
    {
        _cameraYaw -= defaultAngleSensitivity * dx;
        _cameraPitch += defaultAngleSensitivity * dy;
        clampCamera();
    }
}

void Camera::update( void)
{
//    XTRACE();
    if( _moveCam)
    {
        move( 0.3f/*_stepSize*/, _cameraMove);
    }
}

void Camera::move( Direction::DirectionEnum dir, bool isDown)
{
//    XTRACE();
    _moveCam = isDown;

    switch( dir)
    {
        case Direction::eDown:
            _cameraMove = 180.0;
            break;
        case Direction::eUp:
            _cameraMove = 0.0;
            break;

        case Direction::eLeft:
            _cameraMove = 90.0;
            break;
        case Direction::eRight:
            _cameraMove = 270.0;
            break;

        default:
            break;
    }
}

void Camera::move( GLfloat stepSize, GLfloat angle)
{
//    XTRACE();
    GLfloat cosPitch = 1.0;

    if( (angle==0) || (angle==180))
    {
        GLfloat sinPitch;
        GLfloat actualAngleInRadians2( ( _cameraPitch+angle) * (float)M_PI / 180.0f );
        sinPitch = sin( actualAngleInRadians2 );
        cosPitch = cos( actualAngleInRadians2 );
        _cameraY += sinPitch * stepSize;
    }

    if( angle==180) cosPitch = -cosPitch;

    GLfloat actualAngleInRadians( ( _cameraYaw + angle ) * (float)M_PI / 180.0f );
    _cameraX -= sin( actualAngleInRadians ) * stepSize * cosPitch;
    _cameraZ -= cos( actualAngleInRadians ) * stepSize * cosPitch;
}

void Camera::clampCamera( void)
{
    // Clamp the _cameraera's pitch.
    if( _cameraPitch > 90.0 )
        _cameraPitch = 90.0;
    else if( _cameraPitch < -90.0 )
        _cameraPitch = -90.0;

    // wrap the yaw/heading
    while( _cameraYaw < 0.0 )
        _cameraYaw += 360.0;
    while( _cameraYaw >= 360.0 )
        _cameraYaw -= 360.0;
}

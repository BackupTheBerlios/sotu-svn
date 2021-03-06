// Description:
//   Action callbacks for mouse and keyboard events.
//
// Copyright (C) 2001 Frank Becker
// Copyright (C) 2006 Milan Babuskov
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
#ifndef _ActionCallbacks_hpp_
#define _ActionCallbacks_hpp_

#include <Trace.hpp>
#include <Callback.hpp>
#include <GameState.hpp>
//----------------------------------------------------------------------------
class MotionAction: public Callback
{
public:
    MotionAction( void): Callback( "Motion") { XTRACE(); }
    virtual ~MotionAction() { XTRACE(); }
    virtual void performAction( Trigger &trigger, bool isDown);
};
//----------------------------------------------------------------------------
class MotionLeftAction: public Callback
{
public:
    MotionLeftAction( void): Callback( "MotionLeft") { XTRACE(); }
    virtual ~MotionLeftAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class MotionRightAction: public Callback
{
public:
    MotionRightAction( void): Callback( "MotionRight") { XTRACE(); }
    virtual ~MotionRightAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class MotionUpAction: public Callback
{
public:
    MotionUpAction( void): Callback( "MotionUp") { XTRACE(); }
    virtual ~MotionUpAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class MotionDownAction: public Callback
{
public:
    MotionDownAction( void): Callback( "MotionDown") { XTRACE(); }
    virtual ~MotionDownAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class WeaponFireAction: public Callback
{
public:
    WeaponFireAction( int numWeap, string actionName):
        Callback( actionName),
        _numWeap( numWeap)
    {
        XTRACE();
    }
    virtual ~WeaponFireAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
private:
    int _numWeap;
};
//----------------------------------------------------------------------------
class RocketFireAction: public Callback
{
public:
    RocketFireAction(): Callback("Rocket") { XTRACE(); }
    virtual ~RocketFireAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class MegaBombFireAction: public Callback
{
public:
    MegaBombFireAction(): Callback("MegaBomb") { XTRACE(); }
    virtual ~MegaBombFireAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class SnapshotAction: public Callback
{
public:
    SnapshotAction( void): Callback( "Snapshot") { XTRACE(); }
    virtual ~SnapshotAction() { XTRACE(); }
    virtual void performAction( Trigger &trigger, bool isDown);
};
//----------------------------------------------------------------------------
class ConfirmAction: public Callback
{
public:
    ConfirmAction( void): Callback( "Confirm") { XTRACE(); }
    virtual ~ConfirmAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class ChangeContext: public Callback
{
public:
    ChangeContext( void): Callback( "ChangeContext") { XTRACE(); }
    virtual ~ChangeContext() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class CritterBoard: public Callback
{
public:
    CritterBoard( void): Callback( "CritterBoard") { XTRACE(); }
    virtual ~CritterBoard() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class PauseGame: public Callback
{
public:
    PauseGame( void): Callback( "PauseGame")
    {
        XTRACE();
    }
    virtual ~PauseGame() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class EscapeAction: public Callback
{
public:
    EscapeAction( void): Callback( "EscapeAction")
    {
        XTRACE();
    }
    virtual ~EscapeAction() { XTRACE(); }
    virtual void performAction( Trigger &, bool isDown);
};
//----------------------------------------------------------------------------
class HyperSpaceJump: public Callback
{
public:
    HyperSpaceJump(): Callback("HyperSpace")
    {
        XTRACE();
    }
    virtual ~HyperSpaceJump() { XTRACE(); }
    virtual void performAction( Trigger&, bool isDown);
};
//----------------------------------------------------------------------------
#endif

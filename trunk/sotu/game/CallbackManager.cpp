// Description:
//   Callback manager.
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
#include <Trace.hpp>
#include <ActionCallbacks.hpp>
#include <CallbackManager.hpp>
#include <FindHash.hpp>
#include <Hero.hpp>
//----------------------------------------------------------------------------
CallbackManager::CallbackManager( void)
{
    XTRACE();
}
//----------------------------------------------------------------------------
CallbackManager::~CallbackManager()
{
    XTRACE();

    hash_map< string, Callback*, hash<string> >::const_iterator ci;
    for( ci=_actionMap.begin(); ci!=_actionMap.end(); ci++)
    {
        delete ci->second;
    }

    _actionMap.clear();
}
//----------------------------------------------------------------------------
void CallbackManager::init( void)
{
    XTRACE();

    //actions will add themselves to the CallbackManager
    new WeaponFireAction( Hero::PRIMARY_WEAPON, "PrimaryFire");
    new WeaponFireAction( Hero::SECONDARY_WEAPON, "SecondaryFire");
    new WeaponFireAction( Hero::TERTIARY_WEAPON, "TertiaryFire");
    new MegaBombFireAction();
    new RocketFireAction();
    new SnapshotAction();

    new MotionAction();
    new MotionRightAction();
    new MotionLeftAction();
    new MotionUpAction();
    new MotionDownAction();
    //new ChangeContext();
    new PauseGame();
    new ConfirmAction();
    new EscapeAction();
    new CritterBoard();
    new HyperSpaceJump();
}
//----------------------------------------------------------------------------
void CallbackManager::addCallback( Callback *cb)
{
    XTRACE();
    LOG_INFO << "Adding callback for action ["
             << cb->getActionName() << "]" << endl;
    _actionMap[ cb->getActionName()] = cb;
}
//----------------------------------------------------------------------------
Callback *CallbackManager::getCallback( string actionString)
{
    XTRACE();
    Callback * cb = findHash( actionString, _actionMap);
    if( !cb)
    {
        LOG_ERROR << "Unable to find callback for " << actionString << endl;
        string dummyAction = "TertiaryFire";
        cb = findHash( dummyAction, _actionMap);
    }
    return cb;
}
//----------------------------------------------------------------------------

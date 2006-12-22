// Description:
//   Action callbacks for mouse and keyboard events.
//
// Copyright (C) 2001 Frank Becker
// Copyright (c) 2006 Milan Babuskov
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
#include "SDL.h"

#include <Trace.hpp>

#include <Input.hpp>
#include <Video.hpp>
#include <Hero.hpp>
#include <Game.hpp>
#include <Direction.hpp>
#include <Camera.hpp>
#include <MenuManager.hpp>
#include <ActionCallbacks.hpp>
#include <Audio.hpp>
//----------------------------------------------------------------------------
void MotionAction::performAction( Trigger &trigger, bool /*isDown*/)
{
    //    XTRACE();
    if (GameS::instance()->getContext() == eInGame)
        HeroS::instance()->move( trigger.fData1, trigger.fData2);
}
//----------------------------------------------------------------------------
void MotionLeftAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if (GameS::instance()->getContext() == eInGame)
        HeroS::instance()->move( Direction::eLeft, isDown);
}
//----------------------------------------------------------------------------
void MotionRightAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if (GameS::instance()->getContext() == eInGame)
        HeroS::instance()->move( Direction::eRight, isDown);
}
//----------------------------------------------------------------------------
void MotionUpAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if (GameS::instance()->getContext() == eInGame)
        HeroS::instance()->move( Direction::eUp, isDown);
}
//----------------------------------------------------------------------------
void MotionDownAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if (GameS::instance()->getContext() == eInGame)
        HeroS::instance()->move( Direction::eDown, isDown);
}
//----------------------------------------------------------------------------
void WeaponFireAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if (GameS::instance()->getContext() == eInGame)
        HeroS::instance()->weaponFire( isDown, _numWeap);
}
//----------------------------------------------------------------------------
void SnapshotAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if( !isDown)
        return;

    VideoS::instance()->takeSnapshot();
}
//----------------------------------------------------------------------------
void ConfirmAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if( !isDown)
        return;

    LOG_INFO << "Yes Sir!" << endl;
}
//----------------------------------------------------------------------------
void CritterBoard::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if( !isDown)
        return;

    //    LOG_INFO << "toggle CritterBoard..." << endl;
    VideoS::instance()->toggleCritterBoard();
}
//----------------------------------------------------------------------------
void PauseGame::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if (!isDown)
        return;

    if (GameS::instance()->getContext() == ePaused)
        GameS::instance()->switchContext(eInGame);
    else
    {
        GameS::instance()->switchContext(ePaused);
        SDL_ShowCursor(SDL_ENABLE);
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    }
}
//----------------------------------------------------------------------------
void EscapeAction::performAction( Trigger &, bool isDown)
{
    //    XTRACE();
    if( !isDown)
        return;

    // TRY IT!
    GameS::instance()->switchContext(ePlanetMenu);
}
//----------------------------------------------------------------------------
void HyperSpaceJump::performAction(Trigger &, bool isDown)
{
    if (!isDown)
        return;

    if (GameS::instance()->getContext() != eInGame)
        return;

    // check fuel
    Planet *target = PlanetManagerS::instance()->_hyperspaceTarget;
    Planet *source = GameS::instance()->_currentPlanet;
    if (target == source)
    {
        HeroS::instance()->popMessage("No hyperspace target", 1.0f, 0.0f, 0.0f);
        return;
    }

    float dist = source->getDistance(target->_x, target->_y);
    CargoItem* fuel = GameS::instance()->_cargo.findItem("Fuel");
    if (fuel->_quantity < (int)(dist + 0.92))
    {
        HeroS::instance()->popMessage("Not enough fuel", 1.0f, 0.0f, 0.0f);
        return;
    }

    // start hyperspace countdown
    GameS::instance()->hyperspaceJump();
}
//----------------------------------------------------------------------------

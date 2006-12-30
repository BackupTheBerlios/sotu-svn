// Description:
//   Input subsystem.
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
#include <math.h>

#include <Input.hpp>
#include <Trace.hpp>
#include <Config.hpp>
#include <Callback.hpp>
#include <GameState.hpp>
#include <Video.hpp>
#include <FindHash.hpp>
#include <CallbackManager.hpp>
#include <Tokenizer.hpp>

#include <FPS.hpp>

#include "SDL.h"
//----------------------------------------------------------------------------
Input::Input( void):
    _bindMode( false),
    _callback(0),
    _callbackManager(),
    _memoryDX(0.0),
    _memoryDY(0.0),
    _valDX(0.0),
    _valDY(0.0),
    _dampVal(0.0),
    _interceptor(0)
{
    XTRACE();
}
//----------------------------------------------------------------------------
Input::~Input()
{
    XTRACE();
    _callbackMap.clear();
}
//----------------------------------------------------------------------------
Input*  Input::preinit( void)
{
    _callbackManager.init();
    return this;
}
//----------------------------------------------------------------------------
bool Input::init( void)
{
    XTRACE();
    LOG_INFO << "Initializing Input..." << endl;

    updateMouseSettings();
    LOG_INFO << "Mouse smoothing: " << _dampVal << endl;
    LOG_INFO << "Mouse sensitivity: " << _sensitivity << endl;

    SDL_EnableKeyRepeat( 300,200);

    LOG_INFO << "Input OK." << endl;
    return true;
}
//----------------------------------------------------------------------------
void Input::updateMouseSettings( void)
{
    ConfigS::instance()->getFloat( "mouseSmooth", _dampVal);
    if( (_dampVal<0.0) || (_dampVal>0.9))
    {
        _dampVal = 0.0f;
    }

    ConfigS::instance()->getFloat( "mouseSensitivity", _sensitivity);
    if( (_sensitivity<0.01) || (_sensitivity>1.0))
    {
        _sensitivity = 0.1f;
    }
}
//----------------------------------------------------------------------------
// Returns false, if there are no more events available.
bool Input::tryGetTrigger( Trigger &trigger, bool &isDown)
{
//    XTRACE();
    isDown = false;

    SDL_Event event;
    if( !SDL_PollEvent( &event))
    {
        return false;
    }

    switch( event.type )
    {
        case SDL_KEYDOWN:
            isDown = true;
#ifndef NO_QUICK_EXIT
            if( event.key.keysym.sym == SDLK_BACKQUOTE)
            {
                GameState::isAlive = false;
                LOG_WARNING << "Quick Exit invoked..." << endl;

                trigger.type = eUnknownTrigger;
                break;
            }
#endif
            //fall through

        case SDL_KEYUP:
            trigger.type = eKeyTrigger;
            trigger.data1 = event.key.keysym.sym;
            trigger.data2 = event.key.keysym.mod;
            trigger.data3 = event.key.keysym.unicode;
            break;

        case SDL_MOUSEBUTTONDOWN:
            isDown = true;
            //fall through

        case SDL_MOUSEBUTTONUP:
            trigger.type = eButtonTrigger;
            trigger.data1 = event.button.button;
            trigger.data2 = 0;
            trigger.data3 = 0;
            break;

        case SDL_MOUSEMOTION:
            trigger.type = eUnknownTrigger;
            _valDX = event.motion.xrel*_sensitivity;
            _valDY = -event.motion.yrel*_sensitivity;

        //if (event.motion.x == 0 || event.motion.y == 0)
        //    Video
            break;

        case SDL_QUIT:
            GameState::isAlive = false;
            break;

        default:
            trigger.type = eUnknownTrigger;
            break;
    }

    return true;
}
//----------------------------------------------------------------------------
bool Input::update( void)
{
//    XTRACE();
    bool isDown;
    Trigger trigger;

    static float nextTime = Timer::getTime()+0.5f;
    float thisTime = Timer::getTime();
    if( thisTime > nextTime)
    {
        updateMouseSettings();
        nextTime = thisTime+0.5f;
    }

    _valDX = 0.0f;
    _valDY = 0.0f;

    while( tryGetTrigger( trigger, isDown))
    {
        if( trigger.type == eUnknownTrigger)
        {
            //for unkown trigger we don't need to do a lookup
            continue;
        }

        if( _interceptor)
        {
            //feed trigger to interceptor instead of normal callback mechanism
            _interceptor->input( trigger, isDown);
            continue;
        }

        if( !_bindMode)
        {
            //find callback for this trigger
            //i.e. the action bound to this key
            Callback * cb = findHash( trigger, _callbackMap);
            if( cb)
            {
//                LOG_INFO << "Callback for [" << cb->getActionName() << "]" << endl;
                cb->performAction( trigger, isDown);
            }
        }
        else if( _callback && (trigger.type!=eMotionTrigger))
        {
            //Note: motion triggers can't be bound

            //we are in bind-mode, so bind this trigger to callback
            bind( trigger, _callback);
            //go back to normal mode
            _bindMode = false;
        }
        else if( !_callback)
        {
            LOG_ERROR << "Input is in bind mode, but callback is 0" << endl;
            _bindMode = false;
        }
    }
    _valDX = feedbackFilter( _valDX, _dampVal, _memoryDX);
    _valDY = feedbackFilter( _valDY, _dampVal, _memoryDY);

    if( (fabs(_valDX)>1.0e-10) || (fabs(_valDY)>1.0e-10))
    {
        trigger.type = eMotionTrigger;
        trigger.fData1 = _valDX;
        trigger.fData2 = _valDY;

        if( _interceptor)
        {
            //feed trigger to interceptor instead of normal callback mechanism
            _interceptor->input( trigger, true);
        }
        else
        {
            Callback * cb = findHash( trigger, _callbackMap);
            if( cb)
            {
//                LOG_INFO << "Callback for [" << cb->getActionName() << "]" << endl;
                cb->performAction( trigger, isDown);
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------
void Input::handleLine( const string line)
{
    //    XTRACE();
    Tokenizer  t( line);
    string bindKeyword = t.next();
    if( bindKeyword != "bind")
        return;

    string action = t.next();
    string keyname = t.next();
    //    LOG_INFO << "action [" << action << "], "
    //             << "keyname [" << keyname << "]" << endl;

    Trigger trigger;
    if( _keys.convertStringToTrigger( keyname, trigger))
    {
        Callback *cb = _callbackManager.getCallback( action);
        bind( trigger, cb);
    }
}
//----------------------------------------------------------------------------
void Input::defaultKeys()
{
    static string bindings[] = { "PauseGame", "CritterBoard",
        "MotionLeft", "MotionRight",  "MotionUp",    "MotionDown", "HyperSpace",
        "PrimaryFire","SecondaryFire","TertiaryFire","MegaBomb",   "Rocket" };
    static string keys[] = {     "P",         "TAB",
        "LEFT",       "RIGHT",        "UP",          "DOWN",       "H",
        "SPACE",      "LALT",         "LCTRL",       "D",          "F" };

    for (unsigned int i=0; i<sizeof(keys)/sizeof(string); ++i)
    {
        unbindKeys(bindings[i]);
        string line = "bind " + bindings[i] + " " + keys[i];
        handleLine(line);
    }
}
//----------------------------------------------------------------------------
void Input::unbindKeys(const std::string& action)
{
    bool found;
    do
    {
        found = false;
        std::string retval;
        hash_map< Trigger, Callback*, hash<Trigger> >::iterator ci;
        for( ci=_callbackMap.begin(); ci!=_callbackMap.end(); ci++)
        {
            if (ci->second->getActionName() == action &&
                ci->first.type == eKeyTrigger)
            {
                found = true;
                //LOG_INFO << "REMOVING PREVIOUS KEY BINDING" << endl;
                _callbackMap.erase(ci);
                //LOG_INFO << "DONE" << endl;
                return;
            }
        }
    }
    while (found);
}
//----------------------------------------------------------------------------
std::string Input::getKeyForAction(const std::string& action)
{
    hash_map< Trigger, Callback*, hash<Trigger> >::const_iterator ci;
    for( ci=_callbackMap.begin(); ci!=_callbackMap.end(); ci++)
        if (ci->second->getActionName() == action &&  ci->first.type == eKeyTrigger)
            return _keys.convertTriggerToString(ci->first);
    return "NONE!!!";   // <- MAGIC STRING!!! change in Video.cpp as well
}
//----------------------------------------------------------------------------
void Input::save( ofstream &outfile)
{
    XTRACE();
    outfile << "# --- Binding section --- " << endl;

    hash_map< Trigger, Callback*, hash<Trigger> >::const_iterator ci;
    for( ci=_callbackMap.begin(); ci!=_callbackMap.end(); ci++)
    {
        outfile << "bind "
                << ci->second->getActionName() << " "
                << _keys.convertTriggerToString( ci->first)
                << endl;
    }
}
//----------------------------------------------------------------------------
void Input::bind( Trigger &trigger, Callback *callback)
{
    XTRACE();
    Callback * cb = findHash( trigger, _callbackMap);
    if( cb)
    {
        LOG_INFO << "Removing old binding" << endl;
        //remove previous callback...
        _callbackMap.erase( trigger);
        //delete cb;
        LOG_INFO << "DELETE SUCCESSFULL" << endl;
    }

    LOG_INFO << "Creating binding for " << callback->getActionName()
             << " - " << trigger.type << ":" << trigger.data1 << endl;
    _callbackMap[ trigger] = callback;
}
//----------------------------------------------------------------------------

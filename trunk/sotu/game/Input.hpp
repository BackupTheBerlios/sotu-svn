// Description:
//   Input subsystem.
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
#ifndef _Input_hpp_
#define _Input_hpp_

#include <hashMap.hpp>

#include <Singleton.hpp>
#include <Trigger.hpp>
#include <Keys.hpp>
#include <ConfigHandler.hpp>
#include <CallbackManager.hpp>
#include <InterceptorI.hpp>

class Callback;

namespace HASH_NAMESPACE
{
    template<>
	struct hash<Trigger>
    {
	//a simple hash function for Trigger
	int operator()(const Trigger &t) const
	{
	    int hashval;
	    
	    if( t.type == eMotionTrigger)
	    {
		hashval = t.type*1000;
	    }
	    else
	    {
		hashval = t.type*1000+t.data1;
	    }

	    return hashval;
	}
    };
}

class Input: public ConfigHandler
{
friend class Singleton<Input>;
public:
    Input *preinit( void);
    bool init( void);
    bool update( void);

    //used for loading/saving bindings
    virtual void handleLine( const string line);
    virtual void save( ofstream &of);

    //Input takes ownership of callback
    void bindNextTrigger( Callback *cb)
    {
        _bindMode = true;
        _callback = cb;
    }

    bool waitingForBind( void)
    {
        return _bindMode;
    }

    void addCallback( Callback *cb)
    {
	_callbackManager.addCallback( cb);
    }

    void enableInterceptor( InterceptorI *i)
    {
	_interceptor = i;
    }

    void disableInterceptor( void)
    {
	_interceptor = 0;
    }

private:
    virtual ~Input();
    Input( void);
    Input( const Input&);
    Input &operator=(const Input&);

    void bind( Trigger &t, Callback *action);
    bool tryGetTrigger( Trigger &trigger, bool &isDown);
    void updateMouseSettings( void);

    bool _bindMode;
    Callback *_callback;
    CallbackManager _callbackManager;
    Keys _keys;

    hash_map< Trigger, Callback*, hash<Trigger>, equal_to<Trigger> > _callbackMap;

    //stuff for mouse smoothing
    float _memoryDX;
    float _memoryDY;
    float _valDX;
    float _valDY;
    float _dampVal;
    float feedbackFilter( float value, float damp, float &memory)
    {
	//damp of 0 means no filtering
	//damp of 1 means input value is filtered out completely
	return memory = value*(1.0f-damp) + memory*damp;
    }   
    float _sensitivity;

    //intercept raw input
    InterceptorI *_interceptor;
};

typedef Singleton<Input> InputS;

#endif

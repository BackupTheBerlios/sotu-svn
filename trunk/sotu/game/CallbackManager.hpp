// Description:
//   Callback manager.
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
#ifndef _CallbackManager_hpp_
#define _CallbackManager_hpp_

#include <string>
#include <hashMap.hpp>

#include <HashString.hpp>

class Callback;

class CallbackManager
{
public:
    CallbackManager( void);
    ~CallbackManager();
    void init( void);

    Callback *getCallback( string actionString);
    void addCallback( Callback *cb);

private:
    CallbackManager( const CallbackManager&);
    CallbackManager &operator=(const CallbackManager&);

    hash_map< string, Callback*, hash<string>, equal_to<string> > _actionMap;
};

#endif

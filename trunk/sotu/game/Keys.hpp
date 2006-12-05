// Description:
//   Keyboard/Trigger helpers.
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
#ifndef _Keys_hpp_
#define _Keys_hpp_

using namespace std;

#include "SDL.h"
#include <string>
#include <Trigger.hpp>

class Keys
{
public:
    Keys( void);
    ~Keys();
    string convertTriggerToString( const Trigger & trigger);
    bool convertStringToTrigger( 
        string & triggerStr, Trigger & trigger);

private:
    Keys( const Keys&);
    Keys &operator=(const Keys&);

    string _symmap[ SDLK_LAST];
};

#endif

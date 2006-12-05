// Description:
//   Base for callbacks.
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
#ifndef _Callback_hpp_
#define _Callback_hpp_

#include <string>
#include <Input.hpp>

class Callback
{
public:
    Callback( string actionName);
    virtual ~Callback();

    virtual void performAction( Trigger &trigger, bool isDown) = 0;
    string &getActionName( void){ return _actionName;};

protected:
    string _actionName;

private:
    Callback( const Callback&);
    Callback &operator=(const Callback&);
};

#endif

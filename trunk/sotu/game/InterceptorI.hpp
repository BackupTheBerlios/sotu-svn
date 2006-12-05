// Description:
//   Interface for interceptors.
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
#ifndef _InterceptorI_hpp_
#define _InterceptorI_hpp_

#include <Trigger.hpp>

class InterceptorI
{
public:
    virtual void input( const Trigger &trigger, const bool &isDown) = 0;

	virtual ~InterceptorI() {}
};

#endif

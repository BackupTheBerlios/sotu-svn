// Description:
//   Helpers to get time.
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
#ifndef _Timer_hpp_
#define _Timer_hpp_
//----------------------------------------------------------------------------

#include <defines.h>

#ifdef TIMER_USE_GETTIMEOFDAY
#include <gettimeofday.hpp>
#else
#include "SDL.h"
#endif
//----------------------------------------------------------------------------
class Timer
{
public:
#ifdef TIMER_USE_GETTIMEOFDAY
    static float getTime( void)
    {
        struct timeval tv;
        struct timezone tz;
        static float startSeconds = -1.0;

        gettimeofday(&tv, &tz);
        if (startSeconds < 0.0) //not interested in subsecond start time
            startSeconds = tv.tv_sec;
        return( tv.tv_sec - startSeconds + tv.tv_usec/1000000.0);
    }
#else
    static float getTime(void)
    {
        return ((float)SDL_GetTicks()/1000.0f);
    }

    static void sleep( float sleep)
    {
        Uint32 ms = (Uint32)(sleep*1000.0);
        SDL_Delay(ms);
    }
#endif
};
//----------------------------------------------------------------------------
#endif

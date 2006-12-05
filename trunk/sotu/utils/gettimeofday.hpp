// Description:
//   gettimeofday wrapper
//
// Copyright (C) 2003 Frank Becker
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
#ifndef _gettimeofday_hpp_
#define _gettimeofday_hpp_

#ifdef WIN32
#  include <sys/timeb.h>

typedef long int suseconds_t;
typedef long int time_t;

#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval 
{
    time_t tv_sec;
    suseconds_t tv_usec;
};
#endif

struct timezone 
{
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval *tv, struct timezone * /*tz*/)
{
    struct timeb t;
    ftime (&t);

    tv->tv_sec  = t.time;
    tv->tv_usec = t.millitm*1000;

    return 0;
}
#else
#  include <sys/time.h>
#endif

#endif

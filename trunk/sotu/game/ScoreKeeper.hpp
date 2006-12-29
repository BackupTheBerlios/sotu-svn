// Description:
//   Score keeper.
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
#ifndef _ScoreKeeper_hpp_
#define _ScoreKeeper_hpp_

#include <string>
#include <vector>

#include <time.h>

#include <Singleton.hpp>
//----------------------------------------------------------------------------
class ScoreKeeper
{
friend class Singleton<ScoreKeeper>;
public:
    ~ScoreKeeper();
    ScoreKeeper();

    int getCurrentScore();
    void resetCurrentScore();
    int addToCurrentScore(int value);
    void setScore(int value);

private:
    ScoreKeeper( const ScoreKeeper&);
    ScoreKeeper &operator=(const ScoreKeeper&);

    int _score;
};

typedef Singleton<ScoreKeeper> ScoreKeeperS;
//----------------------------------------------------------------------------
#endif

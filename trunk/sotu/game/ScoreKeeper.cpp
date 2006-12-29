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

#include <Trace.hpp>
#include <ScoreKeeper.hpp>
//----------------------------------------------------------------------------
ScoreKeeper::ScoreKeeper()
{
    XTRACE();
    _score = 0;
}
//----------------------------------------------------------------------------
ScoreKeeper::~ScoreKeeper( void)
{
    XTRACE();
}
//----------------------------------------------------------------------------
void ScoreKeeper::resetCurrentScore()
{
    _score = 0;
}
//----------------------------------------------------------------------------
void ScoreKeeper::setScore(int value)
{
    _score = value;
}
//----------------------------------------------------------------------------
int ScoreKeeper::addToCurrentScore( int value)
{
    XTRACE();

    _score += value;
    return value;
}
//----------------------------------------------------------------------------
int ScoreKeeper::getCurrentScore()
{
    return _score;
}
//----------------------------------------------------------------------------

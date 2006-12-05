// Description:
//   Stage manager controls movement from levelpack to levelpack and
//   level to level.
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
#ifndef _StageManager_hpp_
#define _StageManager_hpp_

#include <Singleton.hpp>
#include <GameState.hpp>
#include <LevelPack.hpp>

#include <tinyxml.h>

class StageManager
{
friend class Singleton<StageManager>;
public:
    bool init( void);
    void reset( void);
    void update( void);

    float levelStartTime( void)
    {
        return _levelStartTime;
    }

    const string &getActiveLevelName( void)
    {
        return _activeLevelName;
    }

private:
    StageManager( void):
        _activeLevelPack(0),
        _activeLevel(0)
    {
    }
    ~StageManager()
    {
        delete _activeLevelPack;
        _activeLevelPack = 0;
        _activeLevel = 0;
    }

    StageManager( const StageManager&);
    StageManager &operator=(const StageManager&);

    TiXmlDocument *_activeLevelPack;
    TiXmlNode *_activeLevel;
    float _levelStartTime;
    string _activeLevelName;

    list<string> _levelPackList;
    list<string>::iterator _levelPackIterator;

    bool findLevelPacks( void);
    bool loadNextLevelPack( void);
    bool selectLevel();
    bool activateLevel( void);
};

typedef Singleton<StageManager> StageManagerS;

#endif
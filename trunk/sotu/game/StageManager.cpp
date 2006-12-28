// Description:
//   Stage manager controls movement from levelpack to levelpack and
//   level to level.
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
#include <GameState.hpp>
#include <StageManager.hpp>
#include <EnemyFactory.hpp>
#include <Skill.hpp>
#include <XMLHelper.hpp>

#include "Game.hpp"
#include <Random.hpp>
#include <ParticleInfo.hpp>
#include <Particles.hpp>
#include <ParticleGroup.hpp>
#include <ParticleGroupManager.hpp>
//----------------------------------------------------------------------------
EnemyWaves::EnemyWaves():
    _alienTotal(-1), _rebelTotal(0), _empireTotal(0),
    _alienDone(0), _rebelDone(0), _empireDone(0)
{
    _rebelsAreNext = _empireIsNext = false;
}
//----------------------------------------------------------------------------
void EnemyWaves::reset()
{
    _alienDone = _rebelDone = _empireDone = 0;
    // sledeci nivo
    // Odabir protivnika u zavisnosti od toga kakva
    // je planeta u pitanju. Faktori:
    //       1. blizina aliena (0-100%))
    //       2. rebel sentiment (0-100%)
    //       3. rebel status  (neutral=0,unfriendly=1,wanted=2)
    //       4. empire status (neutral=0,unfriendly=1,wanted=2)
    // Algoritam:
    //       broj talasa aliena = 4 + random(2) + aliens/10 = 5+[0-10]
    //       broj talasa rebela = (rebels-50)/10 * rebel_status  = [1-5]*[0-2] = [0-10]
    //       broj talasa empire = (50-rebels)/10 * empire_status = [1-5]*[0-2] = [0-10]
    Planet *p = GameS::instance()->_currentPlanet;
    int rebelStatus = (int)(GameS::instance()->_rebelStatus);
    int empireStatus = (int)(GameS::instance()->_empireStatus);
    _alienTotal  = 4 + Random::integer(2) + (p->_alienActivity / 10);
    _rebelTotal  = ((p->_rebelSentiment - 50) / 10) * rebelStatus;
    _empireTotal = ((50 - p->_rebelSentiment) / 10) * empireStatus;
    if (_rebelTotal > 3)
        _rebelTotal  = _rebelTotal  + 2 - Random::integer(5);
    if (_empireTotal > 3)
        _empireTotal = _empireTotal + 2 - Random::integer(5);

    if (p->_name == "ARMADA")
    {
        _alienTotal = _rebelTotal = 0;
        _empireTotal = 35;
    }
    if (p->_name == "CLOAKED")
    {
        _rebelTotal = _empireTotal = 0;
        _alienTotal = 99;
    }
}
//----------------------------------------------------------------------------
void EnemyWaves::getCounts(int &a, int& e, int& r)
{
    if (_alienTotal == -1)
        a = e = r = 0;
    else
    {
        a = _alienTotal  - _alienDone;
        e = _empireTotal - _empireDone;
        r = _rebelTotal  - _rebelDone;
    }
}
//----------------------------------------------------------------------------
// returns -1 if no more
int EnemyWaves::getNextWave(std::string& name)
{
    if (_alienTotal == -1)     // just in case
        reset();

    if (_alienDone >= _alienTotal && _rebelDone >= _rebelTotal
        && _empireDone >= _empireTotal)
    {
        return -1;
    }

    int type;
    int index;
    char buff[200];
    if (_rebelsAreNext && _rebelDone < _rebelTotal)
    {
        type = 1;
        _rebelsAreNext = false;
    }
    else if (_empireIsNext && _empireDone < _empireTotal)
    {
        type = 2;
        _empireIsNext = false;
    }
    else
    {
        while (true)
        {
            type = Random::integer(3);
            if (type == 0 && _alienDone < _alienTotal ||
                type == 1 && _rebelDone < _rebelTotal ||
                type == 2 && _empireDone < _empireTotal)
            {
                break;
            }
        }
    }

    if (type == 0)
    {
        _alienDone++;
        sprintf(buff, "Alien wave %d of %d", _alienDone, _alienTotal);
        index = 1;
        while (index == 1 || index == 10)
            index = Random::integer(18);
        if (_alienDone == 99)
            index = 19;         // big boss
    }
    else if (type == 1)
    {
        _rebelDone++;
        sprintf(buff, "Rebel wave %d of %d", _rebelDone, _rebelTotal);
        index = 10;
    }
    else // if (type == 2)
    {
        _empireDone++;
        sprintf(buff, "Empire fleet wave %d of %d", _empireDone, _empireTotal);
        index = 1;
    }
    name = buff;
    return index;
}
//----------------------------------------------------------------------------
// STAGE MANAGER *************************************************************
//----------------------------------------------------------------------------
void StageManager::getCounts(int& a, int& e, int&r)
{
    _enemies.getCounts(a, e, r);
}
//----------------------------------------------------------------------------
void StageManager::rebelsAreNext()
{
    _enemies._rebelsAreNext = true;
}
//----------------------------------------------------------------------------
void StageManager::empireIsNext()
{
    _enemies._empireIsNext = true;
}
//----------------------------------------------------------------------------
bool StageManager::init( void)
{
    return findLevelPacks();
}
//----------------------------------------------------------------------------
void StageManager::reset( void)
{
    //catch used skill changes via runtime config
    SkillS::instance()->updateSkill();

    _levelPackIterator = _levelPackList.begin();
    loadNextLevelPack();
    // planet was probably changed so we need to recalculate
    _enemies.reset();
    selectLevel();
    activateLevel();
}
//----------------------------------------------------------------------------
void StageManager::update( void)
{   // pobio sve protivnike
    if( GameState::numObjects == 0)
    {
        if (selectLevel())
            activateLevel();
        else
        {
            if (GameS::instance()->_hyperspaceCount != 0)
                return;
            float& ssa = GameS::instance()->_spaceStationApproach;
            if (ssa == 0)
                ssa = GameState::stopwatch.getTime();
            else
            {
                float ssadiff = GameState::stopwatch.getTime() - ssa;
                if (ssadiff > 7.0f)     // reached space station
                {
                    ssa = 0;
                    GameS::instance()->_landed = true;
                    GameS::instance()->_galaxy.update();
                    if (GameS::instance()->_currentPlanet->isSpecial())
                        GameS::instance()->reachedSpecialPlanet();
                    else
                    {
                        GameS::instance()->switchContext(ePlanetMenu);
                        // don't allow to go back
                        GameS::instance()->setPreviousContext(ePlanetMenu);
                        static int counter = 0;
                        if (counter++ > 10)
                        {
                            counter = 0;
                            SkillS::instance()->incrementSkill();
                        }
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
// vraca false ako je dosta bilo
bool StageManager::selectLevel()
{
    int index = _enemies.getNextWave(_activeLevelName);
    if (index == -1)
        return false;

    TiXmlNode *tmp = _activeLevel;
    while (tmp->PreviousSibling())      // find first
        tmp = tmp->PreviousSibling();
    for (int i=0; i<index; i++)         // move to Nth
        tmp = tmp->NextSibling();
    _activeLevel = tmp;
    return true;
}
//----------------------------------------------------------------------------
bool StageManager::findLevelPacks( void)
{
    list<string> rList;
    ResourceManagerS::instance()->getResourceList( rList);

    list<string>::iterator i;
    for( i=rList.begin(); i!=rList.end(); i++)
    {
        string resourceName = *i;

        string::size_type end = resourceName.length()-8; // strlen("Pack.xml")
        string::size_type find = resourceName.find("Pack.xml");
        if( (find!=string::npos) && (find==end))
        {
            LOG_INFO << "Adding LevelPack [" << resourceName << "]" << endl;
            _levelPackList.insert( _levelPackList.end(), resourceName);
        }
    }

    _levelPackIterator = _levelPackList.begin();

    bool foundSome = _levelPackIterator != _levelPackList.end();
    if( !foundSome)
    {
        LOG_WARNING << "No levelpacks found in resource file!" << endl;
        string defaultLevelPack = "levelpacks/CritterPack.xml";
        _levelPackList.insert( _levelPackList.end(), defaultLevelPack);
    }

    return true;
}
//----------------------------------------------------------------------------
bool StageManager::loadNextLevelPack( void)
{
    XTRACE();

    if (_levelPackIterator == _levelPackList.end())
        _levelPackIterator = _levelPackList.begin();
    delete _activeLevelPack;

    string levelPackName = *_levelPackIterator;
    _activeLevelPack = XMLHelper::load(levelPackName);

    if( !_activeLevelPack)
    {
        _activeLevel = 0;
        return false;
    }

    TiXmlNode *levelPack = _activeLevelPack->FirstChild("LevelPack");
    _activeLevel = levelPack->ToElement()->FirstChild();

    //advance to next level pack
    _levelPackIterator++;

    if (!_activeLevelPack)
    {
        LOG_ERROR << "No level pack found!" << endl;
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool StageManager::activateLevel()
{
    XTRACE();

    if (!_activeLevel)
        return false;

    LOG_INFO << "Level '" << _activeLevelName << endl;

    static ParticleGroup *effects =
        ParticleGroupManagerS::instance()->getParticleGroup(EFFECTS_GROUP2);
    ParticleInfo pi;
    pi.position.x =  0.0;
    pi.position.y =  0.0;
    pi.position.z =-50.0;
    pi.text = _activeLevelName;

    pi.color.x = 1.0;
    pi.color.y = 1.0;
    pi.color.z = 1.0;

    pi.extra.y = 0.1f;
    pi.extra.z = 0.1f;

    effects->newParticle("StatusMessage", pi);

    GameState::numObjects = 0;
    _levelStartTime = GameState::stopwatch.getTime();

    TiXmlNode *enemyNode = _activeLevel->FirstChild();
    while( enemyNode)
    {
        EnemyFactory::createEnemy( enemyNode);
        enemyNode = enemyNode->NextSibling();
    }

    return true;
}
//----------------------------------------------------------------------------

// Description:
//   High level infrastructure for game.
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
#include "utils/Random.hpp"

#include <Game.hpp>
#include <GameState.hpp>
#include <Constants.hpp>
#include <Config.hpp>
#include <PausableTimer.hpp>
#include <ScoreKeeper.hpp>

#include <Audio.hpp>
#include <Video.hpp>
#include <Input.hpp>

#include <Hero.hpp>
#include <WeaponDepot.hpp>
#include <ParticleGroup.hpp>
#include <ParticleGroupManager.hpp>
#include <Starfield.hpp>

#include <LPathManager.hpp>
#include <ModelManager.hpp>
#include <StageManager.hpp>
#include <MenuManager.hpp>
#include <ResourceManager.hpp>
//----------------------------------------------------------------------------
Game::Game( void)
{
    XTRACE();
}
//----------------------------------------------------------------------------
Game::~Game()
{
    XTRACE();

    LOG_INFO << "Shutting down..." << endl;

#if 1
    // save config stuff
    ConfigS::instance()->saveToFile();

    // save leaderboard
    ScoreKeeperS::instance()->save();
#endif

    StarfieldS::cleanup();

    // misc cleanup
    MenuManagerS::cleanup();
    PlanetManagerS::cleanup();
    StageManagerS::cleanup();
    ModelManagerS::cleanup();
    InputS::cleanup();
    AudioS::cleanup();
    VideoS::cleanup();  //Video calls SDL_Quit
    LPathManagerS::cleanup();
    ParticleGroupManagerS::cleanup();
    WeaponDepotS::cleanup();
    ResourceManagerS::cleanup();
    ScoreKeeperS::cleanup();

    HeroS::cleanup(); //has to be after ParticleGroupManager
}
//----------------------------------------------------------------------------
bool Game::init( void)
{
    XTRACE();
    bool result = true;

    ScoreKeeperS::instance()->load();

    // init subsystems et al
    if( ! ParticleGroupManagerS::instance()->init()) return false;
    if( ! AudioS::instance()->init()) return false;
    if( ! VideoS::instance()->init()) return false;
    if( ! InputS::instance()->init()) return false;
    if( ! MenuManagerS::instance()->init()) return false;
    if( ! PlanetManagerS::instance()->init()) return false;
    if( ! HeroS::instance()->init()) return false;

    StarfieldS::instance()->init( -150.0);

    ParticleGroupManager *pgm = ParticleGroupManagerS::instance();
    if( ! pgm->init()) return false;

    //init all the paricle groups and links between them
    pgm->addGroup( HERO_GROUP, 1);
    pgm->addGroup( ENEMIES_GROUP, 300);
    pgm->addGroup( ENEMIES_BULLETS_GROUP, 300);

    //there are 3 effect groups to give very simple control over the order
    //of drawing which is important for alpha blending.
    pgm->addGroup( EFFECTS_GROUP1, 1000);
    pgm->addGroup( EFFECTS_GROUP2, 1000);
    pgm->addGroup( EFFECTS_GROUP3, 1000);

    pgm->addGroup( HERO_BULLETS_GROUP, 300);
    pgm->addGroup( BONUS_GROUP, 50);
    pgm->addGroup( SHOOTABLE_ENEMY_BULLETS_GROUP, 100);
    pgm->addGroup( SHOOTABLE_BONUS_GROUP, 50);

    //collision detect between the following groups
    pgm->addLink( HERO_BULLETS_GROUP, SHOOTABLE_ENEMY_BULLETS_GROUP);
    pgm->addLink( HERO_BULLETS_GROUP, ENEMIES_GROUP);
    pgm->addLink( HERO_BULLETS_GROUP, SHOOTABLE_BONUS_GROUP);
    pgm->addLink( HERO_GROUP, ENEMIES_GROUP);
    pgm->addLink( HERO_GROUP, ENEMIES_BULLETS_GROUP);
    pgm->addLink( HERO_GROUP, BONUS_GROUP);
    pgm->addLink( HERO_GROUP, SHOOTABLE_ENEMY_BULLETS_GROUP);
    pgm->addLink( HERO_GROUP, SHOOTABLE_BONUS_GROUP);

    //reset our stopwatch
    GameState::stopwatch.reset();
    GameState::stopwatch.pause();

    if( ! StageManagerS::instance()->init()) return false;

    ConfigS::instance()->getFloat( "horsePower", GameState::horsePower);

    //add our hero...
    ParticleGroupManagerS::instance()->
        getParticleGroup( HERO_GROUP)->newParticle( string("Hero"), 0, 0, -100);

    LOG_INFO << "Initialization complete OK." << endl;

    return result;
}
//----------------------------------------------------------------------------
void Game::reset( void)
{
    //reset in order to start new game
    ParticleGroupManagerS::instance()->reset();
    HeroS::instance()->reset();
    ParticleGroupManagerS::instance()->
        getParticleGroup( HERO_GROUP)->newParticle( string("Hero"), 0, 0, -100);

    ScoreKeeperS::instance()->resetCurrentScore();
    GameState::stopwatch.reset();
    GameState::startOfGameStep = GameState::stopwatch.getTime();
    StageManagerS::instance()->reset();
}
//----------------------------------------------------------------------------
void Game::startNewGame( void)
{
    GameS::instance()->reset();
    GameState::context = Context::eInGame;
    InputS::instance()->disableInterceptor();
    GameState::stopwatch.start();
    AudioS::instance()->playSample( "sounds/voiceGo.wav");

    bool allowVerticalMovement = false;
    ConfigS::instance()->getBoolean( "allowVerticalMovement", allowVerticalMovement);
    HeroS::instance()->allowVerticalMovement(allowVerticalMovement);
}
//----------------------------------------------------------------------------
void Game::startNewCampaign()
{
    _cargo.clearCargo();

    // reset player stats (kills, empStatus, rebelStatus)
    // reset campaign data, Chapter = 0
    // reset quests

    PlanetManagerS::instance()->enable();
}
//----------------------------------------------------------------------------
void Game::updateOtherLogic( void)
{
    int stepCount = 0;
    float currentTime = Timer::getTime();
    while( (currentTime - GameState::startOfStep) > GAME_STEP_SIZE)
    {
        StarfieldS::instance()->update();

        if (GameState::context == Context::eMenu)
            MenuManagerS::instance()->update();

        if (GameState::context == Context::ePlanetMenu)
            PlanetManagerS::instance()->update();

        //advance to next start-of-game-step point in time
        GameState::startOfStep += GAME_STEP_SIZE;
        currentTime = Timer::getTime();

        stepCount++;
        if( stepCount > MAX_GAME_STEPS)
            break;
    }

    GameState::frameFractionOther =
    (currentTime - GameState::startOfStep) / GAME_STEP_SIZE;

    if( stepCount > 1)
    {
        LOG_WARNING << "Skipped " << stepCount << " frames." << endl;

        if( GameState::frameFractionOther > 1.0)
        {
            //Our logic is still way behind where it should be at this
            //point in time. If we get here we already ran through
            //MAX_GAME_STEPS logic runs trying to catch up.

            //We clamp the value to 1.0, higher values would try
            //to predict were we are visually. Maybe not a bad idea
            //to allow values up to let's say 1.3...
            GameState::frameFractionOther = 1.0;
        }
    }
}
//----------------------------------------------------------------------------
void Game::updateInGameLogic( void)
{
    int stepCount = 0;
    float currentGameTime = GameState::stopwatch.getTime();
    while( (currentGameTime - GameState::startOfGameStep) > GAME_STEP_SIZE)
    {
        // adding new enemies, moving on to the next level, etc.
        StageManagerS::instance()->update();
        // update all objects, particles, etc.
        ParticleGroupManagerS::instance()->update();

        //FIXME: Currently the Critterboard is updated in the video system. Should be on its own.
        VideoS::instance()->updateLogic();

        //advance to next start-of-game-step point in time
        GameState::startOfGameStep += GAME_STEP_SIZE;
        currentGameTime = GameState::stopwatch.getTime();

        stepCount++;
        if( stepCount > MAX_GAME_STEPS)
            break;
    }

    GameState::frameFraction =
    (currentGameTime - GameState::startOfGameStep) / GAME_STEP_SIZE;

    if( stepCount > 1)
    {
        if( GameState::frameFraction > 1.0)
        {
            //Our logic is still way behind where it should be at this
            //point in time. If we get here we already ran through
            //MAX_GAME_STEPS logic runs trying to catch up.

            //We clamp the value to 1.0, higher values would try
            //to predict were we are visually. Maybe not a bad idead
            //to allow values up to let's say 1.3...
            GameState::frameFraction = 1.0;
        }
    }
}
//----------------------------------------------------------------------------
void Game::run( void)
{
    XTRACE();

    Audio &audio = *AudioS::instance();
    Video &video = *VideoS::instance();
    Input &input = *InputS::instance();

    //make sure we start of in menu mode
    MenuManagerS::instance()->turnMenuOn();

    // Here it is: the main loop.
    LOG_INFO << "Entering Main loop." << endl;

    GameState::startOfStep = Timer::getTime();
    GameState::startOfGameStep = GameState::stopwatch.getTime();
    while( GameState::isAlive)
    {
        switch( GameState::context)
        {
            case Context::eInGame:
                //stuff that only needs updating when game is actually running
                updateInGameLogic();
            break;

            /* moved to updateOtherLogic()
            case Context::ePlanetMenu:
                // TODO: update planet menu logic?
                break;
            */

            default:
            break;
        }

        //stuff that should run all the time
        updateOtherLogic();

        input.update();
        audio.update();
        video.update();
    }
}
//----------------------------------------------------------------------------
// CARGO *********************************************************************
//----------------------------------------------------------------------------
void Cargo::addItem(std::string groupName, const CargoItem& c)
{
    _items.insert(std::pair<std::string,CargoItem>(groupName, c));
}
//----------------------------------------------------------------------------
void Cargo::clearCargo()
{
    _items.clear();

    // create empty cargo list (used for trading as well)
    // when player visits planet for first time, the list is built

    // Limited by cargo space (0 - total)
    addItem("Goods", CargoItem("Food"));
    addItem("Goods", CargoItem("Electronics"));
    addItem("Goods", CargoItem("Clothing"));
    addItem("Goods", CargoItem("Alien artifacts"));
    addItem("Goods", CargoItem("Jewelry"));
    addItem("Goods", CargoItem("Firearms"));   // * illegal with empire
    addItem("Goods", CargoItem("Narcotics"));  // * illegal everywhere
    addItem("Goods", CargoItem("Slaves"));     // * illegal with rebels

    // Don't take cargo space, Min = 0, Max = 1
    addItem("Equipment", CargoItem("Rocket launcher"));
    addItem("Equipment", CargoItem("Neutron gun"));
    addItem("Equipment", CargoItem("Neutron gun enhancer"));
    addItem("Equipment", CargoItem("Proton flank burst"));
    addItem("Equipment", CargoItem("Proton enhancer"));
    addItem("Equipment", CargoItem("Wave emitter"));
    addItem("Equipment", CargoItem("Smart bomb"));
    addItem("Equipment", CargoItem("Vertical thrusters"));
    // special case, Min = 0, Max = 9
    addItem("Equipment", CargoItem("Fuel"));

    // special case
    addItem("Other", CargoItem("Save game"));
}
//----------------------------------------------------------------------------
// PLANET ********************************************************************
//----------------------------------------------------------------------------
Planet::Planet(float x, float y, const std::string& name)
    :_x(x), _y(y)
{
    if (name == "XEN")
    {
        _name = name;
        _radius = 60;
        //_hasRing = false;
        _techLevel = 9;
        _rebelSentiment = 0;
        _alienActivity = 5;
        _textureIndex = 3;
        return;
    }
    int rnd = Random::integer(10);
    if (rnd < 8)
        _radius = 50 + 2 * rnd;
    else if (rnd == 8)
        _radius = 45;
    else
        _radius = 70;
    _textureIndex = Random::integer(PLANET_TEXTURES);
    _techLevel = 1+Random::integer(9);
    const float mapWidth = 760.0f;
    const float mapHeight = 600.0f;
    if (x < mapWidth*0.3 && y < mapHeight * 0.2)
    {
        _rebelSentiment = 0;
        _alienActivity = 10 + Random::integer(10);
    }
    else if (x > mapWidth*0.7 && y > mapHeight * 0.2)
    {
        _rebelSentiment = 80 + Random::integer(20);
        _alienActivity = 95 + Random::integer(5);
    }
    else    // in between
    {
        float base = 40.0f * (x/mapWidth + y/mapHeight);
        _rebelSentiment = (int)base + Random::integer(20);
        _alienActivity = 10 + (int)base + Random::integer(10);
    }
    if (name != "")
        _name = name;
    else    // generate random name
    {
        char one[] = "BCDFGJKLMNPRSTZ";
        char two[] = "AEIOU";
        std::string three[] = { "TH", "R", "ST", "S", "T", "RAM", "N", "NT", "NIR" };
        char name[10];
        name[0] = one[Random::integer(sizeof(one)/sizeof(char) - 1)];   // -1 because of string terminator char
        name[1] = two[Random::integer(sizeof(two)/sizeof(char) - 1)];
        name[2] = one[Random::integer(sizeof(one)/sizeof(char) - 1)];
        name[3] = two[Random::integer(sizeof(two)/sizeof(char) - 1)];
        name[4] = '\0';
        _name = name;
        if (Random::integer(10) > 5)
            _name += three[Random::integer(sizeof(three)/sizeof(std::string))];
    }

    // TODO: generate starting marketplace
    //LOG_INFO << "CREATED PLANET (" << _name.c_str() << ") x = " << x << ", y = " << y << endl;
}
//----------------------------------------------------------------------------
float Planet::distance(float x, float y)
{
    // x^2+y^2
    return (x-_x)*(x-_x) + (y-_y)*(y-_y);
}
//----------------------------------------------------------------------------
bool Planet::isAt(float x, float y)                // allow few pixels miss
{
    const float range = 3;
    return (x >= _x-range) && (x <= _x + range)
        && (y >= _y-range) && (y <= _y + range);
}
//----------------------------------------------------------------------------
float Planet::getPrice(const std::string& itemName)
{
    //return _marketplace._items[itemName]._price;
    return 0;
}
//----------------------------------------------------------------------------
// called once each 5 turns or so
void Planet::update()
{
    // always rs=0, tl=9, aa=0
    if (_name == "XEN")
        return;
    int rs_before = _rebelSentiment;
    if (_rebelSentiment > 20)
        _rebelSentiment += -5 + Random::integer(11);
    if (_rebelSentiment == 50)
        _rebelSentiment = rs_before;
    if (_rebelSentiment > 100)
        _rebelSentiment = 100;
    if (_rebelSentiment < 0)
        _rebelSentiment = 0;
    if (rs_before < 50 && _rebelSentiment > 50) // civil war - rebels take over
    {
        _rebelSentiment = 70;
        // TODO: NewsManagerS::instance()->addItem("Civil war in " + _name + ". Rebels take over the planet.");
        if (_techLevel > 1)
        {
            _techLevel -= 4;
            // TODO: NewsManagerS::instance()->addItem("Civil war in " + _name + ". Technology level reduced.");
        }
    }
    if (rs_before > 50 && _rebelSentiment < 50)
    {
        _rebelSentiment = 40;
        _techLevel -= 2;
        // TODO: NewsManagerS::instance()->addItem("Empire has taken " + _name + " into their power.");
    }
    if (_techLevel < 1)
        _techLevel = 1;
    if (_techLevel < 9 && _rebelSentiment > 70 || _rebelSentiment < 30)   // planet of stable government
    {
        _techLevel += 0.1f;
        //if ((float)((int)_techLevel) == _techLevel)
        //    NewsManagerS::instance()->addItem("Tech. level of " + _name + " has increased.");

        if (_techLevel > 9)
            _techLevel = 9;
    }

    // TODO: update prices
}
//----------------------------------------------------------------------------
// PLANET ********************************************************************
//----------------------------------------------------------------------------
Map::Map()  // creates galaxy of planets
{
    LOG_INFO << "CREATING PLANETS" << endl;
    const int mapWidth = 760;
    const int mapHeight = 600;
    const int stepSize = 20;
    for (int x = 0; x < mapWidth; x += stepSize)
    {
        for (int y = 0; y < mapHeight; y += stepSize)
        {
            Planet *p;
            if (x == stepSize && y == stepSize)   // XEN
                p = new Planet(1.5*(float)stepSize, 1.5*(float)stepSize, "XEN");
            else
            {
                if ((x+y)%50 == 0 && Random::integer(5) == 0)
                    continue;
                p = new Planet( x + 2 + Random::integer(stepSize-4),
                                y + 2 + Random::integer(stepSize-4));
            }
            _planets.push_back(p);
        }
    }
}
//----------------------------------------------------------------------------
Map::~Map()
{
    LOG_INFO << "DESTROYING PLANETS" << endl;
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
        delete (*it);
}
//----------------------------------------------------------------------------
// renders galaxy as set of points
void Map::draw(float x, float y)
{
    glPointSize(2.0f);
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);    // Specify point drawing
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
    {
        switch ((*it)->_textureIndex)
        {
            case 0: case 5:
                glColor3f(1.0, 1.0, 0.0); break;
            case 3: case 6:
                glColor3f(0.0, 0.8, 1.0); break;
            case 2: case 4:
                glColor3f(1.0, 0.4, 0.3); break;
            case 1: default:
                glColor3f(1.0, 1.0, 1.0); break;
        };
        //if ((*it)->isSpecial())
        // bigger or flashing or whaever
        glVertex3f(x+(*it)->_x, y+(*it)->_y, 0.0f);
    }
    glEnd();
}
//----------------------------------------------------------------------------
Planet* Map::getPlanetAt(float x, float y)
{
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
        if ((*it)->isAt(x, y))
            return (*it);
    return 0;
}
//----------------------------------------------------------------------------
Planet* Map::getNearest(float x, float y)
{
    float mindist = 0.0f;
    Planet *retval = 0;
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
    {
        float d = (*it)->distance(x, y);
        if (d < mindist || it == _planets.begin())
        {
            mindist = d;
            retval = (*it);
        }
    }
    return retval;
}
//----------------------------------------------------------------------------

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
#include <math.h> //we need cos(..) and sin(..) and sqrt(..)
#include <Trace.hpp>
#include "utils/Random.hpp"

#include <Game.hpp>
#include <GameState.hpp>
#include <Constants.hpp>
#include <Config.hpp>
#include <PausableTimer.hpp>
#include <ScoreKeeper.hpp>
#include <Selectable.hpp>

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
Game::Game(void)
    :_currentPlanet(0), _context(eUnknown), _previousContext(eUnknown)
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

    if (!StageManagerS::instance()->init())
        return false;

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
void Game::startNewGame()
{
    GameS::instance()->reset();
    switchContext(eInGame);
    AudioS::instance()->playSample( "sounds/voiceGo.wav");

    _landed = false;

    bool allowVerticalMovement = false;
    ConfigS::instance()->getBoolean( "allowVerticalMovement", allowVerticalMovement);
    HeroS::instance()->allowVerticalMovement(allowVerticalMovement);
}
//----------------------------------------------------------------------------
void Game::startNewCampaign()
{
    _cargo.clear();
    _cargo.create(0);   // create empty cargo bay
    _cargo.findItem("Fuel")->_quantity += 30;
    _cargo.findItem("Proton spread fire")->_quantity += 1;
    _money = 2000;
    _landed = true;
    _galaxy.recreate();
    _empireStatus = _rebelStatus = psClean;
    _kills = 0;

    // TODO: reset quests

    switchContext(ePlanetMenu);
}
//----------------------------------------------------------------------------
void Game::updateOtherLogic()
{
    int stepCount = 0;
    float currentTime = Timer::getTime();
    while( (currentTime - GameState::startOfStep) > GAME_STEP_SIZE)
    {
        StarfieldS::instance()->update();

        if (_context == eMenu)
            MenuManagerS::instance()->update();

        if (_context == ePlanetMenu)
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
void Game::updateInGameLogic()
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

    //make sure we start off in menu mode
    switchContext(eMenu);

    // Here it is: the main loop.
    LOG_INFO << "Entering Main loop." << endl;

    GameState::startOfStep = Timer::getTime();
    GameState::startOfGameStep = GameState::stopwatch.getTime();
    while( GameState::isAlive)
    {
        //stuff that only needs updating when game is actually running
        if (_context == eInGame)
            updateInGameLogic();

        //stuff that should run all the time
        updateOtherLogic();

        input.update();
        audio.update();
        video.update();
    }
}
//----------------------------------------------------------------------------
ContextEnum Game::getContext()
{
    return _context;
}
//----------------------------------------------------------------------------
void Game::switchContext(ContextEnum c)
{
    if (_context == c)  // nothing to change
        return;

    if (c != eInGame)
        AudioS::instance()->playSample( "sounds/humm.wav");

    if (c == eInGame)               // entering game mode
        GameState::stopwatch.start();
    else if (_context == eInGame)   // leaving game mode
        GameState::stopwatch.pause();
    else
        Selectable::reset();

    if (_context == ePaused)
    {
        SDL_ShowCursor(SDL_DISABLE);
        SDL_WM_GrabInput(SDL_GRAB_ON);
    }

    if (c == ePlanetMenu)
        InputS::instance()->enableInterceptor(PlanetManagerS::instance());
    else if (c == eMenu)
        InputS::instance()->enableInterceptor(MenuManagerS::instance());
    else
        InputS::instance()->disableInterceptor();

    _previousContext = _context;
    _context = c;
}
//----------------------------------------------------------------------------
void Game::previousContext()
{
    if (_previousContext != eUnknown)
        switchContext(_previousContext);
}
//----------------------------------------------------------------------------
void Game::hyperspaceJump()
{
    Planet *target = PlanetManagerS::instance()->_hyperspaceTarget;
    float dist = _currentPlanet->getDistance(target->_x, target->_y);
    CargoItem* fuel = _cargo.findItem("Fuel");
    fuel->_quantity -= (int)(dist + 0.92);
    _currentPlanet = target;
    startNewGame();
}
//----------------------------------------------------------------------------
// CARGO *********************************************************************
//----------------------------------------------------------------------------
std::vector<CargoItemInfo>* CargoItemInfo::getCargoInfo()
{
    static std::vector<CargoItemInfo> info;
    if (info.empty())
    {
        info.push_back(CargoItemInfo(4.0f, "models/Pumpkin", "Food",                   1,   50, 1,  0, pmProTech,
            "Cheaper on planets with lower tech level"));
        info.push_back(CargoItemInfo(6.0f, "models/ArmorPierce", "Electronics",        5,  195, 1,  0, pmContraTech,
            "Cheaper on planets with higher tech level"));
        info.push_back(CargoItemInfo(5.0f, "models/SuperBonus", "Alien artifacts",    -1,   20,-1,  0, pmNormal,
            "Some of the aliens you shoot might drop it"));
        info.push_back(CargoItemInfo(6.0f, "models/Bonus1", "Jewelry",                 3,  400, 1,  0, pmRandom,
            "Price of this item varies randomly"));
        info.push_back(CargoItemInfo(1.0f, "GUN", "Firearms",                          4,  300, 1,  0, pmNormal,
            "Trading this item is illegal on Empire planets", lsiEmpire));
        info.push_back(CargoItemInfo(2.0f, "models/Boss1_Teeth", "Narcotics",          3,  395, 1,  0, pmProTech,
            "Trading this item is illegal on all planets",    lsiBoth));
        info.push_back(CargoItemInfo(5.0f, "models/Boss1_Eye1", "Slaves",              1,  195, 1,  0, pmProTech,
            "Trading this item is illegal on Rebel planets",  lsiRebels));

        info.push_back(CargoItemInfo(1.0f, "models/IceSpray", "Proton spread fire",    3,  400, 0,  1, pmNormal,
            "Secondary weapon - use right mouse button or CTRL key"));
        info.push_back(CargoItemInfo(1.0f, "models/IceSprayPierce", "Proton enhancer", 6,  500, 0,  1, pmNormal,
            "Enhances primary and secondary weapon power"));
        info.push_back(CargoItemInfo(0.8f, "models/FlankBurster", "Wave emitter",      8, 1200, 0,  1, pmNormal,
            "Tertiary weapon - use middle mouse button or ALT key"));
        info.push_back(CargoItemInfo(5.0f, "models/WeaponUpgrade", "Smart bomb",       8, 3000, 0, 10, pmNormal,
            "Destroys all nearby enemies - press key D to detonate"));
        info.push_back(CargoItemInfo(1.0f, "models/Stinger", "Stinger rocket",         3,  500, 0, 20, pmNormal,
            "Ammo for rocket launcher - press key F to fire"));
        info.push_back(CargoItemInfo(3.0f, "models/ShieldBoost", "Shield upgrade",     7, 2000, 0,  1, pmNormal,
            "Allows your ship to have 200 shield energy"));
        info.push_back(CargoItemInfo(1.5f, "models/Boss1_Leg1", "Fuel",                1,   20, 0, 40, pmNormal,
            "Needed for hyperspace travel between planets"));
    }
    return &info;
}
//----------------------------------------------------------------------------
void Cargo::create(Planet *p)
{
    if (!_items.empty())
        return;

    std::vector<CargoItemInfo>* info = CargoItemInfo::getCargoInfo();
    for (std::vector<CargoItemInfo>::iterator it = info->begin();
        it != info->end(); ++it)
    {
        float price = (*it)._basePrice;
        if ((*it)._priceModel == CargoItemInfo::pmRandom)
            price = price*0.6 + Random::integer((int)(price*0.8f));
        else if ((*it)._priceModel != CargoItemInfo::pmNormal)
            price += Random::integer(10);
        if (p)
        {
            bool illegal =
                ((*it)._legalStatus == CargoItemInfo::lsiEmpire && p->_rebelSentiment < 50 ||
                 (*it)._legalStatus == CargoItemInfo::lsiRebels && p->_rebelSentiment > 50);
            float multiply = (illegal ? 2.0f : 1.0f);
            float tleffect = ((*it)._legalStatus == CargoItemInfo::lsiBoth ? 3.0f : 1.0f );
            if ((*it)._priceModel == CargoItemInfo::pmProTech)
                multiply += ((p->_techLevel * tleffect - 5.0f)*0.1f); // 0.6 - 1.4 of original price
            if ((*it)._priceModel == CargoItemInfo::pmContraTech)
                multiply -= ((p->_techLevel - 5.0f)*0.1f); // 0.6 - 1.4 of original price
            price *= multiply;
        }
        _items.push_back(CargoItem((*it)._name, 0, (int)price));
    }
}
//----------------------------------------------------------------------------
CargoItem *Cargo::findItem(const std::string& itemName)
{
    for (std::vector<CargoItem>::iterator it = _items.begin();
        it != _items.end(); ++it)
    {
        if ((*it)._name == itemName)
            return &(*it);
    }
    LOG_WARNING << "CANNOT FIND CARGO ITEM " << itemName.c_str() << endl;
    static CargoItem dummy("MUNGIE", 10, 1000); // let's not crash
    return &dummy;
}
//----------------------------------------------------------------------------
void Cargo::clear()
{
    _items.clear();
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

    //LOG_INFO << "CREATED PLANET (" << _name.c_str() << ") x = " << x << ", y = " << y << endl;
}
//----------------------------------------------------------------------------
bool Planet::isAt(float x, float y)                // allow few pixels miss
{
    const float range = 3;
    return (x >= _x-range) && (x <= _x + range)
        && (y >= _y-range) && (y <= _y + range);
}
//----------------------------------------------------------------------------
float Planet::getDistance(float x, float y)
{
    return sqrt((x-_x)*(x-_x) + (y-_y)*(y-_y));
}
//----------------------------------------------------------------------------
int Planet::getPrice(const std::string& itemName)
{
    _marketplace.create(this);
    CargoItem *c = _marketplace.findItem(itemName);
    if (c)
        return c->_price;
    return 0;
}
//----------------------------------------------------------------------------
Planet::BuyStatus Planet::canBuy(CargoItemInfo& item)
{
    int tlr = item._techLevelRequired;
    if (tlr == -1)
        return bsNA;
    else if (tlr > _techLevel)
        return bsNoTech;
    else if (GameS::instance()->_money < getPrice(item._name))
        return bsNoMoney;
    return bsOk;
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

    // update prices
    _marketplace.clear();
    _marketplace.create(this);
}
//----------------------------------------------------------------------------
// PLANET ********************************************************************
//----------------------------------------------------------------------------
void Map::recreate()
{
    deletePlanets();
    LOG_INFO << "CREATING PLANETS" << endl;
    const int mapWidth = 760;
    const int mapHeight = 600;
    const int stepSize = 20;
    for (int x = 0; x < mapWidth; x += stepSize)
    {
        for (int y = 0; y < mapHeight; y += stepSize)
        {
            Planet *p;
            if (x == stepSize && y == stepSize)   // XEN (cell=1,1)
            {
                p = new Planet(1.5*(float)stepSize, 1.5*(float)stepSize, "XEN");
                PlanetManagerS::instance()->_hyperspaceTarget = p;
                GameS::instance()->_currentPlanet = p;
            }
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
void Map::deletePlanets()
{
    LOG_INFO << "DESTROYING PLANETS" << endl;
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
        delete (*it);
    _planets.clear();
}
//----------------------------------------------------------------------------
Map::~Map()
{
    deletePlanets();
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
    glDisable(GL_POINT_SMOOTH);

    Planet *p = GameS::instance()->_currentPlanet;
    if (p)
    {
        //glPointSize(1.0f);
        glColor4f(0.6f, 1.0f, 0.5f, 0.4f);
        float w = VideoS::instance()->getWidth();
        w = w / (float)VIDEO_ORTHO_WIDTH;
        float h = VideoS::instance()->getHeight();
        h = h / (float)VIDEO_ORTHO_HEIGHT;
        glScissor((int)(w*212.0f), (int)(h*62.0f), (int)(w*768.0f), (int)(h*608.0f));
        glEnable(GL_SCISSOR_TEST);
        glBegin(GL_POLYGON);
        CargoItem *ci = GameS::instance()->_cargo.findItem("Fuel");
        float radius = ci->_quantity;
        for (float i = 0.0f; i < 360.0f; i+=5.0f)
        {
            float degInRad = i * 0.017453278f;
            glVertex2f( x + p->_x + cos(degInRad) * radius,
                        y + p->_y + sin(degInRad) * radius);
        }
        glEnd();
        glDisable(GL_SCISSOR_TEST);
    }
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
    // instead of real distance we use simple calc to aviod sqrt
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
    {   // (_x-x)^2 + (_y-y)^2
        float d = ((*it)->_x-x)*((*it)->_x-x) + ((*it)->_y-y)*((*it)->_y-y);
        if (d < mindist || it == _planets.begin())
        {
            mindist = d;
            retval = (*it);
        }
    }
    return retval;
}
//----------------------------------------------------------------------------

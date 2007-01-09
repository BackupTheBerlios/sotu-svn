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
#include "SDL.h"
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

#define DEMO
//#define TRAINER
//----------------------------------------------------------------------------
Game::Game(void):
    _currentPlanet(0), _spaceStationApproach(0),
    _context(eUnknown), _previousContext(eUnknown)
{
    XTRACE();
}
//----------------------------------------------------------------------------
Game::~Game()
{
    XTRACE();

    LOG_INFO << "Shutting down..." << endl;

    // save config stuff
    ConfigS::instance()->saveToFile();

    StarfieldS::cleanup();

    // misc cleanup
    MenuManagerS::cleanup();
    PlanetManagerS::cleanup();
    StageManagerS::cleanup();
    ModelManagerS::cleanup();
    MessageBoxManagerS::cleanup();
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

    ScoreKeeperS::instance();   // init

    // init subsystems et al
    if( ! ParticleGroupManagerS::instance()->init()) return false;
    if( ! AudioS::instance()->init()) return false;
    if( ! VideoS::instance()->init()) return false;
    if( ! InputS::instance()->init()) return false;
    if( ! MenuManagerS::instance()->init()) return false;
    if( ! PlanetManagerS::instance()->init()) return false;
    if( ! MessageBoxManagerS::instance()->init()) return false;
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
void Game::reset()
{
    ParticleGroupManagerS::instance()->reset();
    // don't change energy if we come from hyperspaceJump
    HeroS::instance()->reset(_hyperspaceCount != 0);
    ParticleGroupManagerS::instance()->
        getParticleGroup(HERO_GROUP)->newParticle( string("Hero"), 0, 0, -100);

    GameState::stopwatch.reset();
    GameState::startOfGameStep = GameState::stopwatch.getTime();
    StageManagerS::instance()->reset();
}
//----------------------------------------------------------------------------
std::string Game::getReputation(bool ucase)
{
    int rep_kills[] = { 0, 500, 1000, 2500, 4500, 6500, 9000 };
    std::string reputation[] = { "Harmless", "Poor", "Average", "Competent",
        "Dangerous", "Deadly", "Elite" };
    std::string urep[] = { "HARMLESS", "POOR", "AVERAGE", "COMPETENT",
        "DANGEROUS", "DEADLY", "ELITE" };
    std::string reps;
    for (unsigned int i = 0; i < sizeof(reputation)/sizeof(string); ++i)
        if (_kills >= rep_kills[i])
            reps = (ucase ? urep[i] : reputation[i]);
    return reps;
}
//----------------------------------------------------------------------------
void Game::startNewGame()
{
    reset();
    _hyperspaceCount = 0;
    switchContext(eInGame);
    AudioS::instance()->playSample("sounds/voiceGo.wav");
    _landed = false;

    bool allowVerticalMovement = true;
    ConfigS::instance()->getBoolean("allowVerticalMovement", allowVerticalMovement);
    HeroS::instance()->allowVerticalMovement(allowVerticalMovement);
}
//----------------------------------------------------------------------------
void Game::startNewCampaign()
{
    _cargo.clear();
    _cargo.create(0);   // create empty cargo bay
#ifdef TRAINER
    _cargo.findItem("Fuel")->_quantity += 300000;
    _cargo.findItem("Space grenade")->_quantity += 2000;
#else
    _cargo.findItem("Fuel")->_quantity += 30;
#endif
    _money = 2000;
    _landed = true;
    _galaxy.recreate();
    _questTargets.clear();
    _questTargets.push_back("TORRES");
    _chapter = 1;
    _empireStatus = _rebelStatus = psClean;
    _kills = 0;
    ConfigS::instance()->updateKeyword("skill", SKILL_ROOKIE);

    ScoreKeeperS::instance()->resetCurrentScore();
    PlanetManagerS::instance()->clearEvents();
    PlanetManagerS::instance()->addEvent(QuestEvent::qeGlobal,
        "War between empire and rebels has started.");
    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "Two pirates destroyed my ship and took the crystal.");
    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "Started trip to planet Torres to find the pirates.");

    switchContext(ePlanetMenu);
}
//----------------------------------------------------------------------------
void Game::updateOtherLogic()
{
    int stepCount = 0;
    float currentTime = Timer::getTime();
    while ((currentTime - GameState::startOfStep) > GAME_STEP_SIZE)
    {
        StarfieldS::instance()->update();

        if (_context == eMenu)
            MenuManagerS::instance()->update();

        if (_context == ePlanetMenu)
            PlanetManagerS::instance()->update();

        if (_context == eMessageBox)
            MessageBoxManagerS::instance()->update();

        //advance to next start-of-game-step point in time
        GameState::startOfStep += GAME_STEP_SIZE;
        currentTime = Timer::getTime();

        stepCount++;
        if( stepCount > MAX_GAME_STEPS)
            break;
    }

    GameState::frameFractionOther =
    (currentTime - GameState::startOfStep) / GAME_STEP_SIZE;

    bool minimizeCPU = false;
    ConfigS::instance()->getBoolean("minimizeCPU", minimizeCPU);

    if( stepCount > 1)
    {
        //LOG_WARNING << "Skipped " << stepCount << " frames outside game." << endl;
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
    else if (minimizeCPU)
    {   // give 80% of free CPU time back to system
        float extraTime = 800.0f * (GAME_STEP_SIZE - (currentTime - GameState::startOfStep));
        SDL_Delay((unsigned int)extraTime);
    }
}
//----------------------------------------------------------------------------
void Game::updateInGameLogic()
{
    float currentGameTime = GameState::stopwatch.getTime();
    float hyspace = 0;
    if (_hyperspaceCount != 0)
        hyspace = currentGameTime - _hyperspaceCount;

    int stepCount = 0;
    while ((currentGameTime - GameState::startOfGameStep) > GAME_STEP_SIZE)
    {
        if (hyspace < 10)
        {
            // adding new enemies, moving on to the next level, etc.
            StageManagerS::instance()->update();
            // update all objects, particles, etc.
            ParticleGroupManagerS::instance()->update();
        }

        // update rotation for OSD objects
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

    bool minimizeCPU = false;
    ConfigS::instance()->getBoolean("minimizeCPU", minimizeCPU);

    if( stepCount > 1)
    {
        //LOG_WARNING << "Skipped " << stepCount << " frames in game." << endl;
        if (GameState::frameFraction > 1.0)
        {
            // Our logic is still way behind where it should be at this
            // point in time. If we get here we already ran through
            // MAX_GAME_STEPS logic runs trying to catch up.

            // We clamp the value to 1.0, higher values would try
            // to predict were we are visually. Maybe not a bad idea
            // to allow values up to let's say 1.3...
            GameState::frameFraction = 1.0;
        }
    }
    else if (minimizeCPU)
    {   // give 80% of free CPU time back to system
        float extraTime = 800.0f * (GAME_STEP_SIZE - (currentGameTime - GameState::startOfGameStep));
        SDL_Delay((unsigned int)extraTime);
    }
}
//----------------------------------------------------------------------------
void savePlanetCoords(ofstream& out, Planet *p)
{
    out << "" << p->_x << " " << p->_y << " ";
}
//----------------------------------------------------------------------------
Planet *loadPlanetCoords(ifstream& in)
{
    float x, y;
    in >> x;
    in >> y;
    return GameS::instance()->_galaxy.getPlanetAt(x, y);
}
//----------------------------------------------------------------------------
bool Game::saveGame(const std::string& savename, int slot)
{
    char buff[100];
    sprintf(buff, "savegame%d", slot);
    string fileName = ConfigS::instance()->getConfigDirectory();
    fileName += buff;
    LOG_INFO << "Saving game to " << fileName << endl;

    //Save in a compressed file to make it a bit tougher to cheat...
#ifdef COMPRESS_SAVEGAME
    ofstream outfile( fileName.c_str(), ios::out | ios::binary);
    if (!outfile.good())
        return false;
    zoStream zout(outfile);
#else
    ofstream zout( fileName.c_str(), ios::out | ios::binary);
    if (!zout.good())
        return false;
#endif

    _money -= 1000;

    zout << "#------ SOTU Saved Game -----#\n";
    zout << removespaces(savename) << " ";
    _galaxy.save(zout);
    LOG_INFO << "  Saving current planet coords" << endl;
    savePlanetCoords(zout, _currentPlanet);
    LOG_INFO << "  Saving player's cargo" << endl;
    _cargo.save(zout);
    LOG_INFO << "  Saving money, kills, etc." << endl;
    SkillS::instance(); // initialize
    zout << "" << _money << " " << (int)_rebelStatus << " " << (int)_empireStatus
        << " " << _kills << " " << _chapter << " " << (int)GameState::skill
        << " " << ScoreKeeperS::instance()->getCurrentScore() << " ";
    LOG_INFO << "  Saving Quest Targets" << endl;
    for (std::vector<std::string>::iterator it = _questTargets.begin(); it != _questTargets.end(); ++it)
        zout << (*it) << " ";
    zout << "QUEST_TARGETS_END ";

    LOG_INFO << "  Saving hyperspace target coords" << endl;
    savePlanetCoords(zout, PlanetManagerS::instance()->getHyperspaceTarget());
    LOG_INFO << "  Saving events" << endl;
    PlanetManagerS::instance()->saveEvents(zout);

    if (!zout.good())
    {
        _money += 1000;
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
bool Game::loadGame(int slot)
{
    char buff[100];
    sprintf(buff, "savegame%d", slot);
    string fileName = ConfigS::instance()->getConfigDirectory();
    fileName += buff;
    LOG_INFO << "Loading game from " << fileName << endl;

    //Save in a compressed file to make it a bit tougher to cheat...
#ifdef COMPRESS_SAVEGAME
    ifstream infile( fileName.c_str(), ios::in | ios::binary);
    if (!infile.good())
        return false;
    ziStream zin(infile);
#else
    ifstream zin( fileName.c_str(), ios::in | ios::binary);
    if (!zin.good())
        return false;
#endif

    std::string line;
    if (getline(zin, line).eof() || line != "#------ SOTU Saved Game -----#")
        return false;

    LOG_INFO << "  defaults" << endl;
    // load other defaults
    _landed = true;
    _spaceStationApproach = _hyperspaceCount = 0;
    zin >> line;    // load save name
    _galaxy.load(zin);
    LOG_INFO << "  current planet" << endl;
    _currentPlanet = loadPlanetCoords(zin);
    LOG_INFO << "  cargo" << endl;
    _cargo.load(zin);
    LOG_INFO << "  money, kills, etc." << endl;
    int tmpi;
    zin >> _money;
    zin >> tmpi;
    _rebelStatus = (PlayerStatus)tmpi;
    zin >> tmpi;
    _empireStatus = (PlayerStatus)tmpi;
    zin >> _kills;
    zin >> _chapter;
    zin >> tmpi;
    Skill::SkillEnum sk = (Skill::SkillEnum)tmpi;
    SkillS::instance()->updateSkill(sk);
    zin >> tmpi;
    ScoreKeeperS::instance()->setScore(tmpi);
    LOG_INFO << "  quest targets" << endl;
    _questTargets.clear();
    while (!zin.eof())
    {
        std::string s;
        zin >> s;
        if (s == "QUEST_TARGETS_END")
            break;
        _questTargets.push_back(s);
    }
    LOG_INFO << "   hyperspace target" << endl;
    Planet *p = loadPlanetCoords(zin);
    PlanetManagerS::instance()->setHyperspaceTarget(p);
    LOG_INFO << "   events" << endl;
    PlanetManagerS::instance()->loadEvents(zin);
    switchContext(ePlanetMenu);
    setPreviousContext(ePlanetMenu);
    return true;
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
        bool minimizeCPU = false;
        ConfigS::instance()->getBoolean("minimizeCPU", minimizeCPU);
        if (!minimizeCPU)
            SDL_Delay(1);
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
    else if (c == eMessageBox)
        InputS::instance()->enableInterceptor(MessageBoxManagerS::instance());
    else
        InputS::instance()->disableInterceptor();

    _previousContext = _context;
    _context = c;
}
//----------------------------------------------------------------------------
void Game::setPreviousContext(ContextEnum c)
{
    _previousContext = c;
}
//----------------------------------------------------------------------------
void Game::previousContext()
{
    if (_previousContext != eUnknown)
        switchContext(_previousContext);
}
//----------------------------------------------------------------------------
// returns true if no illegal goods are found OR player is already fugitive
bool Game::illegalTradeCheck()
{
    if (_currentPlanet->_rebelSentiment <= 50 && _empireStatus != psClean)
        return true;
    if (_currentPlanet->_rebelSentiment > 50 && _rebelStatus != psClean)
        return true;

    // if illegal goods are found, pop up message box
    std::vector<CargoItemInfo *> illegal;
    std::vector<CargoItemInfo>* info = CargoItemInfo::getCargoInfo();
    for (std::vector<CargoItemInfo>::iterator it = info->begin();
        it != info->end(); ++it)
    {
        if ((*it)._legalStatus == CargoItemInfo::lsiEmpire && _currentPlanet->_rebelSentiment <= 50 ||
            (*it)._legalStatus == CargoItemInfo::lsiRebels && _currentPlanet->_rebelSentiment >  50 ||
            (*it)._legalStatus == CargoItemInfo::lsiBoth)
        {
            CargoItem *c = _cargo.findItem((*it)._name);
            if (c->_quantity > 0)
                illegal.push_back(&(*it));
        }
    }

    if (illegal.empty())
        return true;

    std::string who;
    if (_currentPlanet->_rebelSentiment <= 50)
        who = "Empire";
    else
        who = "Rebel";

    std::string what;
    for (std::vector<CargoItemInfo *>::iterator it = illegal.begin(); it != illegal.end(); ++it)
    {
        if (!what.empty())
            what += ", ";
        what += (*it)->_name;
    }

    std::string msg = who + " agents discovered that you are trading illegal goods ("
        + what + "). If you do not surrender, you will become a fugitive, and " + who
        + " fleet will attack you whenever you are in orbit of " + who
        + " planets.\n\nWill you surrender and give up your cargo?";

    MessageBoxManagerS::instance()->setup("ILLEGAL TRADING", msg,
        "Surrender",   "IllegalTradeAccept",
        "Let's fight", "IllegalTradeRefuse");
    switchContext(eMessageBox);
    return false;
}
//----------------------------------------------------------------------------
// accept = true if player accepted to loose all the cargo
void Game::illegalTradeDecision(bool accept)
{
    if (accept)
    {
        std::vector<CargoItemInfo>* info = CargoItemInfo::getCargoInfo();
        for (std::vector<CargoItemInfo>::iterator it = info->begin();
            it != info->end(); ++it)
        {
            if ((*it)._legalStatus == CargoItemInfo::lsiEmpire && _currentPlanet->_rebelSentiment <= 50 ||
                (*it)._legalStatus == CargoItemInfo::lsiRebels && _currentPlanet->_rebelSentiment >  50 ||
                (*it)._legalStatus == CargoItemInfo::lsiBoth)
            {
                CargoItem *c = _cargo.findItem((*it)._name);
                c->_quantity = 0;
            }
        }
    }
    else    // player refused
    {
        if (_currentPlanet->_rebelSentiment <= 50)
        {
            _empireStatus = psFugitive;
            StageManagerS::instance()->empireIsNext();
        }
        else
        {
            _rebelStatus = psFugitive;
            StageManagerS::instance()->rebelsAreNext();
        }
    }
}
//----------------------------------------------------------------------------
void Game::ulegAccept()
{   // add Armada to Map
    if (_rebelStatus != psClean)
    {
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "Rebels agreed to forget my previous illegal trading.");
    }
    _rebelStatus = psClean;
    Planet *p = new Planet(30, 150, "ARMADA");
    p->_rebelSentiment = 0;
    p->_techLevel = 9;
    p->_alienActivity = 0;
    _galaxy.addPlanet(p);
    _questTargets.push_back("ARMADA");
    switchContext(ePlanetMenu);
    setPreviousContext(ePlanetMenu);
    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "I accepted Uleg's offer, and must now try to destroy the Armada.");
    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "Uleg will give me 50000 credits when I return to Jothan.");
}
//----------------------------------------------------------------------------
void Game::ulegRefuse()
{
    _rebelStatus = psTerrorist;
#ifndef TRAINER
    _cargo.clear();
    _cargo.create(0);   // create empty cargo bay
    _cargo.findItem("Fuel")->_quantity += 30;
    _money = 2000;
#endif

    Planet *p = new Planet(30, 570, "CYROS");
    p->_rebelSentiment = 90;
    _galaxy.addPlanet(p);
    _questTargets.push_back("CYROS");
    switchContext(ePlanetMenu);
    setPreviousContext(ePlanetMenu);

    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "I managed to find an empty ship and escape the rebels.");
    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "Rebels consider me a traitor of their cause now.");
}
//----------------------------------------------------------------------------
void Game::reachedSpecialPlanet()
{
    //                dylka,jot,cir,arm,bor,liz,ris,cloaked
    //int specialX[] = { 28, 21,  1,  1, 36,  4, 17, 35};
    //int specialY[] = {  1, 27, 28,  7, 18,  5, 16, 29};

    std::string msg;
    std::string title;
    std::string okButton = "OK";
    std::string okAction = "PlanetMenu";
    std::string cancelButton = "";
    std::string cancelAction = "";
#ifdef DEMO
    if (_currentPlanet->_name == "TORRES")
    {
        title = "TRIAL VERSION";
        msg = "This is the most you can play with trial version of the game.\n"
            "To buy the full version, please visit our website at\n"
            "www.wodan.com\n\n"
            "The full version consists of five chapters, some new enemy types, "
            "choice to join the rebels or fight against them, and some surprises "
            "you'd rather discover yourself...";
        okAction = "Quit";
        okButton = "Exit game";
        _questTargets.clear();
    }
#else
    if (_currentPlanet->_name == "TORRES")
    {
        _chapter = 2;
        title = "Chapter Two: On The Trail";
        msg = "Asking around Torres space station, you learn that those two "
            "pirates are known to cooperate with the rebel army. They were "
            "here few days ago and went separate ways.\n\nOne of them went to "
            "nearby planet Dylka, where he lives.\nThe other went to a distant "
            "planet called Jothan.\nYour gallactic map is updated.";
        Planet *p = new Planet(570, 30, "DYLKA");
        _galaxy.addPlanet(p);
        p = new Planet(430, 550, "JOTHAN");
        p->_rebelSentiment = 100;
        p->_techLevel = 9;
        _galaxy.addPlanet(p);
        _questTargets.clear();
        _questTargets.push_back("DYLKA");
        _questTargets.push_back("JOTHAN");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Pirates cooperate with rebel army.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Pirates separated in Torres and went to Dylka and Jothan.");
    }
    else if (_currentPlanet->_name == "DYLKA")
    {
        title = "Chapter Two: On The Trail";
        msg = "Finally, you found one of the pirates. You managed to capture "
            "him, but he hasn't got the crystal. Your learn that it has some "
            "special imporance as the rebels hired the pirates to find it.\n\n"
            "The other pirate has the crystal and he went to planet Jothan.";
        _questTargets.clear();
        _questTargets.push_back("JOTHAN");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Pirate on Dylka doesn't have the crystal. I must go to Jothan.");
    }
    else if (_currentPlanet->_name == "JOTHAN" && _chapter == 2)
    {
        _chapter = 3;
        _questTargets.clear();
        title = "Chapter Three: Uleg's Offer";
        msg = "After docking to spacestation, the rebels capture you. While in"
            " prison, you learn that the other pirate went to planet Cyros.\n\n"
            "Rebel leader Uleg is amazed with your skills and offers to "
            "free you if you would destroy Empire Armada. If you succeed, "
            "you'll get 50000 credits. You may try to escape, but "
            "rebels took all the cargo and weapons from your ship.";
        okButton = "Accept Offer";
        cancelButton = "Escape prison";
        okAction =     "UlegAccept";
        cancelAction = "UlegRefuse";
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "I am captured by the rebels at Jothan space station.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Jothan is one of the key rebel planets.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Rebels want me to destroy Empire Armada.");
    }
    else if (_currentPlanet->_name == "JOTHAN" && _chapter == 3)
    {
        title = "Chapter Three: Uleg's Offer";
        msg = "Looks like you won't be able to collect your reward as the "
            "entire rebel compound has been destroyed by aliens.\n\n"
            "In ruins you find one computer that still works. Searching "
            "through the files you discover a secret document about planet "
            "Border, where rebels and aliens are creating a special weapon to "
            "destroy the Empire.";
        _currentPlanet->_techLevel = 2;
        _currentPlanet->_rebelSentiment = 60;
        _currentPlanet->_alienActivity = 90;
        Planet *p = new Planet(730, 370, "BORDER");
        p->_rebelSentiment = 99;
        p->_alienActivity = 99;
        p->_techLevel = 9;
        _galaxy.addPlanet(p);
        _questTargets.clear();
        _questTargets.push_back("BORDER");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Aliens destroyed rebel compound at Jothan.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Rebels were working together with aliens.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Rebels and aliens are creating a super weapon on planet Border.");
    }
    else if (_currentPlanet->_name == "ARMADA")
    {
        _empireStatus = psTerrorist;
        title = "Chapter Three: Uleg's Offer";
        msg = "Well done. You have destroyed the Empire Armada and now you "
            "can go back to Jothan to collect your reward.\n\nBe careful, "
            "the Empire army has learned about this, and they now treat you "
            "the same way they treat the rebels. You are considered a "
            "terrorist and you will be violently attacked whenever you reach "
            "orbit of any Empire planet.";
        _questTargets.clear();
        _questTargets.push_back("JOTHAN");
        _currentPlanet->_name = "Armada";
        _currentPlanet->_rebelSentiment = 90;
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "I have destroyed biggest Empire fleet - The Armada.");
    }
    else if (_currentPlanet->_name == "CYROS")
    {
        title = "Chapter Three: Uleg's Offer";
        msg = "Finally, you captured the other pirate. You learn that rebels "
            " have joined with the aliens to create a weapon to destroy the "
            "Empire. Crystal you found is one of key ingredients to that "
            "weapon.\n\nThe weapon is being constructed on planet Border.";
        Planet *p = new Planet(730, 370, "BORDER");
        p->_rebelSentiment = 99;
        p->_alienActivity = 99;
        p->_techLevel = 9;
        _galaxy.addPlanet(p);
        _questTargets.clear();
        _questTargets.push_back("BORDER");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Rebels are working together with aliens.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Rebels and aliens are creating a super weapon on planet Border.");
    }
    else if (_currentPlanet->_name == "BORDER")
    {
        title = "Chapter Four: The Cloaked Planet";
        msg = "Upon arriving, you see that all the rebels are dead. It looks "
            "like the aliens only used them as help to create a weapon which "
            "would destroy all the humans.\n\n"
            "One of the survivors tells you that weapon is hidden "
            "on a cloaked alien planet. Only way to discover it is to use a "
            "special device which is produced on planet Lizdor.";
        Planet *p = new Planet(90, 110, "LIZDOR");
        p->_techLevel = 9;
        _galaxy.addPlanet(p);
        _questTargets.clear();
        _questTargets.push_back("LIZDOR");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Aliens betrayed the Rebels and want to destroy us all.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "The super-weapon is on a cloaked alien planet.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "To detect the planet I need the device from Lizdor");
    }
    else if (_currentPlanet->_name == "LIZDOR" && _chapter == 3)
    {
        _chapter = 4;
        title = "Chapter Four: The Cloaked Planet";
        msg = "You found the people who are able to build the device which "
            "would detect the hidden alien planet.\n\nHowever, they are "
            "missing one ingredient - a rare metal Palladium, which is only "
            "to be found on planet Risar.";
        Planet *p = new Planet(350, 330, "RISAR");
        p->_techLevel = 9;
        _galaxy.addPlanet(p);
        _questTargets.clear();
        _questTargets.push_back("RISAR");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Rare metal Palladium is required to build the device.");
    }
    else if (_currentPlanet->_name == "RISAR")
    {
        title = "Chapter Four: The Cloaked Planet";
        msg = "Friendly people on planet listened to your story and were so "
            "thrilled, that they decided to give you as much Palladium as "
            "you need at no cost.";
        _questTargets.clear();
        _questTargets.push_back("LIZDOR");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "I got Palladium for free. Going back to Lizdor.");
    }
    else if (_currentPlanet->_name == "LIZDOR" && _chapter == 4)
    {
        _chapter = 5;
        title = "Chapter Five: Alien nest";
        msg = "You brought back the needed metal, and scientists have created "
            "the uncloaking device. As the device has unlimited range, "
            "they started it and, few hours later, the location of hidden "
            "alien planet has been revealed.\n\nNow you must travel there for "
            "a final shootdown.";
        Planet *p = new Planet(710, 590, "CLOAKED");
        p->_techLevel = 1;
        p->_alienActivity = 100;
        _galaxy.addPlanet(p);
        _questTargets.clear();
        _questTargets.push_back("CLOAKED");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "We have discovered cloaked alien planet.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "I must go there and destroy the super weapon.");
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "Super Weapon is actually a big alien spaceship.");
    }
    else if (_currentPlanet->_name == "CLOAKED")
    {
        title = "THE END";
        msg = "The threat for human race is now eliminated.\n\nYou would "
            "surely become a hero if there was anyone else alive who would "
            "confirm your story. Unfortunately, all others who were involved "
            "in this are dead, and war between Empire and Rebels is still "
            "raging across the galaxy.";
        okAction = "Quit";
        okButton = "End game";
        cancelButton = "Keep playing";
        cancelAction = "PlanetMenu";
        _questTargets.clear();
        _currentPlanet->_name = "Cloaked";
        PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
            "The human race is safe now.");
    }
#endif

    MessageBoxManagerS::instance()->setup(title, msg, okButton, okAction,
        cancelButton, cancelAction);
    switchContext(eMessageBox);
}
//----------------------------------------------------------------------------
void Game::hyperspaceJump()
{
    Planet *target = PlanetManagerS::instance()->getHyperspaceTarget();
    float dist = _currentPlanet->getDistance(target->_x, target->_y);
    CargoItem* fuel = _cargo.findItem("Fuel");
    fuel->_quantity -= (int)(dist + 0.92);
    _currentPlanet = target;
    _spaceStationApproach = 0;
    startNewGame();

    PlanetManagerS::instance()->addEvent(QuestEvent::qePlayer,
        "Jumped hyperspace to planet " + _currentPlanet->_name);
}
//----------------------------------------------------------------------------
std::string Game::getHyperspaceAvailable()
{
    if (_spaceStationApproach != 0)
        return "Docking to space station";

    Planet *target = PlanetManagerS::instance()->getHyperspaceTarget();
    if (target == _currentPlanet)
        return "No hyperspace target";

    float dist = _currentPlanet->getDistance(target->_x, target->_y);
    CargoItem* fuel = _cargo.findItem("Fuel");
    if (fuel->_quantity < (int)(dist + 0.92))
        return "Not enough fuel";
    return "OK";
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
        info.push_back(CargoItemInfo(1.0f, "GUN", "Firearms",                          4,  600, 1,  0, pmNormal,
            "Trading this item is illegal on Empire planets", lsiEmpire));
        info.push_back(CargoItemInfo(2.0f, "models/Boss1_Teeth", "Narcotics",          3,  395, 1,  0, pmProTech,
            "Trading this item is illegal on all planets",    lsiBoth));
        info.push_back(CargoItemInfo(5.0f, "models/Boss1_Eye1", "Slaves",              1,  400, 1,  0, pmNormal,
            "Trading this item is illegal on Rebel planets",  lsiRebels));

        info.push_back(CargoItemInfo(1.0f, "models/IceSprayPierce", "Proton enhancer", 6,  900, 0,  1, pmNormal,
            "Enhances primary and secondary weapon power"));
        info.push_back(CargoItemInfo(0.8f, "models/FlankBurster", "Wave emitter",      8, 1200, 0,  1, pmNormal,
            "Tertiary weapon - use middle mouse button or left ALT key"));
        info.push_back(CargoItemInfo(5.0f, "models/WeaponUpgrade", "Space grenade",    8, 1000, 0, 20, pmNormal,
            "Destroys multiple enemies - press key D to detonate"));
        info.push_back(CargoItemInfo(1.0f, "models/Stinger", "Stinger rocket",         1,   50, 0, 20, pmNormal,
            "Ammo for rocket launcher - press key F to fire"));
        info.push_back(CargoItemInfo(3.0f, "models/ShieldBoost", "Shield upgrade",     7, 2000, 0,  1, pmNormal,
            "Allows your ship to have 200 shield energy"));
        info.push_back(CargoItemInfo(1.5f, "models/Boss1_Leg1", "Fuel",                1,   20, 0, 40, pmNormal,
            "Needed for hyperspace travel between planets"));
    }
    return &info;
}
//----------------------------------------------------------------------------
void CargoItem::save(ofstream& os)
{
    os << removespaces(_name) << " " << _quantity << " " << _price << " ";
}
//----------------------------------------------------------------------------
bool CargoItem::load(ifstream& is)
{
    std::string s;
    is >> s;
    if (is.eof() || s == "CARGOEND")
        return false;
    _name = addspaces(s);
    is >> _quantity;
    is >> _price;
    return true;
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
                ((*it)._legalStatus == CargoItemInfo::lsiEmpire && p->_rebelSentiment <= 50 ||
                 (*it)._legalStatus == CargoItemInfo::lsiRebels && p->_rebelSentiment > 50);
            float multiply = (illegal ? 0.5f : 1.0f);
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
Cargo::iterator Cargo::begin()
{
    return _items.begin();
}
//----------------------------------------------------------------------------
Cargo::iterator Cargo::end()
{
    return _items.end();
}
//----------------------------------------------------------------------------
void Cargo::save(ofstream& os)
{
    for (std::vector<CargoItem>::iterator it = _items.begin(); it != _items.end(); ++it)
        (*it).save(os);
    os << "CARGOEND ";
}
//----------------------------------------------------------------------------
void Cargo::load(ifstream& is)
{
    clear();
    CargoItem c;
    while (c.load(is))
        _items.push_back(c);
}
//----------------------------------------------------------------------------
// PLANET ********************************************************************
//----------------------------------------------------------------------------
void Planet::save(ofstream& os)
{
    os << _name << " " << _x << " " << _y << " " << _radius << " "
        << _textureIndex << " " << _techLevel << " " << _rebelSentiment << " "
        << _alienActivity << " ";
    _marketplace.save(os);
}
//----------------------------------------------------------------------------
bool Planet::load(ifstream& is)
{
    is >> _name;
    if (_name == "GALAXY_END")
        return false;
    is >> _x;
    is >> _y;
    is >> _radius;
    is >> _textureIndex;
    is >> _techLevel;
    is >> _rebelSentiment;
    is >> _alienActivity;
    _marketplace.load(is);
    return true;
}
//----------------------------------------------------------------------------
Planet::Planet()
{
    // used only for loading
}
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
bool Planet::isSpecial()
{
    std::vector<std::string>& t = GameS::instance()->_questTargets;
    for (std::vector<std::string>::iterator it = t.begin(); it != t.end(); ++it)
        if (_name == (*it))
            return true;
    return false;
}
//----------------------------------------------------------------------------
float Planet::getDistance(float x, float y)
{
    float retval = 10.0f * sqrt((x-_x)*(x-_x) + (y-_y)*(y-_y));
    int conv = (int)retval;
    return 0.1f * (float)conv;
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
// called once each 10 turns or so
void Planet::update()
{
    // always the same
    if (_name == "XEN" || _name == "TORRES" || _name == "CLOAKED")
        return;

    static int counter = 0;
    if (counter++ % 10 != 0)
        return;

    bool changed = false;
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
        changed = true;
        _rebelSentiment = 70;
        std::string msg = "Civil war on " + _name + ". Rebels take over the planet.";
        if (_techLevel > 1)
        {
            _techLevel -= 2;
            msg += " Tech.level reduced.";
        }
        PlanetManagerS::instance()->addEvent(QuestEvent::qeGlobal, msg);
    }
    if (rs_before > 50 && _rebelSentiment < 50)
    {
        changed = true;
        _rebelSentiment = 40;
        std::string msg = "Empire army has conquered planet " + _name + " back.";
        PlanetManagerS::instance()->addEvent(QuestEvent::qeGlobal, msg);
    }
    if (_techLevel < 1)
        _techLevel = 1;
    if (_techLevel < 9 && _rebelSentiment > 70 || _rebelSentiment < 30)   // planet of stable government
    {
        _techLevel += 0.3f;
        if ((float)((int)_techLevel) == _techLevel)
        {
            changed = true;
            PlanetManagerS::instance()->addEvent(QuestEvent::qeGlobal,
                "Tech. level of " + _name + " has increased.");
        }

        if (_techLevel > 9)
            _techLevel = 9;
    }

    // update prices
    if (changed)
    {
        _marketplace.clear();
        _marketplace.create(this);
    }
}
//----------------------------------------------------------------------------
// MAP ***********************************************************************
//----------------------------------------------------------------------------
void Map::addPlanet(Planet *p)
{
    _planets.push_back(p);
}
//----------------------------------------------------------------------------
void Map::recreate()
{
    deletePlanets();
    LOG_INFO << "CREATING PLANETS" << endl;
    const int mapWidth = 760;
    const int mapHeight = 600;
    const int stepSize = 20;
    //              dylka,jot,cir,arm,bor,liz,ris,cloaked
    int specialX[] = { 28, 21,  1,  1, 36,  4, 17, 35};
    int specialY[] = {  1, 27, 28,  7, 18,  5, 16, 29};
    for (int x = 0; x < mapWidth; x += stepSize)
    {
        for (int y = 0; y < mapHeight; y += stepSize)
        {
            Planet *p;
            if (x == stepSize && y == stepSize)   // XEN (cell=1,1)
            {
                p = new Planet(30, 30, "XEN");
                PlanetManagerS::instance()->setHyperspaceTarget(p);
                GameS::instance()->_currentPlanet = p;
            }
            else if (x == stepSize*31 && y == stepSize*6 )   // TORRES (31,6)
            {
                p = new Planet(630, 130, "TORRES");
                p->_rebelSentiment = 99;
            }
            else
            {   // skip places of special planets
                bool skip = false;
                for (unsigned int i = 0; i < sizeof(specialX)/sizeof(int); ++i)
                {
                    if (x == stepSize * specialX[i] && y == stepSize * specialY[i])
                    {
                        skip = true;
                        break;
                    }
                }
                if (skip || (x+y)%50 == 0 && Random::integer(5) == 0)
                    continue;
                p = new Planet( x + 2 + Random::integer(stepSize-4),
                                y + 2 + Random::integer(stepSize-4));
            }
            _planets.push_back(p);
        }
    }
}
//----------------------------------------------------------------------------
void Map::update()
{
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
        (*it)->update();
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
void Map::save(ofstream& os)
{
    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
        (*it)->save(os);
    os << "GALAXY_END ";
}
//----------------------------------------------------------------------------
void Map::load(ifstream& is)
{
    deletePlanets();
    while (!is.eof())
    {
        Planet *p = new Planet;
        if (p->load(is))
        {
            //LOG_INFO << "Loaded planet: " << p->_name << endl;
            addPlanet(p);
        }
        else
        {
            delete p;
            return;
        }
    }
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
        glVertex3f(x+(*it)->_x, y+(*it)->_y, 0.0f);
    }
    glEnd();
    glDisable(GL_POINT_SMOOTH);
    glPointSize(1.0f);

    for (PlanetList::iterator it = _planets.begin(); it != _planets.end(); ++it)
        if ((*it)->isSpecial())
            PlanetManagerS::instance()->drawCursor(x+(*it)->_x, y+(*it)->_y, true);

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

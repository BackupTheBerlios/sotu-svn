// Description:
//   Menu Manager/Controller.
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
#ifndef _MenuManger_hpp_
#define _MenuManger_hpp_

#include <list>

#include <Singleton.hpp>
#include <tinyxml.h>

#include <InterceptorI.hpp>
#include <Context.hpp>
#include <Model.hpp>
#include <ParticleGroup.hpp>
#include <OnlineUpdateDisplay.hpp>

class Selectable;
class GLTexture;
class Planet;
// class GLTextureCubeMap;
//----------------------------------------------------------------------------
class MenuManager: public InterceptorI
{
friend class Singleton<MenuManager>;
public:
    bool init( void);
    bool update( void);
    bool draw( void);

    virtual void input( const Trigger &trigger, const bool &isDown);
    void turnMenuOn( void);
    void turnMenuOff( void);
    void makeMenu( TiXmlNode *_node);

    //Menu navigation
    void Down( void);
    void Up( void);
    void Enter( void);
    bool Exit( bool delayed=false);
    void Goto( Selectable *s);

    void reload( void);
private:
    virtual ~MenuManager();
    MenuManager( void);
    MenuManager( const MenuManager&);
    MenuManager &operator=(const MenuManager&);

    void loadMenuLevel( void);
    void clearActiveSelectables( void);
    void updateSettings( void);

    TiXmlDocument *_menu;

    TiXmlNode *_topMenu;
    TiXmlNode *_currentMenu;

    list<Selectable*> _activeSelectables;
    list<Selectable*>::iterator _currentSelectable;

    int _board;
    int _pointer;

    float _mouseX;
    float _mouseY;

    Context::ContextEnum _prevContext;
    bool _delayedExit;

    Model *_nextGenShippy;
    Model *_mapleLeaf;
    float _angle;
    float _prevAngle;

    bool _showSparks;
    ParticleGroup _burst;
    OnlineUpdateDisplay _onlineUpdateDisplay;
    // GLTextureCubeMap *_nextGenShippyCubeMap;
};
typedef Singleton<MenuManager> MenuManagerS;
//----------------------------------------------------------------------------
class PlanetManager: public InterceptorI
{
friend class Singleton<PlanetManager>;
public:
    bool init();
    bool update();
    bool draw();
    void enable(bool doEnable=true);
    void reload();

    void Up();
    void Down();
    void Left();
    void Right();
    void Enter();
    void Goto(Selectable *s);

    typedef enum { stMap=0, stTrade, stQuests } ScreenType;
    void setActiveScreen(ScreenType newone);

    virtual void input( const Trigger &trigger, const bool &isDown);

private:
    virtual ~PlanetManager();
    PlanetManager();
    PlanetManager( const PlanetManager&);
    PlanetManager &operator=(const PlanetManager&);

    list<Selectable*> _activeSelectables;
    list<Selectable*>::iterator _currentSelectable;

    int _pointer;
    //int _board;
    float _mouseX, _mouseY;
    float _angle, _prevAngle;
    Context::ContextEnum _prevContext;

    // there will be more, this is just a test
    typedef std::vector<GLTexture *> PlanetTexList;
    PlanetTexList _planetTex;

    ScreenType _screenType;
    void drawCargo();
    void drawMap();
    void drawQuests();
    void drawPlanet(float x, float y, Planet *, const std::string& title);
};

typedef Singleton<PlanetManager> PlanetManagerS;
//----------------------------------------------------------------------------
#endif

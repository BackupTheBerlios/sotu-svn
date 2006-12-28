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
class CargoItemInfo;
// class GLTextureCubeMap;
//----------------------------------------------------------------------------
// load/save helpers
std::string removespaces(const std::string& str);
std::string addspaces(const std::string& str);
//----------------------------------------------------------------------------
class MenuManager: public InterceptorI
{
friend class Singleton<MenuManager>;
public:
    bool init( void);
    bool update( void);
    bool draw( void);

    virtual void input( const Trigger &trigger, const bool &isDown);
    void makeMenu( TiXmlNode *_node);

    //Menu navigation
    void Down( void);
    void Up( void);
    void Enter( void);
    bool exitMenu(bool allowQuitGame);

private:
    virtual ~MenuManager();
    MenuManager( void);
    MenuManager( const MenuManager&);
    MenuManager &operator=(const MenuManager&);

    void loadMenuLevel( void);
    void clearActiveSelectables( void);

    TiXmlDocument *_menu;

    TiXmlNode *_topMenu;
    TiXmlNode *_currentMenu;

    list<Selectable*> _activeSelectables;

    int _board;
    int _pointer;

    float _mouseX;
    float _mouseY;

    //bool _delayedExit;

    Model *_nextGenShippy;
    Model *_mapleLeaf;
    float _angle;
    float _prevAngle;

    OnlineUpdateDisplay _onlineUpdateDisplay;
};
typedef Singleton<MenuManager> MenuManagerS;
//----------------------------------------------------------------------------
class QuestEvent
{
public:
    typedef enum { qePlayer = 0, qeGlobal } QuestEventType;

    QuestEventType _type;
    std::string _text;

    QuestEvent(QuestEventType type, const std::string& text)
    {
        _type = type;
        _text = text;
    }
};
//----------------------------------------------------------------------------
class PlanetManager: public InterceptorI
{
friend class Singleton<PlanetManager>;
public:
    bool init();
    bool update();
    bool draw();
    void drawCursor(float x, float y, bool special);
    void reload();

    void Up();
    void Down();
    void Left();
    void Right();
    void Enter();

    void addEvent(const QuestEvent& event);
    void addEvent(QuestEvent::QuestEventType type, const std::string& text);
    void clearEvents();
    void saveEvents(ofstream& os);
    void loadEvents(ifstream& is);

    typedef enum { stMap=0, stTrade, stQuests } ScreenType;
    void setActiveScreen(ScreenType newone);

    virtual void input( const Trigger &trigger, const bool &isDown);

    void setHyperspaceTarget(Planet *p);
    Planet *getHyperspaceTarget();

private:
    virtual ~PlanetManager();
    PlanetManager();
    PlanetManager( const PlanetManager&);
    PlanetManager &operator=(const PlanetManager&);

    list<Selectable*> _activeSelectables;

    int _pointer;
    //int _board;
    float _mouseX, _mouseY;
    float _angle, _prevAngle;

    // there will be more, this is just a test
    typedef std::vector<GLTexture *> PlanetTexList;
    PlanetTexList _planetTex;

    Planet* _hyperspaceTarget;

    typedef std::list<QuestEvent> QuestList;
    QuestList _questEvents;

    ScreenType _screenType;
    void drawCargo();
    void drawMap();
    void drawQuests();
    void drawPlanet(float x, float y, Planet *, const std::string& title);
    void drawItemIcon(CargoItemInfo& info, float offset);
    void drawPlayer(float yoffset);

    void planetClick();
    void tradeClick();
    void saveClick();
};

typedef Singleton<PlanetManager> PlanetManagerS;
//----------------------------------------------------------------------------
class MessageBoxManager: public InterceptorI
{
friend class Singleton<MessageBoxManager>;
public:
    bool init();
    bool update();
    bool draw();

    /*
    void Up();
    void Down();
    void Left();
    void Right();
    void Enter();
    */

    virtual void input( const Trigger &trigger, const bool &isDown);

    void setup(const std::string& title, const std::string& text,
        const std::string& okText, const std::string& okAction,
        const std::string& cancelText = "",
        const std::string& cancelAction = "");

private:
    virtual ~MessageBoxManager();
    MessageBoxManager();
    MessageBoxManager( const MessageBoxManager&);
    MessageBoxManager &operator=(const MessageBoxManager&);

    list<Selectable*> _activeSelectables;
    void clearSelectables();

    int _pointer;
    int _board;
    float _mouseX, _mouseY;
    float _angle, _prevAngle;

    std::string _title;
    std::vector<std::string> _text;
};

typedef Singleton<MessageBoxManager> MessageBoxManagerS;
//----------------------------------------------------------------------------
#endif

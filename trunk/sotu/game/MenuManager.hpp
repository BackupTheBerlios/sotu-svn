// Description:
//   Menu Manager/Controller.
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
class GLTextureCubeMap;

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
    GLTextureCubeMap *_nextGenShippyCubeMap;
};

typedef Singleton<MenuManager> MenuManagerS;

#endif

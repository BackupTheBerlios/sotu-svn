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
#include "SDL.h" //key syms

#include <Trace.hpp>
#include <MenuManager.hpp>
#include <XMLHelper.hpp>
#include <GameState.hpp>
#include <Game.hpp>
#include <Audio.hpp>
#include <Input.hpp>
#include <FontManager.hpp>
#include <SelectableFactory.hpp>
#include <ModelManager.hpp>
#include <Video.hpp>
#include <BitmapManager.hpp>
#include <Config.hpp>

#include <gl++.hpp>

#include <GLExtensionTextureCubeMap.hpp>
#include <GLTextureCubeMap.hpp>
#include <ResourceManager.hpp>
#include <zrwops.hpp>

MenuManager::MenuManager():
    _menu(0),
    _topMenu(0),
    _currentMenu(0),
    _mouseX(200.0),
    _mouseY(650.0),
    _prevContext(Context::eUnknown),
    _delayedExit(false),
    _angle(0.0),
    _prevAngle(0.0),
    _showSparks(false),
    _burst( "SparkBurst", 1000),
    _nextGenShippyCubeMap(0)
{
    XTRACE();

    updateSettings();
}

MenuManager::~MenuManager()
{
    XTRACE();

    clearActiveSelectables();

    SelectableFactory::cleanup();

    delete _menu;
    _menu = 0;

    delete _nextGenShippyCubeMap;
}

bool MenuManager::init( void)
{
    XTRACE();
    _menu = XMLHelper::load( "system/Menu.xml");
    if( !_menu)
    {
    _menu = 0;
    return false;
    }

    _currentMenu = _menu->FirstChild("Menu");
    _topMenu = _currentMenu;

    loadMenuLevel();

    GLBitmapCollection *icons =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    if( !icons)
    {
    LOG_ERROR << "Unable to load menuIcons." << endl;
    return false;
    }
    _pointer = icons->getIndex( "Pointer");

    GLBitmapCollection *menuBoard =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuBoard");
    if( !menuBoard)
    {
    LOG_ERROR << "Unable to load menuBoard." << endl;
    return false;
    }
    _board = menuBoard->getIndex( "MenuBoard");

    _mapleLeaf = ModelManagerS::instance()->getModel("models/MapleLeaf");
    if( !_mapleLeaf)
    {
    LOG_ERROR << "Unable to load Maple Leaf, eh?" << endl;
    return false;
    }

    _nextGenShippy = ModelManagerS::instance()->getModel("models/NextGenShippy");
    if( !_nextGenShippy)
    {
    LOG_ERROR << "Unable to sphere."<< endl;
    return false;
    }

    GLExtensionTextureCubeMap _nextGenShippyExt;
    if( _nextGenShippyExt.isSupported())
    {
    SDL_Surface *images[6];
    for( int i=0; i<6; i++)
    {
        char buf[128];
        sprintf( buf, "bitmaps/cubemap_%d.png", i);
        string fileName = buf;
        if( ResourceManagerS::instance()->selectResource( fileName))
        {
        ziStream &bminfile1 = ResourceManagerS::instance()->getInputStream();
        SDL_RWops *src = RWops_from_ziStream( bminfile1);
        images[i] = IMG_LoadPNG_RW( src);
        SDL_RWclose( src);
        }
        else
        {
        LOG_ERROR << "Could not load cubemap image\n";
        images[i] = 0;
        }
    }

    _nextGenShippyCubeMap = new GLTextureCubeMap( images);
    }
    else
    {
    LOG_WARNING << "ARB_texture_cube_map not supported\n";
    }

    _burst.init();

    return true;
}

void MenuManager::updateSettings( void)
{
    ConfigS::instance()->getBoolean( "showSparks", _showSparks);
}

void MenuManager::clearActiveSelectables( void)
{
    list<Selectable*>::iterator i;
    for( i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
    {
        delete (*i);
    }
    _activeSelectables.clear();

    Selectable::reset();
}

void MenuManager::loadMenuLevel( void)
{
    clearActiveSelectables();

    TiXmlNode *node = _currentMenu->FirstChild();
    while( node)
    {
//  LOG_INFO << "MenuItem: [" << node->Value() << "]" << endl;

    SelectableFactory *selF = SelectableFactory::getFactory( node->Value());
    if( selF)
    {
        Selectable *sel = selF->createSelectable( node);
        if( sel)
        {
        _activeSelectables.insert( _activeSelectables.end(), sel);
        }
    }
    else
    {
        LOG_WARNING << "No Factory found for:" << node->Value() << endl;
    }
    node = node->NextSibling();
    }

    //add escape button
    {
    BoundingBox r;
    r.min.x = 860;
    r.min.y = 530;
    r.max.x = 875;
    r.max.y = 545;

    Selectable *sel = new EscapeSelectable( r, 2.0);
    _activeSelectables.insert( _activeSelectables.end(), sel);
    }

    _currentSelectable = _activeSelectables.begin();
    if( _currentSelectable != _activeSelectables.end())
    {
    (*_currentSelectable)->activate();
    }
}

void MenuManager::makeMenu( TiXmlNode *_node)
{
    _currentMenu = _node;
    loadMenuLevel();
}

bool MenuManager::update( void)
{
    static float nextTime = Timer::getTime()+0.5f;
    float thisTime = Timer::getTime();
    if( thisTime > nextTime)
    {
    updateSettings();
    nextTime = thisTime+0.5f;
    }

    _onlineUpdateDisplay.update();

    _prevAngle = _angle;
    _angle += 10.0f;

    if( _delayedExit)
    {
    if( !Exit())
    {
        turnMenuOff();
    }
    _delayedExit = false;
    }
    list<Selectable*>::iterator i;
    for( i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
    {
        (*i)->update();
    }

    /*
    if( _showSparks)
    {
        GLBitmapCollection *icons =
            BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
        float w = _mouseX + icons->getWidth( _pointer)*0.4f;
        float h = _mouseY - icons->getHeight( _pointer)*0.4f;
        for( int i=0; i<10; i++)
        {
            _burst.newParticle( "Spark", w-i, h+i, 0);
        }

        _burst.update();
    }*/

    return true;
}

bool MenuManager::draw( void)
{
    glEnable( GL_LIGHTING);
    glEnable( GL_DEPTH_TEST);

    GLfloat light_position[] = { 820.0, 620.0, 500.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glPushMatrix();
    glTranslatef( 820.0, 620.0, 0.0);
    float iAngle = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
    glRotatef(iAngle/7.0, 2.0, 3.0, 5.0);
    _mapleLeaf->draw();
    glPopMatrix();

    if( _nextGenShippyCubeMap)
    {
        glPushMatrix();
        glTranslatef( 200.0, 620.0, 0.0);
        iAngle = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
        glRotatef(-iAngle/10.0, 2.0,3.0,5.0);
        //_nextGenShippyCubeMap->bind();
        //_nextGenShippyCubeMap->enable();
        _nextGenShippy->draw();
        //_nextGenShippyCubeMap->disable();
        glPopMatrix();
    }

    glDisable( GL_DEPTH_TEST);
    glDisable( GL_LIGHTING);

    _onlineUpdateDisplay.draw();

    GLBitmapCollection *menuBoard =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuBoard");
    menuBoard->bind();
    glColor4f(1.0, 1.0, 1.0, 0.7f);
    glEnable(GL_TEXTURE_2D);
    menuBoard->Draw( _board, 100.0, 50.0, 800.0/256.0, 2.0);
    glDisable(GL_TEXTURE_2D);

    TiXmlElement* elem = _currentMenu->ToElement();
    const string* val = elem->Attribute("Text");
    if( val)
    {
    GLBitmapFont &fontWhite =
      *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
    glColor4f(1.0, 1.0, 1.0, 1.0);
        fontWhite.DrawString( (*val).c_str(), 122, 527, 0.5, 0.5);
    }

    list<Selectable*>::iterator i;
    for( i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
    {
        (*i)->draw();
    }

    glEnable(GL_TEXTURE_2D);
    //if( _showSparks)
    //    _burst.draw();

    GLBitmapCollection *icons =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    icons->Draw( _pointer, _mouseX, _mouseY, 0.5, 0.5);
    glDisable(GL_TEXTURE_2D);

    return true;
}

void MenuManager::reload( void)
{
    _nextGenShippyCubeMap->reload();
}

void MenuManager::turnMenuOn( void)
{
    AudioS::instance()->playSample( "sounds/humm.wav");
    _prevContext = GameState::context;
    GameState::context = Context::eMenu;

    //ask input system to forward all input to us
    InputS::instance()->enableInterceptor( this);
    GameState::stopwatch.pause();
}

void MenuManager::turnMenuOff( void)
{
    if( _prevContext == Context::eUnknown) return;

    AudioS::instance()->playSample( "sounds/humm.wav");
    GameState::context = _prevContext;

    //don't want anymore input
    InputS::instance()->disableInterceptor();

    if( GameState::context == Context::eInGame)
    {
    GameState::stopwatch.start();
    }
}

void MenuManager::input( const Trigger &trigger, const bool &isDown)
{
    Trigger t = trigger;
    if( isDown)
    {
    switch( trigger.type)
    {
        case eKeyTrigger:
        switch( trigger.data1)
        {
            case SDLK_RETURN:
            Enter();
            break;
            case SDLK_ESCAPE:
            if( !Exit())
            {
                turnMenuOff();
            }
            break;
            case SDLK_UP:
            Up();
            break;
            case SDLK_DOWN:
            Down();
            break;

            case SDLK_F12:
            VideoS::instance()->takeSnapshot();
            break;

            default:
            break;
        }
        break;

        case eButtonTrigger:
        break;

        case eMotionTrigger:
        _mouseX += (trigger.fData1*10.0f);
        _mouseY += (trigger.fData2*10.0f);

        Clamp( _mouseX, 0.0f, 1000.0f);
        Clamp( _mouseY, 0.0f, 750.0f);

        break;

        default:
        break;
    }
    }

    //put the absolute mouse position in to trigger
    t.fData1 = _mouseX;
    t.fData2 = _mouseY;

    list<Selectable*>::iterator i;
    list<Selectable*>::iterator check = _activeSelectables.begin();
    for( i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
    {
    Selectable *sel = *i;
    if( !sel)
    {
        LOG_ERROR << "Selectable is 0 !!!" << endl;
        continue;
    }
    const BoundingBox &r = sel->getInputBox();

    if( (_mouseX >= r.min.x) &&
        (_mouseX <= r.max.x) &&
        (_mouseY >= r.min.y) &&
        (_mouseY <= r.max.y))
    {
        sel->input( t, isDown);
        //one of the selectables may trigger a loadMenuLevel and our
        //iterator will be invalid. Drop out of this loop!
        if( check != _activeSelectables.begin())
        {
//      LOG_INFO << "active selectables have changed..." << endl;
        break;
        }
    }
    }
}

void MenuManager::Down( void)
{
    XTRACE();
    if( _currentSelectable == _activeSelectables.end()) return;

    _currentSelectable++;
    if( _currentSelectable == _activeSelectables.end())
    {
    _currentSelectable = _activeSelectables.begin();
    }

    (*_currentSelectable)->activate();
}

void MenuManager::Up( void)
{
    XTRACE();
    if( _currentSelectable == _activeSelectables.end()) return;

    if( _currentSelectable == _activeSelectables.begin())
    {
    _currentSelectable = _activeSelectables.end();
    }
    _currentSelectable--;

    (*_currentSelectable)->activate();
}

void MenuManager::Goto( Selectable *s)
{
    list<Selectable*>::iterator i;
    for( i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
    {
        if( (*i) == s)
    {
//      LOG_INFO << "Goto found match" << endl;
        break;
    }
    }
    _currentSelectable = i;
}

void MenuManager::Enter( void)
{
    XTRACE();
    (*_currentSelectable)->select();
}

bool MenuManager::Exit( bool delayed)
{
    XTRACE();
    if( delayed)
    {
    //while iterating over the selectables we dont want to loadMenuLevel
    _delayedExit = true;
    return true;
    }
    if( _currentMenu != _topMenu)
    {
    _currentMenu = _currentMenu->Parent();
    loadMenuLevel();
    AudioS::instance()->playSample( "sounds/humm.wav");
    return true;
    }

    //at the top level menu
    return false;
}

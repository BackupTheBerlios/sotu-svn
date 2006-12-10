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

//#include <GLExtensionTextureCubeMap.hpp>
//#include <GLTextureCubeMap.hpp>
#include <ResourceManager.hpp>
#include <zrwops.hpp>
//----------------------------------------------------------------------------
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
    _burst( "SparkBurst", 1000) //,
    //_nextGenShippyCubeMap(0)
{
    XTRACE();

    updateSettings();
}
//----------------------------------------------------------------------------
MenuManager::~MenuManager()
{
    XTRACE();

    clearActiveSelectables();

    SelectableFactory::cleanup();

    delete _menu;
    _menu = 0;

    //delete _nextGenShippyCubeMap;
}
//----------------------------------------------------------------------------
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

    /*
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
    }*/

    //_burst.init();
    return true;
}
//----------------------------------------------------------------------------
void MenuManager::updateSettings( void)
{
    ConfigS::instance()->getBoolean( "showSparks", _showSparks);
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
void MenuManager::makeMenu( TiXmlNode *_node)
{
    _currentMenu = _node;
    loadMenuLevel();
}
//----------------------------------------------------------------------------
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
    return true;
}
//----------------------------------------------------------------------------
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

    //if( _nextGenShippyCubeMap)
    //{
        glPushMatrix();
        glTranslatef( 200.0, 620.0, 0.0);
        iAngle = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
        glRotatef(-iAngle/10.0, 2.0,3.0,5.0);
        _nextGenShippy->draw();
        glPopMatrix();
    //}

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
    GLBitmapCollection *icons =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    icons->Draw( _pointer, _mouseX, _mouseY, 0.5, 0.5);
    glDisable(GL_TEXTURE_2D);
    return true;
}
//----------------------------------------------------------------------------
void MenuManager::reload( void)
{
    //_nextGenShippyCubeMap->reload();
}
//----------------------------------------------------------------------------
void MenuManager::turnMenuOn( void)
{
    AudioS::instance()->playSample( "sounds/humm.wav");
    _prevContext = GameState::context;
    GameState::context = Context::eMenu;

    //ask input system to forward all input to us
    InputS::instance()->enableInterceptor( this);
    GameState::stopwatch.pause();

    // just in case we were in pause mode in game
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
}
//----------------------------------------------------------------------------
void MenuManager::turnMenuOff( void)
{
    if( _prevContext == Context::eUnknown)
        return;

    AudioS::instance()->playSample( "sounds/humm.wav");
    GameState::context = _prevContext;

    //don't want anymore input
    InputS::instance()->disableInterceptor();

    if( GameState::context == Context::eInGame)
    {
        GameState::stopwatch.start();
    }
}
//----------------------------------------------------------------------------
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
                    case SDLK_RETURN:   Enter();    break;
                    case SDLK_ESCAPE:
                        if( !Exit())
                            turnMenuOff();
                        break;
                    case SDLK_UP:       Up();       break;
                    case SDLK_DOWN:     Down();     break;
                    case SDLK_F12:      VideoS::instance()->takeSnapshot(); break;
                    default:            break;
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

        if( (_mouseX >= r.min.x) && (_mouseX <= r.max.x) &&
            (_mouseY >= r.min.y) && (_mouseY <= r.max.y))
        {
            sel->input( t, isDown);
            //one of the selectables may trigger a loadMenuLevel and our
            //iterator will be invalid. Drop out of this loop!
            if( check != _activeSelectables.begin())
            {
                // LOG_INFO << "active selectables have changed..." << endl;
                break;
            }
        }
    }
}
//----------------------------------------------------------------------------
void MenuManager::Down( void)
{
    XTRACE();
    if( _currentSelectable == _activeSelectables.end())
        return;

    _currentSelectable++;
    if( _currentSelectable == _activeSelectables.end())
        _currentSelectable = _activeSelectables.begin();
    (*_currentSelectable)->activate();
}
//----------------------------------------------------------------------------
void MenuManager::Up( void)
{
    XTRACE();
    if( _currentSelectable == _activeSelectables.end())
        return;

    if( _currentSelectable == _activeSelectables.begin())
        _currentSelectable = _activeSelectables.end();
    _currentSelectable--;
    (*_currentSelectable)->activate();
}
//----------------------------------------------------------------------------
void MenuManager::Goto(Selectable *s)
{
    list<Selectable*>::iterator i;
    for (i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
        if ((*i) == s)
            break;
    _currentSelectable = i;
}
//----------------------------------------------------------------------------
void MenuManager::Enter( void)
{
    XTRACE();
    if (_currentSelectable != _activeSelectables.end())
        (*_currentSelectable)->select();
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
// PLANET MANAGER ************************************************************
//----------------------------------------------------------------------------
// load graphics and stuff
bool PlanetManager::init()
{
    GLBitmapCollection *icons =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    if( !icons)
    {
        LOG_ERROR << "Unable to load menuIcons." << endl;
        return false;
    }
    _pointer = icons->getIndex("Pointer");

    // create clickable stuff available in all tabs
    const GLfloat tabh = 60.0f;
    const GLfloat tabw[] = { 300.0f, 300.0f, 300.0f };

    BoundingBox r;
    //r.min = Point2D(20.0f,           740.0f);
    //r.max = Point2D(20.0f + tabw[0], 740.0f - tabh);
    r.min = Point2D(20.0f + 0.5f * tabw[0], 740.0f - tabh + 5.0f);
    _activeSelectables.insert( _activeSelectables.end(),
        new ActionSelectable(r, "ShowMap", "Galactic Map", "Shows the map"));
    r.min = Point2D(20.0f + tabw[0] + 0.5f * tabw[1], 740.0f - tabh + 5.0f);
    _activeSelectables.insert( _activeSelectables.end(),
        new ActionSelectable(r, "ShowCargo", "Cargo & Trade", "Buy and sell goods and equipment"));
    r.min = Point2D(20.0f + tabw[0] + tabw[1] + 0.5f * tabw[2], 740.0f - tabh + 5.0f);
    _activeSelectables.insert( _activeSelectables.end(),
        new ActionSelectable(r, "ShowQuests", "Quests", "Shows the major and minor quests"));

    _currentSelectable = _activeSelectables.begin();
    /*
    if( _currentSelectable != _activeSelectables.end())
        (*_currentSelectable)->activate();
    */
    return true;
}
//----------------------------------------------------------------------------
void PlanetManager::setActiveScreen(ScreenType newone)
{
    if (_currentSelectable != _activeSelectables.end())
        (*_currentSelectable)->deactivate();
    _currentSelectable = _activeSelectables.end();
    _screenType = newone;
}
//----------------------------------------------------------------------------
bool PlanetManager::update()
{
    for(list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); ++i)
    {
        (*i)->update();
    }
    return true;
}
//----------------------------------------------------------------------------
void PlanetManager::enable(bool doEnable)
{
    if (doEnable)
    {
        // remove ptr to active node
        Selectable::reset();

        _prevContext = GameState::context;
        GameState::context = Context::ePlanetMenu;
        InputS::instance()->enableInterceptor(this);
        GameState::stopwatch.pause();

        // just in case we were in pause mode in game
        SDL_ShowCursor(SDL_DISABLE);
        SDL_WM_GrabInput(SDL_GRAB_ON);
    }
    else
    {
        if(_prevContext == Context::eUnknown)
            return;

        AudioS::instance()->playSample( "sounds/humm.wav");
        GameState::context = _prevContext;
        InputS::instance()->disableInterceptor();
        if (GameState::context == Context::eInGame)
            GameState::stopwatch.start();
        if (GameState::context == Context::eMenu)
            MenuManagerS::instance()->turnMenuOn();
    }
}
//----------------------------------------------------------------------------
void PlanetManager::Goto(Selectable *s)
{
    list<Selectable*>::iterator i;
    for (i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
        if ((*i) == s)
            break;
    _currentSelectable = i;
}
//----------------------------------------------------------------------------
void PlanetManager::Enter( void)
{
    XTRACE();
    if (_currentSelectable != _activeSelectables.end())
        (*_currentSelectable)->select();
}
//----------------------------------------------------------------------------
void PlanetManager::drawCargo()
{
    //if (_currentPlanet == 0)
    //    hide trade buttons and prices
}
//----------------------------------------------------------------------------
void PlanetManager::drawMap()
{
    // to draw planets use this:
    glBegin(GL_POINTS);    // Specify point drawing
      glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();
}
//----------------------------------------------------------------------------
void PlanetManager::drawQuests()
{
}
//----------------------------------------------------------------------------
bool PlanetManager::draw()
{
    // set minimal line size to 2pixels
    GLfloat sizes[2];  // Store supported line width range
    GLfloat step;     // Store supported line width increments
    glGetFloatv(GL_LINE_WIDTH_RANGE,sizes);
    glGetFloatv(GL_LINE_WIDTH_GRANULARITY,&step);
    GLfloat size = sizes[0];
    while (size < 2.0f && size + step <= sizes[1] && step > 0)
        size += step;
    glLineWidth(size);

    // draw lines
    const GLfloat tabh = 60.0f;
    const GLfloat tabw[] = { 300.0f, 300.0f, 300.0f };
    const GLfloat tabspace = 5.0f;
    glColor3f(0.2f, 0.8f, 0.0f);    // green color
    glBegin(GL_LINE_STRIP);
    glVertex2f( 10.0f, 740.0f - tabh);
    glVertex2f( 10.0f,  10.0f);
    glVertex2f(990.0f,  10.0f);
    glVertex2f(990.0f, 740.0f - tabh);

    // selected tab - calculate right edge
    GLfloat left = 20;  // offset (10 screen + 10 first tab)
    int nextTab=0;
    for (; nextTab != (int)_screenType; ++nextTab)
        left += tabw[nextTab];
    glVertex2f( left + tabw[nextTab], 740.0f - tabh);
    glVertex2f( left + tabw[nextTab], 740.0f       );
    glVertex2f( left,                 740.0f       );
    glVertex2f( left,                 740.0f - tabh);
    glVertex2f( 10.0f, 740.0f - tabh);
    glEnd();

    // render other tabs
    left = 20;  // reset
    for (int i=0; i<3; ++i)
    {
        left += tabw[i];
        if (i == (int)_screenType)
            continue;
        glBegin(GL_LINE_STRIP);
        if (i < (int)_screenType)
        {
            glVertex2f( left - tabw[i],  740.0f - tabh + tabspace);
            glVertex2f( left - tabw[i],  740.0f);
            glVertex2f( left - tabspace, 740.0f);
        }
        else
        {
            glVertex2f( left - tabw[i] + tabspace, 740.0f);
            glVertex2f( left,                      740.0f);
            glVertex2f( left,                      740.0f - tabh + tabspace);
        }
        glEnd();
    }

    // tab selectors are selectables
    for (list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); i++)
    {
        //LOG_INFO << "Drawing " << (*i) << endl;
        (*i)->draw();
    }

    // render the active page
    if (_screenType == stMap)
        drawMap();
    if (_screenType == stTrade)
        drawCargo();
    if (_screenType == stQuests)
        drawQuests();

    // render mouse pointer
    glEnable(GL_TEXTURE_2D);
    GLBitmapCollection *icons =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    icons->Draw( _pointer, _mouseX, _mouseY, 0.5, 0.5);
    glDisable(GL_TEXTURE_2D);
    return true;
}
//----------------------------------------------------------------------------
void PlanetManager::Up()
{

}
//----------------------------------------------------------------------------
void PlanetManager::Down()
{
}
//----------------------------------------------------------------------------
void PlanetManager::Left()
{
}
//----------------------------------------------------------------------------
void PlanetManager::Right()
{
}
//----------------------------------------------------------------------------
void PlanetManager::input(const Trigger &trigger, const bool &isDown)
{
    Trigger t = trigger;
    if( isDown)
    {
        switch( trigger.type)
        {
            case eKeyTrigger:
                switch( trigger.data1)
                {
                    case SDLK_ESCAPE:   enable(false);  return;
                    case SDLK_UP:       Up();           break;
                    case SDLK_DOWN:     Down();         break;
                    case SDLK_LEFT:     Left();         break;
                    case SDLK_RIGHT:    Right();        break;
                    case SDLK_F12: VideoS::instance()->takeSnapshot();  break;
                    default:                break;
                }
                break;

            case eMotionTrigger:
                _mouseX += (trigger.fData1*10.0f);
                _mouseY += (trigger.fData2*10.0f);
                Clamp( _mouseX, 0.0f, 1000.0f);
                Clamp( _mouseY, 0.0f, 750.0f);
                break;

            case eButtonTrigger:
            default:    break;
        }
    }

    //put the absolute mouse position in to trigger
    t.fData1 = _mouseX;
    t.fData2 = _mouseY;

    // handle mouse event on selectables
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
        if( (_mouseX >= r.min.x) && (_mouseX <= r.max.x) &&
            (_mouseY >= r.min.y) && (_mouseY <= r.max.y))
        {
            sel->input( t, isDown);
            //one of the selectables may trigger a loadMenuLevel and our
            //iterator will be invalid. Drop out of this loop!
            if( check != _activeSelectables.begin())
            {
                // LOG_INFO << "active selectables have changed..." << endl;
                break;
            }
        }
    }
}
//----------------------------------------------------------------------------
PlanetManager::~PlanetManager()
{
    XTRACE();

    for(list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); ++i)
    {
        delete (*i);
    }
    _activeSelectables.clear();
    Selectable::reset();
    SelectableFactory::cleanup();
}
//----------------------------------------------------------------------------
PlanetManager::PlanetManager():
    _mouseX(200.0),
    _mouseY(650.0)
{
    _screenType = stMap;
}
//----------------------------------------------------------------------------

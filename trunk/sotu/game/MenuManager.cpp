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
#include <GL/glu.h>

#include <ResourceManager.hpp>
#include <zrwops.hpp>
//----------------------------------------------------------------------------
MenuManager::MenuManager():
    _menu(0),
    _topMenu(0),
    _currentMenu(0),
    _mouseX(200.0),
    _mouseY(650.0),
    //_delayedExit(false),
    _angle(0.0),
    _prevAngle(0.0)
{
    XTRACE();
}
//----------------------------------------------------------------------------
MenuManager::~MenuManager()
{
    XTRACE();

    clearActiveSelectables();
    SelectableFactory::cleanup();

    delete _menu;
    _menu = 0;
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
    return true;
}
//----------------------------------------------------------------------------
void MenuManager::clearActiveSelectables( void)
{
    Selectable::reset();
    list<Selectable*>::iterator i;
    for( i=_activeSelectables.begin(); i!=_activeSelectables.end(); i++)
        delete (*i);
    _activeSelectables.clear();
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

    if (_activeSelectables.begin() != _activeSelectables.end())
        (*(_activeSelectables.begin()))->activate();
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
        nextTime = thisTime + 0.5f;

    _onlineUpdateDisplay.update();
    _prevAngle = _angle;
    _angle += 10.0f;

    for (list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); i++)
    {
        // grow or shrink text selectables
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

    float iAngle = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
    glPushMatrix();
    glTranslatef( 820.0, 620.0, 0.0);
    glRotatef(iAngle/7.0, 2.0, 3.0, 5.0);
    _mapleLeaf->draw();
    glPopMatrix();

    glPushMatrix();
    glTranslatef( 200.0, 620.0, 0.0);
    glRotatef(-iAngle/10.0, 2.0,3.0,5.0);
    _nextGenShippy->draw();
    glPopMatrix();

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
    if (val)
    {
        GLBitmapFont &fontWhite =
          *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
        glColor4f(1.0, 1.0, 1.0, 1.0);
        fontWhite.DrawString( (*val).c_str(), 122, 527, 0.5, 0.5);
    }

    for (list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); ++i)
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
void MenuManager::input( const Trigger &trigger, const bool &isDown)
{
    Trigger t = trigger;
    if (isDown)
    {
        switch( trigger.type)
        {
            case eKeyTrigger:
                switch( trigger.data1)
                {
                    case SDLK_RETURN:   Enter();            return;
                    case SDLK_ESCAPE:   exitMenu(false);    return;
                    case SDLK_UP:       Up();               return;
                    case SDLK_DOWN:     Down();             return;
                    case SDLK_F12:      VideoS::instance()->takeSnapshot(); return;
                    default:            break;
                }
                break;

            case eButtonTrigger:
                break;

            case eMotionTrigger:
                _mouseX += (trigger.fData1*10.0f);
                _mouseY += (trigger.fData2*10.0f);

                Clamp (_mouseX, 0.0f, 1000.0f);
                Clamp (_mouseY, 0.0f, 750.0f);
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
        if (!(*i))
        {
            LOG_ERROR << "Selectable is 0 !!!" << endl;
            continue;
        }
        const BoundingBox &r = (*i)->getInputBox();

        if( (_mouseX >= r.min.x) && (_mouseX <= r.max.x) &&
            (_mouseY >= r.min.y) && (_mouseY <= r.max.y))
        {
            (*i)->input(t, isDown);
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
void MenuManager::Down(void)
{
    XTRACE();
    for(list<Selectable*>::iterator i = _activeSelectables.begin();
        i!=_activeSelectables.end(); i++)
    {
        if ((*i)->isActive())
        {
            i++;
            if (i == _activeSelectables.end())
                i = _activeSelectables.begin();
            (*i)->activate();
            return;
        }
    }
}
//----------------------------------------------------------------------------
void MenuManager::Up( void)
{
    XTRACE();
    for(list<Selectable*>::iterator i = _activeSelectables.begin();
        i!=_activeSelectables.end(); i++)
    {
        if ((*i)->isActive())
        {
            if (i == _activeSelectables.begin())
                i = _activeSelectables.end();
            --i;
            (*i)->activate();
            return;
        }
    }
}
//----------------------------------------------------------------------------
void MenuManager::Enter( void)
{
    XTRACE();
    for (list<Selectable*>::iterator i = _activeSelectables.begin();
        i!=_activeSelectables.end(); i++)
    {
        if ((*i)->isActive())
        {
            (*i)->select();
            return;
        }
    }
}
//----------------------------------------------------------------------------
// returns true if we should exit
void MenuManager::exitMenu(bool allowQuitGame)
{
    XTRACE();
    if( _currentMenu != _topMenu)   // if not top level menu
    {
        _currentMenu = _currentMenu->Parent();
        loadMenuLevel();
        AudioS::instance()->playSample( "sounds/humm.wav");
    }
    else if (allowQuitGame)
        GameState::isAlive = false;
}
//----------------------------------------------------------------------------
// PLANET MANAGER ************************************************************
//----------------------------------------------------------------------------
// load graphics and stuff
bool PlanetManager::init()
{
    LOG_INFO << "PlanetManager::init()" << endl;

    GLBitmapCollection *icons =
        BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    if( !icons)
    {
        LOG_ERROR << "Unable to load menuIcons." << endl;
        return false;
    }
    _pointer = icons->getIndex("Pointer");

    for (int i=0; i<PLANET_TEXTURES; ++i)
    {
        char fn[30];
        sprintf(fn, "bitmaps/planet%d.png", i);
        GLTexture* t = ResourceManagerS::instance()->getTexture(fn);
        if (!t)
            return false;
        _planetTex.push_back(t);
    }

    // create clickable stuff available in all tabs
    const GLfloat tabh = 60.0f;
    const GLfloat tabw[] = { 270.0f, 300.0f, 180.0f, 170.0f };
    const char *actions[] = { "ShowMap", "ShowCargo", "ShowQuests", "MainMenu" };
    const char *labels[] = { "Galactic Map", "Cargo & Trade", "Quests", "Quit" };
    const char *infos[] = {
        "Display map and navigate planets",
        "Buy and sell goods and equipment",
        "Places to see and things to do",
        "Go back to main menu"
    };
    GLfloat loffset = 20.0f;
    for (unsigned int i=0; i < sizeof(tabw)/sizeof(GLfloat); ++i)
    {
        BoundingBox r;
        r.min = Point2D(loffset + 0.5f * tabw[i], 740.0f - tabh + 5.0f);
        Selectable *s = new ActionSelectable(r, actions[i], labels[i], infos[i], 1.4f);
        s->_infoLocation.x = 20.0f;
        s->_infoLocation.y = 17.0f;
        _activeSelectables.insert( _activeSelectables.end(), s);
        loffset += tabw[i];
    }
    return true;
}
//----------------------------------------------------------------------------
void PlanetManager::setActiveScreen(ScreenType newone)
{
    Selectable::reset();
    _screenType = newone;
}
//----------------------------------------------------------------------------
bool PlanetManager::update()
{
    _prevAngle = _angle;
    _angle += 1.0f;

    for(list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); ++i)
    {
        (*i)->update();
    }
    return true;
}
//----------------------------------------------------------------------------
void PlanetManager::Enter( void)
{
    XTRACE();
    for (list<Selectable*>::iterator i = _activeSelectables.begin();
        i!=_activeSelectables.end(); i++)
    {
        if ((*i)->isActive())
        {
            (*i)->select();
            return;
        }
    }
}
//----------------------------------------------------------------------------
void PlanetManager::drawCargo()
{
    Planet* _currentPlanet  = GameS::instance()->_galaxy.getPlanetAt(30, 30);
    drawPlanet(10.0f, 590.0f, _currentPlanet, "CURRENT PLANET");

    //if (in orbit)
    //{
    //    hide trade buttons and prices
    //    show text "IN ORBIT" under planet
    //}
    //else
    //{
    //    show text "DOCKED IN SPACESTATION"
    //}
}
//----------------------------------------------------------------------------
void setMinLineSize(float desiredSize)
{
    GLfloat sizes[2];   // Store supported line width range
    GLfloat step;       // Store supported line width increments
    glGetFloatv(GL_LINE_WIDTH_RANGE,sizes);
    glGetFloatv(GL_LINE_WIDTH_GRANULARITY,&step);
    GLfloat size = sizes[0];
    while (size < desiredSize && size + step <= sizes[1] && step > 0)
        size += step;
    glLineWidth(size);
}
//----------------------------------------------------------------------------
void gauge(const std::string& label, float x, float y,
    float w, float h, float value, float maxvalue)
{
    const float labeloffset = 1.0f;
    // draw label
    glColor3f(1.0f, 1.0f, 1.0f);
    GLBitmapFont &fontWhite = *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
    fontWhite.DrawString(label.c_str(), x, y, 0.5f, 0.5f);

    // fill in the gauge
    glBegin(GL_POLYGON);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f( x,                  y-h-labeloffset );
        glVertex2f( x,                  y  -labeloffset );
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f( x+w*value/maxvalue, y  -labeloffset );
        glVertex2f( x+w*value/maxvalue, y-h-labeloffset );
    glEnd();

    char buff[50];
    if (maxvalue == 100.0f) // percentages
        sprintf(buff, "%0.0f%%", value);
    else
        sprintf(buff, "%0.0f", value);
    float width = fontWhite.GetWidth(buff, 0.5f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fontWhite.DrawString(buff, x+w-width, y, 0.5f, 0.5f);

    // draw surrounding rectangle
    setMinLineSize(2.0f);
    glColor3f(1.0f, 8.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f( x,   y  -labeloffset );
        glVertex2f( x+w, y  -labeloffset );
        glVertex2f( x+w, y-h-labeloffset );
        glVertex2f( x,   y-h-labeloffset );
    glEnd();
}
//----------------------------------------------------------------------------
void PlanetManager::drawPlanet(float x, float y, Planet *pl,
    const std::string& title)
{
    if (!pl)
        return;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    GLUquadricObj *qobj = gluNewQuadric();
    //gluQuadricNormals(qobj, GL_NONE);
    gluQuadricTexture(qobj, GL_TRUE);

    if (qobj)
    {
        glPushMatrix();
            glEnable(GL_POLYGON_SMOOTH);
            glTranslatef(x + 100.0f, y, 0.0f);   // move
            float ang = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
            glRotatef(50.0f, 1.0f, 0.0f, 0.0f);   // 50deg. X-axis
            glRotatef(ang, 0.0f, 0.0f, -1.0f);     // rotate Z axis all the time
            _planetTex[pl->_textureIndex]->bind();
            gluSphere(qobj, pl->_radius, 48, 16);       // 60 = radius
            glDisable(GL_POLYGON_SMOOTH);
        glPopMatrix();
    }
    gluDeleteQuadric(qobj);
    glDisable(GL_TEXTURE_2D);

    // title and planet name
    GLBitmapFont &fontWhite = *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
    glColor4f(1.0f, 1.0f, 0.7f, 0.7f);
    fontWhite.DrawString(title.c_str(), x+10.0f, y+60.0f, 0.4f, 0.4f);
    float w = fontWhite.GetWidth(pl->_name.c_str(), 0.6f);
    fontWhite.DrawString(pl->_name.c_str(), 200.0f - w, y - pl->_radius - 10.0f, 0.6f, 0.6f);

    // draw stats
    gauge("Tech level",      25.0f, y-100, 170, 15, pl->_techLevel, 9);
    gauge("Rebel sentiment", 25.0f, y-145, 170, 15, pl->_rebelSentiment, 100);
    gauge("Alien activity",  25.0f, y-190, 170, 15, pl->_alienActivity, 100);

    // TODO: DISTANCE: xxx.x LY - maybe in bottom left corner of planet's image
}
//----------------------------------------------------------------------------
void PlanetManager::drawMap()
{
    // galaxy offset on screen
    const float gxoffset = 215.0f;
    const float gyoffset = 65.0f;

    // draw rectangle around galaxy
    glColor3f(0.0f, 0.7f, 0.0f);    // darker green color
    glBegin(GL_LINE_LOOP);
        glVertex2f( 980.0f,  60.0f);
        glVertex2f( 980.0f, 670.0f);
        glVertex2f( 210.0f, 670.0f);
        glVertex2f( 210.0f,  60.0f);
    glEnd();

    drawPlanet(10.0f, 270.0f, _hyperspaceTarget, "HYPERSPACE TARGET");

    Map& galaxy = GameS::instance()->_galaxy;
    galaxy.draw(gxoffset, gyoffset);

    // if mouse is inside, draw special cursor
    if (_mouseX >= 210 && _mouseX <= 980 && _mouseY >= 60 && _mouseY <= 670)
    {
        // if mouse is over some planet, center it
        float tmpx = _mouseX;
        float tmpy = _mouseY;
        Planet *pl = galaxy.getNearest(_mouseX - gxoffset, _mouseY - gyoffset);
        if (pl)
        {
            tmpx = gxoffset + pl->_x - 1.0f;
            tmpy = gyoffset + pl->_y + 1.0f;
            drawPlanet(10.0f, 590.0f, pl, "CURRENT SELECTION");
        }

        // plusing cursor (needs reworking so machine speed doesn't affect it)
        static float trans = 0.4f;
        static float dir = 0.005f;
        trans += dir;
        if (trans > 0.8f || trans < 0.4f)
            dir = -dir;
        glColor4f(0.0f, 1.0f, 0.0f, trans);

        setMinLineSize(3.0f);
        glEnable(GL_LINE_SMOOTH);
        #if 0
        const float ptrSize = 15.0f;
        const float ptrLen = 10.0f;
        glBegin(GL_LINE_LOOP);  // around
            glVertex2f( tmpx - ptrSize, tmpy - ptrSize);
            glVertex2f( tmpx - ptrSize, tmpy + ptrSize);
            glVertex2f( tmpx + ptrSize, tmpy + ptrSize);
            glVertex2f( tmpx + ptrSize, tmpy - ptrSize);
        glEnd();
        glBegin(GL_LINES);      // +
            glVertex2f( tmpx - ptrSize - ptrLen, tmpy);
            glVertex2f( tmpx - ptrSize + ptrLen, tmpy);
            glVertex2f( tmpx + ptrSize - ptrLen, tmpy);
            glVertex2f( tmpx + ptrSize + ptrLen, tmpy);
            glVertex2f( tmpx, tmpy - ptrSize - ptrLen);
            glVertex2f( tmpx, tmpy - ptrSize + ptrLen);
            glVertex2f( tmpx, tmpy + ptrSize - ptrLen);
            glVertex2f( tmpx, tmpy + ptrSize + ptrLen);
        glEnd();
        #else
        const float offs = 5.0f;
        const float len = 10.0f;
        glBegin(GL_LINE_STRIP);      // J
            glVertex2f( tmpx - offs - len, tmpy + offs);
            glVertex2f( tmpx - offs      , tmpy + offs);
            glVertex2f( tmpx - offs      , tmpy + offs + len);
        glEnd();
        glBegin(GL_LINE_STRIP);     // L
            glVertex2f( tmpx + offs + len, tmpy + offs);
            glVertex2f( tmpx + offs      , tmpy + offs);
            glVertex2f( tmpx + offs      , tmpy + offs + len);
        glEnd();
        glBegin(GL_LINE_STRIP);     // 7
            glVertex2f( tmpx - offs - len, tmpy - offs);
            glVertex2f( tmpx - offs      , tmpy - offs);
            glVertex2f( tmpx - offs      , tmpy - offs - len);
        glEnd();
        glBegin(GL_LINE_STRIP);     // F
            glVertex2f( tmpx + offs + len, tmpy - offs);
            glVertex2f( tmpx + offs      , tmpy - offs);
            glVertex2f( tmpx + offs      , tmpy - offs - len);
        glEnd();
        #endif
        glDisable(GL_LINE_SMOOTH);
    }
}
//----------------------------------------------------------------------------
void PlanetManager::drawQuests()
{
}
//----------------------------------------------------------------------------
bool PlanetManager::draw()
{
    setMinLineSize(2.0f);
    // draw lines
    const GLfloat tabh = 60.0f;
	const GLfloat tabw[] = { 270.0f, 300.0f, 180.0f, 170.0f };
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
    for (unsigned int i=0; i < sizeof(tabw)/sizeof(GLfloat); ++i)
    {
        left += tabw[i];
        if (i == (unsigned int)_screenType)
            continue;
        glBegin(GL_LINE_STRIP);
        if (i < (unsigned int)_screenType)
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

    // draw line separating info line from the rest of tab contents
    glBegin(GL_LINE_STRIP);
    glVertex2f(  20.0f, 50.0f );
    glVertex2f( 980.0f, 50.0f );
    glEnd();

    // tab selectors are selectables
    for (list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); i++)
    {
        (*i)->draw();
    }

    // render the active page
    if (_screenType == stMap)
        drawMap();
    if (_screenType == stTrade)
        drawCargo();
    if (_screenType == stQuests)
        drawQuests();

    // render mouse pointer unless in galaxy map
    if (_screenType != stMap ||
        _mouseX < 210 || _mouseX > 980 || _mouseY < 60 || _mouseY > 670)
    {
        glEnable(GL_TEXTURE_2D);
        GLBitmapCollection *icons =
            BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
        icons->bind();
        glColor4f(1.0, 1.0, 1.0, 1.0);
        icons->Draw( _pointer, _mouseX, _mouseY, 0.5, 0.5);
        glDisable(GL_TEXTURE_2D);
    }
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
                    case SDLK_ESCAPE:   GameS::instance()->previousContext();  return;
                    case SDLK_UP:       Up();           break;
                    case SDLK_DOWN:     Down();         break;
                    case SDLK_LEFT:     Left();         break;
                    case SDLK_RIGHT:    Right();        break;
                    case SDLK_F12:      VideoS::instance()->takeSnapshot();  break;
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

    //put the absolute mouse position into trigger
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
void PlanetManager::reload()
{
    for (PlanetTexList::iterator it = _planetTex.begin();
        it != _planetTex.end(); ++it)
    {
        (*it)->reload();
    }
}
//----------------------------------------------------------------------------
PlanetManager::~PlanetManager()
{
    XTRACE();

    Selectable::reset();
    LOG_INFO << "DELETING SELECTABLES" << endl;
    for(list<Selectable*>::iterator i = _activeSelectables.begin();
        i != _activeSelectables.end(); ++i)
    {
        delete (*i);
    }
    _activeSelectables.clear();

    LOG_INFO << "DELETING PLANETS" << endl;
    SelectableFactory::cleanup();
    for (PlanetTexList::iterator it = _planetTex.begin();
        it != _planetTex.end(); ++it)
    {
        delete (*it);
    }
}
//----------------------------------------------------------------------------
PlanetManager::PlanetManager():
    _mouseX(200.0),
    _mouseY(650.0),
    _angle(0.0),
    _prevAngle(0.0)
{
    _screenType = stMap;
}
//----------------------------------------------------------------------------

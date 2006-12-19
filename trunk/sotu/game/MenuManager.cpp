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
// Some utility functions ****************************************************
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
    float w, float h, float value, float maxvalue, std::string text = "")
{
    if (value > maxvalue)
        maxvalue = value;

    const float labeloffset = 1.0f;
    // draw label
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
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

    if (text == "")
    {
        char buff[50];
        if (maxvalue == 100.0f) // percentages
            sprintf(buff, "%0.0f%%", value);
        else
            sprintf(buff, "%0.0f", value);
        text = buff;
    }
    float width = fontWhite.GetWidth(text.c_str(), 0.5f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fontWhite.DrawString(text.c_str(), x+w-width, y, 0.5f, 0.5f);

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
// Menu manager **************************************************************
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
    const GLfloat tabw[] =  { 240.0f,         290.0f,          140.0f,       150.0f,    110.0f };
    const char *actions[] = { "ShowMap",      "ShowCargo",     "ShowQuests", "NewGame", "MainMenu" };
    const char *labels[] =  { "Galactic Map", "Cargo & Trade", "Quest",      "Launch",  "Quit" };
    const char *infos[] = {
        "Display map and navigate planets",
        "Buy and sell goods and equipment",
        "Places to see and things to do",
        "Exit spacestation and go into planet's orbit",
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
void drawGun()
{
    float visina = 10.0f;
    float sirina = 18.0f;
    float debljina = 2.0f;
    float cev = 4.0f;
    float ng = 2.0f;
    glColor4f(0.8f, 1.0f, 0.6f, 1.0f);
    glBegin(GL_QUAD_STRIP);
        glVertex3f(-sirina,   -visina, -debljina);
        glVertex3f(-sirina/2, -visina, -debljina);
        glVertex3f(-sirina + ng,    visina, -debljina);
        glVertex3f(-sirina/2 + ng,  cev, -debljina);
        glVertex3f( sirina, visina,        -debljina);
        glVertex3f( sirina, cev, -debljina);

        glVertex3f( sirina, visina,        debljina);
        glVertex3f( sirina, cev, debljina);
        glVertex3f(-sirina + ng,    visina, debljina);
        glVertex3f(-sirina/2 + ng,  cev, debljina);
        glVertex3f(-sirina,   -visina, debljina);
        glVertex3f(-sirina/2, -visina, debljina);
    glEnd();

    glBegin(GL_QUAD_STRIP);
        glVertex3f( sirina, visina,        -debljina);
        glVertex3f( sirina, visina,         debljina);

        glVertex3f(-sirina + ng, visina,        -debljina);
        glVertex3f(-sirina + ng, visina,         debljina);

        glVertex3f(-sirina, -visina,        -debljina);
        glVertex3f(-sirina, -visina,         debljina);

        glVertex3f(-sirina/2, -visina,        -debljina);
        glVertex3f(-sirina/2, -visina,         debljina);

        glVertex3f(-sirina/2 + ng, cev,        -debljina);
        glVertex3f(-sirina/2 + ng, cev,         debljina);

        glVertex3f( sirina, cev,        -debljina);
        glVertex3f( sirina, cev,         debljina);
    glEnd();
}
//----------------------------------------------------------------------------
void drawButton(float x, float y, float w, float h, bool highlight)
{
    if (highlight)
    {
        glColor4f(0.0f, 0.0f, 0.6f, 1.0f);
        glBegin(GL_POLYGON);
            glVertex2f( x,   y+h);
            glVertex2f( x+w, y+h);
            glVertex2f( x+w, y);
            glVertex2f( x,   y);
        glEnd();

        static float lastx = 0;
        static float lasty = 0;
        if (lastx != x || lasty != y)
        {
            AudioS::instance()->playSample( "sounds/beep.wav");
            lastx = x;
            lasty = y;
        }
    }
    if (highlight)
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    else
        glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
    setMinLineSize(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f( x,   y+h);
        glVertex2f( x+w, y+h);
        glVertex2f( x+w, y);
        glVertex2f( x,   y);
    glEnd();
}
//----------------------------------------------------------------------------
void buyInfo(Planet::BuyStatus bs, CargoItemInfo& c, int price, int money,
    GLBitmapFont &font)
{
    Selectable::reset();
    char buff[300];
    std::string msg;
    if (bs == Planet::bsOk)
    {
        sprintf(buff, "Buy %s for %d credits", c._name.c_str(), price);
        msg = buff;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

    if (bs == Planet::bsNA)
        msg = "Not available for sale";
    else if (bs == Planet::bsNoTech)
    {
        sprintf(buff, "Item is only available on planets with tech.level %d or higher",
            c._techLevelRequired);
        msg = buff;
    }
    else if (bs == Planet::bsNoMoney)
    {
        sprintf(buff, "Item costs %d, but you only have %d credits", price, money);
        msg = buff;
    }
    else if (bs == Planet::bsMAX)
    {
        sprintf(buff, "You already have maximum possible quantity (%d)", c._maxQty);
        msg = buff;
    }
    else if (bs == Planet::bsCargoFull)
        msg = "No more space in cargo hull.";
    font.DrawString(msg.c_str(), 20.0f, 17.0f, 0.65f, 0.65f);
}
//----------------------------------------------------------------------------
void PlanetManager::drawItemIcon(CargoItemInfo& info, float offset)
{
    string model = info._modelName;
    if (model != "")
    {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        Model *m = 0;
        if (model != "GUN")
            m = ModelManagerS::instance()->getModel(model.c_str());
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        GLfloat light_position2[] = { 820.0, 620.0, 500.0, 0.0 };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position2);
        glPushMatrix();
            float ypos = offset + 17.0f;
            if (model == "models/ShieldBoost")
                ypos -= 5.0f;
            glTranslatef(240.0, ypos, 25.0f);
            float ang = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
            glRotatef(-ang+offset, 0.0, 1.0, 0.0);
            if (info._scale != 1.0f)
                glScalef(info._scale, info._scale, info._scale);
            if (m)
                m->draw();
            else
                drawGun();
        glPopMatrix();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
    }
}
//----------------------------------------------------------------------------
void PlanetManager::drawPlayer(float yoffset)
{
    GLBitmapFont &fontWhite = *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
    glColor4f(1.0f, 1.0f, 0.7f, 0.7f);
    fontWhite.DrawString("PLAYER INFO", 20.0f, yoffset, 0.4f, 0.4f);

    // spaceship model
    Model *m = ModelManagerS::instance()->getModel("models/Hero");
    if (m)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        GLfloat light_position2[] = { 820.0, 620.0, 500.0, 0.0 };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position2);
        glPushMatrix();
            glTranslatef(100.0, yoffset - 40.0f, 1.0f);
            float ang = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
            if (ang > 360)
                ang = (int)ang % 360;
            if (ang > 270)          // -90..0
                ang = ang - 360;
            else if (ang > 90)     //   0..-90
                ang = 180 - ang;
            glRotatef(ang, 0.0, 1.0, 0.0);
            glScalef(6.0f, 6.0f, 6.0f);
            m->draw();
        glPopMatrix();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
    }

    // between fighter image and text
    yoffset -= 80.0f;

    // money     20.f
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    const float spacing = 30.0f;
    yoffset -= spacing;
    fontWhite.DrawString("Money:", 20.0f, yoffset, 0.5f, 0.5f);
    char buff[30];
    sprintf(buff, "%d", GameS::instance()->_money);
    glColor4f(1, 1, 0, 1);
    fontWhite.DrawString(buff, 200.0f, yoffset, 0.65f, 0.65f, GLBitmapFont::alRight);

    // rebel & empire status
    std::string statuss[] = { "Rebel status:", "Empire status:" };
    int sti[] = { GameS::instance()->_rebelStatus, GameS::instance()->_empireStatus };
    std::string stname[] = { "Clean", "Fugitive", "Terrorist" };
    for (int i=0; i<2; ++i)
    {
        yoffset -= spacing;
        glColor4f(1, 1, 1, 1);
        fontWhite.DrawString(statuss[i].c_str(), 20.0f, yoffset, 0.5f, 0.5f);
        if (sti[i] == psClean)
            glColor4f(0, 1, 0, 1);
        else if (sti[i] == psFugitive)
            glColor4f(1, 1, 0, 1);
        else
            glColor4f(1, 0, 0, 1);
        fontWhite.DrawString(stname[sti[i]].c_str(), 200.0f, yoffset, 0.5f, 0.5f, GLBitmapFont::alRight);
    }

    yoffset -= spacing;
    int rep_kills[] = { 0, 500, 1000, 2500, 4500, 6500, 9000 };
    std::string reputation[] = { "Harmless", "Poor", "Average", "Competent",
        "Dangerous", "Deadly", "Elite" };
    std::string reps;
    int kills = GameS::instance()->_kills;
    for (unsigned int i = 0; i < sizeof(reputation)/sizeof(string); ++i)
        if (kills >= rep_kills[i])
            reps = reputation[i];
    gauge("Reputation:", 20.0f, yoffset, 180.0f, 15, kills, 9000, reps.c_str());
}
//----------------------------------------------------------------------------
void PlanetManager::drawCargo()
{
    float offset = 630.0f;

    Planet *pl = GameS::instance()->_currentPlanet;
    drawPlanet(10.0f, 590.0f, pl, "CURRENT PLANET");
    bool landed = GameS::instance()->_landed;
    int money = GameS::instance()->_money;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    GLBitmapFont &fontWhite = *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
    if (landed)
    {
        fontWhite.DrawString("DOCKED IN", 110.0f, 340.0f, 0.5f, 0.5f, GLBitmapFont::alCenter);
        fontWhite.DrawString("SPACESTATION", 110.0f, 320.0f, 0.5f, 0.5f, GLBitmapFont::alCenter);
    }
    else
    {
        fontWhite.DrawString("FLOATING IN", 110.0f, 340.0f, 0.5f, 0.5f, GLBitmapFont::alCenter);
        fontWhite.DrawString("OUTER ORBIT", 110.0f, 320.0f, 0.5f, 0.5f, GLBitmapFont::alCenter);
    }

    drawPlayer(280.0f);

    // draw surrounding graphics
    setMinLineSize(2.0f);
    glColor3f(0.7f, 0.7f, 0.0f);    // darker yellow color
    glBegin(GL_LINE_LOOP);
        glVertex2f( 980.0f,  60.0f);
        glVertex2f( 980.0f, 670.0f);
        glVertex2f( 210.0f, 670.0f);
        glVertex2f( 210.0f,  60.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex2f( 970.0f, offset);
        glVertex2f( 220.0f, offset);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    float columns[] = { 280.0f, 600.0f, 750.0f };
    float fsize = 0.65f;
    fontWhite.DrawString("ITEM NAME", columns[0], offset, fsize, fsize);
    fontWhite.DrawString("YOU HAVE",  columns[1], offset, fsize, fsize, GLBitmapFont::alRight);
    fontWhite.DrawString("PRICE",     columns[2], offset, fsize, fsize, GLBitmapFont::alRight);
    fontWhite.DrawString("TRADE",         900.0f, offset, fsize, fsize, GLBitmapFont::alCenter);

    // draw cargo
    std::vector<CargoItemInfo>* info = CargoItemInfo::getCargoInfo();
    Cargo &pc = GameS::instance()->_cargo;

    int total = 0;
    for (std::vector<CargoItemInfo>::iterator it = info->begin();
        it != info->end(); ++it)
    {
        CargoItem *c = pc.findItem((*it)._name);
        if ((*it)._weight > 0)
            total += c->_quantity * (*it)._weight;
    }

    for (std::vector<CargoItemInfo>::iterator it = info->begin();
        it != info->end(); ++it)
    {
        offset -= 35.0f;
        drawItemIcon(*it, offset);

        if ((*it)._legalStatus == CargoItemInfo::lsLegal)
            glColor4f(0.8f, 0.8f, 1.0f, 1.0f);
        else
            glColor4f(1.0f, 0.8f, 0.8f, 1.0f);
        fontWhite.DrawString((*it)._name.c_str(), columns[0], offset, fsize, fsize);
        CargoItem *c = pc.findItem((*it)._name);
        char buff[30];
        sprintf(buff, "%d", c->_quantity);
        fontWhite.DrawString(buff, columns[1], offset, fsize, fsize, GLBitmapFont::alRight);

        int price = pl->getPrice((*it)._name);
        sprintf(buff, "%d", price);
        fontWhite.DrawString(buff, columns[2], offset, fsize, fsize, GLBitmapFont::alRight);
        fontWhite.DrawString(" cr.", columns[2], offset, fsize, fsize);

        Planet::BuyStatus bs = pl->canBuy(*it);
        if ((*it)._maxQty > 0 && ((*it)._maxQty == c->_quantity))
        {
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            fontWhite.DrawString(" MAX", columns[1], offset + 2.0f, 0.5f, 0.5f, GLBitmapFont::alLeft);
            bs = Planet::bsMAX;
        }
        if ((*it)._weight != 0 && total >= 20)  // !=0 is important as it can be -1
            bs = Planet::bsCargoFull;

        // set color if box is higlighted
        bool hi_buy = false;
        bool hi_sell = false;
        if (_mouseX >= 210 && _mouseX <= 980 && _mouseY > offset && _mouseY < offset + 35.0f)
        {
            glColor4f(0.0f, 1.0f, 0.0f, 0.3f);
            glBegin(GL_POLYGON);
                glVertex2f( 980.0f, offset + 35.0f);
                glVertex2f( 980.0f, offset);
                glVertex2f( 210.0f, offset);
                glVertex2f( 210.0f, offset + 35.0f);
            glEnd();
            glColor4f(3.0f, 1.0f, 3.0f, 0.8f);
            fontWhite.DrawString((*it)._info.c_str(), 595.0f, 70.0f, 0.6f, 0.6f, GLBitmapFont::alCenter);

            if (_mouseX > 820.0f && _mouseX < 880.0f)
            {
                hi_buy = true;
                buyInfo(bs, *it, price, money, fontWhite);
            }
            if (_mouseX > 900.0f && _mouseX < 960.0f)
                hi_sell = true;
        }

        // trade items
        if (bs != Planet::bsCargoFull)
        {
            const std::string names[] = { "N/A", "N/T", "N/M", "MAX", "BUY", "FULL" };
            setMinLineSize(1.0f);
            drawButton( 820.0f, offset + 5.0f, 60.0f, 25.f, hi_buy && (bs == Planet::bsOk));
            if (bs == Planet::bsOk)
                glColor4f(1.0f, 0.852f, 0.0f, 1.0f);
            else
                glColor4f(1.0f, 0.2f, 0.0f, 1.0f);
            fontWhite.DrawString(names[(int)bs].c_str(), 850.0f, offset + 3.0f, 0.6f, 0.6f, GLBitmapFont::alCenter);
        }

        if (c->_quantity > 0)   // sell
        {
            drawButton( 900.0f, offset + 5.0f, 60.0f, 25.f, hi_sell);
            glColor4f(1.0f, 0.852f, 0.0f, 1.0f);
            fontWhite.DrawString("SELL", 930.0f, offset + 3.0f, 0.6f, 0.6f, GLBitmapFont::alCenter);
        }

        if ((*it)._name == "Slaves")
        {
            setMinLineSize(2.0f);
            glColor3f(0.7f, 0.7f, 0.0f);        // darker yellow color
            glBegin(GL_LINE_STRIP);
                glVertex2f( columns[0], offset);
                glVertex2f( columns[1], offset);
            glEnd();
            offset -=35;
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            fontWhite.DrawString("Available ship capacity", columns[0], offset, fsize, fsize);
            char buff2[30];
            sprintf(buff2, "%d", ( total >= 20 ? 0 : 20 - total));
            if (total >= 20)
                glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            fontWhite.DrawString(buff2, columns[1], offset, fsize, fsize, GLBitmapFont::alRight);
        }
        if ((*it)._name == "Slaves" || (*it)._name == "Fuel")   // separator
        {
            setMinLineSize(2.0f);
            glColor3f(0.7f, 0.7f, 0.0f);        // darker yellow color
            glBegin(GL_LINE_STRIP);
                glVertex2f( 970.0f, offset);
                glVertex2f( 220.0f, offset);
            glEnd();
        }
    }

    if (total >= 20)
    {
        glPushMatrix();
            glTranslatef(870.0f, 500.0f, 0.0f);
            glRotatef(90.0f, 0, 0, 1);
            glColor4f(1.0f, 0.852f, 0.0f, 1.0f);
            fontWhite.DrawString("SHIP FULL", 0.0f, 0.0f, 1.0f, 1.0f, GLBitmapFont::alCenter);
        glPopMatrix();
    }
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
}
//----------------------------------------------------------------------------
void PlanetManager::drawCursor(float x, float y, bool special)
{
    float ang = _prevAngle+(_angle-_prevAngle)*GameState::frameFractionOther;
    int angi = (int)ang;
    angi %= 80;
    if (angi > 40)
        angi = 80 - angi;
    float alpha = 0.4f + (float)angi * 0.01f;
    float offs = 5.0f;
    float len = 10.0f;
    if (special)
    {
        glColor4f(1, 1, 1, 1);
        setMinLineSize(2.0f);
        offs = 4.0f - (float)angi * 0.08f;
        len = 4.0f;
    }
    else
    {
        glColor4f(0, 1, 0, alpha);
        setMinLineSize(3.0f);
    }

    glEnable(GL_LINE_SMOOTH);
    glBegin(GL_LINE_STRIP);      // J
        glVertex2f( x - offs - len, y + offs);
        glVertex2f( x - offs      , y + offs);
        glVertex2f( x - offs      , y + offs + len);
    glEnd();
    glBegin(GL_LINE_STRIP);     // L
        glVertex2f( x + offs + len, y + offs);
        glVertex2f( x + offs      , y + offs);
        glVertex2f( x + offs      , y + offs + len);
    glEnd();
    glBegin(GL_LINE_STRIP);     // 7
        glVertex2f( x - offs - len, y - offs);
        glVertex2f( x - offs      , y - offs);
        glVertex2f( x - offs      , y - offs - len);
    glEnd();
    glBegin(GL_LINE_STRIP);     // F
        glVertex2f( x + offs + len, y - offs);
        glVertex2f( x + offs      , y - offs);
        glVertex2f( x + offs      , y - offs - len);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
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

    drawPlanet(10.0f, 590.0f, _hyperspaceTarget, "HYPERSPACE TARGET");
    if (_hyperspaceTarget)
        drawCursor( gxoffset + _hyperspaceTarget->_x, gyoffset + _hyperspaceTarget->_y, true);

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
            drawPlanet(10.0f, 270.0f, pl, "CURRENT SELECTION");
        }

        // Info text: distance from planet
        Planet *p = GameS::instance()->_currentPlanet;
        if (p)
        {
            Selectable::reset(true);    // deactivate active node
            GLBitmapFont &fontWhite = *(FontManagerS::instance()->getFont( "bitmaps/menuWhite"));
            if (p == pl)
            {
                glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
                fontWhite.DrawString(pl->_name.c_str(), 20.0f, 17.0f, 0.7f, 0.65f);
                glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
                float w = fontWhite.GetWidth(pl->_name.c_str(), 0.7f);
                fontWhite.DrawString(" - current planet", w + 20.0f, 17.0f, 0.7f, 0.65f);
            }
            else
            {
                glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
                fontWhite.DrawString("Distance: ", 20.0f, 17.0f, 0.7f, 0.65f);
                float w = fontWhite.GetWidth("Distance: ", 0.7f);

                float dist = pl->getDistance(p->_x, p->_y);
                CargoItem *c = GameS::instance()->_cargo.findItem("Fuel");
                if (c->_quantity >= dist)
                    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
                else
                    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
                char buff[300];
                sprintf(buff, "%0.1f", dist);
                fontWhite.DrawString(buff, w + 20.0f, 17.0f, 0.7f, 0.65f);
                w += fontWhite.GetWidth(buff, 0.7f);

                glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
                fontWhite.DrawString(" light years. ", w + 20.0f, 17.0f, 0.7f, 0.65f);
                w += fontWhite.GetWidth(" light years. ", 0.7f);

                glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
                char ok[] = "Click to set as hyperspace target.";
                char too_far[] = "Planet is out of range.";
                char *text = (c->_quantity >= dist ? ok : too_far);
                fontWhite.DrawString(text, w + 20.0f, 17.0f, 0.7f, 0.65f);
            }
        }

        drawCursor(tmpx, tmpy, false);
    }
}
//----------------------------------------------------------------------------
void PlanetManager::drawQuests()
{
    drawPlayer(650);
}
//----------------------------------------------------------------------------
bool PlanetManager::draw()
{
    setMinLineSize(2.0f);
    // draw lines
    const GLfloat tabh = 60.0f;
    const GLfloat tabw[] =  { 240.0f,         290.0f,          140.0f,       150.0f,    110.0f };
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
void PlanetManager::planetClick()
{
    const float gxoffset = 215.0f;
    const float gyoffset = 65.0f;
    if (_mouseX >= 210 && _mouseX <= 980 && _mouseY >= 60 && _mouseY <= 670)
    {
        Planet *pl = GameS::instance()->_galaxy.getNearest(
            _mouseX - gxoffset, _mouseY - gyoffset);
        Planet *pc = GameS::instance()->_currentPlanet;
        if (!pl || !pc || pl == pc)
            return;
        float dist = pc->getDistance(pl->_x, pl->_y);
        CargoItem *c = GameS::instance()->_cargo.findItem("Fuel");
        if (c->_quantity >= dist)
        {
            _hyperspaceTarget = pl;
            AudioS::instance()->playSample("sounds/confirm.wav");
        }
    }
}
//----------------------------------------------------------------------------
void PlanetManager::tradeClick()
{
    if (!GameS::instance()->_landed)
        return;

    std::vector<CargoItemInfo>* info = CargoItemInfo::getCargoInfo();
    Cargo &pc = GameS::instance()->_cargo;
    int total = 0;
    for (std::vector<CargoItemInfo>::iterator it = info->begin();
        it != info->end(); ++it)
    {
        CargoItem *c = pc.findItem((*it)._name);
        total += c->_quantity * (*it)._weight;
    }

    Planet *pl = GameS::instance()->_currentPlanet;
    float offset = 630.0f;
    for (std::vector<CargoItemInfo>::iterator it = info->begin();
        it != info->end(); ++it)
    {
        offset -= 35.0f;
        CargoItem *c = pc.findItem((*it)._name);

        // skip the cargo sum
        if ((*it)._name == "Proton flank burst")
            offset -=35;

        if (_mouseY <= offset || _mouseY >= offset + 35.0f)
            continue;

        int price = pl->getPrice((*it)._name);
        if (_mouseX > 900.0f && _mouseX < 960.0f)
        {
            if (c->_quantity > 0)   // sell
            {
                c->_quantity--;
                GameS::instance()->_money += price;
                AudioS::instance()->playSample("sounds/whoop.wav");
            }
            else
                AudioS::instance()->playSample("sounds/bark.wav");
            return;
        }

        if (_mouseX <= 820.0f || _mouseX >= 880.0f)
            continue;
        if (total >= 20 && (*it)._weight > 0 ||
            Planet::bsOk != pl->canBuy(*it)  ||
            (*it)._maxQty > 0 && ((*it)._maxQty == c->_quantity))
        {
            AudioS::instance()->playSample("sounds/bark.wav");
            return;
        }
        c->_quantity++;
        GameS::instance()->_money -= price;
        AudioS::instance()->playSample("sounds/whoop.wav");
    }
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
                // handle mouse click
                if (_screenType == stMap)   // click on target planet
                    planetClick();
                if (_screenType == stTrade)
                    tradeClick();
                break;
            default:
                break;
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
    SelectableFactory::cleanup();

    LOG_INFO << "Deleting planet textures..." << endl;
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

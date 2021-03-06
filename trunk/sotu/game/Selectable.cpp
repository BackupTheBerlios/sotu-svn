// Description:c
//   A Selectable
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
#include <Tokenizer.hpp>
#include <Selectable.hpp>
#include <Config.hpp>
#include <Game.hpp>
#include <GameState.hpp>
#include <Audio.hpp>
#include <Input.hpp>
#include <MenuManager.hpp>
#include <FontManager.hpp>
#include <BitmapManager.hpp>
#include <ScoreKeeper.hpp>

#include <gl++.hpp>

Selectable *Selectable::_active = 0;
//------------------------------------------------------------------------------
Selectable::Selectable( const BoundingBox &r, const string &info):
    _infoLocation(122, 57),
    _inputBox(r),
    _boundingBox(r),
    _info(info)
{
    XTRACE();
    _fontWhite = FontManagerS::instance()->getFont( "bitmaps/menuWhite");
    if( !_fontWhite)
    {
        LOG_ERROR << "Unable to load menuWhite font." << endl;
    }
}
//------------------------------------------------------------------------------
Selectable::~Selectable()
{
    XTRACE();
    if (Selectable::_active == this)
    {
        LOG_INFO << "DELETING SELECTABLE::active (yes)" << endl;
        Selectable::reset();
    }
}
//------------------------------------------------------------------------------
void Selectable::draw( )
{
#if 0
    glColor4f( 1.0, 0.2, 0.2, 0.5);
    glBegin(GL_QUADS);
    glVertex3f( _inputBox.min.x, _inputBox.min.y, 0);
    glVertex3f( _inputBox.min.x, _inputBox.max.y, 0);
    glVertex3f( _inputBox.max.x, _inputBox.max.y, 0);
    glVertex3f( _inputBox.max.x, _inputBox.min.y, 0);
    glEnd();
#endif

    if( _active == this)
    {
        glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
        _fontWhite->DrawString( _info.c_str(), _infoLocation.x, _infoLocation.y,
            0.7f, 0.65f);
    }
}
//------------------------------------------------------------------------------
EscapeSelectable::EscapeSelectable( const BoundingBox &r, float size):
    Selectable(r, "Escape"),
    _size(size)
{
    _icons = BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    if( !_icons)
    {
        LOG_ERROR << "Unable to load menuIcons." << endl;
    }

    _exitOn = _icons->getIndex( "ExitOn");
    if( _exitOn == -1)
    {
        LOG_ERROR << "ExitOn button not found" << endl;
    }

    _exitOff = _icons->getIndex( "ExitOff");
    if( _exitOff == -1)
    {
        LOG_ERROR << "ExitOff button not found" << endl;
    }
}
//------------------------------------------------------------------------------
void EscapeSelectable::input(const Trigger &trigger, const bool &isDown)
{
    if( !isDown) return;

    switch( trigger.type)
    {
        case eButtonTrigger:
            MenuManagerS::instance()->exitMenu(true);   // true=allowQuitGame
            break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void EscapeSelectable::activate( )
{
    if( (_active != this))
    {
        if( _active)
            _active->deactivate();
        _active = this;
        AudioS::instance()->playSample("sounds/beep.wav");
    }
}
//------------------------------------------------------------------------------
void EscapeSelectable::draw( )
{
    Selectable::draw();

    _icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    if( (_active == this))
    {
        _icons->Draw( _exitOn,
            _boundingBox.min.x, _boundingBox.min.y, _size, _size);
    }
    else
    {
        _icons->Draw( _exitOff,
            _boundingBox.min.x, _boundingBox.min.y, _size, _size);
    }
    glDisable(GL_TEXTURE_2D);
}
//------------------------------------------------------------------------------
TextOnlySelectable::TextOnlySelectable(
    const BoundingBox &r, const string &text, const string &info,
    bool center, float size, float red, float green, float blue)
:
    Selectable(r, info), _text(text), r(red), g(green), b(blue), _size(size)
{
    _fontShadow = FontManagerS::instance()->getFont( "bitmaps/menuShadow");
    if( !_fontShadow)
    {
        LOG_ERROR << "Unable to load shadow font." << endl;
    }

    float width  = _fontWhite->GetWidth( _text.c_str(), _size);
    float height = _fontWhite->GetHeight( _size);

    if( center)
    {
        _boundingBox.min.x -= width*0.5f;
    }
    _boundingBox.max.x = _boundingBox.min.x + width;
    _boundingBox.max.y = _boundingBox.min.y + height;

    _inputBox = _boundingBox;

    _icons = BitmapManagerS::instance()->getBitmap( "bitmaps/menuIcons");
    if( !_icons)
    {
        LOG_ERROR << "Unable to load menuIcons." << endl;
    }
}
//------------------------------------------------------------------------------
void TextOnlySelectable::input( const Trigger &trigger, const bool &/*isDown*/)
{
    switch( trigger.type)
    {
        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void TextOnlySelectable::activate()
{
    if( (_active != this))
    {
        if( _active)
            _active->deactivate();
        _active = this;
    }
}
//------------------------------------------------------------------------------
void TextOnlySelectable::draw( )
{
    Selectable::draw();

    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString(
        _text.c_str(),
        _boundingBox.min.x+9*_size,_boundingBox.min.y-9*_size,
        _size, _size);

    glColor4f(r, g, b, 1.0);
    _fontWhite->DrawString(
        _text.c_str(),
        _boundingBox.min.x, _boundingBox.min.y,
        _size, _size);
}
//------------------------------------------------------------------------------
FloatSelectable::FloatSelectable(const BoundingBox &r, const string &text,
    const string &info,    const string &variable,
    const string &range,   const string &sliderOffset)
:
    TextOnlySelectable(r, text, info, false, 0.7f, 1.0f, 1.0f, 1.0f),
    _variable(variable), _startX(-1.0f)
{
    Tokenizer t(range);

    _min = (float)atof(t.next().c_str());
    _max = (float)atof(t.next().c_str());

    _sliderOffset = atoi(sliderOffset.c_str());

    float curVal = _min;
    ConfigS::instance()->getFloat( _variable, curVal);
    _xPos = (curVal-_min) * 140.0f / (_max-_min);

    //bounding box for double arrow
    _bRect.min.x = _boundingBox.min.x + 102 + _sliderOffset;
    _bRect.max.x = _bRect.min.x + 20;
    _bRect.min.y = _boundingBox.min.y ;
    _bRect.max.y = _bRect.min.y + 30;

    //we want ALL input
    _inputBox.min.x = 0;
    _inputBox.min.y = 0;
    _inputBox.max.x = 1000;
    _inputBox.max.y = 750;

    _slider = _icons->getIndex( "Slider");
    if( _slider == -1)
    {
        LOG_ERROR << "Slider not found" << endl;
    }
    _doubleArrow = _icons->getIndex( "DoubleArrow");
    if( _doubleArrow == -1)
    {
        LOG_ERROR << "DoubleArrow not found" << endl;
    }
}
//------------------------------------------------------------------------------
void FloatSelectable::input( const Trigger &trigger, const bool &isDown)
{
    float mouseX = trigger.fData1;
    float mouseY = trigger.fData2;

    switch( trigger.type)
    {
        case eButtonTrigger:
            if( isDown)
            {
                if( (mouseX >= (_bRect.min.x+_xPos)) && (mouseX <= (_bRect.max.x+_xPos))
                    && (mouseY >= _bRect.min.y)      && (mouseY <= _bRect.max.y))
                {
                    _startX = mouseX;
                }
            }
            else
            {
                _startX = -1.0f;
                float curVal = _min;
                ConfigS::instance()->getFloat( _variable, curVal);
                curVal = _min + _xPos*(_max-_min)/140.0f;

                Value *v = new Value( curVal);
                ConfigS::instance()->updateKeyword( _variable, v);
            }
            break;

        case eMotionTrigger:
            if( _startX >= 0.0f)
            {
                float dx =  mouseX - _startX;
                _startX = mouseX;
                _xPos += dx;
                Clamp( _xPos, 0.0, 140.0);
            }

            if( (mouseX >= _boundingBox.min.x)    && (mouseX <= _boundingBox.max.x)
                && (mouseY >= _boundingBox.min.y) && (mouseY <= _boundingBox.max.y))
            {
                this->activate();
            }
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void FloatSelectable::draw( )
{
#if 0
    glColor4f( 0.2, 0.2, 1.0, 0.5);
    glBegin(GL_QUADS);
    glVertex3f( _bRect.min.x+_xPos, _bRect.min.y, 0);
    glVertex3f( _bRect.min.x+_xPos, _bRect.max.y, 0);
    glVertex3f( _bRect.max.x+_xPos, _bRect.max.y, 0);
    glVertex3f( _bRect.max.x+_xPos, _bRect.min.y, 0);
    glEnd();
#endif
    TextOnlySelectable::draw();

    _icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    _icons->DrawC(
        _slider, _boundingBox.min.x+180+_sliderOffset, _boundingBox.min.y+15, 1.6f, 0.4f);
    _icons->DrawC( _doubleArrow,
        _boundingBox.min.x+112+_sliderOffset+_xPos, _boundingBox.min.y+15, 0.8f, 0.8f);
    glDisable(GL_TEXTURE_2D);
}
//------------------------------------------------------------------------------
EnumSelectable::EnumSelectable(const BoundingBox &r, const string &text,
    const string &info, const string &variable, const string &values)
:
    TextOnlySelectable(r, text, info, false, 0.7f, 1.0f, 1.0f, 1.0f),
    _variable(variable)
{
    string val;
    ConfigS::instance()->getString( _variable, val);
    Tokenizer t(values);
    string s = t.next();
    while( s != "")
    {
        _enumList.insert( _enumList.end(), s);
        if( s == val)
            _activeEnum = --_enumList.end();
        s = t.next();
    }

    _xOff = _boundingBox.max.x + 5.0f;
    _boundingBox.max.x += 100.0f;
    _inputBox = _boundingBox;
}
//------------------------------------------------------------------------------
void EnumSelectable::input( const Trigger &trigger, const bool &isDown)
{
    if( !isDown)
        return;

    switch( trigger.type)
    {
        case eButtonTrigger:
            {
                _activeEnum++;
                if( _activeEnum == _enumList.end())
                {
                    _activeEnum = _enumList.begin();
                }

                //      LOG_INFO << "New value for " << _variable
                //               << " is " << *_activeEnum << endl;

                Value *v = new Value( *_activeEnum);
                ConfigS::instance()->updateKeyword( _variable, v);
            }
            break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void EnumSelectable::draw( )
{
    TextOnlySelectable::draw();

    string val;
    ConfigS::instance()->getString( _variable, val);

    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString( val.c_str(), _xOff+9*_size,
        _boundingBox.min.y-9*_size, _size, _size);
    glColor4f(1.0f, 0.852f, 0.0f, 1.0f);
    //    glColor4f(r, g, b, 1.0);
    _fontWhite->DrawString(
        val.c_str(), _xOff, _boundingBox.min.y, _size, _size);
}
//------------------------------------------------------------------------------
BoolSelectable::BoolSelectable(const BoundingBox &r, const string &text,
    const string &info, const string &variable)
:
    TextOnlySelectable(r, text, info, false, 0.7f, 1.0f, 1.0f, 1.0f),
    _variable(variable)
{
    _checkmark = _icons->getIndex( "Checkmark");
    _checkmarkOff = _icons->getIndex( "CheckmarkOff");

    _xOff = _boundingBox.max.x + 10.0f;
    _boundingBox.max.x = _xOff + _icons->getWidth( _checkmark)*0.5f;

    _inputBox = _boundingBox;
}
//------------------------------------------------------------------------------
void BoolSelectable::input( const Trigger &trigger, const bool &isDown)
{
    if( !isDown)
        return;

    switch( trigger.type)
    {
        case eButtonTrigger:
            {
                bool val = false;
                ConfigS::instance()->getBoolean( _variable, val);
                Value *v = new Value( !val);

                //      LOG_INFO << "New value for " << _variable
                //               << " is " << v->getString() << endl;
                ConfigS::instance()->updateKeyword( _variable, v);
            }
            break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void BoolSelectable::draw( )
{
    TextOnlySelectable::draw();
    bool val = false;
    ConfigS::instance()->getBoolean( _variable, val);

    _icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    if( val)
        _icons->Draw( _checkmark, _xOff, _boundingBox.min.y+2, 0.5, 0.5);
    else
        _icons->Draw( _checkmarkOff, _xOff, _boundingBox.min.y+2, 0.5, 0.5);
    glDisable(GL_TEXTURE_2D);
}
//------------------------------------------------------------------------------
LeaderBoardSelectable::LeaderBoardSelectable(const BoundingBox &r,
    const string &text, const string &info)
:
    Selectable(r, info), _text(text), _size(1.0f)
{
    _fontShadow = FontManagerS::instance()->getFont( "bitmaps/menuShadow");

    float width  = _fontWhite->GetWidth( _text.c_str(), _size);
    float height = _fontWhite->GetHeight( _size);

    _boundingBox.min.x -= width*0.5f;
    _boundingBox.max.x = _boundingBox.min.x + width;
    _boundingBox.max.y = _boundingBox.min.y + height;

    _inputBox.min.x = 245;
    _inputBox.min.y =  90;
    _inputBox.max.x = 750;
    _inputBox.max.y = 520;
}
//------------------------------------------------------------------------------
void LeaderBoardSelectable::input( const Trigger &trigger, const bool &isDown)
{
    if( !isDown) return;

    float mouseY = trigger.fData2;
    int idx = (int)((mouseY-100.0f)/36.0f);
    if( idx < 0)
        idx = 0;
    if( idx > 9)
        idx = 9;

    switch( trigger.type)
    {
    case eButtonTrigger:
        //TODO: Load the saved game level here - go to planetary screen
        GameS::instance()->startNewGame();
        break;

    case eMotionTrigger:
        //_info = ScoreKeeperS::instance()->getInfoText( 9-idx);
        this->activate();
        break;

    default:
        break;
    }
}
//------------------------------------------------------------------------------
void LeaderBoardSelectable::activate( )
{
    if( (_active != this))
    {
        if( _active)
            _active->deactivate();
        _active = this;
    }
}
//------------------------------------------------------------------------------
void LeaderBoardSelectable::draw( )
{
    Selectable::draw();

    // "Load one of the saved games" or whatever is in Menu.xml file
    // shadow...
    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString(
    _text.c_str(),
    _boundingBox.min.x+9*_size,_boundingBox.min.y-9*_size,
    _size, _size);

    // ... + text
    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontWhite->DrawString(
    _text.c_str(),
    _boundingBox.min.x, _boundingBox.min.y,
    _size, _size);

    //ScoreKeeperS::instance()->draw();
}
//------------------------------------------------------------------------------
ResolutionSelectable::ResolutionSelectable(
    const BoundingBox &r,
    const string &text,
    const string &info):

    TextOnlySelectable(r, text, info, false, 0.7f, 1.0f, 1.0f, 1.0f)
{
    _fontShadow = FontManagerS::instance()->getFont( "bitmaps/menuShadow");

    _checkmark = _icons->getIndex( "Checkmark");
    _checkmarkOff = _icons->getIndex( "CheckmarkOff");

    int currentWidth = 640;
    ConfigS::instance()->getInteger( "width", currentWidth);
    int currentHeight = 480;
    ConfigS::instance()->getInteger( "height", currentHeight);

    _activeResolution = _resolutionList.end();

    string resolutions;
    if( !ConfigS::instance()->getString( "resolutions", resolutions))
    {
    resolutions = "1600x1200,1280x960,1152x864,1024x768,800x600,640x480,512x384";
    Value *r = new Value( resolutions);
    ConfigS::instance()->updateKeyword( "resolutions", r);
    }

    Tokenizer rToken(resolutions, ",");

    int count = 1;
    while( count < 20)
    {
        string nextResolution = rToken.next();
        if( nextResolution == "")
        {
            if( count == 1)
            {
                _resolutionList.insert( _resolutionList.end(), Resolution(0,0));
            }
            if( _activeResolution == _resolutionList.end())
            {
                _activeResolution = --_resolutionList.end();
            }
            break;
        }

        Tokenizer t(nextResolution, "x");
        int width = atoi(t.next().c_str());
        int height = atoi(t.next().c_str());

        _resolutionList.insert( _resolutionList.end(), Resolution(width, height, nextResolution));

        count++;
    }

    addFullscreenResolutions();

    list<Resolution>::iterator walker = _resolutionList.begin();
    for( walker=_resolutionList.begin(); walker!=_resolutionList.end(); walker++)
    {
        if( ((*walker).width == currentWidth) && ((*walker).height == currentHeight))
        {
            _activeResolution = walker;
        }
    }

    float fwidth  = _fontWhite->GetWidth( _text.c_str(), _size);
    float fheight = _fontWhite->GetHeight( _size);

    //    _boundingBox.min.x -= fwidth*0.5;
    _boundingBox.max.x = _boundingBox.min.x + fwidth;
    _boundingBox.max.y = _boundingBox.min.y + fheight;

    _boundingBox.max.x += 165.0f;

    //bounding box for checkmark
    _bRect.min.x = _boundingBox.max.x - _icons->getWidth( _checkmark)*0.5f;
    _bRect.max.x = _boundingBox.max.x;
    _bRect.min.y = _boundingBox.min.y ;
    _bRect.max.y = _bRect.min.y + 30;

    _inputBox = _boundingBox;
}
//------------------------------------------------------------------------------
void ResolutionSelectable::addFullscreenResolutions()
{
    SDL_Rect **modes=SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);

    if( modes == (SDL_Rect **)0)
    {
        return;
    }

    if( modes == (SDL_Rect **)-1)
    {
        return;
    }

    for(int i=0; modes[i]; i++)
    {
        list<Resolution>::iterator walker = _resolutionList.begin();
        char buf[32];
        sprintf( buf, "%dx%d", modes[i]->w, modes[i]->h);
        Resolution newRes( modes[i]->w, modes[i]->h, buf);
        bool add = true;
        while( (walker!=_resolutionList.end()) && ((*walker).width >= modes[i]->w))
        {
            if( (*walker) == newRes)
            {
                //LOG_INFO << "Found dup " << buf << endl;
                add = false;
                break;
            }
            walker++;
        }

        if( add)
        {
            //LOG_INFO << "Adding new " << buf << endl;
            _resolutionList.insert( walker, newRes);
        }
    }
}
//------------------------------------------------------------------------------
void ResolutionSelectable::input( const Trigger &trigger, const bool &isDown)
{
    if( !isDown) return;

    float mouseX = trigger.fData1;
    float mouseY = trigger.fData2;

    switch( trigger.type)
    {
        case eButtonTrigger:
        {
            if( isDown)
            {
                if( (mouseX >= (_bRect.min.x)) && (mouseX <= (_bRect.max.x)) &&
                (mouseY >= _bRect.min.y) && (mouseY <= _bRect.max.y))
                {
                    Resolution none(0,0);
                    if( (*_activeResolution) != none)
                    {
                        int width = (*_activeResolution).width;
                        int height = (*_activeResolution).height;

                        Value *w = new Value( width);
                        ConfigS::instance()->updateKeyword( "width", w);
                        Value *h = new Value( height);
                        ConfigS::instance()->updateKeyword( "height", h);
                    }
                }
                else
                {
                    bool allowSkew = false;
                    ConfigS::instance()->getBoolean( "allowAnyAspectRatio", allowSkew);

                    bool foundNext = false;
                    while( !foundNext)
                    {
                        if( trigger.data1 == SDL_BUTTON_WHEELUP)
                        {
                            if( _activeResolution == _resolutionList.begin())
                                _activeResolution = _resolutionList.end();
                            _activeResolution--;
                        }
                        else
                        {
                            _activeResolution++;
                            if( _activeResolution == _resolutionList.end())
                                _activeResolution = _resolutionList.begin();
                        }

                        int width = (*_activeResolution).width;
                        int height = (*_activeResolution).height;
                        if( allowSkew || ((height*4/3) == width))
                            foundNext = true;
                    }
                }
            }
        }
            break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void ResolutionSelectable::activate( )
{
    if( (_active != this))
    {
        if( _active) _active->deactivate();
        _active = this;
    }
}
//------------------------------------------------------------------------------
void ResolutionSelectable::draw( )
{
//    Selectable::draw();
    TextOnlySelectable::draw();
    string resolution = /*_text +*/ (*_activeResolution).text;

    float xOff = _fontWhite->GetWidth( _text.c_str(), _size);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString(
        resolution.c_str(),
        xOff+_boundingBox.min.x+9*_size,_boundingBox.min.y-9*_size,
        _size, _size);

    glColor4f(1.0f, 0.852f, 0.0f, 1.0f);
    //    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontWhite->DrawString(
        resolution.c_str(),
        xOff+_boundingBox.min.x, _boundingBox.min.y,
        _size, _size);

    _icons->bind();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    float _xOff = _bRect.min.x;

    int currentWidth;
    ConfigS::instance()->getInteger( "width", currentWidth);
    int currentHeight;
    ConfigS::instance()->getInteger( "height", currentHeight);

    int width = (*_activeResolution).width;
    int height = (*_activeResolution).height;

    if( (currentWidth == width) && (currentHeight == height))
        _icons->Draw( _checkmark, _xOff, _boundingBox.min.y+2, 0.5, 0.5);
    else
        _icons->Draw( _checkmarkOff, _xOff, _boundingBox.min.y+2, 0.5, 0.5);
    glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------
KeySelectable::KeySelectable(const BoundingBox &r, const BoundingBox& keybox,
    const string& text, const string& info, const string& bindingName)
:
    TextSelectable(r, text, info, 1.3f)
{
    _awaitingInput = false;
    _bindingName = bindingName;
    _keyBox = keybox;

    // reset some stuff
    _boundingBox.min.x = r.min.x;

    _inputBox = _boundingBox;
    _inputBox.max.x = _inputBox.min.x + 330.f;

    // TODO: _inputBox ...
}
//------------------------------------------------------------------------------
void KeySelectable::input( const Trigger &trigger, const bool &isDown)
{
    if( !isDown)
        return;

    switch( trigger.type)
    {
        case eButtonTrigger:
            select();
            break;

        case eMotionTrigger:
            activate();
            break;

        case eKeyTrigger:
            keypress((SDLKey)trigger.data1);
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void KeySelectable::keypress(SDLKey key)
{
    if (!_awaitingInput)
        return;

    if (key == SDLK_ESCAPE)
    {
        stopWaiting();
        return;
    }

    std::string keyname = InputS::instance()->getKeys().getKeyAsString(key);
    if (keyname != "")        // set a key
    {
        InputS::instance()->unbindKeys(_bindingName);

        string line = "bind " + _bindingName + " " + keyname;
        InputS::instance()->handleLine(line);
        stopWaiting();
    }
}
//------------------------------------------------------------------------------
void KeySelectable::stopWaiting()
{
    MenuManagerS::instance()->_waitSingleKey = 0;
    _awaitingInput = false;
}
//------------------------------------------------------------------------------
void KeySelectable::select()
{
    if (!MenuManagerS::instance()->_waitSingleKey)
    {
        MenuManagerS::instance()->_waitSingleKey = this;
        _awaitingInput = true;
    }
}
//------------------------------------------------------------------------------
void KeySelectable::update( )
{
    _prevSize = _size;
    _size += _ds;
    Clamp( _size, 1.0, _maxSize - 0.2f);
}
//------------------------------------------------------------------------------
void KeySelectable::draw( )
{
    r = 1.0f;
    g = 1.0f;
    b = 1.0f;
    float preSize = _size;
    _size = 0.7f;
    TextOnlySelectable::draw();
    _size = preSize;

    /*
    glColor4f(0.0f, 0.8f, 1.0f, 0.6f);
    glBegin(GL_POLYGON);
        glVertex2f(_inputBox.min.x, _inputBox.min.y);
        glVertex2f(_inputBox.max.x, _inputBox.min.y);
        glVertex2f(_inputBox.max.x, _inputBox.max.y);
        glVertex2f(_inputBox.min.x, _inputBox.max.y);
    glEnd();
    */

    // draw key name
    std::string key = InputS::instance()->getKeyForAction(_bindingName);
    if (_awaitingInput)
        key = "PRESS KEY";
    float iSize = _prevSize + (_size - _prevSize) * GameState::frameFractionOther;
    Clamp( iSize, 1.0, _maxSize);

    float halfWidth = _fontWhite->GetWidth( key.c_str(), iSize-1.0f)/2.0f;
    float halfHeight = _fontWhite->GetHeight( iSize-1.0f)/2.0f;

    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString( key.c_str(),
        _keyBox.min.x - halfWidth  + 5*iSize,
        _keyBox.min.y - halfHeight - 5*iSize - 6.0f,
        iSize, iSize);
    glColor4f(1.0f, 0.852f, 0.0f, 0.8f);
    _fontWhite->DrawString( key.c_str(),
        _keyBox.min.x - halfWidth,
        _keyBox.min.y - halfHeight  - 6.0f,
        iSize, iSize);
}
//------------------------------------------------------------------------------
TextSelectable::TextSelectable(const BoundingBox &r, const string &text,
    const string &info, float(maxSize))
:
    TextOnlySelectable(r, text, info), _ds(0.0), _maxSize(maxSize)
{
    _prevSize = _size;
}
//------------------------------------------------------------------------------
void TextSelectable::input( const Trigger &trigger, const bool &isDown)
{
    if( !isDown)
        return;

    switch( trigger.type)
    {
        case eButtonTrigger:
            select();
            break;

        case eMotionTrigger:
            // this->activate();
            activate();
            break;

        default:
            break;
    }
}
//------------------------------------------------------------------------------
void TextSelectable::activate( )
{
    if( (_active != this))
    {
        if( _active)
            _active->deactivate();
        _active = this;
        _ds = 0.1f;
        AudioS::instance()->playSample( "sounds/beep.wav");
    }
}
//------------------------------------------------------------------------------
void TextSelectable::deactivate( )
{
    //  LOG_INFO << "Deactivate " << _text << endl;
    _ds = -0.1f;
}
//------------------------------------------------------------------------------
void TextSelectable::update( )
{
    _prevSize = _size;
    _size += _ds;
    Clamp( _size, 1.0, _maxSize - 0.2f); //any bigger and we'll have overlapping activation areas

    //adjust the input box according to the scaled text
    float dx = (float)(_boundingBox.max.x - _boundingBox.min.x) * (_size-1.0f) / 2.0f;
    float dy = (float)(_boundingBox.max.y - _boundingBox.min.y) * (_size-1.0f) / 2.0f;

    _inputBox.min.x = _boundingBox.min.x - dx;
    _inputBox.min.y = _boundingBox.min.y - dy;
    _inputBox.max.x = _boundingBox.max.x + dx;
    _inputBox.max.y = _boundingBox.max.y + dy;
}
//------------------------------------------------------------------------------
void TextSelectable::draw( )
{
    Selectable::draw();

    float iSize = _prevSize + (_size - _prevSize) * GameState::frameFractionOther;
    Clamp( iSize, 1.0, _maxSize);

    float halfWidth = _fontWhite->GetWidth( _text.c_str(), iSize-1.0f)/2.0f;
    float halfHeight = _fontWhite->GetHeight( iSize-1.0f)/2.0f;

    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString( _text.c_str(),
        _boundingBox.min.x - halfWidth  + 5*iSize,
        _boundingBox.min.y - halfHeight - 5*iSize,
        iSize, iSize);
    glColor4f(r, g, b, 0.8f);
    _fontWhite->DrawString( _text.c_str(),
        _boundingBox.min.x - halfWidth,
        _boundingBox.min.y - halfHeight,
        iSize, iSize);
}
//------------------------------------------------------------------------------
ActionSelectable::ActionSelectable(const BoundingBox &r, const string &action,
    const string &text, const string &info, float maxSize)
:
    TextSelectable(r, text, info, maxSize), _action(action)
{
}
//------------------------------------------------------------------------------
void ActionSelectable::select( )
{
    LOG_INFO << "Selecting: " << _action << endl;
    if (_action == "NewCampaign")
        GameS::instance()->startNewCampaign();
    else if (_action == "NewGame")
    {
        if (GameS::instance()->_landed)
        {
            if (GameS::instance()->illegalTradeCheck())
                GameS::instance()->startNewGame();
        }
        else
            GameS::instance()->switchContext(eInGame);
        return;
    }
    else if (_action == "ResetKeys")
    {
        InputS::instance()->defaultKeys();
    }
    else if (_action == "LoadGame")
    {
        MessageBoxManagerS::instance()->file(false);
        GameS::instance()->switchContext(eMessageBox);
    }
    else if (_action == "SaveGame")
    {
        MessageBoxManagerS::instance()->file(true);
    }
    else if (_action == "LoadGameSlot")
    {
        int slot = MessageBoxManagerS::instance()->getSlot();
        if (!GameS::instance()->loadGame(slot))
        {
            MessageBoxManagerS::instance()->setup("LOAD GAME",
                "LOADING FAILED", "OK", "MainMenu");
        }
    }
    else if (_action == "SaveGameSlot")
    {
        int slot = MessageBoxManagerS::instance()->getSlot();
        std::string sname = MessageBoxManagerS::instance()->getSlotName();
        if (sname == "")
            sname = "SAVED GAME";
        std::string msg = "GAME SAVED SUCCESSFULLY\n\n"
            "You can load it from the main menu.\n\n";
        if (!GameS::instance()->saveGame(sname, slot))
        {
            msg = "SAVE FAILED\n\nPerhaps you don't have "
                "enough disk space or you can't write to game's directory.\n\n"
                "Try saving to a different save slot.";
        }
        MessageBoxManagerS::instance()->setup("SAVE GAME", msg,
            "OK", "PlanetMenu");
    }
    else if (_action == "IllegalTradeAccept")
    {
        GameS::instance()->illegalTradeDecision(true);
        GameS::instance()->startNewGame();
    }
    else if (_action == "IllegalTradeRefuse")
    {
        GameS::instance()->illegalTradeDecision(false);
        GameS::instance()->startNewGame();
    }
    else if (_action == "PlanetMenu")
    {
        GameS::instance()->switchContext(ePlanetMenu);
        // don't allow to go back to message box:
        GameS::instance()->setPreviousContext(ePlanetMenu);
    }
    else if (_action == "UlegAccept")
        GameS::instance()->ulegAccept();
    else if (_action == "UlegRefuse")
        GameS::instance()->ulegRefuse();
    else if (_action == "Quit")
       GameState::isAlive = false;
    else if (_action == "ShowMap")
        PlanetManagerS::instance()->setActiveScreen(PlanetManager::stMap);
    else if (_action == "ShowCargo")
        PlanetManagerS::instance()->setActiveScreen(PlanetManager::stTrade);
    else if (_action == "ShowQuests")
        PlanetManagerS::instance()->setActiveScreen(PlanetManager::stQuests);
    else if (_action == "MainMenu")
    {
        GameS::instance()->switchContext(eMenu);
        while (MenuManagerS::instance()->exitMenu(false))   // don't exit game
            ;                                               // but go to top
    }
    else if (_action == "KeysSetupDone")
    {
        MenuManagerS::instance()->exitMenu(false);
    }
    else
        LOG_ERROR << "Unknown action" << _action << endl;
    AudioS::instance()->playSample( "sounds/confirm.wav");
}
//------------------------------------------------------------------------------
MenuSelectable::MenuSelectable(TiXmlNode *node, const BoundingBox &r,
    const string &text, const string &info)
    :TextSelectable(r, text, info), _node(node)
{
}
//------------------------------------------------------------------------------
void MenuSelectable::select( )
{
    MenuManagerS::instance()->makeMenu( _node);
    AudioS::instance()->playSample( "sounds/confirm.wav");
}
//------------------------------------------------------------------------------

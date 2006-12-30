// Description:
//   Different kinds of selectables.
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
#ifndef _Selectable_hpp_
#define _Selectable_hpp_

#include <string>
#include <list>

#include <Point.hpp>
#include <Trigger.hpp>
#include <GLBitmapFont.hpp>
#include <GLBitmapCollection.hpp>

#include <tinyxml.h>
//------------------------------------------------------------------------------
class Selectable
{
public:
    Point2D _infoLocation;

    Selectable( const BoundingBox &r, const string &info);
    virtual ~Selectable();

    virtual void input( const Trigger &/*trigger*/, const bool &/*isDown*/)
    {
    }
    virtual void activate( )
    {
    }
    virtual void deactivate( )
    {
    }
    virtual void select( )
    {
    }
    virtual void update( )
    {
    }

    virtual void draw( );

    const BoundingBox &getInputBox( )
    {
        return _inputBox;
    }

    static void reset()
    {
        if (_active)
            _active->deactivate();
        _active = 0;
    }

    bool isActive()
    {
        return _active == this;
    }

protected:
    static Selectable *_active;

    BoundingBox _inputBox;
    BoundingBox _boundingBox;
    string _info;
    GLBitmapFont *_fontWhite;
};
//------------------------------------------------------------------------------
class EscapeSelectable:public Selectable
{
public:
    EscapeSelectable( const BoundingBox &r, float size=1.0);

    virtual void input( const Trigger &trigger, const bool &isDown);
    virtual void activate( );
    virtual void draw( );
protected:
    float _size;
    GLBitmapCollection *_icons;
    int _exitOn;
    int _exitOff;
};
//------------------------------------------------------------------------------
class LeaderBoardSelectable:public Selectable
{
public:
    LeaderBoardSelectable(const BoundingBox &r, const string &text,
        const string &info);

    virtual void input( const Trigger &trigger, const bool &/*isDown*/);
    virtual void draw( );
    void activate( );
protected:
    string _text;
    GLBitmapFont *_fontShadow;
    float _size;
};
//------------------------------------------------------------------------------
class TextOnlySelectable:public Selectable
{
public:
    TextOnlySelectable(const BoundingBox &r, const string &text,
        const string &info, bool center=true, float size=1.0,
        float red=1.0, float green=0.852, float blue=0.0);

    virtual void input( const Trigger &trigger, const bool &/*isDown*/);
    virtual void activate( );
    virtual void draw( );
protected:
    string _text;
    GLBitmapFont *_fontShadow;
    GLBitmapCollection *_icons;
    float r;
    float g;
    float b;
    float _size;
};
//------------------------------------------------------------------------------
class ResolutionSelectable:public TextOnlySelectable
{
    struct Resolution
    {
        Resolution( int w, int h):width(w), height(h) {}
        Resolution( int w, int h, string t):width(w), height(h), text(t) {}
        int width;
        int height;
        string text;
        bool operator==(Resolution &r1)
        {
            return (r1.width=width) && (r1.height==height);
        }
        bool operator!=(Resolution &r1)
        {
            return ! operator==(r1);
        }
    };

public:
    ResolutionSelectable(const BoundingBox &r,
        const string &text, const string &info);

    virtual void input( const Trigger &trigger, const bool &/*isDown*/);
    virtual void draw( );
    void activate( );
protected:
    void addFullscreenResolutions();

    GLBitmapFont *_fontShadow;
    BoundingBox _bRect;
    int _checkmark;
    int _checkmarkOff;
    list<Resolution> _resolutionList;
    list<Resolution>::iterator _activeResolution;
};
//------------------------------------------------------------------------------
class FloatSelectable:public TextOnlySelectable
{
public:
    FloatSelectable(const BoundingBox &r, const string &text,
        const string &info,  const string &variable,
        const string &range, const string &sliderOffset);

    virtual void input( const Trigger &trigger, const bool &isDown);
    virtual void draw( );
protected:
    BoundingBox _bRect;
    string _variable;
    float _max;
    float _min;
    float _startX;
    float _curVal;
    float _xPos;
    int _slider;
    int _doubleArrow;
    int _sliderOffset;
};
//------------------------------------------------------------------------------
class EnumSelectable:public TextOnlySelectable
{
public:
    EnumSelectable(const BoundingBox &r, const string &text,
        const string &info, const string &variable, const string &values);

    virtual void input( const Trigger &trigger, const bool &isDown);
    virtual void draw( );
protected:
    string _variable;
    float _xOff;
    list<string> _enumList;
    list<string>::iterator _activeEnum;
};
//------------------------------------------------------------------------------
class BoolSelectable:public TextOnlySelectable
{
public:
    BoolSelectable(const BoundingBox &r, const string &text,
        const string &info, const string &variable);

    virtual void input( const Trigger &trigger, const bool &isDown);
    virtual void draw( );
protected:
    string _variable;
    float _xOff;
    int _checkmark;
    int _checkmarkOff;
};
//------------------------------------------------------------------------------
class TextSelectable: public TextOnlySelectable
{
public:
    TextSelectable(const BoundingBox &r, const string &text,
        const string &info, float _maxSize = 2.0f);

    virtual void input( const Trigger &trigger, const bool &isDown);
    virtual void activate( );
    virtual void deactivate( );
    virtual void update( );
    virtual void draw( );
protected:
    float _ds;
    float _prevSize;
    float _maxSize;
};
//------------------------------------------------------------------------------
class ActionSelectable: public TextSelectable
{
public:
    ActionSelectable(const BoundingBox &r, const string &action,
        const string &text, const string &info, float _maxSize = 2.0f);

    virtual void select( );
protected:
    string _action;
};
//------------------------------------------------------------------------------
class MenuSelectable: public TextSelectable
{
public:
    MenuSelectable(TiXmlNode *node, const BoundingBox &r,
        const string &text, const string &info);

    virtual void select( );
protected:
    TiXmlNode *_node;
};
//------------------------------------------------------------------------------
class KeySelectable: public TextSelectable
{
public:
    KeySelectable(const BoundingBox &r, const BoundingBox& kb,
        const std::string& text,
        const std::string& info, const std::string& bindingName);

    virtual void input( const Trigger &trigger, const bool &isDown);
    virtual void update( );
    virtual void draw();
    virtual void select();
protected:
    BoundingBox _keyBox;
    string _bindingName;
    bool _awaitingInput;

    void keypress(SDLKey key);
    void stopWaiting();
};
//------------------------------------------------------------------------------
#endif

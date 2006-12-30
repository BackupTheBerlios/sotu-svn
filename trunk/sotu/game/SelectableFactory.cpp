// Description:
//   Different kinds of selectable factories.
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
#include <SelectableFactory.hpp>
//----------------------------------------------------------------------------
hash_map<
    const string,
    SelectableFactory*,
    hash<const string>, equal_to<const string> > SelectableFactory::_sfMap;

bool SelectableFactory::_initialized = false;
//----------------------------------------------------------------------------
SelectableFactory *SelectableFactory::getFactory( const string &name)
{
    XTRACE();
    if( ! _initialized)
    {
        _initialized = true;

        new ActionItemFactory();
        new MenuItemFactory();
        new TextItemFactory();
        new BoolFactory();
        new EnumFactory();
        new FloatFactory();
        new LeaderBoardFactory();
        new ResolutionFactory();
        new KeyboardItemFactory();
    }
    return findHash( name, _sfMap);
}
//----------------------------------------------------------------------------
void SelectableFactory::cleanup( void)
{
    hash_map< const string,
              SelectableFactory*,
              hash<const string> >::const_iterator ci;
    for( ci=_sfMap.begin(); ci!=_sfMap.end(); ci++)
    {
        SelectableFactory *sf = ci->second;
        delete sf;
    }
    _sfMap.clear();

    _initialized = false;
}
//----------------------------------------------------------------------------
void SelectableFactory::posToPoint2D( const string &pos, Point2D &point)
{
    if( pos == "")
    {
        point.x = 100;
        point.y = 100;
        return;
    }

    Tokenizer t(pos);
    point.x = (float)atof( t.next().c_str());
    point.y = (float)atof( t.next().c_str());
}
//----------------------------------------------------------------------------
string SelectableFactory::getAttribute( const TiXmlElement* elem, string attr)
{
    const string *attrVal = elem->Attribute( attr);
    if( attrVal)
    {
        return *attrVal;
    }

    return( string(""));
}
//----------------------------------------------------------------------------
void SelectableFactory::getBasics(
    TiXmlElement* elem,
    Point2D &pos,
    string &text,
    string &info)
{
    posToPoint2D( getAttribute( elem, "Position"), pos);
    text = getAttribute( elem, "Text");
    info = getAttribute( elem, "Info");
}
//------------------------------------------------------------------------------
ActionItemFactory::ActionItemFactory( void)
{
    _sfMap[ "ActionItem"] = this;
}
//----------------------------------------------------------------------------
ActionItemFactory::~ActionItemFactory()
{
}
//----------------------------------------------------------------------------
Selectable *ActionItemFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);
    string action = getAttribute( elem, "Action");
    return new ActionSelectable( r, action, text, info);
}
//------------------------------------------------------------------------------
KeyboardItemFactory::KeyboardItemFactory(void)
{
    _sfMap["KeyboardItem"] = this;
}
//------------------------------------------------------------------------------
KeyboardItemFactory::~KeyboardItemFactory()
{
}
//------------------------------------------------------------------------------
Selectable *KeyboardItemFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    string bindingName = getAttribute(elem, "BindingName");
    BoundingBox keyBox;
    posToPoint2D( getAttribute(elem, "KeyPosition"), keyBox.min);

    return new KeySelectable(r, keyBox, text, info, bindingName);
}
//------------------------------------------------------------------------------

MenuItemFactory::MenuItemFactory( void)
{
    _sfMap[ "Menu"] = this;
}
//----------------------------------------------------------------------------
MenuItemFactory::~MenuItemFactory()
{
}
//----------------------------------------------------------------------------
Selectable *MenuItemFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);
    return new MenuSelectable( node, r, text, info);
}
//------------------------------------------------------------------------------

TextItemFactory::TextItemFactory( void)
{
    _sfMap[ "TextItem"] = this;
}

TextItemFactory::~TextItemFactory()
{
}

Selectable *TextItemFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    string size   = getAttribute( elem, "Size");
    float fSize = 0.65f;
    if( size != "")
    {
        fSize = (float)atof( size.c_str());
    }

    return new TextOnlySelectable( r, text, info, true, fSize, 1.0,1.0,1.0);
}

//------------------------------------------------------------------------------

BoolFactory::BoolFactory( void)
{
    _sfMap[ "Bool"] = this;
}

BoolFactory::~BoolFactory()
{
}

Selectable *BoolFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    string var    = getAttribute( elem, "Variable");

    return new BoolSelectable( r, text, info, var);
}

//------------------------------------------------------------------------------

EnumFactory::EnumFactory( void)
{
    _sfMap[ "Enum"] = this;
}

EnumFactory::~EnumFactory()
{
}

Selectable *EnumFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    string var    = getAttribute( elem, "Variable");
    string values = getAttribute( elem, "Values");

    return new EnumSelectable( r, text, info, var, values);
}

//------------------------------------------------------------------------------

FloatFactory::FloatFactory( void)
{
    _sfMap[ "Float"] = this;
}

FloatFactory::~FloatFactory()
{
}

Selectable *FloatFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    string var    = getAttribute( elem, "Variable");
    string range  = getAttribute( elem, "Range");
    string sliderOffset = getAttribute( elem, "SliderOffset");

    return new FloatSelectable( r, text, info, var, range, sliderOffset);
}

//------------------------------------------------------------------------------

LeaderBoardFactory::LeaderBoardFactory( void)
{
    _sfMap[ "LeaderBoard"] = this;
}

LeaderBoardFactory::~LeaderBoardFactory()
{
}

Selectable *LeaderBoardFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    return new LeaderBoardSelectable( r, text, info);
}

//------------------------------------------------------------------------------

ResolutionFactory::ResolutionFactory( void)
{
    _sfMap[ "Resolution"] = this;
}

ResolutionFactory::~ResolutionFactory()
{
}

Selectable *ResolutionFactory::createSelectable( TiXmlNode *node)
{
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    getBasics( elem, r.min, text, info);

    return new ResolutionSelectable( r, text, info);
}

//------------------------------------------------------------------------------

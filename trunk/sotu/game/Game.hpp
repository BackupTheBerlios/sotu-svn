// Description:
//   High level infrastructure for game.
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
#ifndef _Game_hpp_
#define _Game_hpp_

#include <Context.hpp>
#include <Singleton.hpp>
#include <map>
#include <vector>
//----------------------------------------------------------------------------
class CargoItemInfo
{
public:
    typedef enum { pmNormal, pmProTech, pmContraTech } PriceModel;
    typedef enum { lsLegal, lsiEmpire, lsiRebels, lsiBoth } LegalStatus;

    CargoItemInfo(const std::string& gName, const std::string& name,
        int tl, int price, int weight = 1, int maxQty = 0,
        PriceModel pm = pmNormal, LegalStatus ls = lsLegal)
        :_name(name), _groupName(gName), _techLevelRequired(tl),
         _priceModel(pm), _basePrice(price), _legalStatus(ls),
         _weight(weight), _maxQty(maxQty)
    {
    };

    std::string _name;
    std::string _groupName;
    int _techLevelRequired; // ako je -1 onda to ne moze da se kupi N/A
    PriceModel _priceModel;
    int _basePrice;
    LegalStatus _legalStatus;
    int _weight; // equipment doesn't take cargo space so it's zero
    int _maxQty; // 0 = no limit

    static std::vector<CargoItemInfo>* getCargoInfo();
};
//----------------------------------------------------------------------------
class CargoItem
{
public:
    CargoItem(std::string name, int quantity = 0, int price = 0)
        :_name(name), _quantity(quantity), _price(price)
    {
    };
    CargoItem(const CargoItem& other)
    {
        _name = other._name;
        _quantity = other._quantity;
        _price = other._price;
    }
    std::string _name;
    int _quantity;
    int _price;
};
//----------------------------------------------------------------------------
class Planet;
class Cargo
{
private:
    std::vector<CargoItem> _items;
public:
    void clear();
    void create(Planet *p); // give zero to create player's cargo
    CargoItem* findItem(const std::string& itemName);
};
//----------------------------------------------------------------------------
class Planet
{
//private:
public:
    float _radius;          // 45 - 80
    int _textureIndex;
    Cargo _marketplace;
    float _techLevel;         // 1 - 9
    int _rebelSentiment;    // 0 - 100
    int _alienActivity;     // 0 - 100
    std::string _name;
    float _x;
    float _y;

public:
    Planet(float x, float y, const std::string& name = "");
    bool isAt(float x, float y);                // allow few pixels miss
    float distance(float x, float y);

    float getPrice(const std::string& itemName);
    void update();
};
//----------------------------------------------------------------------------
class Map
{
public:
    Map();  // creates galaxy of planets
    ~Map(); // destroy planets
    typedef std::vector<Planet *> PlanetList;
    PlanetList _planets;

    // x, y offset
    void draw(float x, float y);                // renders galaxy as set of points
    Planet* getPlanetAt(float x, float y);
    Planet* getNearest(float x, float y);

private:
    Planet* _currentPlanet;
};
//----------------------------------------------------------------------------
typedef enum { eUnknown, eMenu, eInGame, ePlanetMenu, ePaused, eCameraFlyby,
        eLAST } ContextEnum;
//----------------------------------------------------------------------------
class Game
{
friend class Singleton<Game>;
public:
    Map _galaxy;       // Galaxy
    Cargo _cargo;       // Player's cargo
    int _money;

    bool init( void);
    void run( void);
    void reset( void);
    void startNewGame( void);

    void startNewCampaign();
    ContextEnum getContext();
    void switchContext(ContextEnum c);
    void previousContext();

private:
    ~Game();
    Game( void);
    Game( const Game&);
    Game &operator=(const Game&);

    void updateOtherLogic( void);
    void updateInGameLogic( void);
    ContextEnum _context;
    ContextEnum _previousContext;
};
typedef Singleton<Game> GameS;
//----------------------------------------------------------------------------
#endif

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
#include <iostream>
//----------------------------------------------------------------------------
class CargoItemInfo
{
public:
    typedef enum { pmNormal=0, pmProTech, pmContraTech, pmRandom } PriceModel;
    typedef enum { lsLegal=0, lsiEmpire, lsiRebels, lsiBoth } LegalStatus;

    CargoItemInfo(float scale, const std::string& mName, const std::string& name,
        int tl, int price, int weight = 1, int maxQty = 0,
        PriceModel pm = pmNormal, const std::string& info = "",
        LegalStatus ls = lsLegal)
        :_scale(scale), _name(name), _modelName(mName), _info(info),
         _techLevelRequired(tl), _priceModel(pm), _basePrice(price),
         _legalStatus(ls), _weight(weight), _maxQty(maxQty)
    {
    };

    float _scale;
    std::string _name;
    std::string _modelName;
    std::string _info;
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
    };
    CargoItem()     // just from loading
    {
    };

    void save(std::ofstream& os);
    bool load(std::ifstream& is);
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
    typedef std::vector<CargoItem>::iterator iterator;
    iterator begin();
    iterator end();

    void save(std::ofstream& os);
    void load(std::ifstream& is);
    void clear();
    void create(Planet *p); // give zero to create player's cargo
    CargoItem* findItem(const std::string& itemName);
};
//----------------------------------------------------------------------------
class Planet
{
private:
    Cargo _marketplace;
public:
    float _radius;          // 45 - 80
    int _textureIndex;
    float _techLevel;         // 1 - 9
    int _rebelSentiment;    // 0 - 100
    int _alienActivity;     // 0 - 100
    std::string _name;
    float _x;
    float _y;

    Planet();
    Planet(float x, float y, const std::string& name = "");
    bool isAt(float x, float y);                // allow few pixels miss
    float getDistance(float x, float y);

    typedef enum { bsNA=0, bsNoTech, bsNoMoney, bsMAX, bsOk, bsCargoFull } BuyStatus;
    BuyStatus canBuy(CargoItemInfo& item);
    int getPrice(const std::string& itemName);

    bool isSpecial();
    void update();
    void save(std::ofstream& os);
    bool load(std::ifstream& is);
};
//----------------------------------------------------------------------------
class Map
{
public:
    ~Map();             // destroy planets
    void recreate();    // creates planets

    void draw(float x, float y);                // renders galaxy as set of points
    Planet* getPlanetAt(float x, float y);
    Planet* getNearest(float x, float y);
    void addPlanet(Planet *p);
    void update();

    void save(std::ofstream& os);
    void load(std::ifstream& is);

private:
    void deletePlanets();
    typedef std::vector<Planet *> PlanetList;
    PlanetList _planets;
};
//----------------------------------------------------------------------------
typedef enum { eUnknown, eMenu, eInGame, ePlanetMenu, eMessageBox, ePaused,
    eCameraFlyby, eLAST } ContextEnum;
typedef enum { psClean=0, psFugitive, psTerrorist } PlayerStatus;
//----------------------------------------------------------------------------
class Game
{
friend class Singleton<Game>;
public:
    Map _galaxy;            // Galaxy
    Planet* _currentPlanet;
    Cargo _cargo;           // Player's cargo
    int _money;             // and money
    PlayerStatus _rebelStatus;       // clean, fugitive, terrorist
    PlayerStatus _empireStatus;
    int _kills;
    bool _landed;
    int _chapter;
    std::string getReputation(bool upcase = false);

    std::vector<std::string> _questTargets;
    void reachedSpecialPlanet();
    void ulegAccept();
    void ulegRefuse();

    bool illegalTradeCheck();
    void illegalTradeDecision(bool accept);

    float _spaceStationApproach;
    float _hyperspaceCount;
    void hyperspaceJump();
    std::string getHyperspaceAvailable();   // returns "OK" is o.k.

    bool init( void);
    void run( void);
    void reset();
    void startNewGame();

    bool saveGame(const std::string& savename, int slot);
    bool loadGame(int slot);

    void startNewCampaign();
    ContextEnum getContext();
    void switchContext(ContextEnum c);
    void setPreviousContext(ContextEnum c);
    void previousContext();

private:
    ~Game();
    Game( void);
    Game( const Game&);
    Game &operator=(const Game&);

    void updateOtherLogic();
    void updateInGameLogic();
    ContextEnum _context;
    ContextEnum _previousContext;
};
typedef Singleton<Game> GameS;
//----------------------------------------------------------------------------
#endif

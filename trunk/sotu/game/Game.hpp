// Description:
//   High level infrastructure for game.
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
#ifndef _Game_hpp_
#define _Game_hpp_

#include <Singleton.hpp>
#include <map>
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
class Cargo
{
public:
    // Group, CargoItem(name, qty)
    std::multimap<std::string, CargoItem> _items;

    void addItem(std::string, const CargoItem&);
    void clearCargo();
};
//----------------------------------------------------------------------------
class Game
{
friend class Singleton<Game>;
public:
    bool init( void);
    void run( void);
    void reset( void);
    void startNewGame( void);

    void startNewCampaign();

private:
    ~Game();
    Game( void);
    Game( const Game&);
    Game &operator=(const Game&);

    void updateOtherLogic( void);
    void updateInGameLogic( void);

    Cargo _cargo;
    // Planet *_currentPlanet;
};
typedef Singleton<Game> GameS;
//----------------------------------------------------------------------------
#endif

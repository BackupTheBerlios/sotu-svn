// Description:
//   Our Hero!
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
#ifndef _Hero_hpp_
#define _Hero_hpp_

#include <Singleton.hpp>
#include <Model.hpp>
#include <Direction.hpp>
#include <ParticleType.hpp>
#include <Skill.hpp>

class Weapon;

class Hero: public ParticleType
{
friend class Singleton<Hero>;
public:
    enum MountedWeapon
    {
       PRIMARY_WEAPON,
       SECONDARY_WEAPON,
       TERTIARY_WEAPON,
       MAX_WEAPONS
    };

    bool init( void);
    void reset( void);


    virtual void init( ParticleInfo *p);
    virtual bool update( ParticleInfo *p);
    virtual void hit( ParticleInfo *p, int damage, int /*radIndex*/);
    virtual void draw( ParticleInfo *){ /* not used, see other draw */; }
    virtual void draw( void);
    void drawWeapon( unsigned int weapNum);

    void popMessage(const char *msg, float red, float green, float blue);

    void move( float dx, float dy);
    void move( Direction::DirectionEnum d, bool isDown);
    void allowVerticalMovement(bool allow);

    bool weaponLoaded( int weapNum);
    void weaponFire( bool isDown, int weapNum=0);

    bool alive( void)
    {
        return _isAlive;
    }

    void alive( bool a)
    {
        _isAlive = a;
    }

    void addEnergy( int val);
    void addShield( int val);

    float getEnergy( void)
    {
        return (float)_energy;
    }

    float getShieldEnergy( void)
    {
        return (float)_shieldEnergy;
    }

    float getWeaponEnergy( void)
    {
        return _weaponEnergy;
    }

    void setArmorPierce( float damageMultiplier);
    float getArmorPierce( void)
    {
        return _damageMultiplier;
    }

    void upgradeWeapons( void);

    void assignWeapons();

    float lastXPos;
    float lastYPos;

private:
    virtual ~Hero();
    Hero( void);

    Hero( const Hero&);
    Hero &operator=(const Hero&);

    void spawnSparks( int spawnCount, float r, ParticleInfo &pi);

    Model *_model;
    ParticleInfo *pInfo;

    float _moveLeft;
    float _moveRight;
    float _moveUp;
    float _moveDown;
    float _maxY;
    bool _isAlive;

    int _energy;
    int _shieldEnergy;
    float _weaponEnergy;

    float _weaponReload[ MAX_WEAPONS];
    bool _weaponAutofire[ MAX_WEAPONS];
    Weapon *_weapon[ MAX_WEAPONS];
    int _weaponAmmo[ MAX_WEAPONS];
    float _damageMultiplier;
    bool _autofireOn;

    float _sint[360];
    float _cost[360];
};

typedef Singleton<Hero> HeroS;

#endif

// Description:
//   All kinds of Weapons.
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
#ifndef _Weapon_hpp_
#define _Weapon_hpp_

#include <string>
class Model;

const string WeaponWINGPHASER="WingPhaser";
const string WeaponICESPRAY="IceSpray";
const string WeaponFLANKBURST="FlankBurst";
const string WeaponSTINGER="HeroStinger";
const string WeaponDOG="GuardDog";

class Weapon
{
public:
    Weapon( float energyRequired, float reloadTime, string name, string desc);
    virtual ~Weapon();

    float energyRequired( void);
    float reloadTime( void);
    virtual void launch( float x, float y, float z) = 0;

    void setDamageMultiplier( float damageMultiplier);

    const string& name( void){ return _name;}
    const string& description( void){ return _description;}

    virtual void draw( void) {}

protected:
    float _energyRequired;
    float _reloadTime;
    float _damageMultiplier;
    const string _name;
    const string _description;
};

class GuardDog: public Weapon
{
public:
    GuardDog( void);
    virtual ~GuardDog();

    virtual void launch( float x, float y, float z);
};

class Stinger: public Weapon
{
public:
    Stinger( void);
    virtual ~Stinger();

    virtual void launch( float x, float y, float z);
    virtual void draw( void);
private:
    Model *_stinger;
};

class FlankBurster: public Weapon
{
public:
    FlankBurster( void);
    virtual ~FlankBurster();

    virtual void launch( float x, float y, float z);
    virtual void draw( void);
private:
    Model *_flankBurster;
};

class IceSpray: public Weapon
{
public:
    IceSpray( void);
    virtual ~IceSpray();

    virtual void launch( float x, float y, float z);
    virtual void draw( void);
private:
    Model *_iceSpray;
    Model *_iceSprayPierce;
};

class WingPhaser: public Weapon
{
public:
    WingPhaser( void);
    virtual ~WingPhaser();

    virtual void launch( float x, float y, float z);
    virtual void draw( void);
private:
    Model *_wingPhaser;
    Model *_wingPhaserPierce;
};

#endif

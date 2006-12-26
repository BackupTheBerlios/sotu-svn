// Description:
//   Our Hero!
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
#include <Hero.hpp>
#include <ParticleGroup.hpp>
#include <ParticleGroupManager.hpp>
#include <WeaponDepot.hpp>
#include <Weapon.hpp>
#include <Timer.hpp>
#include <Trace.hpp>
#include <GameState.hpp>
#include <ModelManager.hpp>
#include <Audio.hpp>
#include <Point.hpp>
#include <ScoreKeeper.hpp>
#include <Config.hpp>
#include <Game.hpp>
#include <Input.hpp>
#include <Random.hpp>

#include <BitmapManager.hpp>

#include <gl++.hpp>

const float MAX_X= 63; //(int)(47.5*4/3);
const float MIN_X=-63; //-(int)(47.5*4/3);
const float MAX_Y= 45;
const float MIN_Y=-45;
//----------------------------------------------------------------------------
Hero::Hero():
    ParticleType( "Hero"),
    _pInfo(0),
    _maxY(MIN_Y)
{
    XTRACE();
    for( int i=0; i<360; i++)
    {
        _sint[i] = sin( i * ((float)M_PI/180.0f));
        _cost[i] = cos( i * ((float)M_PI/180.0f));
    }
    reset();
}
//----------------------------------------------------------------------------
void Hero::reset(bool leaveEnergy)
{
    XTRACE();
    _isAlive = true;
    _moveLeft=0;
    _moveRight=0;
    _moveUp=0;
    _moveDown=0;
    if (!leaveEnergy)
    {
        _energy=100;
        _shieldEnergy=100;
    }
    _weaponEnergy=100.0;
    _damageMultiplier = 1.0;

    for( int i=0; i<Hero::MAX_WEAPONS; i++)
    {
        _weapon[ i] = 0;
        _weaponAutofire[ i] = false;
        _weaponReload[ i] = 0.0;
#define UNLIMITED_AMMO -1
        _weaponAmmo[ i] = UNLIMITED_AMMO;
    }
}
//----------------------------------------------------------------------------
Hero::~Hero()
{
    XTRACE();
}
//----------------------------------------------------------------------------
void Hero::init(ParticleInfo *p)
{
    XTRACE();
    _pInfo = p;

    //override Y
    p->position.y = MIN_Y;

    // set some reasonable damage when player collides with aliens
    p->damage = 30;
    p->tod = -1;    // prevents reseting the damage

    vec3 minpt, maxpt;
    _model->getBoundingBox( minpt, maxpt);
    p->radius = (maxpt.x-minpt.x)/2.0;

    updatePrevs(p);
}
//----------------------------------------------------------------------------
void Hero::assignWeapons()
{
    WeaponDepot *wDepot = WeaponDepotS::instance();
    _weapon[ Hero::PRIMARY_WEAPON] = wDepot->getWeapon(WeaponWINGPHASER);
    Cargo &c = GameS::instance()->_cargo;
    CargoItem *spread = c.findItem("Proton spread fire");
    if (spread->_quantity > 0)
        _weapon[ Hero::TERTIARY_WEAPON] = wDepot->getWeapon(WeaponICESPRAY);
    else
        _weapon[ Hero::TERTIARY_WEAPON] = wDepot->getWeapon(WeaponDOG);

    CargoItem *sideways = c.findItem("Wave emitter");
    if (sideways->_quantity > 0)
        _weapon[ Hero::SECONDARY_WEAPON] = wDepot->getWeapon(WeaponFLANKBURST);
    else
        _weapon[ Hero::SECONDARY_WEAPON] = wDepot->getWeapon(WeaponDOG);

    CargoItem *enh = c.findItem("Proton enhancer");
    if (enh->_quantity > 0)
        setArmorPierce(2.0f);
    else
        setArmorPierce(1.0f);
}
//----------------------------------------------------------------------------
void Hero::addEnergy(int val)
{
    _energy += val;
    if( _energy > 100)
        _energy = 100;
}
//----------------------------------------------------------------------------
void Hero::addShield(int val)
{
    _shieldEnergy += val;
    Cargo& c = GameS::instance()->_cargo;
    CargoItem *item = c.findItem("Shield upgrade");
    int upgrades = (item ? item->_quantity : 0);    // safeguard
    int maxval = 100 + 100 * upgrades;
    if( _shieldEnergy > maxval)
        _shieldEnergy = maxval;
}
//----------------------------------------------------------------------------
void Hero::popMessage(const char *msg, float red, float green, float blue)
{
    if( GameState::horsePower > 90.0)
    {
        static ParticleGroup *effects =
            ParticleGroupManagerS::instance()->getParticleGroup(EFFECTS_GROUP2);
        ParticleInfo pi;
        pi.color.x = red;
        pi.color.y = green;
        pi.color.z = blue;
        pi.position = _pInfo->position;
        pi.text = msg;
        effects->newParticle("ScoreHighlight", pi);
    }
}
//----------------------------------------------------------------------------
#include "SimpleEnemy.hpp"
void Hero::hit(ParticleInfo * p, int damage, int /*radIndex*/)
{
    //    XTRACE();
    if( !_isAlive)
        return;
    //cout << "Hero hit with " << damage << endl;

    if (damage > 0)
        AudioS::instance()->playSample( "sounds/bang.wav");

    if( _shieldEnergy > 0)
    {
        _shieldEnergy -= damage;
        if( _shieldEnergy <= 0)
        {
            _shieldEnergy = 0;
            popMessage("Shield lost", 1.0f, 0.8f, 0.0f);
        }
    }
    else
    {
        _energy -= damage;
        if (damage > 0)
        {
            // remove some of the weapons
            Cargo &c = GameS::instance()->_cargo;
            std::string toLose[] = { "Proton spread fire", "Proton enhancer",
                "Wave emitter", "Smart bomb", "Shield upgrade" };
            for (int tries = 0; tries < 3; tries++)
            {
                int rnd = Random::integer(sizeof(toLose)/sizeof(std::string));
                CargoItem *item = c.findItem(toLose[rnd]);
                if (item->_quantity > 0)
                {
                    item->_quantity--;
                    char buff[200];
                    sprintf(buff, "%s lost", toLose[rnd].c_str());
                    popMessage(buff, 1.0f, 0.6f, 0.0f);
                    assignWeapons();
                    break;
                }
            }
        }
    }

    //hero dead...
    if( _energy <= 0)
    {
        static ParticleGroup *effects =
            ParticleGroupManagerS::instance()->getParticleGroup(EFFECTS_GROUP2);

        AudioS::instance()->playSample( "sounds/big_explosion.wav");

        //spawn explosion
        for( int i=0; i<(int)(GameState::horsePower/1.5); i++)
        {
            effects->newParticle(
            "ExplosionPiece", p->position.x, p->position.y, p->position.z);
        }
        _isAlive = false;
        GameS::instance()->_hyperspaceCount = 0;
        GameS::instance()->_spaceStationApproach = 0;
    }
}
//----------------------------------------------------------------------------
bool Hero::init( void)
{
    XTRACE();
    _model = ModelManagerS::instance()->getModel("models/Hero");
    return(_model!=0);
}
//----------------------------------------------------------------------------
void Hero::spawnSparks( int spawnCount, float r, ParticleInfo &pi)
{
    static ParticleGroup *effects =
    ParticleGroupManagerS::instance()->getParticleGroup( EFFECTS_GROUP3);

    for( int i=0; i<spawnCount; i++)
    {
        int sparkType = Random::random()%3;
        string effect;
        switch( sparkType)
        {
            default:
            case 0:
                effect = "FireSpark1";
            break;
            case 1:
                effect = "FireSpark2";
            break;
            case 2:
                effect = "FireSpark3";
            break;
        }

        int rot = Random::random()%360;
        float sina = _sint[rot] * r;
        float cosa = _cost[rot] * r;

        effects->newParticle( effect,
            pi.position.x+sina, pi.position.y+cosa, pi.position.z);
    }
}
//----------------------------------------------------------------------------
void Hero::allowVerticalMovement(bool allow)
{
    if( allow)
        _maxY = MAX_Y;
    else
        _maxY = MIN_Y;
}
//----------------------------------------------------------------------------
bool Hero::update(ParticleInfo *p)
{
    //    XTRACE();
    if( !_isAlive)
        return false;

    updatePrevs(p);

    float & _xPos = p->position.x;
    float & _yPos = p->position.y;

    _weaponEnergy += 3.0f * GAME_STEP_SCALE;

    Clamp(_weaponEnergy, 0.0, 100.0);

    if (_moveLeft)
    {
        _xPos += _moveLeft;
        Clamp( _xPos, MIN_X, MAX_X);
    }
    if (_moveRight)
    {
        _xPos += _moveRight;
        Clamp( _xPos, MIN_X, MAX_X);
    }

    if (_moveUp)
    {
        _yPos += _moveUp;
        Clamp( _yPos, MIN_Y, _maxY);
    }
    if (_moveDown)
    {
        _yPos += _moveDown;
        Clamp( _yPos, MIN_Y, _maxY);
    }

    for (int i=0; i<Hero::MAX_WEAPONS; i++)
        if (_weaponAutofire[i] && weaponLoaded(i))
            weaponFire(true, i);

    spawnSparks(_shieldEnergy/3, _pInfo->radius, *p);
    lastXPos = _xPos;
    lastYPos = _yPos;
    return true;
}
//----------------------------------------------------------------------------
bool Hero::weaponLoaded( int weapNum)
{
//    XTRACE();
    if( _weaponAmmo[ weapNum] == 0)
        return false;
    return( _weaponReload[ weapNum] < GameState::stopwatch.getTime());
}
//----------------------------------------------------------------------------
void Hero::move( float dx, float dy)
{
//    XTRACE();
    if( !_isAlive)
        return;
    if( !_pInfo)
        return;

    float & _xPos = _pInfo->position.x;
    float & _yPos = _pInfo->position.y;

    _xPos += dx;
    _yPos += dy;

    Clamp( _xPos, MIN_X, MAX_X);
    Clamp( _yPos, MIN_Y, _maxY);
}
//----------------------------------------------------------------------------
void Hero::move( Direction::DirectionEnum dir, bool isDown)
{
//    XTRACE();
    if( !_isAlive)
        return;
    float delta;

    if( isDown)
    {
        delta = 3.0f * GAME_STEP_SCALE;
    }
    else
    {
        delta = 0;
    }

//    LOG_INFO << "delta = " << delta << endl;

    switch( dir)
    {
        case Direction::eDown:
            _moveDown = -delta;
            break;
        case Direction::eUp:
            _moveUp = delta;
            break;

        case Direction::eLeft:
            _moveLeft = -delta;
            break;
        case Direction::eRight:
            _moveRight = delta;
            break;

        default:
            break;
    }
}
//----------------------------------------------------------------------------
void Hero::upgradeWeapons( void)
{
//    XTRACE();
}
//----------------------------------------------------------------------------
void Hero::weaponFire( bool isDown, int weapNum)
{
    //    XTRACE();
    if( !_isAlive || !_pInfo)
        return;

    float & _xPos = _pInfo->position.x;
    float & _yPos = _pInfo->position.y;
    float & _zPos = _pInfo->position.z;

    static bool needButtonUp = false;
    if (isDown == false)
        needButtonUp = false;
    if (needButtonUp)
        return;

    _weaponAutofire[weapNum] = isDown;
    if (isDown && weaponLoaded(weapNum))
    {
        if (!_weapon[weapNum])
        {
            LOG_WARNING << "No weapon mounted in slot " << weapNum << endl;
            return;
        }

        float er = _weapon[weapNum]->energyRequired();
        if (_weaponEnergy < er)
            return;

        ConfigS::instance()->getBoolean( "autofireOn", _autofireOn);
        if (!_autofireOn)
            needButtonUp = true;
        _weaponEnergy -= er;

        //FIXME: offset depending on where weapon is mounted
        _weapon[weapNum]->launch(_xPos, _yPos, _zPos);

        _weaponReload[weapNum] = _weapon[weapNum]->reloadTime() +
            GameState::stopwatch.getTime();
        if (_weaponAmmo[weapNum] > 0)
            _weaponAmmo[weapNum]--;
    }
}
//----------------------------------------------------------------------------
void Hero::setArmorPierce( float damageMultiplier)
{
    _damageMultiplier = damageMultiplier;
    for (int i = 0; i < Hero::MAX_WEAPONS; i++)
        _weapon[i]->setDamageMultiplier(_damageMultiplier);
}
//----------------------------------------------------------------------------
void Hero::drawWeapon( unsigned int weapNum)
{
    if ((weapNum < Hero::MAX_WEAPONS) && _weapon[weapNum])
        _weapon[weapNum]->draw();
}
//----------------------------------------------------------------------------
void Hero::draw( void)
{
//    XTRACE();
    if( !_isAlive || !_pInfo)
        return;

    ParticleInfo pi;
    interpolate( _pInfo, pi);

    glDisable( GL_DEPTH_TEST);
    glDisable( GL_LIGHTING);

    glTranslatef( pi.position.x, pi.position.y, pi.position.z);

    //draw a halo around hero...
    GLBitmapCollection *bitmaps =
    BitmapManagerS::instance()->getBitmap( "bitmaps/ammo");
    static bool wasInit = false;
    static int shield;
    if( !wasInit)
    {
        shield = bitmaps->getIndex( "Shield");
        wasInit = true;
    }

    glEnable(GL_TEXTURE_2D);
    bitmaps->bind();
    float r = _pInfo->radius/((42.0f-2.0f)/2.0f);
    glColor4f(1.0,1.0,1.0,(float)_shieldEnergy/(float)200.0);
    bitmaps->DrawC( shield, 0, 0, r, r);
    glDisable(GL_TEXTURE_2D);

    glEnable( GL_LIGHTING);
    glEnable( GL_DEPTH_TEST);

    _model->draw();
}
//----------------------------------------------------------------------------

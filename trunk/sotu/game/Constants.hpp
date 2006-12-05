// Description:
//   All kinds of constants.
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
#ifndef _Constants_hpp_
#define _Constants_hpp_

#include <string>
#include <defines.h>

#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif

const string GAMETITLE   = PACKAGE;
const string GAMEVERSION = VERSION;

const string HERO_GROUP                    = "Hero";
const string HERO_BULLETS_GROUP            = "HeroBullets";
const string ENEMIES_GROUP                 = "Enemies";
const string ENEMIES_BULLETS_GROUP         = "EnemyBullets";
const string BONUS_GROUP                   = "Bonus";
const string SHOOTABLE_ENEMY_BULLETS_GROUP = "ShootableEnemyBullets";
const string SHOOTABLE_BONUS_GROUP         = "ShootableBonus";
const string EFFECTS_GROUP1                = "Effects1";
const string EFFECTS_GROUP2                = "Effects2";
const string EFFECTS_GROUP3                = "Effects3";

const int MAX_PARTICLES_PER_GROUP=2048;

const float GAME_STEP_SIZE = 1.0f/30.0f; //run logic 30 times per second
const int MAX_GAME_STEPS = 10; //max number of logic runs per frame

// All updates in out logic are based on a game step size of 1/30.
// In case we want to use a different GAME_STEP_SIZE in the future,
// multiply all update values by GAME_STEP_SCALE.
const float GAME_STEP_SCALE = 30.0f*GAME_STEP_SIZE;

#endif

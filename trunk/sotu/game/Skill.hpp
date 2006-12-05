// Description:
//   Skill settings.
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
#ifndef _Skill_hpp_
#define _Skill_hpp_

#include <string>
#include <Singleton.hpp>

using std::string;

const string SKILL_ROOKIE = "ROOKIE";
const string SKILL_NORMAL = "NORMAL";
const string SKILL_EXPERT = "EXPERT";
const string SKILL_INSANE = "INSANE";
const string SKILL_ERROR = "*ERROR*";

class Skill
{
friend class Singleton<Skill>;
public:
    enum SkillEnum
    {
	eUnknown = 0,
	eRookie = 1,
	eNormal = 2,
	eExpert = 3,
	eInsane = 4,
	eLAST
    };

    inline static const string &getString( SkillEnum e)
    {
	switch( e)
	{
	    case eRookie:
	        return SKILL_ROOKIE;

	    case eNormal:
	        return SKILL_NORMAL;

	    case eExpert:
	        return SKILL_EXPERT;

	    case eInsane:
	        return SKILL_INSANE;

	    case eUnknown:
	    case eLAST:
	    default:
	        break;
	}
	return SKILL_ERROR;
    }

    static inline int getFireProbability( void);
    static inline int getMaxBullets( void);
    static inline int getMaxAttacking( void);

    void updateSkill( const Skill::SkillEnum &skill);
    void updateSkill( void);
    void incrementSkill( void);

private:
    ~Skill();
    Skill( void);
    Skill( const Skill&);
    Skill &operator=(const Skill&);

    SkillEnum convertStringToSkill( const string &skill);

    int _fireProbability;
    int _maxBullets;
    int _maxAttacking;
};

typedef Singleton<Skill> SkillS;

int Skill::getFireProbability( void) 
{ 
    return SkillS::instance()->_fireProbability;
}

int Skill::getMaxBullets( void) 
{ 
    return SkillS::instance()->_maxBullets;
}

int Skill::getMaxAttacking( void) 
{ 
    return SkillS::instance()->_maxAttacking;
}

#endif

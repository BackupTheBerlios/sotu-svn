// Description:
//   Particle group manager.
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
#ifndef _ParticleGroupManager_hpp_
#define _ParticleGroupManager_hpp_

#include <string>
#include <list>
#include <hashMap.hpp>

#include <HashString.hpp>
#include <Singleton.hpp>

class ParticleGroup;

class ParticleGroupManager
{
friend class Singleton<ParticleGroupManager>;
public:
    bool init( void);
    void reset( void);
    void draw( void);
    bool update( void);

    void addGroup( const string &groupName, int groupSize);
    void addLink( const string &group1, const string &group2);
    ParticleGroup *getParticleGroup( const string &groupName);

    int getAliveCount( void);

private:
    ~ParticleGroupManager();
    ParticleGroupManager( void);

    ParticleGroupManager( const ParticleGroupManager&);
    ParticleGroupManager &operator=(const ParticleGroupManager&);

    hash_map< 
        const string, ParticleGroup*, hash<const string>, equal_to<const string> > _particleGroupMap;
    list<ParticleGroup*> _particleGroupList;

    struct LinkedParticleGroup
    {
        ParticleGroup *group1;
        ParticleGroup *group2;
    }; 

    list<LinkedParticleGroup*> _linkedParticleGroupList;
};

typedef Singleton<ParticleGroupManager> ParticleGroupManagerS;

#endif

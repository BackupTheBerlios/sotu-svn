// Description:
//   Model Manager.
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
#ifndef _ModelManager_hpp_
#define _ModelManager_hpp_

#include <string>
#include <hashMap.hpp>

#include <Trace.hpp>
#include <HashString.hpp>
#include <FindHash.hpp>
#include <Model.hpp>
#include <Singleton.hpp>

class ModelManager
{
friend class Singleton<ModelManager>;
public:
    Model *getModel( string modelName)
    {
        Model *model = findHash( modelName, _modelMap);
        if( !model)
        {
	    model = load( modelName);
	    if( !model)
	    {
		LOG_ERROR << "Unable to find model " << modelName << endl;
		return 0;
	    }
	    _modelMap[ modelName] = model;
        }
        return model;
    }

    void reload( void);

private:
    ~ModelManager();
    ModelManager( void);
    ModelManager( const ModelManager&);
    ModelManager &operator=(const ModelManager&);

    Model *load( const string &model);

    hash_map< string, Model*, hash<string>, equal_to<string> > _modelMap;
};

typedef Singleton<ModelManager> ModelManagerS;

#endif

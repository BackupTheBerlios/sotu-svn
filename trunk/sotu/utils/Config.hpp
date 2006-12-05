// Description:
//   Singleton to Keep track of configurable values. 
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
#ifndef _Config_hpp_
#define _Config_hpp_

#include <list>
#include <hashMap.hpp>
#include <string>

using namespace std;

#include <Value.hpp>
#include <Singleton.hpp>
#include <ConfigHandler.hpp>

#include <HashString.hpp>

const string CONFIGFILE = "ConfigFile";

class Config
{
friend class Singleton<Config>;
public:
    struct ConfigItem
    {
	string key;
	string value;
    };

    //Persistent keyword/values start with a "+", transitory with a "-"
    void updateFromCommandLine( int argc, char *argv[]);

    void updateFromFile( void);
    void updateKeyword( const char *keyword, const char *value);
    void updateKeyword( const string keyword, const string value);
    void updateKeyword( const string keyword, Value *value);
    void remove( const string &keyword);

    //Transitory items are not saved. They override persistent values.
    void updateTransitoryKeyword( const char *keyword, const char *value);
    void updateTransitoryKeyword( const string keyword, const string value);
    void updateTransitoryKeyword( const string keyword, Value *value);
    void removeTrans( const string &keyword);

    //when reading/writing the config file, registerd ConfigHandlers
    //will be notified (see ConfigHandler)
    void registerConfigHandler( ConfigHandler *ch)
    {
        _configHandlerList.insert( _configHandlerList.begin(), ch);
    }

    bool getString( const string &keyword, string &value);
    bool getInteger( const string &keyword, int &value);
    bool getFloat( const string &keyword, float &value);
    bool getBoolean( const string &keyword, bool &value);

    void saveToFile( bool truncate=true);
    void dump( void);
    void getConfigItemList( list<ConfigItem> &ciList);

    const string &getConfigFileName( void);
    void setDefaultConfigFileName( char *defaultName)
    {
	_defaultConfigFileName = defaultName;
    }
    void setSubdirectory( char *dirname)
    {
	_subdir = dirname;
    }
    string getConfigDirectory( void)
    {
    	return _configDirectory;
    }

private:
    ~Config();
    Config( void);
    Config( const Config&);
    Config &operator=(const Config&);

    void removeImpl( const string &keyword,
	    hash_map< string, Value*, hash<string>, equal_to<string> > &kvmap);

    bool getStringImpl( const string &keyword, string &value, 
	    hash_map< string, Value*, hash<string>, equal_to<string> > &kvmap);
    bool getIntegerImpl( const string &keyword, int &value, 
	    hash_map< string, Value*, hash<string>, equal_to<string> > &kvmap);
    bool getBooleanImpl( const string &keyword, bool &value,
	    hash_map< string, Value*, hash<string>, equal_to<string> > &kvmap);
    bool getFloatImpl( const string &keyword, float &value,
	    hash_map< string, Value*, hash<string>, equal_to<string> > &kvmap);

    string _defaultConfigFileName;
    string _subdir;
    string _configDirectory;
    list<ConfigHandler*> _configHandlerList;
    hash_map< string, Value*, hash<string>, equal_to<string> > _keyValueMap;
    hash_map< string, Value*, hash<string>, equal_to<string> > _keyValueMapTrans;
};

typedef Singleton<Config> ConfigS;

#endif

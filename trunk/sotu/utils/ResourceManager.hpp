// Description:
//   Simple pak file type resouce manager.
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
#ifndef _ResourceManager_hpp_
#define _ResourceManager_hpp_

#include <list>
#include <hashMap.hpp>

#include <zStream.hpp>
#include <HashString.hpp>
#include <Singleton.hpp>

class GLTexture;
struct DirectoryEntry
{
    string resourceName;
    string resourcePackFilename;
    Uint32 offset;
    Uint32 origSize;
    Uint32 compSize;
};

const Uint32 RESOURCE_MAGIC = 0xfbfb3755;

class ResourceManager
{
friend class Singleton<ResourceManager>;
public:
    bool addResourcePack( const string &filename, const string &basedir = "");
    void addResourceDirectory( const string &dirName);

    bool selectResource( const string &name);
    int getResourceSize( const string &name);
    int readResource( char *buffer, int size)
    {
        _activeInput->read( buffer, size);
        return size;
    }
    ziStream &getInputStream( void);
    GLTexture* getTexture(const std::string& name);

    void getResourceList( list<string> &rNameList);
    void dump( void);

private:
    ResourceManager( const ResourceManager&);
    ResourceManager &operator=(const ResourceManager&);

    ResourceManager( void):
        HEADER_SIZE(8),
        _openFilename(""),
        _activeInput(0)
    {
    }
    ~ResourceManager();
    void updateInfile( const string &inputFilename);
    void addResourceDirectoryRecursive( const string &dirName);

    const streamoff HEADER_SIZE; //8;

    string _openFilename;
    ifstream _infile;
    ziStream *_activeInput;
    hash_map< const string, DirectoryEntry*, hash<const string>, equal_to<const string> > _dirEntryMap;
    string::size_type _baseLen;
};

typedef Singleton<ResourceManager> ResourceManagerS;

#endif

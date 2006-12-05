// Description:
//   Resource cache template.
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

#ifndef _ResourceCache_hpp_
#define _ResourceCache_hpp_

#include <string>
#include <hashMap.hpp>
#include <HashString.hpp>
#include <FindHash.hpp>
#include <Trace.hpp>

template< class ResourceT >
class ResourceCache
{
public:
    ResourceT *getResource( const string &resourceName)
    {
        ResourceT *resource = findHash( resourceName, _resourceMap);
        if( !resource)
        {
	    LOG_INFO << resourceName << " not cached -> loading..." << endl;
            resource = load( resourceName);
            if( !resource)
            {
                return 0;
            }
            _resourceMap[ resourceName] = resource;
        }
        return resource;
    }

    virtual void reload( void)
    {
#if 0
	hash_map< string, ResourceT*, hash<string>, equal_to<string> >::const_iterator ci;
	for( ci=_resourceMap.begin(); ci!=_resourceMap.end(); ci++)
	{
	    ResourceT *resource = ci->second;
	    resource->reload();
	}
#endif
    }

protected:
    ResourceCache( void)
    {
    }
    virtual ~ResourceCache()
    {
	typename hash_map< const string, ResourceT*, hash<const string>, equal_to<const string> >::const_iterator ci;
	for( ci=_resourceMap.begin(); ci!=_resourceMap.end(); ci++)
	{
	    ResourceT *res = ci->second;
	    delete res;
	}
	_resourceMap.clear();
    }

    virtual ResourceT *load( const string &resource) = 0;
    hash_map< const string, ResourceT*, hash<const string>, equal_to<const string> > _resourceMap;

private:
    ResourceCache( const ResourceCache&);
    ResourceCache &operator=(const ResourceCache&);
};

#endif

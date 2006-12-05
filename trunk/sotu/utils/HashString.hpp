// Description:
//   Hash functions for strings.
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
#ifndef _HashString_hpp_
#define _HashString_hpp_

#include <hashMap.hpp>
#include <string>
using namespace std;

namespace HASH_NAMESPACE
{
    template<>
	struct hash<const string>
    {
	//a simple hash function for string
	int operator()(const string & s) const
	{
	    int hashval = 13*(int)s.size() + 5*s[(int)s.size()/2];
    //        cout << "  > Hash of " << s << " = " << hashval << endl;
	    return hashval;
	}
    };

    template<>
	struct hash<string>
    {
	//a simple hash function for string
	int operator()(string s) const
	{
	    int hashval = 13*(int)s.size() + 5*s[(int)s.size()/2];
    //        cout << "  > Hash of " << s << " = " << hashval << endl;
	    return hashval;
	}
    };

} //namespace std
#endif

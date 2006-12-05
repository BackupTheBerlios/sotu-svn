// Description:
//   SGI/GCC hash_map STL extension.
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
#if defined( __GNUC__ )

#  if (__GNUC__ == 2)
#    include <hash_map>
#    define HASH_NAMESPACE std

#  elif (__GNUC__ == 3)
#    include <ext/hash_map>

#    if (__GNUC_MINOR__ == 0)
#      define HASH_NAMESPACE std
#    else
#      define HASH_NAMESPACE __gnu_cxx
#    endif
     using namespace HASH_NAMESPACE;

#  elif (__GNUC__ >= 4)
#    include <ext/hash_map>
#    define HASH_NAMESPACE __gnu_cxx
     using namespace HASH_NAMESPACE;

#  endif

#elif (__INTEL_COMPILER >= 700)
#  define HASH_NAMESPACE std
#  include <hash_map>
   using namespace HASH_NAMESPACE;
#elif defined(STLPORT)
#  define HASH_NAMESPACE std
#  include <hash_map>
   using namespace HASH_NAMESPACE;
#else

#error "Need hash_map STL extension"

#endif

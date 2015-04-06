/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __DAVAENGINE_HASH__
#define __DAVAENGINE_HASH__

#include <stddef.h>
#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
namespace DAVA
{
    // TODO: Think how to make it work for generic pointers and char * at the same time
    class Entity;
    
	// default hash function for strings
	inline size_t DavaHashString(const char* str)
	{
		DVASSERT(str && "Can't be NULL. Check logics.");

		size_t hash = 0;
		for (; *str; ++str)
		{
			hash = 5 * hash + *str;
		}
		return hash;
	}

	// Base hash type
	// Any child (template specialized) structure
	// should implement for specific type T:
	// - hash function: operator()(T value)
	// - compare function: compare(T value1, T value2)
	template <typename T>
    struct Hash;

	// specialization for char *
	template<> struct Hash <char *>
	{
		size_t operator()(const char *str) const
		{
			return DavaHashString(str);
		}

		bool Compare(const char *str1, const char *str2) const
		{
			return (str1 == str2) || (0 == strcmp(str1, str2));
		}
	};

	// specialization for const char *
	template<> struct Hash <const char *>
	{
		size_t operator()(const char *str) const
		{
			return DavaHashString(str);
		}

		bool Compare(const char *str1, const char *str2) const
		{
			return (str1 == str2) || (0 == strcmp(str1, str2));
		}
	};

	// specialization for const DAVA::String &
	template<> struct Hash <DAVA::String>
	{
		size_t operator()(const DAVA::String &str) const
		{
			return DavaHashString(str.c_str());
		}

		bool Compare(const DAVA::String &str1, const DAVA::String &str2) const
		{
			return (str1 == str2);
		}
	};

	// specialization for DAVA::String *
	template<> struct Hash <DAVA::String *>
	{
		size_t operator()(const DAVA::String *str) const
		{
			return DavaHashString(str->c_str());
		}

		bool Compare(const DAVA::String *str1, const DAVA::String *str2) const
		{
			return (str1 == str2) || 0 == str1->compare(str2->c_str());
		}
	};

	// specialization for const DAVA::String *
	template<> struct Hash <const DAVA::String *>
	{
		size_t operator()(const DAVA::String *str) const
		{
			return DavaHashString(str->c_str());
		}

		bool Compare(const DAVA::String *str1, const DAVA::String *str2) const
		{
			return (str1 == str2) || 0 == str1->compare(str2->c_str());
		}
	};
    
    // specialization for all pointers
	template<typename T> struct Hash <T*>
	{
		size_t operator()(T * pointer) const
		{
			return (size_t)pointer;
		}
        
		bool Compare(T *ptr1, T *ptr2) const
		{
			return (ptr1 == ptr2);
		}
	};

	template<> struct Hash <DAVA::int32>
	{
		size_t operator()(const DAVA::int32 i) const
		{
			return i;
		}

		bool Compare(const DAVA::int32 i1, const DAVA::int32 i2) const
		{
			return (i1 == i2);
		}
	};

	template<> struct Hash <DAVA::uint32>
	{
		size_t operator()(const DAVA::uint32 i) const
		{
			return i;
		}

		bool Compare(const DAVA::uint32 i1, const DAVA::uint32 i2) const
		{
			return (i1 == i2);
		}
	};




// NOTE: 'str' expected to be immutable const static string
#define DV_HASH(str)    DAVA::StringHash(str).hash
#define DV_HASH_INLINE  inline

DV_HASH_INLINE DAVA::uint32
HashValue_N( const char* key, unsigned length )
{
    using DAVA::uint32;

    #define _Hash_Mix( a, b, c )    \
    a -= b; a -= c; a ^= (c>>13);   \
    b -= c; b -= a; b ^= (a<<8);    \
    c -= a; c -= b; c ^= (b>>13);   \
    a -= b; a -= c; a ^= (c>>12);   \
    b -= c; b -= a; b ^= (a<<16);   \
    c -= a; c -= b; c ^= (b>>5);    \
    a -= b; a -= c; a ^= (c>>3);    \
    b -= c; b -= a; b ^= (a<<10);   \
    c -= a; c -= b; c ^= (b>>15);   \


    // set up the internal state

    uint32  len = length;
    uint32  a   = 0x9E3779B9;   // the golden ratio, an arbitrary value
    uint32  b   = 0x9E3779B9;   // the golden ratio, an arbitrary value
    uint32  c   = 0;            // the previous hash value


    // handle most of the key
    
    while( len >= 12 )
    {
        a += (key[0] + ((uint32)key[1]<<8) + ((uint32)key[2]<<16) + ((uint32)key[3]<<24));
        b += (key[4] + ((uint32)key[5]<<8) + ((uint32)key[6]<<16) + ((uint32)key[7]<<24));
        c += (key[8] + ((uint32)key[9]<<8) + ((uint32)key[10]<<16) + ((uint32)key[11]<<24));

        _Hash_Mix( a, b, c );

        key += 12; 
        len -= 12;
    }


    // handle the last 11 bytes
    
    c += length;

    //lint -e{616} // control flows into case/default
    switch( len ) 
    {
        // all the case statements fall through
        
        case 11: c += ((uint32)key[10]<<24);
        case 10: c += ((uint32)key[9]<<16);
        case 9 : c += ((uint32)key[8]<<8);
        // the first byte of c is reserved for the length
        case 8 : b += ((uint32)key[7]<<24);
        case 7 : b += ((uint32)key[6]<<16);
        case 6 : b += ((uint32)key[5]<<8);
        case 5 : b += key[4];
        case 4 : a += ((uint32)key[3]<<24);
        case 3 : a += ((uint32)key[2]<<16);
        case 2 : a += ((uint32)key[1]<<8);
        case 1 : a += key[0];
    //  case 0: nothing left to add
    }

    _Hash_Mix( a, b, c );

    return c;
    #undef _Hash_Mix
}


class 
StringHash
{ 
public:

    uint32  hash;

    DV_HASH_INLINE  StringHash( const char (&str)[1] )  : hash(HashValue_N(str,1-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[2] )  : hash(HashValue_N(str,2-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[3] )  : hash(HashValue_N(str,3-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[4] )  : hash(HashValue_N(str,4-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[5] )  : hash(HashValue_N(str,5-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[6] )  : hash(HashValue_N(str,6-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[7] )  : hash(HashValue_N(str,7-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[8] )  : hash(HashValue_N(str,8-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[9] )  : hash(HashValue_N(str,9-1))    {}
    DV_HASH_INLINE  StringHash( const char (&str)[10] ) : hash(HashValue_N(str,10-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[11] ) : hash(HashValue_N(str,11-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[12] ) : hash(HashValue_N(str,12-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[13] ) : hash(HashValue_N(str,13-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[14] ) : hash(HashValue_N(str,14-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[15] ) : hash(HashValue_N(str,15-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[16] ) : hash(HashValue_N(str,16-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[17] ) : hash(HashValue_N(str,17-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[18] ) : hash(HashValue_N(str,18-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[19] ) : hash(HashValue_N(str,19-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[20] ) : hash(HashValue_N(str,20-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[21] ) : hash(HashValue_N(str,21-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[22] ) : hash(HashValue_N(str,22-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[23] ) : hash(HashValue_N(str,23-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[24] ) : hash(HashValue_N(str,24-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[25] ) : hash(HashValue_N(str,25-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[26] ) : hash(HashValue_N(str,26-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[27] ) : hash(HashValue_N(str,27-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[28] ) : hash(HashValue_N(str,28-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[29] ) : hash(HashValue_N(str,29-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[30] ) : hash(HashValue_N(str,30-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[31] ) : hash(HashValue_N(str,31-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[32] ) : hash(HashValue_N(str,32-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[33] ) : hash(HashValue_N(str,33-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[34] ) : hash(HashValue_N(str,34-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[35] ) : hash(HashValue_N(str,35-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[36] ) : hash(HashValue_N(str,36-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[37] ) : hash(HashValue_N(str,37-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[38] ) : hash(HashValue_N(str,38-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[39] ) : hash(HashValue_N(str,39-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[40] ) : hash(HashValue_N(str,40-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[41] ) : hash(HashValue_N(str,41-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[42] ) : hash(HashValue_N(str,42-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[43] ) : hash(HashValue_N(str,43-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[44] ) : hash(HashValue_N(str,44-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[45] ) : hash(HashValue_N(str,45-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[46] ) : hash(HashValue_N(str,46-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[47] ) : hash(HashValue_N(str,47-1))   {}
    DV_HASH_INLINE  StringHash( const char (&str)[48] ) : hash(HashValue_N(str,48-1))   {}
};


};

#endif // __DAVAENGINE_HASH__

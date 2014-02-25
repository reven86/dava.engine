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
	struct Hash
	{ };

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
};

#endif // __DAVAENGINE_HASH__

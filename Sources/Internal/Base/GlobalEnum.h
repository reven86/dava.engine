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


#ifndef __DAVAENGINE_GLOBAL_ENUM_H__
#define __DAVAENGINE_GLOBAL_ENUM_H__

#include "Base/EnumMap.h"

template<typename T>
class GlobalEnumMap
{
public:
	explicit GlobalEnumMap();
	~GlobalEnumMap();

	static const EnumMap* Instance();

protected:
	static void RegisterAll();
	static void Register(const int e, const char *s);
};

template<typename T>
const EnumMap* GlobalEnumMap<T>::Instance()
{
	static EnumMap enumMap;
	static bool initialized = false;

	if(!initialized)
	{
		initialized = true;
		RegisterAll();
	}

	return &enumMap;
}

template<typename T>
void GlobalEnumMap<T>::Register(const int e, const char *s)
{
	Instance()->Register(e, s);
}

#define ENUM_DECLARE(eType) template<> void GlobalEnumMap<eType>::RegisterAll()
#define ENUM_ADD(eValue) Register(eValue, #eValue)
#define ENUM_ADD_DESCR(eValue, eDescr) Register(eValue, eDescr)

// Define:
//	ENUM_DECLARE(AnyEnumType)
//	{
//		ENUM_ADDS(AnyEnumType::Value1);
//		ENUM_ADD_DESCR(AnyEnumType::Value2, "Value2");
//	}
//	
// Usage:
//  GlobalEnumMap::Instance<AnyEnumType>();

#endif // __DAVAENGINE_GLOBAL_ENUM_H__

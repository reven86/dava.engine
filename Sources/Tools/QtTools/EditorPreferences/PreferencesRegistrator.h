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


#pragma once

#include "PreferencesStorage.h"

namespace DAVA
{
class InspBase;
}

struct InspInfoRegistrator
{
    InspInfoRegistrator(const DAVA::InspInfo* inspInfo, const PreferencesStorage::DefaultValuesList& defaultValues = PreferencesStorage::DefaultValuesList());
};

struct GlobalValuesRegistrator
{
    GlobalValuesRegistrator(const DAVA::FastName& key, const DAVA::VariantType& defaultValue);
};

template <typename T>
class PreferencesRegistrator
{
public:
    PreferencesRegistrator(T* obj);
    ~PreferencesRegistrator();

private:
    T* objectPtr = nullptr;
};

template <typename T>
PreferencesRegistrator<T>::PreferencesRegistrator(T* obj)
    : objectPtr(obj)
{
    PreferencesStorage::Instance()->RegisterPreferences(objectPtr);
}

template <typename T>
PreferencesRegistrator<T>::~PreferencesRegistrator()
{
    PreferencesStorage::Instance()->UnregisterPreferences(objectPtr);
}

//use this macro to register PreferencesRegistrator as class member. Will not work if preferences methods use class members, which will be created in c-tor after
#define REGISTER_PREFERENCES(Class) \
    PreferencesRegistrator<Class> preferencesRegistrator = PreferencesRegistrator<Class>(this);

//use this macro to register introspection when program starts. A PreferencesStorage require this to work correctly
#define REGISTER_PREFERENCES_ON_START(Class, ...) \
    namespace Class##_local \
    { \
    InspInfoRegistrator inspInfoRegistrator(Class::TypeInfo(), { __VA_ARGS__ }); \
    };

//use this macro with macro REGISTER_PREFERENCES_ON_START(Class, PREF_ARG("name", true))
#define PREF_ARG(name, value) \
    { DAVA::FastName(name), DAVA::VariantType(value) }

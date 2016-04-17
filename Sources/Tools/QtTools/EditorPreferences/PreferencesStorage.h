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


#ifndef PREFERENCES_STORAGE
#define PREFERENCES_STORAGE

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FilePath.h"
#include "Base/StaticSingleton.h"
#include "PreferencesRegistrator.h"
#include "Functional/Signal.h"

namespace DAVA
{
class InspBase;
};

class PreferencesStorage : public DAVA::StaticSingleton<PreferencesStorage>
{
public:
    using DefaultValuesList = DAVA::Map<DAVA::FastName, DAVA::VariantType>;
    using RegisteredIntrospection = DAVA::Vector<const DAVA::InspInfo*>;

    PreferencesStorage();
    ~PreferencesStorage();
    static void RegisterType(const DAVA::InspInfo* inspInfo, const DefaultValuesList& defaultValues = DefaultValuesList());
    template <typename T>
    static void RegisterPreferences(T* obj);
    template <typename T>
    static void UnregisterPreferences(T* obj);

    static void RegisterPreferences(void* realObj, DAVA::InspBase* inspBase);
    static void UnregisterPreferences(void* realObj, const DAVA::InspBase* inspBase);

    static void SetupStoragePath(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage);

    static void SaveValueByKey(const DAVA::FastName& key, const DAVA::VariantType& value);
    static DAVA::VariantType LoadValueByKey(const DAVA::FastName& key);

    static const DAVA::InspInfo* GetInspInfo(const DAVA::FastName& className);
    static const DAVA::InspMember* GetInspMember(const DAVA::InspInfo* inspInfo, const DAVA::FastName& propertyName);

    static void SetNewValueToAllRegisteredObjects(const DAVA::InspInfo* inspInfo, const DAVA::InspMember* member, const DAVA::VariantType& value);
    static DAVA::VariantType GetPreferencesValue(const DAVA::InspMember* member);

    static const RegisteredIntrospection& GetRegisteredInsp();

    struct PreferencesStorageSaver
    {
        PreferencesStorageSaver();
        ~PreferencesStorageSaver();
    };

    DAVA::Signal<const DAVA::InspInfo*, const DAVA::InspMember*, const DAVA::VariantType&> ValueChanged;

private:
    void SetupStoragePathImpl(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage);
    void RegisterPreferencesImpl(void* realObj, DAVA::InspBase* inspBase);
    void UnregisterPreferencesImpl(void* realObj, const DAVA::InspBase* inspBase);

    void SaveValueByKeyImpl(const DAVA::FastName& key, const DAVA::VariantType& value);
    DAVA::VariantType LoadValueByKeyImpl(const DAVA::FastName& key);

    void SetNewValueToAllRegisteredObjectsImpl(const DAVA::InspInfo* inspInfo, const DAVA::InspMember* member, const DAVA::VariantType& value);

    static DAVA::String GenerateKey(const DAVA::InspInfo* inspInfo);

    DAVA::FilePath localStorage;
    DAVA::ScopedPtr<DAVA::KeyedArchive> editorPreferences;

    RegisteredIntrospection registeredInsp;
    DAVA::Map<const DAVA::InspInfo*, DefaultValuesList> defaultValues;
    DAVA::Map<const DAVA::InspInfo*, DAVA::Set<void*>> registeredObjects;
    DAVA::KeyedArchive* preferencesArchive = nullptr;
    DAVA::KeyedArchive* unnamedPreferencesArchive = nullptr;
    const DAVA::String unnamedPreferencesKey;
    const DAVA::String preferencesKey;
};

template <typename T>
void PreferencesStorage::RegisterPreferences(T* realObject)
{
    static_assert(std::is_base_of<DAVA::InspBase, T>::value, "type T must be derived from InspBase");
    RegisterPreferences(realObject, static_cast<DAVA::InspBase*>(realObject));
}

template <typename T>
void PreferencesStorage::UnregisterPreferences(T* realObject)
{
    static_assert(std::is_base_of<DAVA::InspBase, T>::value, "type T must be derived from InspBase");
    UnregisterPreferences(realObject, static_cast<DAVA::InspBase*>(realObject));
}

#endif //PREFERENCES_STORAGE

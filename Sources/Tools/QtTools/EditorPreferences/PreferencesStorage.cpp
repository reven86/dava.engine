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


#include "Debug/DVAssert.h"
#include "PreferencesStorage.h"

namespace PreferencesStorage_local
{
DAVA::String rawPreferencesKey = "raw local preferences storage preferences";
}

struct PreferencesStorage::ClassInfo
{
    ClassInfo(const DAVA::InspInfo* inspInfo_, const PreferencesRegistrator::DefaultValuesList& defaultValues_)
        : inspInfo(inspInfo_)
        , defaultValues(defaultValues_)
    {
    }
    const DAVA::InspInfo* inspInfo;
    PreferencesRegistrator::DefaultValuesList defaultValues;
};

PreferencesStorage::PreferencesStorageSaver::PreferencesStorageSaver()
{
    PreferencesStorage::Instance(); //check that the storage exists
}

PreferencesStorage::PreferencesStorageSaver::~PreferencesStorageSaver()
{
    PreferencesStorage* storage = PreferencesStorage::Instance();
    if (!storage->editorPreferences->Save(storage->localStorage))
    {
        DAVA::Logger::Error("can not save editor preferences!");
    }
}

PreferencesStorage::PreferencesStorage()
    : editorPreferences(new DAVA::KeyedArchive())
{
}

PreferencesStorage::~PreferencesStorage() = default;

void PreferencesStorage::RegisterType(const DAVA::InspInfo* inspInfo, const PreferencesRegistrator::DefaultValuesList& defaultValues)
{
    PreferencesStorage* self = Instance();
    self->registeredInsp.insert(std::make_unique<ClassInfo>(inspInfo, defaultValues)); //storing pointer to static memory
}

void PreferencesStorage::RegisterPreferences(void* realObject, DAVA::InspBase* inspBase)
{
    PreferencesStorage* self = Instance();
    self->RegisterPreferencesImpl(realObject, inspBase);
}

void PreferencesStorage::UnregisterPreferences(void* realObject, const DAVA::InspBase* inspBase)
{
    PreferencesStorage* self = Instance();
    self->UnregisterPreferencesImpl(realObject, inspBase);
}

void PreferencesStorage::SetupStoragePath(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage)
{
    PreferencesStorage* self = Instance();
    self->SetupStoragePathImpl(defaultStorage, localStorage);
}

void PreferencesStorage::SetupStoragePathImpl(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage_)
{
    localStorage = localStorage_;
    DAVA::ScopedPtr<DAVA::KeyedArchive> loadedPreferences(new DAVA::KeyedArchive);

    if (defaultStorage.Exists())
    {
        if (!loadedPreferences->Load(defaultStorage))
        {
            DAVA::Logger::Error("failed to load editor preferences from default storage");
        }
    }

    if (localStorage.Exists())
    {
        if (!loadedPreferences->Load(localStorage))
        {
            DAVA::Logger::Error("faild to load editor preferences from local storage");
        }
    }
    //all preferences must be registered on app start
    for (const auto& classInfo : registeredInsp) //create new settings archive
    {
        const PreferencesRegistrator::DefaultValuesList& defaultValues = classInfo->defaultValues;
        const DAVA::InspInfo* inspInfo = classInfo->inspInfo;
        DAVA::String key = GenerateKey(inspInfo);
        DAVA::KeyedArchive* loadedData = loadedPreferences->GetArchive(key);
        DAVA::ScopedPtr<DAVA::KeyedArchive> classInfoArchive(new DAVA::KeyedArchive);
        for (int i = 0, count = inspInfo->MembersCount(); i < count; ++i)
        {
            const DAVA::InspMember* member = inspInfo->Member(i);
            const DAVA::FastName memberName(member->Name());
            DAVA::VariantType value;
            if (nullptr != loadedData)
            {
                DAVA::VariantType* loadedValue = loadedData->GetVariant(memberName.c_str());
                if (nullptr != loadedValue)
                {
                    value = *loadedValue;
                }
            }
            if (value.type == DAVA::VariantType::TYPE_NONE)
            {
                auto iter = defaultValues.find(memberName);
                if (iter != defaultValues.end())
                {
                    value = iter->second;
                }
                else
                {
                    DAVA::StringStream ss;
                    ss << "no default value for insp member: " << inspInfo->Name().c_str() << " : " << memberName.c_str();
                    DVASSERT(false && ss.str().c_str());
                }
            }
            if (value.type != DAVA::VariantType::TYPE_NONE)
            {
                classInfoArchive->SetVariant(memberName.c_str(), value);
            }
        }
        if (classInfoArchive->Count() != 0)
        {
            editorPreferences->SetArchive(key, classInfoArchive);
        }
    }
    DAVA::KeyedArchive* rawPreferencesArchive = loadedPreferences->GetArchive(PreferencesStorage_local::rawPreferencesKey);
    if (rawPreferencesArchive != nullptr)
    {
        editorPreferences->SetArchive(PreferencesStorage_local::rawPreferencesKey, rawPreferencesArchive);
    }
}

void PreferencesStorage::RegisterPreferencesImpl(void* realObj, DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);

    const DAVA::InspInfo* info = inspBase->GetTypeInfo();
    registeredObjects[info].insert(realObj);
    DAVA::String key = GenerateKey(inspBase->GetTypeInfo());
    DAVA::KeyedArchive* archive = editorPreferences->GetArchive(key, nullptr);
    if (nullptr == archive)
    {
        return;
    }
    for (int i = 0, count = info->MembersCount(); i < count; ++i)
    {
        const DAVA::InspMember* member = info->Member(i);
        if ((member->Flags() & DAVA::I_PREFERENCE) != DAVA::I_PREFERENCE)
        {
            continue;
        }
        DAVA::String name(member->Name().c_str());
        if (!archive->IsKeyExists(name))
        {
            continue;
        }
        DAVA::VariantType* value = archive->GetVariant(name);
        if (value != nullptr && value->GetType() != DAVA::VariantType::TYPE_NONE)
        {
            member->SetValue(realObj, *value);
        }
    }
}

void PreferencesStorage::UnregisterPreferencesImpl(void* realObj, const DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);
    const DAVA::InspInfo* info = inspBase->GetTypeInfo();

    DAVA::ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive);

    for (int i = 0, count = info->MembersCount(); i < count; ++i)
    {
        const DAVA::InspMember* member = info->Member(i);
        if ((member->Flags() & DAVA::I_PREFERENCE) != DAVA::I_PREFERENCE)
        {
            continue;
        }
        DAVA::String name(member->Name().c_str());
        DAVA::VariantType value = member->Value(realObj); //SUDDENLY! current version not support Value by const pointer
        archive->SetVariant(name, value);
    }

    DAVA::String key = GenerateKey(inspBase->GetTypeInfo());
    editorPreferences->SetArchive(key, archive);
}

void PreferencesStorage::SaveValueByKey(const DAVA::FastName& key, const DAVA::VariantType& value)
{
    PreferencesStorage* self = Instance();
    self->SaveValueByKeyImpl(key, value);
}

DAVA::VariantType PreferencesStorage::LoadValueByKey(const DAVA::FastName& key)
{
    PreferencesStorage* self = Instance();
    return self->LoadValueByKeyImpl(key);
}

void PreferencesStorage::SaveValueByKeyImpl(const DAVA::FastName& key, const DAVA::VariantType& value)
{
    DAVA::KeyedArchive* rawPreferencesArchive = editorPreferences->GetArchive(PreferencesStorage_local::rawPreferencesKey);
    if (rawPreferencesArchive != nullptr)
    {
        rawPreferencesArchive->SetVariant(key.c_str(), value);
    }
}

DAVA::VariantType PreferencesStorage::LoadValueByKeyImpl(const DAVA::FastName& key)
{
    DAVA::KeyedArchive* rawPreferencesArchive = editorPreferences->GetArchive(PreferencesStorage_local::rawPreferencesKey);
    if (rawPreferencesArchive != nullptr)
    {
        DAVA::VariantType* loadedValutPtr = rawPreferencesArchive->GetVariant(key.c_str());
        if (nullptr != loadedValutPtr)
        {
            return *loadedValutPtr;
        }
    }
    return DAVA::VariantType();
}

const DAVA::InspInfo* PreferencesStorage::GetInspInfo(const DAVA::FastName& className)
{
    PreferencesStorage* self = Instance();
    for (const auto& iter : self->registeredObjects)
    {
        const DAVA::InspInfo* info = iter.first;
        if (info->Name() == className)
        {
            return info;
        }
    }
    return nullptr;
}

const DAVA::InspMember* PreferencesStorage::GetInspMember(const DAVA::InspInfo* inspInfo, const DAVA::FastName& propertyName)
{
    if (nullptr != inspInfo)
    {
        for (int i = 0, count = inspInfo->MembersCount(); i < count; ++i)
        {
            const DAVA::InspMember* member = inspInfo->Member(i);
            if (member->Name() == propertyName)
            {
                return member;
            }
        }
    }
    return nullptr;
}

void PreferencesStorage::SetNewValueToAllRegisteredObjects(const DAVA::InspInfo* inspInfo, const DAVA::InspMember* member, const DAVA::VariantType& value)
{
    DVASSERT(nullptr != inspInfo && nullptr != member);
    PreferencesStorage* self = Instance();
    const auto& registeredObjects = self->registeredObjects.at(inspInfo);
    for (void* registeredObject : registeredObjects)
    {
        member->SetValue(registeredObject, value);
        self->ValueChanged.Emit(inspInfo, member, value);
    }
}

DAVA::VariantType PreferencesStorage::GetPreferencesValue(const DAVA::InspMember* member)
{
    if (member == nullptr)
    {
        return DAVA::VariantType();
    }
    const DAVA::InspInfo* inspInfo = member->GetParentInsp();
    DVASSERT(nullptr != inspInfo);
    auto key = GenerateKey(inspInfo);
    PreferencesStorage* self = Instance();
    DAVA::KeyedArchive* archive = self->editorPreferences->GetArchive(key, nullptr);
    if (archive == nullptr)
    {
        DVASSERT(false && "introspection are not registered in preferences storage");
        return DAVA::VariantType();
    }
    DAVA::VariantType* value = archive->GetVariant(key);
    if (value == nullptr)
    {
        DVASSERT(false && "required value not found");
        return DAVA::VariantType();
    }
    return *value;
}

DAVA::String PreferencesStorage::GenerateKey(const DAVA::InspInfo* inspInfo)
{
    DVASSERT(nullptr != inspInfo);

    std::hash<DAVA::String> hashFn;
    size_t hash = hashFn(DAVA::String(inspInfo->Name().c_str()));
    for (int i = 0, count = inspInfo->MembersCount(); i < count; ++i)
    {
        size_t hash2 = hashFn(DAVA::String(inspInfo->Member(i)->Name().c_str()));
        hash = hash2 ^ (hash << 1);
    }
    return std::to_string(hash);
}

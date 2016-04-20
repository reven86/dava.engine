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

#include "Base/BaseTypes.h"
#include "Math/AABBox3.h"

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
    , unnamedPreferencesKey("unnamed preferences")
    , preferencesKey("preferences")
{
    editorPreferences->SetArchive(unnamedPreferencesKey, new DAVA::KeyedArchive());
    editorPreferences->SetArchive(preferencesKey, new DAVA::KeyedArchive());
    unnamedPreferencesArchive = editorPreferences->GetArchive(unnamedPreferencesKey);
    preferencesArchive = editorPreferences->GetArchive(preferencesKey);
}

PreferencesStorage::~PreferencesStorage() = default;

void PreferencesStorage::RegisterType(const DAVA::InspInfo* inspInfo, const DefaultValuesList& defaultValues)
{
    PreferencesStorage* self = Instance();
    RegisteredIntrospection& registeredInsp = self->registeredInsp;
    if (std::find(registeredInsp.begin(), registeredInsp.end(), inspInfo) == registeredInsp.end())
    {
        registeredInsp.push_back(inspInfo);
        self->defaultValues[inspInfo] = defaultValues;
    }
}

void PreferencesStorage::SetupStoragePath(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage_)
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

    DAVA::KeyedArchive* loadedUnnamedPreferences = loadedPreferences->GetArchive(unnamedPreferencesKey);
    if (nullptr != loadedUnnamedPreferences)
    {
        for (const auto& obj : loadedUnnamedPreferences->GetArchieveData())
        {
            unnamedPreferencesArchive->SetVariant(obj.first, *obj.second);
        }
    }
    DAVA::KeyedArchive* loadedPreferencesArchive = loadedPreferences->GetArchive(preferencesKey);
    if (nullptr != loadedPreferencesArchive)
    {
        for (const auto& obj : loadedPreferencesArchive->GetArchieveData())
        {
            preferencesArchive->SetVariant(obj.first, *obj.second);
        }
    }
    for (const DAVA::InspInfo* inspInfo : registeredInsp) //create new settings archive
    {
        const DefaultValuesList& defaultValuesList = defaultValues[inspInfo];
        DAVA::String key = GenerateKey(inspInfo);
        DAVA::KeyedArchive* loadedData = preferencesArchive->GetArchive(key);
        DAVA::ScopedPtr<DAVA::KeyedArchive> classInfoArchive(new DAVA::KeyedArchive);
        for (int i = 0, count = inspInfo->MembersCount(); i < count; ++i)
        {
            const DAVA::InspMember* member = inspInfo->Member(i);
            if ((member->Flags() & DAVA::I_PREFERENCE) == 0)
            {
                continue;
            }
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
                auto iter = defaultValuesList.find(memberName);
                if (iter != defaultValuesList.end())
                {
                    value = iter->second;
                }
                else
                {
                    DAVA::StringStream ss;
                    ss << "no default value for insp member: " << inspInfo->Name().c_str() << " : " << memberName.c_str();
                    DVASSERT_MSG(false, ss.str().c_str());
                }
            }
            DVASSERT(value.type != DAVA::VariantType::TYPE_NONE)
            classInfoArchive->SetVariant(memberName.c_str(), value);
        }
        if (classInfoArchive->Count() != 0)
        {
            preferencesArchive->SetArchive(key, classInfoArchive);
        }
    }
}

void PreferencesStorage::RegisterPreferences(void* realObj, DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);
    const DAVA::InspInfo* info = inspBase->GetTypeInfo();
    registeredObjects[info].insert(realObj);
    DAVA::String key = GenerateKey(info);
    DAVA::KeyedArchive* archive = preferencesArchive->GetArchive(key, nullptr);
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
            DVASSERT(false && "Preference not exist in archive!")
            continue;
        }
        DAVA::VariantType* value = archive->GetVariant(name);
        if (value != nullptr && value->GetType() != DAVA::VariantType::TYPE_NONE)
        {
            member->SetValue(realObj, *value);
        }
    }
}

void PreferencesStorage::UnregisterPreferences(void* realObj, const DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);
    const DAVA::InspInfo* info = inspBase->GetTypeInfo();
    DVVERIFY(registeredObjects[info].erase(realObj) > 0);
    DAVA::ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive);

    for (int i = 0, count = info->MembersCount(); i < count; ++i)
    {
        const DAVA::InspMember* member = info->Member(i);
        if ((member->Flags() & DAVA::I_PREFERENCE) != DAVA::I_PREFERENCE)
        {
            continue;
        }
        DAVA::String name(member->Name().c_str());
        DAVA::VariantType value = member->Value(realObj);
        archive->SetVariant(name, value);
        ValueChanged.Emit(info, member, value);
    }

    DAVA::String key = GenerateKey(inspBase->GetTypeInfo());
    preferencesArchive->SetArchive(key, archive);
}

void PreferencesStorage::SaveValueByKey(const DAVA::FastName& key, const DAVA::VariantType& value)
{
    unnamedPreferencesArchive->SetVariant(key.c_str(), value);
}

DAVA::VariantType PreferencesStorage::LoadValueByKey(const DAVA::FastName& key)
{
    DAVA::VariantType* loadedValutPtr = unnamedPreferencesArchive->GetVariant(key.c_str());
    if (nullptr != loadedValutPtr)
    {
        return *loadedValutPtr;
    }
    return DAVA::VariantType();
}

const DAVA::InspInfo* PreferencesStorage::GetInspInfo(const DAVA::FastName& className) const
{
    for (const DAVA::InspInfo* info : registeredInsp)
    {
        if (info->Name() == className)
        {
            return info;
        }
    }
    return nullptr;
}

void PreferencesStorage::SetNewValueToAllRegisteredObjects(const DAVA::InspInfo* inspInfo, const DAVA::InspMember* member, const DAVA::VariantType& value)
{
    DVASSERT(nullptr != inspInfo && nullptr != member);

    DAVA::KeyedArchive* archive = preferencesArchive->GetArchive(GenerateKey(inspInfo));
    DVASSERT(nullptr != archive);

    archive->SetVariant(member->Name().c_str(), value);

    auto findIter = registeredObjects.find(inspInfo);
    if (findIter != registeredObjects.end())
    {
        const auto& registeredObjects = findIter->second;
        for (void* registeredObject : registeredObjects)
        {
            member->SetValue(registeredObject, value);
            ValueChanged.Emit(inspInfo, member, value);
            break;
        }
    }
}

DAVA::VariantType PreferencesStorage::GetPreferencesValue(const DAVA::InspMember* member) const
{
    if (member == nullptr)
    {
        return DAVA::VariantType();
    }
    const DAVA::InspInfo* inspInfo = member->GetParentInsp();
    DVASSERT(nullptr != inspInfo);
    auto key = GenerateKey(inspInfo);
    PreferencesStorage* self = Instance();
    DAVA::KeyedArchive* archive = self->preferencesArchive->GetArchive(key, nullptr);
    if (archive == nullptr)
    {
        DVASSERT(false && "introspection are not registered in preferences storage");
        return DAVA::VariantType();
    }
    DAVA::VariantType* value = archive->GetVariant(member->Name().c_str());
    if (value == nullptr)
    {
        DVASSERT(false && "required value not found");
        return DAVA::VariantType();
    }
    return *value;
}

const PreferencesStorage::RegisteredIntrospection& PreferencesStorage::GetRegisteredInsp() const
{
    return registeredInsp;
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

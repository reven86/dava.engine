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
#include <locale>

PreferencesStorage* PreferencesStorage::self = nullptr;

PreferencesStorage::PreferencesStorage(const DAVA::FilePath& defaultStorage, const DAVA::FilePath& localStorage_)
    : localStorage(localStorage_)
    , editorPreferences(new DAVA::KeyedArchive())
    , preferencesToSave(new DAVA::KeyedArchive())
{
    DVASSERT(nullptr == self);
    self = this;
    if (defaultStorage.Exists())
    {
        if (!editorPreferences->Load(defaultStorage))
        {
            DVASSERT(false && "failed to load editor preferences from default storage");
        }
    }
    if (localStorage.Exists())
    {
        if (!editorPreferences->Load(localStorage))
        {
            DVASSERT(false && "faild to load editor preferences from local storage");
        }
    }
}

PreferencesStorage::~PreferencesStorage()
{
    if (!preferencesToSave->Save(localStorage))
    {
        DAVA::Logger::Error("can not save editor preferences!");
    }
    self = nullptr;
}

void PreferencesStorage::RegisterPreferences(DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);
    if (self == nullptr)
    {
        DVASSERT(false && "can not register preferences without PreferencesStorage!");
        return;
    }
    self->RegisterPreferencesImpl(inspBase);
}

void PreferencesStorage::UnregisterPreferences(const DAVA::InspBase* inspBase)
{
    if (self == nullptr)
    {
        DVASSERT(false && "can not unregister preferences without PreferencesStorage!");
        return;
    }
    self->UnregisterPreferencesImpl(inspBase);
}

void PreferencesStorage::RegisterPreferencesImpl(DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);
    const DAVA::InspInfo* info = inspBase->GetTypeInfo();
    DAVA::String key = GenerateKey(inspBase->GetTypeInfo());
    if (!editorPreferences->IsKeyExists(key))
    {
        return;
    }
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
        if (value != nullptr)
        {
            member->SetValue(inspBase, *value);
        }
    }
}

void PreferencesStorage::UnregisterPreferencesImpl(const DAVA::InspBase* inspBase)
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
        archive->SetVariant(name, member->Value(const_cast<void*>(static_cast<const void*>(inspBase)))); //SUDDENLY! current version not support Value by const pointer
    }
    DAVA::String key = GenerateKey(inspBase->GetTypeInfo());
    editorPreferences->SetArchive(key, archive);
    preferencesToSave->SetArchive(key, archive);
}

DAVA::String PreferencesStorage::GenerateKey(const DAVA::InspInfo* inspInfo) const
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

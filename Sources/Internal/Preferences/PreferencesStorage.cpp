#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Preferences/PreferencesStorage.h"

#include "Base/BaseTypes.h"

PreferencesStorage::PreferencesStorage()
    : editorPreferences(new DAVA::KeyedArchive())
    , inspPreferencesKey("preferences")
    , keyedPreferencesKey("unnamed preferences")
{
    DAVA::ScopedPtr<DAVA::KeyedArchive> dummyArchive(new DAVA::KeyedArchive());

    editorPreferences->SetArchive(keyedPreferencesKey, dummyArchive); //will copy dummy archive
    editorPreferences->SetArchive(inspPreferencesKey, dummyArchive); //will copy dummy archive
    keyedPreferencesArchive = editorPreferences->GetArchive(keyedPreferencesKey);
    inspPreferencesArchive = editorPreferences->GetArchive(inspPreferencesKey);
}

void PreferencesStorage::RegisterType(const DAVA::InspInfo* inspInfo, const DefaultValuesList& defaultValues)
{
    PreferencesStorage* self = Instance();
    RegisteredIntrospection& registeredInsp = self->registeredInsp;
    if (std::find(registeredInsp.begin(), registeredInsp.end(), inspInfo) == registeredInsp.end())
    {
        registeredInsp.push_back(inspInfo);
        self->defaultValues[inspInfo] = defaultValues;
    }
    else
    {
        DAVA::StringStream ss;
        ss << "introspection " << inspInfo->Name().c_str() << "already registered!";
        DVASSERT(false, ss.str().c_str());
    }
}

void PreferencesStorage::SetupStoragePath(const DAVA::FilePath& localStorage_)
{
    localStorage = localStorage_;
    DAVA::ScopedPtr<DAVA::KeyedArchive> loadedPreferences(new DAVA::KeyedArchive());

    if (localStorage.Exists())
    {
        if (!loadedPreferences->Load(localStorage))
        {
            DAVA::Logger::Error("faild to load editor preferences from local storage");
        }
    }
    //all preferences must be registered on app start

    DAVA::KeyedArchive* loadedUnnamedPreferences = loadedPreferences->GetArchive(keyedPreferencesKey);
    if (nullptr != loadedUnnamedPreferences)
    {
        for (const auto& obj : loadedUnnamedPreferences->GetArchieveData())
        {
            keyedPreferencesArchive->SetVariant(obj.first, *obj.second);
        }
    }
    DAVA::KeyedArchive* loadedPreferencesArchive = loadedPreferences->GetArchive(inspPreferencesKey);
    if (nullptr != loadedPreferencesArchive)
    {
        for (const auto& obj : loadedPreferencesArchive->GetArchieveData())
        {
            inspPreferencesArchive->SetVariant(obj.first, *obj.second);
        }
    }
    for (const DAVA::InspInfo* inspInfo : registeredInsp) //create new settings archive
    {
        const DefaultValuesList& defaultValuesList = defaultValues[inspInfo];
        DAVA::String key = GenerateKey(inspInfo);
        DAVA::KeyedArchive* loadedData = inspPreferencesArchive->GetArchive(key);
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
                    value = DAVA::VariantType::FromType(member->Type());
                    if (value.type == DAVA::VariantType::TYPE_NONE)
                    {
                        DAVA::StringStream ss;
                        ss << "no default variant for member: " << inspInfo->Name().c_str() << " : " << memberName.c_str();
                        DVASSERT(false, ss.str().c_str());
                    }
                }
            }
            classInfoArchive->SetVariant(memberName.c_str(), value);
        }
        if (classInfoArchive->Count() != 0)
        {
            inspPreferencesArchive->SetArchive(key, classInfoArchive);
        }
    }
}

void PreferencesStorage::RegisterPreferences(void* realObj, DAVA::InspBase* inspBase)
{
    DVASSERT(nullptr != inspBase);
    const DAVA::InspInfo* info = inspBase->GetTypeInfo();
    registeredObjects[info].insert(realObj);
    DAVA::String key = GenerateKey(info);
    DAVA::KeyedArchive* archive = inspPreferencesArchive->GetArchive(key, nullptr);
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
            DVASSERT(false && "Preference not exist in archive!");
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

    const size_t erasedCount = registeredObjects[info].erase(realObj);
    DVASSERT(erasedCount > 0);
    DAVA::ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive);

    for (int i = 0, count = info->MembersCount(); i < count; ++i)
    {
        const DAVA::InspMember* member = info->Member(i);
        if ((member->Flags() & DAVA::I_PREFERENCE) != DAVA::I_PREFERENCE)
        {
            continue;
        }
        DAVA::String name(member->Name().c_str());
        DAVA::VariantType* oldValue = archive->GetVariant(name);
        DAVA::VariantType value = member->Value(realObj);
        if (oldValue == nullptr || *oldValue != value)
        {
            archive->SetVariant(name, value);
            valueChanged.Emit(member, value);
        }
    }

    DAVA::String key = GenerateKey(info);
    inspPreferencesArchive->SetArchive(key, archive);
    Save();
}

void PreferencesStorage::SetValue(const DAVA::FastName& key, const DAVA::VariantType& value)
{
    keyedPreferencesArchive->SetVariant(key.c_str(), value);
    Save();
}

DAVA::VariantType PreferencesStorage::GetValue(const DAVA::FastName& key)
{
    DAVA::VariantType* loadedValutPtr = keyedPreferencesArchive->GetVariant(key.c_str());
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

void PreferencesStorage::SetValue(const DAVA::InspMember* member, const DAVA::VariantType& value)
{
    const DAVA::InspInfo* inspInfo = member->GetParentInsp();
    DVASSERT(nullptr != inspInfo && nullptr != member);

    DAVA::KeyedArchive* archive = inspPreferencesArchive->GetArchive(GenerateKey(inspInfo));
    DVASSERT(nullptr != archive);

    archive->SetVariant(member->Name().c_str(), value);
    Save();

    auto findIter = registeredObjects.find(inspInfo);
    if (findIter != registeredObjects.end())
    {
        const auto& registeredObjects = findIter->second;
        for (void* registeredObject : registeredObjects)
        {
            member->SetValue(registeredObject, value);
        }
    }
    valueChanged.Emit(member, value);
}

DAVA::VariantType PreferencesStorage::GetValue(const DAVA::InspMember* member) const
{
    if (member == nullptr)
    {
        DVASSERT(false);
        return DAVA::VariantType();
    }
    const DAVA::InspInfo* inspInfo = member->GetParentInsp();
    DVASSERT(nullptr != inspInfo);
    auto key = GenerateKey(inspInfo);
    PreferencesStorage* self = Instance();
    DAVA::KeyedArchive* archive = self->inspPreferencesArchive->GetArchive(key, nullptr);
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
    DVASSERT(value->Meta() == member->Type());
    return *value;
}

DAVA::VariantType PreferencesStorage::GetDefaultValue(const DAVA::InspMember* member) const
{
    if (member == nullptr)
    {
        return DAVA::VariantType();
    }
    const DAVA::InspInfo* inspInfo = member->GetParentInsp();
    DVASSERT(nullptr != inspInfo);
    auto mapIter = defaultValues.find(inspInfo);
    if (mapIter == defaultValues.end())
    {
        //unregistered introspection. Looks like we trying to get non-preferences membrer or forget to register it
        DVASSERT(mapIter != defaultValues.end());
        return DAVA::VariantType();
    }
    const DefaultValuesList& defaultValuesList = mapIter->second;
    const DAVA::FastName& name = member->Name();
    auto defaultValuesIter = defaultValuesList.find(name);
    if (defaultValuesIter == defaultValuesList.end())
    {
        //this is normal case, default value may noty exists
        return DAVA::VariantType();
    }
    return defaultValuesIter->second;
}

const PreferencesStorage::RegisteredIntrospection& PreferencesStorage::GetRegisteredInsp() const
{
    return registeredInsp;
}

bool PreferencesStorage::Save() const
{
    if (!localStorage.IsEmpty())
    {
        return editorPreferences->Save(localStorage);
    }
    return false;
}

DAVA::String PreferencesStorage::GenerateKey(const DAVA::InspInfo* inspInfo)
{
    return inspInfo->Name().c_str();
}

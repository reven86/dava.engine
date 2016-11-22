#pragma once

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FilePath.h"
#include "Functional/Signal.h"
#include "Base/StaticSingleton.h"

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

    //register introspection on start
    void RegisterType(const DAVA::InspInfo* inspInfo, const DefaultValuesList& defaultValues = DefaultValuesList());

    //register object when it created or initialized
    template <typename T>
    void RegisterPreferences(T* obj);

    //unregister object on desctruction to save it state
    template <typename T>
    void UnregisterPreferences(T* obj);

    void SetupStoragePath(const DAVA::FilePath& localStorage);

    //getter and setter to use preferences manually without introspections. Desired for namespaces and local functions;
    DAVA::VariantType GetValue(const DAVA::FastName& key);
    void SetValue(const DAVA::FastName& key, const DAVA::VariantType& value);

    const DAVA::InspInfo* GetInspInfo(const DAVA::FastName& className) const;

    //getter and setter to use preferences by introspection member
    DAVA::VariantType GetValue(const DAVA::InspMember* member) const;
    //when set value by member it saves as preferences value and applys to all registered objects
    void SetValue(const DAVA::InspMember* member, const DAVA::VariantType& value);

    //get default value;
    //if default value was not set - empty variant type will be returned
    DAVA::VariantType GetDefaultValue(const DAVA::InspMember* member) const;

    //all registered InspInfo to build model
    const RegisteredIntrospection& GetRegisteredInsp() const;

    bool Save() const; //saves preferences to the storage path

    DAVA::Signal<const DAVA::InspMember*, const DAVA::VariantType&> valueChanged;

private:
    void RegisterPreferences(void* realObj, DAVA::InspBase* inspBase);
    void UnregisterPreferences(void* realObj, const DAVA::InspBase* inspBase);

    static DAVA::String GenerateKey(const DAVA::InspInfo* inspInfo);

    DAVA::FilePath localStorage;
    DAVA::ScopedPtr<DAVA::KeyedArchive> editorPreferences;

    RegisteredIntrospection registeredInsp;
    DAVA::Map<const DAVA::InspInfo*, DefaultValuesList> defaultValues;
    DAVA::Map<const DAVA::InspInfo*, DAVA::Set<void*>> registeredObjects;

    //two different storages: for introspection properties and for manually created preferences
    DAVA::KeyedArchive* inspPreferencesArchive = nullptr;
    DAVA::KeyedArchive* keyedPreferencesArchive = nullptr;
    const DAVA::String inspPreferencesKey;
    const DAVA::String keyedPreferencesKey;
};

template <typename T>
void PreferencesStorage::RegisterPreferences(T* realObject)
{
    static_assert(std::is_base_of<DAVA::InspBase, T>::value, "type T must be derived from InspBase");
    Instance()->RegisterPreferences(realObject, static_cast<DAVA::InspBase*>(realObject));
}

template <typename T>
void PreferencesStorage::UnregisterPreferences(T* realObject)
{
    static_assert(std::is_base_of<DAVA::InspBase, T>::value, "type T must be derived from InspBase");
    Instance()->UnregisterPreferences(realObject, static_cast<DAVA::InspBase*>(realObject));
}

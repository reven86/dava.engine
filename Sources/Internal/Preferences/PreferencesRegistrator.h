#pragma once

#include "Preferences/PreferencesStorage.h"

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

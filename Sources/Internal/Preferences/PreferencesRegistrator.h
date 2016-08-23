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

//use this macro to register introspection when program starts. A PreferencesStorage require this to work correctly
#define REGISTER_PREFERENCES_ON_START(Class, ...) \
    namespace Class##_local \
    { \
    InspInfoRegistrator inspInfoRegistrator(Class::TypeInfo(), { __VA_ARGS__ }); \
    };

//use this macro with macro REGISTER_PREFERENCES_ON_START(Class, PREF_ARG("name", true))
#define PREF_ARG(name, value) \
    { DAVA::FastName(name), DAVA::VariantType(value) }

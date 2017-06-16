#include "Modules/PreferencesModule/PreferencesData.h"

#include <Preferences/PreferencesStorage.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PreferencesData)
{
    DAVA::ReflectionRegistrator<PreferencesData>::Begin()
    .Field(guidesEnabledPropertyName.c_str(), &PreferencesData::IsGuidesEnabled, &PreferencesData::SetGuidesEnabled)
    .End();
}

bool PreferencesData::IsGuidesEnabled() const
{
    using namespace DAVA;
    PreferencesStorage* storage = PreferencesStorage::Instance();
    VariantType currentValue = storage->GetValue(guidesEnabledPropertyName);
    if (currentValue.type != VariantType::TYPE_BOOLEAN)
    {
        return true;
    }
    return currentValue.AsBool();
}

void PreferencesData::SetGuidesEnabled(bool value)
{
    using namespace DAVA;
    PreferencesStorage* storage = PreferencesStorage::Instance();
    storage->SetValue(guidesEnabledPropertyName, VariantType(value));
}

DAVA::FastName PreferencesData::guidesEnabledPropertyName{ "guides enabled" };

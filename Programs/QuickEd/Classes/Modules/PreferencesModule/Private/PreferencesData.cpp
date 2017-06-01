#include "Modules/PreferencesModule/PreferencesData.h"

#include <Preferences/PreferencesStorage.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PreferencesData)
{
    DAVA::ReflectionRegistrator<PreferencesData>::Begin()
    .Field(guidesEnabledPropertyName.c_str(), &PreferencesData::IsGuidesEnabled, &PreferencesData::SetGuidesEnabled)
    .End();
}

namespace PreferencesDataDetails
{
const DAVA::FastName guidesEnabledSettingsKey{ "guides enabled" };
}

bool PreferencesData::IsGuidesEnabled() const
{
    using namespace DAVA;
    PreferencesStorage* storage = PreferencesStorage::Instance();
    VariantType currentValue = storage->GetValue(PreferencesDataDetails::guidesEnabledSettingsKey);
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
    storage->SetValue(PreferencesDataDetails::guidesEnabledSettingsKey, VariantType(value));
}

DAVA::FastName PreferencesData::guidesEnabledPropertyName{ "guides enabled" };

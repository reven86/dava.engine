#include "Base/Introspection.h"

#include "Preferences/PreferencesRegistrator.h"
#include "Preferences/PreferencesStorage.h"

InspInfoRegistrator::InspInfoRegistrator(const DAVA::InspInfo* inspInfo, const PreferencesStorage::DefaultValuesList& defaultValues)
{
    PreferencesStorage::Instance()->RegisterType(inspInfo, defaultValues);
}

GlobalValuesRegistrator::GlobalValuesRegistrator(const DAVA::FastName& key, const DAVA::VariantType& defaultValue)
{
    PreferencesStorage* storage = PreferencesStorage::Instance();
    DAVA::VariantType currentValue = storage->GetValue(key);
    if (currentValue.type == DAVA::VariantType::TYPE_NONE)
    {
        storage->SetValue(key, defaultValue);
    }
}

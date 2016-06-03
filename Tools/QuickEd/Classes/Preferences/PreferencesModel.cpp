#include "Preferences/PreferencesModel.h"
#include "Preferences/PreferencesRootProperty.h"

PreferencesModel::PreferencesModel(QObject* parent)
    : PropertiesModel(parent)
{
    rootProperty = new PreferencesRootProperty();
}

void PreferencesModel::ApplyAllChangedProperties()
{
    DAVA::DynamicTypeCheck<PreferencesRootProperty*>(rootProperty)->ApplyPreference();
}

void PreferencesModel::ChangeProperty(AbstractProperty* property, const DAVA::VariantType& value)
{
    property->SetValue(value);
}

void PreferencesModel::ResetProperty(AbstractProperty* property)
{
    property->ResetValue();
}

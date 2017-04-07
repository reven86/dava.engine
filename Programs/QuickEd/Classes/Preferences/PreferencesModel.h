#pragma once

#include "UI/Properties/PropertiesModel.h"

class PreferencesModel : public PropertiesModel
{
public:
    PreferencesModel(QObject* parent);

    void ApplyAllChangedProperties();

protected:
    void ChangeProperty(AbstractProperty* property, const DAVA::Any& value) override;
    void ResetProperty(AbstractProperty* property) override;
};

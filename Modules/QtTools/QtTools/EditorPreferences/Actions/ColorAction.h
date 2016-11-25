#pragma once

#include "QtTools/EditorPreferences/Actions/AbstractAction.h"

class ColorAction : public AbstractAction
{
public:
    ColorAction(const DAVA::InspMember* member, QObject* parent);

private:
    void OnTriggered(bool triggered) override;
    void OnValueChanged(const DAVA::VariantType& value) override;
};

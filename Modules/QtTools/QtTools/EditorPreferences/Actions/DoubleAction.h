#pragma once

#include "QtTools/EditorPreferences/Actions/AbstractAction.h"

class DoubleAction : public AbstractAction
{
public:
    DoubleAction(const DAVA::InspMember* member, QObject* parent);

private:
    void OnTriggered(bool triggered) override;
    void OnValueChanged(const DAVA::VariantType& value) override;
};

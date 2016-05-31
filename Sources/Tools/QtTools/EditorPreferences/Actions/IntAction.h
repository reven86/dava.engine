#pragma once

#include "QtTools/EditorPreferences/Actions/AbstractAction.h"

class IntAction : public AbstractAction
{
public:
    IntAction(const DAVA::InspMember* member, QObject* parent);

private:
    void OnTriggered(bool triggered) override;
    void OnValueChanged(const DAVA::VariantType& value) override;
};

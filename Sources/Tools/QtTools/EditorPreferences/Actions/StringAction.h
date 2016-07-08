#pragma once

#include "QtTools/EditorPreferences/Actions/AbstractAction.h"

class StringAction : public AbstractAction
{
public:
    StringAction(const DAVA::InspMember* member, QObject* parent);

private:
    void OnTriggered(bool triggered) override;
    void OnValueChanged(const DAVA::VariantType& value) override;
};

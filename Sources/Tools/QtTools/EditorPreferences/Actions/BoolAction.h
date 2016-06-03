#pragma once

#include "EditorPreferences/Actions/AbstractAction.h"

class BoolAction final : public AbstractAction
{
public:
    BoolAction(const DAVA::InspMember* member_, QObject* parent);

    void Init() override;

private:
    void OnValueChanged(const DAVA::VariantType& value) override;
    void OnTriggered(bool triggered) override;
};

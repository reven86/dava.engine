#include "QtTools/EditorPreferences/Actions/BoolAction.h"
#include "Preferences/PreferencesStorage.h"

BoolAction::BoolAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void BoolAction::Init()
{
    AbstractAction::Init();
    setCheckable(true);
}

void BoolAction::OnValueChanged(const DAVA::VariantType& value)
{
    setChecked(value.AsBool());
}

void BoolAction::OnTriggered(bool triggered)
{
    PreferencesStorage::Instance()->SetValue(member, DAVA::VariantType(triggered));
}

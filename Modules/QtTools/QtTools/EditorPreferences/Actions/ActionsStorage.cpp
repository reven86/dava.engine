#include "QtTools/EditorPreferences/Actions/ActionsStorage.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/Actions/AbstractAction.h"
#include "FileSystem/VariantType.h"

ActionsStorage::ActionsStorage()
{
    PreferencesStorage::Instance()->valueChanged.Connect(this, &ActionsStorage::OnPreferencesValueChanged);
}

void ActionsStorage::OnPreferencesValueChanged(const DAVA::InspMember* member, const DAVA::VariantType& value)
{
    for (AbstractAction* action : registeredActions[member])
    {
        action->OnValueChanged(value);
    }
}

void ActionsStorage::RegisterAction(AbstractAction* action)
{
    registeredActions[action->member].insert(action);
}

void ActionsStorage::UnregisterAction(AbstractAction* action)
{
    const size_t erasedCount = registeredActions[action->member].erase(action);
    DVASSERT(erasedCount > 0);
}
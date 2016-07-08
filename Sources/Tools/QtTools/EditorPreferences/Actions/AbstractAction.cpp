#include "QtTools/EditorPreferences/Actions/AbstractAction.h"
#include "QtTools/EditorPreferences/Actions/ActionsStorage.h"
#include "Preferences/PreferencesStorage.h"

AbstractAction::AbstractAction(const DAVA::InspMember* member_, QObject* parent)
    : QAction(parent)
    , member(member_)
{
    ActionsStorage::Instance()->RegisterAction(this);
    setText(member->Desc().text);
}

AbstractAction::~AbstractAction()
{
    ActionsStorage::Instance()->UnregisterAction(this);
}

void AbstractAction::Init()
{
    DAVA::VariantType defaultValue = PreferencesStorage::Instance()->GetValue(member);
    OnValueChanged(defaultValue);
    type = defaultValue.type;

    connect(this, &QAction::triggered, this, &AbstractAction::OnTriggered);
}

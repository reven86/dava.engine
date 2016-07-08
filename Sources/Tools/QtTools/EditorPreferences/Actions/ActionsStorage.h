#pragma once
#include "Functional/Signal.h"
#include "Base/Singleton.h"

namespace DAVA
{
class InspMember;
}
class AbstractAction;

class ActionsStorage : public DAVA::TrackedObject, public DAVA::StaticSingleton<ActionsStorage>
{
public:
    ActionsStorage();

    void OnPreferencesValueChanged(const DAVA::InspMember* member, const DAVA::VariantType& value);

    void RegisterAction(AbstractAction* action);
    void UnregisterAction(AbstractAction* action);

private:
    DAVA::UnorderedMap<const DAVA::InspMember*, DAVA::Set<AbstractAction*>> registeredActions;
};
#pragma once
#include "Base/BaseTypes.h"

class CommandStack;

namespace wgt
{
class IEnvManager;
}

class CommandStackGroup
{
public:
    CommandStackGroup();
    void RemoveStack(CommandStack* stackToRemove);
    void AddStack(CommandStack* stackToAdd);

    void SetActiveStack(CommandStack* commandStack);

private:
    CommandStack* activeStack = nullptr;
    DAVA::Set<CommandStack*> stacks;
    wgt::IEnvManager* envManager = nullptr;
};

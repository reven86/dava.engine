#include "Command/Private/CommandsHolder.h"
#include "Command/Command.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
bool CommandsHolder::Add(std::unique_ptr<Command>&& command)
{
    DVASSERT(command);
    if (commands.empty() == false)
    {
        DAVA::Command* lastCommand = commands.back().get();
        const int32 id = lastCommand->GetID();
        DVASSERT(id != COMMAND_BATCH, "we can not store batch inside another batch");
        if (id == command->GetID())
        {
            if (lastCommand->MergeWith(command.get()))
            {
                return false;
            }
        }
    }
    commands.push_back(std::move(command));
    return true;
}
} //namespace DAVA

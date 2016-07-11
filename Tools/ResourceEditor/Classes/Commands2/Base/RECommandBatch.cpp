#include "Commands2/Base/RECommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "Commands2/Base/RECommand.h"

RECommandBatch::RECommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount)
    : CommandBatch(text, commandsCount)
{
}

RECommand* RECommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
    {
        DAVA::Command* cmd = commandList.at(index).get();
        return DAVA::DynamicTypeCheck<RECommand*>(cmd);
    }

    DVASSERT_MSG(false, DAVA::Format("index %u, size %u", index, static_cast<DAVA::uint32>(commandList.size())).c_str());
    return nullptr;
}

void RECommandBatch::RemoveCommands(DAVA::CommandID_t commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Pointer& cmd) {
        return cmd->GetID() == commandId;
    });

    commandList.erase(it, commandList.end());
    commandIDs.erase(commandId);
}

bool RECommandBatch::IsMultiCommandBatch() const
{
    return commandIDs.size() > 1;
}

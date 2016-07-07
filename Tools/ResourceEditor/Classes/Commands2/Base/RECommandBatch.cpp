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

void RECommandBatch::RemoveCommands(DAVA::int32 commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Pointer& cmd) {
        return cmd->GetID() == commandId;
    });

    commandList.erase(it, commandList.end());
    commandIDs.erase(commandId);
}

bool RECommandBatch::MatchCommandID(DAVA::int32 commandId) const
{
    return commandIDs.count(commandId) > 0;
}

bool RECommandBatch::MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIdVector) const
{
    for (auto id : commandIdVector)
    {
        if (MatchCommandID(id))
        {
            return true;
        }
    }

    return false;
}

bool RECommandBatch::IsMultiCommandBatch() const
{
    return commandIDs.size() > 1;
}

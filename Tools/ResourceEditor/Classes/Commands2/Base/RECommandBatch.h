#pragma once

#include "Base/BaseTypes.h"
#include "Command/CommandBatch.h"

class CommandWithoutExecute;

class RECommandBatch final : public DAVA::CommandBatch
{
public:
    RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount);

    void RemoveCommands(DAVA::CommandID_t commandId);

    CommandWithoutExecute* GetCommand(DAVA::uint32 index) const;

    bool IsMultiCommandBatch() const;
};

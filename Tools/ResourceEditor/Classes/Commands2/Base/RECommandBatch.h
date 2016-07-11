#pragma once

#include "Base/BaseTypes.h"
#include "Command/CommandBatch.h"

class RECommand;

class RECommandBatch final : public DAVA::CommandBatch
{
public:
    RECommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount);

    void RemoveCommands(DAVA::CommandID_t commandId);

    RECommand* GetCommand(DAVA::uint32 index) const;

    bool IsMultiCommandBatch() const;
};

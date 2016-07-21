#pragma once

#include "Base/BaseTypes.h"
#include "Command/CommandBatch.h"
#include "Commands2/Base/RECommand.h"

class RECommand;

class RECommandBatch final : public DAVA::CommandBatch, public RECommand
{
public:
    RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount);

    void AddAndRedo(Pointer&& command) override;

    void RemoveCommands(DAVA::uint32 commandId);

    RECommand* GetCommand(DAVA::uint32 index) const;

    bool IsMultiCommandBatch() const;

    bool MatchCommandID(DAVA::uint32 commandID) const override;

private:
    UnorderedSet<DAVA::uint32> commandIDs;
};

#pragma once

#include "Base/BaseTypes.h"
#include "Command/CommandBatch.h"

class RECommand;

class RECommandBatch final : public DAVA::CommandBatch
{
public:
    RECommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount);

    void RemoveCommands(DAVA::int32 commandId);

    RECommand* GetCommand(DAVA::uint32 index) const;

    bool MatchCommandID(DAVA::int32 commandID) const;
    bool MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const;

    bool IsMultiCommandBatch() const;
};

#pragma once

#include "Functional/Function.h"

#include "Commands2/Base/RECommand.h"
#include "Commands2/Base/RECommandBatch.h"

class RECommandNotificationObject
{
public:
    bool IsEmpty() const;
    void ExecuteForAllCommands(const DAVA::Function<void(const RECommand*, bool)>& fn) const;

    bool MatchCommandID(DAVA::uint32 commandID) const;
    bool MatchCommandIDs(const DAVA::Vector<DAVA::uint32>& commandIDVector) const;

    const RECommand* command = nullptr;
    const RECommandBatch* batch = nullptr;
    bool redo = true;
};

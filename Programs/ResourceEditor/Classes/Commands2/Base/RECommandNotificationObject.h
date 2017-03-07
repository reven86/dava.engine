#pragma once

#include "Commands2/Base/RECommand.h"
#include "Commands2/Base/RECommandBatch.h"

#include <Functional/Function.h>

class RECommandNotificationObject
{
public:
    bool IsEmpty() const;
    void ExecuteForAllCommands(const DAVA::Function<void(const RECommand*, bool)>& fn) const;

    bool MatchCommandID(DAVA::CommandID commandID) const;
    bool MatchCommandIDs(const DAVA::Vector<DAVA::CommandID>& commandIDVector) const;

    const RECommand* command = nullptr;
    const RECommandBatch* batch = nullptr;
    bool redo = true;
};

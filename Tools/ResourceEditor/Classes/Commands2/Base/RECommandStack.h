#pragma once

#include "Base/BaseTypes.h"
#include "NgtTools/Commands/CommandStack.h"
#include "Commands2/Base/CommandNotify.h"

class RECommandStack : public CommandStack, public CommandNotifyProvider
{
public:
    RECommandStack();
    ~RECommandStack() override;

    void Clear();
    void RemoveCommands(DAVA::CommandID_t commandId);

    void Activate();
    void Undo() override;
    void Redo() override;
    void Exec(DAVA::Command::Pointer&& command) override;

    bool IsUncleanCommandExists(DAVA::CommandID_t commandId) const;

    void EndBatch() override;

    bool IsClean() const override;
    void SetClean() override;

private:
    void CleanCheck();

    DAVA::UnorderedSet<DAVA::CommandID_t> uncleanCommandIds;
};

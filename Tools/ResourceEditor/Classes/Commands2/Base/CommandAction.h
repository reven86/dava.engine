#pragma once

#include "Commands2/Base/RECommand.h"

class CommandAction : public RECommand
{
public:
    CommandAction(DAVA::CommandID_t id, const DAVA::String& text = DAVA::String());

    bool CanUndo() const override;
    void Undo() override;
};

inline void CommandAction::Undo()
{
}

inline bool CommandAction::CanUndo() const
{
    return false;
}

#ifndef __WAYEDIT_COMMANDS_H__
#define __WAYEDIT_COMMANDS_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

class EnableWayEditCommand : public CommandWithoutExecute
{
public:
    EnableWayEditCommand()
        : CommandWithoutExecute(CMDID_ENABLE_WAYEDIT, "Enable waypoint edit mode")
    {
    }
    void Undo() override
    {
    }
    void Redo() override
    {
    }
};

class DisableWayEditCommand : public CommandWithoutExecute
{
public:
    DisableWayEditCommand()
        : CommandWithoutExecute(CMDID_DISABLE_WAYEDIT, "Disable waypoint edit mode")
    {
    }
    void Undo() override
    {
    }
    void Redo() override
    {
    }
};

#endif // __WAYEDIT_COMMANDS_H__

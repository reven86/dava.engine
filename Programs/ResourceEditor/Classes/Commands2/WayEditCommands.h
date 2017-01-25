#pragma once

#include "Commands2/RECommandIDs.h"
#include "Commands2/Base/RECommand.h"

class EnableWayEditCommand : public RECommand
{
public:
    EnableWayEditCommand()
        : RECommand(CMDID_ENABLE_WAYEDIT, "Enable waypoint edit mode")
    {
    }
    void Undo() override
    {
    }
    void Redo() override
    {
    }
};

class DisableWayEditCommand : public RECommand
{
public:
    DisableWayEditCommand()
        : RECommand(CMDID_DISABLE_WAYEDIT, "Disable waypoint edit mode")
    {
    }
    void Undo() override
    {
    }
    void Redo() override
    {
    }
};

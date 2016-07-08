#ifndef __WAYEDIT_COMMANDS_H__
#define __WAYEDIT_COMMANDS_H__

#include "Commands2/Base/Command2.h"

class EnableWayEditCommand : public Command2
{
public:
    EnableWayEditCommand()
        : Command2(CMDID_ENABLE_WAYEDIT, "Enable waypoint edit mode")
    {
    }
    void Undo() override
    {
    }
    void Redo() override
    {
    }
    DAVA::Entity* GetEntity() const override
    {
        return nullptr;
    }
};

class DisableWayEditCommand : public Command2
{
public:
    DisableWayEditCommand()
        : Command2(CMDID_DISABLE_WAYEDIT, "Disable waypoint edit mode")
    {
    }
    void Undo() override
    {
    }
    void Redo() override
    {
    }
    DAVA::Entity* GetEntity() const override
    {
        return nullptr;
    }
};

#endif // __WAYEDIT_COMMANDS_H__

#pragma once

#include "Base/BaseTypes.h"
#include "Command/Command.h"
#include "Scene3D/Entity.h"

#include "Commands2/RECommandIDs.h"

class RECommand : public DAVA::Command
{
public:
    RECommand(DAVA::CommandID_t, const DAVA::String& text = "");
    ~RECommand() override;

    static Pointer CreateEmptyCommand();

    DAVA_DEPRECATED(virtual DAVA::Entity* GetEntity() const);

    void Execute() override;
};

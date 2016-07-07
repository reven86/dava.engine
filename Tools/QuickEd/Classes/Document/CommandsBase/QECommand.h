#pragma once

#include "Command/Command.h"
#include "Base/BaseTypes.h"

class QECommand : public DAVA::Command
{
public:
    QECommand(DAVA::CommandID_t id, const DAVA::String& text = "");
    void Execute() override;
};

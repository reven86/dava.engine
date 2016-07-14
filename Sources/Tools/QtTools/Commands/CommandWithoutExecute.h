#pragma once

#include "Base/BaseTypes.h"
#include "Command/Command.h"

//this class designed for projects, which do not use Execute in commands
class CommandWithoutExecute : public DAVA::Command
{
public:
    CommandWithoutExecute(DAVA::CommandID_t id, const DAVA::String& text = "");
    ~CommandWithoutExecute() override = default;

    //in our tools we do not use Execute but use Redo.
    void Execute() override;
};

#pragma once

#include <Command/Command.h>

class RECommand : public DAVA::Command
{
public:
    RECommand(DAVA::CommandID id, const DAVA::String& description = "");
    bool MatchCommandID(DAVA::CommandID commandID) const;

    //re implement pure virtual function Undo for commands which can not make Undo itself
    void Undo() override
    {
    }
};

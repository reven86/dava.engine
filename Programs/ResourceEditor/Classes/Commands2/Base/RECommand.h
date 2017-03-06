#pragma once

#include "Command/Command.h"

class RECommand : public DAVA::Command
{
public:
    RECommand(DAVA::int32 id, const DAVA::String& description = "");
    bool MatchCommandID(DAVA::int32 commandID) const;

    //re implement pure virtual function Undo for commands which can not make Undo itself
    void Undo() override
    {
    }
};

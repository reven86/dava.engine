#pragma once

#include "Command/Command.h"

class RECommand : public DAVA::Command
{
public:
    RECommand(DAVA::uint32 id, const DAVA::String& description = "");

    DAVA::uint32 GetID() const;
    virtual bool MatchCommandID(DAVA::uint32 commandID) const;
    bool MatchCommandIDs(const DAVA::Vector<DAVA::uint32>& commandIDVector) const;

private:
    const DAVA::uint32 id;
};

inline bool RECommand::MatchCommandID(DAVA::uint32 commandID) const
{
    return (id == commandID);
}

bool IsRECommand(const DAVA::Command* command)
{
    return dynamic_cast<const RECommand*>(command) != nullptr;
}

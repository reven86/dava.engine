#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Command;

class CommandsHolder
{
public:
    /**
    \brief Moves command to the batch.
    */
    bool Add(std::unique_ptr<Command>&& command);

    /**
    \brief Returns whether the batch is empty (i.e. whether its size is 0)
    \returns true if batch size is 0, false otherwise.
    */
    bool IsEmpty() const;

    /**
    \brief Returns the number of commands in the batch.
    \returns The number of commands in the batch.
    */
    uint32 Size() const;

protected:
    using CommandsContainer = Vector<std::unique_ptr<Command>>;
    CommandsContainer commands;

    CommandsHolder() = default;
    virtual ~CommandsHolder() = default;
};

inline bool CommandsHolder::IsEmpty() const
{
    return commands.empty();
}

inline uint32 CommandsHolder::Size() const
{
    return static_cast<uint32>(commands.size());
}
} //namespace DAVA

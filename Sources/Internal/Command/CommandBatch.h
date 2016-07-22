#pragma once

#include "Command/Command.h"

namespace DAVA
{
class CommandBatch : public Command
{
public:
    /**
    \brief Creates an empty command batch with required name.
    \param[in] text command batch text description to be displayed in widgets / network packets / log texts.
    \param[in] commandsCoubnt commands count to reserve memory to optimize memory allocation count.
    */
    CommandBatch(const String& description = "", uint32 commandsCount = 0);

    /**
    \brief Calls Redo to the all commands in batch.
    */
    void Redo() override;

    /**
    \brief Calls Undo to the all commands in batch in a reverse order.
    */
    void Undo() override;

    /**
    \brief Moves command to the batch and calls Execute to the moved command.
    */
    virtual void AddAndRedo(Pointer&& command);

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
    using CommandsContainer = Vector<Pointer>;
    CommandsContainer commandList;
};

inline bool CommandBatch::IsEmpty() const
{
    return commandList.empty();
}

inline uint32 CommandBatch::Size() const
{
    return static_cast<uint32>(commandList.size());
}

bool IsCommandBatch(const DAVA::Command* command);
}

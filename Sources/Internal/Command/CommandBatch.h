#pragma once

#include "Command/Command.h"
#include "Command/Private/CommandsHolder.h"

namespace DAVA
{
class CommandBatch : public Command, public CommandsHolder
{
public:
    /**
    \brief Creates an empty command batch with required name.
    \param[in] text command batch text description to be displayed in widgets / network packets / log texts.
    \param[in] commandsCoubnt commands count to reserve memory to optimize memory allocation count.
    */
    CommandBatch(const String& description = "", uint32 commandsCount = 1);

    /**
    \brief Calls Redo to the all commands in batch.
    */
    void Redo() override;

    /**
    \brief Calls Undo to the all commands in batch in a reverse order.
    */
    void Undo() override;

    /**
    \brief Moves command to the batch and calls Redo to the moved command.
    */
    void AddAndRedo(std::unique_ptr<Command>&& command);

    /**
    \brief Works the same as Command::IsClean
    \returns true if empty or contain only clean commands
    */
    bool IsClean() const override;
};

bool IsCommandBatch(const Command* command);
} //namespace DAVA

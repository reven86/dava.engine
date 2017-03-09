#pragma once

#include "Base/BaseTypes.h"
#include "Command/ICommand.h"
#include "Command/CommandIDs.h"

namespace DAVA
{
class Command : public ICommand
{
public:
    /**
    \brief Creates instance of a command base class.
    \param[in] id derived class ID, like CMDID_BATCH.
    \param[in] text command text description to be displayed in widgets / network packets / log texts.
    */
    Command(CommandID commandID, const String& description = "");

    /**
    \brief Returns command text description.
    \returns String command text description.
    */
    const String& GetDescription() const;

    /**
    \brief Returns command id.
    \returns int32 command id.
     */
    CommandID GetID() const;

    /**
    \brief Some commands passed to stack can make Redo and Undo, but do not change any files so do not change save state.
    As an example it can be selection command or command which toggle view state in the editor;
    \returns true if command change save state aka modify any files or serializable objects.
    */
    virtual bool IsClean() const;

private:
    //this function is not a part of public API and can be called only by CommandBatch
    virtual bool MergeWith(const Command* command);
    friend class CommandBatch;

    const CommandID id = INVALID_COMMAND;
    const String description;
};

inline const String& Command::GetDescription() const
{
    return description;
}

inline CommandID Command::GetID() const
{
    return id;
}

inline bool Command::IsClean() const
{
    return false;
}

inline bool Command::MergeWith(const Command* command)
{
    return false;
}
} //namespace DAVA

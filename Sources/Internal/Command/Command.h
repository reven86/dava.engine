#pragma once

#include "Base/BaseTypes.h"
#include "Command/ICommand.h"

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
    Command(const String& description = "");

    /**
    \brief Returns command's text description.
    \returns command's text description.
    */
    const String& GetDescription() const;

    /**
    \brief Some commands passed to stack can make Redo and Undo, but do not change any files so do not change save state.
    As an example it can be selection command or command which toggle view state in the editor;
    \returns true if command change save state aka modify any files or serializable objects.
    */
    virtual bool IsClean() const;

    //re implement pure virtual function Undo for commands which can not make Undo itself
    void Undo() override;

private:
    const String description;
};

inline const String& Command::GetDescription() const
{
    return description;
}

inline bool Command::IsClean() const
{
    return false;
}

inline void Command::Undo()
{
}
}

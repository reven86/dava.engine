#pragma once

#include "Base/BaseTypes.h"
#include "Command/ICommand.h"
#include "Command/CommandIDs.h"

namespace DAVA
{
class Command : public ICommand
{
public:
    using Pointer = std::unique_ptr<Command>;

protected:
    /**
    \brief Creates instance of a command base class.
    \param[in] id derived class ID, like CMDID_BATCH.
    \param[in] text command text description to be displayed in widgets / network packets / log texts.
    */
    Command(CommandID_t id, const String& text = "");

public:
    /**
    \brief Creates given instance of command with the given arguments.
    \param[in] The command arguments list.
    */
    template <typename CMD, typename... Arg>
    static std::unique_ptr<CMD> Create(Arg&&... arg);

    /**
    \brief Returns command's ID. 
    \returns the specific identificator of a class, which derived from Command; 
    */
    CommandID_t GetID() const;

    /**
    \brief Returns command's text description.
    \returns command's text description.
    */
    const String& GetText() const;

    /**
    \brief Some commands passed to stack can make Redo and Undo, but do not change any files so do not change save state.
    As an example it can be selection command or command which toggle view state in the editor;
    \returns true if command change save state aka modify any files or serializable objects.
    */
    virtual bool IsModifying() const;

    /**
    \brief Returns true if command have Undo realization.
    Some commands can only change frameworks state without making undo after that.
    \returns returns true if command can undo.
    */
    virtual bool CanUndo() const;

    /**
    \brief check that the class command ID is equal to given command ID.
    \returns returns true if command ID is equal to given command ID. Otherwise return false.
    */
    virtual bool MatchCommandID(DAVA::CommandID_t commandID) const;

    /**
    \brief check that the command is equal to the any one of given command IDs.
    \returns returns true if command command ID is equal to the any one of given command IDs. Otherwise return false.
    */
    bool MatchCommandIDs(const DAVA::Vector<DAVA::CommandID_t>& commandIDVector) const;

    //re implement pure virtual function Undo for commands which can not make Undo itself
    void Undo() override;

private:
    const CommandID_t id;
    const String text;
};

template <typename CMD, typename... Arg>
std::unique_ptr<CMD> Command::Create(Arg&&... arg)
{
    return std::unique_ptr<CMD>(new CMD(std::forward<Arg>(arg)...));
}

inline CommandID_t Command::GetID() const
{
    return id;
}

inline const String& Command::GetText() const
{
    return text;
}

inline bool Command::IsModifying() const
{
    return true;
}

inline bool Command::CanUndo() const
{
    return true;
}

inline void Command::Undo()
{
}

inline bool Command::MatchCommandID(DAVA::CommandID_t commandID) const
{
    return (id == commandID);
}
}

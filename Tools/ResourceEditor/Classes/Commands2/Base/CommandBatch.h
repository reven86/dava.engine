#ifndef __COMMAND_BATCH_H__
#define __COMMAND_BATCH_H__

#include "Base/BaseTypes.h"

#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandNotify.h"

class CommandBatch final : public Command2
{
public:
    CommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount);

    void Execute() override;
    void Undo() override;
    void Redo() override;

    DAVA_DEPRECATED(DAVA::Entity* GetEntity() const override);

    void AddAndExec(Command2::Pointer&& command);
    void RemoveCommands(DAVA::int32 commandId);

    bool Empty() const;
    DAVA::uint32 Size() const;

    Command2* GetCommand(DAVA::uint32 index) const;

    bool MatchCommandID(DAVA::int32 commandID) const override;
    bool MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const override;

    bool IsMultiCommandBatch() const;

protected:
    using CommandsContainer = DAVA::Vector<Command2::Pointer>;
    CommandsContainer commandList;

    DAVA::UnorderedSet<DAVA::int32> commandIDs;
};

inline bool CommandBatch::Empty() const
{
    return commandList.empty();
}

inline DAVA::uint32 CommandBatch::Size() const
{
    return static_cast<DAVA::uint32>(commandList.size());
}

inline bool CommandBatch::IsMultiCommandBatch() const
{
    return (commandIDs.size() > 1);
}


#endif // __COMMAND_BATCH_H__

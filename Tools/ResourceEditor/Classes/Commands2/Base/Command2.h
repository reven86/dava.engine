#ifndef __COMMAND2_H__
#define __COMMAND2_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"

#include "Command/ICommand.h"

#include "Commands2/CommandID.h"
#include "Commands2/Base/CommandNotify.h"

class Command2 : public CommandNotifyProvider, public DAVA::ICommand
{
public:
    using Pointer = std::unique_ptr<Command2>;

protected:
    Command2(DAVA::int32 id, const DAVA::String& text = "");

public:
    ~Command2() override;

    template <typename CMD, typename... Arg>
    static std::unique_ptr<CMD> Create(Arg&&... arg)
    {
        return std::unique_ptr<CMD>(new CMD(std::forward<Arg>(arg)...));
    }

    static Pointer CreateEmptyCommand()
    {
        return Pointer();
    }

    DAVA::int32 GetId() const;
    const DAVA::String& GetText() const;

    DAVA_DEPRECATED(virtual DAVA::Entity* GetEntity() const = 0);

    void Execute() override;

    virtual bool IsModifying() const;
    virtual bool CanUndo() const;

    virtual bool MatchCommandID(DAVA::int32 commandID) const;
    virtual bool MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const;

protected:
    void UndoInternalCommand(Command2* command);
    void RedoInternalCommand(Command2* command);

    const DAVA::String text;
    const DAVA::int32 id;
};

inline DAVA::int32 Command2::GetId() const
{
    return id;
}

inline const DAVA::String& Command2::GetText() const
{
    return text;
}

inline bool Command2::CanUndo() const
{
    return true;
}

inline bool Command2::IsModifying() const
{
    return true;
}

#endif // __COMMAND2_H__

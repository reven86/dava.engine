#pragma once

#include "Base/BaseTypes.h"

class CommandAction
{
public:
    using Pointer = std::unique_ptr<CommandAction>;

    CommandAction(DAVA::uint32 id, const DAVA::String& text = DAVA::String());
    virtual void Redo() = 0;

    template <typename CMD, typename... Arg>
    static std::unique_ptr<CMD> Create(Arg&&... arg);

private:
    const DAVA::uint32 id;
    const DAVA::String text;
};

template <typename CMD, typename... Arg>
std::unique_ptr<CMD> CommandAction::Create(Arg&&... arg)
{
    return std::make_unique<CMD>(std::forward<Arg>(arg)...);
}

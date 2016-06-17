#include "Document/CommandsBase/Command.h"

Command::Command(const DAVA::String& text_)
    : text(text_)
{
}

void Command::SetText(const DAVA::String& text_)
{
    text = text_;
}

DAVA::String Command::GetText() const
{
    return text;
}

void Command::Execute()
{
    Redo();
}

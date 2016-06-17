#include "Document/CommandsBase/Command.h"

QECommand::QECommand(const DAVA::String& text_)
    : text(text_)
{
}

void QECommand::SetText(const DAVA::String& text_)
{
    text = text_;
}

DAVA::String QECommand::GetText() const
{
    return text;
}

void QECommand::Execute()
{
    Redo();
}

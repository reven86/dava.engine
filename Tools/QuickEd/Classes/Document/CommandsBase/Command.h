#pragma once

#include "Command/ICommand.h"
#include "Base/BaseTypes.h"

class Command : public DAVA::ICommand
{
public:
    using CommandPtr = std::unique_ptr<ICommand>;

    Command(const DAVA::String& text);

    void SetText(const DAVA::String& text);
    const DAVA::String& GetText() const;

    void Execute() override;

private:
    DAVA::String text;
};
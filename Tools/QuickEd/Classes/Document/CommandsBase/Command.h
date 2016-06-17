#pragma once

#include "Command/ICommand.h"
#include "Base/BaseTypes.h"

class QECommand : public DAVA::ICommand
{
public:
    using CommandPtr = std::unique_ptr<ICommand>;

    QECommand(const DAVA::String& text);

    void SetText(const DAVA::String& text);
    DAVA::String GetText() const;

    void Execute() override;

private:
    DAVA::String text;
};
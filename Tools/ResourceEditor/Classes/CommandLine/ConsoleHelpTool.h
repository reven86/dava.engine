#pragma once

#include "CommandLine/Private/CommandLineModule.h"

class ConsoleHelpTool : public CommandLineModule
{
public:
    ConsoleHelpTool(const DAVA::Vector<DAVA::String>& commandLine);
    static const DAVA::String Key;

private:
    void ShowHelpInternal() override;
};

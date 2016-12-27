#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"
class ConsoleHelpTool : public REConsoleModuleCommon
{
public:
    ConsoleHelpTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    void ShowHelpInternal() override;
};

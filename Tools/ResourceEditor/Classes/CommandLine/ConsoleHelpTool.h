#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"
class ConsoleHelpTool : public REConsoleModuleCommon
{
public:
    ConsoleHelpTool();

protected:
    eFrameResult OnFrameInternal() override;
};

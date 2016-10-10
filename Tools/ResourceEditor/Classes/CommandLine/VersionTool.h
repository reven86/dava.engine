#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"

class VersionTool : public REConsoleModuleCommon
{
public:
    VersionTool();

protected:
    eFrameResult OnFrameInternal() override;
};

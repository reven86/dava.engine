#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"

class VersionTool : public REConsoleModuleCommon
{
public:
    VersionTool(const DAVA::Vector<DAVA::String>& commandLine);

protected:
    eFrameResult OnFrameInternal() override;
};

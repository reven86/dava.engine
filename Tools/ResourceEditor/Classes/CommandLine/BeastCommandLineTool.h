#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include "FileSystem/FilePath.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

class BeastCommandLineTool : public REConsoleModuleCommon
{
public:
    BeastCommandLineTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::FilePath scenePathname;
    DAVA::FilePath outputPathname;
};

#endif //#if defined (__DAVAENGINE_BEAST__)

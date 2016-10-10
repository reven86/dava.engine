#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include "FileSystem/FilePath.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

class BeastCommandLineTool : public REConsoleModuleCommon
{
public:
    BeastCommandLineTool();

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;

private:
    bool ReadCommandLine();

    DAVA::FilePath scenePathname;
    DAVA::FilePath outputPathname;
    DAVA::FilePath qualityConfigPathname;
};

#endif //#if defined (__DAVAENGINE_BEAST__)

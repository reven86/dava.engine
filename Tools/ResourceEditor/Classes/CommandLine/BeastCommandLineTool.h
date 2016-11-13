#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include "FileSystem/FilePath.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

class SceneEditor2;
class BeastRunner;
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

    BeastRunner* beastRunner = false;
    SceneEditor2* scene = nullptr;
};

#endif //#if defined (__DAVAENGINE_BEAST__)

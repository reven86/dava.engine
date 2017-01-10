#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include "FileSystem/FilePath.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
class Scene;
}

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
    DAVA::Scene* scene = nullptr;

    DAVA_VIRTUAL_REFLECTION(BeastCommandLineTool, REConsoleModuleCommon)
    {
        DAVA::ReflectionRegistrator<BeastCommandLineTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};

#endif //#if defined (__DAVAENGINE_BEAST__)

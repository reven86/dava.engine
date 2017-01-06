#pragma once

#include "Render/RenderBase.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
class Camera;
}

class SceneImageDump : public REConsoleModuleCommon
{
public:
    SceneImageDump(const DAVA::Vector<DAVA::String>& commandLine);

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::Camera* FindCamera(DAVA::Entity* rootNode) const;

    DAVA::FilePath sceneFilePath;
    DAVA::FastName cameraName;
    DAVA::int32 width;
    DAVA::int32 height;
    DAVA::eGPUFamily gpuFamily = DAVA::GPU_ORIGIN;
    DAVA::FilePath outputFile;

    DAVA_VIRTUAL_REFLECTION(SceneImageDump, REConsoleModuleCommon)
    {
        DAVA::ReflectionRegistrator<SceneImageDump>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};

#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"
#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

namespace DAVA
{
class Camera;
class Scene;
}

class SceneImageDump : public REConsoleModuleCommon
{
public:
    SceneImageDump();

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;

private:
    bool ReadCommandLine();
    DAVA::Camera* FindCamera(DAVA::Entity* rootNode) const;

    DAVA::FilePath sceneFilePath;
    DAVA::FastName cameraName;
    DAVA::int32 width;
    DAVA::int32 height;
    DAVA::eGPUFamily gpuFamily = DAVA::GPU_ORIGIN;
    DAVA::FilePath outputFile;
    DAVA::FilePath qualityConfigPath;
};

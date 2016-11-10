#pragma once

#include "Render/RenderBase.h"
#include "CommandLine/CommandLineModule.h"

namespace DAVA
{
class Camera;
}

class SceneImageDump : public CommandLineModule
{
public:
    SceneImageDump(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

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
};

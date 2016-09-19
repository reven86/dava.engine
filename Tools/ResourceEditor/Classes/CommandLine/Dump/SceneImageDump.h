#pragma once

#include "CommandLine/CommandLineTool.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Camera;
class Scene;
}

class SceneImageDump : public CommandLineTool
{
public:
    SceneImageDump();

protected:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    DAVA::Camera* FindCamera(DAVA::Entity* rootNode) const;

private:
    DAVA::FilePath sceneFilePath;
    DAVA::FastName cameraName;
    DAVA::int32 width;
    DAVA::int32 height;
    DAVA::FilePath outputFile;
    DAVA::FilePath qualityConfigPath;
};

#include "SceneImageGraber.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/Scene/SceneTabWidget.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"

#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"
#include "Math/MathHelpers.h"

#include <QQuickWindow>
#include <QImage>

namespace SceneImageGrabber
{
namespace SceneImageGrabberDetail
{
struct InternalParams
{
    SceneImageGrabber::Params inputParams;
    DAVA::RefPtr<DAVA::Texture> renderTarget;
};

void GrabImage(Params params)
{
    InternalParams internalParams;
    internalParams.inputParams = std::move(params);

    DAVA::int32 maxSize = std::max(params.imageSize.dx, params.imageSize.dy);
    internalParams.renderTarget = DAVA::Texture::CreateFBO(maxSize, maxSize, DAVA::PixelFormat::FORMAT_RGBA8888, true);

    DAVA::float32 cameraAspect = internalParams.inputParams.cameraToGrab->GetAspect();
    internalParams.inputParams.cameraToGrab->SetAspect(static_cast<DAVA::float32>(params.imageSize.dx) / params.imageSize.dy);
    internalParams.inputParams.scene->renderSystem->SetDrawCamera(params.cameraToGrab.Get());
    internalParams.inputParams.scene->renderSystem->SetMainCamera(params.cameraToGrab.Get());

    rhi::RenderPassConfig& config = params.scene->GetMainPassConfig();
    config.colorBuffer[0].texture = internalParams.renderTarget->handle;
    config.depthStencilBuffer.texture = internalParams.renderTarget->handleDepthStencil;
    config.priority = PRIORITY_SERVICE_2D;
    config.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    config.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    config.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    config.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    Rect viewportRect(0, 0, params.imageSize.dx, params.imageSize.dy);
    params.scene->SetMainPassViewport(viewportRect);
    params.scene->Draw();
    params.cameraToGrab->SetAspect(cameraAspect);

    DAVA::RenderCallbacks::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(), [internalParams](rhi::HSyncObject)
                                                {
                                                    DAVA::FilePath filePath = internalParams.inputParams.outputPath;
                                                    if (filePath.IsEmpty())
                                                    {
                                                        filePath = internalParams.inputParams.scene->GetScenePath().GetDirectory();
                                                    }

                                                    if (filePath.IsDirectoryPathname())
                                                    {
                                                        filePath = DAVA::FilePath(filePath, "GrabbedScene.png");
                                                    }

                                                    DAVA::ScopedPtr<DAVA::Image> image(internalParams.renderTarget->CreateImageFromMemory());
                                                    DAVA::Size2i imageSize = internalParams.inputParams.imageSize;
                                                    DAVA::ScopedPtr<DAVA::Image> regionImage(DAVA::Image::CopyImageRegion(image, DAVA::Rect(0, 0, imageSize.dx, imageSize.dy)));
                                                    regionImage->Save(filePath);
                                                    if (internalParams.inputParams.readyCallback)
                                                    {
                                                        internalParams.inputParams.readyCallback();
                                                    }
                                                });
}
}

void GrabImage(Params params)
{
    if (params.processInDAVAFrame)
    {
        JobManager::Instance()->CreateMainJob(DAVA::Bind(&SceneImageGrabberDetail::GrabImage, params), DAVA::JobManager::eMainJobType::JOB_MAINLAZY);
    }
    else
    {
        SceneImageGrabberDetail::GrabImage(params);
    }
}
}

#include "SceneImageDump.h"

#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"

#include "Base/ScopedPtr.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"

#include "Render/GPUFamilyDescriptor.h"
#include "Render/Texture.h"

#include "Scene/SceneEditor2.h"
#include "Scene/SceneImageGraber.h"

SceneImageDump::SceneImageDump()
    : REConsoleModuleCommon("-sceneimagedump")
{
    using namespace DAVA;
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Full pathname to scene file *.sc2");
    options.AddOption(OptionName::Camera, VariantType(String("")), "Camera name for draw");
    options.AddOption(OptionName::Width, VariantType(int32(0)), "Result image width");
    options.AddOption(OptionName::Height, VariantType(int32(0)), "Result image height");
    options.AddOption(OptionName::GPU, VariantType(String("origin")), "GPU family: PowerVR_iOS, PowerVR_Android, tegra, mali, adreno, origin, dx11");
    options.AddOption(OptionName::OutFile, VariantType(String("")), "Path to output file");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

bool SceneImageDump::PostInitInternal()
{
    bool commandLineIsCorrect = ReadCommandLine();
    if (commandLineIsCorrect == false)
    {
        return false;
    }

    SceneConsoleHelper::InitializeRenderer(qualityPathname);
    return true;
}

bool SceneImageDump::ReadCommandLine()
{
    sceneFilePath = options.GetOption(OptionName::ProcessFile).AsString();
    if (sceneFilePath.IsEmpty() || !sceneFilePath.Exists())
    {
        DAVA::Logger::Error("Path to scene is incorrect");
        return false;
    }

    cameraName = DAVA::FastName(options.GetOption(OptionName::Camera).AsString());
    if (!cameraName.IsValid())
    {
        DAVA::Logger::Error("Camera name is not specified");
        return false;
    }

    width = options.GetOption(OptionName::Width).AsInt32();
    height = options.GetOption(OptionName::Height).AsInt32();
    if (width <= 0 || height <= 0)
    {
        DAVA::Logger::Error("Incorrect size for output image");
        return false;
    }

    DAVA::String gpuName = options.GetOption(OptionName::GPU).AsString();
    gpuFamily = DAVA::GPUFamilyDescriptor::GetGPUByName(gpuName);
    outputFile = options.GetOption(OptionName::OutFile).AsString();

    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();
    qualityPathname = SceneConsoleHelper::CreateQualityPathname(qualityPathname, sceneFilePath);
    if (qualityPathname.IsEmpty())
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", sceneFilePath.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult SceneImageDump::OnFrameInternal()
{
    const rhi::HTexture nullTexture;
    const rhi::Viewport nullViewport(0, 0, 1, 1);

    DAVA::Vector<DAVA::eGPUFamily> textureLoadingOrder = DAVA::Texture::GetGPULoadingOrder();
    DAVA::Texture::SetGPULoadingOrder({ gpuFamily });

    DAVA::ScopedPtr<SceneEditor2> scene(new SceneEditor2());
    if (scene->LoadScene(sceneFilePath) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR)
    {
        scene->EnableEditorSystems();
        DAVA::Camera* camera = FindCamera(scene);
        if (camera == nullptr)
        {
            DAVA::Logger::Error("Camera %s not found in scene %s", cameraName.c_str(), sceneFilePath.GetAbsolutePathname().c_str());
            return;
        }

        bool grabFinished = false;

        SceneImageGrabber::Params params;
        params.scene = scene;
        params.cameraToGrab = camera;
        params.imageSize = DAVA::Size2i(width, height);
        params.outputFile = DAVA::FilePath(outputFile);
        params.processInDAVAFrame = false;
        params.readyCallback = [&grabFinished]()
        {
            grabFinished = true;
        };

        scene->Update(0.1f);
        DAVA::Renderer::BeginFrame();
        DAVA::RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
        SceneImageGrabber::GrabImage(params);
        DAVA::Renderer::EndFrame();

        while (grabFinished == false)
        {
            DAVA::Renderer::BeginFrame();
            DAVA::RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
            scene->Update(0.1f);
            DAVA::Renderer::EndFrame();
        }
    }

    DAVA::Texture::SetGPULoadingOrder(textureLoadingOrder);

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

DAVA::Camera* SceneImageDump::FindCamera(DAVA::Entity* rootNode) const
{
    for (DAVA::Entity* entity : rootNode->children)
    {
        if (entity->GetName() == cameraName)
        {
            DAVA::Camera* camera = DAVA::GetCamera(entity);
            if (camera != nullptr)
            {
                return camera;
            }
        }
    }

    return nullptr;
}

void SceneImageDump::BeforeDestroyedInternal()
{
    SceneConsoleHelper::ReleaseRendering();
}

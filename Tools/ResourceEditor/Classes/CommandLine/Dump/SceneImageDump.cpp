#include "SceneImageDump.h"

#include "Classes/Qt/Scene/SceneImageGraber.h"

SceneImageDump::SceneImageDump()
    : CommandLineTool("-sceneimagedump")
{
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Full pathname to scene file *.sc2");
    options.AddOption(OptionName::Camera, VariantType(String("")), "Camera name for draw");
    options.AddOption(OptionName::Width, VariantType(int32(0)), "Result image width");
    options.AddOption(OptionName::Height, VariantType(int32(0)), "Result image height");
    options.AddOption(OptionName::GPU, VariantType(String("origin")), "GPU family: PowerVR_iOS, PowerVR_Android, tegra, mali, adreno, origin, dx11");
    options.AddOption(OptionName::OutFile, VariantType(String("")), "Path to output file");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

void SceneImageDump::ConvertOptionsToParamsInternal()
{
    sceneFilePath = options.GetOption(OptionName::ProcessFile).AsString();
    cameraName = DAVA::FastName(options.GetOption(OptionName::Camera).AsString());
    width = options.GetOption(OptionName::Width).AsInt32();
    height = options.GetOption(OptionName::Height).AsInt32();
    String gpuName = options.GetOption(OptionName::GPU).AsString();
    gpuFamily = DAVA::GPUFamilyDescriptor::GetGPUByName(gpuName);
    outputFile = options.GetOption(OptionName::OutFile).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();
}

bool SceneImageDump::InitializeInternal()
{
    if (sceneFilePath.IsEmpty() || !sceneFilePath.Exists())
    {
        DAVA::Logger::Error("Path to scene is incorrect");
        return false;
    }

    if (!cameraName.IsValid())
    {
        DAVA::Logger::Error("Camera name is not specified");
        return false;
    }

    if (width <= 0 || height <= 0)
    {
        DAVA::Logger::Error("Incorrect size for output image");
        return false;
    }

    return true;
}

void SceneImageDump::ProcessInternal()
{
    const rhi::HTexture nullTexture;
    const rhi::Viewport nullViewport(0, 0, 1, 1);

    DAVA::Vector<DAVA::eGPUFamily> textureLoadingOrder = Texture::GetGPULoadingOrder();
    Texture::SetGPULoadingOrder({ gpuFamily });

    ScopedPtr<SceneEditor2> scene(new SceneEditor2());
    if (scene->LoadScene(sceneFilePath) == SceneFileV2::eError::ERROR_NO_ERROR)
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
        Renderer::BeginFrame();
        RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
        SceneImageGrabber::GrabImage(params);
        Renderer::EndFrame();

        while (grabFinished == false)
        {
            Renderer::BeginFrame();
            RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
            scene->Update(0.1f);
            Renderer::EndFrame();
        }
    }

    Texture::SetGPULoadingOrder(textureLoadingOrder);
}

DAVA::FilePath SceneImageDump::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(sceneFilePath);
    }

    return qualityConfigPath;
}

DAVA::Camera* SceneImageDump::FindCamera(DAVA::Entity* rootNode) const
{
    for (Entity* entity : rootNode->children)
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

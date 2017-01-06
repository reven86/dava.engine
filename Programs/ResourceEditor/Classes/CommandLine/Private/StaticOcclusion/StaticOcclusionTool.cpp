#include "CommandLine/StaticOcclusionTool.h"
#include "CommandLine/Private/SceneConsoleHelper.h"
#include "CommandLine/Private/OptionName.h"

#include "Utils/SceneUtils/SceneUtils.h"

#include "TArc/Utils/ModuleCollection.h"

#include "Math/Color.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/StaticOcclusionBuildSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"

StaticOcclusionTool::StaticOcclusionTool(const DAVA::Vector<DAVA::String>& commandLine)
    : REConsoleModuleCommon(commandLine, "-staticocclusion")
    , scene(nullptr)
{
    using namespace DAVA;

    options.AddOption(OptionName::Build, VariantType(false), "Enables build of static occlusion");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Full pathname to scene file *.sc2");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

bool StaticOcclusionTool::PostInitInternal()
{
    using namespace DAVA;

    if (options.GetOption(OptionName::Build).AsBool())
    {
        commandAction = ACTION_BUILD;
    }
    else
    {
        Logger::Error("Wrong action was selected");
        return false;
    }

    scenePathname = options.GetOption(OptionName::ProcessFile).AsString();
    if (scenePathname.IsEmpty())
    {
        Logger::Error("Filename was not set");
        return false;
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, scenePathname);
    if (!qualityInitialized)
    {
        Logger::Error("Cannot create path to quality.yaml from %s", scenePathname.GetAbsolutePathname().c_str());
        return false;
    }

    if (commandAction == ACTION_BUILD)
    {
        scene.reset(new Scene());
        staticOcclusionBuildSystem = new StaticOcclusionBuildSystem(scene);
        scene->AddSystem(staticOcclusionBuildSystem, MAKE_COMPONENT_MASK(Component::STATIC_OCCLUSION_COMPONENT) | MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS, scene->renderUpdateSystem);

        if (scene->LoadScene(scenePathname) != SceneFileV2::eError::ERROR_NO_ERROR)
        {
            Logger::Error("Cannot load scene %s", scenePathname.GetAbsolutePathname().c_str());

            staticOcclusionBuildSystem = nullptr;
            scene.reset();
            return false;
        }

        ScopedPtr<Camera> lodSystemDummyCamera(new Camera());
        lodSystemDummyCamera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        lodSystemDummyCamera->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        lodSystemDummyCamera->SetTarget(Vector3(0.0f, 0.1f, 0.0f));
        lodSystemDummyCamera->SetupPerspective(90.f, 320.0f / 480.0f, 1.f, 5000.f);
        lodSystemDummyCamera->SetAspect(1.0f);

        scene->SetCurrentCamera(lodSystemDummyCamera);

        scene->Update(0.1f); // we need to call update to initialize (at least) QuadTree.
        staticOcclusionBuildSystem->Build();
        SceneConsoleHelper::FlushRHI();
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult StaticOcclusionTool::OnFrameInternal()
{
    if (commandAction == ACTION_BUILD)
    {
        if (staticOcclusionBuildSystem != nullptr && staticOcclusionBuildSystem->IsInBuild())
        {
            const rhi::HTexture nullTexture;
            const rhi::Viewport nullViewport(0, 0, 1, 1);

            DAVA::Renderer::BeginFrame();
            DAVA::RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
            scene->Update(0.1f);
            DAVA::Renderer::EndFrame();

            return DAVA::TArc::ConsoleModule::eFrameResult::CONTINUE;
        }
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void StaticOcclusionTool::BeforeDestroyedInternal()
{
    if (scene)
    {
        scene->SetCurrentCamera(nullptr);
        scene->SaveScene(scenePathname, true);
        staticOcclusionBuildSystem = nullptr;
        scene.reset();
    }

    SceneConsoleHelper::FlushRHI();
}

void StaticOcclusionTool::ShowHelpInternal()
{
    REConsoleModuleCommon::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-staticocclusion -build -processfile /Users/Test/DataSource/3d/Maps/scene.sc2");
}

DECL_CONSOLE_MODULE(StaticOcclusionTool, "-staticocclusion");
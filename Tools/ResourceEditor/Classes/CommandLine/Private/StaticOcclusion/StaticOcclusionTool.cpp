#include "CommandLine/StaticOcclusionTool.h"
#include "CommandLine/Private/SceneConsoleHelper.h"
#include "CommandLine/Private/OptionName.h"

#include "Utils/SceneUtils/SceneUtils.h"

#include "Scene/SceneEditor2.h"

#include <QOpenGLContext>

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
    if (options.GetOption(OptionName::Build).AsBool())
    {
        commandAction = ACTION_BUILD;
    }
    else
    {
        DAVA::Logger::Error("Wrong action was selected");
        return false;
    }

    DAVA::FilePath scenePathname = options.GetOption(OptionName::ProcessFile).AsString();
    if (scenePathname.IsEmpty())
    {
        DAVA::Logger::Error("Filename was not set");
        return false;
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, scenePathname);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", scenePathname.GetAbsolutePathname().c_str());
        return false;
    }

    if (commandAction == ACTION_BUILD)
    {
        if (QOpenGLContext::currentContext() == nullptr)
        {
            DAVA::Logger::Error("Cannot swap buffers because of no OPEN GL Context.");
        }

        scene.reset(new SceneEditor2());
        if (scene->LoadScene(scenePathname) != SceneFileV2::eError::ERROR_NO_ERROR)
        {
            DAVA::Logger::Error("Cannot load scene %s", scenePathname.GetAbsolutePathname().c_str());

            scene.reset();
            return false;
        }

        ScopedPtr<DAVA::Camera> lodSystemDummyCamera(new Camera());
        {
            lodSystemDummyCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
            lodSystemDummyCamera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 0.0f));
            lodSystemDummyCamera->SetTarget(DAVA::Vector3(0.0f, 0.1f, 0.0f));
            lodSystemDummyCamera->SetupPerspective(90.f, 320.0f / 480.0f, 1.f, 5000.f);
            lodSystemDummyCamera->SetAspect(1.0f);
        }

        scene->SetCurrentCamera(lodSystemDummyCamera);

        scene->Update(0.1f); // we need to call update to initialize (at least) QuadTree.
        scene->staticOcclusionBuildSystem->Build();
        SceneConsoleHelper::FlushRHI();
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult StaticOcclusionTool::OnFrameInternal()
{
    if (commandAction == ACTION_BUILD)
    {
        if (scene && scene->staticOcclusionBuildSystem->IsInBuild())
        {
            const rhi::HTexture nullTexture;
            const rhi::Viewport nullViewport(0, 0, 1, 1);

            Renderer::BeginFrame();
            RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
            scene->Update(0.1f);
            Renderer::EndFrame();

            QOpenGLContext* context = QOpenGLContext::currentContext();
            if (context)
            {
                context->swapBuffers(context->surface());
            }

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
        scene->SaveScene();
        scene.reset();
    }

    SceneConsoleHelper::FlushRHI();
}

void StaticOcclusionTool::ShowHelpInternal()
{
    REConsoleModuleCommon::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-staticocclusion -build -processfile /Users/SmokeTest/DataSource/3d/Maps/scene.sc2");
}

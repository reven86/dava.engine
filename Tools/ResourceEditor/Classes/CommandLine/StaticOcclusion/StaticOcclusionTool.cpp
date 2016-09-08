#include "CommandLine/StaticOcclusion/StaticOcclusionTool.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "Scene/SceneEditor2.h"
#include "CommandLine/OptionName.h"

#include <QOpenGLContext>

using namespace DAVA;

StaticOcclusionTool::StaticOcclusionTool()
    : CommandLineTool("-staticocclusion")
{
    options.AddOption(OptionName::Build, VariantType(false), "Enables build of static occlusion");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Full pathname to scene file *.sc2");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

void StaticOcclusionTool::ConvertOptionsToParamsInternal()
{
    if (options.GetOption(OptionName::Build).AsBool())
    {
        commandAction = ACTION_BUILD;
    }

    scenePathname = options.GetOption(OptionName::ProcessFile).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();
}

bool StaticOcclusionTool::InitializeInternal()
{
    if (commandAction == ACTION_NONE)
    {
        Logger::Error("Wrong action was selected");
        return false;
    }

    if (scenePathname.IsEmpty())
    {
        Logger::Error("Filename was not set");
        return false;
    }

    return true;
}

void StaticOcclusionTool::ProcessInternal()
{
    const rhi::HTexture nullTexture;
    const rhi::Viewport nullViewport(0, 0, 1, 1);

    if (commandAction == ACTION_BUILD)
    {
        if (QOpenGLContext::currentContext() == nullptr)
        {
            Logger::Error("Cannot swap buffers because of no OPEN GL Context.");
        }

        ScopedPtr<SceneEditor2> scene(new SceneEditor2());
        if (scene->LoadScene(scenePathname) == SceneFileV2::eError::ERROR_NO_ERROR)
        {
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
            RenderObjectsFlusher::Flush();

            while (scene->staticOcclusionBuildSystem->IsInBuild())
            {
                Renderer::BeginFrame();
                RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
                scene->Update(0.1f);
                Renderer::EndFrame();

                QOpenGLContext* context = QOpenGLContext::currentContext();
                if (context)
                {
                    context->swapBuffers(context->surface());
                }
            }

            scene->SetCurrentCamera(nullptr);
            scene->SaveScene();
        }
        RenderObjectsFlusher::Flush();
    }
}

DAVA::FilePath StaticOcclusionTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(scenePathname);
    }

    return qualityConfigPath;
}

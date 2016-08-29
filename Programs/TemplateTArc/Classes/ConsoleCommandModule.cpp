#include "ConsoleCommandModule.h"

#include "TArcCore/ContextAccessor.h"
#include "DataProcessing/DataNode.h"
#include "DataProcessing/DataContext.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Systems/StaticOcclusionBuildSystem.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"

#include "Debug/DVAssert.h"

struct StaticOcclusionToolData: public DAVA::TArc::DataNode
{
    ~StaticOcclusionToolData()
    {
        DAVA::SafeRelease(scene);
    }

    DAVA::Scene* scene = nullptr;
    DAVA::StaticOcclusionBuildSystem* buildSystem = nullptr;

    DAVA_VIRTUAL_REFLECTION(StaticOcclusionToolData, DAVA::TArc::DataNode)
    {
    }
};

void ConsoleCommandModule::PostInit()
{
    using namespace DAVA;
    DAVA::TArc::ContextAccessor& accessor = GetAccessor();
    DVASSERT(accessor.HasActiveContext());

    std::unique_ptr<StaticOcclusionToolData> data = std::make_unique<StaticOcclusionToolData>();
    data->scene = new DAVA::Scene();
    data->scene->LoadScene(DAVA::FilePath(scenePath));
    data->buildSystem = new DAVA::StaticOcclusionBuildSystem(data->scene);
    uint64 componentMask = MAKE_COMPONENT_MASK(DAVA::Component::STATIC_OCCLUSION_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::TRANSFORM_COMPONENT);
    data->scene->AddSystem(data->buildSystem, componentMask, Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    data->scene->Update(0.1f);
    data->buildSystem->Build();

    accessor.GetActiveContext().CreateData(std::move(data));
}

DAVA::TArc::ConsoleModule::eFrameResult ConsoleCommandModule::OnFrame()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    ContextAccessor& accessor = GetAccessor();
    DVASSERT(accessor.HasActiveContext());
    StaticOcclusionToolData& data = accessor.GetActiveContext().GetData<StaticOcclusionToolData>();

    EngineContext& engineContext = accessor.GetEngineContext();

    if (data.buildSystem->IsInBuild())
    {
        const rhi::HTexture nullTexture;
        const rhi::Viewport nullViewport(0, 0, 1, 1);

        data.scene->Update(0.1f);
        Renderer::BeginFrame();
        RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, Color::Clear, nullViewport);
        Renderer::EndFrame();
    }

    return data.buildSystem->IsInBuild() ? ConsoleModule::eFrameResult::CONTINUE : ConsoleModule::eFrameResult::JOB_FINISHED;
}

void ConsoleCommandModule::BeforeDestroyed()
{
    DAVA::TArc::ContextAccessor& accessor = GetAccessor();
    DVASSERT(accessor.HasActiveContext());
    StaticOcclusionToolData& data = accessor.GetActiveContext().GetData<StaticOcclusionToolData>();
    data.scene->SaveScene(DAVA::FilePath(scenePath));
    accessor.GetActiveContext().DeleteData<StaticOcclusionToolData>();
}

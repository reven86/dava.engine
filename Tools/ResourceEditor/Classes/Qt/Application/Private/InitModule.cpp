#include "Classes/Qt/Application/InitModule.h"
#include "Classes/Qt/DataStructures/RECommonData.h"
#include "Classes/Qt/DataStructures/ProjectManagerData.h"

#include "Classes/Qt/Scene/Selectable.h"
#include "Classes/Qt/Scene/BaseTransformProxies.h"

#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitterInstance.h"

InitModule::~InitModule()
{
    Selectable::RemoveAllTransformProxies();
}

void InitModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
}

void InitModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
}

void InitModule::PostInit()
{
    Selectable::AddTransformProxyForClass<DAVA::Entity, EntityTransformProxy>();
    Selectable::AddTransformProxyForClass<DAVA::ParticleEmitterInstance, EmitterTransformProxy>();

    using namespace DAVA::TArc;
    ContextAccessor& accessor = GetAccessor();
    DataContext* ctx = accessor.GetGlobalContext();

    ctx->CreateData(std::make_unique<RECommonData>());
    ctx->GetData<RECommonData>()->Init();
    projectDataWrapper = accessor.CreateWrapper(DAVA::ReflectedType::Get<ProjectManagerData>());
    projectDataWrapper.AddListener(this);
}

void InitModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields)
{
    using namespace DAVA::TArc;

    ContextAccessor& accessor = GetAccessor();
    DataContext* ctx = accessor.GetGlobalContext();

    RECommonData* sharedData = ctx->GetData<RECommonData>();
    DVASSERT(sharedData != nullptr);

    ProjectManagerData* projectData = ctx->GetData<ProjectManagerData>();
    if (projectData == nullptr)
    {
        return;
    }

    DAVA::FilePath projectPath = projectData->GetProjectPath();
    if (!projectPath.IsEmpty())
    {
        sharedData->editorConfig->ParseConfig(projectPath + "EditorConfig.yaml");
        sharedData->sceneValidator->SetPathForChecking(projectPath);
        sharedData->wasDataChanged = !sharedData->wasDataChanged;
    }
}

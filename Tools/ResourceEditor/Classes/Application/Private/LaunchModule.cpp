#include "Classes/Application/LaunchModule.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Application/LaunchModuleData.h"

#include "Classes/Project/ProjectManagerData.h"

void LaunchModule::PostInit()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();

    accessor->GetGlobalContext()->CreateData(std::make_unique<LaunchModuleData>());

    projectDataWrapper = accessor->CreateWrapper(DAVA::ReflectedType::Get<ProjectManagerData>());
    projectDataWrapper.SetListener(this);

    InvokeOperation(REGlobal::OpenLastProjectOperation.ID);
}

void LaunchModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA::TArc;

    DVASSERT(projectDataWrapper == wrapper);

    DataContext* ctx = GetAccessor()->GetGlobalContext();
    if (ctx->GetData<LaunchModuleData>()->launchFinished == true)
    {
        return;
    }

    ProjectManagerData* data = ctx->GetData<ProjectManagerData>();
    if (data == nullptr || data->GetProjectPath().IsEmpty())
        return;

    ctx->GetData<LaunchModuleData>()->launchFinished = true;
    projectDataWrapper.SetListener(nullptr);
    projectDataWrapper = DAVA::TArc::DataWrapper();
}

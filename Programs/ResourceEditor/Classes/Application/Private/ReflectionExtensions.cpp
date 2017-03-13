#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/SceneManager/SceneData.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/CommonFieldNames.h>

#include <Reflection/ReflectionRegistrator.h>

namespace ReflectionExtensionsDetail
{
DAVA::String GetSceneName(DAVA::TArc::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScenePath().GetBasename();
}

DAVA::String GetScenePath(DAVA::TArc::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScenePath().GetAbsolutePathname();
}

bool IsSceneChanged(DAVA::TArc::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScene()->IsChanged();
}
}

void RegisterDataContextExtensions()
{
    using namespace ReflectionExtensionsDetail;
    using namespace DAVA::TArc;
    DAVA::ReflectionRegistrator<DAVA::TArc::DataContext>::Begin()
    .Field(ContextNameFieldName, &GetSceneName, nullptr)
    .Field(ContextToolTipFieldName, &GetScenePath, nullptr)
    .Field(IsContextModifiedFieldName, &IsSceneChanged, nullptr)
    .End();
}

void RegisterReflectionExtensions()
{
    RegisterDataContextExtensions();
}

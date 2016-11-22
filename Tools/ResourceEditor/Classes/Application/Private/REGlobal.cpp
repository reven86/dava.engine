#include "Classes/Application/REGlobal.h"

#include "TArc/Core/Private/CoreInterface.h"

namespace REGlobal
{
namespace REGlobalDetails
{
DAVA::TArc::CoreInterface* coreInstance = nullptr;
}

DAVA::TArc::WindowKey MainWindowKey(DAVA::FastName("ResourceEditor"));

DAVA::TArc::DataContext* GetGlobalContext()
{
    if (REGlobalDetails::coreInstance == nullptr)
        return nullptr;
    return REGlobalDetails::coreInstance->GetGlobalContext();
}

DAVA::TArc::DataContext* GetActiveContext()
{
    if (REGlobalDetails::coreInstance == nullptr)
        return nullptr;
    return REGlobalDetails::coreInstance->GetActiveContext();
}

DAVA::TArc::OperationInvoker* GetInvoker()
{
    return REGlobalDetails::coreInstance;
}

DAVA::TArc::ContextAccessor* GetAccessor()
{
    return REGlobalDetails::coreInstance;
}

DAVA::TArc::DataWrapper CreateDataWrapper(const DAVA::ReflectedType* type)
{
    if (REGlobalDetails::coreInstance == nullptr)
        return DAVA::TArc::DataWrapper();
    return REGlobalDetails::coreInstance->CreateWrapper(type);
}

void InitTArcCore(DAVA::TArc::CoreInterface* core)
{
    REGlobalDetails::coreInstance = core;
}

IMPL_OPERATION_ID(OpenLastProjectOperation);
IMPL_OPERATION_ID(CreateNewSceneOperation);
IMPL_OPERATION_ID(OpenSceneOperation);
IMPL_OPERATION_ID(SaveCurrentScene);
IMPL_OPERATION_ID(CloseAllScenesOperation);
IMPL_OPERATION_ID(ReloadTexturesOperation);
IMPL_OPERATION_ID(ShowScenePreviewOperation);
IMPL_OPERATION_ID(HideScenePreviewOperation);

} // namespace REGlobal
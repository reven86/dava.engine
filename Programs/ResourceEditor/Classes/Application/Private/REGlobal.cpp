#include "Classes/Application/REGlobal.h"

#include "TArc/Core/Core.h"

namespace REGlobal
{
namespace REGlobalDetails
{
DAVA::TArc::Core* coreInstance = nullptr;

DAVA::TArc::CoreInterface* GetCoreInterface()
{
    return coreInstance->GetCoreInterface();
}

DAVA::TArc::UI* GetUI()
{
    return coreInstance->GetUI();
}
}

DAVA::TArc::WindowKey MainWindowKey(DAVA::FastName("ResourceEditor"));

DAVA::TArc::DataContext* GetGlobalContext()
{
    DAVA::TArc::CoreInterface* coreInterface = REGlobalDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return nullptr;
    return coreInterface->GetGlobalContext();
}

DAVA::TArc::DataContext* GetActiveContext()
{
    DAVA::TArc::CoreInterface* coreInterface = REGlobalDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return nullptr;
    return coreInterface->GetActiveContext();
}

DAVA::TArc::OperationInvoker* GetInvoker()
{
    return REGlobalDetails::GetCoreInterface();
}

DAVA::TArc::ContextAccessor* GetAccessor()
{
    return REGlobalDetails::GetCoreInterface();
}

DAVA::TArc::DataWrapper CreateDataWrapper(const DAVA::ReflectedType* type)
{
    DAVA::TArc::CoreInterface* coreInterface = REGlobalDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return DAVA::TArc::DataWrapper();
    return coreInterface->CreateWrapper(type);
}

DAVA::TArc::ModalMessageParams::Button ShowModalMessage(const DAVA::TArc::ModalMessageParams& params)
{
    DAVA::TArc::UI* ui = REGlobalDetails::GetUI();
    DVASSERT(ui != nullptr);
    if (ui == nullptr)
    {
        return DAVA::TArc::ModalMessageParams::NoButton;
    }
    return ui->ShowModalMessage(MainWindowKey, params);
}

void InitTArcCore(DAVA::TArc::Core* core)
{
    REGlobalDetails::coreInstance = core;
}

IMPL_OPERATION_ID(OpenLastProjectOperation);
IMPL_OPERATION_ID(CreateNewSceneOperation);
IMPL_OPERATION_ID(OpenSceneOperation);
IMPL_OPERATION_ID(AddSceneOperation);
IMPL_OPERATION_ID(SaveCurrentScene);
IMPL_OPERATION_ID(CloseAllScenesOperation);
IMPL_OPERATION_ID(ReloadTexturesOperation);

} // namespace REGlobal

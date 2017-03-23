#include "Application/QEGlobal.h"

#include <TArc/Core/Core.h>

namespace QEGlobal
{
namespace QEGlobalDetails
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

DAVA::TArc::DataContext* GetGlobalContext()
{
    DAVA::TArc::CoreInterface* coreInterface = QEGlobalDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return nullptr;
    return coreInterface->GetGlobalContext();
}

DAVA::TArc::DataContext* GetActiveContext()
{
    DAVA::TArc::CoreInterface* coreInterface = QEGlobalDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return nullptr;
    return coreInterface->GetActiveContext();
}

DAVA::TArc::OperationInvoker* GetInvoker()
{
    return QEGlobalDetails::GetCoreInterface();
}

DAVA::TArc::ContextAccessor* GetAccessor()
{
    return QEGlobalDetails::GetCoreInterface();
}

DAVA::TArc::DataWrapper CreateDataWrapper(const DAVA::ReflectedType* type)
{
    DAVA::TArc::CoreInterface* coreInterface = QEGlobalDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return DAVA::TArc::DataWrapper();
    return coreInterface->CreateWrapper(type);
}

DAVA::TArc::ModalMessageParams::Button ShowModalMessage(const DAVA::TArc::ModalMessageParams& params)
{
    DAVA::TArc::UI* ui = QEGlobalDetails::GetUI();
    DVASSERT(ui != nullptr);
    if (ui == nullptr)
    {
        return DAVA::TArc::ModalMessageParams::NoButton;
    }
    return ui->ShowModalMessage(windowKey, params);
}

void InitTArcCore(DAVA::TArc::Core* core)
{
    QEGlobalDetails::coreInstance = core;
}

DAVA::TArc::WindowKey windowKey(DAVA::FastName("QuickEd"));

IMPL_OPERATION_ID(OpenLastProject);
IMPL_OPERATION_ID(OpenDocumentByPath);
IMPL_OPERATION_ID(CloseAllDocuments);
IMPL_OPERATION_ID(SelectFile);
IMPL_OPERATION_ID(SelectControl);
}

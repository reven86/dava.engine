#include "Classes/Qt/Application/REGlobal.h"

#include "TArc/Core/Private/CoreInterface.h"

namespace REGlobal
{
namespace REGlobalDetails
{
DAVA::TArc::CoreInterface* coreInstance = nullptr;
}

DAVA::FastName MainWindowName = DAVA::FastName("ResourceEditor");

DAVA::TArc::DataContext* GetGlobalContext()
{
    if (REGlobalDetails::coreInstance == nullptr)
        return nullptr;
    return REGlobalDetails::coreInstance->GetGlobalContext();
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

} // namespace REGlobal
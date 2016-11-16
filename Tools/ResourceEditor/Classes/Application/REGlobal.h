#pragma once

#include "Base/FastName.h"

#include "TArc/Core/OperationRegistrator.h"
#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/WindowSubSystem/UI.h"

namespace DAVA
{
namespace TArc
{
class CoreInterface;
class DataContext;
}
}

namespace REGlobal
{
extern DAVA::TArc::WindowKey MainWindowKey;

DAVA::TArc::DataContext* GetGlobalContext();
DAVA::TArc::DataWrapper CreateDataWrapper(const DAVA::ReflectedType* type);
void InitTArcCore(DAVA::TArc::CoreInterface* core);

template <typename T>
T* GetDataNode()
{
    DAVA::TArc::DataContext* ctx = GetGlobalContext();
    if (ctx == nullptr)
        return nullptr;
    return ctx->GetData<T>();
}

DECLARE_OPERATION_ID(OpenLastProjectOperation);
}
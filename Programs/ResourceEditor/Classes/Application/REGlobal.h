#pragma once

#include "Base/FastName.h"

#include "TArc/Core/OperationRegistrator.h"
#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Core/OperationInvoker.h"
#include "TArc/Core/ContextAccessor.h"

namespace DAVA
{
namespace TArc
{
class Core;
class DataContext;
}
}

namespace REGlobal
{
extern DAVA::TArc::WindowKey MainWindowKey;
void InitTArcCore(DAVA::TArc::Core* core);

DAVA::TArc::DataContext* GetGlobalContext();
DAVA::TArc::DataContext* GetActiveContext();

DAVA::TArc::OperationInvoker* GetInvoker();
DAVA::TArc::ContextAccessor* GetAccessor();

DAVA::TArc::DataWrapper CreateDataWrapper(const DAVA::ReflectedType* type);
DAVA::TArc::ModalMessageParams::Button ShowModalMessage(const DAVA::TArc::ModalMessageParams& params);

template <typename T>
T* GetDataNode()
{
    DAVA::TArc::DataContext* ctx = GetGlobalContext();
    if (ctx == nullptr)
        return nullptr;
    return ctx->GetData<T>();
}

template <typename T>
T* GetActiveDataNode()
{
    DAVA::TArc::DataContext* ctx = GetActiveContext();
    if (ctx == nullptr)
        return nullptr;
    return ctx->GetData<T>();
}

DECLARE_OPERATION_ID(OpenLastProjectOperation); // Args - empty
DECLARE_OPERATION_ID(CreateNewSceneOperation); // Args - empty
DECLARE_OPERATION_ID(OpenSceneOperation); // Args - scenePath: DAVA::FilePath
DECLARE_OPERATION_ID(AddSceneOperation); // Args - scenePath: DAVA::FilePath
DECLARE_OPERATION_ID(SaveCurrentScene); // Args - empty
DECLARE_OPERATION_ID(CloseAllScenesOperation); // Args - need ask user about saving scenes : bool
DECLARE_OPERATION_ID(ReloadTexturesOperation); // Args - gpu : eGpuFamily
}

#pragma once

#include "TArc/Core/Private/CoreInterface.h"

namespace DAVA
{
namespace TArc
{
class UI;
class DataContext;
class ContextAccessor;
class WindowKey;

class ClientModule
{
public:
    virtual ~ClientModule()
    {
    }

protected:
    virtual void OnContextCreated(DataContext& context) = 0;
    virtual void OnContextDeleted(DataContext& context) = 0;
    virtual void OnWindowClosed(const WindowKey& key)
    {
    }

    virtual void PostInit() = 0;
    ContextAccessor& GetAccessor();
    UI& GetUI();

    template <typename Ret, typename Cls, typename... Args>
    void RegisterOperation(int operationID, Cls* object, Ret (Cls::*fn)(Args...) const);

    template <typename Ret, typename Cls, typename... Args>
    void RegisterOperation(int operationID, Cls* object, Ret (Cls::*fn)(Args...));

    template <typename... Args>
    void InvokeOperation(int operationId, const Args&... args);

private:
    void Init(CoreInterface* coreInterface, UI* ui);

private:
    friend class Core;
    friend class ControllerModule;

    CoreInterface* coreInterface = nullptr;
    UI* ui = nullptr;
};

template <typename Ret, typename Cls, typename... Args>
inline void ClientModule::RegisterOperation(int operationID, Cls* object, Ret (Cls::*fn)(Args...) const)
{
    coreInterface->RegisterOperation(operationID, AnyFn(fn).BindThis(object));
}

template <typename Ret, typename Cls, typename... Args>
inline void ClientModule::RegisterOperation(int operationID, Cls* object, Ret (Cls::*fn)(Args...))
{
    coreInterface->RegisterOperation(operationID, AnyFn(fn).BindThis(object));
}

template <typename... Args>
inline void ClientModule::InvokeOperation(int operationId, const Args&... args)
{
    coreInterface->Invoke(operationId, args...);
}

} // namespace TArc
} // namespace DAVA

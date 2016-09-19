#pragma once

#include "TArcCore/Private/CoreInterface.h"
#include "WindowSubSystem/Private/UIManager.h"

#include "Base/BaseTypes.h"

#include <memory>

namespace DAVA
{
class Engine;

namespace TArc
{
class ClientModule;
class ControllerModule;

class Core final : private CoreInterface, private UIManager::Delegate
{
public:
    Core(Engine& engine_);
    ~Core();

    template<typename T>
    void CreateModule()
    {
        static_assert(std::is_base_of<TArc::ClientModule, T>::value ||
                      std::is_base_of<TArc::ControllerModule, T>::value,
                      "Module should be Derived from tarc::ControllerModule or tarc::ClientModule");
        AddModule(new T());
    }

private:
    void AddModule(ClientModule* module);
    void AddModule(ControllerModule* module);

    void OnLoopStarted();
    void OnWindowCreated(Window& w);
    void OnFrame();
    void OnLoopStopped();

    // Inherited via ContextAccessor
    void ForEachContext(const Function<void(DataContext&)>& functor) override;
    DataContext& GetGlobalContext() override;
    DataContext& GetContext(DataContext::ContextID contextID) override;
    DataContext& GetActiveContext() override;
    bool HasActiveContext() const override;
    DataWrapper CreateWrapper(const ReflectedType* type) override;
    DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor) override;
    EngineContext& GetEngineContext() override;

    // Inherited via ContextManager
    DataContext::ContextID CreateContext() override;
    void DeleteContext(DataContext::ContextID contextID) override;
    void ActivateContext(DataContext::ContextID contextID) override;

    void ActivateContext(DataContext* context);
    RenderWidget* GetRenderWidget() const;

    // Inherited via OperationInvoker
    void RegisterOperation(int operationID, AnyFn&& fn) override;
    void Invoke(int operationId) override;
    void Invoke(int operationId, const Any& a) override;
    void Invoke(int operationId, const Any& a1, const Any& a2) override;
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3) override;
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) override;
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) override;
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) override;

    template<typename... Args>
    void InvokeImpl(int operationId, const Args&... args);

    // Inherited via UIManager::Delegate
    bool WindowCloseRequested(const WindowKey& key) override;
    void WindowClosed(const WindowKey& key) override;

    /////////// Local methods ///////////////
    AnyFn FindOperation(int operationId);

private:
    Engine& engine;

    std::unique_ptr<DataContext> globalContext;
    Vector<std::unique_ptr<DataContext>> contexts;
    DataContext* activeContext = nullptr;

    Vector<std::unique_ptr<ClientModule>> modules;
    ControllerModule* controllerModule = nullptr;

    Vector<DataWrapper> wrappers;
    UnorderedMap<int, AnyFn> globalOperations;

    std::unique_ptr<UIManager> uiManager;
};

template<typename... Args>
void Core::InvokeImpl(int operationId, const Args&... args)
{
    AnyFn fn = FindOperation(operationId);
    if (!fn.IsValid())
    {
        Logger::Error("Operation with ID %d has not been registered yet", operationId);
        return;
    }

    try
    {
        fn.Invoke(args...);
    }
    catch (const AnyFn::Exception& e)
    {
        Logger::Error("Operation (%d) call failed: %s", operationId, e.what());
    }
}

} // namespace TArc
} // namespace DAVA

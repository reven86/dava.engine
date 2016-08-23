#pragma once

#include "TArcCore/Private/CoreInterface.h"

#include "Base/BaseTypes.h"

#include <memory>

namespace DAVA
{
class Engine;
}

namespace tarc
{

class ClientModule;
class ControllerModule;
class UIManager;

class Core final : private CoreInterface
{
public:
    Core(DAVA::Engine& engine_);
    ~Core();

    template<typename T>
    void CreateModule()
    {
        static_assert(std::is_base_of<tarc::ClientModule, T>::value ||
                      std::is_base_of<tarc::ControllerModule, T>::value,
                      "Module should be Derived from tarc::ControllerModule or tarc::ClientModule");
        AddModule(new T());
    }

private:
    void AddModule(ClientModule* module);
    void AddModule(ControllerModule* module);

    void OnLoopStarted();
    void OnWindowCreated(DAVA::Window& w);
    void OnFrame();
    void OnLoopStopped();

    // Inherited via ContextAccessor
    void ForEachContext(const DAVA::Function<void(DataContext&)>& functor) override;
    DataContext& GetGlobalContext() override;
    DataContext& GetContext(DataContext::ContextID contextID) override;
    DataContext& GetActiveContext() override;
    bool HasActiveContext() const override;
    DataWrapper CreateWrapper(const DAVA::ReflectedType* type) override;
    DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor) override;
    DAVA::EngineContext& GetEngineContext() override;

    // Inherited via ContextManager
    DataContext::ContextID CreateContext() override;
    void DeleteContext(DataContext::ContextID contextID) override;
    void ActivateContext(DataContext::ContextID contextID) override;

    void ActivateContext(DataContext* context);
    DAVA::RenderWidget* GetRenderWidget() const;

    void RegisterOperation(int operationID, DAVA::AnyFn&& fn) override;
    void Invoke(int operationId) override;
    void Invoke(int operationId, const DAVA::Any& a) override;
    void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2) override;
    void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3) override;
    void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3, const DAVA::Any& a4) override;
    void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3, const DAVA::Any& a4, const DAVA::Any& a5) override;
    void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3, const DAVA::Any& a4, const DAVA::Any& a5, const DAVA::Any& a6) override;

    template<typename... Args>
    void InvokeImpl(int operationId, const Args&... args);

    /////////// Local methods ///////////////
    DAVA::AnyFn FindOperation(int operationId);

private:
    DAVA::Engine& engine;

    std::unique_ptr<DataContext> globalContext;
    DAVA::Vector<std::unique_ptr<DataContext>> contexts;
    DataContext* activeContext = nullptr;

    DAVA::Vector<std::unique_ptr<ClientModule>> modules;
    ControllerModule* controllerModule = nullptr;

    DAVA::Vector<DataWrapper> wrappers;
    DAVA::UnorderedMap<int, DAVA::AnyFn> globalOperations;

    std::unique_ptr<UIManager> uiManager;
};

template<typename... Args>
void Core::InvokeImpl(int operationId, const Args&... args)
{
    DAVA::AnyFn fn = FindOperation(operationId);
    if (!fn.IsValid())
    {
        DAVA::Logger::Error("Operation with ID %d has not been registered yet", operationId);
        return;
    }

    try
    {
        fn.Invoke(args...);
    }
    catch (const DAVA::AnyFn::Exception& e)
    {
        DAVA::Logger::Error("Operation (%d) call failed: %s", operationId, e.what());
    }
}

}
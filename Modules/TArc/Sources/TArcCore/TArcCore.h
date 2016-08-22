#pragma once

#include "TArcCore/ContextAccessor.h"
#include "TArcCore/ContextManager.h"

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

class Core final : private ContextAccessor, private ContextManager
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

private:
    DAVA::Engine& engine;

    std::unique_ptr<DataContext> globalContext;
    DAVA::Vector<std::unique_ptr<DataContext>> contexts;
    DataContext* activeContext = nullptr;

    DAVA::Vector<std::unique_ptr<ClientModule>> modules;
    ControllerModule* controllerModule = nullptr;

    DAVA::Vector<DataWrapper> wrappers;

    std::unique_ptr<UIManager> uiManager;
};

}
#pragma once

#include "TArcCore/ContextAccessor.h"
#include "TArcCore/ContextManager.h"

#include "Engine/EngineFwd.h"
#include "Base/BaseTypes.h"

#include <memory>

namespace tarc
{

class ClientModule;
class ControllerModule;

class Core final : private ContextAccessor, private ContextManager
{
public:
    Core(DAVA::Engine& engine_);
    ~Core();

    template<typename T>
    void CreateModule()
    {
        AddModule(new T());
    }

    template<typename T>
    void CreateControllerModule()
    {
        static_assert(std::is_base_of<tarc::ControllerModule, T>::value, "Controller modules should be Derived from tarc::ControllerModule");
        SetControllerModule(new T());
    }

private:
    void AddModule(ClientModule* module);
    void SetControllerModule(ControllerModule* module);

    void OnFrame();
    void OnLoopStarted();
    void OnLoopStopped();

    // Inherited via ContextAccessor
    void ForEachContext(const DAVA::Function<void(DataContext&)>& functor) override;
    DataContext& GetContext(DataContext::ContextID contextID) override;
    DataContext& GetActiveContext() override;
    bool HasActiveContext() const override;
    DataWrapper CreateWrapper(const DAVA::Type* type, bool listenRecursive = false) override;
    DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor, bool listenRecursive = false) override;

    // Inherited via ContextManager
    DataContext::ContextID CreateContext() override;
    void DeleteContext(DataContext::ContextID contextID) override;
    void ActivateContext(DataContext::ContextID contextID) override;

    void ActivateContext(DataContext* context);

private:
    DAVA::Engine& engine;

    DAVA::Vector<std::unique_ptr<DataContext>> contexts;
    DataContext* activeContext = nullptr;

    DAVA::Vector<ClientModule*> modules;
    ControllerModule* controllerModule = nullptr;

    DAVA::Vector<DataWrapper> wrappers;
};

}
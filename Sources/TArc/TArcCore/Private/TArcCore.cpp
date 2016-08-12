#include "TArcCore/TArcCore.h"
#include "TArcCore/ControllerModule.h"
#include "TArcCore/ClientModule.h"
#include "WindowSubSystem/Private/UIManager.h"

#include "Engine/Public/Engine.h"
#include "Engine/Public/Window.h"
#include "Engine/Public/WindowNativeService.h"
#include "Functional/Function.h"

#include "Debug/DVAssert.h"

namespace tarc
{

Core::Core(DAVA::Engine& engine_)
    : engine(engine_)
{
    engine.beginFrame.Connect(DAVA::MakeFunction(this, &Core::OnFrame));
    engine.gameLoopStarted.Connect(DAVA::MakeFunction(this, &Core::OnLoopStarted));
    engine.gameLoopStopped.Connect(DAVA::MakeFunction(this, &Core::OnLoopStopped));
}

Core::~Core()
{
    DVASSERT(modules.empty());
    DVASSERT(contexts.empty());
    DVASSERT(wrappers.empty());
}

void Core::AddModule(ClientModule* module)
{
    modules.push_back(module);
}

void Core::SetControllerModule(ControllerModule* module)
{
    DVASSERT(controllerModule == nullptr);
    controllerModule = module;
    modules.push_back(module);
}

void Core::OnFrame()
{
    for (DataWrapper& wrapper : wrappers)
    {
        wrapper.Sync(true);
    }
}

void Core::OnLoopStarted()
{
    uiManager.reset(new UIManager());
    DVASSERT_MSG(controllerModule != nullptr, "Controller Module hasn't been registered");
    controllerModule->SetContextManager(this);

    for (ClientModule* module : modules)
    {
        module->Init(this);
    }

    for (ClientModule* module : modules)
    {
        module->PostInit(*uiManager);
    }
}

void Core::OnLoopStopped()
{
    ActivateContext(nullptr);
    controllerModule = nullptr;
    for (std::unique_ptr<DataContext>& context : contexts)
    {
        for (ClientModule* module : modules)
        {
            module->OnContextDeleted(*context);
        }
    }

    contexts.clear();

    for (ClientModule* module : modules)
    {
        delete module;
    }

    modules.clear();
    wrappers.clear();
}

void Core::ForEachContext(const DAVA::Function<void(DataContext&)>& functor)
{
    for (std::unique_ptr<DataContext>& context : contexts)
    {
        functor(*context);
    }
}

DataContext& Core::GetContext(DataContext::ContextID contextID)
{
    auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
    {
        return context->GetID() == contextID;
    });

    if (iter == contexts.end())
    {
        throw std::runtime_error(DAVA::Format("There is no context with contextID %d at the moment", contextID));
    }

    return **iter;
}

DataContext& Core::GetActiveContext()
{
    if (activeContext == nullptr)
    {
        throw std::runtime_error("There is no active context at the moment");
    }

    return *activeContext;
}

bool Core::HasActiveContext() const
{
    return activeContext != nullptr;
}

DataWrapper Core::CreateWrapper(const DAVA::Type* type, bool listenRecursive)
{
    DataWrapper wrapper(type, listenRecursive);
    wrapper.SetContext(activeContext);
    wrappers.push_back(wrapper);
    return wrapper;
}

DataWrapper Core::CreateWrapper(const DataWrapper::DataAccessor& accessor, bool listenRecursive)
{
    DataWrapper wrapper(accessor, listenRecursive);
    wrapper.SetContext(activeContext);
    wrappers.push_back(wrapper);
    return wrapper;
}

DAVA::EngineContext& Core::GetEngine()
{
    DAVA::EngineContext* engineContext = engine.GetContext();
    DVASSERT(engineContext);
    return *engineContext;
}

DataContext::ContextID Core::CreateContext()
{
    contexts.push_back(std::make_unique<DataContext>());
    DataContext& context = *contexts.back();
    for (ClientModule* module : modules)
    {
        module->OnContextCreated(context);
    }

    return context.GetID();
}

void Core::DeleteContext(DataContext::ContextID contextID)
{
    auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
    {
        return context->GetID() == contextID;
    });

    if (iter == contexts.end())
    {
        throw std::runtime_error(DAVA::Format("DeleteContext failed for contextID : %d", contextID));
    }

    for (ClientModule* module : modules)
    {
        module->OnContextDeleted(**iter);
    }

    if (activeContext->GetID() == contextID)
    {
        ActivateContext(nullptr);
    }

    contexts.erase(iter);
}

void Core::ActivateContext(DataContext::ContextID contextID)
{
    if (activeContext->GetID() == contextID)
    {
        return;
    }

    if (contextID == DataContext::Empty)
    {
        ActivateContext(nullptr);
        return;
    }

    auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
    {
        return context->GetID() == contextID;
    });

    if (iter == contexts.end())
    {
        throw std::runtime_error(DAVA::Format("ActivateContext failed for contextID : %d", contextID));
    }

    ActivateContext((*iter).get());
}

void Core::ActivateContext(DataContext* context)
{
    activeContext = context;
    for (DataWrapper& wrapper : wrappers)
    {
        wrapper.SetContext(activeContext);
    }
}

DAVA::RenderWidget* Core::GetRenderWidget() const
{
    return engine.PrimaryWindow()->GetNativeService()->GetRenderWidget();
}

}

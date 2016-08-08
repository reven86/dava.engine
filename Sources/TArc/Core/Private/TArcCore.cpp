#include "Core/TArcCore.h"
#include "Core/ControllerModule.h"
#include "Core/ClientModule.h"

#include "Engine/Engine.h"
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

void Core::AddModule(std::unique_ptr<ClientModule>&& module)
{
    modules.push_back(std::move(module));
}

void Core::SetControllerModule(std::unique_ptr<ControllerModule>&& module)
{
    DVASSERT(controllerModule == nullptr);
    controllerModule = module.get();
    modules.push_back(std::move(module));
}

void Core::OnFrame()
{
}

void Core::OnLoopStarted()
{
    DVASSERT_MSG(controllerModule != nullptr, "Controller Module hasn't been registered");
    controllerModule->SetContextManager(this);

    for (std::unique_ptr<ClientModule>& module : modules)
    {
        module->Init(this);
    }
}

void Core::OnLoopStopped()
{
    ActivateContext(nullptr);
    controllerModule = nullptr;
    for (std::unique_ptr<DataContext>& context : contexts)
    {
        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->OnContextDeleted(*context);
        }
    }

    contexts.clear();
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
    auto iter = std::find(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
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

DataWrapper Core::CreateWrapper(const DAVA::Type* type)
{
    return DataWrapper();
}

DataContext::ContextID Core::CreateContext()
{
    contexts.push_back(std::make_unique<DataContext>());
    DataContext& context = *contexts.back();
    for (std::unique_ptr<ClientModule>& module : modules)
    {
        module->OnContextCreated(context);
    }

    return context.GetID();
}

void Core::DeleteContext(DataContext::ContextID contextID)
{
    auto iter = std::find(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
    {
        return context->GetID() == contextID;
    });

    if (iter == contexts.end())
    {
        throw std::runtime_error(DAVA::Format("DeleteContext failed for contextID : %d", contextID));
    }

    for (std::unique_ptr<ClientModule>& module : modules)
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
    auto iter = std::find(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
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

}

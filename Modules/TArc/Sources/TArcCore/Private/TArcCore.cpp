#include "TArcCore/TArcCore.h"
#include "TArcCore/ControllerModule.h"
#include "TArcCore/ClientModule.h"
#include "DataProcessing/PropertiesHolder.h"
#include "WindowSubSystem/Private/UIManager.h"
#include "TArcUtils/AssertGuard.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/NativeService.h"
#include "Functional/Function.h"
#include "FileSystem/FileSystem.h"

#include "Debug/DVAssert.h"

#include <QApplication>

namespace DAVA
{
namespace TArc
{
Core::Core(Engine& engine_)
    : engine(engine_)
    , globalContext(new DataContext())
{
    engine.beginFrame.Connect(MakeFunction(this, &Core::OnFrame));
    engine.gameLoopStarted.Connect(MakeFunction(this, &Core::OnLoopStarted));
    engine.gameLoopStopped.Connect(MakeFunction(this, &Core::OnLoopStopped));
    engine.windowCreated.Connect(MakeFunction(this, &Core::OnWindowCreated));
}

Core::~Core()
{
    DVASSERT(modules.empty());
    DVASSERT(contexts.empty());
    DVASSERT(wrappers.empty());
}

void Core::AddModule(ClientModule* module)
{
    modules.emplace_back(module);
}

void Core::AddModule(ControllerModule* module)
{
    DVASSERT(controllerModule == nullptr);
    controllerModule = module;
    AddModule(static_cast<ClientModule*>(module));
}

void Core::OnWindowCreated(Window& w)
{
    controllerModule->OnRenderSystemInitialized(w);
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
    ToolsAssetGuard::Instance()->Init();
    engine.GetNativeService()->GetApplication()->setWindowIcon(QIcon(":/icons/appIcon.ico"));
    FileSystem* fileSystem = GetEngineContext().fileSystem;
    DVASSERT(fileSystem != nullptr);
    propertiesHolder.reset(new PropertiesHolder("TArc properties", fileSystem->GetCurrentDocumentsDirectory()));
    uiManager.reset(new UIManager(this, propertiesHolder->CreateSubHolder("UIManager")));
    DVASSERT_MSG(controllerModule != nullptr, "Controller Module hasn't been registered");

    for (std::unique_ptr<ClientModule>& module : modules)
    {
        module->Init(this, uiManager.get());
    }

    for (std::unique_ptr<ClientModule>& module : modules)
    {
        module->PostInit();
    }
    uiManager->InitializationFinished();
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
    uiManager.reset();
    globalContext.reset();
}

void Core::ForEachContext(const Function<void(DataContext&)>& functor)
{
    for (std::unique_ptr<DataContext>& context : contexts)
    {
        functor(*context);
    }
}

DataContext& Core::GetGlobalContext()
{
    return *globalContext;
}

DataContext& Core::GetContext(DataContext::ContextID contextID)
{
    auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
                             {
                                 return context->GetID() == contextID;
                             });

    if (iter == contexts.end())
    {
        throw std::runtime_error(Format("There is no context with contextID %d at the moment", contextID));
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

DataWrapper Core::CreateWrapper(const ReflectedType* type)
{
    DataWrapper wrapper(type);
    wrapper.SetContext(activeContext != nullptr ? activeContext : globalContext.get());
    wrappers.push_back(wrapper);
    return wrapper;
}

DataWrapper Core::CreateWrapper(const DataWrapper::DataAccessor& accessor)
{
    DataWrapper wrapper(accessor);
    wrapper.SetContext(activeContext != nullptr ? activeContext : globalContext.get());
    wrappers.push_back(wrapper);
    return wrapper;
}

EngineContext& Core::GetEngineContext()
{
    EngineContext* engineContext = engine.GetContext();
    DVASSERT(engineContext);
    return *engineContext;
}

DataContext::ContextID Core::CreateContext()
{
    contexts.push_back(std::make_unique<DataContext>(globalContext.get()));
    DataContext& context = *contexts.back();
    for (std::unique_ptr<ClientModule>& module : modules)
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
        throw std::runtime_error(Format("DeleteContext failed for contextID : %d", contextID));
    }

    for (std::unique_ptr<ClientModule>& module : modules)
    {
        module->OnContextDeleted(**iter);
    }

    if (activeContext != nullptr && activeContext->GetID() == contextID)
    {
        ActivateContext(nullptr);
    }

    contexts.erase(iter);
}

void Core::ActivateContext(DataContext::ContextID contextID)
{
    if (activeContext != nullptr && activeContext->GetID() == contextID)
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
        throw std::runtime_error(Format("ActivateContext failed for contextID : %d", contextID));
    }

    ActivateContext((*iter).get());
}

void Core::ActivateContext(DataContext* context)
{
    activeContext = context;
    DataContext* wrapperActiveContext = activeContext != nullptr ? activeContext : globalContext.get();
    for (DataWrapper& wrapper : wrappers)
    {
        wrapper.SetContext(wrapperActiveContext);
    }
}

RenderWidget* Core::GetRenderWidget() const
{
    return engine.GetNativeService()->GetRenderWidget();
}

void Core::RegisterOperation(int operationID, AnyFn&& fn)
{
    auto iter = globalOperations.find(operationID);
    if (iter != globalOperations.end())
    {
        Logger::Error("Global operation with ID %d, has already been registered", operationID);
        return;
    }

    globalOperations.emplace(operationID, fn);
}

void Core::Invoke(int operationId)
{
    InvokeImpl(operationId);
}

void Core::Invoke(int operationId, const Any& a)
{
    InvokeImpl(operationId, a);
}
void Core::Invoke(int operationId, const Any& a1, const Any& a2)
{
    InvokeImpl(operationId, a1, a2);
}

void Core::Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3)
{
    InvokeImpl(operationId, a1, a2, a3);
}

void Core::Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4)
{
    InvokeImpl(operationId, a1, a2, a3, a4);
}

void Core::Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5)
{
    InvokeImpl(operationId, a1, a2, a3, a4, a5);
}

void Core::Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6)
{
    InvokeImpl(operationId, a1, a2, a3, a4, a5, a6);
}

AnyFn Core::FindOperation(int operationId)
{
    AnyFn operation;
    auto iter = globalOperations.find(operationId);
    if (iter != globalOperations.end())
    {
        operation = iter->second;
    }

    return operation;
}

bool Core::WindowCloseRequested(const WindowKey& key)
{
    DVASSERT(controllerModule != nullptr);
    bool result = true;
    if (controllerModule->CanWindowBeClosedSilently(key) == false)
    {
        ModalMessageParams params;
        params.buttons = ModalMessageParams::Buttons(ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Cancel);
        params.message = "Some files have been modified\nDo you want to save changes?";
        params.title = "Save Changes?";
        ModalMessageParams::Button resultButton = uiManager->ShowModalMessage(key, params);
        if (resultButton == ModalMessageParams::Yes)
        {
            controllerModule->SaveOnWindowClose(key);
        }
        else if (resultButton == ModalMessageParams::No)
        {
            controllerModule->RestoreOnWindowClose(key);
        }
        else
        {
            result = false;
        }
    }

    return result;
}

void Core::WindowClosed(const WindowKey& key)
{
    std::for_each(modules.begin(), modules.end(), [&key](std::unique_ptr<ClientModule>& module)
                  {
                      module->OnWindowClosed(key);
                  });
}

} // namespace TArc
} // namespace DAVA

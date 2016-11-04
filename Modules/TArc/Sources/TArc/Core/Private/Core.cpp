#include "TArc/Core/Core.h"
#include "TArc/Core/ControllerModule.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Core/ConsoleModule.h"
#include "TArc/Core/Private/MacOSUtils.h"

#include "TArc/DataProcessing/PropertiesHolder.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/Utils/AssertGuard.h"
#include "QtTools/Utils/QtDelayedExecutor.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/NativeService.h"
#include "Engine/EngineContext.h"

#include "Functional/Function.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/RenderHelper.h"
#include "UI/UIControlSystem.h"

#include "Render/Renderer.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"

#include <QApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace DAVA
{
namespace TArc
{
class Core::Impl : public CoreInterface
{
public:
    Impl(Engine& engine_, Core* core_)
        : engine(engine_)
        , core(core_)
        , globalContext(new DataContext())
    {
    }

    ~Impl()
    {
        DVASSERT(contexts.empty());
        DVASSERT(wrappers.empty());
    }

    virtual void AddModule(ConsoleModule* module)
    {
        DVASSERT(false);
    }

    virtual void AddModule(ClientModule* module)
    {
        DVASSERT(false);
    }

    virtual void AddModule(ControllerModule* module)
    {
        DVASSERT(false);
    }

    bool IsConsoleMode() const
    {
        return engine.IsConsoleMode();
    }

    virtual void OnLoopStarted()
    {
        FileSystem* fileSystem = GetEngineContext()->fileSystem;
        DVASSERT(fileSystem != nullptr);
        propertiesHolder.reset(new PropertiesHolder("TArcProperties", fileSystem->GetCurrentDocumentsDirectory()));
    }

    virtual void OnLoopStopped()
    {
        contexts.clear();
        wrappers.clear();
        globalContext.reset();
    }

    virtual void OnFrame(DAVA::float32 delta)
    {
        if (isInFrame)
        {
            return;
        }

        isInFrame = true;
        SCOPE_EXIT
        {
            isInFrame = false;
        };
        delayedExecutor.DelayedExecute(DAVA::Function<void(void)>(this, &Core::Impl::SyncWrappers));
    }

    virtual void OnWindowCreated(DAVA::Window* w)
    {
    }

    void ForEachContext(const Function<void(DataContext&)>& functor) override
    {
        for (std::unique_ptr<DataContext>& context : contexts)
        {
            functor(*context);
        }
    }

    DataContext* GetGlobalContext() override
    {
        return globalContext.get();
    }

    DataContext* GetContext(DataContext::ContextID contextID) override
    {
        auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const std::unique_ptr<DataContext>& context)
                                 {
                                     return context->GetID() == contextID;
                                 });

        if (iter == contexts.end())
        {
            return nullptr;
        }

        return iter->get();
    }

    DataContext* GetActiveContext() override
    {
        return activeContext;
    }

    DataWrapper CreateWrapper(const ReflectedType* type) override
    {
        DataWrapper wrapper(type);
        wrapper.SetContext(activeContext != nullptr ? activeContext : globalContext.get());
        wrappers.push_back(wrapper);
        return wrapper;
    }

    DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor) override
    {
        DataWrapper wrapper(accessor);
        wrapper.SetContext(activeContext != nullptr ? activeContext : globalContext.get());
        wrappers.push_back(wrapper);
        return wrapper;
    }

    PropertiesItem CreatePropertiesNode(const String& nodeName)
    {
        DVASSERT(propertiesHolder != nullptr);
        return propertiesHolder->CreateSubHolder(nodeName);
    }

    EngineContext* GetEngineContext() override
    {
        EngineContext* engineContext = engine.GetContext();
        DVASSERT(engineContext);
        return engineContext;
    }

protected:
    void ActivateContextImpl(DataContext* context)
    {
        activeContext = context;
        for (DataWrapper& wrapper : wrappers)
        {
            wrapper.SetContext(activeContext != nullptr ? activeContext : globalContext.get());
        }
    }

    void SyncWrappers()
    {
        size_t index = 0;
        while (index < wrappers.size())
        {
            if (!wrappers[index].IsActive())
            {
                DAVA::RemoveExchangingWithLast(wrappers, index);
            }
            else
            {
                ++index;
            }
        }
        for (DataWrapper& wrapper : wrappers)
        {
            wrapper.Sync(true);
        }
    }

protected:
    Engine& engine;
    Core* core;

    std::unique_ptr<DataContext> globalContext;
    Vector<std::unique_ptr<DataContext>> contexts;
    DataContext* activeContext = nullptr;
    Vector<DataWrapper> wrappers;
    bool isInFrame = false;

    std::unique_ptr<PropertiesHolder> propertiesHolder;
    QtDelayedExecutor delayedExecutor;
};

class Core::ConsoleImpl : public Core::Impl
{
public:
    ConsoleImpl(Engine& engine, Core* core)
        : Impl(engine, core)
    {
        DVASSERT(engine.IsConsoleMode() == true);
        argv = engine.GetCommandLineAsArgv();
        argc = static_cast<int>(argv.size());
    }

    ~ConsoleImpl()
    {
        delete context;
        delete surface;
        delete application;
    }

    void OnLoopStarted() override
    {
        Impl::OnLoopStarted();
        application = new QGuiApplication(argc, argv.data());
        surface = new QOffscreenSurface();
        surface->create();

        context = new QOpenGLContext();
        if (context->create() == false)
        {
            throw std::runtime_error("OGL context creation failed");
        }

        if (context->makeCurrent(surface) == false)
        {
            throw std::runtime_error("MakeCurrent for offscreen surface failed");
        }

        rhi::Api renderer = rhi::RHI_GLES2;
        rhi::InitParam rendererParams;
        rendererParams.threadedRenderFrameCount = 1;
        rendererParams.threadedRenderEnabled = false;
        rendererParams.acquireContextFunc = []() {};
        rendererParams.releaseContextFunc = []() {};

        const DAVA::KeyedArchive* options = engine.GetOptions();

        rendererParams.maxIndexBufferCount = options->GetInt32("max_index_buffer_count");
        rendererParams.maxVertexBufferCount = options->GetInt32("max_vertex_buffer_count");
        rendererParams.maxConstBufferCount = options->GetInt32("max_const_buffer_count");
        rendererParams.maxTextureCount = options->GetInt32("max_texture_count");

        rendererParams.maxTextureSetCount = options->GetInt32("max_texture_set_count");
        rendererParams.maxSamplerStateCount = options->GetInt32("max_sampler_state_count");
        rendererParams.maxPipelineStateCount = options->GetInt32("max_pipeline_state_count");
        rendererParams.maxDepthStencilStateCount = options->GetInt32("max_depthstencil_state_count");
        rendererParams.maxRenderPassCount = options->GetInt32("max_render_pass_count");
        rendererParams.maxCommandBuffer = options->GetInt32("max_command_buffer_count");
        rendererParams.maxPacketListCount = options->GetInt32("max_packet_list_count");

        rendererParams.shaderConstRingBufferSize = options->GetInt32("shader_const_buffer_size");

        rendererParams.window = nullptr;
        rendererParams.width = 1024;
        rendererParams.height = 768;
        rendererParams.scaleX = 1.0f;
        rendererParams.scaleY = 1.0f;
        Renderer::Initialize(renderer, rendererParams);

        EngineContext* engineContext = engine.GetContext();
        VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
        vcs->SetInputScreenAreaSize(rendererParams.width, rendererParams.height);
        vcs->SetPhysicalScreenSize(rendererParams.width, rendererParams.height);
        vcs->SetVirtualScreenSize(rendererParams.width, rendererParams.height);
        vcs->UnregisterAllAvailableResourceSizes();
        vcs->RegisterAvailableResourceSize(rendererParams.width, rendererParams.height, "Gfx");
        vcs->ScreenSizeChanged();

        Texture::SetGPULoadingOrder({ GPU_ORIGIN });

        ActivateContextImpl(globalContext.get());
        for (std::unique_ptr<ConsoleModule>& module : modules)
        {
            module->Init(this);
        }

        RhiEmptyFrame frame;
        for (std::unique_ptr<ConsoleModule>& module : modules)
        {
            module->PostInit();
        }
    }

    void OnFrame(DAVA::float32 delta) override
    {
        context->makeCurrent(surface);
        Impl::OnFrame(delta);
        {
            RhiEmptyFrame frame;
            if (modules.front()->OnFrame() == ConsoleModule::eFrameResult::FINISHED)
            {
                modules.front()->BeforeDestroyed();
                modules.pop_front();
            }

            if (modules.empty() == true)
            {
                engine.Quit(0);
            }
        }
        context->swapBuffers(surface);
    }

    void OnLoopStopped() override
    {
        DVASSERT(modules.empty());
        ActivateContextImpl(nullptr);

        rhi::ResetParam rendererParams;
        rendererParams.window = nullptr;
        rendererParams.width = 0;
        rendererParams.height = 0;
        rendererParams.scaleX = 1.f;
        rendererParams.scaleY = 1.f;
        Renderer::Reset(rendererParams);

        context->doneCurrent();
        surface->destroy();

        Impl::OnLoopStopped();
    }

    void AddModule(ConsoleModule* module) override
    {
        modules.push_back(std::unique_ptr<ConsoleModule>(module));
    }

private:
    void RegisterOperation(int operationID, AnyFn&& fn) override
    {
    }
    DataContext::ContextID CreateContext() override
    {
        DVASSERT(false);
        return DataContext::ContextID();
    }
    void DeleteContext(DataContext::ContextID contextID) override
    {
    }
    void ActivateContext(DataContext::ContextID contextID) override
    {
    }
    RenderWidget* GetRenderWidget() const override
    {
        return nullptr;
    }

    void Invoke(int operationId) override
    {
    }
    void Invoke(int operationId, const Any& a) override
    {
    }
    void Invoke(int operationId, const Any& a1, const Any& a2) override
    {
    }
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3) override
    {
    }
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) override
    {
    }
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) override
    {
    }
    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) override
    {
    }

private:
    DAVA::Deque<std::unique_ptr<ConsoleModule>> modules;
    QGuiApplication* application = nullptr;
    QOffscreenSurface* surface = nullptr;
    QOpenGLContext* context = nullptr;
    int argc = 0;
    Vector<char*> argv;
};

class Core::GuiImpl : public Core::Impl, public UIManager::Delegate
{
public:
    GuiImpl(Engine& engine, Core* core)
        : Impl(engine, core)
    {
#if defined(__DAVAENGINE_MACOS__)
        MakeAppForeground();
        FixOSXFonts();
#endif
        DVASSERT(engine.IsConsoleMode() == false);
    }

    ~GuiImpl()
    {
        DVASSERT(modules.empty());
    }

    void AddModule(ClientModule* module) override
    {
        modules.emplace_back(module);
    }

    void AddModule(ControllerModule* module) override
    {
        DVASSERT(controllerModule == nullptr);
        controllerModule = module;
        AddModule(static_cast<ClientModule*>(module));
    }

    void OnLoopStarted() override
    {
        Impl::OnLoopStarted();

        ToolsAssetGuard::Instance()->Init();

        engine.GetNativeService()->GetApplication()->setWindowIcon(QIcon(":/icons/appIcon.ico"));
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
#if defined(__DAVAENGINE_MACOS__)
        RestoreMenuBar();
#endif
    }

    void OnFrame(DAVA::float32 delta) override
    {
        Impl::OnFrame(delta);
    }

    void OnLoopStopped() override
    {
        ActivateContextImpl(nullptr);
        controllerModule = nullptr;
        for (std::unique_ptr<DataContext>& context : contexts)
        {
            for (std::unique_ptr<ClientModule>& module : modules)
            {
                module->OnContextDeleted(*context);
            }
        }
        modules.clear();
        uiManager.reset();

        Impl::OnLoopStopped();
    }

    void OnWindowCreated(Window* w) override
    {
        Impl::OnWindowCreated(w);
        controllerModule->OnRenderSystemInitialized(w);
    }

    void RegisterOperation(int operationID, AnyFn&& fn) override
    {
        auto iter = globalOperations.find(operationID);
        if (iter != globalOperations.end())
        {
            Logger::Error("Global operation with ID %d, has already been registered", operationID);
        }

        globalOperations.emplace(operationID, fn);
    }

    DataContext::ContextID CreateContext() override
    {
        contexts.push_back(std::make_unique<DataContext>(globalContext.get()));
        DataContext& context = *contexts.back();
        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->OnContextCreated(context);
        }

        return context.GetID();
    }

    void DeleteContext(DataContext::ContextID contextID) override
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
            ActivateContextImpl(nullptr);
        }

        contexts.erase(iter);
    }

    void ActivateContext(DataContext::ContextID contextID) override
    {
        if (activeContext != nullptr && activeContext->GetID() == contextID)
        {
            return;
        }

        if (contextID == DataContext::Empty)
        {
            ActivateContextImpl(nullptr);
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

        ActivateContextImpl((*iter).get());
    }

    RenderWidget* GetRenderWidget() const override
    {
        return engine.GetNativeService()->GetRenderWidget();
    }

    void Invoke(int operationId) override
    {
        InvokeImpl(operationId);
    }

    void Invoke(int operationId, const Any& a) override
    {
        InvokeImpl(operationId, a);
    }
    void Invoke(int operationId, const Any& a1, const Any& a2) override
    {
        InvokeImpl(operationId, a1, a2);
    }

    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3) override
    {
        InvokeImpl(operationId, a1, a2, a3);
    }

    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) override
    {
        InvokeImpl(operationId, a1, a2, a3, a4);
    }

    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) override
    {
        InvokeImpl(operationId, a1, a2, a3, a4, a5);
    }

    void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) override
    {
        InvokeImpl(operationId, a1, a2, a3, a4, a5, a6);
    }

    template <typename... Args>
    void InvokeImpl(int operationId, const Args&... args)
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
        catch (const DAVA::Exception& e)
        {
            Logger::Error("Operation (%d) call failed: %s", operationId, e.what());
        }
    }

    bool WindowCloseRequested(const WindowKey& key) override
    {
        DVASSERT(controllerModule != nullptr);
        bool result = true;
        QCloseEvent closeEvent;
        if (controllerModule->ControlWindowClosing(key, &closeEvent))
        {
            result = closeEvent.isAccepted();
        }
        else if (controllerModule->CanWindowBeClosedSilently(key) == false)
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

    void OnWindowClosed(const WindowKey& key) override
    {
        std::for_each(modules.begin(), modules.end(), [&key](std::unique_ptr<ClientModule>& module)
                      {
                          module->OnWindowClosed(key);
                      });
    }

    bool HasControllerModule() const
    {
        return controllerModule != nullptr;
    }

private:
    AnyFn FindOperation(int operationId)
    {
        AnyFn operation;
        auto iter = globalOperations.find(operationId);
        if (iter != globalOperations.end())
        {
            operation = iter->second;
        }

        return operation;
    }

private:
    Vector<std::unique_ptr<ClientModule>> modules;
    ControllerModule* controllerModule = nullptr;

    UnorderedMap<int, AnyFn> globalOperations;

    std::unique_ptr<UIManager> uiManager;
};

Core::Core(Engine& engine)
    : Core(engine, true)
{
}

Core::Core(Engine& engine, bool connectSignals)
{
    if (engine.IsConsoleMode())
    {
        impl.reset(new ConsoleImpl(engine, this));
    }
    else
    {
        impl.reset(new GuiImpl(engine, this));
    }

    if (connectSignals)
    {
        engine.update.Connect(this, &Core::OnFrame);
        engine.gameLoopStarted.Connect(this, &Core::OnLoopStarted);
        engine.gameLoopStopped.Connect(this, &Core::OnLoopStopped);
        engine.windowCreated.Connect(this, &Core::OnWindowCreated);
    }
}

Core::~Core() = default;

EngineContext* Core::GetEngineContext()
{
    return impl->GetEngineContext();
}

CoreInterface* Core::GetCoreInterface()
{
    return impl.get();
}

bool Core::IsConsoleMode() const
{
    return impl->IsConsoleMode();
}

void Core::AddModule(ConsoleModule* module)
{
    impl->AddModule(module);
}

void Core::AddModule(ClientModule* module)
{
    impl->AddModule(module);
}

void Core::AddModule(ControllerModule* module)
{
    impl->AddModule(module);
}

void Core::OnLoopStarted()
{
    DVASSERT(impl);
    impl->OnLoopStarted();
}

void Core::OnLoopStopped()
{
    DVASSERT(impl);
    impl->OnLoopStopped();
}

void Core::OnFrame(float32 delta)
{
    DVASSERT(impl);
    impl->OnFrame(delta);
}

void Core::OnWindowCreated(DAVA::Window* w)
{
    DVASSERT(impl);
    impl->OnWindowCreated(w);
}

bool Core::HasControllerModule() const
{
    GuiImpl* guiImpl = dynamic_cast<GuiImpl*>(impl.get());
    return guiImpl != nullptr && guiImpl->HasControllerModule();
}

OperationInvoker* Core::GetMockInvoker()
{
    return nullptr;
}

DataContext* Core::GetActiveContext()
{
    DVASSERT(impl);
    return impl->GetActiveContext();
}

DataContext* Core::GetGlobalContext()
{
    DVASSERT(impl);
    return impl->GetGlobalContext();
}

DataWrapper Core::CreateWrapper(const DAVA::ReflectedType* type)
{
    GuiImpl* guiImpl = dynamic_cast<GuiImpl*>(impl.get());
    DVASSERT(guiImpl != nullptr);
    return guiImpl->CreateWrapper(type);
}

} // namespace TArc
} // namespace DAVA

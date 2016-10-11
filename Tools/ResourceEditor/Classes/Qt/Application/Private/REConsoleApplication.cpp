#include "Classes/Qt/Application/REConsoleApplication.h"

#include "CommandLine/WinConsoleIOLocker.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"
#include "Debug/DVAssert.h"

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "Render/Renderer.h"
#include "Engine/EngineContext.h"

REConsoleApplication::REConsoleApplication(CommandLineManager& cmdLineManager_)
    : cmdLineManager(cmdLineManager_)
{
}

void REConsoleApplication::OnLoopStarted()
{
    REBaseApplication::OnLoopStarted();
    DVASSERT(engine.IsConsoleMode() == true);

#if defined(__DAVAENGINE_WIN32__)
    locker = new WinConsoleIOLocker();
#endif

    argv = engine.GetCommandLineAsArgv();
    argc = static_cast<int>(argv.size());
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

    rendererParams.maxIndexBufferCount = 0;
    rendererParams.maxVertexBufferCount = 0;
    rendererParams.maxConstBufferCount = 0;
    rendererParams.maxTextureCount = 0;

    rendererParams.maxTextureSetCount = 0;
    rendererParams.maxSamplerStateCount = 0;
    rendererParams.maxPipelineStateCount = 0;
    rendererParams.maxDepthStencilStateCount = 0;
    rendererParams.maxRenderPassCount = 0;
    rendererParams.maxCommandBuffer = 0;
    rendererParams.maxPacketListCount = 0;

    rendererParams.shaderConstRingBufferSize = 0;

    rendererParams.window = nullptr;
    rendererParams.width = 1024;
    rendererParams.height = 768;
    rendererParams.scaleX = 1.0f;
    rendererParams.scaleY = 1.0f;
    DAVA::Renderer::Initialize(renderer, rendererParams);

    DAVA::EngineContext* engineContext = engine.GetContext();
    engineContext->virtualCoordSystem->UnregisterAllAvailableResourceSizes();
    engineContext->virtualCoordSystem->RegisterAvailableResourceSize(1, 1, "Gfx");

    engineContext->logger->EnableConsoleMode();
    engineContext->logger->SetLogLevel(DAVA::Logger::LEVEL_INFO);

    DAVA::Texture::SetGPULoadingOrder({ DAVA::eGPUFamily::GPU_ORIGIN });
}

void REConsoleApplication::OnUpdate(DAVA::float32 delta)
{
    engine.Quit(0);
}

void REConsoleApplication::OnLoopStopped()
{
    rhi::ResetParam rendererParams;
    rendererParams.window = nullptr;
    rendererParams.width = 0;
    rendererParams.height = 0;
    rendererParams.scaleX = 1.f;
    rendererParams.scaleY = 1.f;
    DAVA::Renderer::Reset(rendererParams);

    context->doneCurrent();
    surface->destroy();

    DAVA::SafeDelete(context);
    DAVA::SafeDelete(surface);
    DAVA::SafeDelete(application);
#if defined(__DAVAENGINE_WIN32__)
    DAVA::SafeDelete(locker);
#endif

    argv.clear();

    REBaseApplication::OnLoopStopped();
}

DAVA::eEngineRunMode REConsoleApplication::GetEngineMode() const
{
    return DAVA::eEngineRunMode::CONSOLE_MODE;
}

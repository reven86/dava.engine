#include "REApplication.h"

#include "version.h"
#include "Main/mainwindow.h"
#include "ResourceEditorLauncher.h"
#include "Settings/SettingsManager.h"
#include "TextureCompression/PVRConverter.h"
#include "TextureCache.h"
#include "CommandLine/CommandLineManager.h"
#include "Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "CommandLine/WinConsoleIOLocker.h"

#include "QtTools/RunGuard/RunGuard.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/Utils/Utils.h"
#include "QtTools/Utils/AssertGuard.h"

#include "Preferences/PreferencesStorage.h"

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/ControlsFactory.h"

#include "Engine/Public/Engine.h"
#include "Engine/Public/EngineContext.h"
#include "Engine/Public/NativeService.h"
#include "Engine/Public/Window.h"

#include "Core/PerformanceSettings.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace REAppDetail
{
class REApp
{
public:
    REApp(DAVA::Engine& e)
        : engine(e)
    {
    }

    void Init()
    {
        engine.SetOptions(GetEngineOptions());
        engine.Init(GetEngineMode(), GetEngineModules());
#if defined(__DAVAENGINE_MACOS__)
        const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
        const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
        DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);

        DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
        DAVA::QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);
        DAVA::QualitySettingsSystem::Instance()->SetMetalPreview(true);
        DAVA::QualitySettingsSystem::Instance()->SetRuntimeQualitySwitching(true);
        DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

        engine.gameLoopStarted.Connect(this, &REApp::OnLoopStarted);
        engine.update.Connect(this, &REApp::OnUpdate);
        engine.gameLoopStopped.Connect(this, &REApp::OnLoopStopped);
        engine.windowCreated.Connect(this, &REApp::OnWindowCreated);
    }

    virtual void OnLoopStarted()
    {
        QApplication* app = engine.GetNativeService()->GetApplication();
        app->setOrganizationName("DAVA");
        app->setApplicationName("Resource Editor");

        config = new EditorConfig();
        settingsManager = new SettingsManager();
        sceneValidator = new SceneValidator();

        DAVA::EngineContext* engineContext = engine.GetContext();
        const char* settingsPath = "ResourceEditorSettings.archive";
        DAVA::FilePath localPrefrencesPath(engineContext->fileSystem->GetCurrentDocumentsDirectory() + settingsPath);
        PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);
        SettingsManager::UpdateGPUSettings();
        DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

        engineContext->virtualCoordSystem->EnableReloadResourceOnResize(false);
        engineContext->performanceSettings->SetPsPerformanceMinFPS(5.0f);
        engineContext->performanceSettings->SetPsPerformanceMaxFPS(10.0f);
    }

    virtual void OnUpdate(DAVA::float32 delta)
    {
    }

    virtual void OnLoopStopped()
    {
        config->Release();
        settingsManager->Release();
        sceneValidator->Release();

        VisibilityCheckSystem::ReleaseCubemapRenderTargets();
    }

    virtual void OnWindowCreated(DAVA::Window& w)
    {
    }

    virtual DAVA::KeyedArchive* GetEngineOptions()
    {
        DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

        auto title = DAVA::Format("DAVA Framework - ResourceEditor | %s.%s [%u bit]", DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION,
                                  static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));
        appOptions->SetString("title", title);

        appOptions->SetInt32("fullscreen", 0);
        appOptions->SetInt32("bpp", 32);
        appOptions->SetInt32("renderer", rhi::RHI_GLES2);
        appOptions->SetInt32("width", 1024);
        appOptions->SetInt32("height", 768);

        appOptions->SetInt32("max_index_buffer_count", 16384);
        appOptions->SetInt32("max_vertex_buffer_count", 16384);
        appOptions->SetInt32("max_const_buffer_count", 32767);
        appOptions->SetInt32("max_texture_count", 2048);

        appOptions->SetInt32("shader_const_buffer_size", 256 * 1024 * 1024);

        return appOptions;
    }

    DAVA::Vector<DAVA::String> GetEngineModules()
    {
        DAVA::Vector<DAVA::String> modules = {
            "JobManager",
            "NetCore",
            "LocalizationSystem",
            "SoundSystem",
            "DownloadManager",
        };

        return modules;
    }

    virtual DAVA::eEngineRunMode GetEngineMode() = 0;

protected:
    DAVA::Engine& engine;

private:
#ifdef __DAVAENGINE_BEAST__
    BeastProxyImpl beastProxy;
#else
    BeastProxy beastProxy;
#endif //__DAVAENGINE_BEAST__
    EditorConfig* config;
    SettingsManager* settingsManager;
    SceneValidator* sceneValidator;
};

class REConsoleApp : public REApp
{
public:
    REConsoleApp(DAVA::Engine& e, CommandLineManager& cmdLineManager_)
        : REApp(e)
        , cmdLineManager(cmdLineManager_)
    {
    }

    ~REConsoleApp()
    {
        delete context;
        delete surface;
        delete application;
    }

    void OnLoopStarted() override
    {
        DVASSERT(engine.IsConsoleMode() == true);
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

        DAVA::Logger::Instance()->EnableConsoleMode();
        DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);

        DAVA::Texture::SetDefaultGPU(DAVA::eGPUFamily::GPU_ORIGIN);
    }

    void OnUpdate(DAVA::float32 delta)
    {
        cmdLineManager.Process();
        engine.Quit(0);
    }

    void OnLoopStopped() override
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
    }

    DAVA::eEngineRunMode GetEngineMode() override
    {
        return DAVA::eEngineRunMode::CONSOLE_MODE;
    }

private:
#if defined(__DAVAENGINE_WIN32__)
    WinConsoleIOLocker locker;
#endif //platforms
    CommandLineManager& cmdLineManager;
    QGuiApplication* application = nullptr;
    QOffscreenSurface* surface = nullptr;
    QOpenGLContext* context = nullptr;
    int argc = 0;
    DAVA::Vector<char*> argv;
};

class REGuiApp : public REApp
{
public:
    REGuiApp(DAVA::Engine& e)
        : REApp(e)
    {
        FixOSXFonts();
        MakeAppForeground();
    }

    void OnLoopStarted() override
    {
        REApp::OnLoopStarted();

        textureCache = new TextureCache();

        ToolsAssetGuard::Instance()->Init();
        Themes::InitFromQApplication();
        Q_INIT_RESOURCE(QtToolsResources);

        DAVA::EngineContext* engineContext = engine.GetContext();
        engineContext->localizationSystem->InitWithDirectory("~res:/Strings/");
        engineContext->localizationSystem->SetCurrentLocale("en");
        engineContext->uiControlSystem->SetClearColor(DAVA::Color(.3f, .3f, .3f, 1.f));
        UnpackHelpDoc();

        mainWindow = new QtMainWindow();
        mainWindow->EnableGlobalTimeout(true);
        mainWindow->InjectRenderWidget(engine.GetNativeService()->GetRenderWidget());
        mainWindow->show();
    }

    void OnLoopStopped()
    {
        DAVA::SafeDelete(mainWindow);
        ControlsFactory::ReleaseFonts();
        textureCache->Release();

        REApp::OnLoopStopped();
    }

    void OnWindowCreated(DAVA::Window& w) override
    {
        DAVA::Renderer::SetDesiredFPS(60);
        DAVA::DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

        DVASSERT(mainWindow != nullptr);
        QObject::connect(&launcher, &ResourceEditorLauncher::LaunchFinished, [this]()
                         {
                             mainWindow->SetupTitle();
                             mainWindow->OnSceneNew();
                         });
        launcher.Launch();

        REApp::OnWindowCreated(w);
    }

    DAVA::eEngineRunMode GetEngineMode() override
    {
        return DAVA::eEngineRunMode::GUI_EMBEDDED;
    }

private:
    void UnpackHelpDoc()
    {
        DAVA::EngineContext* engineContext = engine.GetContext();
        DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
        DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
        if (editorVer != APPLICATION_BUILD_VERSION || !engineContext->fileSystem->Exists(docsPath))
        {
            DAVA::Logger::FrameworkDebug("Unpacking Help...");
            try
            {
                DAVA::ResourceArchive helpRA("~res:/Help.docs");
                engineContext->fileSystem->DeleteDirectory(docsPath);
                engineContext->fileSystem->CreateDirectory(docsPath, true);
                helpRA.UnpackToFolder(docsPath);
            }
            catch (std::exception& ex)
            {
                DAVA::Logger::Error("can't unpack Help.docs: %s", ex.what());
                DVASSERT(false && "can't upack Help.docs");
            }
        }
        SettingsManager::SetValue(Settings::Internal_EditorVersion, DAVA::VariantType(DAVA::String(APPLICATION_BUILD_VERSION)));
    }

    void FixOSXFonts()
    {
        if (QSysInfo::MacintoshVersion != QSysInfo::MV_None && QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
        {
            // fix Mac OS X 10.9 (mavericks) font issue
            QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
        }
    }

private:
    ResourceEditorLauncher launcher;
    TextureCache* textureCache;
    QtMainWindow* mainWindow = nullptr;
};
}

namespace REApplication
{
int REApplication::Run(CommandLineManager& cmdLineMng)
{
    DAVA::Engine e;
    REAppDetail::REConsoleApp app(e, cmdLineMng);
    app.Init();
    return e.Run();
}

int REApplication::Run()
{
    DAVA::Engine e;
    const DAVA::Vector<DAVA::String>& cmdLine = e.GetCommandLine();
    DVASSERT(!cmdLine.empty());

    DAVA::String appPath = cmdLine.front();
    QFileInfo appFileInfo(QString::fromStdString(appPath));

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash((appUid + appFileInfo.absoluteDir().absolutePath()).toUtf8(), QCryptographicHash::Sha1).toHex();
    RunGuard runGuard(appUidPath);
    if (!runGuard.tryToRun())
        return 0;

    REAppDetail::REGuiApp app(e);
    app.Init();
    return e.Run();
}
}

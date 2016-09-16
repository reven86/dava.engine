#include "Classes/Qt/Application/REBaseApplication.h"
#include "version.h"

#include "Preferences/PreferencesStorage.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "QtHelpers/RunGuard.h"

#include "TextureCompression/PVRConverter.h"
#include "Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Engine/Engine.h"
#include "Engine/NativeService.h"
#include "Engine/EngineContext.h"

#include "Core/PerformanceSettings.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>

REBaseApplication::REBaseApplication()
{
    engine.gameLoopStarted.Connect(this, &REBaseApplication::OnLoopStarted);
    engine.gameLoopStopped.Connect(this, &REBaseApplication::OnLoopStopped);
    engine.update.Connect(this, &REBaseApplication::OnUpdate);
    engine.windowCreated.Connect(this, &REBaseApplication::OnWindowCreated);
}

void REBaseApplication::OnLoopStarted()
{
    config = new EditorConfig();
    settingsManager = new SettingsManager();
    sceneValidator = new SceneValidator();

#ifdef __DAVAENGINE_BEAST__
    beastProxy = new BeastProxyImpl();
#else
    beastProxy = new BeastProxy();
#endif //__DAVAENGINE_BEAST__

    DAVA::EngineContext* engineContext = engine.GetContext();
    const char* settingsPath = "ResourceEditorSettings.archive";
    DAVA::FilePath localPrefrencesPath(engineContext->fileSystem->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);
    SettingsManager::UpdateGPUSettings();

    engineContext->logger->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());
    engineContext->virtualCoordSystem->EnableReloadResourceOnResize(false);
    engineContext->performanceSettings->SetPsPerformanceMinFPS(5.0f);
    engineContext->performanceSettings->SetPsPerformanceMaxFPS(10.0f);
}

void REBaseApplication::OnLoopStopped()
{
    DAVA::SafeRelease(beastProxy);
    DAVA::SafeRelease(config);
    DAVA::SafeRelease(settingsManager);
    DAVA::SafeRelease(sceneValidator);

    VisibilityCheckSystem::ReleaseCubemapRenderTargets();
}

void REBaseApplication::OnUpdate(DAVA::float32 delta)
{
}

void REBaseApplication::OnWindowCreated(DAVA::Window& w)
{
}

DAVA::KeyedArchive* REBaseApplication::GetEngineOptions()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

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

DAVA::Vector<DAVA::String> REBaseApplication::GetEngineModules()
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

void REBaseApplication::Init()
{
    DAVA::ScopedPtr<DAVA::KeyedArchive> options(GetEngineOptions());
    engine.SetOptions(options);
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
    engine.GetContext()->logger->SetLogFilename("ResEditor.txt");
}

int REBaseApplication::Run()
{
    Init();
    if (!engine.IsConsoleMode())
    {
        const DAVA::Vector<DAVA::String>& cmdLine = engine.GetCommandLine();
        DVASSERT(!cmdLine.empty());

        DAVA::String appPath = cmdLine.front();
        QFileInfo appFileInfo(QString::fromStdString(appPath));

        const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
        const QString appUidPath = QCryptographicHash::hash((appUid + appFileInfo.absoluteDir().absolutePath()).toUtf8(), QCryptographicHash::Sha1).toHex();
        QtHelpers::RunGuard runGuard(appUidPath);
        if (!runGuard.tryToRun())
            return 0;
    }

    return engine.Run();
}

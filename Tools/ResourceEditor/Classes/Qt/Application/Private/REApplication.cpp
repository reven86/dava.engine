#include "Classes/Qt/Application/REApplication.h"
#include "Classes/Qt/Application/REModule.h"

#include "TextureCompression/PVRConverter.h"
#include "Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Preferences/PreferencesStorage.h"
#include "Deprecated/EditorConfig.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "TArcCore/TArcCore.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Engine/EngineContext.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RHI/rhi_Type.h"
#include "Core/PerformanceSettings.h"
#include "Base/BaseTypes.h"

#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>

#include "CommandLine/BeastCommandLineTool.h"
#include "CommandLine/ConsoleHelpTool.h"
#include "CommandLine/DumpTool.h"
#include "CommandLine/SceneImageDump.h"
#include "CommandLine/StaticOcclusionTool.h"
#include "CommandLine/VersionTool.h"

#include "CommandLine/ImageSplitterTool.h"
#include "CommandLine/TextureDescriptorTool.h"
#include "CommandLine/SceneSaverTool.h"
#include "CommandLine/SceneExporterTool.h"

namespace REApplicationDetail
{
DAVA::KeyedArchive* CreateOptions()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("max_index_buffer_count", 16384);
    appOptions->SetInt32("max_vertex_buffer_count", 16384);
    appOptions->SetInt32("max_const_buffer_count", 32767);
    appOptions->SetInt32("max_texture_count", 2048);

    appOptions->SetInt32("shader_const_buffer_size", 256 * 1024 * 1024);

    return appOptions;
}
}

REApplication::REApplication(DAVA::Vector<DAVA::String>&& cmdLine_)
    : cmdLine(std::move(cmdLine_))
{
    if (cmdLine.size() > 1)
    {
        DAVA::String command = cmdLine[1];
        isConsoleMode = (command != "--selftest");
    }
}

DAVA::TArc::BaseApplication::EngineInitInfo REApplication::GetInitInfo() const
{
    EngineInitInfo initInfo;
    initInfo.runMode = isConsoleMode ? DAVA::eEngineRunMode::CONSOLE_MODE : DAVA::eEngineRunMode::GUI_EMBEDDED;
    initInfo.modules = DAVA::Vector<DAVA::String>
    {
      "JobManager",
      "NetCore",
      "LocalizationSystem",
      "SoundSystem",
      "DownloadManager",
    };

    initInfo.options.Set(REApplicationDetail::CreateOptions());
    return initInfo;
}

void REApplication::CreateModules(DAVA::TArc::Core* tarcCore) const
{
    if (isConsoleMode)
    {
        CreateConsoleModules(tarcCore);
    }
    else
    {
        CreateGUIModules(tarcCore);
    }
}

void REApplication::Init(DAVA::EngineContext& engineContext)
{
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

    engineContext.logger->SetLogFilename("ResEditor.txt");

    config = new EditorConfig();
    settingsManager = new SettingsManager();
    sceneValidator = new SceneValidator();
    beastProxy = new BEAST_PROXY_TYPE();

    const char* settingsPath = "ResourceEditorSettings.archive";
    DAVA::FilePath localPrefrencesPath(engineContext.fileSystem->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);
    SettingsManager::UpdateGPUSettings();

    engineContext.logger->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());
    engineContext.uiControlSystem->vcs->EnableReloadResourceOnResize(false);
    engineContext.performanceSettings->SetPsPerformanceMinFPS(5.0f);
    engineContext.performanceSettings->SetPsPerformanceMaxFPS(10.0f);
}

void REApplication::Cleanup()
{
    DAVA::SafeRelease(beastProxy);
    DAVA::SafeRelease(config);
    DAVA::SafeRelease(settingsManager);
    DAVA::SafeRelease(sceneValidator);

    VisibilityCheckSystem::ReleaseCubemapRenderTargets();

    cmdLine.clear();
}

bool REApplication::AllowMultipleInstances() const
{
    return isConsoleMode;
}

QString REApplication::GetInstanceKey() const
{
    DAVA::String appPath = cmdLine.front();
    QFileInfo appFileInfo(QString::fromStdString(appPath));

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash((appUid + appFileInfo.absoluteDir().absolutePath()).toUtf8(), QCryptographicHash::Sha1).toHex();
    return appUidPath;
}

void REApplication::CreateGUIModules(DAVA::TArc::Core* tarcCore) const
{
    Q_INIT_RESOURCE(QtToolsResources);
    tarcCore->CreateModule<REModule>();
}

void REApplication::CreateConsoleModules(DAVA::TArc::Core* tarcCore) const
{
    DVASSERT(cmdLine.size() > 1);

    DAVA::String toolKey = cmdLine[1];
    if (toolKey == ConsoleHelpTool::Key)
    {
        tarcCore->CreateModule<ConsoleHelpTool>(cmdLine);
    }
    else if (toolKey == VersionTool::Key)
    {
        tarcCore->CreateModule<VersionTool>(cmdLine);
    }
#if defined(__DAVAENGINE_BEAST__)
    else if (toolKey == BeastCommandLineTool::Key)
    {
        tarcCore->CreateModule<BeastCommandLineTool>(cmdLine);
    }
#endif //#if defined (__DAVAENGINE_BEAST__)
    else if (toolKey == DumpTool::Key)
    {
        tarcCore->CreateModule<DumpTool>(cmdLine);
    }
    else if (toolKey == SceneImageDump::Key)
    {
        tarcCore->CreateModule<SceneImageDump>(cmdLine);
    }
    else if (toolKey == StaticOcclusionTool::Key)
    {
        tarcCore->CreateModule<StaticOcclusionTool>(cmdLine);
    }
    else if (toolKey == ImageSplitterTool::Key)
    {
        tarcCore->CreateModule<ImageSplitterTool>(cmdLine);
    }
    else if (toolKey == TextureDescriptorTool::Key)
    {
        tarcCore->CreateModule<TextureDescriptorTool>(cmdLine);
    }
    else if (toolKey == SceneExporterTool::Key)
    {
        tarcCore->CreateModule<SceneExporterTool>(cmdLine);
    }
    else if (toolKey == SceneSaverTool::Key)
    {
        tarcCore->CreateModule<SceneSaverTool>(cmdLine);
    }
    else if (toolKey == SceneExporterTool::Key)
    {
        tarcCore->CreateModule<SceneExporterTool>(cmdLine);
    }
    else
    {
        DAVA::Logger::Error("Cannot create commandLine module for command \'%s\'", toolKey.c_str());
        tarcCore->CreateModule<ConsoleHelpTool>(cmdLine);
    }
}

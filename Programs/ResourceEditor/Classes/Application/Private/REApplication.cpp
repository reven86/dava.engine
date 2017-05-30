#include "Classes/Application/REApplication.h"
#include "Classes/Application/REModule.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/Application/LaunchModule.h"

#include <Tools/TextureCompression/PVRConverter.h>
#include "Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Preferences/PreferencesStorage.h"
#include "Deprecated/EditorConfig.h"

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
#include "CommandLine/SceneValidationTool.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "Classes/DevFuncs/TestUIModuleData.h"

#include "TArc/Core/Core.h"
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Utils/ModuleCollection.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RHI/rhi_Type.h"
#include "Core/PerformanceSettings.h"
#include "Base/BaseTypes.h"

#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>

namespace REApplicationDetail
{
rhi::Api Convert(RenderingBackend r)
{
    switch (r)
    {
    case RenderingBackend::DX11:
        return rhi::RHI_DX11;
    case RenderingBackend::DX9:
        return rhi::RHI_DX9;
    case RenderingBackend::OpenGL:
        return rhi::RHI_GLES2;
    default:
        DVASSERT(false);
        break;
    }

    return rhi::RHI_GLES2;
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

    initInfo.options.Set(CreateOptions());
    return initInfo;
}

void REApplication::CreateModules(DAVA::TArc::Core* tarcCore) const
{
    REGlobal::InitTArcCore(tarcCore);
    if (isConsoleMode)
    {
        CreateConsoleModules(tarcCore);
    }
    else
    {
        CreateGUIModules(tarcCore);
    }
}

void REApplication::Init(const DAVA::EngineContext* engineContext)
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

    DAVA::FilePath documentsFolder = engineContext->fileSystem->GetCurrentDocumentsDirectory() + "ResourceEditor/";
    engineContext->fileSystem->CreateDirectory(documentsFolder, true);
    engineContext->fileSystem->SetCurrentDocumentsDirectory(documentsFolder);
    engineContext->logger->SetLogFilename("ResourceEditor.txt");

    settingsManager = new SettingsManager();
#if !defined(DEPLOY_BUILD)
    RenderingBackend renderBackend = static_cast<RenderingBackend>(settingsManager->GetValue(Settings::General_RenderBackend).AsInt32());
    appOptions->SetInt32("renderer", REApplicationDetail::Convert(renderBackend));
#endif

    beastProxy = new BEAST_PROXY_TYPE();
    const char* settingsPath = "ResourceEditorSettings.archive";
    DAVA::FilePath localPrefrencesPath(engineContext->fileSystem->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);
    SettingsManager::UpdateGPUSettings();

    engineContext->logger->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());
    engineContext->uiControlSystem->vcs->EnableReloadResourceOnResize(false);
    engineContext->performanceSettings->SetPsPerformanceMinFPS(5.0f);
    engineContext->performanceSettings->SetPsPerformanceMaxFPS(10.0f);

    if (IsTestEnvironment())
    {
        DAVA::TArc::TestClass::coreChanged.Connect(&REGlobal::InitTArcCore);
    }
}

void REApplication::Cleanup()
{
    REGlobal::InitTArcCore(nullptr);
    DAVA::SafeRelease(beastProxy);
    DAVA::SafeRelease(settingsManager);

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
    tarcCore->CreateModule<ReflectionExtensionsModule>();
    tarcCore->CreateModule<REModule>();
    tarcCore->CreateModule<ProjectManagerModule>();
    tarcCore->CreateModule<SceneManagerModule>();

    for (const DAVA::ReflectedType* type : DAVA::TArc::ModuleCollection::Instance()->GetGuiModules())
    {
        tarcCore->CreateModule(type);
    }

    tarcCore->CreateModule<LaunchModule>();
}

void REApplication::CreateConsoleModules(DAVA::TArc::Core* tarcCore) const
{
    DVASSERT(cmdLine.size() > 1);

    DAVA::Vector<std::pair<const DAVA::ReflectedType*, DAVA::String>> modules = DAVA::TArc::ModuleCollection::Instance()->GetConsoleModules();

    auto createModuleFn = [&](const DAVA::String& command) -> bool
    {
        for (const auto& module : modules)
        {
            if (module.second == command)
            {
                tarcCore->CreateModule(module.first, cmdLine);
                return true;
            }
        }

        return false;
    };

    DAVA::String command = cmdLine[1];
    if (createModuleFn(command) == false)
    {
        DAVA::Logger::Error("Cannot create commandLine module for command \'%s\'", command.c_str());
        createModuleFn("-help");
    }
}

void REApplication::RegisterEditorAnyCasts()
{
    DAVA::TArc::BaseApplication::RegisterEditorAnyCasts();

    DAVA::AnyCast<ComboBoxTestDataDescr, DAVA::String>::Register(&ComboBoxTestDataDescrToString);
    DAVA::AnyCast<ComboBoxTestDataDescr, QIcon>::Register(&ComboBoxTestDataDescrToQIcon);
}

DAVA::KeyedArchive* REApplication::CreateOptions() const
{
    appOptions.ConstructInplace();

    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("max_index_buffer_count", 16384);
    appOptions->SetInt32("max_vertex_buffer_count", 16384);
    appOptions->SetInt32("max_const_buffer_count", 32767);
    appOptions->SetInt32("max_texture_count", 2048);

    appOptions->SetInt32("max_pipeline_state_count", 32 * 1024);

    appOptions->SetInt32("shader_const_buffer_size", 256 * 1024 * 1024);

    appOptions->SetBool("separate_net_thread", true);

    appOptions->Retain();

    return appOptions.Get();
}

#include "REApplication.h"

#include "version.h"
#include "Main/mainwindow.h"
#include "ResourceEditorLauncher.h"
#include "Commands2/NGTCommand.h"
#include "Settings/SettingsManager.h"
#include "TextureCompression/PVRConverter.h"
#include "TextureCache.h"

#include "Tools/LoggerOutput/ErrorDialogOutput.h"

#include "NgtTools/Common/GlobalContext.h"
#include "NgtTools/Application/NGTCmdLineParser.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/RunGuard/RunGuard.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/Utils/AssertGuard.h"

#include "Preferences/PreferencesStorage.h"

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/ControlsFactory.h"

#include "Platform/Qt5/QtLayer.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include <core_command_system/i_command_manager.hpp>
#include <core_generic_plugin/interfaces/i_application.hpp>

#include <QCryptographicHash>

namespace REAppDetails
{
void UnpackHelpDoc()
{
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !DAVA::FileSystem::Instance()->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        try
        {
            DAVA::ResourceArchive helpRA("~res:/Help.docs");
            DAVA::FileSystem::Instance()->DeleteDirectory(docsPath);
            DAVA::FileSystem::Instance()->CreateDirectory(docsPath, true);
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
#ifdef Q_OS_MAC
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif
}
}

#include <core_reflection/i_definition_manager.hpp>
#include <core_ui_framework/i_ui_framework.hpp>

REApplication::REApplication(int argc, char** argv)
    : BaseApplication(argc, argv)
    , ngtCommand(new NGTCommand())
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
    DAVA::Core::Run(argc, argv);
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);

    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
    DAVA::QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);
    DAVA::QualitySettingsSystem::Instance()->SetMetalPreview(true);
    DAVA::QualitySettingsSystem::Instance()->SetRuntimeQualitySwitching(true);
    DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

    cmdLineManager.reset(new CommandLineManager(argc, argv));
}

REApplication::~REApplication() = default;

int REApplication::Run()
{
    DAVA::QtLayer qtLayer;

#ifdef __DAVAENGINE_BEAST__
    BeastProxyImpl beastProxyImpl;
#else
    BeastProxy beastProxy;
#endif //__DAVAENGINE_BEAST__

    EditorConfig config;
    SettingsManager settingsManager;
    SettingsManager::UpdateGPUSettings();
    SceneValidator sceneValidator;

    int argc = GetCommandLine().argc();
    char** argv = GetCommandLine().argv();

    LoadPlugins();
    DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

    if (cmdLineManager->IsEnabled())
    {
        RunConsole();
    }
    else if (argc == 1
#if defined(__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             || (argc == 3 && argv[1] == DAVA::String("-NSDocumentRevisionsDebugMode") && argv[2] == DAVA::String("YES"))
#endif //#if defined (__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             )
    {
        RunWindow();
    }
    else
    {
        return 1; //wrong commandLine
    }

    return 0;
}

void REApplication::GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const
{
    names.push_back(L"plg_variant");
    names.push_back(L"plg_reflection");
    names.push_back(L"plg_command_system");
    names.push_back(L"plg_serialization");
    names.push_back(L"plg_file_system");
    names.push_back(L"plg_editor_interaction");
    names.push_back(L"plg_qt_app");
    names.push_back(L"plg_qt_common");
}

void REApplication::OnPostLoadPlugins()
{
    qApp->setOrganizationName("DAVA");
    qApp->setApplicationName("Resource Editor");

    commandManager = NGTLayer::queryInterface<wgt::ICommandManager>();
    commandManager->SetHistorySerializationEnabled(false);
    commandManager->registerCommand(ngtCommand.get());

    wgt::IUIFramework* uiFramework = NGTLayer::queryInterface<wgt::IUIFramework>();
    DVASSERT(uiFramework != nullptr);

    wgt::IDefinitionManager* defManager = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defManager);

    componentProvider.reset(new NGTLayer::ComponentProvider(*defManager));
    uiFramework->registerComponentProvider(*componentProvider);

    const char* settingsPath = "ResourceEditorSettings.archive";
    DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    Themes::InitFromQApplication();

    BaseApplication::OnPostLoadPlugins();
}

void REApplication::OnPreUnloadPlugins()
{
    commandManager->deregisterCommand(ngtCommand->getId());
}

bool REApplication::OnRequestCloseApp()
{
    return mainWindow->CanBeClosed();
}

void REApplication::ConfigureLineCommand(NGTLayer::NGTCmdLineParser& lineParser)
{
    lineParser.addParam("preferenceFolder", DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory().GetAbsolutePathname());
    if (cmdLineManager->IsEnabled())
    {
        lineParser.addFlag("hideLogo");
    }
}

void REApplication::RunWindow()
{
#ifdef Q_OS_MAC
    // Must be called before creating QApplication instance
    REAppDetails::FixOSXFonts();
    DAVA::QtLayer::MakeAppForeground(false);
#endif

    ToolsAssetGuard::Instance()->Init();

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
    RunGuard runGuard(appUidPath);
    if (!runGuard.tryToRun())
        return;

    Q_INIT_RESOURCE(QtToolsResources);

    TextureCache textureCache;

    DAVA::LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");
    DAVA::LocalizationSystem::Instance()->SetCurrentLocale("en");

    // check and unpack help documents
    REAppDetails::UnpackHelpDoc();

#ifdef Q_OS_MAC
    QTimer::singleShot(0, [] { DAVA::QtLayer::MakeAppForeground(); });
    QTimer::singleShot(0, [] { DAVA::QtLayer::RestoreMenuBar(); });
#endif

    DAVA::Logger::AddCustomOutput(new ErrorDialogOutput());

    // create and init UI
    ResourceEditorLauncher launcher;
    mainWindow = new QtMainWindow(GetComponentContext());

    mainWindow->EnableGlobalTimeout(true);
    DavaGLWidget* glWidget = mainWindow->GetSceneWidget()->GetDavaWidget();

    QObject::connect(glWidget, &DavaGLWidget::Initialized, &launcher, &ResourceEditorLauncher::Launch);
    StartApplication(mainWindow);

    DAVA::SafeRelease(mainWindow);
    ControlsFactory::ReleaseFonts();
}

void REApplication::RunConsole()
{
#if defined(__DAVAENGINE_MACOS__)
    DAVA::QtLayer::MakeAppForeground(false);
#elif defined(__DAVAENGINE_WIN32__)
//    WinConsoleIOLocker locker; //temporary disabled because of freezes of Windows Console
#endif //platforms

    DAVA::Core::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);

    wgt::IApplication* application = NGTLayer::queryInterface<wgt::IApplication>();

    DavaGLWidget glWidget;
    glWidget.MakeInvisible();

    // Delayed initialization throught event loop
    glWidget.show();
#ifdef Q_OS_WIN
    QObject::connect(&glWidget, &DavaGLWidget::Initialized, [application]() { application->quitApplication(); });
    application->startApplication();
#endif
    glWidget.hide();

    //Trick for correct loading of sprites.
    DAVA::VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1, 1, "Gfx");

    DAVA::Texture::SetDefaultGPU(DAVA::eGPUFamily::GPU_ORIGIN);

    cmdLineManager->Process();
}

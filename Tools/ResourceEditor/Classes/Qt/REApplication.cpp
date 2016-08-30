#include "REApplication.h"

#include "version.h"
#include "Main/mainwindow.h"
#include "ResourceEditorLauncher.h"
#include "Settings/SettingsManager.h"
#include "TextureCompression/PVRConverter.h"
#include "TextureCache.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/RunGuard/RunGuard.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/Utils/Utils.h"
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

REApplication::REApplication(int& argc, char** argv)
    : QApplication(argc, argv)
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
    setOrganizationName("DAVA");
    setApplicationName("Resource Editor");

    const char* settingsPath = "ResourceEditorSettings.archive";
    DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

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

    DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

    if (cmdLineManager->IsEnabled())
    {
        RunConsole();
    }
    else if (arguments().size() == 1
#if defined(__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             || (arguments().size() == 3 && arguments().at(1) == "-NSDocumentRevisionsDebugMode" && arguments().at(2) == "YES")
#endif //#if defined (__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             )
    {
        RunWindow();
    }
    else
    {
        DAVA::Logger::Error("Wrong command line. Exit on start.");
        return 1; //wrong commandLine
    }

    return 0;
}

void REApplication::RunWindow()
{
#ifdef Q_OS_MAC
    // Must be called before creating QApplication instance
    REAppDetails::FixOSXFonts();
    DAVA::QtLayer::MakeAppForeground(false);
#endif
    Themes::InitFromQApplication();

    ToolsAssetGuard::Instance()->Init();

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
    RunGuard runGuard(appUidPath);
    if (!runGuard.tryToRun())
        return;

    Q_INIT_RESOURCE(QtToolsResources);
    ConnectApplicationFocus();
    TextureCache textureCache;

    DAVA::LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");
    DAVA::LocalizationSystem::Instance()->SetCurrentLocale("en");

    // check and unpack help documents
    REAppDetails::UnpackHelpDoc();

#ifdef Q_OS_MAC
    QTimer::singleShot(0, [] { DAVA::QtLayer::MakeAppForeground(); });
    QTimer::singleShot(0, [] { DAVA::QtLayer::RestoreMenuBar(); });
#endif

    // create and init UI
    ResourceEditorLauncher launcher;
    mainWindow = new QtMainWindow();
    QObject::connect(&launcher, &ResourceEditorLauncher::LaunchFinished, [this]()
                     {
                         mainWindow->SetupTitle();
                         mainWindow->OnSceneNew();
                     });

    mainWindow->EnableGlobalTimeout(true);
    DavaGLWidget* glWidget = mainWindow->GetSceneWidget()->GetDavaWidget();

    QObject::connect(glWidget, &DavaGLWidget::Initialized, &launcher, &ResourceEditorLauncher::Launch);
    mainWindow->show();
    exec();

    DAVA::SafeDelete(mainWindow);
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

    DavaGLWidget glWidget;
    glWidget.MakeInvisible();

    // Delayed initialization throught event loop
    glWidget.show();
#ifdef Q_OS_WIN
    QObject::connect(&glWidget, &DavaGLWidget::Initialized, this, &QApplication::quit);
    exec();
#endif
    glWidget.hide();

    //Trick for correct loading of sprites.
    DAVA::VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1, 1, "Gfx");

    DAVA::Texture::SetDefaultGPU(DAVA::eGPUFamily::GPU_ORIGIN);

    cmdLineManager->Process();
}

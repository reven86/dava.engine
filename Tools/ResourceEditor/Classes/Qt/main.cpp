#include "DAVAEngine.h"

#include <QApplication>
#include <QCryptographicHash>

#include "version.h"
#include "Main/mainwindow.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "Project/ProjectManager.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TextureCompression/PVRConverter.h"
#include "CommandLine/CommandLineManager.h"
#include "FileSystem/ResourceArchive.h"
#include "TextureBrowser/TextureCache.h"

#include "Qt/Settings/SettingsManager.h"
#include "QtTools/RunGuard/RunGuard.h"
#include "NgtTools/Application/NGTApplication.h"

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/ControlsFactory.h"

#include "Platform/Qt5/QtLayer.h"

#include "ResourceEditorLauncher.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

void UnpackHelpDoc();
void FixOSXFonts();

void RunConsole(int argc, char* argv[], CommandLineManager& cmdLine);
void RunGui(int argc, char* argv[], CommandLineManager& cmdLine);

class REApplication : public NGTLayer::BaseApplication
{
public:
    REApplication(int argc, char** argv)
        : BaseApplication(argc, argv)
    {
    }

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override
    {
        names.push_back(L"plg_reflection");
        names.push_back(L"plg_variant");
        names.push_back(L"plg_command_system");
        names.push_back(L"plg_serialization");
        names.push_back(L"plg_file_system");
        names.push_back(L"plg_editor_interaction");
        names.push_back(L"plg_qt_app");
        names.push_back(L"plg_qt_common");
    }

    void OnPostLoadPugins() override
    {
        qApp->setOrganizationName("DAVA");
        qApp->setApplicationName("Resource Editor");
    }
};

int main(int argc, char* argv[])
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif

    DAVA::Core::Run(argc, argv);
    DAVA::QtLayer qtLayer;
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);

    DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

#ifdef __DAVAENGINE_BEAST__
    BeastProxyImpl beastProxyImpl;
#else
    BeastProxy beastProxy;
#endif //__DAVAENGINE_BEAST__

    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
    DAVA::QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);

    int exitCode = 0;
    {
        EditorConfig config;
        SettingsManager settingsManager;
        SettingsManager::UpdateGPUSettings();
        SceneValidator sceneValidator;

        CommandLineManager cmdLine(argc, argv);
        if (cmdLine.IsEnabled())
        {
            RunConsole(argc, argv, cmdLine);
        }
        else if (argc == 1
#if defined(__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
                 || (argc == 3 && argv[1] == DAVA::String("-NSDocumentRevisionsDebugMode") && argv[2] == DAVA::String("YES"))
#endif //#if defined (__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
                 )
        {
            RunGui(argc, argv, cmdLine);
        }
        else
        {
            exitCode = 1; //wrong commandLine
        }
    }

    return exitCode;
}

void RunConsole(int argc, char* argv[], CommandLineManager& cmdLineManager)
{
#if defined(__DAVAENGINE_MACOS__)
    DAVA::QtLayer::MakeAppForeground(false);
#elif defined(__DAVAENGINE_WIN32__)
//    WinConsoleIOLocker locker; //temporary disabled because of freezes of Windows Console
#endif //platforms

    DAVA::Core::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);

    QApplication a(argc, argv);

    DavaGLWidget glWidget;
    glWidget.MakeInvisible();

    DAVA::Logger::Info(QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

    // Delayed initialization throught event loop
    glWidget.show();
#ifdef Q_OS_WIN
    QObject::connect(&glWidget, &DavaGLWidget::Initialized, &a, &QApplication::quit);
    a.exec();
#endif
    glWidget.hide();

    //Trick for correct loading of sprites.
    DAVA::VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1, 1, "Gfx");

    cmdLineManager.Process();
}

void RunGui(int argc, char* argv[], CommandLineManager& cmdLine)
{
#ifdef Q_OS_MAC
    // Must be called before creating QApplication instance
    FixOSXFonts();
    DAVA::QtLayer::MakeAppForeground(false);
#endif

    REApplication a(argc, argv);
    a.LoadPlugins();

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
    RunGuard runGuard(appUidPath);
    if (!runGuard.tryToRun())
        return;

    Q_INIT_RESOURCE(QtToolsResources);

    TextureCache textureCache;

    DAVA::LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");
    DAVA::LocalizationSystem::Instance()->SetCurrentLocale("en");

    DAVA::int32 val = SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32();
    DAVA::eGPUFamily family = static_cast<DAVA::eGPUFamily>(val);
    DAVA::Texture::SetDefaultGPU(family);

    // check and unpack help documents
    UnpackHelpDoc();

#ifdef Q_OS_MAC
    QTimer::singleShot(0, [] { DAVA::QtLayer::MakeAppForeground(); });
    QTimer::singleShot(0, [] { DAVA::QtLayer::RestoreMenuBar(); });
#endif

    // create and init UI
    ResourceEditorLauncher launcher;
    QtMainWindow mainWindow(a.GetComponentContext());

    mainWindow.EnableGlobalTimeout(true);
    DavaGLWidget* glWidget = mainWindow.GetSceneWidget()->GetDavaWidget();

    QObject::connect(glWidget, &DavaGLWidget::Initialized, &launcher, &ResourceEditorLauncher::Launch);
    DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

    // start app
    a.StartApplication(&mainWindow);

    ControlsFactory::ReleaseFonts();
}

void UnpackHelpDoc()
{
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !DAVA::FileSystem::Instance()->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        try
        {
            DAVA::ResourceArchive helpRA("~res:/ResourceEditor/Help.docs");
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

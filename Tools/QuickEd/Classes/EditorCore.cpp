#include "EditorCore.h"

#include "Project/Project.h"
#include "UI/mainwindow.h"

#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/Utils/AssertGuard.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/Utils/Utils.h"

#include "TextureCompression/PVRConverter.h"
#include "version.h"

#include "Base/Result.h"
#include "DAVAVersion.h"
#include "Engine/Engine.h"
#include "Engine/Qt/NativeServiceQt.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"
#include "Particles/ParticleEmitter.h"
#include "UI/Input/UIInputSystem.h"
#include "UI/Layouts/UILayoutSystem.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"

#include <QFile>
#include <QDir>

using namespace DAVA;

namespace EditorCoreDetails
{
static const char* EDITOR_TITLE = "DAVA Framework - QuickEd | %s-%s [%u bit]";
static const DAVA::String DOCUMENTATION_DIRECTORY("~doc:/Help/");

void InitPVRTexTool()
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);
}

std::tuple<std::unique_ptr<Project>, ResultList> CreateProject(const QString& path, MainWindow* mainWindow)
{
    ResultList resultList;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        QString message = QObject::tr("%1 does not exist.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    if (!fileInfo.isFile())
    {
        QString message = QObject::tr("%1 is not a file.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    ProjectProperties properties;
    std::tie(resultList, properties) = Project::ParseProjectPropertiesFromFile(path);
    if (!resultList.IsSuccess())
    {
        return std::make_tuple(nullptr, resultList);
    }

    return std::make_tuple(std::make_unique<Project>(mainWindow->GetProjectView(), properties), resultList);
}

DAVA::Result CreateProjectStructure(const QString& projectFilePath)
{
    QDir projectDir(QFileInfo(projectFilePath).absolutePath());

    ProjectProperties defaultSettings = ProjectProperties::Default();

    QString resourceDirectory = QString::fromStdString(defaultSettings.GetResourceDirectory().relative);
    if (!projectDir.mkpath(resourceDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create resource directory %1.").arg(resourceDirectory).toStdString());
    }

    QString intermediateResourceDirectory = QString::fromStdString(defaultSettings.GetConvertedResourceDirectory().relative);
    if (intermediateResourceDirectory != resourceDirectory && !projectDir.mkpath(intermediateResourceDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create intermediate resource directory %1.").arg(intermediateResourceDirectory).toStdString());
    }

    QDir resourceDir(projectDir.absolutePath() + resourceDirectory);

    QString gfxDirectory = QString::fromStdString(defaultSettings.GetGfxDirectories().front().directory.relative);
    if (!resourceDir.mkpath(gfxDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create gfx directory %1.").arg(gfxDirectory).toStdString());
    }

    QString uiDirectory = QString::fromStdString(defaultSettings.GetUiDirectory().relative);
    if (!resourceDir.mkpath(uiDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create UI directory %1.").arg(uiDirectory).toStdString());
    }

    QString textsDirectory = QString::fromStdString(defaultSettings.GetTextsDirectory().relative);
    if (!resourceDir.mkpath(textsDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create UI directory %1.").arg(textsDirectory).toStdString());
    }

    QString fontsDirectory = QString::fromStdString(defaultSettings.GetFontsDirectory().relative);
    if (!resourceDir.mkpath(fontsDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts directory %1.").arg(fontsDirectory).toStdString());
    }

    QString fontsConfigDirectory = QString::fromStdString(defaultSettings.GetFontsConfigsDirectory().relative);
    if (!resourceDir.mkpath(fontsConfigDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts config directory %1.").arg(fontsConfigDirectory).toStdString());
    }

    QDir textsDir(resourceDir.absolutePath() + textsDirectory);
    QFile defaultLanguageTextFile(textsDir.absoluteFilePath(QString::fromStdString(defaultSettings.GetDefaultLanguage()) + ".yaml"));
    if (!defaultLanguageTextFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create localization file %1.").arg(defaultLanguageTextFile.fileName()).toStdString());
    }
    defaultLanguageTextFile.close();

    QDir fontsConfigDir(resourceDir.absolutePath() + fontsConfigDirectory);
    QFile defaultFontsConfigFile(fontsConfigDir.absoluteFilePath(QString::fromStdString(ProjectProperties::GetFontsConfigFileName())));
    if (!defaultFontsConfigFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create localization file %1.").arg(defaultFontsConfigFile.fileName()).toStdString());
    }
    defaultFontsConfigFile.close();

    defaultSettings.SetProjectFile(projectFilePath.toStdString());
    if (!Project::EmitProjectPropertiesToFile(defaultSettings))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create project file %1.").arg(projectFilePath).toStdString());
    }

    return Result();
}

std::tuple<QString, DAVA::ResultList> CreateNewProject()
{
    DAVA::ResultList resultList;

    QString projectDirPath = QFileDialog::getExistingDirectory(qApp->activeWindow(), QObject::tr("Select directory for new project"));
    if (projectDirPath.isEmpty())
    {
        return std::make_tuple(QString(), resultList);
    }

    QDir projectDir(projectDirPath);
    const QString projectFileName = QString::fromStdString(ProjectProperties::GetProjectFileName());
    QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
    if (QFile::exists(fullProjectFilePath))
    {
        resultList.AddResult(Result::RESULT_FAILURE, QObject::tr("Project file exists!").toStdString());
        return std::make_tuple(QString(), resultList);
    }

    Result result = CreateProjectStructure(fullProjectFilePath);
    if (result.type != Result::RESULT_SUCCESS)
    {
        resultList.AddResult(result);
        return std::make_tuple(QString(), resultList);
    }

    return std::make_tuple(fullProjectFilePath, resultList);
}
};

REGISTER_PREFERENCES_ON_START(EditorCore,
                              PREF_ARG("isUsingAssetCache", false),
                              PREF_ARG("projectsHistory", DAVA::String()),
                              PREF_ARG("projectsHistorySize", static_cast<DAVA::uint32>(5))
                              )

EditorCore::EditorCore(DAVA::Engine& engine)
    : QObject()
    , cacheClient()
{
    ParticleEmitter::FORCE_DEEP_CLONE = true;
    ToolsAssetGuard::Instance()->Init();

    using namespace DAVA;
    EngineContext* context = engine.GetContext();

    FileSystem* fs = context->fileSystem;
    fs->SetCurrentDocumentsDirectory(fs->GetUserDocumentsPath() + "QuickEd/");
    fs->CreateDirectory(fs->GetCurrentDocumentsDirectory(), true);

    UIControlSystem* uiControlSystem = context->uiControlSystem;
    uiControlSystem->GetLayoutSystem()->SetAutoupdatesEnabled(false);

    UIInputSystem* inputSystem = uiControlSystem->GetInputSystem();
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::LEFT), UIInputSystem::ACTION_FOCUS_LEFT);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::RIGHT), UIInputSystem::ACTION_FOCUS_RIGHT);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::UP), UIInputSystem::ACTION_FOCUS_UP);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::DOWN), UIInputSystem::ACTION_FOCUS_DOWN);

    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::TAB), UIInputSystem::ACTION_FOCUS_NEXT);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::TAB, eModifierKeys::SHIFT), UIInputSystem::ACTION_FOCUS_PREV);

    context->logger->SetLogFilename("QuickEd.txt");

    Renderer::SetDesiredFPS(60);

    const char* settingsPath = "QuickEdSettings.archive";
    FilePath localPrefrencesPath(fs->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    EditorCoreDetails::InitPVRTexTool();

    qInstallMessageHandler(DAVAMessageHandler);

    Q_INIT_RESOURCE(QtToolsResources);
    Themes::InitFromQApplication();

    mainWindow.reset(new MainWindow());

    DAVA::RenderWidget* renderWidget = engine.GetNativeService()->GetRenderWidget();
    mainWindow->InjectRenderWidget(renderWidget);

    //we need to register preferences when whole class is initialized
    PreferencesStorage::Instance()->RegisterPreferences(this);

    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->SetRecentProjects(GetRecentProjects());

    mainWindow->SetEditorTitle(GenerateEditorTitle());

    connect(mainWindow.get(), &MainWindow::CanClose, this, &EditorCore::CloseProject);
    connect(mainWindow.get(), &MainWindow::NewProject, this, &EditorCore::OnNewProject);
    connect(mainWindow.get(), &MainWindow::OpenProject, this, &EditorCore::OnOpenProject);
    connect(mainWindow.get(), &MainWindow::RecentProject, this, &EditorCore::OpenProject);
    connect(mainWindow.get(), &MainWindow::CloseProject, this, &EditorCore::OnCloseProject);
    connect(mainWindow.get(), &MainWindow::Exit, this, &EditorCore::OnExit);
    connect(mainWindow.get(), &MainWindow::ShowHelp, this, &EditorCore::OnShowHelp);

    UnpackHelp();

    mainWindow->show();
}

EditorCore::~EditorCore()
{
    DVASSERT(project == nullptr);

    DisableCacheClient();

    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void EditorCore::OnRenderingInitialized()
{
    mainWindow->OnWindowCreated();
    QString lastProjectPath = GetLastProject();
    if (!lastProjectPath.isEmpty())
    {
        OpenProject(lastProjectPath);
    }
}

void EditorCore::OnShowHelp()
{
    FilePath docsPath = EditorCoreDetails::DOCUMENTATION_DIRECTORY + "index.html";
    QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
    QDesktopServices::openUrl(QUrl(docsFile));
}

void EditorCore::OpenProject(const QString& path)
{
    if (!CloseProject())
    {
        return;
    }

    ResultList resultList;
    std::unique_ptr<Project> newProject;

    std::tie(newProject, resultList) = EditorCoreDetails::CreateProject(path, mainWindow.get());

    if (newProject.get())
    {
        project = std::move(newProject);
        AddRecentProject(project->GetProjectPath());

        mainWindow->SetRecentProjects(GetRecentProjects());

        connect(this, &EditorCore::AssetCacheChanged, project.get(), &Project::SetAssetCacheClient);
        connect(this, &EditorCore::TryCloseDocuments, project.get(), &Project::TryCloseAllDocuments);

        if (assetCacheEnabled)
        {
            EnableCacheClient();
        }

        project->SetAssetCacheClient(cacheClient.get());
    }

    mainWindow->ShowResultList(tr("Error while loading project"), resultList);
}

void EditorCore::OnCloseProject()
{
    CloseProject();
}

bool EditorCore::CloseProject()
{
    if (project == nullptr)
    {
        return true;
    }

    if (!TryCloseDocuments())
    {
        return false;
    }

    disconnect(this, &EditorCore::AssetCacheChanged, project.get(), &Project::SetAssetCacheClient);
    disconnect(this, &EditorCore::TryCloseDocuments, project.get(), &Project::TryCloseAllDocuments);

    DisableCacheClient();
    project = nullptr;
    return true;
}

void EditorCore::OnExit()
{
    if (CloseProject())
    {
        qApp->quit();
    }
}

bool EditorCore::IsUsingAssetCache() const
{
    return assetCacheEnabled;
}

void EditorCore::SetUsingAssetCacheEnabled(bool enabled)
{
    if (enabled)
    {
        EnableCacheClient();
        assetCacheEnabled = true;
    }
    else
    {
        DisableCacheClient();
        assetCacheEnabled = false;
    }
}

void EditorCore::EnableCacheClient()
{
    DisableCacheClient();
    cacheClient.reset(new AssetCacheClient());
    DAVA::AssetCache::Error connected = cacheClient->ConnectSynchronously(connectionParams);
    if (connected != AssetCache::Error::NO_ERRORS)
    {
        cacheClient.reset();
        Logger::Warning("Asset cache client was not started! Error №%d", connected);
    }
    else
    {
        Logger::Info("Asset cache client started");
        emit AssetCacheChanged(cacheClient.get());
    }
}

void EditorCore::DisableCacheClient()
{
    if (cacheClient != nullptr && cacheClient->IsConnected())
    {
        cacheClient->Disconnect();
        cacheClient.reset();
        emit AssetCacheChanged(cacheClient.get());
    }
}

const QStringList& EditorCore::GetRecentProjects() const
{
    return recentProjects;
}

QString EditorCore::GetLastProject() const
{
    if (!recentProjects.empty())
    {
        return recentProjects.last();
    }

    return QString();
}

void EditorCore::AddRecentProject(const QString& projectPath)
{
    recentProjects.removeAll(projectPath);
    recentProjects += projectPath;
    while (static_cast<DAVA::uint32>(recentProjects.size()) > projectsHistorySize)
    {
        recentProjects.removeFirst();
    }
}

String EditorCore::GetRecentProjectsAsString() const
{
    return recentProjects.join('\n').toStdString();
}

void EditorCore::SetRecentProjectsFromString(const String& history)
{
    recentProjects = QString::fromStdString(history).split("\n", QString::SkipEmptyParts);
}

void EditorCore::OnOpenProject()
{
    QString defaultPath;
    if (project.get() != nullptr)
    {
        defaultPath = project->GetProjectDirectory() + project->GetProjectName();
    }
    else
    {
        defaultPath = QDir::currentPath();
    }

    QString projectPath = QFileDialog::getOpenFileName(mainWindow.get(), tr("Select a project file"),
                                                       defaultPath, tr("Project files(*.quicked *.uieditor)"));

    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);

    OpenProject(projectPath);
}

void EditorCore::OnNewProject()
{
    ResultList resultList;
    QString newProjectPath;

    std::tie(newProjectPath, resultList) = EditorCoreDetails::CreateNewProject();

    if (!newProjectPath.isEmpty())
    {
        OpenProject(newProjectPath);
        return;
    }

    mainWindow->ShowResultList(tr("Error while creating project"), resultList);
}

void EditorCore::UnpackHelp()
{
    FilePath docsPath(EditorCoreDetails::DOCUMENTATION_DIRECTORY);
    FileSystem* fs = FileSystem::Instance();
    if (!fs->Exists(docsPath))
    {
        try
        {
            ResourceArchive helpRA("~res:/Help.docs");

            fs->DeleteDirectory(docsPath);
            fs->CreateDirectory(docsPath, true);

            helpRA.UnpackToFolder(docsPath);
        }
        catch (std::exception& ex)
        {
            Logger::Error("%s", ex.what());
            DVASSERT(false && "can't unpack help docs to documents dir");
        }
    }
}

QString EditorCore::GenerateEditorTitle() const
{
    using namespace EditorCoreDetails;
    return QString::fromStdString(DAVA::Format(EDITOR_TITLE, DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION, static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8)));
}

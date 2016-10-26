#include "UI/mainwindow.h"
#include "Document/DocumentGroup.h"
#include "Document/Document.h"
#include "EditorCore.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/Utils/Utils.h"

#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"
#include "UI/FileSystemView/FileSystemModel.h"
#include "UI/Package/PackageModel.h"
#include "ResourcesManageHelper.h"
#include "QtTools/FileDialogs/FileDialog.h"
#include "Base/Result.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/FileSystem.h"
#include "Project/Project.h"

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(EditorCore,
                              PREF_ARG("isUsingAssetCache", false),
                              PREF_ARG("projectsHistory", DAVA::String()),
                              PREF_ARG("projectsHistorySize", static_cast<DAVA::uint32>(5))
                              )

EditorCore::EditorCore(QObject* parent)
    : QObject(parent)
    , cacheClient()
    , mainWindow(std::make_unique<MainWindow>())
{
    ConnectApplicationFocus();

    PreferencesStorage::Instance()->RegisterPreferences(this);

    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->SetRecentProjects(GetRecentProjects());

    connect(mainWindow.get(), &MainWindow::CanClose, this, &EditorCore::CloseProject);
    connect(mainWindow->actionNew_project, &QAction::triggered, this, &EditorCore::OnNewProject);
    connect(mainWindow->actionOpen_project, &QAction::triggered, this, &EditorCore::OnOpenProject);
    connect(mainWindow->actionClose_project, &QAction::triggered, this, &EditorCore::OnCloseProject);
    connect(mainWindow->actionExit, &QAction::triggered, this, &EditorCore::OnExit);
    connect(mainWindow->menuRecent, &QMenu::triggered, this, &EditorCore::OnRecentMenu);
    connect(mainWindow.get(), &MainWindow::EmulationModeChanged, mainWindow->previewWidget, &PreviewWidget::OnEmulationModeChanged);
    connect(mainWindow->previewWidget, &PreviewWidget::DropRequested, mainWindow->packageWidget->GetPackageModel(), &PackageModel::OnDropMimeData, Qt::DirectConnection);
    connect(mainWindow->previewWidget, &PreviewWidget::DeleteRequested, mainWindow->packageWidget, &PackageWidget::OnDelete);
    connect(mainWindow->previewWidget, &PreviewWidget::ImportRequested, mainWindow->packageWidget, &PackageWidget::OnImport);
    connect(mainWindow->previewWidget, &PreviewWidget::CutRequested, mainWindow->packageWidget, &PackageWidget::OnCut);
    connect(mainWindow->previewWidget, &PreviewWidget::CopyRequested, mainWindow->packageWidget, &PackageWidget::OnCopy);
    connect(mainWindow->previewWidget, &PreviewWidget::PasteRequested, mainWindow->packageWidget, &PackageWidget::OnPaste);
    connect(mainWindow->previewWidget, &PreviewWidget::SelectionChanged, mainWindow->packageWidget, &PackageWidget::OnSelectionChanged);
    connect(mainWindow->packageWidget, &PackageWidget::SelectedNodesChanged, mainWindow->previewWidget, &PreviewWidget::OnSelectionChanged);
    connect(mainWindow->packageWidget, &PackageWidget::CurrentIndexChanged, mainWindow->propertiesWidget, &PropertiesWidget::UpdateModel);
    connect(mainWindow->previewWidget->GetGLWidget(), &DavaGLWidget::Initialized, this, &EditorCore::OnGLWidgedInitialized);
}

EditorCore::~EditorCore()
{
    DVASSERT(project == nullptr);

    DisableCacheClient();

    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void EditorCore::Start()
{
    mainWindow->show();
}

void EditorCore::OnGLWidgedInitialized()
{
    QString lastProjectPath = GetLastProject();
    if (!lastProjectPath.isEmpty())
    {
        OpenProject(lastProjectPath);
    }
}

// void EditorCore::OnProjectPathChanged(const QString& projectPath)
// {
//     DisableCacheClient();
//     if (projectPath.isEmpty())
//     {
//         return;
//     }
//     if (assetCacheEnabled)
//     {
//         EnableCacheClient();
//     }
//
//     spritesPacker->SetCacheClient(cacheClient.get(), "QuickEd.ReloadSprites");
//
//     QRegularExpression searchOption("gfx\\d*$", QRegularExpression::CaseInsensitiveOption);
//     spritesPacker->ClearTasks();
//     QDirIterator it(projectPath + "/DataSource");
//     while (it.hasNext())
//     {
//         it.next();
//         const QFileInfo& fileInfo = it.fileInfo();
//         if (fileInfo.isDir())
//         {
//             QString outputPath = fileInfo.absoluteFilePath();
//             if (!outputPath.contains(searchOption))
//             {
//                 continue;
//             }
//             outputPath.replace(outputPath.lastIndexOf("DataSource"), QString("DataSource").size(), "Data");
//             QDir outputDir(outputPath);
//             spritesPacker->AddTask(fileInfo.absoluteFilePath(), outputDir);
//         }
//     }
// }

void EditorCore::OnRecentMenu(QAction* recentProjectAction)
{
    QString projectPath = recentProjectAction->data().toString();

    if (projectPath.isEmpty())
    {
        return;
    }

    OpenProject(projectPath);
}

void EditorCore::OpenProject(const QString& path)
{
    if (!CloseProject())
    {
        return;
    }

    ResultList resultList;
    std::unique_ptr<Project> newProject;

    std::tie(newProject, resultList) = CreateProject(path, mainWindow.get());

    if (newProject.get())
    {
        project = std::move(newProject);
        OnProjectOpen(project.get());
    }

    mainWindow->ShowResultList(tr("Error while loading project"), resultList);
}

std::tuple<std::unique_ptr<Project>, ResultList> EditorCore::CreateProject(const QString& path, MainWindow* mainWindow)
{
    ResultList resultList;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        QString message = tr("%1 does not exist.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    if (!fileInfo.isFile())
    {
        QString message = tr("%1 is not a file.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    Project::Settings settings;
    std::tie(settings, resultList) = Project::ParseProjectSettings(path);
    if (settings.projectFile.isEmpty())
    {
        return std::make_tuple(nullptr, resultList);
    }

    return std::make_tuple(std::make_unique<Project>(mainWindow, settings), resultList);
}

std::tuple<QString, DAVA::ResultList> EditorCore::CreateNewProject()
{
    DAVA::ResultList resultList;

    QString projectDirPath = QFileDialog::getExistingDirectory(qApp->activeWindow(), tr("Select directory for new project"));
    if (projectDirPath.isEmpty())
    {
        return std::make_tuple(QString(), resultList);
    }
    bool needOverwriteProjectFile = true;
    QDir projectDir(projectDirPath);
    const QString projectFileName = Project::GetProjectFileName();
    QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
    if (QFile::exists(fullProjectFilePath))
    {
        resultList.AddResult(Result::RESULT_FAILURE, String("Project file exists!"));
        return std::make_tuple(QString(), resultList);
    }
    QFile projectFile(fullProjectFilePath);
    if (!projectFile.open(QFile::WriteOnly | QFile::Truncate)) // create project file
    {
        resultList.AddResult(Result::RESULT_ERROR, String("Can not open project file ") + fullProjectFilePath.toUtf8().data());
        return std::make_tuple(QString(), resultList);
    }
    if (!projectDir.mkpath(projectDir.canonicalPath() + Project::GetUIRelativePath()))
    {
        resultList.AddResult(Result::RESULT_ERROR, String("Can not create Data/UI folder"));
        return std::make_tuple(QString(), resultList);
    }

    return std::make_tuple(fullProjectFilePath, resultList);
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
    auto documents = project->GetDocumentGroup()->GetDocuments();
    bool hasUnsaved = std::find_if(documents.begin(), documents.end(), [](Document* document) { return document->CanSave(); }) != documents.end();

    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
        qApp->activeWindow(),
        tr("Save changes"),
        tr("Some files has been modified.\n"
           "Do you want to save your changes?"),
        QMessageBox::SaveAll | QMessageBox::NoToAll | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::SaveAll)
        {
            project->GetDocumentGroup()->SaveAllDocuments();
        }
    }

    for (auto& document : project->GetDocumentGroup()->GetDocuments())
    {
        project->GetDocumentGroup()->CloseDocument(document);
    }
    /*project->Close();*/
    OnProjectClose(project.get());
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
    cacheClient.reset(new AssetCacheClient(true));
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

void EditorCore::OnProjectOpen(const Project* newProject)
{
    AddRecentProject(newProject->GetProjectPath());

    mainWindow->SetRecentProjects(GetRecentProjects());

    mainWindow->actionClose_project->setEnabled(true);
    mainWindow->fileSystemDockWidget->setEnabled(true);
    mainWindow->menuTools->setEnabled(true);
    mainWindow->toolBarPlugins->setEnabled(true);

    for (auto& item : newProject->SourceResourceDirectories())
    {
        QFileInfo pathInfo = item.second + Project::GetUIRelativePath();
        QString path = pathInfo.absoluteFilePath();
        QString displayName = item.first;

        mainWindow->fileSystemDockWidget->AddPath(path, displayName);
    }
    mainWindow->libraryWidget->SetLibraryPackages(newProject->GetLibraryPackages());

    //connect(newProject, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    //connect(newProject, &Project::ProjectPathChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::SetProjectDir);
    //connect(newProject, &Project::IsOpenChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::setEnabled);
    //connect(newProject, &Project::IsOpenChanged, this, &EditorCore::OnProjectOpenChanged);

    //EditorLocalizationSystem* editorLocalizationSystem = newProject->GetEditorLocalizationSystem();

    mainWindow->SetLanguages(newProject->GetAvailableLanguages(), newProject->GetCurrentLanguage());

    connect(mainWindow.get(), &MainWindow::CurrentLanguageChanged, newProject, &Project::SetCurrentLanguage);
    connect(mainWindow.get(), &MainWindow::RtlChanged, newProject, &Project::SetRtl);
    connect(mainWindow.get(), &MainWindow::BiDiSupportChanged, newProject, &Project::SetBiDiSupport);
    connect(mainWindow.get(), &MainWindow::GlobalStyleClassesChanged, newProject, &Project::SetGlobalStyleClasses);

    connect(newProject, &Project::CurrentLanguageChanged, mainWindow.get(), &MainWindow::SetCurrentLanguage);

    DocumentGroup* documentGroup = newProject->GetDocumentGroup();
    mainWindow->AttachDocumentGroup(documentGroup);
    connect(mainWindow.get(), &MainWindow::OpenPackageFile, documentGroup, &DocumentGroup::AddDocument);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow.get(), &MainWindow::OnDocumentChanged);

    //connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->previewWidget, &PreviewWidget::SaveSystemsContextAndClear); //this context will affect other widgets, so he must be updated before other widgets take new document
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->previewWidget, &PreviewWidget::OnDocumentChanged);
    //connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->previewWidget, &PreviewWidget::LoadSystemsContext); //this context will affect other widgets, so he must be updated when other widgets took new document
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->packageWidget, &PackageWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);

    //connect(mainWindow->actionReloadSprites, &QAction::triggered, this, &EditorCore::OnReloadSpritesStarted);
    //connect(spritesPacker.get(), &SpritesPacker::Finished, this, &EditorCore::OnReloadSpritesFinished);

    connect(this, &EditorCore::AssetCacheChanged, newProject, &Project::SetAssetCacheClient);

    mainWindow->setWindowTitle(ResourcesManageHelper::GetProjectTitle());

    if (assetCacheEnabled)
    {
        EnableCacheClient();
    }

    //     spritesPacker->SetCacheClient(cacheClient.get(), "QuickEd.ReloadSprites");
    //
    //     QRegularExpression searchOption("gfx\\d*$", QRegularExpression::CaseInsensitiveOption);
    //     spritesPacker->ClearTasks();
    //     QDirIterator it(/*projectPath + */ "/DataSource"); //TODO fix
    //     while (it.hasNext())
    //     {
    //         it.next();
    //         const QFileInfo& fileInfo = it.fileInfo();
    //         if (fileInfo.isDir())
    //         {
    //             QString outputPath = fileInfo.absoluteFilePath();
    //             if (!outputPath.contains(searchOption))
    //             {
    //                 continue;
    //             }
    //             outputPath.replace(outputPath.lastIndexOf("DataSource"), QString("DataSource").size(), "Data");
    //             QDir outputDir(outputPath);
    //             spritesPacker->AddTask(fileInfo.absoluteFilePath(), outputDir);
    //         }
    //     }
}

void EditorCore::OnProjectClose(const Project* currProject)
{
    for (auto& item : currProject->SourceResourceDirectories())
    {
        QFileInfo pathInfo = item.second + Project::GetUIRelativePath();
        QString path = pathInfo.absoluteFilePath();

        mainWindow->fileSystemDockWidget->RemovePath(path);
    }

    mainWindow->actionClose_project->setEnabled(false);
    mainWindow->fileSystemDockWidget->setEnabled(false);
    mainWindow->menuTools->setEnabled(false);
    mainWindow->toolBarPlugins->setEnabled(false);

    mainWindow->fileSystemDockWidget->RemoveAllPaths();
    mainWindow->libraryWidget->SetLibraryPackages(Vector<FilePath>());

    //disconnect(currProject, &Project::IsOpenChanged, mainWindow->actionClose_project, &QAction::setEnabled);
    //disconnect(currProject, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    //disconnect(currProject, &Project::ProjectPathChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::SetProjectDir);
    //disconnect(currProject, &Project::IsOpenChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::setEnabled);
    //disconnect(currProject, &Project::IsOpenChanged, this, &EditorCore::OnProjectOpenChanged);

    //EditorLocalizationSystem* editorLocalizationSystem = currProject->GetEditorLocalizationSystem();
    //disconnect(languageComboBox, &QComboBox::currentTextChanged, editorLocalizationSystem, &EditorLocalizationSystem::SetCurrentLocale);
    //disconnect(editorLocalizationSystem, &EditorLocalizationSystem::CurrentLocaleChanged, languageComboBox, &QComboBox::setCurrentText);
    disconnect(mainWindow.get(), &MainWindow::CurrentLanguageChanged, currProject, &Project::SetCurrentLanguage);
    disconnect(mainWindow.get(), &MainWindow::RtlChanged, currProject, &Project::SetRtl);
    disconnect(mainWindow.get(), &MainWindow::BiDiSupportChanged, currProject, &Project::SetBiDiSupport);
    disconnect(mainWindow.get(), &MainWindow::GlobalStyleClassesChanged, currProject, &Project::SetGlobalStyleClasses);

    disconnect(currProject, &Project::CurrentLanguageChanged, mainWindow.get(), &MainWindow::SetCurrentLanguage);

    DocumentGroup* documentGroup = currProject->GetDocumentGroup();
    mainWindow->DetachDocumentGroup(documentGroup);
    disconnect(mainWindow.get(), &MainWindow::OpenPackageFile, documentGroup, &DocumentGroup::AddDocument);

    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow.get(), &MainWindow::OnDocumentChanged);

    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->previewWidget, &PreviewWidget::SaveSystemsContextAndClear); //this context will affect other widgets, so he must be updated before other widgets take new document
    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->previewWidget, &PreviewWidget::LoadSystemsContext); //this context will affect other widgets, so he must be updated when other widgets took new document
    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->previewWidget, &PreviewWidget::OnDocumentChanged);

    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->packageWidget, &PackageWidget::OnDocumentChanged);

    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);
    disconnect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);

    disconnect(this, &EditorCore::AssetCacheChanged, currProject, &Project::SetAssetCacheClient);

    DisableCacheClient();
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
                                                       defaultPath,
                                                       tr("Project (*.uieditor)"));

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

    std::tie(newProjectPath, resultList) = CreateNewProject();

    if (!newProjectPath.isEmpty())
    {
        OpenProject(newProjectPath);
        return;
    }

    mainWindow->ShowResultList(tr("Error while creating project"), resultList);
}

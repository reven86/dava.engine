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

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(EditorCore,
                              PREF_ARG("isUsingAssetCache", false),
                              PREF_ARG("projectsHistory", DAVA::String()),
                              PREF_ARG("projectsHistorySize", static_cast<DAVA::uint32>(5))
                              )

EditorCore::EditorCore(QObject* parent)
    : QObject(parent)
    , spritesPacker(std::make_unique<SpritesPacker>())
    , cacheClient(nullptr)
    , documentGroup(new DocumentGroup(this))
    , mainWindow(std::make_unique<MainWindow>())
{
    ConnectApplicationFocus();

    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->AttachDocumentGroup(documentGroup);

    connect(mainWindow.get(), &MainWindow::CanClose, this, &EditorCore::CloseProject);
    connect(mainWindow->actionReloadSprites, &QAction::triggered, this, &EditorCore::OnReloadSpritesStarted);
    connect(spritesPacker.get(), &SpritesPacker::Finished, this, &EditorCore::OnReloadSpritesFinished);
    mainWindow->SetRecentProjects(GetProjectsHistory());

    connect(mainWindow->actionNew_project, &QAction::triggered, this, &EditorCore::OnNewProject);
    connect(mainWindow->actionOpen_project, &QAction::triggered, this, &EditorCore::OnOpenProject);
    connect(mainWindow->actionClose_project, &QAction::triggered, this, &EditorCore::OnCloseProject);
    connect(mainWindow->actionExit, &QAction::triggered, this, &EditorCore::OnExit);
    connect(mainWindow->menuRecent, &QMenu::triggered, this, &EditorCore::OnRecentMenu);

    //connect(mainWindow.get(), &MainWindow::CloseProject, this, &EditorCore::OnCloseProject);
    //connect(mainWindow.get(), &MainWindow::ActionExitTriggered, this, &EditorCore::OnExit);
    //connect(mainWindow.get(), &MainWindow::RecentMenuTriggered, this, &EditorCore::OnRecentMenu);
    //connect(mainWindow.get(), &MainWindow::ActionOpenProjectTriggered, this, &EditorCore::OpenProject);

    connect(mainWindow.get(), &MainWindow::OpenPackageFile, documentGroup, &DocumentGroup::AddDocument);
    connect(mainWindow.get(), &MainWindow::RtlChanged, this, &EditorCore::OnRtlChanged);
    connect(mainWindow.get(), &MainWindow::BiDiSupportChanged, this, &EditorCore::OnBiDiSupportChanged);
    connect(mainWindow.get(), &MainWindow::GlobalStyleClassesChanged, this, &EditorCore::OnGlobalStyleClassesChanged);

    auto previewWidget = mainWindow->previewWidget;

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::SaveSystemsContextAndClear); //this context will affect other widgets, so he must be updated before other widgets take new document
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::OnDocumentChanged);
    connect(mainWindow.get(), &MainWindow::EmulationModeChanged, previewWidget, &PreviewWidget::OnEmulationModeChanged);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow.get(), &MainWindow::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);

    auto packageWidget = mainWindow->packageWidget;
    connect(previewWidget, &PreviewWidget::DropRequested, packageWidget->GetPackageModel(), &PackageModel::OnDropMimeData, Qt::DirectConnection);
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, packageWidget, &PackageWidget::OnDocumentChanged);
    connect(previewWidget, &PreviewWidget::DeleteRequested, packageWidget, &PackageWidget::OnDelete);
    connect(previewWidget, &PreviewWidget::ImportRequested, packageWidget, &PackageWidget::OnImport);
    connect(previewWidget, &PreviewWidget::CutRequested, packageWidget, &PackageWidget::OnCut);
    connect(previewWidget, &PreviewWidget::CopyRequested, packageWidget, &PackageWidget::OnCopy);
    connect(previewWidget, &PreviewWidget::PasteRequested, packageWidget, &PackageWidget::OnPaste);
    connect(previewWidget, &PreviewWidget::SelectionChanged, packageWidget, &PackageWidget::OnSelectionChanged);
    connect(packageWidget, &PackageWidget::SelectedNodesChanged, previewWidget, &PreviewWidget::OnSelectionChanged);
    connect(packageWidget, &PackageWidget::CurrentIndexChanged, mainWindow->propertiesWidget, &PropertiesWidget::UpdateModel);

    connect(previewWidget->GetGLWidget(), &DavaGLWidget::Initialized, this, &EditorCore::OnGLWidgedInitialized);
    //connect(project->GetEditorLocalizationSystem(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &EditorCore::UpdateLanguage); TODO fix

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::LoadSystemsContext); //this context will affect other widgets, so he must be updated when other widgets took new document

    PreferencesStorage::Instance()->RegisterPreferences(this);
}

EditorCore::~EditorCore()
{
    if (cacheClient && cacheClient->IsConnected())
    {
        cacheClient->Disconnect();
    }

    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

// MainWindow* EditorCore::GetMainWindow() const
// {
//     return mainWindow.get();
// }

// Project* EditorCore::GetProject() const
// {
//     return project;
// }

void EditorCore::Start()
{
    mainWindow->show();
}

void EditorCore::OnReloadSpritesStarted()
{
    for (auto& document : documentGroup->GetDocuments())
    {
        if (!documentGroup->TryCloseDocument(document))
        {
            return;
        }
    }
    mainWindow->ExecDialogReloadSprites(spritesPacker.get());
}

void EditorCore::OnReloadSpritesFinished()
{
    if (cacheClient)
    {
        cacheClient->Disconnect();
        cacheClient.reset();
    }

    Sprite::ReloadSprites();
}

void EditorCore::OnGLWidgedInitialized()
{
    QStringList projectsPathes = GetProjectsHistory();
    if (!projectsPathes.isEmpty())
    {
        OpenProject(projectsPathes.last());
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

void EditorCore::UpdateLanguage()
{
    project->GetEditorFontSystem()->RegisterCurrentLocaleFonts();
    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnRtlChanged(bool isRtl)
{
    UIControlSystem::Instance()->SetRtl(isRtl);
    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnBiDiSupportChanged(bool support)
{
    UIControlSystem::Instance()->SetBiDiSupportEnabled(support);
    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OnGlobalStyleClassesChanged(const QString& classesStr)
{
    Vector<String> tokens;
    Split(classesStr.toStdString(), " ", tokens);

    UIControlSystem::Instance()->GetStyleSheetSystem()->ClearGlobalClasses();
    for (String& token : tokens)
    {
        UIControlSystem::Instance()->GetStyleSheetSystem()->AddGlobalClass(FastName(token));
    }

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void EditorCore::OpenProject(const QString& path)
{
    if (!CloseProject())
    {
        return;
    }

    ResultList resultList;
    std::unique_ptr<Project> newProject;

    std::tie(project, resultList) = CreateProject(path);

    OnProjectOpen(project.get());
    mainWindow->ShowResultList(resultList);
}

std::tuple<std::unique_ptr<Project>, ResultList> EditorCore::CreateProject(const QString& path)
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

    ScopedPtr<YamlParser> parser(YamlParser::Create(path.toStdString()));
    if (!parser)
    {
        QString message = tr("Can not parse project file %1.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return std::make_tuple(nullptr, resultList);
    }
    //SetProjectName(fileInfo.fileName());

    //editorLocalizationSystem->Cleanup();

    //     QDir projectDir = fileInfo.absoluteDir(); TODO fix
    //     if (!projectDir.mkpath("." + GetScreensRelativePath()))
    //     {
    //         return false;
    //     }

    //editorLocalizationSystem->Cleanup();

    //SetProjectPath(fileInfo.absolutePath());
    //FilePath projectDirectory = FilePath(path.toStdString()).GetDirectory();

    Project::Settings settings{ path.toStdString() };

    YamlNode* projectRoot = parser->GetRootNode();
    if (nullptr != projectRoot)
    {
        const YamlNode* dataFoldersNode = projectRoot->Get("DataFolders");

        // Get font node
        if (nullptr != dataFoldersNode)
        {
            for (uint32 i = 0; i < dataFoldersNode->GetCount(); i++)
            {
                auto it = dataFoldersNode->Get(i)->AsMap().begin();
                String key = it->first;
                FilePath path = FilePath(it->second->AsString());
                settings.dataFolders.push_back(std::make_pair(key, path));
            }
        }
        else
        {
            FilePath defaultDirectory = FilePath("./Data/");
            QString message = tr("Data source directories not set. Used default directory: %1.").arg(QString::fromStdString(defaultDirectory.GetStringValue()));
            resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
            settings.dataFolders.push_back(std::make_pair("Default", defaultDirectory));
        }

        const YamlNode* fontNode = projectRoot->Get("font");

        // Get font node
        if (nullptr != fontNode)
        {
            // Get default font node
            const YamlNode* defaultFontPath = fontNode->Get("DefaultFontsPath");
            if (nullptr != defaultFontPath)
            {
                FilePath localizationFontsPath(defaultFontPath->AsString());
                if (FileSystem::Instance()->Exists(localizationFontsPath))
                {
                    settings.fontsPath = localizationFontsPath.GetDirectory();
                    //editorFontSystem->SetDefaultFontsPath(localizationFontsPath.GetDirectory());TODO fix
                }
            }
        }

        if (settings.fontsPath.IsEmpty())
        {
            settings.fontsPath = "~res:/UI/Fonts/";
        }
        //
        //         editorFontSystem->LoadLocalizedFonts();

        const YamlNode* localizationPathNode = projectRoot->Get("LocalizationPath");
        const YamlNode* localeNode = projectRoot->Get("Locale");
        if (localizationPathNode != nullptr && localeNode != nullptr)
        {
            FilePath localePath = localizationPathNode->AsString();
            //QString absPath = QString::fromStdString(localePath.GetAbsolutePathname());
            //QDir localePathDir(absPath);
            //editorLocalizationSystem->SetDirectory(localePathDir);
            settings.stringLocalizationsPath = localePath;
            settings.currentLocale = localeNode->AsString();

            //QString currentLocale = QString::fromStdString(localeNode->AsString());
            //editorLocalizationSystem->SetCurrentLocaleValue(currentLocale);
        }

        const YamlNode* libraryNode = projectRoot->Get("Library");
        if (libraryNode != nullptr)
        {
            for (uint32 i = 0; i < libraryNode->GetCount(); i++)
            {
                settings.libraryPackages.push_back(FilePath(libraryNode->Get(i)->AsString()));
            }
        }
    }

    return std::make_tuple(std::make_unique<Project>(settings), resultList);
}

void EditorCore::OnCloseProject()
{
    CloseProject();
}

bool EditorCore::CloseProject()
{
    if (project.get() == nullptr /* !project->IsOpen()*/)
    {
        return true;
    }
    auto documents = documentGroup->GetDocuments();
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
            documentGroup->SaveAllDocuments();
        }
    }

    for (auto& document : documentGroup->GetDocuments())
    {
        documentGroup->CloseDocument(document);
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

void EditorCore::OnNewProject()
{
    Result result;
    auto projectPath = CreateNewProject(&result);
    if (result)
    {
        OpenProject(projectPath);
    }
    else if (result.type == Result::RESULT_ERROR)
    {
        QMessageBox::warning(qApp->activeWindow(), tr("error while creating project"), tr("Can not create new project: %1").arg(result.message.c_str()));
    }
}

void EditorCore::OnProjectOpenChanged(bool isOpen)
{
    if (isOpen)
    {
        mainWindow->libraryWidget->SetLibraryPackages(project->GetLibraryPackages());
    }
    else
    {
        mainWindow->libraryWidget->SetLibraryPackages(Vector<FilePath>());
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
    }
}

void EditorCore::DisableCacheClient()
{
    if (cacheClient != nullptr && cacheClient->IsConnected())
    {
        cacheClient->Disconnect();
        cacheClient.reset();
    }
}

void EditorCore::OnProjectOpen(const Project* newProject)
{
    mainWindow->actionClose_project->setEnabled(true);
    mainWindow->fileSystemDockWidget->setEnabled(true);
    mainWindow->fileSystemDockWidget->SetProjectDir(newProject->GetProjectPath());
    //connect(newProject, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    //connect(newProject, &Project::ProjectPathChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::SetProjectDir);
    //connect(newProject, &Project::IsOpenChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::setEnabled);
    //connect(newProject, &Project::IsOpenChanged, this, &EditorCore::OnProjectOpenChanged);

    EditorLocalizationSystem* editorLocalizationSystem = newProject->GetEditorLocalizationSystem();

    mainWindow->SetRecentProjects(GetProjectsHistory());
    mainWindow->SetLocales(editorLocalizationSystem->GetAvailableLocaleNames(), editorLocalizationSystem->GetCurrentLocale());
    mainWindow->setWindowTitle(ResourcesManageHelper::GetProjectTitle());

    QComboBox* languageComboBox = mainWindow->GetComboBoxLanguage();
    connect(languageComboBox, &QComboBox::currentTextChanged, editorLocalizationSystem, &EditorLocalizationSystem::SetCurrentLocale);
    connect(editorLocalizationSystem, &EditorLocalizationSystem::CurrentLocaleChanged, languageComboBox, &QComboBox::setCurrentText);

    if (assetCacheEnabled)
    {
        EnableCacheClient();
    }

    spritesPacker->SetCacheClient(cacheClient.get(), "QuickEd.ReloadSprites");

    QRegularExpression searchOption("gfx\\d*$", QRegularExpression::CaseInsensitiveOption);
    spritesPacker->ClearTasks();
    QDirIterator it(/*projectPath + */ "/DataSource"); //TODO fix
    while (it.hasNext())
    {
        it.next();
        const QFileInfo& fileInfo = it.fileInfo();
        if (fileInfo.isDir())
        {
            QString outputPath = fileInfo.absoluteFilePath();
            if (!outputPath.contains(searchOption))
            {
                continue;
            }
            outputPath.replace(outputPath.lastIndexOf("DataSource"), QString("DataSource").size(), "Data");
            QDir outputDir(outputPath);
            spritesPacker->AddTask(fileInfo.absoluteFilePath(), outputDir);
        }
    }
}

void EditorCore::OnProjectClose(const Project* currProject)
{
    mainWindow->actionClose_project->setEnabled(false);
    mainWindow->fileSystemDockWidget->setEnabled(false);
    mainWindow->fileSystemDockWidget->SetProjectDir(QString());

    //disconnect(currProject, &Project::IsOpenChanged, mainWindow->actionClose_project, &QAction::setEnabled);
    //disconnect(currProject, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    //disconnect(currProject, &Project::ProjectPathChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::SetProjectDir);
    //disconnect(currProject, &Project::IsOpenChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::setEnabled);
    //disconnect(currProject, &Project::IsOpenChanged, this, &EditorCore::OnProjectOpenChanged);

    QComboBox* languageComboBox = mainWindow->GetComboBoxLanguage();
    EditorLocalizationSystem* editorLocalizationSystem = currProject->GetEditorLocalizationSystem();
    disconnect(languageComboBox, &QComboBox::currentTextChanged, editorLocalizationSystem, &EditorLocalizationSystem::SetCurrentLocale);
    disconnect(editorLocalizationSystem, &EditorLocalizationSystem::CurrentLocaleChanged, languageComboBox, &QComboBox::setCurrentText);

    DisableCacheClient();
}

QStringList EditorCore::GetProjectsHistory() const
{
    QString history = QString::fromStdString(projectsHistory);
    return history.split("\n", QString::SkipEmptyParts);
}

void EditorCore::OnOpenProject()
{
    QString defaultPath;
    if (project.get() != nullptr)
    {
        defaultPath = project->GetProjectPath() + project->GetProjectName();
    }
    else
    {
        defaultPath = QDir::currentPath();
    }

    QString projectPath = FileDialog::getOpenFileName(mainWindow.get(), tr("Select a project file"),
                                                      defaultPath,
                                                      tr("Project (*.uieditor)"));

    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);

    OpenProject(projectPath);
}

QString EditorCore::CreateNewProject(Result* result /*=nullptr*/)
{
    if (result == nullptr)
    {
        Result dummy; //code cleaner
        result = &dummy;
    }
    QString projectDirPath = QFileDialog::getExistingDirectory(qApp->activeWindow(), tr("Select directory for new project"));
    if (projectDirPath.isEmpty())
    {
        *result = Result(Result::RESULT_FAILURE, String("Operation cancelled"));
        return "";
    }
    bool needOverwriteProjectFile = true;
    QDir projectDir(projectDirPath);
    const QString projectFileName = Project::GetProjectFileName();
    QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
    if (QFile::exists(fullProjectFilePath))
    {
        if (QMessageBox::Yes == QMessageBox::question(qApp->activeWindow(), tr("Project file exists!"), tr("Project file %1 exists! Open this project?").arg(fullProjectFilePath)))
        {
            return fullProjectFilePath;
        }

        *result = Result(Result::RESULT_FAILURE, String("Operation cancelled"));
        return "";
    }
    QFile projectFile(fullProjectFilePath);
    if (!projectFile.open(QFile::WriteOnly | QFile::Truncate)) // create project file
    {
        *result = Result(Result::RESULT_ERROR, String("Can not open project file ") + fullProjectFilePath.toUtf8().data());
        return "";
    }
    if (!projectDir.mkpath(projectDir.canonicalPath() + Project::GetScreensRelativePath()))
    {
        *result = Result(Result::RESULT_ERROR, String("Can not create Data/UI folder"));
        return "";
    }
    return fullProjectFilePath;
}

#include "EditorCore.h"

#include "Engine/Public/Engine.h"
#include "Engine/Public/EngineContext.h"
#include "Engine/Public/Qt/RenderWidget.h"
#include "Engine/Public/NativeService.h"

#include "Logger/Logger.h"
#include "Particles/ParticleEmitter.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"

#include "Helpers/ResourcesManageHelper.h"
#include "TextureCompression/PVRConverter.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/AssertGuard.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/Utils/Utils.h"

#include "Document/DocumentGroup.h"
#include "Document/Document.h"
#include "Model/PackageHierarchy/PackageNode.h"

#include "UI/mainwindow.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "UI/FileSystemView/FileSystemModel.h"
#include "UI/Package/PackageModel.h"

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(EditorCore, PREF_ARG("isUsingAssetCache", false));

namespace EditorCoreDetails
{
void UnpackHelp(DAVA::FileSystem* fileSystem)
{
    //Unpack Help to Documents.
    DAVA::FilePath docsPath = DAVA::FilePath(ResourcesManageHelper::GetDocumentationPath().toStdString());
    if (!fileSystem->Exists(docsPath))
    {
        try
        {
            DAVA::ResourceArchive helpRA("~res:/Help.docs");

            fileSystem->DeleteDirectory(docsPath);
            fileSystem->CreateDirectory(docsPath, true);

            helpRA.UnpackToFolder(docsPath);
        }
        catch (std::exception& ex)
        {
            DVASSERT_MSG("can't unpack help docs to documents dir: %s", ex.what());
        }
    }
}

void InitPVRTexTool()
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);
}
}

EditorCore::EditorCore()
    : QObject(nullptr)
    , Singleton<EditorCore>()
{
    ParticleEmitter::FORCE_DEEP_CLONE = true;
    ToolsAssetGuard::Instance()->Init();
}

EditorCore::~EditorCore()
{
    if (cacheClient && cacheClient->IsConnected())
    {
        cacheClient->Disconnect();
    }

    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

MainWindow* EditorCore::GetMainWindow() const
{
    return mainWindow.get();
}

Project* EditorCore::GetProject() const
{
    return project;
}

void EditorCore::ConnectInternal()
{
    connect(mainWindow.get(), &MainWindow::CanClose, this, &EditorCore::CloseProject);
    connect(mainWindow->actionReloadSprites, &QAction::triggered, this, &EditorCore::OnReloadSpritesStarted);
    connect(spritesPacker.get(), &SpritesPacker::Finished, this, &EditorCore::OnReloadSpritesFinished);

    connect(mainWindow->actionClose_project, &QAction::triggered, this, &EditorCore::CloseProject);
    connect(project, &Project::IsOpenChanged, mainWindow->actionClose_project, &QAction::setEnabled);
    connect(project, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    connect(project, &Project::ProjectPathChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::SetProjectDir);
    connect(mainWindow->actionNew_project, &QAction::triggered, this, &EditorCore::OnNewProject);
    connect(project, &Project::IsOpenChanged, mainWindow->fileSystemDockWidget, &FileSystemDockWidget::setEnabled);
    connect(project, &Project::IsOpenChanged, this, &EditorCore::OnProjectOpenChanged);

    connect(mainWindow.get(), &MainWindow::CloseProject, this, &EditorCore::CloseProject);
    connect(mainWindow.get(), &MainWindow::ActionExitTriggered, this, &EditorCore::OnExit);
    connect(mainWindow.get(), &MainWindow::RecentMenuTriggered, this, &EditorCore::RecentMenu);
    connect(mainWindow.get(), &MainWindow::ActionOpenProjectTriggered, this, &EditorCore::OpenProject);
    connect(mainWindow.get(), &MainWindow::OpenPackageFile, documentGroup, &DocumentGroup::AddDocument);
    connect(mainWindow.get(), &MainWindow::RtlChanged, this, &EditorCore::OnRtlChanged);
    connect(mainWindow.get(), &MainWindow::BiDiSupportChanged, this, &EditorCore::OnBiDiSupportChanged);
    connect(mainWindow.get(), &MainWindow::GlobalStyleClassesChanged, this, &EditorCore::OnGlobalStyleClassesChanged);

    QComboBox* languageComboBox = mainWindow->GetComboBoxLanguage();
    EditorLocalizationSystem* editorLocalizationSystem = project->GetEditorLocalizationSystem();
    connect(languageComboBox, &QComboBox::currentTextChanged, editorLocalizationSystem, &EditorLocalizationSystem::SetCurrentLocale);
    connect(editorLocalizationSystem, &EditorLocalizationSystem::CurrentLocaleChanged, languageComboBox, &QComboBox::setCurrentText);

    PreviewWidget* previewWidget = mainWindow->previewWidget;

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::SaveSystemsContextAndClear); //this context will affect other widgets, so he must be updated before other widgets take new document
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::OnDocumentChanged);
    connect(mainWindow.get(), &MainWindow::EmulationModeChanged, previewWidget, &PreviewWidget::OnEmulationModeChanged);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow.get(), &MainWindow::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);

    PackageWidget* packageWidget = mainWindow->packageWidget;
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

    connect(project->GetEditorLocalizationSystem(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &EditorCore::UpdateLanguage);

    connect(documentGroup, &DocumentGroup::ActiveDocumentChanged, previewWidget, &PreviewWidget::LoadSystemsContext); //this context will affect other widgets, so he must be updated when other widgets took new document
}

void EditorCore::Init(DAVA::Engine& engine)
{
    EngineContext* context = engine.GetContext();

    context->logger->SetLogFilename("QuickEd.txt");

    const char* settingsPath = "QuickEdSettings.archive";
    FilePath localPrefrencesPath(context->fileSystem->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    EditorCoreDetails::UnpackHelp(context->fileSystem);
    EditorCoreDetails::InitPVRTexTool();

    qInstallMessageHandler(DAVAMessageHandler);

    Q_INIT_RESOURCE(QtToolsResources);
    Themes::InitFromQApplication();

    DAVA::RenderWidget* renderWidget = engine.GetNativeService()->GetRenderWidget();

    spritesPacker.reset(new SpritesPacker());
    project = new Project(this);
    documentGroup = new DocumentGroup(this);
    mainWindow.reset(new MainWindow());

    mainWindow->previewWidget->InjectRenderWidget(renderWidget);

    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->AttachDocumentGroup(documentGroup, renderWidget);

    mainWindow->RebuildRecentMenu(project->GetProjectsHistory());

    //we need to register preferences when whole class is initialized
    PreferencesStorage::Instance()->RegisterPreferences(this);

    ConnectInternal();

    mainWindow->show();
}

void EditorCore::OnRenderingInitialized()
{
    mainWindow->previewWidget->OnWindowCreated();
    QStringList projectsPathes = project->GetProjectsHistory();
    if (!projectsPathes.isEmpty())
    {
        OpenProject(projectsPathes.last());
    }
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

    Sprite::ReloadSprites(Texture::GetDefaultGPU());
}

void EditorCore::OnProjectPathChanged(const QString& projectPath)
{
    DisableCacheClient();
    if (projectPath.isEmpty())
    {
        return;
    }
    if (assetCacheEnabled)
    {
        EnableCacheClient();
    }

    spritesPacker->SetCacheClient(cacheClient.get(), "QuickEd.ReloadSprites");

    QRegularExpression searchOption("gfx\\d*$", QRegularExpression::CaseInsensitiveOption);
    spritesPacker->ClearTasks();
    QDirIterator it(projectPath + "/DataSource");
    while (it.hasNext())
    {
        const QFileInfo& fileInfo = it.fileInfo();
        it.next();
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

void EditorCore::RecentMenu(QAction* recentProjectAction)
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

    if (project->CanOpenProject(path))
    {
        if (!project->Open(path))
        {
            QString message = tr("Error while opening project %1").arg(path);
            resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        }
    }
    else
    {
        QString message = tr("Can not open project %1").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
    }
    mainWindow->OnProjectOpened(resultList, project);
}

bool EditorCore::CloseProject()
{
    if (!project->IsOpen())
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
    project->Close();
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
    auto projectPath = project->CreateNewProject(&result);
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
    AssetCache::Error connected = cacheClient->ConnectSynchronously(connectionParams);
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

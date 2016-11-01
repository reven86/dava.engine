#include "Project.h"

#include "Document/Document.h"
#include "Document/DocumentGroup.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/YamlPackageSerializer.h"
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"
#include "UI/ProjectView.h"

#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/ProjectInformation/ProjectStructure.h"
#include "QtTools/FileDialogs/FindFileDialog.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"

#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

using namespace DAVA;

Project::Project(MainWindow::ProjectView* aView, const Settings& aSettings)
    : QObject(nullptr)
    , view(aView)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , documentGroup(new DocumentGroup(this, view->GetDocumentGroupView()))
    , spritesPacker(new SpritesPacker())
    , projectStructure(new ProjectStructure(QStringList() << "yaml"))
    , settings(aSettings)
    , projectDirectory(QFileInfo(aSettings.projectFile).absolutePath())
    , projectName(QFileInfo(aSettings.projectFile).fileName())
{
    settings.resourceDirectory = projectDirectory + settings.resourceDirectory;
    settings.additionalResourceDirectory = projectDirectory + settings.additionalResourceDirectory;
    settings.intermediateResourceDirectory = projectDirectory + settings.intermediateResourceDirectory;

    if (QDir(settings.additionalResourceDirectory).exists())
    {
        FilePath::AddResourcesFolder(FilePath(settings.additionalResourceDirectory.toStdString()));
    }

    FilePath::AddResourcesFolder(FilePath(settings.intermediateResourceDirectory.toStdString()));
    FilePath::AddResourcesFolder(FilePath(settings.resourceDirectory.toStdString()));

    editorFontSystem->SetDefaultFontsPath(settings.fontsConfigsDirectory);
    editorFontSystem->LoadLocalizedFonts();

    editorLocalizationSystem->SetDirectory(QDir(QString::fromStdString(settings.textsDirectory.GetAbsolutePathname())));
    editorLocalizationSystem->SetCurrentLocale(QString::fromStdString(settings.defaultLanguage));

    QDir uiDirectoryPath(settings.resourceDirectory + Project::GetUIRelativePath());
    if (!uiDirectoryPath.exists())
    {
        uiDirectoryPath = QDir(settings.additionalResourceDirectory + Project::GetUIRelativePath());
    }
    DVASSERT(uiDirectoryPath.exists());
    uiResourcesPath = uiDirectoryPath.absolutePath();

    projectStructure->AddProjectDirectory(uiResourcesPath);
    view->SetResourceDirectory(uiResourcesPath);

    view->SetProjectActionsEnabled(true);
    view->SetProjectPath(GetProjectPath());
    view->SetLanguages(GetAvailableLanguages(), GetCurrentLanguage());

    connect(editorLocalizationSystem.get(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &Project::CurrentLanguageChanged, Qt::DirectConnection);
    connect(editorFontSystem.get(), &EditorFontSystem::FontPresetChanged, documentGroup.get(), &DocumentGroup::FontPresetChanged, Qt::DirectConnection);

    connect(view, &MainWindow::ProjectView::CurrentLanguageChanged, this, &Project::SetCurrentLanguage);
    connect(view, &MainWindow::ProjectView::RtlChanged, this, &Project::SetRtl);
    connect(view, &MainWindow::ProjectView::BiDiSupportChanged, this, &Project::SetBiDiSupport);
    connect(view, &MainWindow::ProjectView::GlobalStyleClassesChanged, this, &Project::SetGlobalStyleClasses);
    connect(view, &MainWindow::ProjectView::ReloadSprites, this, &Project::OnReloadSprites);
    connect(view, &MainWindow::ProjectView::FindFileInProject, this, &Project::FindFileInProject);

    connect(this, &Project::CurrentLanguageChanged, view, &MainWindow::ProjectView::SetCurrentLanguage);

    connect(spritesPacker.get(), &SpritesPacker::Finished, this, &Project::OnReloadSpritesFinished);

    spritesPacker->ClearTasks();
    for (const auto& gfxOptions : settings.gfxDirectories)
    {
        QDir gfxDirectory(settings.resourceDirectory + gfxOptions.first);
        QDir gfxOutDirectory(settings.intermediateResourceDirectory + gfxOptions.first);
        DVASSERT(gfxDirectory.exists());
        spritesPacker->AddTask(gfxDirectory, gfxOutDirectory);
    }
}

Project::~Project()
{
    view->SetLanguages(QStringList(), QString());
    view->SetProjectPath(QString());
    view->SetProjectActionsEnabled(false);

    projectStructure->RemoveProjectDirectory(uiResourcesPath);
    view->SetResourceDirectory(QString());

    editorLocalizationSystem->Cleanup();
    editorFontSystem->ClearAllFonts();
    FilePath::RemoveResourcesFolder(FilePath(settings.resourceDirectory.toStdString()));
    FilePath::RemoveResourcesFolder(FilePath(settings.additionalResourceDirectory.toStdString()));
    FilePath::RemoveResourcesFolder(FilePath(settings.intermediateResourceDirectory.toStdString()));
}

const Vector<FilePath>& Project::GetLibraryPackages() const
{
    return settings.libraryPackages;
}

std::tuple<Project::Settings, ResultList> Project::ParseProjectSettings(const QString& projectFile)
{
    ResultList resultList;

    ScopedPtr<YamlParser> parser(YamlParser::Create(projectFile.toStdString()));
    if (!parser || parser->GetRootNode() == nullptr)
    {
        QString message = tr("Can not parse project file %1.").arg(projectFile);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return std::make_tuple(Project::Settings(), resultList);
    }

    YamlNode* projectRoot = parser->GetRootNode();

    const YamlNode* headerNode = projectRoot->Get("Header");

    int version = 0;
    if (headerNode != nullptr)
    {
        const YamlNode* versionNode = headerNode->Get("version");
        if (versionNode != nullptr && versionNode->AsInt32())
        {
            version = versionNode->AsInt32();
        }
    }

    if (version != CURRENT_PROJECT_FILE_VERSION)
    {
        return ParseLegacyProjectSettings(projectFile, projectRoot, version);
    }

    return ParseActualProjectSettings(projectFile, projectRoot);
}

const QString& Project::GetUIRelativePath()
{
    static const QString relativePath("/UI");
    return relativePath;
}

const QString& Project::GetProjectFileName()
{
    static const QString projectFile("ui.uieditor");
    return projectFile;
}

const QStringList& Project::GetFontsFileExtensionFilter()
{
    static const QStringList filter(QStringList() << "*.ttf"
                                                  << "*.otf"
                                                  << "*.fon"
                                                  << "*.fnt"
                                                  << "*.def"
                                                  << "*.df");
    return filter;
}

const QString& Project::GetGraphicsFileExtensionFilter()
{
    static const QString filter("*.psd");
    return filter;
}

const QString& Project::Get3dFileExtensionFilter()
{
    static const QString filter("*.sc2");
    return filter;
}

const QString& Project::GetUIFileExtensionFilter()
{
    static const QString filter("*.yaml");
    return filter;
}

const QString& Project::GetUIFileExtension()
{
    static const QString extension(".yaml");
    return extension;
}

QStringList Project::GetAvailableLanguages() const
{
    return editorLocalizationSystem->GetAvailableLocales();
}

QString Project::GetCurrentLanguage() const
{
    return editorLocalizationSystem->GetCurrentLocale();
}

void Project::SetCurrentLanguage(const QString& newLanguageCode)
{
    editorLocalizationSystem->SetCurrentLocale(newLanguageCode);
    editorFontSystem->RegisterCurrentLocaleFonts();

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

const QStringList& Project::GetDefaultPresetNames() const
{
    return editorFontSystem->GetDefaultPresetNames();
}

EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem.get();
}

void Project::SetRtl(bool isRtl)
{
    UIControlSystem::Instance()->SetRtl(isRtl);

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void Project::SetBiDiSupport(bool support)
{
    UIControlSystem::Instance()->SetBiDiSupportEnabled(support);

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void Project::SetGlobalStyleClasses(const QString& classesStr)
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

const QString& Project::SourceResourceDirectory() const
{
    return settings.resourceDirectory;
}

void Project::OnReloadSprites()
{
    for (auto& document : documentGroup->GetDocuments())
    {
        if (!documentGroup->TryCloseDocument(document))
        {
            return;
        }
    }

    view->ExecDialogReloadSprites(spritesPacker.get());
}

void Project::OnReloadSpritesFinished()
{
    Sprite::ReloadSprites();
}

bool Project::TryCloseAllDocuments()
{
    auto documents = documentGroup->GetDocuments();
    bool hasUnsaved = std::find_if(documents.begin(), documents.end(), [](Document* document) { return document->CanSave(); }) != documents.end();

    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
        view->mainWindow,
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

    return true;
}

std::tuple<Project::Settings, DAVA::ResultList> Project::ParseActualProjectSettings(const QString& projectFile, const YamlNode* root)
{
    ResultList resultList;
    Project::Settings settings{ projectFile };

    const YamlNode* projectPropertiesNode = root->Get("ProjectProperties");
    if (projectPropertiesNode == nullptr)
    {
        QString message = tr("Wrong project properties in file %1.").arg(projectFile);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return std::make_tuple(Project::Settings(), resultList);
    }

    const YamlNode* resourceDirNode = projectPropertiesNode->Get("ResourceDirectory");
    if (resourceDirNode != nullptr)
    {
        settings.resourceDirectory = QString::fromStdString(resourceDirNode->AsString());
    }
    else
    {
        QString defaultDirectory = "/DataSource/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(defaultDirectory);
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.resourceDirectory = defaultDirectory;
    }

    const YamlNode* additionalResourceDirNode = projectPropertiesNode->Get("AdditionalResourceDirectory");
    if (additionalResourceDirNode != nullptr)
    {
        settings.additionalResourceDirectory = QString::fromStdString(additionalResourceDirNode->AsString());
    }

    const YamlNode* intermediateResourceDirNode = projectPropertiesNode->Get("IntermediateResourceDirectory");
    if (intermediateResourceDirNode != nullptr)
    {
        settings.intermediateResourceDirectory = QString::fromStdString(intermediateResourceDirNode->AsString());
    }
    else
    {
        QString defaultDirectory = "/Data/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(defaultDirectory);
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.intermediateResourceDirectory = defaultDirectory;
    }

    const YamlNode* gfxDirsNode = projectPropertiesNode->Get("GfxDirectories");
    if (gfxDirsNode != nullptr)
    {
        for (uint32 index = 0; index < gfxDirsNode->GetCount(); ++index)
        {
            const YamlNode* gfxDirNode = gfxDirsNode->Get(index);
            DVASSERT(gfxDirNode);
            QString directory = QString::fromStdString(gfxDirNode->Get("directory")->AsString());
            Vector2 res = gfxDirNode->Get("resolution")->AsVector2();
            QSize resolution(res.dx, res.dy);
            settings.gfxDirectories.push_back(std::make_pair(directory, resolution));
        }
    }
    else
    {
        QString defaultDirectory = "/Gfx/";
        QSize defaultResolution(960, 640);
        QString message = tr("Data source directories not set. Used default directory: %1, with resoulution %2x%3.")
                          .arg(defaultDirectory)
                          .arg(defaultResolution.width())
                          .arg(defaultResolution.height());
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.gfxDirectories.push_back(std::make_pair(defaultDirectory, defaultResolution));
    }

    const YamlNode* fontsDirNode = projectPropertiesNode->Get("FontsDirectory");
    if (fontsDirNode != nullptr)
    {
        settings.fontsDirectory = fontsDirNode->AsString();
    }
    else
    {
        String defaultDirectory = "~res:/Fonts/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(QString::fromStdString(defaultDirectory));
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.fontsDirectory = defaultDirectory;
    }

    const YamlNode* fontsConfigsDirNode = projectPropertiesNode->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        settings.fontsConfigsDirectory = fontsConfigsDirNode->AsString();
    }
    else
    {
        String defaultDirectory = "~res:/UI/Fonts/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(QString::fromStdString(defaultDirectory));
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.fontsConfigsDirectory = defaultDirectory;
    }

    const YamlNode* textsDirNode = projectPropertiesNode->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        settings.textsDirectory = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = projectPropertiesNode->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            settings.defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        String defaultDirectory = "~res:/Strings/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(QString::fromStdString(defaultDirectory));
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.textsDirectory = defaultDirectory;
    }

    const YamlNode* libraryNode = projectPropertiesNode->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            settings.libraryPackages.push_back(FilePath(libraryNode->Get(i)->AsString()));
        }
    }
    return std::make_tuple(std::move(settings), resultList);
}

std::tuple<Project::Settings, DAVA::ResultList> Project::ParseLegacyProjectSettings(const QString& projectFile, const YamlNode* root, int version)
{
    ResultList resultList;
    Project::Settings settings{ projectFile };

    const YamlNode* fontNode = root->Get("font");

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
                settings.fontsConfigsDirectory = localizationFontsPath.GetDirectory();
            }
        }
    }

    const YamlNode* localizationPathNode = root->Get("LocalizationPath");
    const YamlNode* localeNode = root->Get("Locale");
    if (localizationPathNode != nullptr && localeNode != nullptr)
    {
        FilePath localePath = localizationPathNode->AsString();
        settings.textsDirectory = localePath;
        settings.defaultLanguage = localeNode->AsString();
    }

    const YamlNode* libraryNode = root->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            settings.libraryPackages.push_back(FilePath(libraryNode->Get(i)->AsString()));
        }
    }

    QString defaultDirectory = "/Gfx/";
    QSize defaultResolution(960, 640);
    settings.gfxDirectories.push_back(std::make_pair(defaultDirectory, defaultResolution));

    return std::make_tuple(std::move(settings), resultList);
}

void Project::FindFileInProject()
{
    QString filePath = FindFileDialog::GetFilePath(projectStructure.get(), "yaml", view->mainWindow);
    if (filePath.isEmpty())
    {
        return;
    }
    view->SelectFile(filePath);
    //emit OpenPackageFile(filePath); TODO fix
    documentGroup->AddDocument(filePath);
}

void Project::SetAssetCacheClient(DAVA::AssetCacheClient* newCacheClient)
{
    spritesPacker->SetCacheClient(newCacheClient, "QuickEd.ReloadSprites");
}

// QString Project::CreateNewProject(Result* result /*=nullptr*/)
// {
//     if (result == nullptr)
//     {
//         Result dummy; //code cleaner
//         result = &dummy;
//     }
//     QString projectDirPath = QFileDialog::getExistingDirectory(qApp->activeWindow(), tr("Select directory for new project"));
//     if (projectDirPath.isEmpty())
//     {
//         *result = Result(Result::RESULT_FAILURE, String("Operation cancelled"));
//         return "";
//     }
//     bool needOverwriteProjectFile = true;
//     QDir projectDir(projectDirPath);
//     const QString projectFileName = GetProjectFileName();
//     QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
//     if (QFile::exists(fullProjectFilePath))
//     {
//         if (QMessageBox::Yes == QMessageBox::question(qApp->activeWindow(), tr("Project file exists!"), tr("Project file %1 exists! Open this project?").arg(fullProjectFilePath)))
//         {
//             return fullProjectFilePath;
//         }
//
//         *result = Result(Result::RESULT_FAILURE, String("Operation cancelled"));
//         return "";
//     }
//     QFile projectFile(fullProjectFilePath);
//     if (!projectFile.open(QFile::WriteOnly | QFile::Truncate)) // create project file
//     {
//         *result = Result(Result::RESULT_ERROR, String("Can not open project file ") + fullProjectFilePath.toUtf8().data());
//         return "";
//     }
//     if (!projectDir.mkpath(projectDir.canonicalPath() + GetScreensRelativePath()))
//     {
//         *result = Result(Result::RESULT_ERROR, String("Can not create Data/UI folder"));
//         return "";
//     }
//     return fullProjectFilePath;
// }

QString Project::GetProjectPath() const
{
    return settings.projectFile;
}

QString Project::GetProjectDirectory() const
{
    return projectDirectory;
}

QString Project::GetProjectName() const
{
    return projectName;
}

#include "Project.h"

#include "Document/DocumentGroup.h"
#include "Document/Document.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"

#include "UI/UIControlSystem.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/FileSystem.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "UI/mainwindow.h"
#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "UI/Library/LibraryWidget.h"

#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

using namespace DAVA;

Project::Project(MainWindow* aMainWindow, const Settings& aSettings)
    : QObject(nullptr)
    , mainWindow(aMainWindow)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , documentGroup(new DocumentGroup(mainWindow, this))
    , spritesPacker(new SpritesPacker(this))
    , settings(aSettings)
    , projectDirectory(QFileInfo(aSettings.projectFile).absolutePath())
    , projectName(QFileInfo(aSettings.projectFile).fileName())
{
    for (auto& folderInfo : settings.sourceResourceDirectories)
    {
        folderInfo.second = projectDirectory + folderInfo.second;
        FilePath::AddResourcesFolder(FilePath(folderInfo.second.toStdString()));
    }

    settings.intermediateResourceDirectory = projectDirectory + settings.intermediateResourceDirectory;

    editorFontSystem->SetDefaultFontsPath(settings.fontsConfigsDirectory);
    editorFontSystem->LoadLocalizedFonts();

    editorLocalizationSystem->SetDirectory(QDir(QString::fromStdString(settings.textsDirectory.GetAbsolutePathname())));
    editorLocalizationSystem->SetCurrentLocale(QString::fromStdString(settings.defaultLanguage));

    for (auto& item : SourceResourceDirectories())
    {
        QFileInfo pathInfo = item.second + Project::GetUIRelativePath();
        QString path = pathInfo.absoluteFilePath();
        QString displayName = item.first;

        mainWindow->GetFileSystemWidget()->AddPath(path, displayName);
    }
    mainWindow->GetLibraryWidget()->SetLibraryPackages(GetLibraryPackages());

    mainWindow->SetProjectActionsEnabled(true);
    mainWindow->SetProjectPath(GetProjectPath());
    mainWindow->SetLanguages(GetAvailableLanguages(), GetCurrentLanguage());

    connect(editorLocalizationSystem.get(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &Project::CurrentLanguageChanged, Qt::DirectConnection);
    connect(editorFontSystem.get(), &EditorFontSystem::FontPresetChanged, documentGroup.get(), &DocumentGroup::FontPresetChanged, Qt::DirectConnection);

    connect(mainWindow, &MainWindow::CurrentLanguageChanged, this, &Project::SetCurrentLanguage);
    connect(mainWindow, &MainWindow::RtlChanged, this, &Project::SetRtl);
    connect(mainWindow, &MainWindow::BiDiSupportChanged, this, &Project::SetBiDiSupport);
    connect(mainWindow, &MainWindow::GlobalStyleClassesChanged, this, &Project::SetGlobalStyleClasses);

    connect(this, &Project::CurrentLanguageChanged, mainWindow, &MainWindow::SetCurrentLanguage);
    connect(mainWindow, &MainWindow::ReloadSprites, this, &Project::OnReloadSprites);
    connect(spritesPacker.get(), &SpritesPacker::Finished, this, &Project::OnReloadSpritesFinished);

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

Project::~Project()
{
    disconnect(mainWindow, &MainWindow::CurrentLanguageChanged, this, &Project::SetCurrentLanguage);
    disconnect(mainWindow, &MainWindow::RtlChanged, this, &Project::SetRtl);
    disconnect(mainWindow, &MainWindow::BiDiSupportChanged, this, &Project::SetBiDiSupport);
    disconnect(mainWindow, &MainWindow::GlobalStyleClassesChanged, this, &Project::SetGlobalStyleClasses);

    disconnect(this, &Project::CurrentLanguageChanged, mainWindow, &MainWindow::SetCurrentLanguage);

    mainWindow->SetLanguages(QStringList(), QString());
    mainWindow->SetProjectPath(QString());
    mainWindow->SetProjectActionsEnabled(false);

    mainWindow->GetLibraryWidget()->SetLibraryPackages(Vector<FilePath>());
    for (auto& item : SourceResourceDirectories())
    {
        QFileInfo pathInfo = item.second + Project::GetUIRelativePath();
        QString path = pathInfo.absoluteFilePath();

        mainWindow->GetFileSystemWidget()->RemovePath(path);
    }
    mainWindow->GetFileSystemWidget()->RemoveAllPaths();

    editorLocalizationSystem->Cleanup();
    editorFontSystem->ClearAllFonts();
    for (auto& folderInfo : settings.sourceResourceDirectories)
    {
        FilePath::RemoveResourcesFolder(FilePath(folderInfo.second.toStdString()));
    }
}
// bool Project::OpenInternal(const QString& path)
// {
//     // Attempt to create a project
//     ScopedPtr<YamlParser> parser(YamlParser::Create(path.toStdString()));
//     if (!parser)
//     {
//         return false;
//     }
//
//     QFileInfo fileInfo(path);
//     if (!fileInfo.exists())
//     {
//         return false;
//     }
//     SetProjectName(fileInfo.fileName());
//
//     editorLocalizationSystem->Cleanup();
//
//     QDir projectDir = fileInfo.absoluteDir();
//     if (!projectDir.mkpath("." + GetScreensRelativePath()))
//     {
//         return false;
//     }
//
//     editorLocalizationSystem->Cleanup();
//
//     SetProjectPath(fileInfo.absolutePath());
//
//     YamlNode* projectRoot = parser->GetRootNode();
//     if (nullptr != projectRoot)
//     {
//         const YamlNode* dataFoldersNode = projectRoot->Get("DataFolders");
//
//         // Get font node
//         if (nullptr != dataFoldersNode)
//         {
//             for (uint32 i = 0; i < dataFoldersNode->GetCount(); i++)
//             {
//                 auto it = dataFoldersNode->Get(i)->AsMap().begin();
//                 String key = it->first;
//                 FilePath path = FilePath(projectPath.GetAbsolutePathname() + it->second->AsString());
//                 dataFolders.push_back(std::make_pair(key, path));
//             }
//         }
//         else
//         {
//             dataFolders.push_back(std::make_pair("Default", "./Data/"));
//         }
//
//         const YamlNode* fontNode = projectRoot->Get("font");
//
//         // Get font node
//         if (nullptr != fontNode)
//         {
//             // Get default font node
//             const YamlNode* defaultFontPath = fontNode->Get("DefaultFontsPath");
//             if (nullptr != defaultFontPath)
//             {
//                 FilePath localizationFontsPath(defaultFontPath->AsString());
//                 if (FileSystem::Instance()->Exists(localizationFontsPath))
//                 {
//                     editorFontSystem->SetDefaultFontsPath(localizationFontsPath.GetDirectory());
//                 }
//             }
//         }
//
//         if (editorFontSystem->GetDefaultFontsPath().IsEmpty())
//         {
//             editorFontSystem->SetDefaultFontsPath(FilePath(projectPath.GetAbsolutePathname() + "Data/UI/Fonts/"));
//         }
//
//         editorFontSystem->LoadLocalizedFonts();
//
//         const YamlNode* localizationPathNode = projectRoot->Get("LocalizationPath");
//         const YamlNode* localeNode = projectRoot->Get("Locale");
//         if (localizationPathNode != nullptr && localeNode != nullptr)
//         {
//             FilePath localePath = localizationPathNode->AsString();
//             QString absPath = QString::fromStdString(localePath.GetAbsolutePathname());
//             QDir localePathDir(absPath);
//             editorLocalizationSystem->SetDirectory(localePathDir);
//
//             QString currentLocale = QString::fromStdString(localeNode->AsString());
//             editorLocalizationSystem->SetCurrentLocaleValue(currentLocale);
//         }
//         const YamlNode* libraryNode = projectRoot->Get("Library");
//         libraryPackages.clear();
//         if (libraryNode != nullptr)
//         {
//             for (uint32 i = 0; i < libraryNode->GetCount(); i++)
//             {
//                 libraryPackages.push_back(FilePath(libraryNode->Get(i)->AsString()));
//             }
//         }
//     }
//
//     return true;
// }

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

    Project::Settings settings{ projectFile };

    YamlNode* projectRoot = parser->GetRootNode();

    const YamlNode* sourceResourceDirsNode = projectRoot->Get("SourceResourceDirectories");
    if (nullptr != sourceResourceDirsNode)
    {
        for (uint32 i = 0; i < sourceResourceDirsNode->GetCount(); i++)
        {
            auto it = sourceResourceDirsNode->Get(i)->AsMap().begin();
            QString alias = QString::fromStdString(it->first);
            QString directory = QString::fromStdString(it->second->AsString());
            settings.sourceResourceDirectories.push_back(qMakePair(alias, directory));
        }
    }
    else
    {
        QString defaultDirectory = "./DataSource/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(defaultDirectory);
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.sourceResourceDirectories.push_back(qMakePair(QString("Default"), defaultDirectory));
    }

    const YamlNode* intermediateResourceDirNode = projectRoot->Get("IntermediateResourceDirectory");
    if (nullptr != intermediateResourceDirNode)
    {
        settings.intermediateResourceDirectory = QString::fromStdString(intermediateResourceDirNode->AsString());
    }
    else
    {
        QString defaultDirectory = "./quickEd/Data/";
        QString message = tr("Data source directories not set. Used default directory: %1.").arg(defaultDirectory);
        resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
        settings.intermediateResourceDirectory = defaultDirectory;
    }

    const YamlNode* fontsDirNode = projectRoot->Get("FontsDirectory");
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

    const YamlNode* fontsConfigsDirNode = projectRoot->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        settings.fontsConfigsDirectory = fontsConfigsDirNode->AsString();
    }
    else
    {
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
                    settings.fontsConfigsDirectory = localizationFontsPath.GetDirectory();
                }
            }
        }
        else
        {
            String defaultDirectory = "~res:/UI/Fonts/";
            QString message = tr("Data source directories not set. Used default directory: %1.").arg(QString::fromStdString(defaultDirectory));
            resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
            settings.fontsConfigsDirectory = defaultDirectory;
        }
    }

    const YamlNode* textsDirNode = projectRoot->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        settings.textsDirectory = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = projectRoot->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            settings.defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        const YamlNode* localizationPathNode = projectRoot->Get("LocalizationPath");
        const YamlNode* localeNode = projectRoot->Get("Locale");
        if (localizationPathNode != nullptr && localeNode != nullptr)
        {
            FilePath localePath = localizationPathNode->AsString();
            //QString absPath = QString::fromStdString(localePath.GetAbsolutePathname());
            //QDir localePathDir(absPath);
            //editorLocalizationSystem->SetDirectory(localePathDir);
            settings.textsDirectory = localePath;
            settings.defaultLanguage = localeNode->AsString();

            //QString currentLocale = QString::fromStdString(localeNode->AsString());
            //editorLocalizationSystem->SetCurrentLocaleValue(currentLocale);
        }
        else
        {
            String defaultDirectory = "~res:/Strings/";
            QString message = tr("Data source directories not set. Used default directory: %1.").arg(QString::fromStdString(defaultDirectory));
            resultList.AddResult(Result::RESULT_WARNING, message.toStdString());
            settings.textsDirectory = defaultDirectory;
        }
    }

    const YamlNode* libraryNode = projectRoot->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            settings.libraryPackages.push_back(FilePath(libraryNode->Get(i)->AsString()));
        }
    }

    return std::make_tuple(settings, resultList);
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

// DocumentGroup* Project::GetDocumentGroup() const
// {
//     return documentGroup.get();
// }

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

const QVector<QPair<QString, QString>>& Project::SourceResourceDirectories() const
{
    return settings.sourceResourceDirectories;
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

    mainWindow->ExecDialogReloadSprites(spritesPacker.get());
}

void Project::OnReloadSpritesFinished()
{
    //     if (cacheClient)
    //     {
    //         cacheClient->Disconnect();
    //         cacheClient.reset();
    //     }

    Sprite::ReloadSprites();
}

bool Project::TryCloseAllDocuments()
{
    auto documents = documentGroup->GetDocuments();
    bool hasUnsaved = std::find_if(documents.begin(), documents.end(), [](Document* document) { return document->CanSave(); }) != documents.end();

    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
        mainWindow,
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

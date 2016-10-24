#include "DAVAEngine.h"

#include "Project.h"
#include "EditorFontSystem.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Helpers/ResourcesManageHelper.h"

#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

using namespace DAVA;

Project::Project(const Settings& aSettings)
    : QObject(nullptr)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , settings(aSettings)
    , projectDirectory(aSettings.projectPath.GetDirectory())
    , projectName(aSettings.projectPath.GetFilename())
{
    for (auto& folderInfo : settings.dataFolders)
    {
        FilePath::AddResourcesFolder(projectDirectory + folderInfo.second);
    }
    //FilePath::AddResourcesFolder(projectDirectory + "Data/");

    editorFontSystem->SetDefaultFontsPath(settings.fontsPath);
    //editorFontSystem->SetDefaultFontsPath(FilePath(projectPath.GetAbsolutePathname() + "Data/UI/Fonts/"));
    editorFontSystem->LoadLocalizedFonts();

    editorLocalizationSystem->SetDirectory(QDir(QString::fromStdString(settings.stringLocalizationsPath.GetAbsolutePathname())));
    editorLocalizationSystem->SetCurrentLocaleValue(QString::fromStdString(settings.currentLocale));
}

Project::~Project()
{
    editorLocalizationSystem->Cleanup();
    editorFontSystem->ClearAllFonts();
    for (auto& folderInfo : settings.dataFolders)
    {
        FilePath::RemoveResourcesFolder(projectDirectory + folderInfo.second);
    }

    //SetProjectName("");
    //SetProjectPath("");
}

// bool Project::Open(const QString& path)
// {
//     bool result = OpenInternal(path);
//     return result;
// }

// void Project::Close()
// {
//     FilePath::RemoveResourcesFolder(projectPath + "Data/");
//
//     SetProjectName("");
//     SetProjectPath("");
// }

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

// bool Project::CanOpenProject(const QString& projectPath) const
// {
//     if (projectPath.isEmpty())
//     {
//         return false; //this is not performace fix. QDir return true for empty path
//     }
//     QFileInfo fileInfo(projectPath);
//     return fileInfo.exists() && fileInfo.isFile();
// }

const Vector<FilePath>& Project::GetLibraryPackages() const
{
    return settings.libraryPackages;
}

EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem.get();
}

EditorLocalizationSystem* Project::GetEditorLocalizationSystem() const
{
    return editorLocalizationSystem.get();
}

const QString& Project::GetScreensRelativePath()
{
    static const QString relativePath("/UI");
    return relativePath;
}

const QString& Project::GetProjectFileName()
{
    static const QString projectFile("ui.uieditor");
    return projectFile;
}

const DAVA::Vector<std::pair<DAVA::String, DAVA::String>>& Project::GetDataFolders() const
{
    return settings.dataFolders;
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

// void Project::SetIsOpen(bool arg) //TODO fix
// {
//     if (isOpen == arg)
//     {
//         //return; //TODO: implement this after we create CloseProject function
//     }
//     isOpen = arg;
//     if (arg)
//     {
//         ResourcesManageHelper::SetProjectPath(QString::fromStdString(projectPath.GetAbsolutePathname()));
//         QString newProjectPath = GetProjectPath() + GetProjectName();
//         //         QStringList projectsPathes = GetProjectsHistory(); TODO fix
//         //         projectsPathes.removeAll(newProjectPath);
//         //         projectsPathes += newProjectPath;
//         //         while (static_cast<DAVA::uint32>(projectsPathes.size()) > projectsHistorySize)
//         //         {
//         //             projectsPathes.removeFirst();
//         //         }
//         //         projectsHistory = projectsPathes.join('\n').toStdString();
//     }
//     emit IsOpenChanged(arg);
// }
QString Project::GetProjectPath() const
{
    return QString::fromStdString(settings.projectPath.GetAbsolutePathname());
}

QString Project::GetProjectDirectory() const
{
    return QString::fromStdString(projectDirectory.GetAbsolutePathname());
}

QString Project::GetProjectName() const
{
    return QString::fromStdString(projectName);
}

// void Project::SetProjectPath(QString arg)
// {
//     if (GetProjectPath() != arg)
//     {
//         if (!projectPath.IsEmpty())
//         {
//             FilePath::RemoveResourcesFolder(projectPath + "Data/");
//         }
//         projectPath = arg.toStdString().c_str();
//         if (!projectPath.IsEmpty())
//         {
//             projectPath.MakeDirectoryPathname();
//             FilePath::AddResourcesFolder(projectPath + "Data/");
//         }
//         emit ProjectPathChanged(arg);
//     }
// }

// void Project::SetProjectName(QString arg)
// {
//     if (projectName != arg)
//     {
//         projectName = arg;
//         emit ProjectNameChanged(arg);
//     }
// }

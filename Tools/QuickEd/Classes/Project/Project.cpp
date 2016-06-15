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

REGISTER_PREFERENCES_ON_START(Project,
                              PREF_ARG("projectsHistory", DAVA::String()),
                              PREF_ARG("projectsHistorySize", static_cast<DAVA::uint32>(5))
                              )

Project::Project(QObject* parent)
    : QObject(parent)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , isOpen(false)
{
}

bool Project::Open(const QString& path)
{
    bool result = OpenInternal(path);
    SetIsOpen(result);
    return result;
}

void Project::Close()
{
    FilePath::RemoveResourcesFolder(projectPath + "Data/");

    SetProjectName("");
    SetProjectPath("");
    SetIsOpen(false);
}

bool Project::OpenInternal(const QString& path)
{
    // Attempt to create a project
    ScopedPtr<YamlParser> parser(YamlParser::Create(path.toStdString()));
    if (!parser)
    {
        return false;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        return false;
    }
    SetProjectName(fileInfo.fileName());

    editorLocalizationSystem->Cleanup();

    QDir projectDir = fileInfo.absoluteDir();
    if (!projectDir.mkpath("." + GetScreensRelativePath()))
    {
        return false;
    }

    editorLocalizationSystem->Cleanup();

    SetProjectPath(fileInfo.absolutePath());

    YamlNode* projectRoot = parser->GetRootNode();
    if (nullptr != projectRoot)
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
                    editorFontSystem->SetDefaultFontsPath(localizationFontsPath.GetDirectory());
                }
            }
        }

        if (editorFontSystem->GetDefaultFontsPath().IsEmpty())
        {
            editorFontSystem->SetDefaultFontsPath(FilePath(projectPath.GetAbsolutePathname() + "Data/UI/Fonts/"));
        }

        editorFontSystem->LoadLocalizedFonts();

        const YamlNode* localizationPathNode = projectRoot->Get("LocalizationPath");
        const YamlNode* localeNode = projectRoot->Get("Locale");
        if (localizationPathNode != nullptr && localeNode != nullptr)
        {
            FilePath localePath = localizationPathNode->AsString();
            QString absPath = QString::fromStdString(localePath.GetAbsolutePathname());
            QDir localePathDir(absPath);
            editorLocalizationSystem->SetDirectory(localePathDir);

            QString currentLocale = QString::fromStdString(localeNode->AsString());
            editorLocalizationSystem->SetCurrentLocaleValue(currentLocale);
        }
    }

    return true;
}

bool Project::CanOpenProject(const QString& projectPath) const
{
    if (projectPath.isEmpty())
    {
        return false; //this is not performace fix. QDir return true for empty path
    }
    QFileInfo fileInfo(projectPath);
    return fileInfo.exists() && fileInfo.isFile();
}

EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem;
}

EditorLocalizationSystem* Project::GetEditorLocalizationSystem() const
{
    return editorLocalizationSystem;
}

const QString& Project::GetScreensRelativePath()
{
    static const QString relativePath("/Data/UI");
    return relativePath;
}

const QString& Project::GetProjectFileName()
{
    static const QString projectFile("ui.uieditor");
    return projectFile;
}

QString Project::CreateNewProject(Result* result /*=nullptr*/)
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
    const QString projectFileName = GetProjectFileName();
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
    if (!projectDir.mkpath(projectDir.canonicalPath() + GetScreensRelativePath()))
    {
        *result = Result(Result::RESULT_ERROR, String("Can not create Data/UI folder"));
        return "";
    }
    return fullProjectFilePath;
}

bool Project::IsOpen() const
{
    return isOpen;
}

void Project::SetIsOpen(bool arg)
{
    if (isOpen == arg)
    {
        //return; //TODO: implement this after we create CloseProject function
    }
    isOpen = arg;
    if (arg)
    {
        ResourcesManageHelper::SetProjectPath(QString::fromStdString(projectPath.GetAbsolutePathname()));
        QString newProjectPath = GetProjectPath() + GetProjectName();
        QStringList projectsPathes = GetProjectsHistory();
        projectsPathes.removeAll(newProjectPath);
        projectsPathes += newProjectPath;
        while (static_cast<DAVA::uint32>(projectsPathes.size()) > projectsHistorySize)
        {
            projectsPathes.removeFirst();
        }
        projectsHistory = projectsPathes.join('\n').toStdString();
    }
    emit IsOpenChanged(arg);
}

QString Project::GetProjectPath() const
{
    return QString::fromStdString(projectPath.GetAbsolutePathname());
}

QString Project::GetProjectName() const
{
    return projectName;
}

QStringList Project::GetProjectsHistory() const
{
    QString history = QString::fromStdString(projectsHistory);
    return history.split("\n", QString::SkipEmptyParts);
}

void Project::SetProjectPath(QString arg)
{
    if (GetProjectPath() != arg)
    {
        if (!projectPath.IsEmpty())
        {
            FilePath::RemoveResourcesFolder(projectPath + "Data/");
        }
        projectPath = arg.toStdString().c_str();
        if (!projectPath.IsEmpty())
        {
            projectPath.MakeDirectoryPathname();
            FilePath::AddResourcesFolder(projectPath + "Data/");
        }
        emit ProjectPathChanged(arg);
    }
}

void Project::SetProjectName(QString arg)
{
    if (projectName != arg)
    {
        projectName = arg;
        emit ProjectNameChanged(arg);
    }
}

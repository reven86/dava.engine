/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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

Project::Project(QObject* parent)
    : QObject(parent)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , isOpen(false)
{
}

Project::~Project()
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

    FilePath::RemoveResourcesFolder(projectPath);
    editorLocalizationSystem->Cleanup();

    QDir projectDir = fileInfo.absoluteDir();
    if (!projectDir.mkpath("." + GetScreensRelativePath()))
    {
        return false;
    }

    FilePath::RemoveResourcesFolder(projectPath + "Data/");
    editorLocalizationSystem->Cleanup();

    SetProjectPath(fileInfo.absolutePath());
    projectPath.MakeDirectoryPathname();

    const auto& resFolders = FilePath::GetResourcesFolders();
    const auto& searchIt = find(resFolders.begin(), resFolders.end(), projectPath);

    if (searchIt == resFolders.end())
    {
        FilePath::AddResourcesFolder(projectPath + "Data/");
    }

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

void Project::SetProjectPath(QString arg)
{
    if (GetProjectPath() != arg)
    {
        projectPath = arg.toStdString().c_str();
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

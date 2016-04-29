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


#include "ResourcesManageHelper.h"
#include "Core/Core.h"
#include "EditorSettings.h"
#include <QString>
#include <QStringList>
#include <QDir>


using namespace DAVA;

namespace ResourcesManageHelperLocal
{
// True type fonts resource folder path
const String FONTS_RES_PATH("~res:/Fonts/");
// Graphics fonts definition resource folder path
const String GRAPHICS_FONTS_RES_PATH("~res:/Fonts/");
// Documentation path.
const QString DOCUMENTATION_PATH = "~doc:/UIEditorHelp/";
// Project DATA folder
const QString PROJECT_DATA = "%1/Data";
// Project file path
const QString PROJECT_FILE_PATH = "%1ui.uieditor";
// Default project title
const QString PROJECT_TITLE = "QuickEd";
//Available fonts extensions
const QStringList FONTS_EXTENSIONS_FILTER = (QStringList() << "*.ttf"
                                                           << "*.otf"
                                                           << "*.fon"
                                                           << "*.fnt"
                                                           << "*.def"
                                                           << "*.df");
}

QString ResourcesManageHelper::projectTitle;
QString ResourcesManageHelper::projectPath;

QString ResourcesManageHelper::GetFontRelativePath(const QString& resourceFileName, bool graphicsFont)
{
    using namespace ResourcesManageHelperLocal;
    QString fontPath = graphicsFont ? QString::fromStdString(FilePath(GRAPHICS_FONTS_RES_PATH).GetAbsolutePathname())
                                      :
                                      QString::fromStdString(FilePath(FONTS_RES_PATH).GetAbsolutePathname());
    fontPath += resourceFileName;

    return fontPath;
}

QStringList ResourcesManageHelper::GetFontsList()
{
    using namespace ResourcesManageHelperLocal;
    QStringList filesNamesList;
    // Get true type fonts
    // Get absoulute path
    QString fontsPath = QString::fromStdString(FilePath(FONTS_RES_PATH).GetAbsolutePathname());
    QDir dir(fontsPath);
    // Get the list of files in fonts directory - both true type fonts and graphics fonts
    filesNamesList = dir.entryList(FONTS_EXTENSIONS_FILTER, QDir::Files);
    fontsPath.clear();
    return filesNamesList;
}

void ResourcesManageHelper::InitInternalResources()
{
    using namespace ResourcesManageHelperLocal;
    // Save project default title
    if (DAVA::Core::Instance())
    {
        DAVA::KeyedArchive* options = DAVA::Core::Instance()->GetOptions();
        if (options)
        {
            projectTitle = options->GetString("title", PROJECT_TITLE.toStdString()).c_str();
        }
    }
    // If project name wasn't set - create default name
    if (projectTitle.isNull() || projectTitle.isEmpty())
        projectTitle = PROJECT_TITLE;
}

QString ResourcesManageHelper::GetDocumentationPath()
{
    return ResourcesManageHelperLocal::DOCUMENTATION_PATH;
}

void ResourcesManageHelper::SetProjectPath(const QString& path)
{
    projectPath = path;
}

QString ResourcesManageHelper::GetProjectPath()
{
    return projectPath;
}

QString ResourcesManageHelper::GetProjectTitle()
{
    // Set default project title
    QString projectTitleString = projectTitle;
    // Get active project path
    QString projectPath = GetProjectPath();
    if (!projectPath.isNull() && !projectPath.isEmpty())
    {
        projectTitleString = GetProjectTitle(GetProjectFilePath(projectPath));
    }

    return projectTitleString;
}

QString ResourcesManageHelper::GetProjectTitle(const QString& projectFilePath)
{
    return QString("%1 | Project %2").arg(projectTitle).arg(projectFilePath);
}

QString ResourcesManageHelper::GetDefaultDirectory()
{
    QString defaultDir = QString::fromStdString(EditorSettings::Instance()->GetProjectPath());
    //If default directory path is not available in project settings - use current working path
    if (defaultDir.isNull() || defaultDir.isEmpty())
    {
        defaultDir = QDir::currentPath();
    }
    return defaultDir;
}

QString ResourcesManageHelper::GetResourceRootDirectory()
{
    QString projectPath = GetProjectPath();
    if (projectPath.isNull() || projectPath.isEmpty())
    {
        return QString();
    }
    return GetDataPath(projectPath);
}

QString ResourcesManageHelper::GetDataPath(const QString& projectPath)
{
    return QString(ResourcesManageHelperLocal::PROJECT_DATA).arg(projectPath);
}

QString ResourcesManageHelper::GetProjectFilePath(const QString& projectPath)
{
    return QString(ResourcesManageHelperLocal::PROJECT_FILE_PATH).arg(projectPath);
}

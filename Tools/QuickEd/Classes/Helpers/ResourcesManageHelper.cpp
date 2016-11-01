#include "ResourcesManageHelper.h"
#include "Engine/Engine.h"
#include "Utils/StringFormat.h"
#include "DAVAVersion.h"
#include "QtTools/Utils/Utils.h"
#include "version.h"
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
const char* PROJECT_TITLE = "DAVA Framework - QuickEd | %s-%s [%u bit]";
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
    QString fontPath = graphicsFont ? QString::fromStdString(FilePath(GRAPHICS_FONTS_RES_PATH).GetAbsolutePathname()) :
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
    projectTitle = QString::fromStdString(DAVA::Format(PROJECT_TITLE, DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION, static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8)));
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

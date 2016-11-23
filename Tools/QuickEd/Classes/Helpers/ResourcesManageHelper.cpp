#include "ResourcesManageHelper.h"
#include "QtTools/Utils/Utils.h"
#include "Project/Project.h"
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
}

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
    filesNamesList = dir.entryList(Project::GetFontsFileExtensionFilter(), QDir::Files);
    fontsPath.clear();
    return filesNamesList;
}

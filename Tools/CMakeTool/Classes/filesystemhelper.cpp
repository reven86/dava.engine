#include "filesystemhelper.h"
#include <QDir>
#include <QRegularExpression>

FileSystemHelper::FileSystemHelper(QObject *parent) : QObject(parent)
{

}

QVariant FileSystemHelper::resolveUrl(QVariant url)
{
    if(!url.canConvert<QString>())
    {
        return "";
    }
    QString str = url.toString();
    QRegularExpression regExp;
#ifdef Q_OS_MAC
    regExp.setPattern("^(file:/{2})"); //on unix systems path started with '/'
#elif defined Q_OS_WIN
    regExp.setPattern("^(file:/{3})");
#endif //Q_OS_MAC Q_OS_WIN;
    str.replace(regExp, "");
    return str;
}

QVariant FileSystemHelper::isDirExists(QVariant dirPath)
{
    if(!dirPath.canConvert<QString>() || dirPath.toString().isEmpty())
    {
        return false;
    }
    QDir dir(dirPath.toString());
    return dir.exists();
}

QVariant FileSystemHelper::FindCMakeBin(QVariant pathToDavaFramework)
{
    if (!pathToDavaFramework.canConvert<QString>())
    {
        return "";
    }
    QString davaFolder = "dava.framework";
    QString davaPath = pathToDavaFramework.toString();
    int index = davaPath.indexOf(davaFolder);
    if (index == -1)
    {
        return "";
    }
    davaPath = davaPath.left(davaPath.indexOf(index + davaFolder.length()));
    QString cmakePath = davaPath + "/Tools/Bin" +
#ifdef Q_OS_MAC
    "/CMake.app/Contents/bin/cmake";
#elif defined Q_OS_WIN
    "/cmake/bin/cmake.exe";
#endif //Q_OS_MAC Q_OS_WIN
    if (!QFile::exists(cmakePath))
    {
        return "";
    }
    return cmakePath;
}

QVariant FileSystemHelper::ClearBuildFolder(QVariant buildFolder)
{
    if (!buildFolder.canConvert<QString>())
    {
        return false;
    }
    QString folderPath = buildFolder.toString();
    QDir dir(folderPath);
    if (folderPath.isEmpty() || !dir.exists())
    {
        return false;
    }
    if (dir.removeRecursively())
    {
        return dir.mkpath(folderPath);
    }
    return false;
}

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
    regExp.setPattern("^(file:/{2})");
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

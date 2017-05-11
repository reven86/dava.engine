#include "QtHelpers/HelperFunctions.h"

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>
#include <QApplication>

#ifdef Q_OS_MAC
#include <QUrl>
#include <CoreFoundation/CoreFoundation.h>
#endif //Q_OS_MAC

namespace QtHelpers
{
void ShowInOSFileManager(const QString& path)
{
#if defined(Q_OS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + path + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(path);
    QProcess::startDetached("explorer", args);
#endif //
}

//realisation for windows, only calls given function
#ifdef Q_OS_WIN
void InvokeInAutoreleasePool(std::function<void()> function)
{
    function();
}
#endif
QString GetApplicationFilePath()
{
#ifdef Q_OS_MAC
    CFURLRef url = (CFURLRef)CFAutorelease((CFURLRef)CFBundleCopyBundleURL(CFBundleGetMainBundle()));
    QString appPath = QUrl::fromCFURL(url).path();
    //launcher will use app as a file, but not as a folder
    while (appPath.endsWith('/'))
    {
        appPath.chop(1);
    }
    //sometimes CFBundleCopyBundleURL returns url with a double slashes
    appPath.replace("//", "/");
#else
    QString appPath = QApplication::applicationFilePath();
#endif //platform
    return appPath;
}

QString GetApplicationDirPath()
{
    QString appPath = GetApplicationFilePath();
    int charIndex = appPath.lastIndexOf('/');
    if (charIndex != -1)
    {
        //grab '/' symbol too
        return appPath.left(charIndex + 1);
    }
    return QString();
}
} // namespace QtHelpers

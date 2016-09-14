#include "QtHelpers/HelperFunctions.h"

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>

namespace QtHelpers
{
void QtHelpers::ShowInOSFileManager(const QString& path)
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

} // namespace QtHelpers

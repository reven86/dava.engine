#include "filemanager.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QDirIterator>
#include <QProcess>
#include <functional>

namespace FileManager
{
const QString tempSelfUpdateDir = "selfupdate/";
const QString baseAppDir = "DAVATools/";
const QString tempDir = "temp/";

QStringList OwnDirectories()
{
    QString path = GetLauncherDirectory();
    return QStringList() << path + tempSelfUpdateDir
                         << path + baseAppDir
                         << path + tempDir;
}

QString GetDocumentsDirectory()
{
    QString docDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/DAVALauncher/";
    MakeDirectory(docDir);
    return docDir;
}

QString GetBaseAppsDirectory()
{
    QString path = GetLauncherDirectory() + baseAppDir;
    MakeDirectory(path);
    return path;
}

QString GetTempDirectory()
{
    QString path = GetLauncherDirectory() + tempDir;
    MakeDirectory(path);
    return path;
}

QString GetSelfUpdateTempDirectory()
{
    QString path = GetLauncherDirectory() + tempSelfUpdateDir;
    MakeDirectory(path);
    return path;
}

QString GetTempDownloadFilePath()
{
    return GetTempDirectory() + "archive.zip";
}

QString GetLauncherDirectory()
{
    QString path =
#if defined(Q_OS_WIN)
    qApp->applicationDirPath();
#elif defined(Q_OS_MAC)
    qApp->applicationDirPath().replace("/Contents/MacOS", "");
    path = path.left(path.lastIndexOf('/'));
#endif //platform
    return path + "/";
}

QString GetPackageInfoFilePath()
{
    return GetLauncherDirectory() + "Launcher.packageInfo";
}

bool CreateFileAndWriteData(const QString& filePath, const QByteArray& data)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        if (file.write(data) == data.size())
        {
            return true;
        }
    }
    return false;
}

bool DeleteDirectory(const QString& path)
{
    QDir dir(path);
    return dir.removeRecursively();
}

namespace FileManager_local
{
bool MoveEntry(const QFileInfo& fileInfo, const QString& newFilePath)
{
    QString absPath = fileInfo.absoluteFilePath();

    QFileInfo destFi(newFilePath);
    if (destFi.exists())
    {
        if (fileInfo.isDir())
        {
            QDir dir(newFilePath);
            if (!dir.remove("."))
            {
                return false;
            }
        }
        else
        {
            if (!QFile::remove(newFilePath))
            {
                return false;
            }
        }
    }
    if (fileInfo.isDir())
    {
        QDir dir(newFilePath);
        return dir.rename(absPath, newFilePath);
    }
    else
    {
        return QFile::rename(absPath, newFilePath);
    }
}
}

bool MoveLauncherRecursively(const QString& pathOut, const QString& pathIn)
{
    QDir outDir(pathOut);
    if (!outDir.exists())
    {
        return false;
    }

    bool success = true;
    QString infoFilePath = GetPackageInfoFilePath();
    bool moveFilesFromInfoList = QFile::exists(infoFilePath);
    QStringList archiveFiles;
    if (moveFilesFromInfoList)
    {
        QFile file(infoFilePath);
        if (file.open(QFile::ReadOnly))
        {
            QString data = QString::fromUtf8(file.readAll());
            archiveFiles = data.split('\n', QString::SkipEmptyParts);
        }
        else
        {
            return false;
        }
    }

    QDirIterator di(outDir.path(), QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    while (di.hasNext())
    {
        di.next();
        const QFileInfo& fi = di.fileInfo();
        QString absPath = fi.absoluteFilePath();
        QString relPath = absPath.right(absPath.length() - pathOut.length());
        if (moveFilesFromInfoList && !archiveFiles.contains(relPath))
        {
            continue;
        }
        //this code need for compability with previous launcher versions
        else if (!moveFilesFromInfoList &&
#if defined(Q_OS_WIN)
                 (fi.suffix() != "dll" && fi.suffix() != "exe")
#elif defined(Q_OS_MAC)
                 fi.fileName() != "Launcher.app"
#endif //platform
                 )
        {
            continue;
        }
        QString newFilePath = pathIn + relPath;
        if (!fi.isDir() || !OwnDirectories().contains(absPath + '/'))
        {
            success &= FileManager_local::MoveEntry(fi, newFilePath);
        }
    }
    return success;
}

void MakeDirectory(const QString& path)
{
    if (!QDir(path).exists())
        QDir().mkpath(path);
}

QString GetApplicationDirectory(const QString& branchID, const QString& appID)
{
    QString path = GetBranchDirectory(branchID) + appID + "/";
    if (!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}

QString GetBranchDirectory(const QString& branchID)
{
    QString path = GetBaseAppsDirectory() + branchID + "/";
    if (!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}
}

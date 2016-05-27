#include "filemanager.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

FileManager* FileManager::instance;

FileManager::FileManager()
{
    QString appDir = QCoreApplication::instance()->applicationDirPath();
#ifdef Q_OS_DARWIN
    appDir.replace("/Contents/MacOS", "");

    int nPos = appDir.lastIndexOf("/");
    if (nPos != -1)
        appDir.chop(appDir.size() - nPos);
#endif

    launcherDir = appDir + "/";
    baseAppDir = appDir + "/DAVATools/";
    tempDir = appDir + "/temp/";
    tempSelfUpdateDir = appDir + "/selfupdate/";
    tempFile = tempDir + "archive.zip";

    docDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/DAVALauncher/";
}

FileManager* FileManager::Instance()
{
    if (!instance)
        instance = new FileManager();
    return instance;
}

const QString& FileManager::GetDocumentsDirectory()
{
    MakeDirectory(docDir);
    return docDir;
}

const QString& FileManager::GetBaseAppsDirectory()
{
    MakeDirectory(baseAppDir);
    return baseAppDir;
}

const QString& FileManager::GetTempDirectory()
{
    MakeDirectory(tempDir);
    return tempDir;
}

const QString& FileManager::GetTempDownloadFilepath()
{
    MakeDirectory(tempDir);
    return tempFile;
}

const QString& FileManager::GetLauncherDirectory()
{
    return launcherDir;
}

const QString& FileManager::GetSelfUpdateTempDirectory()
{
    MakeDirectory(tempSelfUpdateDir);
    return tempSelfUpdateDir;
}

bool FileManager::DeleteDirectory(const QString& path)
{
    bool result = true;
    QDir aDir(path);
    if (aDir.exists())
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                result = DeleteDirectory(path);
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                    result = false;
            }
        }
        if (!aDir.rmdir(aDir.absolutePath()))
            result = false;
    }
    return result;
}

void FileManager::ClearTempDirectory()
{
    FileManager::Instance()->DeleteDirectory(FileManager::Instance()->GetTempDirectory());
}

void FileManager::MakeDirectory(const QString& path)
{
    if (!QDir(path).exists())
        QDir().mkpath(path);
}

void FileManager::MoveFilesOnlyToDirectory(const QString& dirFrom, const QString& dirTo)
{
    QDir fromDir(dirFrom);

    fromDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    fromDir.setNameFilters(QStringList() << "*.exe"
                                         << "*.dll");
    QFileInfoList list = fromDir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
        QFile file(dirFrom + fileName);
        file.rename(dirTo + fileName);
    }

    QDir fromDirMac(dirFrom);

    fromDirMac.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
    fromDirMac.setNameFilters(QStringList() << "Launcher.app");
    list = fromDirMac.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
        QFile file(dirFrom + fileName);
        file.rename(dirTo + fileName);
    }
}

QString FileManager::GetApplicationFolder(const QString& branchID, const QString& appID)
{
    QString path = GetBranchFolder(branchID) + appID + "/";
    if (!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}

QString FileManager::GetBranchFolder(const QString& branchID)
{
    QString path = baseAppDir + branchID + "/";
    if (!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}

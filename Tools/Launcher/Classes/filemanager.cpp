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


#include "filemanager.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QDirIterator>
#include <QProcess>

QString InQuotes(const QString& fileName)
{
    QString result = fileName;
    if (!fileName.startsWith('\"'))
    {
        result.prepend('\"');
    }
    if (!fileName.endsWith('\"'))
    {
        result.append('\"');
    }
    return result;
}

namespace
{
QString docDir;
QString launcherDir;
QString baseAppDir;
QString tempDir;
QString tempFile;
QString backupFile;
QString tempSelfUpdateDir;
QStringList ownDirectories;

#if defined Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif //Q_OS_WIN
}
void FileManager::Init(QCoreApplication* app)
{
#if defined Q_OS_WIN
    //need for isWritable on NTFS. For the detais look to the Qt documentation
    qt_ntfs_permission_lookup++;
#endif //Q_OS_WIN

    QString appDir = app->applicationDirPath();
#ifdef Q_OS_DARWIN
    appDir.replace("/Contents/MacOS", "");

    int nPos = appDir.lastIndexOf("/");
    if (nPos != -1)
        appDir.chop(appDir.size() - nPos);
#endif

    launcherDir = appDir + "/";
    baseAppDir = appDir + "/DAVATools/";
    tempDir = appDir + "/temp/";
    tempFile = tempDir + "archive.zip";
    backupFile = tempDir + "backup.zip";
    docDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/DAVALauncher/";
    tempSelfUpdateDir = appDir + "/selfupdate/";

    ownDirectories << baseAppDir << tempDir << tempSelfUpdateDir;
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

const QString& FileManager::GetBackupDirectory()
{
    MakeDirectory(tempDir);
    return backupFile;
}

const QString& FileManager::GetSelfUpdateTempDirectory()
{
    MakeDirectory(tempSelfUpdateDir);
    return tempSelfUpdateDir;
}

namespace
{
bool IterateDirectory(const QString& dirPath, std::function<bool(const QFileInfo&, const QString& absPath)> callback)
{
    bool success = true;
    // not empty -- we must empty it first
    QDirIterator di(dirPath, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    while (di.hasNext())
    {
        di.next();
        const QFileInfo& fi = di.fileInfo();
        QString absPath = fi.absoluteFilePath();
        if (!fi.isDir() || !ownDirectories.contains(absPath + '/'))
        {
            success &= callback(fi, absPath);
        }
    }
    return success;
}
}

bool FileManager::CheckLauncherFolder(const QString& folder)
{
    QDir dir(folder);
    if (!dir.exists())
    {
        return false;
    }
    return IterateDirectory(dir.path(), [](const QFileInfo& fi, const QString& absPath) {
        if (!fi.isDir() || !ownDirectories.contains(absPath))
        {
            bool ok = fi.isWritable();
            if (fi.isDir())
            {
                ok &= CheckLauncherFolder(absPath);
            }
            return ok;
        }
        return true;
    });
}

const QString& FileManager::GetLauncherDirectory()
{
    return launcherDir;
}

bool FileManager::DeleteDirectory(const QString& path)
{
    QDir dir(path);
    return dir.removeRecursively();
}

bool FileManager::RemoveLauncherFiles()
{
    QDir dir(GetLauncherDirectory());
    if (!dir.exists())
    {
        return true;
    }
    return IterateDirectory(dir.path(), [](const QFileInfo& fi, const QString& absPath) {
        //to prevent situations when absPath is '/' or './'
        if (absPath.length() < 2)
        {
            Q_ASSERT(false);
            return true;
        }
        QString command =
#if defined Q_OS_WIN
        "RMDIR /S";
#elif defined Q_OS_MAC
        "rm -rf";
#else
#error "undefined platform"
#endif //platform
        //compare exit code with zero
        return (QProcess::execute(command + " " + InQuotes(absPath)) == 0);
    });
}

namespace
{
bool CopyDirectoryToDirectoryImpl(const QString& pathOut, const QString& pathIn)
{
    QDir dirOut(pathOut);
    if (!dirOut.exists())
    {
        //no one entry will be copied
        return false;
    }

    QDir inDir(pathIn);
    if (!inDir.mkpath("."))
    {
        return false;
    }
    return IterateDirectory(dirOut.path(), [pathIn](const QFileInfo& fi, const QString& absPath) {
        QString newFilePath = pathIn + "/" + fi.fileName();
        QString command =
#if defined Q_OS_WIN
        "xcopy /s /e /y /i /h /v /r /k";
#elif defined Q_OS_MAC
        "cp -r -f";
#else
#error "undefined platform"
#endif //platform
        //compare exit code with zero
        QStringList commandList = QStringList()
        << command
        << InQuotes(absPath)
        << InQuotes(newFilePath);
        return (QProcess::execute(commandList.join(' ')) == 0);
    });
}
}

bool FileManager::CopyLauncherFilesToDirectory(const QString& launcherPath, const QString& destPath)
{
    return CopyDirectoryToDirectoryImpl(launcherPath, destPath);
}

void FileManager::ClearTempDirectory()
{
    FileManager::DeleteDirectory(FileManager::GetTempDirectory());
}

void FileManager::MakeDirectory(const QString& path)
{
    if (!QDir(path).exists())
        QDir().mkpath(path);
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

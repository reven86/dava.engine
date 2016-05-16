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
#include <functional>

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
const QString tempSelfUpdateDir = "/selfupdate/";
const QString baseAppDir = "/DAVATools/";
const QString tempDir = "/temp/";
const QString tempFile = tempDir + "archive.zip";

QStringList OwnDirectories()
{
    QString path = qApp->applicationDirPath();
    return QStringList() << path + tempSelfUpdateDir
                         << path + baseAppDir
                         << path + tempDir;
}

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
        if (!fi.isDir() || !OwnDirectories().contains(absPath + '/'))
        {
            success &= callback(fi, absPath);
        }
    }
    return success;
}
}

QString FileManager::GetDocumentsDirectory()
{
    QString docDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/DAVALauncher/";
    MakeDirectory(docDir);
    return docDir;
}

QString FileManager::GetBaseAppsDirectory()
{
    QString path = qApp->applicationDirPath() + baseAppDir;
    MakeDirectory(path);
    return path;
}

QString FileManager::GetTempDirectory()
{
    QString path = qApp->applicationDirPath() + tempDir;
    MakeDirectory(path);
    return path;
}

QString FileManager::GetSelfUpdateTempDirectory()
{
    QString path = qApp->applicationDirPath() + tempSelfUpdateDir;
    MakeDirectory(path);
    return path;
}

QString FileManager::GetTempDownloadFilepath()
{
    QString path = qApp->applicationDirPath() + tempFile;
    //make temp directory
    GetTempDirectory();
    return path;
}

QString FileManager::GetLauncherDirectory()
{
    return qApp->applicationDirPath() + "/";
}

bool FileManager::CheckLauncherFolder(const QString& folder)
{
    QDir dir(folder);
    if (!dir.exists())
    {
        return false;
    }
    return IterateDirectory(dir.path(), [](const QFileInfo& fi, const QString& absPath) {
#if defined(Q_OS_WIN)
        bool ok = fi.isReadable(); //isWritable == false on Windows doesn't mean that you can not modify or delete the file
#else
        bool ok = fi.isWritable() && fi.isReadable();
#endif //platform
        if (fi.isDir())
        {
            ok &= CheckLauncherFolder(absPath);
        }
        return ok;
    });
}

bool FileManager::DeleteDirectory(const QString& path)
{
    QDir dir(path);
    return dir.removeRecursively();
}

bool FileManager::MoveLauncherFilesRecursively(const QString& pathOut, const QString& pathIn)
{
    QDir outDir(pathOut);
    if (!outDir.exists())
    {
        return true;
    }
    return IterateDirectory(outDir.path(), [pathIn, pathOut](const QFileInfo& fi, const QString& absPath) {
        QString relPath = absPath.right(absPath.length() - pathOut.length());
        QString newFilePath = pathIn + relPath;
        if (fi.isDir())
        {
            QDir dir(newFilePath);
            return dir.rename(absPath, newFilePath);
        }
        else
        {
            return QFile::rename(absPath, newFilePath);
        }
    });
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
    QString path = qApp->applicationDirPath() + baseAppDir + branchID + "/";
    if (!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}

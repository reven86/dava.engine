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

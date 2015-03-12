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

FileManager * FileManager::instance;

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

FileManager * FileManager::Instance()
{
    if(!instance)
        instance = new FileManager();
    return instance;
}

const QString & FileManager::GetDocumentsDirectory()
{
    MakeDirectory(docDir);
    return docDir;
}

const QString & FileManager::GetBaseAppsDirectory()
{
    MakeDirectory(baseAppDir);
    return baseAppDir;
}

const QString & FileManager::GetTempDirectory()
{
    MakeDirectory(tempDir);
    return tempDir;
}

const QString & FileManager::GetTempDownloadFilepath()
{
    MakeDirectory(tempDir);
    return tempFile;
}

const QString & FileManager::GetLauncherDirectory()
{
    return launcherDir;
}

const QString & FileManager::GetSelfUpdateTempDirectory()
{
    MakeDirectory(tempSelfUpdateDir);
    return tempSelfUpdateDir;
}

bool FileManager::DeleteDirectory(const QString & path)
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

void FileManager::MakeDirectory(const QString & path)
{
    if(!QDir(path).exists())
        QDir().mkpath(path);
}

void FileManager::MoveFilesOnlyToDirectory(const QString & dirFrom, const QString & dirTo)
{
    QDir fromDir(dirFrom);

    fromDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    fromDir.setNameFilters(QStringList() << "*.exe" << "*.dll");
    QFileInfoList list = fromDir.entryInfoList();
    for(int i = 0; i < list.size(); ++i)
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
    for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
        QFile file(dirFrom + fileName);
        file.rename(dirTo + fileName);
    }
}

QString FileManager::GetApplicationFolder(const QString & branchID, const QString & appID)
{
    QString path = GetBranchFolder(branchID) + appID + "/";
    if(!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}

QString FileManager::GetBranchFolder(const QString & branchID)
{
    QString path = baseAppDir + branchID + "/";
    if(!QDir(path).exists())
        QDir().mkpath(path);

    return path;
}

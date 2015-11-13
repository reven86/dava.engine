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


#include "Debug/DVAssert.h"
#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "ProjectFilesWatcher.h"
#include <QFileSystemWatcher>
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>


ProjectFilesWatcher::ProjectFilesWatcher(QObject *parent)
    : QObject(parent)
    , fileSystemWatcher(new QFileSystemWatcher(this))
{
    connect(qApp, &QApplication::applicationStateChanged, this, &ProjectFilesWatcher::OnApplicationStateChanged);

    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &ProjectFilesWatcher::OnFileChanged);
}

void ProjectFilesWatcher::WatchPath(const QString& path)
{
    DVASSERT(!fileSystemWatcher->files().contains(path));
    DVASSERT(QFileInfo::exists(path));
    if(!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
}

void ProjectFilesWatcher::WatchPath(const DAVA::FilePath& davaPath)
{
    DAVA::String filePath = davaPath.GetAbsolutePathname();
    QString path = QString::fromStdString(filePath);
    WatchPath(path);
}

void ProjectFilesWatcher::UnwatchPath(const QString& path)
{
    if(!fileSystemWatcher->removePath(path))
    {
        //qt bug https://bugreports.qt.io/browse/QTBUG-49307 DAVA::Logger::Error("can not remove path from the file watcher %s", path.toUtf8().data());
    }
}

void ProjectFilesWatcher::UnwatchPath(const DAVA::FilePath& davaPath)
{
    DAVA::String filePath = davaPath.GetAbsolutePathname();
    QString path = QString::fromStdString(filePath);
    UnwatchPath(path);
}

void ProjectFilesWatcher::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void ProjectFilesWatcher::OnFileChanged(const QString& path)
{
    changedFiles.insert(path);
}

void ProjectFilesWatcher::ApplyFileChanges()
{
    QStringList changed;
    QStringList removed;
    for (const QString &filePath : changedFiles)
    {
        QFileInfo::exists(filePath) ? changed << filePath : removed << filePath;
    }
    changedFiles.clear();
    if (!changed.empty())
    {
        emit FilesChanged(changed);
    }
    if (!removed.empty())
    {
        emit FilesRemoved(removed);
    }
}

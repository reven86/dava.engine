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


#include "LibraryFileSystemModel.h"

#include "FileSystem/Logger.h"

#include <QDir>
#include <QFileInfoList>
#include <QString>

LibraryFileSystemModel::LibraryFileSystemModel(QObject *parent /* = NULL */)
    : QFileSystemModel(parent)
    , loadingCounter(0)
{
    SetExtensionFilter(QStringList());
    
	connect(this, SIGNAL(directoryLoaded(const QString &)), this, SLOT(DirectoryLoaded(const QString &)));
}


void LibraryFileSystemModel::SetExtensionFilter(const QStringList & extensionFilter)
{
    setNameFilters(extensionFilter);
    setNameFilterDisables(false);

    //magic code for MacOS
    setFilter(QDir::Files);
    setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::AllDirs);
}


void LibraryFileSystemModel::DirectoryLoaded(const QString &path)
{
//disabled for future
//     QDir dir(path);
//     QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
// 
//     auto endIt = entries.end();
//     for (auto it = entries.begin(); it != endIt; ++it)
//     {
//         QModelIndex ind = index(it->absoluteFilePath());
//         if(ind.isValid())
//         {
//             if(canFetchMore(ind))
//             {
//                 ++loadingCounter;
//                 fetchMore(ind);
//             }
//         }
//     }
//     
//     --loadingCounter;
//     if(loadingCounter == 0)
//     {
//         emit ModelLoaded();
//     }
}

void LibraryFileSystemModel::Load(const QString & pathname)
{
	acceptionMap.clear();
	//reset();
    
	loadingCounter = 1;
	
	setRootPath(pathname);
}


void LibraryFileSystemModel::SetAccepted(const QModelIndex &index, bool accepted)
{
    if(index.isValid())
    {
        acceptionMap[fileInfo(index).absoluteFilePath()] = accepted;
    }
}

bool LibraryFileSystemModel::IsAccepted(const QModelIndex & index) const
{
    if(index.isValid())
    {
        return acceptionMap[fileInfo(index).absoluteFilePath()].toBool();
    }
    
    return false;
}


QVariant LibraryFileSystemModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::BackgroundColorRole && IsAccepted(index))
    {
        return QColor(0, 255, 0, 20);
    }
    
    return QFileSystemModel::data(index, role);
}

bool LibraryFileSystemModel::HasFilteredChildren(const QModelIndex &index)
{
    if(!index.isValid()) return false;
    
    QFileInfo info = fileInfo(index);
    if(info.isDir())
    {
        QDir dir = info.filePath();
        
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        dir.setNameFilters(nameFilters());
        
        int filesCount = dir.count();
        if(filesCount > 0) return true;
        
        for(int i = 0; i < rowCount(index); ++i)
        {
            bool hasChildren = HasFilteredChildren(this->index(i, 0, index));
            if(hasChildren) return true;
        }
    }
    
    return false;
}


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


#ifndef __LIBRARY_FILESYSTEM_MODEL_H__
#define __LIBRARY_FILESYSTEM_MODEL_H__

#include <QFileSystemModel>
class  LibraryFileSystemModel: public QFileSystemModel
{
    Q_OBJECT
    
    static const int ACCEPT_ROLE = Qt::UserRole + 5;
    
public:
    explicit LibraryFileSystemModel(QObject *parent = 0);
    
    void Load(const QString & pathname);
    
    void SetExtensionFilter(const QStringList & extensionFilter);

    void SetAccepted(const QModelIndex & index, bool accepted);
    bool IsAccepted(const QModelIndex & index) const;
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool HasFilteredChildren(const QModelIndex &index);
    
signals:
    
    void ModelLoaded();
    
protected slots:
    
    void DirectoryLoaded(const QString &path);
    
protected:
    
    QMap<QString, QVariant> acceptionMap;
    
    uint loadingCounter;
};

#endif // __LIBRARY_FILESYSTEM_MODEL_H__

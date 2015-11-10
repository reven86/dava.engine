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


#ifndef __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__
#define __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__

#include <QDockWidget>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <memory>

namespace Ui {
    class FileSystemDockWidget;
}

class QFileSystemModel;

class FileSystemDockWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit FileSystemDockWidget(QWidget *parent = nullptr);
    void SetProjectDir(const QString &path);

signals:
    void OpenPackageFile(const QString &path);

private slots:
    void OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onDoubleClicked(const QModelIndex &index);
    void setFilterFixedString(const QString &filterStr);
    void onNewFolder();
    void onNewFile();
    void onDeleteFile();

private:
    void RefreshActions(const QModelIndexList &indexList);
    bool CanRemove(const QModelIndex &index) const;
    std::unique_ptr<Ui::FileSystemDockWidget> ui = nullptr;
    QFileSystemModel *model = nullptr;
    QAction *newFolderAction = nullptr;
    QAction *newFileAction = nullptr;
    QAction *deleteAction = nullptr;
};

#endif // __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__

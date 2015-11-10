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


#include "FileSystemDockWidget.h"

#include "ui_FileSystemDockWidget.h"
#include <DAVAEngine.h>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

#include "QtTools/FileDialog/FileDialog.h"


FileSystemDockWidget::FileSystemDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    , model(new QFileSystemModel(this))
{
    ui->setupUi(this);
    ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);

    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*.yaml";
    model->setNameFilters(filters);
    model->setNameFilterDisables(false);

    connect(ui->treeView, &QTreeView::doubleClicked, this, &FileSystemDockWidget::onDoubleClicked);
    connect(ui->filterLine, &QLineEdit::textChanged, this, &FileSystemDockWidget::setFilterFixedString);


    newFolderAction = new QAction(tr("Create folder"), this);
    connect(newFolderAction, &QAction::triggered, this, &FileSystemDockWidget::onNewFolder);

    newFileAction = new QAction(tr("Create file"), this);
    connect(newFileAction, &QAction::triggered, this, &FileSystemDockWidget::onNewFile);

    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
    deleteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(deleteAction, &QAction::triggered, this, &FileSystemDockWidget::onDeleteFile);

    ui->treeView->addAction(newFolderAction);
    ui->treeView->addAction(newFileAction);
    ui->treeView->addAction(deleteAction);
}

FileSystemDockWidget::~FileSystemDockWidget() = default;

void FileSystemDockWidget::SetProjectDir(const QString &path)
{
    if (ui->treeView->selectionModel())
    {
        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }
    QDir dir(path);
    dir.cdUp();
    QString p = dir.path() + "/Data/UI";
    QModelIndex rootIndex = model->setRootPath(p);
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(rootIndex);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
}

void FileSystemDockWidget::RefreshActions( const QModelIndexList &indexList )
{
    bool canCreateFile = true;
    bool canCreateDir = true;
    bool canRemove = !indexList.empty();
    if (indexList.size() == 1)
    {
        QModelIndex selectedIndex = indexList.front();
        bool isDir = model->isDir(selectedIndex);
        canCreateFile = true;
        canCreateDir = isDir;
        canRemove = CanRemove(selectedIndex);
        deleteAction->setText(isDir ? "Remove folder" : "Remove file");
    }
    else
    {
        deleteAction->setText("Remove selected items");
        for (const auto &index : indexList)
        {
            canRemove &= CanRemove(index);
        }
    }
    newFolderAction->setEnabled(canCreateDir);
    newFileAction->setEnabled(canCreateFile);
    deleteAction->setEnabled(canRemove);
}

bool FileSystemDockWidget::CanRemove(const QModelIndex& index) const
{
    if (!model->isDir(index))
    {
        return true;
    }
    else
    {
        QDir dir(model->filePath(index));
        int count = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count();
        return count == 0;
    }
}

void FileSystemDockWidget::OnSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
    RefreshActions(selected.indexes());
}

void FileSystemDockWidget::onDoubleClicked(const QModelIndex &index)
{
    if(!model->isDir(index))
    {
        emit OpenPackageFile(model->filePath(index));
    }
}

void FileSystemDockWidget::setFilterFixedString( const QString &filterStr )
{
    QStringList filters;
    filters << QString("*%1*.yaml").arg(filterStr);
    model->setNameFilters(filters);
}

void FileSystemDockWidget::onNewFolder()
{
    bool ok = false;
    QString folderName = QInputDialog::getText(this, tr("New folder"), tr("Folder name:"), QLineEdit::Normal, tr("New folder"), &ok);
    if (ok && !folderName.isEmpty())
    {
        auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
        DVASSERT(selectedIndexes.empty() || selectedIndexes.size() == 1);
        QModelIndex currIndex = selectedIndexes.empty() ? ui->treeView->rootIndex() : selectedIndexes.front();
        model->mkdir(currIndex, folderName);
    }
}

void FileSystemDockWidget::onNewFile()
{
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selectedIndexes.empty() || selectedIndexes.size() == 1);
    QModelIndex currIndex = selectedIndexes.empty() ? ui->treeView->rootIndex() : selectedIndexes.front();

    QString folderPath = model->filePath(currIndex);
    QString strFile = FileDialog::getSaveFileName(this, tr("Create new file"), folderPath, "*.yaml");
    if (strFile.isEmpty())
    {
        return;
    }
    QFileInfo fileInfo(strFile);
    if (fileInfo.suffix().toLower() != "yaml")
    {
        strFile += ".yaml";
    }

    QFile file(strFile);
    file.open(QIODevice::WriteOnly);
    file.close();
}

void FileSystemDockWidget::onDeleteFile()
{
    bool hasFiles = false;
    bool hasFolders = false;
    const QModelIndexList &indexes = ui->treeView->selectionModel()->selectedIndexes();
    for (const auto &index : indexes)
    {
        bool isDir = model->isDir(index);
        hasFiles |= !isDir;
        hasFolders |= isDir;
    }
    DVASSERT(hasFiles || hasFolders);
    QString text = tr("Delete ") + (hasFiles ? tr("files") : "") + (hasFiles && hasFolders ? tr("and") : "") + (hasFolders ? tr("folders") : "");
    if (QMessageBox::Yes == QMessageBox::question(this, text, text + "?", QMessageBox::Yes | QMessageBox::No))
    {
        for (const auto &index : indexes)
        {
            if (model->isDir(index))
            {
                if (!model->rmdir(index))
                {
                    DAVA::Logger::Error("can not remove folder %s", model->fileName(index).toUtf8().data());
                }
            }
            else
            {
                if (!model->remove(index))
                {
                    DAVA::Logger::Error("can not remove file %s", model->fileName(index).toUtf8().data());
                }
            }
        }
    }
}

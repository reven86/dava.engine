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
#include "FileSystem/Logger.h"

#include "FileSystemDockWidget.h"
#include "ValidatedTextInputDialog.h"
#include "FileSystemModel.h"

#include "ui_FileSystemDockWidget.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QModelIndex>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDirIterator>

#include "QtTools/FileDialog/FileDialog.h"

FileSystemDockWidget::FileSystemDockWidget(QWidget* parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    , model(new FileSystemModel(this))
{
    ui->setupUi(this);
    ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*" + FileSystemModel::GetYamlExtensionString();
    model->setNameFilters(filters);
    model->setNameFilterDisables(false);
    model->setReadOnly(false);

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
    
#if defined Q_OS_WIN
    QString actionName = tr("Show in explorer");
#else if defined Q_OS_MAC
    QString actionName = tr("Show in finder");
#endif //Q_OS_WIN //Q_OS_MAC
    showInSystemExplorerAction = new QAction(actionName, this);
    connect(showInSystemExplorerAction, &QAction::triggered, this, &FileSystemDockWidget::OnShowInExplorer);

    renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this, &FileSystemDockWidget::OnRename);

    openFileAction = new QAction(tr("Open File"), this);
    openFileAction->setShortcuts({ QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter) });
    openFileAction->setShortcutContext(Qt::WidgetShortcut);
    connect(openFileAction, &QAction::triggered, this, &FileSystemDockWidget::OnOpenFile);

    ui->treeView->addAction(newFolderAction);
    ui->treeView->addAction(newFileAction);
    ui->treeView->addAction(deleteAction);
    ui->treeView->addAction(showInSystemExplorerAction);
    ui->treeView->addAction(renameAction);
    ui->treeView->addAction(openFileAction);

    RefreshActions(QModelIndexList());
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
    bool canCreateDir = true;
    bool canRemove = !indexList.empty();
    bool canOpen = false;
    bool canShow = false;
    bool canRename = false;
    if (indexList.size() == 1)
    {
        QModelIndex selectedIndex = indexList.front();
        bool isDir = model->isDir(selectedIndex);
        canCreateDir = isDir;
        canRemove = CanRemove(selectedIndex);
        canOpen = !isDir;
        canShow = true;
        canRename = true;
    }
    newFolderAction->setEnabled(canCreateDir);
    deleteAction->setEnabled(canRemove);
    openFileAction->setEnabled(canOpen);
    openFileAction->setVisible(canOpen);
    showInSystemExplorerAction->setEnabled(canShow);
    showInSystemExplorerAction->setVisible(canShow);
    renameAction->setEnabled(canRename);
    renameAction->setVisible(canRename);
}

bool FileSystemDockWidget::CanRemove(const QModelIndex& index) const
{
    if (!model->isDir(index))
    {
        return true;
    }
    QDir dir(model->filePath(index));
    QDirIterator dirIterator(dir, QDirIterator::Subdirectories);
    while (dirIterator.hasNext())
    {
        if (dirIterator.next().endsWith(FileSystemModel::GetYamlExtensionString()))
        {
            return false;
        }
    }
    return true;
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
    filters << QString("*%1*" + FileSystemModel::GetYamlExtensionString()).arg(filterStr);
    model->setNameFilters(filters);
}

void FileSystemDockWidget::onNewFolder()
{
    ValidatedTextInputDialog dialog(this);
    QString newFolderName = tr("New Folder");
    dialog.setWindowTitle(newFolderName);
    dialog.setInputMethodHints(Qt::ImhUrlCharactersOnly);
    dialog.setLabelText("Enter new folder name:");
    dialog.SetWarningMessage("This folder already exists");

    const auto& selected = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() <= 1);
    QString path;
    if (selected.isEmpty())
    {
        path = model->rootPath();
    }
    else if (selected.size() == 1)
    {
        path = model->filePath(selected.front());
    }
    path += "/";

    auto validateFunction = [path](const QString& text) {
        return !QFileInfo::exists(path + text);
    };

    dialog.SetValidator(validateFunction);

    if (!validateFunction(newFolderName))
    {
        QString newFolderName_ = newFolderName + " (%1)";
        int i = 1;
        do
        {
            newFolderName = newFolderName_.arg(i++);
        } while (!validateFunction(newFolderName));
    }
    dialog.setTextValue(newFolderName);

    int ret = dialog.exec();
    QString folderName = dialog.textValue();
    if (ret == QDialog::Accepted)
    {
        DVASSERT(!folderName.isEmpty());
        auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
        DVASSERT(selectedIndexes.empty() || selectedIndexes.size() == 1);
        QModelIndex currIndex = selectedIndexes.empty() ? ui->treeView->rootIndex() : selectedIndexes.front();
        model->mkdir(currIndex, folderName);
    }
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    RefreshActions(selectedIndexes);
}

void FileSystemDockWidget::onNewFile()
{
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selectedIndexes.empty() || selectedIndexes.size() == 1);
    QModelIndex currIndex = selectedIndexes.empty() ? ui->treeView->rootIndex() : selectedIndexes.front();

    QString folderPath = model->filePath(currIndex);
    QString strFile = FileDialog::getSaveFileName(this, tr("Create new file"), folderPath, "*" + FileSystemModel::GetYamlExtensionString());
    if (strFile.isEmpty())
    {
        return;
    }
    if (!strFile.endsWith(FileSystemModel::GetYamlExtensionString()))
    {
        strFile += FileSystemModel::GetYamlExtensionString();
    }

    QFile file(strFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString title = tr("Can not create file");
        QMessageBox::warning(this, title, title + tr("\n%1").arg(strFile));
        DAVA::Logger::Error("%s", QString(title + ": %1").arg(strFile).toUtf8().data());
    }
    file.close();
    RefreshActions(selectedIndexes);
}

void FileSystemDockWidget::onDeleteFile()
{
    const QModelIndexList& indexes = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(indexes.size() == 1);
    auto index = indexes.front();
    bool isDir = model->isDir(index);
    QString title = tr("Delete ") + (isDir ? "folder" : "file") + "?";
    QString text = tr("Delete ") + (isDir ? "folder" : "file") + " \"" + model->fileName(index) + "\"" + (isDir ? " and its content" : "") + "?";
    if (QMessageBox::Yes == QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::No))
    {
        if (!model->remove(index))
        {
            DAVA::Logger::Error("can not remove file %s", model->isDir(index) ? "folder" : "file", model->fileName(index).toUtf8().data());
        }
    }
    RefreshActions(indexes);
}

void FileSystemDockWidget::OnShowInExplorer()
{
    const QModelIndexList& indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (indexes.size() != 1)
    {
        return;
    }
    QString pathIn = model->fileInfo(indexes.first()).absoluteFilePath();
#ifdef Q_OS_MAC
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + pathIn + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#endif
#ifdef Q_OS_WIN
    QString param;
    param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(pathIn);
    QString command = QString("explorer") + " " + param;
    QProcess::startDetached(command);
#endif
}

void FileSystemDockWidget::OnRename()
{
    const auto& selected = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    ui->treeView->edit(selected.first());
}

void FileSystemDockWidget::OnOpenFile()
{
    const auto& selected = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    onDoubleClicked(selected.first());
}

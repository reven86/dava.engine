//
//  FileSystemTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.9.14.
//
//
#include "FileSystemDockWidget.h"

#include "ui_FileSystemDockWidget.h"
#include <DAVAEngine.h>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

FileSystemDockWidget::FileSystemDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    , model(NULL)
{
    model = new QFileSystemModel(this);
    ui->setupUi(this);
    
    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*.yaml";
    model->setNameFilters(filters);
    model->setNameFilterDisables(false);

    connect(ui->treeView, SIGNAL(doubleClicked (const QModelIndex &)), this, SLOT(onDoubleClicked(const QModelIndex &)));
    connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
    
    connect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(setFilterFixedString(const QString &)));

    ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);

    newFolderAction = new QAction(tr("Create folder"), this);
    connect(newFolderAction, SIGNAL(triggered()), this, SLOT(onNewFolder()));

    newFileAction = new QAction(tr("Create file"), this);
    connect(newFileAction, SIGNAL(triggered(bool)), this, SLOT(onNewFile(bool)));

    delFileAction = new QAction(tr("Delete"), this);
    delFileAction->setShortcut(QKeySequence(QKeySequence::Delete));
    delFileAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delFileAction, SIGNAL(triggered(bool)), this, SLOT(onDeleteFile(bool)));

    ui->treeView->addAction(newFolderAction);
    ui->treeView->addAction(newFileAction);
    ui->treeView->addAction(delFileAction);
}

FileSystemDockWidget::~FileSystemDockWidget()
{
    disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    delete ui;
    ui = NULL;
}

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
    bool isFolder = true;
    bool isFile = true;

    foreach(QModelIndex index, indexList)
    {
        isFolder &= model->isDir(index);
        isFile &= !model->isDir(index);
    }

    RefreshAction(newFolderAction, isFolder, isFolder);
    RefreshAction(newFileAction  , isFolder, isFolder);
    RefreshAction(delFileAction  , isFolder || isFile, isFolder || isFile);
}

void FileSystemDockWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
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

void FileSystemDockWidget::onDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight )
{
    DAVA::Logger::Debug("model::dataChanged file name %s", model->fileName(topLeft).toStdString().c_str());
}

void FileSystemDockWidget::customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    QModelIndex currIndex = ui->treeView->currentIndex();
    DVASSERT(index == currIndex);

    QMenu *menu = new QMenu(this);

    if (model->isDir(index))
    {
        menu->addAction(newFolderAction);
        menu->addAction(newFileAction);
    }
    else
    {
        menu->addAction(delFileAction);
    }
    

    menu->exec(QCursor::pos());
}

void FileSystemDockWidget::onNewFolder()
{
    bool ok = false;
    QString folderName = QInputDialog::getText(this, tr("New folder"), tr("Folder name:"), QLineEdit::Normal, tr("New folder"), &ok);
    if (ok && !folderName.isEmpty())
    {
        QModelIndex currIndex = ui->treeView->currentIndex();
        model->mkdir(currIndex, folderName);
    }
}

void FileSystemDockWidget::onNewFile(bool checked)
{
    QModelIndex currIndex = ui->treeView->currentIndex();
    QString folderPath = model->filePath(currIndex);

    QString strFile = QFileDialog::getSaveFileName(this, tr("Create new file"), folderPath, "*.yaml");

    QFileInfo fileInfo(strFile);
    if (fileInfo.suffix().toLower() != "yaml")
    {
        strFile += ".yaml";
    }

    QFile file(strFile);
    file.open(QIODevice::WriteOnly);
    file.close();
}

void FileSystemDockWidget::onDeleteFile(bool checked)
{
    QModelIndex currIndex = ui->treeView->currentIndex();
    if (model->isDir(currIndex))
    {
        if (QMessageBox::Yes == QMessageBox::question(this, tr("Delete folder"), tr("Delete folder?"), QMessageBox::Yes | QMessageBox::No))
        {
            model->rmdir(currIndex);
        }
    }
    else
    {
        if (QMessageBox::Yes == QMessageBox::question(this, tr("Delete file"), tr("Delete file?"), QMessageBox::Yes | QMessageBox::No))
        {
            model->remove(currIndex);
        }
    }
    

    
}

void FileSystemDockWidget::onReloadFile(bool checked)
{

}

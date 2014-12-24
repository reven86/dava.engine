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
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(customContextMenuRequested(const QPoint &)));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu/*ActionsContextMenu*/);

    newFolderAction = new QAction(tr("Create folder"), this);
    connect(newFolderAction, SIGNAL(triggered(bool)), this, SLOT(onNewFolder(bool)));

    newFileAction = new QAction(tr("Create file"), this);
    connect(newFileAction, SIGNAL(triggered(bool)), this, SLOT(onNewFile(bool)));

    delFileAction = new QAction(tr("Delete"), this);
    connect(delFileAction, SIGNAL(triggered(bool)), this, SLOT(onDeleteFile(bool)));

    addAction(newFolderAction);
    addAction(newFileAction);
    addAction(delFileAction);
}

FileSystemDockWidget::~FileSystemDockWidget()
{
    delete ui;
    ui = NULL;
}

void FileSystemDockWidget::SetProjectDir(const QString &path)
{
    QDir dir(path);
    dir.cdUp();
    QString p = dir.path() + "/Data/UI";
    QModelIndex rootIndex = model->setRootPath(p);
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(rootIndex);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
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

    menu->addAction(delFileAction);

    menu->exec(QCursor::pos());
}

void FileSystemDockWidget::onNewFolder(bool checked)
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

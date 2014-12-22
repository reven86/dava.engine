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
    emit OpenPackageFile(model->filePath(index));
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

    QMenu *menu = new QMenu(this);
    menu->addAction(index.data(Qt::DisplayRole).toString());
    menu->exec(QCursor::pos());
}

#include "FindWidget.h"

#include <QStandardItem>

#include "Document/Document.h"
#include "Project/Project.h"
#include "UI/Find/Finder.h"
#include "QtTools/Utils/QtThread.h"
#include "QtTools/ProjectInformation/FileSystemCache.h"

#include "UI/UIControl.h"

using namespace DAVA;

FindWidget::FindWidget(QWidget* parent)
    : QDockWidget(parent)
{
    qRegisterMetaType<FindItem>("FindItem");

    ui.setupUi(this);

    model = new QStandardItemModel();
    ui.treeView->setModel(model);
    connect(ui.treeView, &QTreeView::activated, this, &FindWidget::OnActivated);
    ui.treeView->installEventFilter(this);
}

FindWidget::~FindWidget()
{
}

void FindWidget::Find(std::unique_ptr<FindFilter> filter)
{
    if (thread == nullptr)
    {
        model->removeRows(0, model->rowCount());
        setVisible(true);
        raise();

        if (project != nullptr)
        {
            thread = new QtThread;
            QStringList files = project->GetFileSystemCache()->GetFiles("yaml");
            Finder* finder = new Finder(files, std::move(filter), &(project->GetPrototypes()));
            finder->moveToThread(thread);

            connect(thread, &QtThread::started, finder, &Finder::Process);
            connect(thread, &QtThread::finished, this, &FindWidget::OnFindFinished);

            connect(finder, &Finder::Finished, thread, &QtThread::quit);
            connect(finder, &Finder::ItemFound, this, &FindWidget::OnItemFound, Qt::QueuedConnection);
            connect(finder, &Finder::ProgressChanged, this, &FindWidget::OnProgressChanged, Qt::QueuedConnection);

            connect(this, &FindWidget::StopAll, finder, &Finder::Cancel, Qt::DirectConnection);
            thread->start();
        }
    }
}

void FindWidget::OnProjectChanged(Project* project_)
{
    emit StopAll();
    project = project_;
    model->removeRows(0, model->rowCount());
}

void FindWidget::OnItemFound(FindItem item)
{
    String fwPath = item.GetFile().GetFrameworkPath();
    QStandardItem* pathItem = new QStandardItem(fwPath.c_str());
    pathItem->setEditable(false);
    pathItem->setData(QString::fromStdString(fwPath), PACKAGE_DATA);
    model->appendRow(pathItem);

    for (const String& pathToControl : item.GetControlPaths())
    {
        QStandardItem* controlItem = new QStandardItem(QString::fromStdString(pathToControl));
        controlItem->setEditable(false);
        controlItem->setData(QString::fromStdString(fwPath), PACKAGE_DATA);
        controlItem->setData(QString::fromStdString(pathToControl), CONTROL_DATA);
        pathItem->appendRow(controlItem);
    }
}

void FindWidget::OnProgressChanged(int filesProcessed, int totalFiles)
{
    this->setWindowTitle(QString("Find - %1%").arg(filesProcessed * 100 / totalFiles));
}

void FindWidget::OnFindFinished()
{
    delete thread;
    thread = nullptr;
    this->setWindowTitle(QString("Find - Finished"));
}

void FindWidget::OnActivated(const QModelIndex& index)
{
    if (project)
    {
        QString path = index.data(PACKAGE_DATA).toString();
        if (index.data(CONTROL_DATA).isValid())
        {
            QString control = index.data(CONTROL_DATA).toString();
            project->JumpToControl(FilePath(path.toStdString()), control.toStdString());
        }
        else
        {
            project->JumpToPackage(FilePath(path.toStdString()));
        }
    }
}

bool FindWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if (ui.treeView->selectionModel()->currentIndex().isValid())
            {
                OnActivated(ui.treeView->selectionModel()->currentIndex());
                return true;
            }
        }
    }

    return QDockWidget::eventFilter(obj, event);
}

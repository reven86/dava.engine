#include "FindWidget.h"

#include "Document/Document.h"
#include "Project/Project.h"
#include "UI/Find/Finder.h"

#include <QtTools/ProjectInformation/FileSystemCache.h>

#include <QtHelpers/HelperFunctions.h>

#include <UI/UIControl.h>

#include <QtConcurrent>
#include <QKeyEvent>

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

FindWidget::~FindWidget() = default;

void FindWidget::Find(std::unique_ptr<FindFilter>&& filter)
{
    totalResults = 0;
    totalFilesWithResults = 0;
    
    if (finder == nullptr)
    {
        model->removeRows(0, model->rowCount());
        setVisible(true);
        raise();

        if (project != nullptr)
        {
            QStringList files = project->GetFileSystemCache()->GetFiles("yaml");
            finder = new Finder(files, std::move(filter), &(project->GetPrototypes()));

            connect(finder, &Finder::ProgressChanged, this, &FindWidget::OnProgressChanged, Qt::QueuedConnection);
            connect(finder, &Finder::ItemFound, this, &FindWidget::OnItemFound, Qt::QueuedConnection);
            connect(finder, &Finder::Finished, this, &FindWidget::OnFindFinished, Qt::QueuedConnection);

            QtConcurrent::run(QtHelpers::InvokeInAutoreleasePool, [this]() { finder->Process(); });
        }
    }
}

void FindWidget::OnProjectChanged(Project* project_)
{
    if (finder)
    {
        finder->Stop();
    }

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

    totalFilesWithResults++;

    for (const String& pathToControl : item.GetControlPaths())
    {
        totalResults++;
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
    if (finder)
    {
        finder->deleteLater();
        finder = nullptr;
    }

    if (totalResults == 0)
    {
        this->setWindowTitle(QString("No results").arg(totalResults).arg(totalFilesWithResults));
    }
    else if (totalResults == 1)
    {
        this->setWindowTitle(QString("%1 result in %2 file").arg(totalResults).arg(totalFilesWithResults));
    }
    else if (totalFilesWithResults == 1)
    {
        this->setWindowTitle(QString("%1 results in %2 file").arg(totalResults).arg(totalFilesWithResults));
    }
    else
    {
        this->setWindowTitle(QString("%1 results in %2 files").arg(totalResults).arg(totalFilesWithResults));
    }
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

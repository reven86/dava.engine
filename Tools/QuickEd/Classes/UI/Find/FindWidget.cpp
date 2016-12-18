#include "FindWidget.h"

#include <QStandardItem>

#include "Document/Document.h"
#include "Project/Project.h"
#include "UI/Find/FindCollector.h"
#include "QtTools/Utils/QtThread.h"

#include "UI/UIControl.h"

using namespace DAVA;

FindWidget::FindWidget(QWidget* parent)
    : QDockWidget(parent)
{
    ui.setupUi(this);

    model = new QStandardItemModel();
    ui.treeView->setModel(model);
    connect(ui.treeView, &QTreeView::activated, this, &FindWidget::OnActivated);
    ui.treeView->installEventFilter(this);
}

void FindWidget::Find(std::unique_ptr<FindFilter> filter)
{
    if (project != nullptr)
    {
        QtThread* thread = new QtThread;
        FindCollector* collector = new FindCollector(project->GetFileSystemCache(), std::move(filter), &(project->GetPrototypes()));
        collector->moveToThread(thread);

        connect(thread, SIGNAL(started()), collector, SLOT(CollectFiles()));
        connect(collector, SIGNAL(finished()), thread, SLOT(quit()));
        thread->start();

        //findCollector.CollectFiles(project->GetFileSystemCache(), *filter.get(), project->GetPrototypes());
        //        ShowResults(collector->GetItems());
    }
}

void FindWidget::ShowResults(const DAVA::Vector<FindItem>& items_)
{
    model->removeRows(0, model->rowCount());
    items = items_;

    String prevFwPath;
    QStandardItem* pathItem = nullptr;
    for (const FindItem& item : items)
    {
        String fwPath = item.GetFile().GetFrameworkPath();
        if (pathItem == nullptr || prevFwPath != fwPath)
        {
            prevFwPath = fwPath;
            pathItem = new QStandardItem(fwPath.c_str());
            pathItem->setEditable(false);
            pathItem->setData(QString::fromStdString(fwPath), PACKAGE_DATA);
            model->appendRow(pathItem);
        }

        QStandardItem* controlItem = new QStandardItem(item.GetPathToControl().c_str());
        controlItem->setEditable(false);
        controlItem->setData(QString::fromStdString(fwPath), PACKAGE_DATA);
        controlItem->setData(QString::fromStdString(item.GetPathToControl()), CONTROL_DATA);
        pathItem->appendRow(controlItem);
        pathItem->setEditable(false);
    }

    setVisible(true);
    raise();
}

void FindWidget::OnDocumentChanged(Document* document)
{
    if (document != nullptr)
    {
        project = document->GetProject();
    }
    else
    {
        project = nullptr;
    }
}

void FindWidget::OnActivated(const QModelIndex& index)
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

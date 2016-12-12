#include "FindWidget.h"

#include <QStandardItem>

#include "Document/Document.h"
#include "Project/Project.h"
#include "UI/UIControl.h"

using namespace DAVA;

FindWidget::FindWidget(QWidget* parent)
    : QDockWidget(parent)
{
    ui.setupUi(this);

    model = new QStandardItemModel();
    ui.treeView->setModel(model);
    connect(ui.treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FindWidget::OnCurrentIndexChanged);
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
        controlItem->setData(item.IsInPrototypeSection(), IS_PROTO_DATA);
        pathItem->appendRow(controlItem);
        pathItem->setEditable(false);
        //        pathItem->
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

void FindWidget::OnCurrentIndexChanged(const QModelIndex& index, const QModelIndex&)
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

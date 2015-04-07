#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QAbstractItemModel>
#include "UI/SharedData.h"

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
}

void PropertiesWidget::OnDocumentChanged(SharedData *arg)
{
    sharedData = arg;
    UpdateActivatedControls();
}
void PropertiesWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "activatedControls")
    {
        UpdateActivatedControls();
    }
}

void PropertiesWidget::UpdateActivatedControls()
{
    QAbstractItemModel *prevModel = treeView->model();
    if (nullptr == sharedData)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        const QList<ControlNode*> &activatedControls = sharedData->GetData("activatedControls").value<QList<ControlNode*> >();
        QAbstractItemModel* model = activatedControls.empty() ? nullptr : new PropertiesModel(activatedControls.first(), sharedData->GetDocument());//TODO this is ugly
        sharedData->SetData("propertiesModel", QVariant::fromValue(model)); //TODO: bad architecture
        treeView->setModel(model);
        treeView->expandToDepth(0);
        treeView->resizeColumnToContents(0);
    }
    delete prevModel;
}
#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QAbstractItemModel>
#include "UI/WidgetContext.h"

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , widgetContext(nullptr)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
}

void PropertiesWidget::OnContextChanged(WidgetContext *arg)
{
    widgetContext = arg;
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
    if (nullptr == widgetContext)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        QList<ControlNode*> &activatedControls = widgetContext->GetData("activatedControls").value<QList<ControlNode*> >();
        QAbstractItemModel* model = activatedControls.empty() ? nullptr : new PropertiesModel(activatedControls.first(), widgetContext->GetDocument());//TODO this is ugly
        widgetContext->SetData(QVariant::fromValue(model), "propertiesModel");
        treeView->setModel(model);
        treeView->expandToDepth(0);
        treeView->resizeColumnToContents(0);
    }
    delete prevModel;
}
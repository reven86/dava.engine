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
    , ui(new Ui::PropertiesWidget())
    , widgetContext(nullptr)
{
    ui->setupUi(this);
    ui->treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
}

PropertiesWidget::~PropertiesWidget()
{
    delete ui;
} 

void PropertiesWidget::OnContextChanged(WidgetContext *arg)
{
    widgetContext = arg;
    UpdateModel();
}
void PropertiesWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "model")
    {
        UpdateModel();
    }
}

void PropertiesWidget::UpdateModel()
{
    if (nullptr == widgetContext)
    {
        ui->treeView->setModel(nullptr);
    }
    else
    {
        QAbstractItemModel *model = widgetContext->GetData("model").value<QAbstractItemModel*>();
        QAbstractItemModel *prevModel = ui->treeView->model();
        ui->treeView->setModel(model);
        delete prevModel;//TODO - this is ugly :(
        ui->treeView->expandToDepth(0);
        ui->treeView->resizeColumnToContents(0);
    }
}

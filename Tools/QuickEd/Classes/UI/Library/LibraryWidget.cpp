#include "LibraryWidget.h"
#include "ui_LibraryWidget.h"
#include "UI/WidgetContext.h"
#include <QAbstractItemModel>

LibraryWidget::LibraryWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LibraryWidget())
    , widgetContext(nullptr)
{
    ui->setupUi(this);
}
LibraryWidget::~LibraryWidget()
{
    delete ui;
}

void LibraryWidget::OnContextChanged(WidgetContext *arg)
{
    widgetContext = arg;
    UpdateModel();
}

void LibraryWidget::OnDataChanged(const QString &role)
{
    if (role == "model")
    {
        UpdateModel();
    }
}

void LibraryWidget::UpdateModel()
{
    QAbstractItemModel *model = widgetContext->GetData<QAbstractItemModel*>("model");
    if (nullptr != model)
    {
        ui->treeView->setModel(model);
    }
}

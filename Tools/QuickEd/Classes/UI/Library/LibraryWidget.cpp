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

void LibraryWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "model")
    {
        UpdateModel();
    }
}

void LibraryWidget::UpdateModel()
{
    if (nullptr == widgetContext)
    {
        ui->treeView->setModel(nullptr);
    }
    else
    {
        QAbstractItemModel *model = widgetContext->GetData("model").value<QAbstractItemModel*>();
        ui->treeView->setModel(model);
    }
}

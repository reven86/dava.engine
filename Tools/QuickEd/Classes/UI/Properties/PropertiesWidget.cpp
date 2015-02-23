#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Document.h"
#include "UI/PropertiesContext.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"
#include "Model/PackageHierarchy/ControlNode.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PropertiesWidget())
    , context(nullptr)
{
    ui->setupUi(this);
    ui->treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
}

PropertiesWidget::~PropertiesWidget()
{
    delete ui;
}

void PropertiesWidget::SetDocument(Document *document)
{
    if (nullptr != context) //remove previous context
    {
        disconnect(context, SIGNAL(ModelChanged(PropertiesModel*)), this, SLOT(OnModelChanged(PropertiesModel*)));
        ui->treeView->setModel(nullptr);
    }
    /*set new context*/
    if (nullptr == document)
    {
        context = nullptr;
    }
    else
    {
        context = document->GetPropertiesContext();
    }

    if (nullptr != context)
    {
        connect(context, SIGNAL(ModelChanged(PropertiesModel*)), this, SLOT(OnModelChanged(PropertiesModel*)));
        ui->treeView->setModel(context->GetModel());
    }
}

void PropertiesWidget::OnModelChanged(PropertiesModel *model)
{
    ui->treeView->setModel(model);
    if (nullptr != model)
    {
        ui->treeView->expandToDepth(0);
        ui->treeView->resizeColumnToContents(0);
    }
}

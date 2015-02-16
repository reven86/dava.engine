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
    , context(NULL)
{
    ui->setupUi(this);
    ui->treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
}

PropertiesWidget::~PropertiesWidget()
{
    delete ui;
    ui = NULL;
}

void PropertiesWidget::SetContext(PropertiesContext *newContext)
{
    if (context)
    {
        disconnect(context, SIGNAL(ModelChanged(PropertiesModel*)), this, SLOT(OnModelChanged(PropertiesModel*)));
        ui->treeView->setModel(nullptr);
    }

    context = newContext;

    if (context)
    {
        connect(context, SIGNAL(ModelChanged(PropertiesModel*)), this, SLOT(OnModelChanged(PropertiesModel*)));
        ui->treeView->setModel(context->GetModel());
    }
}

void PropertiesWidget::OnModelChanged(PropertiesModel *model)
{
    ui->treeView->setModel(model);
    
    if (model)
    {
        ui->treeView->expandToDepth(0);
        ui->treeView->resizeColumnToContents(0);
    }
}

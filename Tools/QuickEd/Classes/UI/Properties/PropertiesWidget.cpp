#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QMenu>

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Document.h"
#include "UI/PropertiesContext.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "UI/Components/UIComponent.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PropertiesWidget())
    , context(nullptr)
{
    ui->setupUi(this);
    ui->treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
    
    QMenu *test = new QMenu("Menu", this);
    for (int32 i = 0; i < UIComponent::COMPONENT_COUNT; i++)
    {
        const char *name = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(i);
        QAction *componentAction = new QAction(tr(name), this);
        componentAction->setData(i);
        test->addAction(componentAction);
        connect(test, SIGNAL(triggered(QAction *)), this, SLOT(OnAddComponent(QAction *)));
    }

    addComponentAction = new QAction(tr("Add Component"), this);
    addComponentAction->setMenu(test);
    ui->treeView->addAction(addComponentAction);

//    ui->treeView->addAction(test);//
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

void PropertiesWidget::OnAddComponent(QAction *action)
{
    uint32 componentType = action->data().toUInt();
    DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
    
    if (context)
    {
        context->GetDocument()->GetCommandExecutor();
        
    }
}

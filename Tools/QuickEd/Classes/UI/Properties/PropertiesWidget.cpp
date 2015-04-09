#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QMenu>
#include <QItemSelection>

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Document.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include "UI/PropertiesContext.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "UI/Components/UIComponent.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PropertiesWidget())
    , context(nullptr)
    , addComponentAction(nullptr)
    , removeComponentAction(nullptr)
{
    ui->setupUi(this);
    ui->treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
    
    QMenu *addComponentMenu = new QMenu("Menu", this);
    for (int32 i = 0; i < UIComponent::COMPONENT_COUNT; i++)
    {
        const char *name = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(i);
        QAction *componentAction = new QAction(tr(name), this);
        componentAction->setData(i);
        addComponentMenu->addAction(componentAction);
        connect(addComponentMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddComponent);
    }

    addComponentAction = new QAction(tr("Add Component"), this);
    addComponentAction->setMenu(addComponentMenu);
    
    removeComponentAction = new QAction(tr("Remove Component"), this);
    connect(removeComponentAction, &QAction::triggered, this, &PropertiesWidget::OnRemoveComponent);
    
    ui->treeView->addAction(addComponentAction);
    ui->treeView->addAction(removeComponentAction);
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
        connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
    }
}

void PropertiesWidget::OnModelChanged(PropertiesModel *model)
{
    ui->treeView->setModel(model);
    if (nullptr != model)
    {
        ui->treeView->expandToDepth(0);
        ui->treeView->resizeColumnToContents(0);
        connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
        addComponentAction->setEnabled(!model->GetControlNode()->GetPropertiesRoot()->IsReadOnly());
        removeComponentAction->setEnabled(false);
    }
}

void PropertiesWidget::OnAddComponent(QAction *action)
{
    uint32 componentType = action->data().toUInt();
    DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
    
    uint32 componentIndex = 0; // TODO fix
    
    if (context)
    {
        context->GetDocument()->GetCommandExecutor()->AddComponent(context->GetModel()->GetControlNode(), componentType, componentIndex);
        
    }
}

void PropertiesWidget::OnRemoveComponent()
{
    if (context)
    {
        //context->GetDocument()->GetCommandExecutor()->RemoveComponent(context->GetModel()->GetControlNode(), componentType);
        
    }
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (!selected.empty())
    {
        const QItemSelectionRange &range = selected.first();
        const QPersistentModelIndex &index = range.topLeft();
        BaseProperty *property = static_cast<BaseProperty*>(index.internalPointer());
        bool enabled = false;
        enabled = property->CanRemove() && context && context->GetModel()->GetControlNode()->IsEditingSupported();
        removeComponentAction->setEnabled(enabled);
    }
}

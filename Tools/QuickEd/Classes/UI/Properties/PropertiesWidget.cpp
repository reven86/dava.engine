#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QMenu>
#include <QItemSelection>

#include "UI/QtModelPackageCommandExecutor.h"

#include <QAbstractItemModel>
#include "SharedData.h"

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

#include "UI/Components/UIComponent.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , addComponentAction(nullptr)
    , removeComponentAction(nullptr)
    , sharedData(nullptr)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
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
    
    treeView->addAction(addComponentAction);
    treeView->addAction(removeComponentAction);
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
        QAbstractItemModel* model = activatedControls.empty() ? nullptr : new PropertiesModel(activatedControls.first(), sharedData->GetDocument()->GetCommandExecutor());//TODO this is ugly
        sharedData->SetData("propertiesModel", QVariant::fromValue(model)); //TODO: bad architecture
        treeView->setModel(model);
        treeView->expandToDepth(0);
        treeView->resizeColumnToContents(0);
    }
    delete prevModel;
}

void PropertiesWidget::OnAddComponent(QAction *action)
{
//    if (context)
//    {
//        uint32 componentType = action->data().toUInt();
//        DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
//        
//        context->GetDocument()->GetCommandExecutor()->AddComponent(context->GetModel()->GetControlNode(), componentType);
//    }
}

void PropertiesWidget::OnRemoveComponent()
{
//    if (context)
//    {
//        //context->GetDocument()->GetCommandExecutor()->RemoveComponent(context->GetModel()->GetControlNode(), componentType);
//        
//    }
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
//    if (!selected.empty())
//    {
//        const QItemSelectionRange &range = selected.first();
//        const QPersistentModelIndex &index = range.topLeft();
//        BaseProperty *property = static_cast<BaseProperty*>(index.internalPointer());
//        bool enabled = false;
//        enabled = property->CanRemove() && context && context->GetModel()->GetControlNode()->IsEditingSupported();
//        removeComponentAction->setEnabled(enabled);
//    }
}

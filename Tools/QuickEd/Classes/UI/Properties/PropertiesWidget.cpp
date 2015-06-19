/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QMenu>
#include <QItemSelection>

#include "UI/QtModelPackageCommandExecutor.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"

#include <QAbstractItemModel>
#include "SharedData.h"

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

#include "UI/Components/UIComponent.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
    , addComponentAction(nullptr)
    , removeComponentAction(nullptr)
    , selectedComponentType(-1)
    , selectedComponentIndex(-1)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
    
    QMenu *addComponentMenu = new QMenu(this);
    for (int32 i = 0; i < UIComponent::COMPONENT_COUNT; i++)
    {
        const char *name = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(i);
        QAction *componentAction = new QAction(name, this); // TODO: Localize name
        componentAction->setData(i);
        addComponentMenu->addAction(componentAction);
    }
    connect(addComponentMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddComponent);

    addComponentAction = new QAction(tr("Add Component"), this);
    addComponentAction->setEnabled(false);
    addComponentAction->setMenu(addComponentMenu);
    
    removeComponentAction = new QAction(tr("Remove Component"), this);
    removeComponentAction->setEnabled(false);
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

void PropertiesWidget::OnAddComponent(QAction *action)
{
    if (sharedData)
    {
        uint32 componentType = action->data().toUInt();
        if (componentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            sharedData->GetDocument()->GetCommandExecutor()->AddComponent(node, componentType);
        }
        else
        {
            DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
        }
    }
}

void PropertiesWidget::OnRemoveComponent()
{
    if (sharedData)
    {
        if (0 <= selectedComponentType && selectedComponentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            sharedData->GetDocument()->GetCommandExecutor()->RemoveComponent(node, selectedComponentType, selectedComponentIndex);
        }
        else
        {
            DVASSERT(false);
        }
    }
    UpdateActions();
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    UpdateActions();
}

ControlNode *PropertiesWidget::GetSelectedControlNode() const
{
    if (!sharedData)
        return nullptr;
    
    const QList<ControlNode*> &activatedControls = sharedData->GetData("activatedControls").value<QList<ControlNode*> >();
    if (activatedControls.empty())
        return nullptr;
    
    return activatedControls.first();
    
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
        ControlNode *node = GetSelectedControlNode();
        if (node)
            treeView->setModel(new PropertiesModel(node, sharedData->GetDocument()->GetCommandExecutor())); //TODO this is ugly // -- why?
        else
            treeView->setModel(nullptr);
        
        treeView->expandToDepth(0);
        treeView->resizeColumnToContents(0);
    }
    
    addComponentAction->setEnabled(treeView->model() != nullptr);
    removeComponentAction->setEnabled(false);
    
    if (treeView->model() != nullptr)
    {
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
    }
    
    delete prevModel;
}

void PropertiesWidget::UpdateActions()
{
    QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
    if (!indices.empty())
    {
        const QModelIndex &index = indices.first();
        AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
        
        bool enabled = (property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0;
        
        if (enabled)
        {
            ComponentPropertiesSection *section = dynamic_cast<ComponentPropertiesSection*>(property);
            if (section)
            {
                selectedComponentType = static_cast<int>(section->GetComponentType());
                selectedComponentIndex = static_cast<int>(section->GetComponentIndex());
            }
            else
            {
                enabled = false;
            }
        }
        removeComponentAction->setEnabled(enabled);
    }
}

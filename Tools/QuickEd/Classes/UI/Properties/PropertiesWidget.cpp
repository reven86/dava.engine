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
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"

#include <QAbstractItemModel>

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

#include "Document.h"
#include "UI/Components/UIComponent.h"
#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));

    addComponentAction = CreateAddComponentAction();
    treeView->addAction(addComponentAction);

    addStylePropertyAction = CreateAddStylePropertyAction();
    treeView->addAction(addStylePropertyAction);

    addStyleSelectorAction = CreateAddStyleSelectorAction();
    treeView->addAction(addStyleSelectorAction);
    
    treeView->addAction(CreateSeparator());
    
    removeAction = CreateRemoveAction();
    treeView->addAction(removeAction);
}

void PropertiesWidget::OnDocumentChanged(Document* arg)
{
    document = arg;
}

void PropertiesWidget::SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
    UpdateSelection();
}

void PropertiesWidget::OnAddComponent(QAction *action)
{
    if (nullptr != document)
    {
        uint32 componentType = action->data().toUInt();
        if (componentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            document->GetCommandExecutor()->AddComponent(node, componentType);
        }
        else
        {
            DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
        }
    }
}

void PropertiesWidget::OnRemove()
{
    if (nullptr != document)
    {
        QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
        if (!indices.empty())
        {
            const QModelIndex &index = indices.first();
            AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
            
            if ((property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0)
            {
                ComponentPropertiesSection *section = dynamic_cast<ComponentPropertiesSection*>(property);
                if (section)
                {
                    ControlNode *node = GetSelectedControlNode();
                    document->GetCommandExecutor()->RemoveComponent(node, section->GetComponentType(), section->GetComponentIndex());
                }
                else
                {
                    StyleSheetProperty *styleProperty = dynamic_cast<StyleSheetProperty*>(property);
                    if (styleProperty)
                    {
                        StyleSheetNode *node = GetSelectedStyleSheetNode();
                        document->GetCommandExecutor()->RemoveStyleProperty(node, styleProperty->GetPropertyIndex());
                    }
                    else
                    {
                        StyleSheetSelectorProperty *selectorProperty = dynamic_cast<StyleSheetSelectorProperty*>(property);
                        if (selectorProperty)
                        {
                            int32 index = property->GetParent()->GetIndex(selectorProperty);
                            if (index != -1)
                                document->GetCommandExecutor()->RemoveStyleSelector(GetSelectedStyleSheetNode(), index);
                        }
                    }
                }
            }
        }
    }
    UpdateActions();
}

void PropertiesWidget::OnAddStyleProperty(QAction *action)
{
    if (nullptr != document)
    {
        uint32 propertyIndex = action->data().toUInt();
        if (propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT)
        {
            StyleSheetNode *node = GetSelectedStyleSheetNode();
            document->GetCommandExecutor()->AddStyleProperty(node, propertyIndex);
        }
        else
        {
            DVASSERT(propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT);
        }
    }
    
}

void PropertiesWidget::OnAddStyleSelector()
{
    if (nullptr != document)
    {
        document->GetCommandExecutor()->AddStyleSelector(GetSelectedStyleSheetNode());
    }
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    UpdateActions();
}

QAction *PropertiesWidget::CreateAddComponentAction()
{
    QMenu *addComponentMenu = new QMenu(this);
    for (int32 i = 0; i < UIComponent::COMPONENT_COUNT; i++)
    {
        const char *name = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(i);
        QAction *componentAction = new QAction(name, this); // TODO: Localize name
        componentAction->setData(i);
        addComponentMenu->addAction(componentAction);
    }
    connect(addComponentMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddComponent);

    QAction *action = new QAction(tr("Add Component"), this);
    action->setEnabled(false);
    action->setMenu(addComponentMenu);
    
    return action;
}

QAction *PropertiesWidget::CreateAddStyleSelectorAction()
{
    QAction *action = new QAction(tr("Add Style Selector"), this);
    action->setEnabled(false);
    connect(action, &QAction::triggered, this, &PropertiesWidget::OnAddStyleSelector);
    return action;
}

QAction *PropertiesWidget::CreateAddStylePropertyAction()
{
    QMenu *propertiesMenu = new QMenu(this);
    UIStyleSheetPropertyDataBase *db = UIStyleSheetPropertyDataBase::Instance();
    for (int32 i = 0; i < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT; i++)
    {
        const UIStyleSheetPropertyDescriptor &descr = db->GetStyleSheetPropertyByIndex(i);
        QAction *componentAction = new QAction(descr.name.c_str(), this);
        componentAction->setData(i);
        propertiesMenu->addAction(componentAction);
    }
    connect(propertiesMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddStyleProperty);

    QAction *action = new QAction(tr("Add Style Property"), this);
    action->setEnabled(false);
    action->setMenu(propertiesMenu);
    
    return action;
}

QAction *PropertiesWidget::CreateRemoveAction()
{
    QAction *action = new QAction(tr("Remove"), this);
    action->setEnabled(false);
    connect(action, &QAction::triggered, this, &PropertiesWidget::OnRemove);
    return action;
}

QAction *PropertiesWidget::CreateSeparator()
{
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    return separator;
}

void PropertiesWidget::OnModelChanged()
{
    treeView->expandToDepth(0);
    treeView->resizeColumnToContents(0);
}

ControlNode *PropertiesWidget::GetSelectedControlNode() const
{
    for (const auto& node : selectionContainer.selectedNodes)
    {
        ControlNode *control = dynamic_cast<ControlNode*>(node);
        if (nullptr != control)
        {
            return control;
        }
    }
    return nullptr;
}

StyleSheetNode *PropertiesWidget::GetSelectedStyleSheetNode() const
{
    for (PackageBaseNode* node : selectionContainer.selectedNodes)
    {
        StyleSheetNode *styleSheet = dynamic_cast<StyleSheetNode*>(node);
        if (nullptr != styleSheet)
        {
            return styleSheet;
        }
    }
    return nullptr;
}

void PropertiesWidget::UpdateSelection()
{
    QAbstractItemModel *prevModel = treeView->model();
    ControlNode *control = nullptr;
    StyleSheetNode *styleSheet = nullptr;
    if (nullptr == document)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        for (PackageBaseNode* node : selectionContainer.selectedNodes)
        {
            control = dynamic_cast<ControlNode*>(node);
            if (nullptr != control)
            {
                treeView->setModel(new PropertiesModel(control, document->GetCommandExecutor()));
                break;
            }
            else
            {
                styleSheet = dynamic_cast<StyleSheetNode*>(node);
                if (nullptr != styleSheet)
                {
                    treeView->setModel(new PropertiesModel(styleSheet, document->GetCommandExecutor()));
                    break;
                }
                else
                {
                    treeView->setModel(nullptr);
                }
            }
        }
    }
    
    addComponentAction->setEnabled(control != nullptr);
    addStylePropertyAction->setEnabled(styleSheet != nullptr);
    addStyleSelectorAction->setEnabled(styleSheet != nullptr);

    removeAction->setEnabled(false);
    
    if (treeView->model() != nullptr)
    {
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
    }
    //delay long time work with view
    QMetaObject::invokeMethod(this, "OnModelChanged", Qt::QueuedConnection);
    delete prevModel;
}

void PropertiesWidget::UpdateActions()
{
    QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
    if (!indices.empty())
    {
        AbstractProperty *property = static_cast<AbstractProperty*>(indices.first().internalPointer());
        removeAction->setEnabled((property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0);
    }
}

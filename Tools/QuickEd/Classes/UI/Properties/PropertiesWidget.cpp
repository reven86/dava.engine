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

namespace
{
String GetPathFromIndex(QModelIndex index)
{
    QString path = index.data().toString();
    while (index.parent().isValid())
    {
        index = index.parent();
        path += "/" + index.data().toString();
    }
    return path.toStdString();
}
}

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);
    propertiesModel = new PropertiesModel(treeView);
    treeView->setModel(propertiesModel);
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);

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

    connect(treeView, &QTreeView::expanded, this, &PropertiesWidget::OnExpanded);
    connect(treeView, &QTreeView::collapsed, this, &PropertiesWidget::OnCollapsed);
    DVASSERT(nullptr == selectedNode);
    UpdateModel(nullptr);
}

void PropertiesWidget::OnDocumentChanged(Document* document)
{
    if (nullptr != document)
    {
        commandExecutor = document->GetCommandExecutor();
    }
    else
    {
        commandExecutor.reset();
    }
    UpdateModel(nullptr); //SelectionChanged will invoke by Queued Connection, so selectedNode have invalid value
}

void PropertiesWidget::OnAddComponent(QAction *action)
{
    auto commandExecutorPtr = commandExecutor.lock();
    DVASSERT(nullptr != commandExecutorPtr);
    if (nullptr != commandExecutorPtr)
    {
        uint32 componentType = action->data().toUInt();
        if (componentType < UIComponent::COMPONENT_COUNT)
        {
            commandExecutorPtr->AddComponent(DynamicTypeCheck<ControlNode*>(selectedNode), componentType);
        }
        else
        {
            DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
        }
    }
}

void PropertiesWidget::OnRemove()
{
    auto commandExecutorPtr = commandExecutor.lock();
    DVASSERT(nullptr != commandExecutorPtr);
    if (nullptr != commandExecutorPtr)
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
                    commandExecutorPtr->RemoveComponent(DynamicTypeCheck<ControlNode*>(selectedNode), section->GetComponentType(), section->GetComponentIndex());
                }
                else
                {
                    StyleSheetProperty *styleProperty = dynamic_cast<StyleSheetProperty*>(property);
                    if (styleProperty)
                    {
                        commandExecutorPtr->RemoveStyleProperty(DynamicTypeCheck<StyleSheetNode*>(selectedNode), styleProperty->GetPropertyIndex());
                    }
                    else
                    {
                        StyleSheetSelectorProperty *selectorProperty = dynamic_cast<StyleSheetSelectorProperty*>(property);
                        if (selectorProperty)
                        {
                            int32 index = property->GetParent()->GetIndex(selectorProperty);
                            if (index != -1)
                            {
                                commandExecutorPtr->RemoveStyleSelector(DynamicTypeCheck<StyleSheetNode*>(selectedNode), index);
                            }
                        }
                    }
                }
            }
        }
        UpdateActions();
    }
}

void PropertiesWidget::OnAddStyleProperty(QAction *action)
{
    auto commandExecutorPtr = commandExecutor.lock();
    DVASSERT(nullptr != commandExecutorPtr);
    if (nullptr != commandExecutorPtr)
    {
        uint32 propertyIndex = action->data().toUInt();
        if (propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT)
        {
            commandExecutorPtr->AddStyleProperty(DynamicTypeCheck<StyleSheetNode*>(selectedNode), propertyIndex);
        }
        else
        {
            DVASSERT(propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT);
        }
    }
    
}

void PropertiesWidget::OnAddStyleSelector()
{
    auto commandExecutorPtr = commandExecutor.lock();
    DVASSERT(nullptr != commandExecutorPtr);
    if (nullptr != commandExecutorPtr)
    {
        commandExecutorPtr->AddStyleSelector(DynamicTypeCheck<StyleSheetNode*>(selectedNode));
    }
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
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
    action->setMenu(addComponentMenu);
    addComponentMenu->setEnabled(false);
    return action;
}

QAction *PropertiesWidget::CreateAddStyleSelectorAction()
{
    QAction *action = new QAction(tr("Add Style Selector"), this);
    connect(action, &QAction::triggered, this, &PropertiesWidget::OnAddStyleSelector);
    action->setEnabled(false);
    return action;
}

QAction *PropertiesWidget::CreateAddStylePropertyAction()
{
    QMenu *propertiesMenu = new QMenu(this);
    QMenu* groupMenu = nullptr;
    UIStyleSheetPropertyGroup* prevGroup = nullptr;
    UIStyleSheetPropertyDataBase *db = UIStyleSheetPropertyDataBase::Instance();
    for (int32 i = 0; i < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT; i++)
    {
        const UIStyleSheetPropertyDescriptor &descr = db->GetStyleSheetPropertyByIndex(i);
        if (descr.group != prevGroup)
        {
            prevGroup = descr.group;
            if (descr.group->prefix.empty())
            {
                groupMenu = propertiesMenu;
            }
            else
            {
                groupMenu = new QMenu(QString::fromStdString(descr.group->prefix), this);
                propertiesMenu->addMenu(groupMenu);
            }
        }
        QAction *componentAction = new QAction(descr.name.c_str(), this);
        componentAction->setData(i);

        groupMenu->addAction(componentAction);
    }
    connect(propertiesMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddStyleProperty);

    QAction *action = new QAction(tr("Add Style Property"), this);
    action->setMenu(propertiesMenu);
    propertiesMenu->setEnabled(false);
    return action;
}

QAction *PropertiesWidget::CreateRemoveAction()
{
    QAction *action = new QAction(tr("Remove"), this);
    connect(action, &QAction::triggered, this, &PropertiesWidget::OnRemove);
    action->setEnabled(false);
    return action;
}

QAction *PropertiesWidget::CreateSeparator()
{
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    return separator;
}

void PropertiesWidget::OnModelUpdated()
{
    bool blocked = treeView->blockSignals(true);
    treeView->expandToDepth(0);
    treeView->blockSignals(blocked);
    treeView->resizeColumnToContents(0);
    ApplyExpanding();
}

void PropertiesWidget::OnExpanded(const QModelIndex& index)
{
    itemsState[GetPathFromIndex(index)] = true;
}

void PropertiesWidget::OnCollapsed(const QModelIndex& index)
{
    itemsState[GetPathFromIndex(index)] = false;
}

void PropertiesWidget::UpdateModel(PackageBaseNode* node)
{
    if (node == selectedNode)
    {
        return;
    }
    if (nullptr != selectedNode)
    {
        auto index = treeView->indexAt(QPoint(0, 0));
        lastTopIndexPath = GetPathFromIndex(index);
    }
    selectedNode = node;
    propertiesModel->Reset(selectedNode, commandExecutor);
    bool isControl = dynamic_cast<ControlNode*>(selectedNode) != nullptr;
    bool isStyle = dynamic_cast<StyleSheetNode*>(selectedNode) != nullptr;
    addComponentAction->menu()->setEnabled(isControl);
    addStylePropertyAction->menu()->setEnabled(isStyle);
    addStyleSelectorAction->setEnabled(isStyle);
    removeAction->setEnabled(false);
    
    //delay long time work with view
    QMetaObject::invokeMethod(this, "OnModelUpdated", Qt::QueuedConnection);
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

void PropertiesWidget::ApplyExpanding()
{
    QModelIndex index = propertiesModel->index(0, 0);
    while (index.isValid())
    {
        const auto& path = GetPathFromIndex(index);
        if (path == lastTopIndexPath)
        {
            treeView->scrollTo(index, QTreeView::PositionAtTop);
        }
        auto iter = itemsState.find(path);
        if (iter != itemsState.end())
        {
            treeView->setExpanded(index, iter->second);
        }

        index = treeView->indexBelow(index);
    }
}

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



#include "SceneGraphModel.h"
#include "SceneGraphItem.h"

#include "Qt/Scene/SceneData.h"
#include "Qt/Scene/SceneDataManager.h"
#include "../EditorScene.h"

#include "GraphItem.h"
#include "SceneGraphModelStateHelper.h"

#include "DockSceneGraph/PointerHolder.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/ArrowsNode.h"

#include "DockParticleEditor/ParticlesEditorController.h"

#include <QTreeView>
#include <QPainter>

using namespace DAVA;

SceneGraphModel::SceneGraphModel(QObject *parent)
    :   GraphModel(parent)
    ,   scene(NULL)
    ,   selectedNode(NULL)
    ,   selectedGraphItemForParticleEditor(NULL)
{
	InitDecorationIcons();
}

SceneGraphModel::~SceneGraphModel()
{
    SafeRelease(scene);
}

void SceneGraphModel::SetScene(EditorScene *newScene)
{
    SafeRelease(scene);
    scene = SafeRetain(newScene);
 
	// If the scene is changed, the selection on the Particles Editor level needs to be reset.
	particlesEditorSceneModelHelper.ResetSelection();
    Rebuild();
}

bool SceneGraphModel::IsNodeAccepted(DAVA::Entity *node)
{
	//Check whether current node is ArrowsNode.
	//ArrowsNode should not be displayed in SceneGraph tree
	ArrowsNode* arrowsNode = dynamic_cast<ArrowsNode*>(node);
	if (arrowsNode)
		return false;

	return true;
}

void SceneGraphModel::AddNodeToTree(GraphItem *parent, DAVA::Entity *node, bool partialUpdate)
{
	if (!IsNodeAccepted(node))
	{
		return;
	}
    // Particles Editor can change the node type during "adopting" Particle Emitter Nodes.
    node = particlesEditorSceneModelHelper.PreprocessSceneNode(node);

	if (partialUpdate)
	{
		QModelIndex parentIndex = createIndex(parent->Row(), 0, parent);
		int32 rowToAdd = parent->Row();
		
		beginInsertRows(parentIndex, rowToAdd, rowToAdd);
	}

    SceneGraphItem *graphItem = new SceneGraphItem();
	graphItem->SetUserData(node);
    parent->AppendChild(graphItem);
    
    // Particles Editor nodes are processed separately.
	bool nodeAddedByParticleEditor = false;
    if (particlesEditorSceneModelHelper.AddNodeToSceneGraph(graphItem, node))
    {
        // Also try to determine selected item from Particle Editor.
        SceneGraphItem *selectedItem = particlesEditorSceneModelHelper.GetGraphItemToBeSelected(graphItem, node);
        if (selectedItem)
        {
            this->selectedGraphItemForParticleEditor = selectedItem;
        }

        nodeAddedByParticleEditor = true;
    }

	if (partialUpdate)
	{
		endInsertRows();
	}
	
	if (nodeAddedByParticleEditor)
	{
		return;
	}

    if(!node->GetSolid())
    {
		// AddNodeToTree() can change the children while Particles Editor adopts orphaned Particle Emitter nodes.
		// Need to store the original pointers list to the current node's children to process them corrrectly.
		Vector<DAVA::Entity*> originalNodes;
        int32 count = node->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
			originalNodes.push_back(node->GetChild(i));
        }

		// Now look through the nodes remembered..
        for(int32 i = 0; i < count; ++i)
        {
            AddNodeToTree(graphItem, originalNodes[i], partialUpdate);
			// Note - after this call originalNodes[i] may become invalid!
        }
    }
}

void SceneGraphModel::AddNodeToTree(GraphItem* parentItem, GraphItem* childItem)
{
	if (!parentItem || !childItem)
	{
		return;
	}
	
	QModelIndex parentIndex = createIndex(parentItem->Row(), 0, parentItem);
	int32 rowToAdd = parentItem->Row();
	beginInsertRows(parentIndex, rowToAdd, rowToAdd);

	parentItem->AppendChild(childItem);

	endInsertRows();
}

void SceneGraphModel::RemoveNodeFromTree(GraphItem* parentItem, GraphItem* childItem)
{
	if (!parentItem || !childItem)
	{
		return;
	}
	
	QModelIndex parentIndex = createIndex(parentItem->Row(), 0, parentItem);
	int32 rowToRemove = childItem->Row();
	
	// Remove the previous children and add new ones.
	beginRemoveRows(parentIndex, rowToRemove, rowToRemove);
	parentItem->RemoveChild(childItem);
	endRemoveRows();
}

void SceneGraphModel::RebuildNode(DAVA::Entity* rootNode)
{
	if(!rootNode)
	{
		return;
	}

	// Lookup for SceneItem for this particular node.
	GraphItem* graphItem = ItemForData(rootNode);
	if (!graphItem)
	{
		return;
	}

	// Do the rebuild itself. Remove the previous children and add new ones.
	// The separate vector is needed to don't corrupt GraphItem iterator while removing items.
	Vector<GraphItem*> childrenToRemove;
	int32 childrenToRemoveCount = graphItem->ChildrenCount();
	for (int32 i = 0; i < childrenToRemoveCount; i ++)
	{
		childrenToRemove.push_back(graphItem->Child(i));
	}
	
	for (int32 i = 0; i < childrenToRemoveCount; i ++)
	{
		GraphItem* childItem = childrenToRemove[i];
		RemoveNodeFromTree(graphItem, childItem);
	}

	// Add the child nodes only if the root node is not solid.
	if (!rootNode->GetSolid())
	{
		int32 childrenToAddCount = rootNode->GetChildrenCount();
		for (int32 i = 0; i < childrenToAddCount; i ++)
		{
			AddNodeToTree(graphItem, rootNode->GetChild(i), true);
		}
	}
}


void SceneGraphModel::Rebuild()
{
	SceneGraphModelStateHelper stateHelper(this->attachedTreeView, this);
	stateHelper.SaveTreeViewState();

    SafeRelease(rootItem);
	rootItem = new SceneGraphItem();
	rootItem->SetUserData(scene);
    
    this->selectedGraphItemForParticleEditor = NULL;
    
    if(scene && !scene->GetSolid())
    {
        int32 count = scene->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
            AddNodeToTree(rootItem, scene->GetChild(i));
        }
    }
    
    this->reset();

	stateHelper.RestoreTreeViewState();
	
    if (HandleParticleEditorSelection())
    {
        // Custom node is selected and needs to be processed separately.
        return;
    }
    
    if(selectedNode)
    {
        GraphItem *selectedItem = ItemForData(selectedNode);
        if(selectedItem)
        {
            SelectItem(selectedItem);
        }
        else
        {
            SelectNode(NULL);
        }
    }
}

bool SceneGraphModel::HandleParticleEditorSelection()
{
    if (this->selectedGraphItemForParticleEditor == NULL)
    {
        return false;
    }

    SelectItem(this->selectedGraphItemForParticleEditor, true);
    return true;
}

void SceneGraphModel::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(LandscapeEditorModeEnabled())
        return;
    
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();
    
    DVASSERT((selectedSize <= 1) && "Wrong count of selected items");
    DVASSERT((deselectedSize <= 1) && "Wrong count of deselected items");

    // Particles Editor Nodes are handled separately.
    if (particlesEditorSceneModelHelper.ProcessSelectionChanged(selected, deselected))
    {
        return;
    }

    if(0 < selectedSize)
    {
        QItemSelectionRange selectedRange = selected.at(0);
        SceneGraphItem *selectedItem = static_cast<SceneGraphItem*>(selectedRange.topLeft().internalPointer());
        Entity *newSelectedNode = static_cast<Entity *>(selectedItem->GetUserData());

        if(0 < deselectedSize)
        {
            QItemSelectionRange deselectedRange = deselected.at(0);
            SceneGraphItem *deselectedItem = static_cast<SceneGraphItem*>(deselectedRange.topLeft().internalPointer());
            if(deselectedItem->GetUserData() == selectedItem->GetUserData())
            {
                Logger::Warning("Select the same node");
            }
        }
        SelectNode(newSelectedNode, false);
    }
    else
    {
        SelectNode(NULL, false);
    }
}

void SceneGraphModel::SelectNode(DAVA::Entity *node)
{
    SelectNode(node, true);
}

void SceneGraphModel::SelectNode(DAVA::Entity *node, bool selectAtGraph)
{
    if(selectedNode != node)
    {
        selectedNode = node;
        emit SceneNodeSelected(selectedNode);
    }
    
    if(selectAtGraph)
    {
        if(selectedNode)
        {
            GraphItem *selectedItem = ItemForData(selectedNode);
            if(selectedItem)
            {
                SelectItem(selectedItem);
            }
            else
            {
                Rebuild();
            }

        }
        else
        {
            itemSelectionModel->clearSelection();
        }
    }
}

void SceneGraphModel::SelectItem(GraphItem *item, bool needExpand)
{
    QModelIndex idx = createIndex(item->Row(), 0, item);
    itemSelectionModel->select(idx, QItemSelectionModel::ClearAndSelect);
    
    if(attachedTreeView)
    {
        attachedTreeView->scrollTo(idx);
        if (needExpand)
        {
            attachedTreeView->expand(idx);
        }
    }
}

DAVA::Entity * SceneGraphModel::GetSelectedNode()
{
    return selectedNode;
}

Qt::ItemFlags SceneGraphModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
	{
		return Qt::ItemIsDropEnabled;
	}
    
    if(LandscapeEditorModeEnabled())
    {
        return Qt::ItemIsEnabled;
    }

	Qt::ItemFlags itemFlags = Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | GraphModel::flags(index);
	if (IsItemCheckable(index))
	{
		itemFlags |= Qt::ItemIsUserCheckable;
	}

    return itemFlags;
}

bool SceneGraphModel::LandscapeEditorModeEnabled() const
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        return screen->LandscapeEditorModeEnabled();
    }
    
    return false;
}

Qt::DropActions SceneGraphModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;    
}

bool SceneGraphModel::removeRows(int row, int count, const QModelIndex &parent/* = QModelIndex()*/)
{
    DVASSERT((0 < count) && "Wrong count of removed rows");
    
    Logger::Warning("[SceneGraphModel::removeRows] why?");
    

//    GraphItem *parentItem = rootItem;
//    if(parent.isValid())
//    {
//        parentItem = static_cast<GraphItem *>(parent.internalPointer());
//    }
//    
//
//    int32 firstRow = row;
//    int32 lastRow = row + count - 1;
//    beginRemoveRows(parent, firstRow, lastRow);
//    
//    Entity *parentNode = static_cast<Entity *>(parentItem->GetUserData());
//    if(parentNode)
//    {
//        int32 lastNodeRow = Min(lastRow, parentNode->GetChildrenCount() - 1);
//        for(int32 i = firstRow; i <= lastNodeRow; ++i)
//        {
//            Entity *removedNode = parentNode->GetChild(i);
//            parentNode->RemoveNode(removedNode);
//            Logger::FrameworkDebug("[%d] Remove %s from %s", i, removedNode->GetName().c_str(), parentNode->GetName().c_str());
//        }
//        
//        int32 lastItemRow = Min(lastRow, parentItem->ChildrenCount() - 1);
//        for(int32 i = firstRow; i <= lastItemRow; ++i)
//        {
//            parentItem->RemoveChild(i);
//        }
//    }
//    
//    endRemoveRows();
    
    return true;
}

bool SceneGraphModel::insertRows(int row, int count, const QModelIndex &parent/* = QModelIndex()*/)
{
    DVASSERT((0 < count) && "Wrong count of inserted rows");

    GraphItem *parentItem = rootItem;
    if(parent.isValid())
    {
        parentItem = static_cast<GraphItem *>(parent.internalPointer());
    }


    int32 firstRow = row;
    int32 lastRow = row + count - 1;
    beginInsertRows(parent, firstRow, lastRow);

    for(int32 i = firstRow; i <= lastRow; ++i)
    {
        SceneGraphItem *graphItem = new SceneGraphItem();
        graphItem->SetUserData(NULL);
        parentItem->AppendChild(graphItem);
    }
    
    
    endInsertRows();
    return true;
}


bool SceneGraphModel::setData(const QModelIndex &index, const QVariant &value, int role/* = Qt::EditRole*/)
{
    GraphItem *item = static_cast<GraphItem*>(index.internalPointer());
    if (role == Qt::CheckStateRole)
	{
		// Update the checkbox state for the particular model index.
		SetItemCheckState(index, value.toBool());
		return true;
	}

    //TODO: change on real value
    GraphItem *newItem = PointerHolder<GraphItem *>::ToPointer(value);
	if (!newItem)
	{
		return true;
	}

    Entity *newNode = (Entity *)newItem->GetUserData();
    SafeRetain(newNode);
    
    if(newNode && newNode->GetParent())
    {
        newNode->GetParent()->RemoveNode(newNode);
    }
    
    GraphItem *parentItem = item->GetParent();
    Entity *parentNode = static_cast<Entity *>(parentItem->GetUserData());
    if(parentNode)
    {
        parentNode->AddNode(newNode);
    }
    
    item->SetUserData(newNode);
    SafeRelease(newNode);
    
    return true;
}

bool SceneGraphModel::MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndex)
{
    GraphItem *newParentItem = rootItem;
    if(newParentIndex.isValid())
    {
        newParentItem = static_cast<GraphItem*>(newParentIndex.internalPointer());
    }
    
	// Ask the Particles Editor if the move should be handled by it separately.
	if (particlesEditorSceneModelHelper.NeedMoveItemToParent(movedItem, newParentItem))
	{
		return particlesEditorSceneModelHelper.MoveItemToParent(movedItem, newParentItem);
	}

	Entity *movedNode = static_cast<Entity *>(movedItem->GetUserData());
	if (!movedNode)
	{
		// We are attempting to move the node to the empty Scene Graph place, which is not allowed.
		// See please DF-1674.
		return false;
	}

    GraphItem *oldParentItem = movedItem->GetParent();
    
    oldParentItem->RemoveChild(movedItem);
    newParentItem->AppendChild(movedItem);
    
    Entity *newParentNode = static_cast<Entity *>(newParentItem->GetUserData());
    Entity *oldParentNode = static_cast<Entity *>(oldParentItem->GetUserData());

	DVASSERT((NULL != newParentNode) && "newParentNode is NULL");
    DVASSERT((NULL != oldParentNode) && "oldParentNode is NULL");
    
    SafeRetain(movedNode);
    oldParentNode->RemoveNode(movedNode);
    newParentNode->AddNode(movedNode);
    SafeRelease(movedNode);
	
	return true;
}

QVariant SceneGraphModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
	{
		return GraphModel::data(index, role);
	}

	if (Qt::TextColorRole == role)
    {
        Entity *node = static_cast<Entity *>(ItemData(index));
        if(node && (node->GetFlags() & Entity::NODE_INVALID))
        {
            return QColor(255, 0, 0);
        }
    }
	else if(Qt::DecorationRole == role)
	{
		Entity *node = static_cast<Entity *>(ItemData(index));
		if(node)
		{
			return GetDecorationIcon(node);
		}
	}
	
	// Some nodes might be checked - verify this separately.
	if (role == Qt::CheckStateRole && index.column() == 0 && IsItemCheckable(index))
	{
		return GetItemCheckState(index);
	}
    
    return GraphModel::data(index, role);
}

void SceneGraphModel::RefreshParticlesLayer(DAVA::ParticleLayer* layer)
{
	// Changing the layer type might add/remove extra items (like Inner Emitter).
	particlesEditorSceneModelHelper.UpdateLayerRepresentation(rootItem, layer, this);
	
	// Ask Helper to return us the SceneGraph node for this Particle layer.
	SceneGraphItem* itemToRefresh = particlesEditorSceneModelHelper.GetGraphItemForParticlesLayer(rootItem, layer);
	if (itemToRefresh)
	{
		QModelIndex refreshIndex = createIndex(itemToRefresh->Row(), 0, itemToRefresh);
		dataChanged(refreshIndex, refreshIndex);
	}
}

void* SceneGraphModel::GetPersistentDataForModelIndex(const QModelIndex &modelIndex)
{
	// Firstly verify whether this index belongs to Particle Editor.
	void* persistentData = particlesEditorSceneModelHelper.GetPersistentDataForModelIndex(modelIndex);
	if (persistentData)
	{
		return persistentData;
	}

	// Then check for the generic model index.
	GraphItem *item = static_cast<GraphItem*>(modelIndex.internalPointer());
	if (item && item->GetUserData())
	{
		return item->GetUserData();
	}

	return NULL;
}

bool SceneGraphModel::IsItemCheckable(const QModelIndex &index) const
{
	GraphItem* graphItem = GetGraphItemByModelIndex(index);
	if (!graphItem)
	{
		return false;
	}
	
	// Currently only the Particle Editor Layers are checkable.
	return particlesEditorSceneModelHelper.IsGraphItemCheckable(graphItem);
}

Qt::CheckState SceneGraphModel::GetItemCheckState(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return Qt::Unchecked;
	}
	
	GraphItem* graphItem = GetGraphItemByModelIndex(index);
	if (!graphItem)
	{
		return Qt::Unchecked;
	}

	// Currently only the Particle Editor Layers has checked state.
	bool isItemChecked = particlesEditorSceneModelHelper.GetCheckableStateForGraphItem(graphItem);
	return isItemChecked ? Qt::Checked : Qt::Unchecked;
}

void SceneGraphModel::SetItemCheckState(const QModelIndex& index, bool value)
{
	GraphItem* graphItem = GetGraphItemByModelIndex(index);
	if (!graphItem)
	{
		return;
	}
	
	// Currently only the Particle Editor Layers has checked state.
	particlesEditorSceneModelHelper.SetCheckableStateForGraphItem(graphItem, value);
}

GraphItem* SceneGraphModel::GetGraphItemByModelIndex(const QModelIndex& index) const
{
	if (!index.isValid() || !index.internalPointer())
	{
		return NULL;
	}
	
	return static_cast<GraphItem*>(index.internalPointer());
}

void SceneGraphModel::InitDecorationIcons()
{
	QPixmap pix(16,16);
	QPainter p(&pix);

	p.setPen(QColor(64, 64, 64));
	p.setBrush(QBrush(QColor(96, 192, 96)));
	p.drawRect(QRect(0,0,15,15));
	decorationIcons["camera"] = QIcon(pix);

	p.setBrush(QBrush(QColor(255, 255, 96)));
	p.drawRect(QRect(0,0,15,15));
	decorationIcons["light"] = QIcon(pix);

	p.setBrush(QBrush(QColor(96, 96, 192)));
	p.drawRect(QRect(0,0,15,15));
	decorationIcons["user"] = QIcon(pix);

	p.setBrush(QBrush(QColor(96, 192, 192)));
	p.drawRect(QRect(0,0,15,15));
	decorationIcons["lod"] = QIcon(pix);

	p.setBrush(QBrush(QColor(192, 96, 96)));
	p.drawRect(QRect(0,0,15,15));
	decorationIcons["render"] = QIcon(pix);
}

QIcon SceneGraphModel::GetDecorationIcon(DAVA::Entity *node) const
{
	QIcon icon;

	if(NULL != node)
	{
		if(NULL != node->GetComponent(Component::CAMERA_COMPONENT))
		{
			icon = decorationIcons["camera"];
		}
		else if(NULL != node->GetComponent(Component::LIGHT_COMPONENT))
		{
			icon = decorationIcons["light"];
		}
		else if(NULL != node->GetComponent(Component::USER_COMPONENT))
		{
			icon = decorationIcons["user"];
		}
		else if(NULL != node->GetComponent(Component::LOD_COMPONENT))
		{
			icon = decorationIcons["lod"];
		}
		else if(NULL != node->GetComponent(Component::RENDER_COMPONENT))
		{
			 icon = decorationIcons["render"];
		}
	}

	return icon;
}

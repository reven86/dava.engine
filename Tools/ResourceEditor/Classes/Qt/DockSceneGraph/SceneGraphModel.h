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



#ifndef __SCENE_GRAPH_MODEL_H__
#define __SCENE_GRAPH_MODEL_H__

#include "DAVAEngine.h"
#include "GraphModel.h"
#include "ParticlesEditorQT/Helpers/ParticlesEditorSceneModelHelper.h"

#include <QObject>
#include <QMap>
#include <QIcon>

class EditorScene;
class SceneGraphItem;
class SceneGraphModel: public GraphModel
{
    Q_OBJECT
    
public:
    SceneGraphModel(QObject *parent = 0);
    virtual ~SceneGraphModel();

    void SetScene(EditorScene * newScene);
	EditorScene* GetScene() const {return scene;};

	// Rebuild the model for the appropriate node and for the whole graph.
	void RebuildNode(DAVA::Entity* rootNode);
    virtual void Rebuild();

	// Refresh the Particle Editor Layer.
	void RefreshParticlesLayer(DAVA::ParticleLayer* layer);

    void SelectNode(DAVA::Entity *node);
    DAVA::Entity * GetSelectedNode();

	// Get the persistent data for Model Index, needed for save/restore
	// SceneGraphModel expanded state.
	void* GetPersistentDataForModelIndex(const QModelIndex &modelIndex);

	// Add/remove the nodes to the tree.
	void AddNodeToTree(GraphItem* parentItem, GraphItem* childItem);
	void RemoveNodeFromTree(GraphItem* parentItem, GraphItem* childItem);

	const DAVA::ParticlesEditorSceneModelHelper& GetParticlesEditorSceneModelHelper() const { return particlesEditorSceneModelHelper; };

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
    virtual bool MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndex);

    virtual QVariant data(const QModelIndex &index, int role) const;
    
Q_SIGNALS:
    
    void SceneNodeSelected(DAVA::Entity *node);
    
protected:
    void InitDecorationIcons();
	QIcon GetDecorationIcon(DAVA::Entity *node) const;

    void SelectNode(DAVA::Entity *node, bool selectAtGraph);
    void SelectItem(GraphItem *item, bool needExpand = false);
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void AddNodeToTree(GraphItem *parent, DAVA::Entity *node, bool partialUpdate = false);
    
    bool LandscapeEditorModeEnabled() const;
    
    // Custom selection handling for Particle Editor.
    bool HandleParticleEditorSelection();
	
	// Tree item checkboxes functionality.
	// Is particular item checkable?
	bool IsItemCheckable(const QModelIndex &index) const;
	
	// Get/set the checked state for the item.
	Qt::CheckState GetItemCheckState(const QModelIndex& index) const;
	void SetItemCheckState(const QModelIndex& index, bool value);
	
	// Get the graph model by Model Index (or NULL if no Graph Model attached).
	GraphItem* GetGraphItemByModelIndex(const QModelIndex& index) const;

	//Nodes which should not be displayed in SceneGraph tree must return false when passed to this function
	bool IsNodeAccepted(DAVA::Entity* node);
protected:

    EditorScene *scene;
    DAVA::Entity *selectedNode;
    
    DAVA::ParticlesEditorSceneModelHelper particlesEditorSceneModelHelper;
    
    // Selected Scene Graph item for Particle Editor.
    SceneGraphItem* selectedGraphItemForParticleEditor;

	QMap<QString, QIcon> decorationIcons;
};

#endif // __SCENE_GRAPH_MODEL_H__

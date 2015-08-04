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


#ifndef __QT_SCENE_TREE_H__
#define __QT_SCENE_TREE_H__

#include <QWidget>
#include <QTreeView>
#include <QTableView>
#include <QTimer>
#include <QPointer>

#include "Scene/SceneSignals.h"
#include "DockSceneTree/SceneTreeModel.h"
#include "DockSceneTree/SceneTreeDelegate.h"


class LazyUpdater;
class SceneTree
    : public QTreeView
{
	Q_OBJECT

public:
	explicit SceneTree(QWidget *parent = 0);
	~SceneTree() = default;

public slots:
	void ShowContextMenu(const QPoint &pos);
	void SetFilter(const QString &filter);

    void LookAtSelection();
	void RemoveSelection();

	void CollapseSwitch();
	
	void SetEntityNameAsFilter();

	// Particle Emitter handlers.
	void AddEmitter();
    void SaveEffectEmitters();
    void SaveEffectEmittersAs();
	void StartEffect();
	void StopEffect();
	void RestartEffect();
	
	void AddLayer();
	void RemoveEmitter();
	void LoadEmitterFromYaml();
	void SaveEmitterToYaml();
	void SaveEmitterToYamlAs();

	void LoadInnerEmitterFromYaml();
	void SaveInnerEmitterToYaml();
	void SaveInnerEmitterToYamlAs();
	void PerformSaveInnerEmitter(bool forceAskFileName);

	void CloneLayer();
	void RemoveLayer();
	void AddForce();
	void RemoveForce();

	void EditModel();
	void ReloadModel();
	void ReloadModelAs();
	void SaveEntityAs();
	
	void CollapseAll();
    
	void SetCurrentCamera();
    void SetCustomDrawCamera();

private slots:
	void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);
	void SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);
	void SceneStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent);
	void CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);

	void ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);

	void TreeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
	void TreeItemClicked(const QModelIndex & index);
	void TreeItemDoubleClicked(const QModelIndex & index);
	void TreeItemCollapsed(const QModelIndex &index);
	void TreeItemExpanded(const QModelIndex &index);

	void SyncSelectionToTree();
	void SyncSelectionFromTree();

private:
	void ShowContextMenuEntity(DAVA::Entity *entity, int entityCustomFlags, const QPoint &pos);
	
	void ShowContextMenuEmitter(DAVA::ParticleEffectComponent *effect, DAVA::ParticleEmitter *emitter, const QPoint &pos);
	void ShowContextMenuLayer(DAVA::ParticleEmitter *emitter, DAVA::ParticleLayer *layer, const QPoint &pos);
	void ShowContextMenuForce(DAVA::ParticleLayer *layer, DAVA::ParticleForce *force, const QPoint &pos);
	void ShowContextMenuInnerEmitter(DAVA::ParticleEffectComponent *effect, DAVA::ParticleEmitter *emitter, DAVA::ParticleLayer *parentLayer, const QPoint &pos);
	// Helpers for Particles.
	// Get the default path to Particles Config.
	QString GetParticlesConfigPath();
	
	// Perform save for selected Emitters.
	void PerformSaveEmitter(ParticleEmitter *emitter, bool forceAskFileName, const QString& defaultName);

    void PerformSaveEffectEmitters(bool forceAskFileName);

	// Cleanup the selected Particle Editor items.
	void CleanupParticleEditorSelectedItems();
    
    void AddCameraActions(QMenu &menu);

	void dropEvent(QDropEvent * event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);

	void GetDropParams(const QPoint &pos, QModelIndex &index, int &row, int &col);

	void EmitParticleSignals(const QItemSelection & selected);

    void ExpandFilteredItems();
    void BuildExpandItemsSet(QSet<QModelIndex>& indexSet, const QModelIndex& parent = QModelIndex());
	
	void UpdateTree();

	ParticleEffectComponent *selectedEffect;
	ParticleEmitter *selectedEmitter;
	ParticleLayer* selectedLayer;
	ParticleForce* selectedForce;

	QPointer< SceneTreeModel > treeModel;
	QPointer< SceneTreeFilteringModel > filteringProxyModel;
	SceneTreeDelegate *treeDelegate;

	bool isInSync;

	LazyUpdater *treeUpdater;
};

#endif // __QT_SCENE_TREE_H__

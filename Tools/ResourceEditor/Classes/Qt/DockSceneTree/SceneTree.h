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
class SceneTree : public QTreeView
{
    Q_OBJECT

public:
    explicit SceneTree(QWidget* parent = 0);

protected:
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;

private slots:
    void ShowContextMenu(const QPoint& pos);
    void SetFilter(const QString& filter);

    void CollapseSwitch();
    void CollapseAll();

    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SceneStructureChanged(SceneEditor2* scene, DAVA::Entity* parent);
    void CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);

    void ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);

    void TreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void TreeItemClicked(const QModelIndex& index);
    void TreeItemDoubleClicked(const QModelIndex& index);
    void TreeItemCollapsed(const QModelIndex& index);
    void TreeItemExpanded(const QModelIndex& index);

    void SyncSelectionToTree();
    void SyncSelectionFromTree();

private:
    void GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col);

    void EmitParticleSignals();

    void ExpandFilteredItems();
    void BuildExpandItemsSet(QSet<QModelIndex>& indexSet, const QModelIndex& parent = QModelIndex());

    void UpdateTree();
    void PropagateSolidFlag();
    void PropagateSolidFlagRecursive(QStandardItem* root);

    class BaseContextMenu;
    class EntityContextMenu;
    class ParticleLayerContextMenu;
    class ParticleForceContextMenu;
    class ParticleEmitterContextMenu;
    class ParticleInnerEmitterContextMenu;

private:
    QPointer<SceneTreeModel> treeModel;
    QPointer<SceneTreeFilteringModel> filteringProxyModel;
    SceneTreeDelegate* treeDelegate = nullptr;
    LazyUpdater* treeUpdater = nullptr;
    bool isInSync = false;
};

#endif // __QT_SCENE_TREE_H__

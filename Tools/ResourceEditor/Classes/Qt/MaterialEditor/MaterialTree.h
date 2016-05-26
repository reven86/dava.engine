#ifndef __MATERIAL_TREE_H__
#define __MATERIAL_TREE_H__

#include <QWidget>
#include <QTreeView>
#include <QMap>

#include "MaterialModel.h"

class SelectableGroup;
class MaterialFilteringModel;

class MaterialTree : public QTreeView
{
    Q_OBJECT

public:
    MaterialTree(QWidget* parent = 0);
    ~MaterialTree();

    void SetScene(SceneEditor2* sceneEditor);
    DAVA::NMaterial* GetMaterial(const QModelIndex& index) const;

    void SelectMaterial(DAVA::NMaterial* material);
    void SelectEntities(const QList<DAVA::NMaterial*>& materials);

    void Update();

    int getFilterType() const;
    void setFilterType(int filterType);

signals:
    void Updated();
    void ContextMenuPrepare(QMenu*);

public slots:
    void ShowContextMenu(const QPoint& pos);
    void OnCommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);
    void OnStructureChanged(SceneEditor2* scene, DAVA::Entity* parent);
    void OnSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void OnSelectEntities();

protected:
    MaterialFilteringModel* treeModel;

    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

    void dragTryAccepted(QDragMoveEvent* event);
    void GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col);
};

#endif // __MATERIALS_TREE_H__

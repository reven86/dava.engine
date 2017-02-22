#ifndef __QT_SCENE_TREE_MODEL_H__
#define __QT_SCENE_TREE_MODEL_H__

#include <QPair>
#include <QMap>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "Scene/SceneEditor2.h"
#include "Qt/DockSceneTree/SceneTreeItem.h"

// framework
#include "Scene3D/Scene.h"

class SceneTreeModel
: public QStandardItemModel
{
    Q_OBJECT

public:
    enum DropType
    {
        DropingUnknown = -1,
        DropingMixed = 0,

        DropingEntity,
        DropingLayer,
        DropingEmitter,
        DropingForce
    };

    enum CustomFlags
    {
        CF_None = 0x0000,
        CF_Disabled = 0x0001,
        CF_Invisible = 0x0002,
    };

    SceneTreeModel(QObject* parent = 0);
    ~SceneTreeModel();

    void SetScene(SceneEditor2* scene);
    SceneEditor2* GetScene() const;

    QModelIndex GetIndex(Selectable::Object* object) const;

    SceneTreeItem* GetItem(const QModelIndex& index) const;

    void SetSolid(const QModelIndex& index, bool solid);
    bool GetSolid(const QModelIndex& index) const;

    void SetLocked(const QModelIndex& index, bool locked);
    bool GetLocked(const QModelIndex& index) const;

    QVector<QIcon> GetCustomIcons(const QModelIndex& index) const;
    int GetCustomFlags(const QModelIndex& index) const;

    // drag and drop support
    Qt::DropActions supportedDropActions() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    QStringList mimeTypes() const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    bool DropCanBeAccepted(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const;
    bool DropAccepted() const;
    int GetDropType(const QMimeData* data) const;

    void ResyncStructure(QStandardItem* item, DAVA::Entity* entity);

    void SetFilter(const QString& text);
    void ReloadFilter();
    bool IsFilterSet() const;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private slots:
    void ItemChanged(QStandardItem* item);

private:
    void RebuildIndexesCache();
    void AddIndexesCache(SceneTreeItem* item);
    bool AreSameType(const QModelIndexList& indexes) const;
    void SetFilterInternal(const QModelIndex& parent, const QString& text);
    void ResetFilter(const QModelIndex& parent = QModelIndex());

    Qt::DropActions supportedDragActions() const override;

private:
    SceneEditor2* curScene = nullptr;
    DAVA::Map<Selectable::Object*, QModelIndex> indexesCache;
    QString filterText;
    bool dropAccepted = false;
};

class SceneTreeFilteringModel : public QSortFilterProxyModel
{
public:
    SceneTreeFilteringModel(SceneTreeModel* treeModel, QObject* parent = NULL);
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

protected:
    SceneTreeModel* treeModel;
};

#endif // __QT_SCENE_TREE_MODEL_H__

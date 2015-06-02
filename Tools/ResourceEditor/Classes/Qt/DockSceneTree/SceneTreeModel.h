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
		DropingForce,
        DropingMaterial
	};

	enum CustomFlags
	{
		CF_None			= 0x0000,
		CF_Disabled		= 0x0001,
		CF_Invisible	= 0x0002,
	};

	SceneTreeModel(QObject* parent = 0);
	~SceneTreeModel();

	void SetScene(SceneEditor2 *scene);
	SceneEditor2* GetScene() const;

	QModelIndex GetIndex(DAVA::Entity *entity) const;
    QModelIndex GetIndex(DAVA::ParticleEmitter *emitter) const;
	QModelIndex GetIndex(DAVA::ParticleLayer *layer) const;
	QModelIndex GetIndex(DAVA::ParticleForce *force) const;

	SceneTreeItem* GetItem(const QModelIndex &index) const;

	void SetSolid(const QModelIndex &index, bool solid);
	bool GetSolid(const QModelIndex &index) const;

	void SetLocked(const QModelIndex &index, bool locked);
	bool GetLocked(const QModelIndex &index) const;

	QVector<QIcon> GetCustomIcons(const QModelIndex &index) const;
	int GetCustomFlags(const QModelIndex &index) const;

	// drag and drop support
	Qt::DropActions supportedDropActions() const override;
	QMimeData *	mimeData(const QModelIndexList & indexes) const override;
	QStringList	mimeTypes() const override;
	bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;
	bool DropCanBeAccepted(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
	bool DropAccepted() const;
	int GetDropType(const QMimeData *data) const;

	void ResyncStructure(QStandardItem *item, DAVA::Entity *entity);

    void SetFilter(const QString& text);
    void ReloadFilter();
    bool IsFilterSet() const;

    Qt::ItemFlags flags ( const QModelIndex & index ) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    
private slots:
	void ItemChanged(QStandardItem * item);

private:
    void RebuildIndexesCache();
	void AddIndexesCache(SceneTreeItem *item);
	bool AreSameType(const QModelIndexList & indexes) const;
    void SetFilterInternal(const QModelIndex& parent, const QString& text);
    void ResetFilter(const QModelIndex& parent = QModelIndex());

    Qt::DropActions supportedDragActions() const;

    SceneEditor2 * curScene;
	bool dropAccepted;
    QString filterText;

	QMap<DAVA::Entity*, QModelIndex> indexesCacheEntities;
    QMap<DAVA::ParticleEmitter*, QModelIndex> indexesCacheEmitters;
	QMap<DAVA::ParticleLayer*, QModelIndex> indexesCacheLayers;
	QMap<DAVA::ParticleForce*, QModelIndex> indexesCacheForces;
};

class SceneTreeFilteringModel : public QSortFilterProxyModel
{
public:
	SceneTreeFilteringModel(SceneTreeModel *treeModel, QObject *parent = NULL);
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

protected:
	SceneTreeModel *treeModel;
};

#endif // __QT_SCENE_TREE_MODEL_H__

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


#ifndef __MATERIAL_TREE_H__
#define __MATERIAL_TREE_H__

#include <QWidget>
#include <QTreeView>
#include <QMap>

#include "MaterialModel.h"

class EntityGroup;
class MaterialFilteringModel;

class MaterialTree : public QTreeView
{
	Q_OBJECT

public:
	MaterialTree(QWidget *parent = 0);
	~MaterialTree();

	void SetScene(SceneEditor2 *sceneEditor);
	DAVA::NMaterial* GetMaterial(const QModelIndex &index) const;

	void SelectMaterial(DAVA::NMaterial *material);
	void SelectEntities(const QList<DAVA::NMaterial *>& materials);

	void Update();

    int getFilterType() const;
    void setFilterType( int filterType );

signals:
    void Updated();
    void ContextMenuPrepare(QMenu *);

public slots:
	void ShowContextMenu(const QPoint &pos);
	void OnCommandExecuted(SceneEditor2 *scene, const Command2 *command, bool redo);
	void OnStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent);
	void OnSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);
	void OnSelectEntities();

protected:
	MaterialFilteringModel *treeModel;

	void dragEnterEvent(QDragEnterEvent * event);
	void dragMoveEvent(QDragMoveEvent * event);
	void dropEvent(QDropEvent * event);

	void dragTryAccepted(QDragMoveEvent *event);
	void GetDropParams(const QPoint &pos, QModelIndex &index, int &row, int &col);
};

#endif // __MATERIALS_TREE_H__

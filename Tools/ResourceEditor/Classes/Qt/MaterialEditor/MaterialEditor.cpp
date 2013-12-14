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

#include "MaterialEditor.h"
#include "ui_materialeditor.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"

MaterialEditor::MaterialEditor(QWidget *parent /* = 0 */)
: QDialog(parent)
, ui(new Ui::MaterialEditor)
{
	ui->setupUi(this);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

	// global scene manager signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));

	// ui signals
	QObject::connect(ui->materialTree, SIGNAL(clicked(const QModelIndex &)), this, SLOT(materialClicked(const QModelIndex &)));

	ui->materialTree->setDragEnabled(true);
	ui->materialTree->setAcceptDrops(true);
	ui->materialTree->setDragDropMode(QAbstractItemView::DragDrop);

	posSaver.Attach(this);
	posSaver.LoadState(ui->splitter);
	posSaver.LoadState(ui->splitter_2);
}

MaterialEditor::~MaterialEditor()
{ 
	posSaver.SaveState(ui->splitter);
	posSaver.SaveState(ui->splitter_2);
}

void MaterialEditor::sceneActivated(SceneEditor2 *scene)
{
	if(isVisible())
	{
		ui->materialTree->SetScene(scene);
		ui->materialTree->sortByColumn(0, Qt::AscendingOrder);
		ui->materialTree->expandToDepth(0);
	}
}

void MaterialEditor::sceneDeactivated(SceneEditor2 *scene)
{ }

void MaterialEditor::sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{ }

void MaterialEditor::materialClicked(const QModelIndex &index)
{
	ui->materialProperty->RemovePropertyAll();

	DAVA::NMaterial *material = ui->materialTree->GetMaterial(index);
	if(NULL != material)
	{
		const DAVA::InspInfo *info = material->GetTypeInfo();
		QtPropertyDataIntrospection *inspData = new QtPropertyDataIntrospection(material, info);

		while(0 != inspData->ChildCount())
		{
			QtPropertyData *c = inspData->ChildGet(0);
			inspData->ChildExtract(c);

			ui->materialProperty->AppendProperty(c->GetName(), c);
		}

		delete inspData;
	}
}

void MaterialEditor::showEvent(QShowEvent * event)
{
	sceneActivated(QtMainWindow::Instance()->GetCurrentScene());
}

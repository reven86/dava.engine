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



#include <QFile>

#include "MaterialBrowser/MaterialBrowser.h"

#include "ui_materialbrowser.h"

MaterialBrowser::MaterialBrowser(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::MaterialBrowser)
{
    ui->setupUi(this);
	setWindowFlags(Qt::Window);

	treeModelScene = new MaterialTreeModel();
	viewModel = new MaterialViewModel();

	SetupModelTree();
	SetupModelView();

	posSaver.Attach(this);
}

MaterialBrowser::~MaterialBrowser()
{
	delete viewModel;
	delete treeModelScene;
    delete ui;
}

void MaterialBrowser::SetScene(DAVA::Scene *scene)
{
	treeModelScene->SetScene(scene);
}

void MaterialBrowser::SetupModelTree()
{
	ui->treeScene->setModel(treeModelScene);

	QObject::connect(ui->treeScene, SIGNAL(Selected(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeItemSelected(const QItemSelection &, const QItemSelection &)));
	QObject::connect(ui->treeScene, SIGNAL(pressed(const QModelIndex &)), this, SLOT(TreeItemPressed(const QModelIndex &)));
}

void MaterialBrowser::SetupModelView()
{
	ui->materialView->setModel(viewModel);
}

void MaterialBrowser::TreeItemSelected(const QItemSelection &selected, const QItemSelection &deselected)
{ }

void MaterialBrowser::TreeItemPressed(const QModelIndex &index)
{
	MaterialTreeItem *item = treeModelScene->Item(index);
	if(NULL != item)
	{
		if(0 == item->ChildCount() && NULL != item->Parent())
		{
			item = item->Parent();
		}

		viewModel->SetTreeItem(item);
	}
}

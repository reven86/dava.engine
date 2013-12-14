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



#include "MaterialsWidget.h"
#include "SimpleMaterialModel.h"

#include "Main/mainwindow.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

#include "MaterialEditor/MaterialAssignSystem.h"

#include <QToolBar>
#include <QLineEdit>
#include <QListView>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QSortFilterProxyModel>

MaterialsWidget::MaterialsWidget(QWidget *parent /* = 0 */)
	: QWidget(parent)
    , curScene(NULL)
{
    SetupToolbar();
    SetupView();
    SetupLayout();

    ViewAsList();
}

MaterialsWidget::~MaterialsWidget()
{
}

void MaterialsWidget::SetupSignals()
{
    QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));

    QObject::connect(materialsView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
    QObject::connect(materialsView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
}

void MaterialsWidget::SetupToolbar()
{
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setMovable(false);

    searchFilter = new QLineEdit(toolbar);
    searchFilter->setToolTip("Enter text to search material");
    searchFilter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    
	QIcon resetIcon(QString::fromUtf8(":/QtIconsTextureDialog/editclear.png"));
    QAction *actionResetFilter = new QAction(resetIcon, "Reset search filter", toolbar);

    QIcon asListIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_list.png"));
    actionViewAsList = new QAction(asListIcon, "View as list", toolbar);
    actionViewAsList->setCheckable(true);
    actionViewAsList->setChecked(true);

    QIcon asTiledIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_pictures.png"));
    actionViewAsTiles = new QAction(asTiledIcon, "View tiled", toolbar);
    actionViewAsTiles->setCheckable(true);
    actionViewAsTiles->setChecked(false);

    QObject::connect(searchFilter, SIGNAL(textChanged(const QString &)), this, SLOT(SetFilter(const QString &)));
    QObject::connect(actionResetFilter, SIGNAL(triggered()), this, SLOT(ResetFilter()));
    QObject::connect(actionViewAsList, SIGNAL(triggered()), this, SLOT(ViewAsList()));
    QObject::connect(actionViewAsTiles, SIGNAL(triggered()), this, SLOT(ViewAsTiles()));
    
    toolbar->addWidget(searchFilter);
    toolbar->addAction(actionResetFilter);
    toolbar->addSeparator();
    
    toolbar->addAction(actionViewAsList);
    toolbar->addAction(actionViewAsTiles);
}

void MaterialsWidget::SetupView()
{
    materialsModel = new SimpleMaterialModel(this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(materialsModel);

    materialsView = new QListView(this);
    materialsView->setContextMenuPolicy(Qt::CustomContextMenu);
    materialsView->setDragDropMode(QAbstractItemView::DragOnly);
	materialsView->setDragEnabled(true);
    materialsView->setModel(proxyModel);
    
	notFoundMessage = new QLabel("Nothing found", this);
	notFoundMessage->setMinimumHeight(20);
	notFoundMessage->setAlignment(Qt::AlignCenter);
}

void MaterialsWidget::SetupLayout()
{
	layout = new QVBoxLayout();
	layout->addWidget(toolbar);
	layout->addWidget(notFoundMessage);
	layout->addWidget(materialsView);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
    
    notFoundMessage->setVisible(false);
    materialsView->setVisible(true);
}


void MaterialsWidget::ViewAsList()
{
    materialsView->setViewMode(QListView::ListMode);
    materialsView->setIconSize(QSize());
    
    actionViewAsList->setChecked(true);
    actionViewAsTiles->setChecked(false);
}

void MaterialsWidget::ViewAsTiles()
{
    materialsView->setViewMode(QListView::IconMode);
    materialsView->setIconSize(QSize(50, 50));
    
    actionViewAsList->setChecked(false);
    actionViewAsTiles->setChecked(true);
}

void MaterialsWidget::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(0 == selected.count()) return;

    const QItemSelection realSelection = proxyModel->mapSelectionToSource(selected);
    const QModelIndex index = realSelection.indexes().first();
}


void MaterialsWidget::ShowContextMenu(const QPoint & point)
{
    const QModelIndex index = proxyModel->mapToSource(materialsView->indexAt(point));
    
	if(!index.isValid()) return;
    
    DAVA::NMaterial *material = materialsModel->GetMaterial(index);
    
    QMenu contextMenu(this);
    QVariant materialAsVariant = QVariant::fromValue<DAVA::NMaterial *>(material);

    QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEdit()));
    actionEdit->setData(materialAsVariant);

    QAction * actionAssign = contextMenu.addAction("Assign to Selection", this, SLOT(OnAssignToSelection()));
    actionAssign->setData(materialAsVariant);

    contextMenu.exec(materialsView->mapToGlobal(point));
}

void MaterialsWidget::SetFilter(const QString &filter)
{
	bool b = proxyModel->blockSignals(true);
	proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
	proxyModel->blockSignals(b);

	Invalidate();

	SwitchListAndLabel();
}


void MaterialsWidget::ResetFilter()
{
	if(searchFilter->text().isEmpty()) return;

    searchFilter->setText("");
    SetFilter("");
}

void MaterialsWidget::SceneActivated(SceneEditor2 *scene)
{
    curScene = scene;
    
    materialsModel->SetScene(curScene);
    Invalidate();
    
    SwitchListAndLabel();
}

void MaterialsWidget::SceneDeactivated(SceneEditor2 *scene)
{
    curScene = NULL;
    
    materialsModel->SetScene(curScene);
    Invalidate();

    SwitchListAndLabel();
}

void MaterialsWidget::Invalidate()
{
    proxyModel->invalidate();

    const QModelIndex rootIndex = materialsModel->indexFromItem(materialsModel->invisibleRootItem());
    materialsView->setRootIndex(proxyModel->mapFromSource(rootIndex));
}


void MaterialsWidget::SwitchListAndLabel()
{
    if(proxyModel->rowCount())
    {
		notFoundMessage->setVisible(false);
		materialsView->setVisible(true);
    }
    else
    {
		notFoundMessage->setVisible(true);
		materialsView->setVisible(false);
    }
}

void MaterialsWidget::OnEdit()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    DAVA::NMaterial *material = indexAsVariant.value<DAVA::NMaterial *>();
}

void MaterialsWidget::OnAssignToSelection()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    DAVA::NMaterial *material = indexAsVariant.value<DAVA::NMaterial *>();
    
    EntityGroup selection = curScene->selectionSystem->GetSelection();
    MaterialAssignSystem::AssignMaterialToGroup(curScene, &selection, material);
}



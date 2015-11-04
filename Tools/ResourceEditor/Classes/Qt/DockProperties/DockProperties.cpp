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


#include "DockProperties.h"
#include "Main/mainwindow.h"
#include <QComboBox>

DockProperties::DockProperties(QWidget *parent /* = NULL */)
	: QDockWidget(parent)
    , addComponentMenu(new QMenu(this))
{ }

DockProperties::~DockProperties()
{ }

void DockProperties::Init()
{
	Ui::MainWindow* ui = QtMainWindow::Instance()->GetUI();

	// toggle propertyEditor view mode
	QComboBox *viewModes = new QComboBox();
	viewModes->addItem("Basic", (int) PropertyEditor::VIEW_NORMAL);
	viewModes->addItem("Advanced", (int) PropertyEditor::VIEW_ADVANCED);
	viewModes->addItem("Favorites only", (int) PropertyEditor::VIEW_FAVORITES_ONLY);

	ui->propertiesToolBar->addSeparator();
	ui->propertiesToolBar->addWidget(viewModes);
	QObject::connect(viewModes, SIGNAL(activated(int)), this, SLOT(ViewModeSelected(int)));

	ui->propertyEditor->SetViewMode(PropertyEditor::VIEW_ADVANCED);
	viewModes->setCurrentIndex(1);

	// toggle favorites edit mode
	QObject::connect(ui->actionFavoritesEdit, SIGNAL(triggered()), this, SLOT(ActionFavoritesEdit()));
	ui->propertyEditor->SetFavoritesEditMode(ui->actionFavoritesEdit->isChecked());

    // Add components
    addComponentMenu->addAction(ui->actionAddActionComponent);
    addComponentMenu->addAction(ui->actionAddQualitySettingsComponent);
    addComponentMenu->addAction(ui->actionAddStaticOcclusionComponent);
    addComponentMenu->addAction(ui->actionAddSoundComponent);
    addComponentMenu->addAction(ui->actionAddWaveComponent);
    addComponentMenu->addAction(ui->actionAddSkeletonComponent);
    addComponentMenu->addAction(ui->actionAddPathComponent);
    addComponentMenu->addAction(ui->actionAddRotationComponent);
    addComponentMenu->addAction(ui->actionAddSnapToLandscapeComponent);
    addComponentMenu->addAction(ui->actionAddWASDComponent);
    
    connect(ui->actionAddNewComponent, SIGNAL(triggered()), SLOT(OnAddAction()));
}

void DockProperties::ActionFavoritesEdit()
{
	QAction *favoritesEditAction = dynamic_cast<QAction *>(QObject::sender());
	if(NULL != favoritesEditAction)
	{
		QtMainWindow::Instance()->GetUI()->propertyEditor->SetFavoritesEditMode(favoritesEditAction->isChecked());
	}
}

void DockProperties::ViewModeSelected(int index)
{
	QComboBox *viewModes = dynamic_cast<QComboBox*>(QObject::sender());

	if(NULL != viewModes)
	{
		PropertyEditor::eViewMode mode = (PropertyEditor::eViewMode) viewModes->itemData(index).toInt();
		QtMainWindow::Instance()->GetUI()->propertyEditor->SetViewMode(mode);
	}
}

void DockProperties::OnAddAction()
{
    Ui::MainWindow* ui = QtMainWindow::Instance()->GetUI();
    QWidget *w = ui->propertiesToolBar->widgetForAction(ui->actionAddNewComponent);
    QPoint pos = QCursor::pos();

    if (w != NULL)
    {
        pos = w->mapToGlobal(QPoint(0, w->height()));
    }

    addComponentMenu->exec(pos, ui->actionAddNewComponent);
}

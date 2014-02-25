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


#include "StatusBar.h"

#include "Main/mainwindow.h"
#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/SelectionSystem.h"

#include <QLabel>
#include <QLayout>
#include <QPalette>

StatusBar::StatusBar(QWidget *parent)
	: QStatusBar(parent)
{
	sceneGeometry = new QLabel(this);
	sceneGeometry->setToolTip("Resolution");
	sceneGeometry->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	addPermanentWidget(sceneGeometry);

	distanceToCamera = new QLabel(this);
	distanceToCamera->setToolTip("Distance from camera to center of the selection");
	distanceToCamera->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	addPermanentWidget(distanceToCamera);

	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setMargin(0);
	layout()->setSpacing(1);
    setStyleSheet("QStatusBar::item {border: none;}");
}

StatusBar::~StatusBar()
{

}

void StatusBar::SetDistanceToCamera(DAVA::float32 distance)
{
    distanceToCamera->setText(QString::fromStdString(DAVA::Format("%0.6f", distance)));
}

void StatusBar::ResetDistanceToCamera()
{
    distanceToCamera->setText(QString::fromStdString("No selection"));
}

void StatusBar::UpdateDistanceToCamera()
{
	SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
	if(!scene)
	{
		ResetDistanceToCamera();
		return;
	}

	if(scene->selectionSystem->GetSelectionCount() > 0)
	{
		float32 distanceToCamera = scene->cameraSystem->GetDistanceToCamera();
		SetDistanceToCamera(distanceToCamera);
	}
	else
	{
		ResetDistanceToCamera();
	}
}

void StatusBar::SceneActivated( SceneEditor2 *scene )
{
	UpdateDistanceToCamera();
}

void StatusBar::SceneSelectionChanged( SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected )
{
	UpdateDistanceToCamera();
}

void StatusBar::UpdateByTimer()
{
	UpdateDistanceToCamera();
}

void StatusBar::OnSceneGeometryChaged( int width, int height )
{
	sceneGeometry->setText(QString::fromStdString(DAVA::Format("%d x %d", width, height)));
}



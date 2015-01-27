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



#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__

#include "DAVAEngine.h"

#include <QStatusBar>

class QLabel;
class SceneEditor2;
class EntityGroup;
class Command2;
class StatusBar : public QStatusBar
{
	Q_OBJECT

public:
	explicit StatusBar(QWidget *parent = 0);
	~StatusBar();


public slots:
	void SceneActivated(SceneEditor2 *scene);
	void SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);
	void CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);
	void StructureChanged(SceneEditor2 *scene, DAVA::Entity *parent);
    
	void UpdateByTimer();

	void OnSceneGeometryChaged(int width, int height, int dpr);

protected:
	void UpdateDistanceToCamera();
	void SetDistanceToCamera(DAVA::float32 distance);
	void ResetDistanceToCamera();
	void UpdateSelectionBoxSize(SceneEditor2 *scene);

    QLabel * distanceToCamera;
	QLabel * sceneGeometry;
    QLabel * selectionBoxSize;
};

#endif // __STATUS_BAR_H__

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


#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__

#include <QWidget>
#include "DAVAEngine.h"

#include "LandscapeEditorPanels/LandscapeEditorBasePanel.h"

class CustomColorsPanel;
class RulerToolPanel;
class VisibilityToolPanel;
class TilemaskEditorPanel;
class HeightmapEditorPanel;

class LandscapeEditorControlsPlaceholder: public QWidget
{
	Q_OBJECT
	
public:
	explicit LandscapeEditorControlsPlaceholder(QWidget* parent = 0);
	~LandscapeEditorControlsPlaceholder();
	
	void SetPanel(LandscapeEditorBasePanel* panel);
	void RemovePanel();

public slots:
    
    void OnOpenGLInitialized();
    
private slots:
	void SceneActivated(SceneEditor2* scene);
	void SceneDeactivated(SceneEditor2* scene);

	void EditorToggled(SceneEditor2* scene);

private:
	SceneEditor2* activeScene;
	LandscapeEditorBasePanel* currentPanel;

	CustomColorsPanel* customColorsPanel;
	RulerToolPanel* rulerToolPanel;
	VisibilityToolPanel* visibilityToolPanel;
	TilemaskEditorPanel* tilemaskEditorPanel;
	HeightmapEditorPanel* heightmapEditorPanel;

	void InitUI();
	void ConnectToSignals();
	void CreatePanels();

	void UpdatePanels();
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__) */

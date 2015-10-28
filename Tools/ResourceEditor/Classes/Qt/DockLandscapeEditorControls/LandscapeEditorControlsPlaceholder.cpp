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


#include "LandscapeEditorControlsPlaceholder.h"
#include "../Scene/SceneSignals.h"
#include "../Scene/SceneEditor2.h"

#include "LandscapeEditorPanels/CustomColorsPanel.h"
#include "LandscapeEditorPanels/RulerToolPanel.h"
#include "LandscapeEditorPanels/VisibilityToolPanel.h"
#include "LandscapeEditorPanels/TilemaskEditorPanel.h"
#include "LandscapeEditorPanels/HeightmapEditorPanel.h"

#include <QVBoxLayout>

#include "QtTools/DavaGLWidget/davaglwidget.h"

LandscapeEditorControlsPlaceholder::LandscapeEditorControlsPlaceholder(QWidget* parent)
:	QWidget(parent)
,	activeScene(nullptr)
,	currentPanel(nullptr)
,   customColorsPanel(nullptr)
,   rulerToolPanel(nullptr)
,   visibilityToolPanel(nullptr)
,   tilemaskEditorPanel(nullptr)
,   heightmapEditorPanel(nullptr)
{
	InitUI();
	ConnectToSignals();

}

void LandscapeEditorControlsPlaceholder::OnOpenGLInitialized()
{
    DVASSERT(!customColorsPanel && !rulerToolPanel && !visibilityToolPanel && !tilemaskEditorPanel && !heightmapEditorPanel);

    customColorsPanel = new CustomColorsPanel();
    rulerToolPanel = new RulerToolPanel();
    visibilityToolPanel = new VisibilityToolPanel();
    tilemaskEditorPanel = new TilemaskEditorPanel();
    heightmapEditorPanel = new HeightmapEditorPanel();
}

LandscapeEditorControlsPlaceholder::~LandscapeEditorControlsPlaceholder()
{
	SafeDelete(customColorsPanel);
	SafeDelete(rulerToolPanel);
	SafeDelete(visibilityToolPanel);
	SafeDelete(tilemaskEditorPanel);
	SafeDelete(heightmapEditorPanel);
}

void LandscapeEditorControlsPlaceholder::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	
	setLayout(layout);
}

void LandscapeEditorControlsPlaceholder::ConnectToSignals()
{
	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));

	connect(SceneSignals::Instance(), SIGNAL(VisibilityToolToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(CustomColorsToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(HeightmapEditorToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(TilemaskEditorToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(RulerToolToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));
}

void LandscapeEditorControlsPlaceholder::SceneActivated(SceneEditor2* scene)
{
	activeScene = scene;

	UpdatePanels();
}

void LandscapeEditorControlsPlaceholder::SceneDeactivated(SceneEditor2* scene)
{
	RemovePanel();

	activeScene = NULL;
}

void LandscapeEditorControlsPlaceholder::SetPanel(LandscapeEditorBasePanel* panel)
{
	if (!panel || panel == currentPanel)
	{
		return;
	}
	
	RemovePanel();
	
	currentPanel = panel;
	layout()->addWidget(panel);
	panel->show();

	panel->InitPanel(activeScene);
}

void LandscapeEditorControlsPlaceholder::RemovePanel()
{
	if (!currentPanel)
	{
		return;
	}
	
	currentPanel->DeinitPanel();

	currentPanel->hide();
	layout()->removeWidget(currentPanel);
	currentPanel->setParent(NULL);
	currentPanel = NULL;
}

void LandscapeEditorControlsPlaceholder::EditorToggled(SceneEditor2* scene)
{
	if (scene != activeScene)
	{
		return;
	}

	UpdatePanels();
}

void LandscapeEditorControlsPlaceholder::UpdatePanels()
{
	RemovePanel();

	int32 tools = activeScene->GetEnabledTools();
	if (tools & SceneEditor2::LANDSCAPE_TOOL_CUSTOM_COLOR)
	{
		SetPanel(customColorsPanel);
	}
	else if (tools & SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
	{
		SetPanel(heightmapEditorPanel);
	}
	else if (tools & SceneEditor2::LANDSCAPE_TOOL_RULER)
	{
		SetPanel(rulerToolPanel);
	}
	else if (tools & SceneEditor2::LANDSCAPE_TOOL_TILEMAP_EDITOR)
	{
		SetPanel(tilemaskEditorPanel);
	}
	else if (tools & SceneEditor2::LANDSCAPE_TOOL_VISIBILITY)
	{
		SetPanel(visibilityToolPanel);
	}
}



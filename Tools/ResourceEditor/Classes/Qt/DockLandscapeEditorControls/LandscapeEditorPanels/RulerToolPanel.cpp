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


#include "RulerToolPanel.h"
#include "../../Scene/SceneEditor2.h"
#include "../../Scene/SceneSignals.h"
#include "../../Tools/SliderWidget/SliderWidget.h"
#include "../LandscapeEditorShortcutManager.h"
#include "Constants.h"

#include <QLayout>
#include <QFormLayout>
#include <QLabel>

RulerToolPanel::RulerToolPanel(QWidget* parent)
    : LandscapeEditorBasePanel(parent)
    , labelLength(nullptr)
    , labelPreview(nullptr)
{
	InitUI();
	ConnectToSignals();
}

RulerToolPanel::~RulerToolPanel()
{
}

bool RulerToolPanel::GetEditorEnabled()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	if (!sceneEditor)
	{
		return false;
	}

	return sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled();
}

void RulerToolPanel::SetWidgetsState(bool enabled)
{
	labelLength->setEnabled(enabled);
	labelPreview->setEnabled(enabled);
}

void RulerToolPanel::BlockAllSignals(bool block)
{
}

void RulerToolPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);

	labelLength = new QLabel(this);
	labelPreview = new QLabel(this);

	QLabel* labelLengthDesc = new QLabel(this);
	QLabel* labelPreviewDesc = new QLabel(this);
	QFrame* frame = new QFrame(this);
	QFormLayout* frameLayout = new QFormLayout(frame);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

	frameLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	frameLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	frameLayout->setContentsMargins(0, 0, 0, 0);
	frame->setLayout(frameLayout);
	frameLayout->addRow(labelLengthDesc, labelLength);
	frameLayout->addRow(labelPreviewDesc, labelPreview);

	layout->addWidget(frame);
	layout->addSpacerItem(spacer);

	setLayout(layout);

	SetWidgetsState(false);
	BlockAllSignals(true);

	labelLength->setNum(0);
	labelPreview->setNum(0);
	labelLengthDesc->setText(ResourceEditor::RULER_TOOL_LENGTH_CAPTION.c_str());
	labelPreviewDesc->setText(ResourceEditor::RULER_TOOL_PREVIEW_LENGTH_CAPTION.c_str());
}

void RulerToolPanel::ConnectToSignals()
{
	connect(SceneSignals::Instance(), SIGNAL(RulerToolToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(SceneSignals::Instance(), SIGNAL(RulerToolLengthChanged(SceneEditor2*, double, double)),
			this, SLOT(UpdateLengths(SceneEditor2*, double, double)));
}

void RulerToolPanel::StoreState()
{
}

void RulerToolPanel::RestoreState()
{
	SceneEditor2* sceneEditor = GetActiveScene();

	bool enabled = sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled();

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	UpdateLengths(sceneEditor,
				  sceneEditor->rulerToolSystem->GetLength(),
				  sceneEditor->rulerToolSystem->GetPreviewLength());
	BlockAllSignals(!enabled);
}


void RulerToolPanel::UpdateLengths(SceneEditor2 *scene, double length, double previewLength)
{
	SceneEditor2* sceneEditor = GetActiveScene();
	if (scene != sceneEditor)
	{
		return;
	}

	if (length < 0.0)
	{
		length = 0.0;
	}
	if (previewLength < 0.0)
	{
		previewLength = 0.0;
	}

	labelLength->setText(QString::number(length));
	labelPreview->setText(QString::number(previewLength));
}

void RulerToolPanel::ConnectToShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();
	shortcutManager->SetBrushSizeShortcutsEnabled(true);
}

void RulerToolPanel::DisconnectFromShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();
	shortcutManager->SetBrushSizeShortcutsEnabled(false);
}

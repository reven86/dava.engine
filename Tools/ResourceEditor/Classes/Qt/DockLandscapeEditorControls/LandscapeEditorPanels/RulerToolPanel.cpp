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
:	LandscapeEditorBasePanel(parent)
,	labelLength(NULL)
,	labelPreview(NULL)
,	sliderWidgetLineWidth(NULL)
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
	sliderWidgetLineWidth->setEnabled(enabled);
}

void RulerToolPanel::BlockAllSignals(bool block)
{
	sliderWidgetLineWidth->blockSignals(block);
}

void RulerToolPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);

	labelLength = new QLabel(this);
	labelPreview = new QLabel(this);
	sliderWidgetLineWidth = new SliderWidget(this);

	QLabel* labelLengthDesc = new QLabel(this);
	QLabel* labelPreviewDesc = new QLabel(this);
	QFrame* frame = new QFrame(this);
	QFormLayout* frameLayout = new QFormLayout(frame);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

	QHBoxLayout* layoutBrushSize = new QHBoxLayout();
	QLabel* labelBrushSize = new QLabel();
	labelBrushSize->setText(ResourceEditor::RULER_TOOL_LINE_WIDTH_CAPTION.c_str());
	layoutBrushSize->addWidget(labelBrushSize);
	layoutBrushSize->addWidget(sliderWidgetLineWidth);

	frameLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	frameLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	frameLayout->setContentsMargins(0, 0, 0, 0);
	frame->setLayout(frameLayout);
	frameLayout->addRow(labelLengthDesc, labelLength);
	frameLayout->addRow(labelPreviewDesc, labelPreview);

	layout->addWidget(frame);
	layout->addLayout(layoutBrushSize);
	layout->addSpacerItem(spacer);

	setLayout(layout);

	SetWidgetsState(false);
	BlockAllSignals(true);

	int32 lineWidthMin = 1;
	int32 lineWidthMax = 1;
	RenderHelper::Instance()->GetLineWidthRange(lineWidthMin, lineWidthMax);
	sliderWidgetLineWidth->Init(false, lineWidthMax, lineWidthMin, lineWidthMin);
	sliderWidgetLineWidth->SetRangeChangingBlocked(true);
	labelLength->setNum(0);
	labelPreview->setNum(0);
	labelLengthDesc->setText(ResourceEditor::RULER_TOOL_LENGTH_CAPTION.c_str());
	labelPreviewDesc->setText(ResourceEditor::RULER_TOOL_PREVIEW_LENGTH_CAPTION.c_str());
}

void RulerToolPanel::ConnectToSignals()
{
	connect(SceneSignals::Instance(), SIGNAL(RulerToolToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(sliderWidgetLineWidth, SIGNAL(ValueChanged(int)), this, SLOT(SetLineWidth(int)));
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
	int32 width = sceneEditor->rulerToolSystem->GetLineWidth();

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	sliderWidgetLineWidth->SetValue(width);
	UpdateLengths(sceneEditor,
				  sceneEditor->rulerToolSystem->GetLength(),
				  sceneEditor->rulerToolSystem->GetPreviewLength());
	BlockAllSignals(!enabled);
}

void RulerToolPanel::SetLineWidth(int width)
{
	SceneEditor2* sceneEditor = GetActiveScene();

	if (width > 0)
	{
		sceneEditor->rulerToolSystem->SetLineWidth(width);
	}
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

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSize()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSize()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSizeLarge()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSizeLarge()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(true);
}

void RulerToolPanel::DisconnectFromShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSize()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSize()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSizeLarge()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSizeLarge()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(false);
}

void RulerToolPanel::IncreaseBrushSize()
{
	sliderWidgetLineWidth->SetValue(sliderWidgetLineWidth->GetValue()
									+ ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void RulerToolPanel::DecreaseBrushSize()
{
	sliderWidgetLineWidth->SetValue(sliderWidgetLineWidth->GetValue()
									- ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void RulerToolPanel::IncreaseBrushSizeLarge()
{
	sliderWidgetLineWidth->SetValue(sliderWidgetLineWidth->GetValue()
									+ ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void RulerToolPanel::DecreaseBrushSizeLarge()
{
	sliderWidgetLineWidth->SetValue(sliderWidgetLineWidth->GetValue()
									- ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

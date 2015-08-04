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


#include "guidepropertygridwidget.h"
#include "ui_guidepropertygridwidget.h"

#include "WidgetSignalsBlocker.h"

#include "GuideCommands.h"
#include "CommandsController.h"

static const QString HORZ_GUIDE_PROPERTY_BLOCK_NAME = "Horizontal Guide";
static const QString VERT_GUIDE_PROPERTY_BLOCK_NAME = "Vertical Guide";

GuidePropertyGridWidget::GuidePropertyGridWidget(QWidget *parent) :
    RootPropertyGridWidget(parent),
    ui(new Ui::GuidePropertyGridWidget)
{
    ui->setupUi(this);
}

GuidePropertyGridWidget::~GuidePropertyGridWidget()
{
    delete ui;
}

void GuidePropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);

    GuideMetadata* metadata = GetGuideMetadata();
    if (!metadata || !metadata->GetActiveScreen())
    {
        return;
    }

    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    const GuideData* selectedGuide = metadata->GetActiveGuide();
    DVASSERT(selectedGuide);

    if (selectedGuide->GetType() == GuideData::Horizontal)
    {
        SetPropertyBlockName(HORZ_GUIDE_PROPERTY_BLOCK_NAME);
        ui->positionDoubleSpinBox->setValue(selectedGuide->GetPosition().y);
    }
    else
    {
        SetPropertyBlockName(VERT_GUIDE_PROPERTY_BLOCK_NAME);
        ui->positionDoubleSpinBox->setValue(selectedGuide->GetPosition().x);
    }

    connect(ui->positionDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnGuidePositionChanged(double)));

    connect(&metadata->GetActiveScreen()->GetGuidesManager(), SIGNAL(GuideMoved(GuideData*)), this, SLOT(OnGuideMoved(GuideData*)));
}

void GuidePropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    GuideMetadata* metadata = GetGuideMetadata();
    if (!metadata || !metadata->GetActiveScreen())
    {
        return;
    }

    disconnect(ui->positionDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnGuidePositionChanged(double)));
    disconnect(&metadata->GetActiveScreen()->GetGuidesManager(), SIGNAL(GuideMoved(GuideData*)), this, SLOT(OnGuideMoved(GuideData*)));
}

GuideMetadata* GuidePropertyGridWidget::GetGuideMetadata()
{
    return dynamic_cast<GuideMetadata*>(activeMetadata);
}

void GuidePropertyGridWidget::OnGuideMoved(GuideData* guideData)
{
    WidgetSignalsBlocker blocker(ui->positionDoubleSpinBox);
    ui->positionDoubleSpinBox->setValue(guideData->GetType() == GuideData::Horizontal ? guideData->GetPosition().y : guideData->GetPosition().x);
}

void GuidePropertyGridWidget::OnGuidePositionChanged(double value)
{
    GuideMetadata* metadata = GetGuideMetadata();
    if (!metadata || !metadata->GetActiveScreen())
    {
        return;
    }

    const GuideData* selectedGuide = metadata->GetActiveGuide();
    DVASSERT(selectedGuide);

    Vector2 delta;
    if (selectedGuide->GetType() == GuideData::Horizontal)
    {
        delta.y = value - selectedGuide->GetPosition().y;
    }
    else
    {
        delta.x = value - selectedGuide->GetPosition().x;
    }

    if (delta.IsZero())
    {
        // Nothing was changed.
        return;
    }
    
    MoveGuideCommand* cmd = new MoveGuideCommand(metadata->GetActiveScreen(), delta);
    CommandsController::Instance()->ExecuteCommand(cmd);
    SafeRelease(cmd);
}

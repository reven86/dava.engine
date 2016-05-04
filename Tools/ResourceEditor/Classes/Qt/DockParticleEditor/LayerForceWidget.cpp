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


#include "LayerForceWidget.h"
#include "TimeLineWidget.h"
#include "Commands2/ParticleEditorCommands.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QSizePolicy>

LayerForceWidget::LayerForceWidget(QWidget* parent)
    : BaseParticleEditorContentWidget(parent)
{
    mainBox = new QVBoxLayout;
    this->setLayout(mainBox);

    forceTimeLine = new TimeLineWidget(this);
    InitWidget(forceTimeLine);
    forceOverLifeTimeLine = new TimeLineWidget(this);
    InitWidget(forceOverLifeTimeLine);

    blockSignals = false;
}

LayerForceWidget::~LayerForceWidget()
{
}

void LayerForceWidget::InitWidget(QWidget* widget)
{
    mainBox->addWidget(widget);
    connect(widget,
            SIGNAL(ValueChanged()),
            this,
            SLOT(OnValueChanged()));
}

void LayerForceWidget::Init(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::uint32 forceIndex, bool updateMinimized)
{
    if (!layer || layer->forces.size() <= forceIndex)
    {
        return;
    }

    this->layer = layer;
    this->forceIndex = forceIndex;
    SetActiveScene(scene);

    blockSignals = true;

    DAVA::float32 lifeTime = layer->endTime;
    DAVA::ParticleForce* curForce = layer->forces[forceIndex];

    DAVA::Vector<QColor> colors;
    colors.push_back(Qt::red);
    colors.push_back(Qt::darkGreen);
    colors.push_back(Qt::blue);
    DAVA::Vector<QString> legends;
    legends.push_back("force x");
    legends.push_back("force y");
    legends.push_back("force z");
    forceTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
    forceTimeLine->AddLines(DAVA::PropLineWrapper<DAVA::Vector3>(DAVA::PropertyLineHelper::GetValueLine(curForce->force)).GetProps(), colors, legends);
    forceTimeLine->EnableLock(true);

    legends.clear();
    legends.push_back("force variable x");
    legends.push_back("force variable y");
    legends.push_back("force variable z");

    forceOverLifeTimeLine->Init(0, 1, updateMinimized, true, false);
    forceOverLifeTimeLine->AddLine(0, DAVA::PropLineWrapper<DAVA::float32>(DAVA::PropertyLineHelper::GetValueLine(curForce->forceOverLife)).GetProps(), Qt::blue, "forces over life");

    blockSignals = false;
}

void LayerForceWidget::RestoreVisualState(DAVA::KeyedArchive* visualStateProps)
{
    if (!visualStateProps)
        return;

    forceTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_PROPS"));
    forceOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_OVER_LIFE_PROPS"));
}

void LayerForceWidget::StoreVisualState(DAVA::KeyedArchive* visualStateProps)
{
    if (!visualStateProps)
        return;

    DAVA::KeyedArchive* props = new DAVA::KeyedArchive();

    forceTimeLine->GetVisualState(props);
    visualStateProps->SetArchive("FORCE_PROPS", props);

    props->DeleteAllKeys();
    forceOverLifeTimeLine->GetVisualState(props);
    visualStateProps->SetArchive("FORCE_OVER_LIFE_PROPS", props);

    DAVA::SafeRelease(props);
}

void LayerForceWidget::OnValueChanged()
{
    if (blockSignals)
        return;

    DAVA::PropLineWrapper<DAVA::Vector3> propForce;
    forceTimeLine->GetValues(propForce.GetPropsPtr());
    DAVA::PropLineWrapper<DAVA::float32> propForceOverLife;
    forceOverLifeTimeLine->GetValue(0, propForceOverLife.GetPropsPtr());

    std::unique_ptr<CommandUpdateParticleForce> updateForceCmd = Command2::Create<CommandUpdateParticleForce>(layer, forceIndex);
    updateForceCmd->Init(propForce.GetPropLine(), propForceOverLife.GetPropLine());

    DVASSERT(GetActiveScene() != nullptr);
    GetActiveScene()->Exec(std::move(updateForceCmd));
    GetActiveScene()->MarkAsChanged();

    Init(GetActiveScene(), layer, forceIndex, false);
    emit ValueChanged();
}

void LayerForceWidget::Update()
{
    Init(GetActiveScene(), layer, forceIndex, false);
}
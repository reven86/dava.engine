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



#include "ParticleEffectPropertiesWidget.h"
#include "Commands2/ParticleEditorCommands.h"
#include "../Scene/SceneDataManager.h"

#include <QLineEdit>
#include <QEvent>

ParticleEffectPropertiesWidget::ParticleEffectPropertiesWidget(QWidget* parent) :
	QWidget(parent),
	BaseParticleEditorContentWidget()
{
	mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignTop);
	this->setLayout(mainLayout);
	
	effectPlaybackSpeedLabel = new QLabel("effect playback speed");
	mainLayout->addWidget(effectPlaybackSpeedLabel);
	
	effectPlaybackSpeed = new QSlider(Qt::Horizontal, this);
	effectPlaybackSpeed->setTracking(true);
	effectPlaybackSpeed->setRange(0, 4); // 25%, 50%, 100%, 200%, 400% - 5 values total.
	effectPlaybackSpeed->setTickPosition(QSlider::TicksBelow);
	effectPlaybackSpeed->setTickInterval(1);
	effectPlaybackSpeed->setSingleStep(1);
	mainLayout->addWidget(effectPlaybackSpeed);

	checkboxStopOnLoad = new QCheckBox("Stop on load");
	mainLayout->addWidget(checkboxStopOnLoad);

	connect(effectPlaybackSpeed, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));
	connect(checkboxStopOnLoad, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));

	particleEffect = NULL;
	blockSignals = false;
}

ParticleEffectPropertiesWidget::~ParticleEffectPropertiesWidget()
{
}

void ParticleEffectPropertiesWidget::InitWidget(QWidget *widget, bool connectWidget)
{
	mainLayout->addWidget(widget);
	if(connectWidget)
		connect(widget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
}

void ParticleEffectPropertiesWidget::OnValueChanged()
{
	if(blockSignals)
		return;
	
	DVASSERT(particleEffect != 0);
	float playbackSpeed = ConvertFromSliderValueToPlaybackSpeed(effectPlaybackSpeed->value());
	bool stopOnLoad = checkboxStopOnLoad->isChecked();

	CommandUpdateEffect* commandUpdateEffect = new CommandUpdateEffect(particleEffect);
	commandUpdateEffect->Init(playbackSpeed, stopOnLoad);

	DVASSERT(activeScene != 0);
	activeScene->Exec(commandUpdateEffect);

	Init(activeScene, particleEffect);
}

void ParticleEffectPropertiesWidget::Init(SceneEditor2* scene, DAVA::ParticleEffectComponent *effect)
{
	DVASSERT(effect != 0);
	this->particleEffect = effect;
	this->emitter = NULL;
	SetActiveScene(scene);

	blockSignals = true;

	// Normalize Playback Speed to the UISlider range.
	float32 playbackSpeed = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeed->setValue(ConvertFromPlaybackSpeedToSliderValue(playbackSpeed));
	UpdatePlaybackSpeedLabel();
	
	checkboxStopOnLoad->setChecked(particleEffect->IsStopOnLoad());
	blockSignals = false;
}

void ParticleEffectPropertiesWidget::UpdatePlaybackSpeedLabel()
{
	if (!particleEffect)
	{
		return;
	}
	
	float32 playbackSpeedValue = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeedLabel->setText(QString("playback speed: %1x").arg(playbackSpeedValue));
}

void ParticleEffectPropertiesWidget::StoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to store for now.
}

void ParticleEffectPropertiesWidget::RestoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to restore for now.
}

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


#include "ParticleEditorWidget.h"
#include "EmitterLayerWidget.h"
#include "LayerForceWidget.h"
#include "ui_mainwindow.h"
#include <QScrollBar>

ParticleEditorWidget::ParticleEditorWidget(QWidget* parent /* = 0*/)
    : QScrollArea(parent)
{
    setWidgetResizable(true);

    emitterLayerWidget = NULL;
    layerForceWidget = NULL;
    emitterPropertiesWidget = NULL;
    effectPropertiesWidget = NULL;

    CreateInnerWidgets();

    // New signals for Scene Tree.
    connect(SceneSignals::Instance(),
            SIGNAL(EffectSelected(SceneEditor2*, DAVA::ParticleEffectComponent*)),
            this,
            SLOT(OnEffectSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleEffectComponent*)));
    connect(SceneSignals::Instance(),
            SIGNAL(EmitterSelected(SceneEditor2*, DAVA::ParticleEffectComponent*, DAVA::ParticleEmitter*)),
            this,
            SLOT(OnEmitterSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleEffectComponent*, DAVA::ParticleEmitter*)));
    connect(SceneSignals::Instance(),
            SIGNAL(InnerEmitterSelected(SceneEditor2*, DAVA::ParticleEffectComponent*, DAVA::ParticleEmitter*)),
            this,
            SLOT(OnInnerEmitterSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleEffectComponent*, DAVA::ParticleEmitter*)));
    connect(SceneSignals::Instance(),
            SIGNAL(LayerSelected(SceneEditor2*, DAVA::ParticleEffectComponent*, DAVA::ParticleEmitter*, DAVA::ParticleLayer*, bool)),
            this,
            SLOT(OnLayerSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleEffectComponent*, DAVA::ParticleEmitter*, DAVA::ParticleLayer*, bool)));
    connect(SceneSignals::Instance(),
            SIGNAL(ForceSelected(SceneEditor2*, DAVA::ParticleLayer*, int)),
            this,
            SLOT(OnForceSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleLayer*, int)));

    // Get the notification about changes in Particle Editor items.
    connect(SceneSignals::Instance(),
            SIGNAL(ParticleLayerValueChanged(SceneEditor2*, DAVA::ParticleLayer*)),
            this,
            SLOT(OnParticleLayerValueChanged(SceneEditor2*, DAVA::ParticleLayer*)));

    connect(SceneSignals::Instance(),
            SIGNAL(ParticleEmitterLoaded(SceneEditor2*, DAVA::ParticleEmitter*)),
            this,
            SLOT(OnParticleEmitterLoaded(SceneEditor2*, DAVA::ParticleEmitter*)));
    connect(SceneSignals::Instance(),
            SIGNAL(ParticleEmitterSaved(SceneEditor2*, DAVA::ParticleEmitter*)),
            this,
            SLOT(OnParticleEmitterSaved(SceneEditor2*, DAVA::ParticleEmitter*)));
}

ParticleEditorWidget::~ParticleEditorWidget()
{
    DeleteInnerWidgets();
}

void ParticleEditorWidget::CreateInnerWidgets()
{
    effectPropertiesWidget = new ParticleEffectPropertiesWidget(this);
    effectPropertiesWidget->hide();

    emitterPropertiesWidget = new ParticleEmitterPropertiesWidget(this);
    emitterPropertiesWidget->hide();

    emitterLayerWidget = new EmitterLayerWidget(this);
    emitterLayerWidget->hide();

    layerForceWidget = new LayerForceWidget(this);
    layerForceWidget->hide();

    widgetMode = MODE_NONE;
}

void ParticleEditorWidget::DeleteInnerWidgets()
{
    SAFE_DELETE(effectPropertiesWidget);
    SAFE_DELETE(emitterPropertiesWidget);
    SAFE_DELETE(emitterLayerWidget);
    SAFE_DELETE(layerForceWidget);
}

void ParticleEditorWidget::OnUpdate()
{
    switch (widgetMode)
    {
    case MODE_EMITTER:
    {
        emitterPropertiesWidget->Update();
        break;
    }

    case MODE_LAYER:
    {
        emitterLayerWidget->Update(false);
        break;
    }

    case MODE_FORCE:
    {
        layerForceWidget->Update();
        break;
    }

    default:
    {
        break;
    }
    }
}

void ParticleEditorWidget::OnValueChanged()
{
    // Update the particle editor widgets when the value on the emitter layer is changed.
    UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::UpdateParticleEditorWidgets()
{
    if (MODE_EMITTER == widgetMode && emitterPropertiesWidget->GetEmitter())
    {
        UpdateVisibleTimelinesForParticleEmitter();
        return;
    }

    if (MODE_LAYER == widgetMode && emitterLayerWidget->GetLayer())
    {
        UpdateWidgetsForLayer();
        return;
    }
}

void ParticleEditorWidget::UpdateVisibleTimelinesForParticleEmitter()
{
    // Safety check.
    if (MODE_EMITTER != widgetMode || !emitterPropertiesWidget->GetEmitter())
    {
        return;
    }

    // Update the visibility of particular timelines based on the emitter type.
    bool radiusTimeLineVisible = false;
    bool angleTimeLineVisible = false;
    bool sizeTimeLineVisible = false;

    switch (emitterPropertiesWidget->GetEmitter()->emitterType)
    {
    case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
    case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
    case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
    {
        radiusTimeLineVisible = true;
        angleTimeLineVisible = true;
        break;
    }

    case DAVA::ParticleEmitter::EMITTER_RECT:
    {
        sizeTimeLineVisible = true;
    }

    default:
    {
        break;
    }
    }

    emitterPropertiesWidget->GetEmitterRadiusTimeline()->setVisible(radiusTimeLineVisible);
    emitterPropertiesWidget->GetEmitterAngleTimeline()->setVisible(angleTimeLineVisible);
    emitterPropertiesWidget->GetEmitterSizeTimeline()->setVisible(sizeTimeLineVisible);
}

void ParticleEditorWidget::UpdateWidgetsForLayer()
{
    if (MODE_LAYER != widgetMode || !emitterLayerWidget->GetLayer())
    {
        return;
    }

    bool isSuperemitter = (emitterLayerWidget->GetLayer()->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES);
    emitterLayerWidget->SetSuperemitterMode(isSuperemitter);
}

void ParticleEditorWidget::OnEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter)
{
    HandleEmitterSelected(scene, effect, emitter, false);
}

void ParticleEditorWidget::OnInnerEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter)
{
    HandleEmitterSelected(scene, effect, emitter, false);
}

void ParticleEditorWidget::HandleEmitterSelected(SceneEditor2* scene, ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, bool forceUpdate)
{
    if (emitter &&
        MODE_EMITTER == widgetMode &&
        (!forceUpdate && (emitterPropertiesWidget->GetEmitter() == emitter)))
    {
        return;
    }

    SwitchEditorToEmitterMode(scene, effect, emitter);
}

void ParticleEditorWidget::OnEffectSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect)
{
    if (!effect)
    {
        return;
    }

    if (MODE_EFFECT == widgetMode && effectPropertiesWidget->GetEffect() == effect)
    {
        return;
    }

    SwitchEditorToEffectMode(scene, effect);
}

void ParticleEditorWidget::OnLayerSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer, bool forceRefresh)
{
    if (!forceRefresh && MODE_LAYER == widgetMode &&
        emitterLayerWidget->GetLayer() == layer &&
        emitterLayerWidget->GetEmitter() == emitter &&
        !forceRefresh)
    {
        return;
    }

    SwitchEditorToLayerMode(scene, effect, emitter, layer);
}

void ParticleEditorWidget::OnForceSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleLayer* layer, int forceIndex)
{
    if (MODE_FORCE == widgetMode &&
        layerForceWidget->GetLayer() == layer &&
        layerForceWidget->GetForceIndex() == forceIndex)
    {
        return;
    }

    SwitchEditorToForceMode(scene, layer, forceIndex);
}

void ParticleEditorWidget::OnParticleLayerValueChanged(SceneEditor2* /*scene*/, DAVA::ParticleLayer* layer)
{
    if (MODE_LAYER != widgetMode || emitterLayerWidget->GetLayer() != layer)
    {
        return;
    }

    // Notify the Emitter Layer widget about its inner layer value is changed and
    // the widget needs to be resynchronized with its values.
    emitterLayerWidget->OnLayerValueChanged();
}

void ParticleEditorWidget::OnParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
{
    // Handle in the same way emitter is selected to update the values. However
    // cause widget to be force updated.
    HandleEmitterSelected(scene, emitterPropertiesWidget->GetEffect(), emitter, true);
}

void ParticleEditorWidget::OnParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
{
    // Handle in the same way emitter is selected to update the values. However
    // cause widget to be force updated.
    ParticleEffectComponent* currEffect = emitterPropertiesWidget->GetEffect();
    ParticleEmitter* currEmitter = emitterPropertiesWidget->GetEmitter();
    if (currEffect && (currEmitter == emitter))
        HandleEmitterSelected(scene, currEffect, emitter, true);
}

void ParticleEditorWidget::SwitchEditorToEffectMode(SceneEditor2* scene, ParticleEffectComponent* effect)
{
    ResetEditorMode();

    if (!effect)
    {
        emit ChangeVisible(false);
        return;
    }

    emit ChangeVisible(true);

    effectPropertiesWidget->Init(scene, effect);
    setWidget(effectPropertiesWidget);
    effectPropertiesWidget->show();

    this->widgetMode = MODE_EFFECT;
}

void ParticleEditorWidget::SwitchEditorToEmitterMode(SceneEditor2* scene, ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter)
{
    ResetEditorMode();

    if (!emitter)
    {
        emit ChangeVisible(false);
        return;
    }

    emit ChangeVisible(true);
    this->widgetMode = MODE_EMITTER;

    emitterPropertiesWidget->Init(scene, effect, emitter, true);
    setWidget(emitterPropertiesWidget);
    emitterPropertiesWidget->show();

    connect(emitterPropertiesWidget,
            SIGNAL(ValueChanged()),
            this,
            SLOT(OnValueChanged()));

    UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::SwitchEditorToLayerMode(SceneEditor2* scene, ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer)
{
    ResetEditorMode();

    if (!emitter || !layer)
    {
        emit ChangeVisible(false);
        return;
    }

    emit ChangeVisible(true);
    this->widgetMode = MODE_LAYER;

    emitterLayerWidget->Init(scene, effect, emitter, layer, true);
    setWidget(emitterLayerWidget);
    emitterLayerWidget->show();

    connect(emitterLayerWidget,
            SIGNAL(ValueChanged()),
            this,
            SLOT(OnValueChanged()));

    UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::SwitchEditorToForceMode(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::int32 forceIndex)
{
    ResetEditorMode();

    if (!layer)
    {
        emit ChangeVisible(false);
        return;
    }

    emit ChangeVisible(true);
    widgetMode = MODE_FORCE;

    layerForceWidget->Init(scene, layer, forceIndex, true);
    setWidget(layerForceWidget);
    layerForceWidget->show();
    connect(layerForceWidget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));

    UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::ResetEditorMode()
{
    QWidget* activeEditorWidget = takeWidget();
    if (!activeEditorWidget)
    {
        DVASSERT(widgetMode == MODE_NONE);
        return;
    }

    switch (widgetMode)
    {
    case MODE_EFFECT:
    {
        effectPropertiesWidget = static_cast<ParticleEffectPropertiesWidget*>(activeEditorWidget);
        effectPropertiesWidget->hide();

        break;
    }

    case MODE_EMITTER:
    {
        emitterPropertiesWidget = static_cast<ParticleEmitterPropertiesWidget*>(activeEditorWidget);
        disconnect(emitterPropertiesWidget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
        emitterPropertiesWidget->hide();

        break;
    }

    case MODE_LAYER:
    {
        emitterLayerWidget = static_cast<EmitterLayerWidget*>(activeEditorWidget);
        disconnect(emitterLayerWidget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
        emitterLayerWidget->hide();

        break;
    }

    case MODE_FORCE:
    {
        layerForceWidget = static_cast<LayerForceWidget*>(activeEditorWidget);
        disconnect(layerForceWidget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
        layerForceWidget->hide();

        break;
    }

    default:
    {
        break;
    }
    }

    widgetMode = MODE_NONE;
}

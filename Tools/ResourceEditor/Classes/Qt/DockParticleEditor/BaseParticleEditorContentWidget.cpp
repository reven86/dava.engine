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

#include "BaseParticleEditorContentWidget.h"
#include "Scene/SceneSignals.h"

const QChar BaseParticleEditorContentWidget::DEGREE_MARK_CHARACTER = QChar(0x00B0);
#define PARTICLE_EMITTER_MIN_PLAYBACK_SPEED 0.25f
#define PARTICLE_EMITTER_MAX_PLAYBACK_SPEED 4.0f

BaseParticleEditorContentWidget::BaseParticleEditorContentWidget(QWidget* parent)
    : QWidget(parent)
{
    SceneSignals* dispatcher = SceneSignals::Instance();
    connect(dispatcher, &SceneSignals::Closed, this, &BaseParticleEditorContentWidget::OnSceneClosed);
    connect(dispatcher, &SceneSignals::Activated, this, &BaseParticleEditorContentWidget::OnSceneActivated);
}

void BaseParticleEditorContentWidget::OnSceneActivated(SceneEditor2* editor)
{
    activeScene = editor;
}

void BaseParticleEditorContentWidget::OnSceneClosed(SceneEditor2* editor)
{
    if (editor == activeScene)
        activeScene = nullptr;

    objectsForScene.erase(editor);
}

int BaseParticleEditorContentWidget::ConvertFromPlaybackSpeedToSliderValue(DAVA::float32 playbackSpeed)
{
    playbackSpeed = DAVA::Clamp(playbackSpeed, PARTICLE_EMITTER_MIN_PLAYBACK_SPEED, PARTICLE_EMITTER_MAX_PLAYBACK_SPEED);

    // Our scale is logarithmic.
    uint playbackSpeedInt = (uint)(playbackSpeed / PARTICLE_EMITTER_MIN_PLAYBACK_SPEED);
    uint logValue = -1;
    while (playbackSpeedInt)
    {
        logValue++;
        playbackSpeedInt >>= 1;
    }

    return logValue;
}

float BaseParticleEditorContentWidget::ConvertFromSliderValueToPlaybackSpeed(int sliderValue)
{
    // Our scale is logarithmic.
    uint scaleFactor = (1 << sliderValue);
    return PARTICLE_EMITTER_MIN_PLAYBACK_SPEED * scaleFactor;
}

BaseParticleEditorContentWidget::Objects BaseParticleEditorContentWidget::GetCurrentObjectsForScene(SceneEditor2* scene) const
{
    auto i = objectsForScene.find(scene);
    return (i == objectsForScene.end()) ? emptyObjects : i->second;
}

void BaseParticleEditorContentWidget::SetObjectsForScene(SceneEditor2* s, DAVA::ParticleEffectComponent* e, DAVA::ParticleEmitterInstance* i)
{
    objectsForScene[s].effect = e;
    objectsForScene[s].instance = i;
}

SceneEditor2* BaseParticleEditorContentWidget::GetActiveScene() const
{
    return activeScene;
}

DAVA::ParticleEffectComponent* BaseParticleEditorContentWidget::GetEffect(SceneEditor2* scene)
{
    return objectsForScene[scene].effect;
}

DAVA::ParticleEmitterInstance* BaseParticleEditorContentWidget::GetEmitterInstance(SceneEditor2* scene)
{
    return objectsForScene[scene].instance.Get();
}

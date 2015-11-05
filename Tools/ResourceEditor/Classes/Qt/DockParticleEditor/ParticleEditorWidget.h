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


#ifndef __ResourceEditorQt__ParticleEditorWidget__
#define __ResourceEditorQt__ParticleEditorWidget__

#include <QScrollArea>

#include <DAVAEngine.h>
#include "ParticleEffectPropertiesWidget.h"
#include "ParticleEmitterPropertiesWidget.h"

#include "Scene/SceneEditor2.h"

class EmitterLayerWidget;
class LayerForceWidget;

using namespace DAVA;

class ParticleEditorWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit ParticleEditorWidget(QWidget* parent = 0);
    ~ParticleEditorWidget();

protected slots:
    // SceneTree-specific slots.
    void OnEffectSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect);
    void OnEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void OnInnerEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void OnLayerSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer, bool forceRefresh);
    void OnForceSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleLayer* layer, int forceIndex);

    void OnUpdate();
    void OnValueChanged();

    // Notifications about changes in the Particles items.
    void OnParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);
    void OnParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);
    void OnParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);

signals:
    void ChangeVisible(bool bVisible);

private:
    enum ParticleEditorWidgetMode
    {
        MODE_NONE = 0,
        MODE_EFFECT,
        MODE_EMITTER,
        MODE_LAYER,
        MODE_FORCE
    };

    void DeleteOldWidget();
    void UpdateParticleEditorWidgets();

    // Handle the "Emitter Selected" notification for different cases.
    void HandleEmitterSelected(SceneEditor2* scene, ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, bool forceUpdate);

    // Update the visible timelines for the particular Particle Emitter elements.
    void UpdateVisibleTimelinesForParticleEmitter();

    // Update visible widgets for the layer.
    void UpdateWidgetsForLayer();

    // Emit the "Value Changed" signal depending on the active widget.
    void EmitValueChangedSceneSignal();

    // Create/delete Inner Widgets. Note - they are created once only.
    void CreateInnerWidgets();
    void DeleteInnerWidgets();

    // Switch editor to the particular mode.
    void SwitchEditorToEffectMode(SceneEditor2* scene, ParticleEffectComponent* effect);
    void SwitchEditorToEmitterMode(SceneEditor2* scene, ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void SwitchEditorToLayerMode(SceneEditor2* scene, ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    void SwitchEditorToForceMode(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::int32 forceIndex);

    // Reset the editor mode, hide/disconnect appropriate widgets.
    void ResetEditorMode();

private:
    // Current Particle Editor Widget mode.
    ParticleEditorWidgetMode widgetMode;

    // Inner widgets.
    ParticleEffectPropertiesWidget* effectPropertiesWidget;
    ParticleEmitterPropertiesWidget* emitterPropertiesWidget;
    EmitterLayerWidget* emitterLayerWidget;
    LayerForceWidget* layerForceWidget;
};

#endif /* defined(__ResourceEditorQt__ParticleEditorWidget__) */

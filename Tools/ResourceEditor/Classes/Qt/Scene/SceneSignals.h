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


#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"

#include "Scene/System/VisibilityToolSystem.h"

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditor2;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
    Q_OBJECT

signals:
    // scene
    void Opened(SceneEditor2* scene);
    void Closed(SceneEditor2* scene);

    void Loaded(SceneEditor2* scene);
    void Saved(SceneEditor2* scene);

    void Activated(SceneEditor2* scene);
    void Deactivated(SceneEditor2* scene);

    void CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);
    void StructureChanged(SceneEditor2* scene, DAVA::Entity* parent);
    void ModifyStatusChanged(SceneEditor2* scene, bool modified);

    // entities
    void SelectionChanged(SceneEditor2* scene, const EntityGroup* selected, const EntityGroup* deselected);

    void SolidChanged(SceneEditor2* scene, const DAVA::Entity* entity, bool value);
    // mouse
    void MouseOver(SceneEditor2* scene, const EntityGroup* entities);
    void MouseOverSelection(SceneEditor2* scene, const EntityGroup* entities);

    // particles - selection
    void EffectSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect);
    void EmitterSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void InnerEmitterSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void LayerSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer, bool forceRefresh);
    void ForceSelected(SceneEditor2* scene, DAVA::ParticleLayer* layer, int forceIndex);

    // particles - value changed
    void ParticleEmitterValueChanged(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);
    void ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);
    void ParticleForceValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer, int32 forceIndex);

    // particles - effect started/stopped.
    void ParticleEffectStateChanged(SceneEditor2* scene, DAVA::Entity* effect, bool isStarted);

    // particles - loading/saving.
    void ParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);
    void ParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);

    // particles - structure changes.
    void ParticleLayerAdded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    void ParticleLayerRemoved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);

    void DropperHeightChanged(SceneEditor2* scene, double height);
    void VisibilityToolStateChanged(SceneEditor2* scene, VisibilityToolSystem::eVisibilityToolState state);
    void CustomColorsTextureShouldBeSaved(SceneEditor2* scene);
    void RulerToolLengthChanged(SceneEditor2* scene, double length, double previewLength);
    void SnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);

    void VisibilityToolToggled(SceneEditor2* scene);
    void CustomColorsToggled(SceneEditor2* scene);
    void HeightmapEditorToggled(SceneEditor2* scene);
    void TilemaskEditorToggled(SceneEditor2* scene);
    void RulerToolToggled(SceneEditor2* scene);
    void NotPassableTerrainToggled(SceneEditor2* scene);

    void EditorLightEnabled(bool enabled);

public:
    void EmitOpened(SceneEditor2* scene)
    {
        emit Opened(scene);
    }
    void EmitClosed(SceneEditor2* scene)
    {
        emit Closed(scene);
    }

    void EmitLoaded(SceneEditor2* scene)
    {
        emit Loaded(scene);
    }
    void EmitSaved(SceneEditor2* scene)
    {
        emit Saved(scene);
    }

    void EmitActivated(SceneEditor2* scene)
    {
        emit Activated(scene);
    }
    void EmitDeactivated(SceneEditor2* scene)
    {
        emit Deactivated(scene);
    }

    void EmitCommandExecuted(SceneEditor2* scene, const Command2* command, bool redo)
    {
        emit CommandExecuted(scene, command, redo);
    };
    void EmitStructureChanged(SceneEditor2* scene, DAVA::Entity* parent)
    {
        emit StructureChanged(scene, parent);
    }

    void EmitSelectionChanged(SceneEditor2* scene, const EntityGroup* selected, const EntityGroup* deselected)
    {
        emit SelectionChanged(scene, selected, deselected);
    }
    void EmitSolidChanged(SceneEditor2* scene, const Entity* entity, bool value)
    {
        emit SolidChanged(scene, entity, value);
    }

    void EmitModifyStatusChanged(SceneEditor2* scene, bool modified)
    {
        emit ModifyStatusChanged(scene, modified);
    }

    void EmitVisibilityToolToggled(SceneEditor2* scene)
    {
        emit VisibilityToolToggled(scene);
    }
    void EmitCustomColorsToggled(SceneEditor2* scene)
    {
        emit CustomColorsToggled(scene);
    }
    void EmitHeightmapEditorToggled(SceneEditor2* scene)
    {
        emit HeightmapEditorToggled(scene);
    }
    void EmitTilemaskEditorToggled(SceneEditor2* scene)
    {
        emit TilemaskEditorToggled(scene);
    }
    void EmitRulerToolToggled(SceneEditor2* scene)
    {
        emit RulerToolToggled(scene);
    }
    void EmitNotPassableTerrainToggled(SceneEditor2* scene)
    {
        emit NotPassableTerrainToggled(scene);
    }

    void EmitDropperHeightChanged(SceneEditor2* scene, DAVA::float32 height)
    {
        emit DropperHeightChanged(scene, (double)height);
    };
    void EmitVisibilityToolStateChanged(SceneEditor2* scene, VisibilityToolSystem::eVisibilityToolState state)
    {
        emit VisibilityToolStateChanged(scene, state);
    };
    void EmitCustomColorsTextureShouldBeSaved(SceneEditor2* scene)
    {
        emit CustomColorsTextureShouldBeSaved(scene);
    };
    void EmitRulerToolLengthChanged(SceneEditor2* scene, double length, double previewLength)
    {
        emit RulerToolLengthChanged(scene, length, previewLength);
    }

    void EmitMouseOver(SceneEditor2* scene, const EntityGroup* entities)
    {
        emit MouseOver(scene, entities);
    }
    void EmitMouseOverSelection(SceneEditor2* scene, const EntityGroup* entities)
    {
        emit MouseOverSelection(scene, entities);
    }

    // Particle Editor Selection signals.
    void EmitEffectSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect)
    {
        emit EffectSelected(scene, effect);
    };
    void EmitEmitterSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter)
    {
        emit EmitterSelected(scene, effect, emitter);
    };
    void EmitInnerEmitterSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter)
    {
        emit InnerEmitterSelected(scene, effect, emitter);
    };
    void EmitLayerSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer, bool forceRefresh)
    {
        emit LayerSelected(scene, effect, emitter, layer, forceRefresh);
    };
    void EmitForceSelected(SceneEditor2* scene, DAVA::ParticleLayer* layer, int forceIndex)
    {
        emit ForceSelected(scene, layer, forceIndex);
    };

    // Particle Editor Value Changed signals.
    void EmitParticleEmitterValueChanged(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
    {
        emit ParticleEmitterValueChanged(scene, emitter);
    }

    void EmitParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer)
    {
        emit ParticleLayerValueChanged(scene, layer);
    }

    void EmitParticleForceValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer, int32 forceIndex)
    {
        emit ParticleForceValueChanged(scene, layer, forceIndex);
    }

    void EmitParticleEffectStateChanged(SceneEditor2* scene, DAVA::Entity* effect, bool isStarted)
    {
        emit ParticleEffectStateChanged(scene, effect, isStarted);
    }

    void EmitParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
    {
        emit ParticleEmitterLoaded(scene, emitter);
    }

    void EmitParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
    {
        emit ParticleEmitterSaved(scene, emitter);
    }

    void EmitParticleLayerAdded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer)
    {
        emit ParticleLayerAdded(scene, emitter, layer);
    }

    void EmitParticleLayerRemoved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
    {
        emit ParticleLayerRemoved(scene, emitter);
    }

    void EmitEditorLightEnabled(bool enabled)
    {
        emit EditorLightEnabled(enabled);
    }

    void EmitSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape)
    {
        emit SnapToLandscapeChanged(scene, isSpanToLandscape);
    }
};

#endif // __SCENE_MANAGER_H__

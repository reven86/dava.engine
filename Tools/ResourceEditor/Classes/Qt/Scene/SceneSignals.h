#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "Scene/SelectableGroup.h"
#include "Scene/SceneEditor2.h"
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditor2;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
    Q_OBJECT

signals:
    // scene
    void Opened(SceneEditor2* scene);
    void Loaded(SceneEditor2* scene);
    void Updated(SceneEditor2* scene);
    void Saved(SceneEditor2* scene);
    void Closed(SceneEditor2* scene);

    void Activated(SceneEditor2* scene);
    void Deactivated(SceneEditor2* scene);

    void CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);
    void StructureChanged(SceneEditor2* scene, DAVA::Entity* parent);
    void ModifyStatusChanged(SceneEditor2* scene, bool modified);

    // entities
    void SelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);

    void SolidChanged(SceneEditor2* scene, const DAVA::Entity* entity, bool value);
    // mouse
    void MouseOver(SceneEditor2* scene, const SelectableGroup* objects);
    void MouseOverSelection(SceneEditor2* scene, const SelectableGroup* objects);

    // particles - value changed
    void ParticleEmitterValueChanged(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter);
    void ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);
    void ParticleForceValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::int32 forceIndex);

    // particles - effect started/stopped.
    void ParticleEffectStateChanged(SceneEditor2* scene, DAVA::Entity* effect, bool isStarted);

    // particles - loading/saving.
    void ParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter);
    void ParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter);

    // particles - structure changes.
    void ParticleLayerAdded(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter, DAVA::ParticleLayer* layer);
    void ParticleLayerRemoved(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter);

    void DropperHeightChanged(SceneEditor2* scene, double height);
    void CustomColorsTextureShouldBeSaved(SceneEditor2* scene);
    void RulerToolLengthChanged(SceneEditor2* scene, double length, double previewLength);
    void SnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);

    void LandscapeEditorToggled(SceneEditor2* scene);

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
    void EmitUpdated(SceneEditor2* scene)
    {
        emit Updated(scene);
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

    void EmitSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
    {
        emit SelectionChanged(scene, selected, deselected);
    }
    void EmitSolidChanged(SceneEditor2* scene, const DAVA::Entity* entity, bool value)
    {
        emit SolidChanged(scene, entity, value);
    }

    void EmitModifyStatusChanged(SceneEditor2* scene, bool modified)
    {
        emit ModifyStatusChanged(scene, modified);
    }

    void EmitLandscapeEditorToggled(SceneEditor2* scene)
    {
        emit LandscapeEditorToggled(scene);
    }

    void EmitDropperHeightChanged(SceneEditor2* scene, DAVA::float32 height)
    {
        emit DropperHeightChanged(scene, (double)height);
    };

    void EmitCustomColorsTextureShouldBeSaved(SceneEditor2* scene)
    {
        emit CustomColorsTextureShouldBeSaved(scene);
    };
    void EmitRulerToolLengthChanged(SceneEditor2* scene, double length, double previewLength)
    {
        emit RulerToolLengthChanged(scene, length, previewLength);
    }

    void EmitMouseOver(SceneEditor2* scene, const SelectableGroup* objects)
    {
        emit MouseOver(scene, objects);
    }
    void EmitMouseOverSelection(SceneEditor2* scene, const SelectableGroup* objects)
    {
        emit MouseOverSelection(scene, objects);
    }

    // Particle Editor Value Changed signals.
    void EmitParticleEmitterValueChanged(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter)
    {
        emit ParticleEmitterValueChanged(scene, emitter);
    }

    void EmitParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer)
    {
        emit ParticleLayerValueChanged(scene, layer);
    }

    void EmitParticleForceValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::int32 forceIndex)
    {
        emit ParticleForceValueChanged(scene, layer, forceIndex);
    }

    void EmitParticleEffectStateChanged(SceneEditor2* scene, DAVA::Entity* effect, bool isStarted)
    {
        emit ParticleEffectStateChanged(scene, effect, isStarted);
    }

    void EmitParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter)
    {
        emit ParticleEmitterLoaded(scene, emitter);
    }

    void EmitParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter)
    {
        emit ParticleEmitterSaved(scene, emitter);
    }

    void EmitParticleLayerAdded(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter, DAVA::ParticleLayer* layer)
    {
        emit ParticleLayerAdded(scene, emitter, layer);
    }

    void EmitParticleLayerRemoved(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter)
    {
        emit ParticleLayerRemoved(scene, emitter);
    }

    void EmitSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape)
    {
        emit SnapToLandscapeChanged(scene, isSpanToLandscape);
    }
};

#endif // __SCENE_MANAGER_H__

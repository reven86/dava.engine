#ifndef __ResourceEditorQt__ParticleEditorWidget__
#define __ResourceEditorQt__ParticleEditorWidget__

#include <QScrollArea>

#include <DAVAEngine.h>
#include "ParticleEffectPropertiesWidget.h"
#include "ParticleEmitterPropertiesWidget.h"

#include "Scene/SceneEditor2.h"

class EmitterLayerWidget;
class LayerForceWidget;

class ParticleEditorWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit ParticleEditorWidget(QWidget* parent = 0);
    ~ParticleEditorWidget();

protected slots:
    // SceneTree-specific slots.
    void OnSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);

    void OnUpdate();
    void OnValueChanged();

    // Notifications about changes in the Particles items.
    void OnParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);
    void OnParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter);
    void OnParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitterInstance* emitter);

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

    void UpdateParticleEditorWidgets();

    // Handle the "Emitter Selected" notification for different cases.
    void HandleEmitterSelected(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* emitter, bool forceUpdate);

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
    void SwitchEditorToEffectMode(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect);
    void SwitchEditorToEmitterMode(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* emitter);
    void SwitchEditorToLayerMode(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* emitter, DAVA::ParticleLayer* layer);
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

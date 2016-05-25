#ifndef __ResourceEditorQt__EmitterLayerWidget__
#define __ResourceEditorQt__EmitterLayerWidget__

#include <DAVAEngine.h>
#include "TimeLineWidget.h"
#include "GradientPickerWidget.h"
#include "BaseParticleEditorContentWidget.h"
#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"
#include "Scene3D/Lod/LodComponent.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTimer>

class EmitterLayerWidget : public BaseParticleEditorContentWidget
{
    Q_OBJECT

public:
    explicit EmitterLayerWidget(QWidget* parent = 0);

    void Init(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* emitter,
              DAVA::ParticleLayer* layer, bool updateMinimized);

    DAVA::ParticleLayer* GetLayer() const
    {
        return layer;
    };
    void Update(bool updateMinimized);

    virtual bool eventFilter(QObject*, QEvent*);

    void StoreVisualState(DAVA::KeyedArchive* visualStateProps) override;
    void RestoreVisualState(DAVA::KeyedArchive* visualStateProps) override;

    // Switch from/to SuperEmitter mode.
    void SetSuperemitterMode(bool isSuperemitter);

    // Notify yhe widget layer value is changed.
    void OnLayerValueChanged();

signals:
    void ValueChanged();

protected slots:
    void OnLodsChanged();
    void OnValueChanged();
    void OnLayerMaterialValueChanged();
    void OnSpriteBtn();
    void OnSpriteFolderBtn();
    void OnSpritePathChanged(const QString& text);
    void OnSpritePathEdited(const QString& text);

    void OnPivotPointReset();
    void OnSpriteUpdateTimerExpired();

private:
    void InitWidget(QWidget*);
    void UpdateTooltip();
    void UpdateLayerSprite();

    void FillLayerTypes();
    DAVA::int32 LayerTypeToIndex(DAVA::ParticleLayer::eType layerType);

private:
    struct LayerTypeMap
    {
        DAVA::ParticleLayer::eType layerType;
        QString layerName;
    };

    struct BlendPreset
    {
        DAVA::eBlending blending;
        QString presetName;
    };

private:
    static const LayerTypeMap layerTypeMap[];
    static const BlendPreset blendPresetsMap[];

    DAVA::ParticleLayer* layer = nullptr;

    QTimer* spriteUpdateTimer = nullptr;
    DAVA::Stack<std::pair<rhi::HSyncObject, DAVA::Texture*>> spriteUpdateTexturesStack;

    QVBoxLayout* mainBox = nullptr;
    QVBoxLayout* pivotPointLayout = nullptr;

    QLabel* scaleVelocityBaseLabel = nullptr;
    QLabel* scaleVelocityFactorLabel = nullptr;
    QLabel* layerTypeLabel = nullptr;
    QLabel* spriteLabel = nullptr;
    QLabel* innerEmitterLabel = nullptr;
    QLabel* pivotPointLabel = nullptr;
    QLabel* pivotPointXSpinBoxLabel = nullptr;
    QLabel* pivotPointYSpinBoxLabel = nullptr;
    QLabel* particleOrientationLabel = nullptr;
    QLabel* blendOptionsLabel = nullptr;
    QLabel* presetLabel = nullptr;
    QLabel* frameOverlifeFPSLabel = nullptr;
    QLabel* deltaSpinLabel = nullptr;
    QLabel* deltaVariationSpinLabel = nullptr;
    QLabel* loopEndSpinLabel = nullptr;
    QLabel* loopVariationSpinLabel = nullptr;

    QCheckBox* enableCheckBox = nullptr;
    QCheckBox* isLongCheckBox = nullptr;
    QCheckBox* isLoopedCheckBox = nullptr;
    QCheckBox* inheritPostionCheckBox = nullptr;
    QCheckBox* layerLodsCheckBox[DAVA::LodComponent::MAX_LOD_LAYERS];
    QCheckBox* frameBlendingCheckBox = nullptr;
    QCheckBox* cameraFacingCheckBox = nullptr;
    QCheckBox* xFacingCheckBox = nullptr;
    QCheckBox* yFacingCheckBox = nullptr;
    QCheckBox* zFacingCheckBox = nullptr;
    QCheckBox* worldAlignCheckBox = nullptr;
    QCheckBox* fogCheckBox = nullptr;
    QCheckBox* frameOverlifeCheckBox = nullptr;
    QCheckBox* randomSpinDirectionCheckBox = nullptr;
    QCheckBox* randomFrameOnStartCheckBox = nullptr;
    QCheckBox* loopSpriteAnimationCheckBox = nullptr;

    QComboBox* degradeStrategyComboBox = nullptr;
    QComboBox* layerTypeComboBox = nullptr;
    QComboBox* presetComboBox = nullptr;

    QLineEdit* layerNameLineEdit = nullptr;
    QLineEdit* spritePathLabel = nullptr;
    QLineEdit* innerEmitterPathLabel = nullptr;

    QPushButton* spriteBtn = nullptr;
    QPushButton* spriteFolderBtn = nullptr;
    QPushButton* pivotPointResetButton = nullptr;

    TimeLineWidget* lifeTimeLine = nullptr;
    TimeLineWidget* numberTimeLine = nullptr;
    TimeLineWidget* sizeTimeLine = nullptr;
    TimeLineWidget* sizeVariationTimeLine = nullptr;
    TimeLineWidget* sizeOverLifeTimeLine = nullptr;
    TimeLineWidget* velocityTimeLine = nullptr;
    TimeLineWidget* velocityOverLifeTimeLine = nullptr;
    TimeLineWidget* spinTimeLine = nullptr;
    TimeLineWidget* spinOverLifeTimeLine = nullptr;
    TimeLineWidget* alphaOverLifeTimeLine = nullptr;
    TimeLineWidget* animSpeedOverLifeTimeLine = nullptr;
    TimeLineWidget* angleTimeLine = nullptr;

    EventFilterDoubleSpinBox* scaleVelocityBaseSpinBox;
    EventFilterDoubleSpinBox* scaleVelocityFactorSpinBox;
    EventFilterDoubleSpinBox* pivotPointXSpinBox = nullptr;
    EventFilterDoubleSpinBox* pivotPointYSpinBox = nullptr;
    EventFilterDoubleSpinBox* startTimeSpin = nullptr;
    EventFilterDoubleSpinBox* endTimeSpin = nullptr;
    EventFilterDoubleSpinBox* deltaSpin = nullptr;
    EventFilterDoubleSpinBox* loopEndSpin = nullptr;
    EventFilterDoubleSpinBox* deltaVariationSpin = nullptr;
    EventFilterDoubleSpinBox* loopVariationSpin = nullptr;

    QSpinBox* frameOverlifeFPSSpin = nullptr;

    GradientPickerWidget* colorRandomGradient = nullptr;
    GradientPickerWidget* colorOverLifeGradient = nullptr;

    bool blockSignals = false;
};

#endif /* defined(__ResourceEditorQt__EmitterLayerWidget__) */

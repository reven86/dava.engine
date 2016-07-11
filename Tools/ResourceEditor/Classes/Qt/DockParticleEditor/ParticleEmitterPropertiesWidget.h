#ifndef __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__
#define __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>
#include "GradientPickerWidget.h"
#include "TimeLineWidget.h"
#include "BaseParticleEditorContentWidget.h"
#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"

#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>

class ParticleEmitterPropertiesWidget : public BaseParticleEditorContentWidget
{
    Q_OBJECT

public:
    explicit ParticleEmitterPropertiesWidget(QWidget* parent = nullptr);

    void Init(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* emitter,
              bool updateMinimize, bool needUpdateTimeLimits = true);
    void Update();

    bool eventFilter(QObject* o, QEvent* e) override;

    void StoreVisualState(DAVA::KeyedArchive* visualStateProps) override;
    void RestoreVisualState(DAVA::KeyedArchive* visualStateProps) override;

    // Accessors to timelines.
    TimeLineWidget* GetEmitterRadiusTimeline()
    {
        return emitterRadius;
    };
    TimeLineWidget* GetEmitterAngleTimeline()
    {
        return emitterAngle;
    };
    TimeLineWidget* GetEmitterSizeTimeline()
    {
        return emitterSize;
    };
    TimeLineWidget* GetEmissionVectorTimeline()
    {
        return emitterEmissionVector;
    };

signals:
    void ValueChanged();

public slots:
    void OnValueChanged();
    void OnEmitterYamlPathChanged(const QString& newPath);
    void OnEmitterPositionChanged();
    void OnCommand(SceneEditor2* scene, const DAVA::Command* command, bool redo);

protected:
    void UpdateProperties();
    void UpdateTooltip();

private:
    QVBoxLayout* mainLayout = nullptr;
    QLineEdit* emitterNameLineEdit = nullptr;
    QLineEdit* originalEmitterYamlPath = nullptr;
    QLineEdit* emitterYamlPath = nullptr;
    QComboBox* emitterType = nullptr;
    EventFilterDoubleSpinBox* positionXSpinBox = nullptr;
    EventFilterDoubleSpinBox* positionYSpinBox = nullptr;
    EventFilterDoubleSpinBox* positionZSpinBox = nullptr;
    QCheckBox* shortEffectCheckBox = nullptr;

    TimeLineWidget* emitterEmissionRange = nullptr;
    TimeLineWidget* emitterEmissionVector = nullptr;
    TimeLineWidget* emitterRadius = nullptr;
    TimeLineWidget* emitterSize = nullptr;
    TimeLineWidget* emitterAngle = nullptr;
    EventFilterDoubleSpinBox* emitterLife = nullptr;
    GradientPickerWidget* emitterColorWidget = nullptr;

    bool blockSignals = false;
    bool updateMinimize = false;
    bool needUpdateTimeLimits = false;

    void InitWidget(QWidget* widget, bool connectWidget = true);
};

#endif // __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__
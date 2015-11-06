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


#ifndef __ResourceEditorQt__EmitterLayerWidget__
#define __ResourceEditorQt__EmitterLayerWidget__

#include <DAVAEngine.h>
#include "TimeLineWidget.h"
#include "GradientPickerWidget.h"
#include "BaseParticleEditorContentWidget.h"
#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTimer>

using namespace DAVA;

class EmitterLayerWidget: public QWidget, public BaseParticleEditorContentWidget
{
    Q_OBJECT
    
public:
    explicit EmitterLayerWidget(QWidget *parent = 0);
    ~EmitterLayerWidget();

	void Init(SceneEditor2* scene, ParticleEffectComponent* effect, ParticleEmitter* emitter, ParticleLayer* layer, bool updateMinimized);
	ParticleLayer* GetLayer() const {return layer;};
	void Update(bool updateMinimized);
	
	virtual bool eventFilter(QObject *, QEvent *);

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

	// Switch from/to SuperEmitter mode.
	void SetSuperemitterMode(bool isSuperemitter);

	// Notify yhe widget layer value is changed.
	void OnLayerValueChanged();

signals:
	void ValueChanged();
	
protected slots:
	void OnLodsChanged();
    void OnValueChanged();
    void OnSpriteBtn();
    void OnSpritePathChanged(const QString& text);

    void OnPivotPointReset();
    void OnSpriteUpdateTimerExpired();

private:
	void InitWidget(QWidget* );
	void UpdateTooltip();
	
	void FillLayerTypes();
	int32 LayerTypeToIndex(ParticleLayer::eType layerType);

private:
	struct LayerTypeMap
	{
		ParticleLayer::eType layerType;
		QString layerName;
	};

	struct BlendPreset
	{
        eBlending blending;
        QString presetName;
    };

private:
    static const LayerTypeMap layerTypeMap[];
    static const BlendPreset blendPresetsMap[];

    ParticleLayer* layer = nullptr;

    Sprite* sprite = nullptr;
    QTimer* spriteUpdateTimer = nullptr;
    DAVA::Stack<std::pair<rhi::HSyncObject, Texture*>> spriteUpdateTexturesStack;

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
    QCheckBox* layerLodsCheckBox[LodComponent::MAX_LOD_LAYERS];
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

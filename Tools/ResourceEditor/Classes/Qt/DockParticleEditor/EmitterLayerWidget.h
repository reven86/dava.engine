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
	void OnPresetChanged();
	void OnSpriteBtn();
	void OnSpritePathChanged(const QString& text);
	
	void OnPivotPointReset();
	
private:
	void InitWidget(QWidget* );
	void UpdateTooltip();
	
	void FillLayerTypes();
	int32 LayerTypeToIndex(ParticleLayer::eType layerType);

	void FillBlendCombos();

private:
	ParticleLayer* layer;
	QVBoxLayout* mainBox;
	
	QLineEdit* layerNameLineEdit;
	QCheckBox* enableCheckBox;	
	
	QCheckBox* isLongCheckBox;
	EventFilterDoubleSpinBox *scaleVelocityBaseSpinBox;
	EventFilterDoubleSpinBox *scaleVelocityFactorSpinBox;
	QLabel *scaleVelocityBaseLabel, *scaleVelocityFactorLabel;

	QCheckBox* isLoopedCheckBox;
	QCheckBox* inheritPostionCheckBox;


	QCheckBox* layerLodsCheckBox[LodComponent::MAX_LOD_LAYERS];
    QComboBox* degradeStrategyComboBox;

	QLabel* layerTypeLabel;
	QComboBox* layerTypeComboBox;

	Sprite* sprite;
	QLabel* spriteLabel;
	QPushButton* spriteBtn;
	QLineEdit* spritePathLabel;
	
	QLabel* innerEmitterLabel;
	QLineEdit* innerEmitterPathLabel;

	QVBoxLayout* pivotPointLayout;
	QLabel* pivotPointLabel;
	EventFilterDoubleSpinBox* pivotPointXSpinBox;
	QLabel* pivotPointXSpinBoxLabel;
	EventFilterDoubleSpinBox* pivotPointYSpinBox;
	QLabel* pivotPointYSpinBoxLabel;
	QPushButton* pivotPointResetButton;

	QCheckBox *frameBlendingCheckBox;

	QLabel *particleOrientationLabel;
	QCheckBox *cameraFacingCheckBox;
	QCheckBox *xFacingCheckBox;
	QCheckBox *yFacingCheckBox;
	QCheckBox *zFacingCheckBox;
	QCheckBox *worldAlignCheckBox;

	QLabel *blendOptionsLabel;
	QLabel *presetLabel;
	QComboBox *presetComboBox;
	QLabel *srcFactorLabel;
	QComboBox *srcFactorComboBox;
	QLabel *dstFactorLabel;
	QComboBox *dstFactorComboBox;
	QCheckBox *fogCheckBox;	

	TimeLineWidget* lifeTimeLine;
	TimeLineWidget* numberTimeLine;
	TimeLineWidget* sizeTimeLine;
	TimeLineWidget* sizeVariationTimeLine;
	TimeLineWidget* sizeOverLifeTimeLine;
	TimeLineWidget* velocityTimeLine;
	TimeLineWidget* velocityOverLifeTimeLine;
	TimeLineWidget* spinTimeLine;
	TimeLineWidget* spinOverLifeTimeLine;
	

	TimeLineWidget* alphaOverLifeTimeLine;
	QCheckBox* frameOverlifeCheckBox;
	QSpinBox* frameOverlifeFPSSpin;
	QLabel* frameOverlifeFPSLabel;
	QCheckBox* randomFrameOnStartCheckBox;
	QCheckBox* loopSpriteAnimationCheckBox;
	TimeLineWidget* animSpeedOverLifeTimeLine;

	QCheckBox* randomSpinDirectionCheckBox;

	TimeLineWidget* angleTimeLine;
	GradientPickerWidget* colorRandomGradient;
	GradientPickerWidget* colorOverLifeGradient;

	EventFilterDoubleSpinBox* startTimeSpin;
	EventFilterDoubleSpinBox* endTimeSpin;
	EventFilterDoubleSpinBox* deltaSpin;
	EventFilterDoubleSpinBox* loopEndSpin;
	EventFilterDoubleSpinBox* deltaVariationSpin;
	EventFilterDoubleSpinBox* loopVariationSpin;
	
	QLabel* deltaSpinLabel;
	QLabel* deltaVariationSpinLabel;
	QLabel* loopEndSpinLabel;
	QLabel* loopVariationSpinLabel;
	
	bool blockSignals;

	struct LayerTypeMap
	{
		ParticleLayer::eType layerType;
		QString layerName;
	};

	static const LayerTypeMap layerTypeMap[];

	struct BlendPreset
	{
        eBlending blending;
		QString presetName;
	};

	static const BlendPreset blendPresetsMap[];
};

#endif /* defined(__ResourceEditorQt__EmitterLayerWidget__) */

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



#include "EmitterLayerWidget.h"
#include "Commands2/ParticleEditorCommands.h"
#include "TextureBrowser/TextureConvertor.h"
#include "SceneEditor/EditorSettings.h"
#include "../Scene/SceneDataManager.h"

#include <QHBoxLayout>
#include <QGraphicsWidget>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

#define SPRITE_SIZE 60

#define ANGLE_MIN_LIMIT_DEGREES -360.0f
#define ANGLE_MAX_LIMIT_DEGREES 360.0f

const EmitterLayerWidget::LayerTypeMap EmitterLayerWidget::layerTypeMap[] =
{
	{ParticleLayer::TYPE_SINGLE_PARTICLE, "Single Particle"},
	{ParticleLayer::TYPE_PARTICLES, "Particles"},
	{ParticleLayer::TYPE_SUPEREMITTER_PARTICLES, "SuperEmitter"}
};

EmitterLayerWidget::EmitterLayerWidget(QWidget *parent) :
	QWidget(parent),
	BaseParticleEditorContentWidget()
{
	mainBox = new QVBoxLayout;
	this->setLayout(mainBox);
	
	layerNameLineEdit = new QLineEdit();
	mainBox->addWidget(layerNameLineEdit);
	connect(layerNameLineEdit,
			SIGNAL(editingFinished()),
			this,
			SLOT(OnValueChanged()));

	QVBoxLayout* lodsLayout = new QVBoxLayout();
	QLabel *lodsLabel = new QLabel("Active in LODs", this);
	lodsLayout->addWidget(lodsLabel);
	QHBoxLayout* lodsInnerLayout = new QHBoxLayout();

	for (int32 i=0; i<LodComponent::MAX_LOD_LAYERS; ++i)
	{
		layerLodsCheckBox[i] = new QCheckBox(QString("LOD")+QString::number(i));
		lodsInnerLayout->addWidget(layerLodsCheckBox[i]);
		connect(layerLodsCheckBox[i],
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnLodsChanged()));		
	}
	lodsLayout->addLayout(lodsInnerLayout);
	mainBox->addLayout(lodsLayout);

	layerTypeLabel = new QLabel(this);
	layerTypeLabel->setText("Layer type");
	mainBox->addWidget(layerTypeLabel);

	layerTypeComboBox = new QComboBox(this);
	FillLayerTypes();
	mainBox->addWidget(layerTypeComboBox);
	connect(layerTypeComboBox,
			SIGNAL(currentIndexChanged(int)),
			this,
			SLOT(OnValueChanged()));

	enableCheckBox = new QCheckBox("Enable layer");
	mainBox->addWidget(enableCheckBox);
	connect(enableCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	
	
	additiveCheckBox = new QCheckBox("Additive");
	mainBox->addWidget(additiveCheckBox);
	connect(additiveCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));


	inheritPostionCheckBox = new QCheckBox("Inherit Position");
	mainBox->addWidget(inheritPostionCheckBox);
	connect(inheritPostionCheckBox,
		SIGNAL(stateChanged(int)),
		this,
		SLOT(OnValueChanged()));

	isLongCheckBox = new QCheckBox("Long");
	mainBox->addWidget(isLongCheckBox);
	connect(isLongCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));

	
	QHBoxLayout* spriteHBox = new QHBoxLayout;
	spriteLabel = new QLabel(this);
	spriteLabel->setMinimumSize(SPRITE_SIZE, SPRITE_SIZE);
	spriteHBox->addWidget(spriteLabel);
	mainBox->addLayout(spriteHBox);
	QVBoxLayout* spriteVBox = new QVBoxLayout;
	spriteHBox->addLayout(spriteVBox);
	spriteBtn = new QPushButton("Set sprite", this);
	spriteBtn->setMinimumHeight(30);
	spritePathLabel = new QLineEdit(this);
	spritePathLabel->setReadOnly(true);
	spriteVBox->addWidget(spriteBtn);
	spriteVBox->addWidget(spritePathLabel);
	connect(spriteBtn,
			SIGNAL(clicked(bool)),
			this,
			SLOT(OnSpriteBtn()));
	connect(spritePathLabel, SIGNAL(textChanged(const QString&)), this, SLOT(OnSpritePathChanged(const QString&)));

	QVBoxLayout* innerEmitterLayout = new QVBoxLayout();
	innerEmitterLabel = new QLabel("Inner Emitter", this);
	innerEmitterPathLabel = new QLineEdit(this);
	innerEmitterPathLabel->setReadOnly(true);
	innerEmitterLayout->addWidget(innerEmitterLabel);
	innerEmitterLayout->addWidget(innerEmitterPathLabel);
	mainBox->addLayout(innerEmitterLayout);
	
	QVBoxLayout* pivotPointLayout = new QVBoxLayout();
	pivotPointLabel = new QLabel("Pivot Point", this);
	pivotPointLayout->addWidget(pivotPointLabel);
	QHBoxLayout* pivotPointInnerLayout = new QHBoxLayout();

	pivotPointXSpinBoxLabel = new QLabel("X:", this);
	pivotPointInnerLayout->addWidget(pivotPointXSpinBoxLabel);
	pivotPointXSpinBox = new QSpinBox(this);
	pivotPointXSpinBox->setMinimum(-9999);
	pivotPointXSpinBox->setMaximum(9999);
	pivotPointInnerLayout->addWidget(pivotPointXSpinBox);

	pivotPointYSpinBoxLabel = new QLabel("Y:", this);
	pivotPointInnerLayout->addWidget(pivotPointYSpinBoxLabel);
	pivotPointYSpinBox = new QSpinBox(this);
	pivotPointYSpinBox->setMinimum(-9999);
	pivotPointYSpinBox->setMaximum(9999);
	pivotPointInnerLayout->addWidget(pivotPointYSpinBox);
	
	pivotPointResetButton = new QPushButton("Reset", this);
	pivotPointInnerLayout->addWidget(pivotPointResetButton);
	connect(pivotPointResetButton, SIGNAL(clicked(bool)), this, SLOT(OnPivotPointReset()));

	connect(pivotPointXSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));
	connect(pivotPointYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));

	pivotPointLayout->addLayout(pivotPointInnerLayout);
	mainBox->addLayout(pivotPointLayout);

	lifeTimeLine = new TimeLineWidget(this);
	InitWidget(lifeTimeLine);
	numberTimeLine = new TimeLineWidget(this);
	InitWidget(numberTimeLine);
	sizeTimeLine = new TimeLineWidget(this);
	InitWidget(sizeTimeLine);
	sizeVariationTimeLine = new TimeLineWidget(this);
	InitWidget(sizeVariationTimeLine);
	sizeOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(sizeOverLifeTimeLine);
	velocityTimeLine = new TimeLineWidget(this);
	InitWidget(velocityTimeLine);
	velocityOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(velocityOverLifeTimeLine);
	spinTimeLine = new TimeLineWidget(this);
	InitWidget(spinTimeLine);
	spinOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(spinOverLifeTimeLine);

	colorRandomGradient = new GradientPickerWidget(this);
	InitWidget(colorRandomGradient);
	colorOverLifeGradient = new GradientPickerWidget(this);
	InitWidget(colorOverLifeGradient);
	alphaOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(alphaOverLifeTimeLine);
	
	QHBoxLayout* frameOverlifeLayout = new QHBoxLayout();
	frameOverlifeCheckBox = new QCheckBox("frame over life", this);
	connect(frameOverlifeCheckBox, SIGNAL(stateChanged(int)),
				  this, SLOT(OnValueChanged()));

	frameOverlifeFPSSpin = new QSpinBox(this);
	frameOverlifeFPSSpin->setMinimum(0);
	frameOverlifeFPSSpin->setMaximum(1000);
	connect(frameOverlifeFPSSpin, SIGNAL(valueChanged(int)),
			this, SLOT(OnValueChanged()));

	frameOverlifeFPSLabel = new QLabel("FPS", this);

	frameOverlifeLayout->addWidget(frameOverlifeCheckBox);
	frameOverlifeLayout->addWidget(frameOverlifeFPSSpin);
	frameOverlifeLayout->addWidget(frameOverlifeFPSLabel);
	mainBox->addLayout(frameOverlifeLayout);

	randomFrameOnStartCheckBox = new QCheckBox("random frame on start", this);
	connect(randomFrameOnStartCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(OnValueChanged()));
	mainBox->addWidget(randomFrameOnStartCheckBox);
	
	angleTimeLine = new TimeLineWidget(this);
	InitWidget(angleTimeLine);

	QHBoxLayout* startTimeHBox = new QHBoxLayout;
	startTimeHBox->addWidget(new QLabel("startTime", this));
	startTimeSpin = new EventFilterDoubleSpinBox(this);
	startTimeSpin->setMinimum(-std::numeric_limits<double>::infinity());
	startTimeSpin->setMaximum(std::numeric_limits<double>::infinity());
	startTimeHBox->addWidget(startTimeSpin);
	mainBox->addLayout(startTimeHBox);
	connect(startTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));

	QHBoxLayout* endTimeHBox = new QHBoxLayout;
	endTimeHBox->addWidget(new QLabel("endTime", this));
	endTimeSpin = new EventFilterDoubleSpinBox(this);
	endTimeSpin->setMinimum(-std::numeric_limits<double>::infinity());
	endTimeSpin->setMaximum(std::numeric_limits<double>::infinity());
	endTimeHBox->addWidget(endTimeSpin);
	mainBox->addLayout(endTimeHBox);
	connect(endTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
			
			
	QHBoxLayout* loopHBox = new QHBoxLayout;	
	isLoopedCheckBox = new QCheckBox("Loop layer");
	loopHBox->addWidget(isLoopedCheckBox);
	connect(isLoopedCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
			
	loopEndSpinLabel = new QLabel("loopEnd", this);
	loopEndSpin = new EventFilterDoubleSpinBox(this);
	loopEndSpin->setMinimum(-std::numeric_limits<double>::infinity());
	loopEndSpin->setMaximum(std::numeric_limits<double>::infinity());
	loopHBox->addWidget(loopEndSpinLabel);
	loopHBox->addWidget(loopEndSpin);
	connect(loopEndSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	
	loopVariationSpinLabel = new QLabel("loopVariation", this);
	loopVariationSpin = new EventFilterDoubleSpinBox(this);
	loopVariationSpin->setMinimum(-std::numeric_limits<double>::infinity());
	loopVariationSpin->setMaximum(std::numeric_limits<double>::infinity());
	loopHBox->addWidget(loopVariationSpinLabel);
	loopHBox->addWidget(loopVariationSpin);
	connect(loopVariationSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
			
	loopHBox->setStretch(0, 1);
	loopHBox->setStretch(2, 1);
	loopHBox->setStretch(4, 1);
	mainBox->addLayout(loopHBox);
	
	QHBoxLayout *deltaHBox = new QHBoxLayout();
	
	deltaSpinLabel = new QLabel("delta", this);
	deltaSpin = new EventFilterDoubleSpinBox(this);
	deltaSpin->setMinimum(-std::numeric_limits<double>::infinity());
	deltaSpin->setMaximum(std::numeric_limits<double>::infinity());
	deltaHBox->addWidget(deltaSpinLabel);
	deltaHBox->addWidget(deltaSpin);
	connect(deltaSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
			
	deltaVariationSpinLabel = new QLabel("deltaVariation", this);
	deltaVariationSpin = new EventFilterDoubleSpinBox(this);
	deltaVariationSpin->setMinimum(-std::numeric_limits<double>::infinity());
	deltaVariationSpin->setMaximum(std::numeric_limits<double>::infinity());
	deltaHBox->addWidget(deltaVariationSpinLabel);
	deltaHBox->addWidget(deltaVariationSpin);
	connect(deltaVariationSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	deltaHBox->setStretch(1, 1);
	deltaHBox->setStretch(3, 1);
	mainBox->addLayout(deltaHBox);	
	
	Q_FOREACH( QAbstractSpinBox * sp, findChildren<QAbstractSpinBox*>() ) {
        sp->installEventFilter( this );
    }
	spritePathLabel->installEventFilter(this);

	sprite = NULL;
	blockSignals = false;
}

EmitterLayerWidget::~EmitterLayerWidget()
{
	disconnect(layerNameLineEdit,
			SIGNAL(editingFinished()),
			this,
			SLOT(OnValueChanged()));
	disconnect(layerTypeComboBox,
			SIGNAL(currentIndexChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(enableCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(additiveCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(inheritPostionCheckBox,
		SIGNAL(stateChanged(int)),
		this,
		SLOT(OnValueChanged()));
	disconnect(isLongCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(spriteBtn,
			SIGNAL(clicked(bool)),
			this,
			SLOT(OnSpriteBtn()));
	disconnect(spritePathLabel,
			SIGNAL(textChanged(const QString&)),
			this,
			SLOT(OnSpritePathChanged(const QString&)));
	disconnect(isLoopedCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(deltaSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(loopEndSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(startTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(endTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(frameOverlifeCheckBox,
		   SIGNAL(stateChanged(int)),
		   this,
		   SLOT(OnValueChanged()));
	disconnect(frameOverlifeFPSSpin,
		   SIGNAL(valueChanged(int)),
		   this,
		   SLOT(OnValueChanged()));
	disconnect(randomFrameOnStartCheckBox,
		SIGNAL(stateChanged(int)),
		this,
		SLOT(OnValueChanged()));
	disconnect(pivotPointXSpinBox,
			   SIGNAL(valueChanged(int)),
			   this,
			   SLOT(OnValueChanged()));
	disconnect(pivotPointYSpinBox,
			SIGNAL(valueChanged(int)),
			this,
			SLOT(OnValueChanged()));
	for (int32 i=0; i<LodComponent::MAX_LOD_LAYERS; ++i)
	{	
		disconnect(layerLodsCheckBox[i],
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnLodsChanged()));		
	}
}

void EmitterLayerWidget::InitWidget(QWidget* widget)
{
	mainBox->addWidget(widget);
	connect(widget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void EmitterLayerWidget::Init(SceneEditor2* scene, ParticleEmitter* emitter, DAVA::ParticleLayer *layer, bool updateMinimized)
{
	if (!emitter || !layer)
		return;
	
	blockSignals = true;
	
	this->emitter = emitter;
	this->layer = layer;
	SetActiveScene(scene);
	
	float32 emitterLifeTime = emitter->GetLifeTime();
	float32 lifeTime = Min(emitterLifeTime, layer->endTime);

	layerNameLineEdit->setText(QString::fromStdString(layer->layerName));
	layerTypeComboBox->setCurrentIndex(LayerTypeToIndex(layer->type));

	enableCheckBox->setChecked(!layer->GetDisabled());
	additiveCheckBox->setChecked(layer->GetAdditive());
	inheritPostionCheckBox->setChecked(layer->GetInheritPosition());
	isLongCheckBox->setChecked(layer->IsLong());
	isLoopedCheckBox->setChecked(layer->GetLooped());

	for (int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
	{
		layerLodsCheckBox[i]->setChecked(layer->IsLodActive(i));
	}

	//LAYER_SPRITE = 0,
	sprite = layer->GetSprite();
	Sprite* renderSprite = Sprite::CreateAsRenderTarget(SPRITE_SIZE, SPRITE_SIZE, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(renderSprite);
	if (sprite)
	{
		sprite->SetScaleSize(SPRITE_SIZE, SPRITE_SIZE);
		sprite->Draw();
	}

	RenderManager::Instance()->RestoreRenderTarget();
	Texture* texture = renderSprite->GetTexture();
	Image* image = texture->CreateImageFromMemory();
	spriteLabel->setPixmap(QPixmap::fromImage(TextureConvertor::FromDavaImage(image)));
	SafeRelease(image);
	SafeRelease(renderSprite);

	QString spriteName = "<none>";
	if (sprite)
	{
		spriteName = QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
	}
	spritePathLabel->setText(spriteName);

	//LAYER_LIFE, LAYER_LIFE_VARIATION,
	lifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	lifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->life).GetProps(), Qt::blue, "life");
	lifeTimeLine->AddLine(1, PropLineWrapper<float32>(layer->lifeVariation).GetProps(), Qt::darkGreen, "life variation");
	lifeTimeLine->SetMinLimits(0.0f);

	//LAYER_NUMBER, LAYER_NUMBER_VARIATION,
	numberTimeLine->Init(layer->startTime, lifeTime, updateMinimized, false, true, true);
//		void Init(float32 minT, float32 maxT, bool updateSizeState, bool aliasLinePoint = false, bool allowDeleteLine = true, bool integer = false);
	numberTimeLine->SetMinLimits(0);
	numberTimeLine->AddLine(0, PropLineWrapper<float32>(layer->number).GetProps(), Qt::blue, "number");
	numberTimeLine->AddLine(1, PropLineWrapper<float32>(layer->numberVariation).GetProps(), Qt::darkGreen, "number variation");
	
	ParticleLayer::eType propLayerType = layerTypeMap[layerTypeComboBox->currentIndex()].layerType;
	numberTimeLine->setVisible(propLayerType != ParticleLayer::TYPE_SINGLE_PARTICLE);

	//LAYER_SIZE, LAYER_SIZE_VARIATION, LAYER_SIZE_OVER_LIFE,
	Vector<QColor> colors;
	colors.push_back(Qt::red); colors.push_back(Qt::darkGreen);
	Vector<QString> legends;
	legends.push_back("size X"); legends.push_back("size Y");
	sizeTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true);
	sizeTimeLine->SetMinLimits(0);
	sizeTimeLine->AddLines(PropLineWrapper<Vector2>(layer->size).GetProps(), colors, legends);
	sizeTimeLine->EnableLock(true);
	
	legends.clear();
	legends.push_back("size variation X"); legends.push_back("size variation Y");
	sizeVariationTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true);
	sizeVariationTimeLine->SetMinLimits(0);
	sizeVariationTimeLine->AddLines(PropLineWrapper<Vector2>(layer->sizeVariation).GetProps(), colors, legends);
	sizeVariationTimeLine->EnableLock(true);

	legends.clear();
	legends.push_back("size overlife X"); legends.push_back("size overlife Y");
	sizeOverLifeTimeLine->Init(0, 1, updateMinimized, true);
	sizeOverLifeTimeLine->SetMinLimits(0);
	sizeOverLifeTimeLine->AddLines(PropLineWrapper<Vector2>(layer->sizeOverLifeXY).GetProps(), colors, legends);
	sizeOverLifeTimeLine->EnableLock(true);

	//LAYER_VELOCITY, LAYER_VELOCITY_VARIATION,
	velocityTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	velocityTimeLine->AddLine(0, PropLineWrapper<float32>(layer->velocity).GetProps(), Qt::blue, "velocity");
	velocityTimeLine->AddLine(1, PropLineWrapper<float32>(layer->velocityVariation).GetProps(), Qt::darkGreen, "velocity variation");
	
	//LAYER_VELOCITY_OVER_LIFE,
	velocityOverLifeTimeLine->Init(0, 1, updateMinimized);
	velocityOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->velocityOverLife).GetProps(), Qt::blue, "velocity over life");

	//LAYER_FORCES, LAYER_FORCES_VARIATION, LAYER_FORCES_OVER_LIFE,

	//LAYER_SPIN, LAYER_SPIN_VARIATION, 
	spinTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	spinTimeLine->AddLine(0, PropLineWrapper<float32>(layer->spin).GetProps(), Qt::blue, "spin");
	spinTimeLine->AddLine(1, PropLineWrapper<float32>(layer->spinVariation).GetProps(), Qt::darkGreen, "spin variation");
	
	//LAYER_SPIN_OVER_LIFE,
	spinOverLifeTimeLine->Init(0, 1, updateMinimized);
	spinOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->spinOverLife).GetProps(), Qt::blue, "spin over life");

	//LAYER_COLOR_RANDOM, LAYER_ALPHA_OVER_LIFE, LAYER_COLOR_OVER_LIFE,
	colorRandomGradient->Init(0, 1, "random color");
	colorRandomGradient->SetValues(PropLineWrapper<Color>(layer->colorRandom).GetProps());
	
	colorOverLifeGradient->Init(0, 1, "color over life");
	colorOverLifeGradient->SetValues(PropLineWrapper<Color>(layer->colorOverLife).GetProps());

	alphaOverLifeTimeLine->Init(0, 1, updateMinimized);
	alphaOverLifeTimeLine->SetMinLimits(0);
	alphaOverLifeTimeLine->SetMaxLimits(1.f);
	alphaOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->alphaOverLife).GetProps(), Qt::blue, "alpha over life");
	
	frameOverlifeCheckBox->setChecked(layer->frameOverLifeEnabled);
	frameOverlifeFPSSpin->setValue(layer->frameOverLifeFPS);
	frameOverlifeFPSSpin->setEnabled(layer->frameOverLifeEnabled);
	randomFrameOnStartCheckBox->setChecked(layer->randomFrameOnStart);
	
	angleTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	angleTimeLine->AddLine(0, PropLineWrapper<float32>(layer->angle).GetProps(), Qt::blue, "angle");
	angleTimeLine->AddLine(1, PropLineWrapper<float32>(layer->angleVariation).GetProps(), Qt::darkGreen, "angle variation");
	angleTimeLine->SetMinLimits(ANGLE_MIN_LIMIT_DEGREES);
	angleTimeLine->SetMaxLimits(ANGLE_MAX_LIMIT_DEGREES);
	angleTimeLine->SetYLegendMark(DEGREE_MARK_CHARACTER);

	//LAYER_START_TIME, LAYER_END_TIME
	startTimeSpin->setMinimum(0);
	startTimeSpin->setValue(layer->startTime);
	startTimeSpin->setMaximum(layer->endTime);
	endTimeSpin->setMinimum(0);
	endTimeSpin->setValue(layer->endTime);
	endTimeSpin->setMaximum(emitter->GetLifeTime());

	// LAYER delta, deltaVariation, loopEnd and loopVariation
	bool isLoopedChecked = isLoopedCheckBox->isChecked();	
	deltaSpin->setMinimum(0);
	deltaSpin->setValue(layer->GetDeltaTime());
	deltaSpin->setVisible(isLoopedChecked);
	deltaSpinLabel->setVisible(isLoopedChecked);
	
	deltaVariationSpin->setMinimum(0);
	deltaVariationSpin->setMaximum(emitter->GetLifeTime());
	deltaVariationSpin->setValue(layer->GetDeltaVariation());
	deltaVariationSpin->setVisible(isLoopedChecked);
	deltaVariationSpinLabel->setVisible(isLoopedChecked);

	loopEndSpin->setMinimum(0);
	loopEndSpin->setMaximum(emitter->GetLifeTime());
	loopEndSpin->setValue(layer->GetLoopEndTime());
	loopEndSpin->setVisible(isLoopedChecked);
	loopEndSpinLabel->setVisible(isLoopedChecked);
	
	loopVariationSpin->setMinimum(0);
	loopVariationSpin->setMaximum(emitter->GetLifeTime());
	loopVariationSpin->setValue(layer->GetLoopVariation());
	loopVariationSpin->setVisible(isLoopedChecked);
	loopVariationSpinLabel->setVisible(isLoopedChecked);

	const Vector2& layerPivotPoint = layer->GetPivotPoint();
	pivotPointXSpinBox->setValue((int)layerPivotPoint.x);
	pivotPointYSpinBox->setValue((int)layerPivotPoint.y);

	blockSignals = false;
}

void EmitterLayerWidget::RestoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	lifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_LIFE_PROPS"));
	numberTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_NUMBER_PROPS"));
	sizeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SIZE_PROPS"));
	sizeVariationTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SIZE_VARIATION_PROPS"));
	sizeOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SIZE_OVER_LIFE_PROPS"));
	velocityTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_VELOCITY_PROPS"));
	velocityOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_VELOCITY_OVER_LIFE"));//todo
	spinTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SPIN_PROPS"));
	spinOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SPIN_OVER_LIFE_PROPS"));
	alphaOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_ALPHA_OVER_LIFE_PROPS"));
	angleTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_ANGLE"));	
}

void EmitterLayerWidget::StoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	KeyedArchive* props = new KeyedArchive();

	lifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	numberTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_NUMBER_PROPS", props);

	props->DeleteAllKeys();
	sizeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SIZE_PROPS", props);

	props->DeleteAllKeys();
	sizeVariationTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SIZE_VARIATION_PROPS", props);

	props->DeleteAllKeys();
	sizeOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SIZE_OVER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	velocityTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_VELOCITY_PROPS", props);

	props->DeleteAllKeys();
	velocityOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_VELOCITY_OVER_LIFE", props);

	props->DeleteAllKeys();
	spinTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SPIN_PROPS", props);

	props->DeleteAllKeys();
	spinOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SPIN_OVER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	alphaOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_ALPHA_OVER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	angleTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_ANGLE", props);

	SafeRelease(props);
}

void EmitterLayerWidget::OnSpriteBtn()
{
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
	projectPath += "Data/Gfx/Particles/";
    
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open particle sprite"), QString::fromStdString(projectPath.GetAbsolutePathname()), QString("Effect File (*.txt)"));
	if (filePath.isEmpty())
		return;
	
	// Yuri Coder. Verify that the path of the file opened is correct (i.e. inside the Project Path),
	// this is according to the DF-551 issue.
    FilePath filePathToBeOpened(filePath.toStdString());
	String relativePathForProjectPath = filePathToBeOpened.GetRelativePathname(projectPath);

// 	if (filePathToBeOpened.GetDirectory() != projectPath)
	if (relativePathForProjectPath.find("../") != String::npos)
	{
		QString message = QString("You've opened Particle Sprite from incorrect path (%1).\n Correct one is %2.").
			arg(QString::fromStdString(filePathToBeOpened.GetDirectory().GetAbsolutePathname())).
			arg(QString::fromStdString(projectPath.GetDirectory().GetAbsolutePathname()));

		QMessageBox msgBox(QMessageBox::Warning, "Warning", message);
		msgBox.exec();

		// TODO: return here in case we'll decide to not allow opening sprite from incorrect path.
	}
	
	filePath.remove(filePath.size() - 4, 4);
	Sprite* sprite = Sprite::Create(filePath.toStdString());
	if (!sprite)
		return;
	
	this->sprite = sprite;
	OnValueChanged();
}

void EmitterLayerWidget::OnValueChanged()
{
	if (blockSignals)
		return;
	
	PropLineWrapper<float32> propLife;
	PropLineWrapper<float32> propLifeVariation;
	lifeTimeLine->GetValue(0, propLife.GetPropsPtr());
	lifeTimeLine->GetValue(1, propLifeVariation.GetPropsPtr());
	
	PropLineWrapper<float32> propNumber;
	PropLineWrapper<float32> propNumberVariation;
	numberTimeLine->GetValue(0, propNumber.GetPropsPtr());
	numberTimeLine->GetValue(1, propNumberVariation.GetPropsPtr());
	
	PropLineWrapper<Vector2> propSize;
	sizeTimeLine->GetValues(propSize.GetPropsPtr());
	
	PropLineWrapper<Vector2> propSizeVariation;
	sizeVariationTimeLine->GetValues(propSizeVariation.GetPropsPtr());

	PropLineWrapper<Vector2> propsizeOverLife;
	sizeOverLifeTimeLine->GetValues(propsizeOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propVelocity;
	PropLineWrapper<float32> propVelocityVariation;
	velocityTimeLine->GetValue(0, propVelocity.GetPropsPtr());
	velocityTimeLine->GetValue(1, propVelocityVariation.GetPropsPtr());

	PropLineWrapper<float32> propVelocityOverLife;
	velocityOverLifeTimeLine->GetValue(0, propVelocityOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propSpin;
	PropLineWrapper<float32> propSpinVariation;
	spinTimeLine->GetValue(0, propSpin.GetPropsPtr());
	spinTimeLine->GetValue(1, propSpinVariation.GetPropsPtr());

	PropLineWrapper<float32> propSpinOverLife;
	spinOverLifeTimeLine->GetValue(0, propSpinOverLife.GetPropsPtr());

	PropLineWrapper<Color> propColorRandom;
	colorRandomGradient->GetValues(propColorRandom.GetPropsPtr());

	PropLineWrapper<Color> propColorOverLife;
	colorOverLifeGradient->GetValues(propColorOverLife.GetPropsPtr());

	PropLineWrapper<float32> propAlphaOverLife;
	alphaOverLifeTimeLine->GetValue(0, propAlphaOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propAngle;
	PropLineWrapper<float32> propAngleVariation;
	angleTimeLine->GetValue(0, propAngle.GetPropsPtr());
	angleTimeLine->GetValue(1, propAngleVariation.GetPropsPtr());

	ParticleLayer::eType propLayerType = layerTypeMap[layerTypeComboBox->currentIndex()].layerType;

	CommandUpdateParticleLayer* updateLayerCmd = new CommandUpdateParticleLayer(emitter, layer);
	updateLayerCmd->Init(layerNameLineEdit->text().toStdString(),
						 propLayerType,
						 !enableCheckBox->isChecked(),
						 additiveCheckBox->isChecked(),
						 inheritPostionCheckBox->isChecked(),
						 isLongCheckBox->isChecked(),
						 isLoopedCheckBox->isChecked(),
						 sprite,
						 propLife.GetPropLine(),
						 propLifeVariation.GetPropLine(),
						 propNumber.GetPropLine(),
						 propNumberVariation.GetPropLine(),
						 propSize.GetPropLine(),
						 propSizeVariation.GetPropLine(),
						 propsizeOverLife.GetPropLine(),
						 propVelocity.GetPropLine(),
						 propVelocityVariation.GetPropLine(),
						 propVelocityOverLife.GetPropLine(),
						 propSpin.GetPropLine(),
						 propSpinVariation.GetPropLine(),
						 propSpinOverLife.GetPropLine(),

						 propColorRandom.GetPropLine(),
						 propAlphaOverLife.GetPropLine(),
						 propColorOverLife.GetPropLine(),
						 propAngle.GetPropLine(),
						 propAngleVariation.GetPropLine(),

						 (float32)startTimeSpin->value(),
						 (float32)endTimeSpin->value(),
						 (float32)deltaSpin->value(),
						 (float32)deltaVariationSpin->value(),
						 (float32)loopEndSpin->value(),
						 (float32)loopVariationSpin->value(),
						 frameOverlifeCheckBox->isChecked(),
						 (float32)frameOverlifeFPSSpin->value(),
						 randomFrameOnStartCheckBox->isChecked(),
						 (float32)pivotPointXSpinBox->value(),
						 (float32)pivotPointYSpinBox->value());

	DVASSERT(activeScene);
	activeScene->Exec(updateLayerCmd);

	Init(activeScene, emitter, layer, false);
	emit ValueChanged();
}

void EmitterLayerWidget::OnLodsChanged()
{
	if (blockSignals)
		return;
	Vector<bool> lods;
	lods.resize(LodComponent::MAX_LOD_LAYERS, true);
	for (int32 i=0; i<LodComponent::MAX_LOD_LAYERS; ++i)
	{
		lods[i] = layerLodsCheckBox[i]->isChecked();
	}
	CommandUpdateParticleLayerLods * updateLodsCmd = new CommandUpdateParticleLayerLods(layer, lods);
	activeScene->Exec(updateLodsCmd);
	emit ValueChanged();
}

void EmitterLayerWidget::Update()
{
	Init(activeScene, emitter, layer, false);
}

void EmitterLayerWidget::UpdateTooltip()
{
	QFontMetrics fm = spritePathLabel->fontMetrics();
	if (fm.width(spritePathLabel->text()) >= spritePathLabel->width())
	{
		spritePathLabel->setToolTip(spritePathLabel->text());
	}
	else
	{
		spritePathLabel->setToolTip("");
	}
}

bool EmitterLayerWidget::eventFilter( QObject * o, QEvent * e )
{
    if ( e->type() == QEvent::Wheel &&
		qobject_cast<QAbstractSpinBox*>( o ) )
    {
        e->ignore();
        return true;
    }

	if (e->type() == QEvent::Resize && qobject_cast<QLineEdit*>(o))
	{
		UpdateTooltip();
		return true;
	}

    return QWidget::eventFilter( o, e );
}

void EmitterLayerWidget::OnSpritePathChanged(const QString& text)
{
	UpdateTooltip();
}

void EmitterLayerWidget::FillLayerTypes()
{
	int32 layerTypes = sizeof(layerTypeMap) / sizeof(*layerTypeMap);
	for (int32 i = 0; i < layerTypes; i ++)
	{
		layerTypeComboBox->addItem(layerTypeMap[i].layerName);
	}
}

int32 EmitterLayerWidget::LayerTypeToIndex(ParticleLayer::eType layerType)
{
	int32 layerTypes = sizeof(layerTypeMap) / sizeof(*layerTypeMap);
	for (int32 i = 0; i < layerTypes; i ++)
	{
		if (layerTypeMap[i].layerType == layerType)
		{
			return i;
		}
	}
	
	return 0;
}

void EmitterLayerWidget::SetSuperemitterMode(bool isSuperemitter)
{
	// Sprite has no sense for Superemitter.
	spriteBtn->setVisible(!isSuperemitter);
	spriteLabel->setVisible(!isSuperemitter);
	spritePathLabel->setVisible(!isSuperemitter);
	
	// The same is for "Additive" flag, Color, Alpha and Frame.
	additiveCheckBox->setVisible(!isSuperemitter);
	colorRandomGradient->setVisible(!isSuperemitter);
	colorOverLifeGradient->setVisible(!isSuperemitter);
	alphaOverLifeTimeLine->setVisible(!isSuperemitter);

	frameOverlifeCheckBox->setVisible(!isSuperemitter);
	frameOverlifeFPSSpin->setVisible(!isSuperemitter);
	frameOverlifeFPSLabel->setVisible(!isSuperemitter);
	randomFrameOnStartCheckBox->setVisible(!isSuperemitter);

	// The Pivot Point must be hidden for Superemitter mode.
	pivotPointLabel->setVisible(!isSuperemitter);
	pivotPointXSpinBox->setVisible(!isSuperemitter);
	pivotPointXSpinBoxLabel->setVisible(!isSuperemitter);
	pivotPointYSpinBox->setVisible(!isSuperemitter);
	pivotPointYSpinBoxLabel->setVisible(!isSuperemitter);
	pivotPointResetButton->setVisible(!isSuperemitter);

	// Some controls are however specific for this mode only - display and update them.
	innerEmitterLabel->setVisible(isSuperemitter);
	innerEmitterPathLabel->setVisible(isSuperemitter);
	
	if (isSuperemitter && this->layer->GetInnerEmitter())
	{
		innerEmitterPathLabel->setText(QString::fromStdString(layer->GetInnerEmitter()->GetConfigPath().GetAbsolutePathname()));
	}
}

void EmitterLayerWidget::OnPivotPointReset()
{
	blockSignals = true;
	this->pivotPointXSpinBox->setValue(0);
	this->pivotPointYSpinBox->setValue(0);
	blockSignals = false;
	
	OnValueChanged();
}

void EmitterLayerWidget::OnLayerValueChanged()
{
	// Start/End time and Enabled flag can be changed from external side.
	blockSignals = true;
	if (startTimeSpin->value() != layer->startTime || endTimeSpin->value() != layer->endTime)
	{
		startTimeSpin->setValue(layer->startTime);
		endTimeSpin->setValue(layer->endTime);
	}
	
	if (deltaSpin->value() != layer->deltaTime || loopEndSpin->value() != layer->loopEndTime)
	{
		deltaSpin->setValue(layer->deltaTime);
		loopEndSpin->setValue(layer->loopEndTime);
	}
	
	// NOTE: inverse logic here.
	if (enableCheckBox->isChecked() == layer->GetDisabled())
	{
		enableCheckBox->setChecked(!layer->GetDisabled());
	}
	
	blockSignals = false;
}
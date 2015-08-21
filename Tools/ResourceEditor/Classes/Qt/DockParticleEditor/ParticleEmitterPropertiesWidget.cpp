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


#include "ParticleEmitterPropertiesWidget.h"
#include "Commands2/ParticleEditorCommands.h"

#include <QLineEdit>
#include <QEvent>

#define EMISSION_RANGE_MIN_LIMIT_DEGREES 0.0f
#define EMISSION_RANGE_MAX_LIMIT_DEGREES 180.0f

ParticleEmitterPropertiesWidget::ParticleEmitterPropertiesWidget(QWidget* parent) :
	QWidget(parent),
	BaseParticleEditorContentWidget()
{
	mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);

	emitterNameLineEdit = new QLineEdit();
	mainLayout->addWidget(emitterNameLineEdit);
	connect(emitterNameLineEdit, SIGNAL(editingFinished()),this, SLOT(OnValueChanged()));

	emitterYamlPath = new QLineEdit(this);
	emitterYamlPath->setReadOnly(true);
	mainLayout->addWidget(emitterYamlPath);
	connect(emitterYamlPath, SIGNAL(textChanged(const QString&)), this, SLOT(OnEmitterYamlPathChanged(const QString&)));

	shortEffectCheckBox = new QCheckBox("Short effect");
	mainLayout->addWidget(shortEffectCheckBox);
	connect(shortEffectCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));

	QHBoxLayout* emitterTypeHBox = new QHBoxLayout();
	emitterTypeHBox->addWidget(new QLabel("type"));
	emitterType = new QComboBox(this);
	emitterType->addItem("Point");
	emitterType->addItem("Box");
	emitterType->addItem("Circle - Volume");
	emitterType->addItem("Circle - Edges");
	emitterType->addItem("Shockwave");
	emitterTypeHBox->addWidget(emitterType);
	mainLayout->addLayout(emitterTypeHBox);
	connect(emitterType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnValueChanged()));



	QHBoxLayout *positionLayout = new QHBoxLayout();
	
	positionLayout->addWidget(new QLabel("Position"));
	positionLayout->addStretch();
	positionLayout->addWidget(new QLabel("X:"));
	positionXSpinBox = new EventFilterDoubleSpinBox();
	positionXSpinBox->setMinimum(-100);
	positionXSpinBox->setMaximum(100);	
	positionXSpinBox->setSingleStep(0.1);
	positionXSpinBox->setDecimals(3);
	positionLayout->addWidget(positionXSpinBox);
	connect(positionXSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnEmitterPositionChanged()));

	positionLayout->addStretch();
	positionLayout->addWidget(new QLabel("Y:"));
	positionYSpinBox = new EventFilterDoubleSpinBox();
	positionYSpinBox->setMinimum(-100);
	positionYSpinBox->setMaximum(100);	
	positionYSpinBox->setSingleStep(0.1);
	positionYSpinBox->setDecimals(3);
	positionLayout->addWidget(positionYSpinBox);
	connect(positionYSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnEmitterPositionChanged()));

	positionLayout->addStretch();
	positionLayout->addWidget(new QLabel("Z:"));
	positionZSpinBox = new EventFilterDoubleSpinBox();
	positionZSpinBox->setMinimum(-100);
	positionZSpinBox->setMaximum(100);	
	positionZSpinBox->setSingleStep(0.1);
	positionZSpinBox->setDecimals(3);
	positionLayout->addWidget(positionZSpinBox);
	connect(positionZSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnEmitterPositionChanged()));
			
	mainLayout->addLayout(positionLayout);	

	emitterEmissionRange = new TimeLineWidget(this);
	InitWidget(emitterEmissionRange);

	emitterEmissionVector = new TimeLineWidget(this);
	InitWidget(emitterEmissionVector);

	emitterRadius = new TimeLineWidget(this);
	InitWidget(emitterRadius);

    emitterAngle = new  TimeLineWidget(this);
    InitWidget(emitterAngle);

	emitterColorWidget = new GradientPickerWidget(this);
	InitWidget(emitterColorWidget);

	emitterSize = new TimeLineWidget(this);
	InitWidget(emitterSize);

	QHBoxLayout *emitterLifeHBox = new QHBoxLayout();
	emitterLifeHBox->addWidget(new QLabel("life"));
	emitterLife = new EventFilterDoubleSpinBox(this);
	emitterLife->setMinimum(0.f);
	emitterLife->setMaximum(10000000);
	emitterLifeHBox->addWidget(emitterLife);
	mainLayout->addLayout(emitterLifeHBox);
	connect(emitterLife, SIGNAL(valueChanged(double)), this, SLOT(OnValueChanged()));
		

	Q_FOREACH( QAbstractSpinBox * sp, findChildren<QAbstractSpinBox*>() ) {
        sp->installEventFilter( this );
    }
	emitterYamlPath->installEventFilter(this);

	blockSignals = false;
}

ParticleEmitterPropertiesWidget::~ParticleEmitterPropertiesWidget()
{
}

void ParticleEmitterPropertiesWidget::InitWidget(QWidget *widget, bool connectWidget)
{
	mainLayout->addWidget(widget);
	if(connectWidget)
		connect(widget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
}

void ParticleEmitterPropertiesWidget::OnEmitterPositionChanged()
{
    if(blockSignals)
        return;

    DVASSERT(emitter != 0);
    DVASSERT(effect != 0);

    Vector3 position;
    position.x = positionXSpinBox->value();
    position.y = positionYSpinBox->value();
    position.z = positionZSpinBox->value();

    CommandUpdateEmitterPosition* commandUpdateEmitter = new CommandUpdateEmitterPosition(effect, emitter);
    commandUpdateEmitter->Init(position);        

    DVASSERT(activeScene != 0);
    activeScene->Exec(commandUpdateEmitter);
	activeScene->MarkAsChanged();

    Init(activeScene, effect, emitter, false, false);
    emit ValueChanged();
}

void ParticleEmitterPropertiesWidget::OnValueChanged()
{
	if(blockSignals)
		return;

	DVASSERT(emitter != 0);

	DVASSERT(emitterType->currentIndex() != -1);
	ParticleEmitter::eType type = (ParticleEmitter::eType)emitterType->currentIndex();

	PropLineWrapper<float32> emissionRange;
	if(!emitterEmissionRange->GetValue(0, emissionRange.GetPropsPtr()))
		return;

	PropLineWrapper<Vector3> emissionVector;
	if(!emitterEmissionVector->GetValues(emissionVector.GetPropsPtr()))
		return;

	PropLineWrapper<float32> radius;
	if(!emitterRadius->GetValue(0, radius.GetPropsPtr()))
		return;

	PropLineWrapper<Color> colorOverLife;
	if(!emitterColorWidget->GetValues(colorOverLife.GetPropsPtr()))
		return;

	PropLineWrapper<Vector3> size;
	if(!emitterSize->GetValues(size.GetPropsPtr()))
		return;


	float32 life = emitterLife->value();
	float32 currentLifeTime = emitter->lifeTime;
	bool initEmittersByDef = FLOAT_EQUAL(life,currentLifeTime) ? false : true;	

	bool isShortEffect = shortEffectCheckBox->isChecked();

    PropLineWrapper<float32> propAngle;
    PropLineWrapper<float32> propAngleVariation;
    emitterAngle->GetValue(0, propAngle.GetPropsPtr());
    emitterAngle->GetValue(1, propAngleVariation.GetPropsPtr());
	

	CommandUpdateEmitter* commandUpdateEmitter = new CommandUpdateEmitter(emitter);

	commandUpdateEmitter->Init(FastName(emitterNameLineEdit->text().toStdString().c_str()),
							   type,
							   emissionRange.GetPropLine(),
							   emissionVector.GetPropLine(),
							   radius.GetPropLine(),
                               propAngle.GetPropLine(),
                               propAngleVariation.GetPropLine(),
							   colorOverLife.GetPropLine(),
							   size.GetPropLine(),
							   life,
							   isShortEffect);

	DVASSERT(activeScene != 0);
	activeScene->Exec(commandUpdateEmitter);
	activeScene->MarkAsChanged();

	Init(activeScene, effect, emitter, false, initEmittersByDef);
	emit ValueChanged();
}

void ParticleEmitterPropertiesWidget::Init(SceneEditor2* scene, DAVA::ParticleEffectComponent *effect, DAVA::ParticleEmitter *emitter, bool updateMinimize, bool needUpdateTimeLimits)
{
	DVASSERT(emitter != 0);
	this->emitter = emitter;
    this->effect = effect;
	SetActiveScene(scene);

	blockSignals = true;

	emitterNameLineEdit->setText(QString::fromStdString(emitter->name.c_str()));
	shortEffectCheckBox->setChecked(emitter->shortEffect);

	float32 emitterLifeTime = emitter->lifeTime;

    
	float minTime		= 0.f;
	float minTimeLimit	= 0.f;
    
	float maxTime		= emitterLifeTime;
	float maxTimeLimit	= emitterLifeTime;
	emitterYamlPath->setText(QString::fromStdString(emitter->configPath.GetAbsolutePathname()));
	emitterType->setCurrentIndex(emitter->emitterType);

    int32 emitterId = effect->GetEmitterId(emitter);    
    Vector3 position = (emitterId==-1)?Vector3(0,0,0):effect->GetSpawnPosition(emitterId);
	positionXSpinBox->setValue((double)position.x);
	positionYSpinBox->setValue((double)position.y);
	positionZSpinBox->setValue((double)position.z);

	if(!needUpdateTimeLimits)
	{
		minTime = emitterEmissionRange->GetMinBoundary();
		maxTime = emitterEmissionRange->GetMaxBoundary();
	}
	emitterEmissionRange->Init(minTime, maxTime, minTimeLimit, maxTimeLimit, updateMinimize);
	emitterEmissionRange->AddLine(0, PropLineWrapper<float32>(PropertyLineHelper::GetValueLine(emitter->emissionRange)).GetProps(), Qt::blue, "emission range");
	emitterEmissionRange->SetMinLimits(EMISSION_RANGE_MIN_LIMIT_DEGREES);
	emitterEmissionRange->SetMaxLimits(EMISSION_RANGE_MAX_LIMIT_DEGREES);
	emitterEmissionRange->SetYLegendMark(DEGREE_MARK_CHARACTER);

	if(!needUpdateTimeLimits)
	{
		minTime = emitterEmissionVector->GetMinBoundary();
		maxTime = emitterEmissionVector->GetMaxBoundary();
	}
	emitterEmissionVector->Init(minTime, maxTime, minTimeLimit, maxTimeLimit, updateMinimize, true);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::red); vectorColors.push_back(Qt::darkGreen); vectorColors.push_back(Qt::blue);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("emission vector: x"); vectorLegends.push_back("emission vector: y"); vectorLegends.push_back("emission vector: z");
	emitterEmissionVector->AddLines(PropLineWrapper<Vector3>(PropertyLineHelper::GetValueLine(emitter->emissionVector)).GetProps(), vectorColors, vectorLegends);

	if(!needUpdateTimeLimits)
	{
		minTime = emitterRadius->GetMinBoundary();
		maxTime = emitterRadius->GetMaxBoundary();
	}
	emitterRadius->Init(minTime, maxTime, minTimeLimit, maxTimeLimit, updateMinimize);
	emitterRadius->AddLine(0, PropLineWrapper<float32>(PropertyLineHelper::GetValueLine(emitter->radius)).GetProps(), Qt::blue, "radius");
	// Radius cannot be negative.
	emitterRadius->SetMinLimits(0.0f);

    if(!needUpdateTimeLimits)
    {
        minTime = emitterAngle->GetMinBoundary();
        maxTime = emitterAngle->GetMaxBoundary();
    }
    emitterAngle->Init(minTime, maxTime, minTimeLimit, maxTimeLimit, updateMinimize);
    emitterAngle->AddLine(0, PropLineWrapper<float32>(PropertyLineHelper::GetValueLine(emitter->emissionAngle)).GetProps(), Qt::blue, "emission angle base");
    emitterAngle->AddLine(1, PropLineWrapper<float32>(PropertyLineHelper::GetValueLine(emitter->emissionAngleVariation)).GetProps(), Qt::green, "emission angle spread");
    emitterAngle->SetYLegendMark(DEGREE_MARK_CHARACTER);
    

	emitterColorWidget->Init(0.f, emitterLifeTime, "color over life");
	emitterColorWidget->SetValues(PropLineWrapper<Color>(PropertyLineHelper::GetValueLine(emitter->colorOverLife)).GetProps());

	if(!needUpdateTimeLimits)
	{
		minTime = emitterSize->GetMinBoundary();
		maxTime = emitterSize->GetMaxBoundary();
	}
	emitterSize->Init(minTime, maxTime, minTimeLimit, maxTimeLimit, updateMinimize, true);
	emitterSize->SetMinLimits(0);
	Vector<QColor> sizeColors;
	sizeColors.push_back(Qt::red); sizeColors.push_back(Qt::darkGreen); sizeColors.push_back(Qt::blue);
	Vector<QString> sizeLegends;
	sizeLegends.push_back("size: x"); sizeLegends.push_back("size: y"); sizeLegends.push_back("size: z");
	emitterSize->AddLines(PropLineWrapper<Vector3>(PropertyLineHelper::GetValueLine(emitter->size)).GetProps(), sizeColors, sizeLegends);
	emitterSize->EnableLock(true);
	
	emitterLife->setValue(emitterLifeTime);	

	blockSignals = false;
}

void ParticleEmitterPropertiesWidget::OnEmitterYamlPathChanged(const QString& newPath)
{
	UpdateTooltip();
}

void ParticleEmitterPropertiesWidget::RestoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	emitterEmissionRange->SetVisualState(visualStateProps->GetArchive("EMITTER_EMISSION_RANGE_PROPS"));
	emitterEmissionVector->SetVisualState(visualStateProps->GetArchive("EMITTER_EMISSION_VECTOR_PROPS"));
	emitterRadius->SetVisualState(visualStateProps->GetArchive("EMITTER_RADIUS_PROPS"));
    emitterAngle->SetVisualState(visualStateProps->GetArchive("EMITTER_ANGLE_PROPS"));
	emitterSize->SetVisualState(visualStateProps->GetArchive("EMITTER_SIZE_PROPS"));
}

void ParticleEmitterPropertiesWidget::StoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	KeyedArchive* props = new KeyedArchive();

	props->DeleteAllKeys();
	emitterEmissionRange->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_EMISSION_RANGE_PROPS", props);

	props->DeleteAllKeys();
	emitterEmissionVector->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_EMISSION_VECTOR_PROPS", props);

	props->DeleteAllKeys();
	emitterRadius->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_RADIUS_PROPS", props);

    props->DeleteAllKeys();
    emitterAngle->GetVisualState(props);
    visualStateProps->SetArchive("EMITTER_ANGLE_PROPS", props);

	props->DeleteAllKeys();
	emitterSize->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_SIZE_PROPS", props);

	SafeRelease(props);
}

void ParticleEmitterPropertiesWidget::Update()
{
	Init(activeScene, effect, emitter, false);
}

bool ParticleEmitterPropertiesWidget::eventFilter(QObject * o, QEvent * e)
{
    if (e->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(o))
    {
        e->ignore();
        return true;
    }

	if (e->type() == QEvent::Resize && qobject_cast<QLineEdit*>(o))
	{
		UpdateTooltip();
		return true;
	}

    return QWidget::eventFilter(o, e);
}

void ParticleEmitterPropertiesWidget::UpdateTooltip()
{
	QFontMetrics fm = emitterYamlPath->fontMetrics();
	if (fm.width(emitterYamlPath->text()) >= emitterYamlPath->width())
	{
		emitterYamlPath->setToolTip(emitterYamlPath->text());
	}
	else
	{
		emitterYamlPath->setToolTip("");
	}
}
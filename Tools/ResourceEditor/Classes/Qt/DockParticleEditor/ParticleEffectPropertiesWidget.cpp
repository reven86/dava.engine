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


#include "ParticleEffectPropertiesWidget.h"
#include "Commands2/ParticleEditorCommands.h"

#include <QLineEdit>
#include <QEvent>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>


#include "Scene3D/Systems/ParticleEffectSystem.h"

static const int TreeItemTypeEmitter  = QTreeWidgetItem::UserType+1;
static const int TreeItemTypeLayer    = QTreeWidgetItem::UserType+2;
static const int TreeItemTypeForce    = QTreeWidgetItem::UserType+3;
static const int TreeItemTypeExternal = QTreeWidgetItem::UserType+4;

ParticleEffectPropertiesWidget::ParticleEffectPropertiesWidget(QWidget* parent) :
	QWidget(parent),
	BaseParticleEditorContentWidget()	
{	
	mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignTop);
	this->setLayout(mainLayout);
	
	effectPlaybackSpeedLabel = new QLabel("effect playback speed");
	mainLayout->addWidget(effectPlaybackSpeedLabel);
	
	effectPlaybackSpeed = new QSlider(Qt::Horizontal, this);
	effectPlaybackSpeed->setTracking(true);
	effectPlaybackSpeed->setRange(0, 4); // 25%, 50%, 100%, 200%, 400% - 5 values total.
	effectPlaybackSpeed->setTickPosition(QSlider::TicksBelow);
	effectPlaybackSpeed->setTickInterval(1);
	effectPlaybackSpeed->setSingleStep(1);
	mainLayout->addWidget(effectPlaybackSpeed);
	
	QHBoxLayout *playerBox = new QHBoxLayout();
	playBtn = new QPushButton(QIcon(":/QtIcons/play.png"), "");	
	playBtn->setToolTip("Play");
	playerBox->addWidget(playBtn);
	stopBtn = new QPushButton(QIcon(":/QtIcons/stop.png"), "");	
	stopBtn->setToolTip("Stop");
	playerBox->addWidget(stopBtn);
	stopAndDeleteBtn = new QPushButton(QIcon(":/QtIcons/stop_clear.png"), "");	
	stopAndDeleteBtn->setToolTip("Stop and delete particles");
	playerBox->addWidget(stopAndDeleteBtn);
	pauseBtn = new QPushButton(QIcon(":/QtIcons/pause.png"), "");	
	pauseBtn->setToolTip("Pause");
	playerBox->addWidget(pauseBtn);
	restartBtn = new QPushButton(QIcon(":/QtIcons/restart.png"), "");	
	restartBtn->setToolTip("Restart");
	playerBox->addWidget(restartBtn);	
	stepForwardBtn = new QPushButton(QIcon(":/QtIcons/step_forward.png"), "");	
	stepForwardBtn->setToolTip("Step forward");
	playerBox->addWidget(stepForwardBtn);	
	stepForwardFPSSpin = new QSpinBox(this);
	stepForwardFPSSpin->setMinimum(1);
	stepForwardFPSSpin->setMaximum(100);
	stepForwardFPSSpin->setValue(30);
	playerBox->addWidget(stepForwardFPSSpin);
	playerBox->addWidget(new QLabel("step FPS"));
	playerBox->addStretch();

	connect(playBtn,SIGNAL(clicked(bool)), this, SLOT(OnPlay()));
	connect(stopBtn,SIGNAL(clicked(bool)), this, SLOT(OnStop()));
	connect(stopAndDeleteBtn,SIGNAL(clicked(bool)), this, SLOT(OnStopAndDelete()));
	connect(pauseBtn,SIGNAL(clicked(bool)), this, SLOT(OnPause()));
	connect(restartBtn,SIGNAL(clicked(bool)), this, SLOT(OnRestart()));
	connect(stepForwardBtn,SIGNAL(clicked(bool)), this, SLOT(OnStepForward()));

	mainLayout->addLayout(playerBox);

	connect(effectPlaybackSpeed, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));

	effectTree = new QTreeWidget();
	mainLayout->addWidget(effectTree);
	effectTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(effectTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuForEffectTree(const QPoint&)));
	connect(effectTree, SIGNAL(itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT(OnTreeItemDoubleClck(QTreeWidgetItem*, int)));
	iconEmitter = QIcon(":/QtIcons/emitter_particle.png");
	iconLayer = QIcon(":/QtIcons/layer_particle.png");
	iconForce = QIcon(":/QtIcons/force.png");
	iconExternal = QIcon(":/QtIcons/external.png");

	mainLayout->addWidget(new QLabel("Effect Variables"));
	effectVariables = new QTableWidget(this);
	effectVariables->setColumnCount(2);
	effectVariables->setRowCount(0);
	effectEditDelegate = new VariableEditDelegate(effectVariables, effectVariables);
	effectVariables->setItemDelegate(effectEditDelegate);
	mainLayout->addWidget(effectVariables);
	connect(effectVariables, SIGNAL(cellChanged(int, int)), this, SLOT(OnVariableValueChanged(int, int)));

	mainLayout->addWidget(new QLabel("Global variables"));
	globalVariables = new QTableWidget(this);
	globalVariables->setColumnCount(2);
	globalVariables->setRowCount(0);	
	globalEditDelegate = new VariableEditDelegate(globalVariables, globalVariables);
	globalVariables->setItemDelegate(globalEditDelegate);
	mainLayout->addWidget(globalVariables);
	connect(globalVariables, SIGNAL(cellChanged(int, int)), this, SLOT(OnGlobalVariableValueChanged(int, int)));


	QHBoxLayout *addGlobalBox = new QHBoxLayout();
	addGlobalBox->addStretch();
	QPushButton *addGlobal = new QPushButton(this);
	addGlobal->setText("+");
	addGlobal->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	addGlobalBox->addWidget(addGlobal);
	mainLayout->addLayout(addGlobalBox);

	connect(addGlobal, SIGNAL(clicked(bool)), this, SLOT(OnAddGlobalExternal()));

	mainLayout->addStretch();

	particleEffect = NULL;
	blockSignals = false;
	blockTables = false;
}

ParticleEffectPropertiesWidget::~ParticleEffectPropertiesWidget()
{
}

void ParticleEffectPropertiesWidget::InitWidget(QWidget *widget, bool connectWidget)
{
	mainLayout->addWidget(widget);
	if(connectWidget)
		connect(widget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
}

void ParticleEffectPropertiesWidget::UpdateVaribleTables()
{
	blockTables = true;
	Set<String> variablesSet = particleEffect->EnumerateVariables();
	effectVariables->clearContents();
	effectVariables->setRowCount(variablesSet.size());
	int32 i=0;
	for (Set<String>::iterator it = variablesSet.begin(), e = variablesSet.end(); it!=e; ++it)
	{
		
		QTableWidgetItem *varName = new QTableWidgetItem(QString((*it).c_str()));
		varName->setFlags(Qt::NoItemFlags);
		effectVariables->setItem(i, 0, varName);		
		QTableWidgetItem *varValue = new QTableWidgetItem(QString::number(particleEffect->GetExternalValue(*it)));		
		effectVariables->setItem(i, 1, varValue);
		i++;
	}


	Map<String, float32> globalVariablesSet = particleEffect->GetEntity()->GetScene()->particleEffectSystem->GetGlobalExternals();
	globalVariables->clearContents();
	globalVariables->setRowCount(globalVariablesSet.size());
	i=0;
	for (Map<String, float32>::iterator it = globalVariablesSet.begin(), e = globalVariablesSet.end(); it!=e; ++it)
	{		
		QTableWidgetItem *varName = new QTableWidgetItem(QString((*it).first.c_str()));
		varName->setFlags(Qt::NoItemFlags);
		globalVariables->setItem(i, 0, varName);		
		QTableWidgetItem *varValue = new QTableWidgetItem(QString::number((*it).second));		
		globalVariables->setItem(i, 1, varValue);
		i++;
	}

	blockTables = false;
}

void ParticleEffectPropertiesWidget::OnVariableValueChanged(int row, int col)
{
	if (blockTables)
		return;
	String varNam = effectVariables->item(row, 0)->text().toStdString();	
	float varValue = effectVariables->item(row, 1)->text().toFloat();
	particleEffect->SetExtertnalValue(varNam, varValue);
}

void ParticleEffectPropertiesWidget::OnGlobalVariableValueChanged(int row, int col)
{
	if (blockTables)
		return;
	String varNam = globalVariables->item(row, 0)->text().toStdString();	
	float varValue = globalVariables->item(row, 1)->text().toFloat();
	particleEffect->GetEntity()->GetScene()->particleEffectSystem->SetGlobalExtertnalValue(varNam, varValue);
	UpdateVaribleTables();
}

void ParticleEffectPropertiesWidget::OnAddGlobalExternal()
{
	AddGlobalExternalDialog dialog(this);	
	if (dialog.exec())
	{			
		particleEffect->GetEntity()->GetScene()->particleEffectSystem->SetGlobalExtertnalValue(dialog.GetVariableName(), dialog.GetVariableValue());
		UpdateVaribleTables();		
	}	
}

void ParticleEffectPropertiesWidget::ShowContextMenuForEffectTree(const QPoint &pos)
{
	QMenu contextMenu;
	QTreeWidgetItem *target = effectTree->itemAt(pos);	
	if (!target) 
		return;
	currSelectedTreeItem = target;
	EffectTreeData treeData = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();	
	int i;
	switch(currSelectedTreeItem->type())
	{
	case TreeItemTypeEmitter:		
		for (i=0; i<EE_TOTAL; ++i)
		{
			if (!GetEmitterLine(treeData.emmiter, EmitterExternals(i)))
			{
				contextMenu.addAction(QString("Add External ")+QString(EXTERNAL_NAMES[i].c_str()))->setData(QVariant(i));
			}
		}		
		break;
	case TreeItemTypeLayer:
		for (i=EE_TOTAL; i<EL_TOTAL; ++i)
		{
			if (!GetLayerLine(treeData.layer, LayerExternals(i)))
			{
				contextMenu.addAction(QString("Add External ")+QString(EXTERNAL_NAMES[i].c_str()))->setData(QVariant(i));
			}
		}
		break;
	case TreeItemTypeForce:
		for (i=EL_TOTAL; i<EF_TOTAL; ++i)
		{
			if (!GetForceLine(treeData.force, ForceExternals(i)))
			{
				contextMenu.addAction(QString("Add External ")+QString(EXTERNAL_NAMES[i].c_str()))->setData(QVariant(i));
			}
		}
		break;
	case TreeItemTypeExternal:
		contextMenu.addAction(QString("Remove External ")+QString(EXTERNAL_NAMES[treeData.externalParamId].c_str()))->setData(QVariant(-1));
	}
	connect(&contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(OnContextMenuCommand(QAction*)));
	contextMenu.exec(effectTree->viewport()->mapToGlobal(pos));
}

void ParticleEffectPropertiesWidget::OnTreeItemDoubleClck(QTreeWidgetItem *treeItem, int column)
{
	EffectTreeData data = treeItem->data(0, Qt::UserRole).value<EffectTreeData>();
	if (treeItem->type() == TreeItemTypeExternal) //edit external on double click
	{
		int externalId = data.externalParamId;
		if (externalId<EE_TOTAL)
		{			
			EditEmitterModifiable(data.emmiter, EmitterExternals(externalId));			
		}
		else if (externalId<EL_TOTAL)
		{			
			EditLayerModifiable(data.layer, LayerExternals(externalId));
		}
		else
		{				
			EditForceModifiable(data.force, ForceExternals(externalId));		
		}
	}
}

void ParticleEffectPropertiesWidget::OnContextMenuCommand(QAction *action)
{
	int commandId = action->data().toInt();
	if (commandId==-1)
	{
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		int externalId = data.externalParamId;
		if (externalId<EE_TOTAL)
		{			
			particleEffect->UnRegisterModifiable(GetEmitterLine(data.emmiter, EmitterExternals(externalId)));
			RemoveEmitterLineModifiable(data.emmiter, EmitterExternals(externalId));			
			UpdateVaribleTables();
		}
		else if (externalId<EL_TOTAL)
		{			
			particleEffect->UnRegisterModifiable(GetLayerLine(data.layer, LayerExternals(externalId)));
			RemoveLayerLineModifiable(data.layer, LayerExternals(externalId));
			UpdateVaribleTables();
		}
		else
		{		
			particleEffect->UnRegisterModifiable(GetForceLine(data.force, ForceExternals(externalId)));
			RemoveForceLineModifiable(data.force, ForceExternals(externalId));			
			UpdateVaribleTables();
		}
		delete currSelectedTreeItem;
	}
	else if (commandId<EE_TOTAL)
	{
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		SetEmitterLineModifiable(data.emmiter, EmitterExternals(commandId));		
		if (EditEmitterModifiable(data.emmiter, EmitterExternals(commandId), true))		
		{					
			data.externalParamId = commandId;
			QTreeWidgetItem *externalItem = new QTreeWidgetItem(currSelectedTreeItem, TreeItemTypeExternal);
			externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[commandId].c_str()));
			externalItem->setIcon(0, iconExternal);
			externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));		
		}
		else
		{
			RemoveEmitterLineModifiable(data.emmiter, EmitterExternals(commandId));
		}
		
	}
	else if (commandId<EL_TOTAL)
	{
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		SetLayerLineModifiable(data.layer, LayerExternals(commandId));	
		if (EditLayerModifiable(data.layer, LayerExternals(commandId), true))
		{			
			data.externalParamId = commandId;
			QTreeWidgetItem *externalItem = new QTreeWidgetItem(currSelectedTreeItem, TreeItemTypeExternal);
			externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[commandId].c_str()));
			externalItem->setIcon(0, iconExternal);
			externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
		}
		else
		{
			RemoveLayerLineModifiable(data.layer, LayerExternals(commandId));
		}				
	}
	else
	{	
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		SetForceLineModifiable(data.force, ForceExternals(commandId));		

		particleEffect->RegisterModifiable(GetForceLine(data.force, ForceExternals(commandId)));
		UpdateVaribleTables();

		if (EditForceModifiable(data.force, ForceExternals(commandId), true))
		{			
			data.externalParamId = commandId;
			QTreeWidgetItem *externalItem = new QTreeWidgetItem(currSelectedTreeItem, TreeItemTypeExternal);
			externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[commandId].c_str()));
			externalItem->setIcon(0, iconExternal);
			externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
		}
		else
		{
			RemoveForceLineModifiable(data.force, ForceExternals(commandId));
		}
	}
}

void ParticleEffectPropertiesWidget::OnValueChanged()
{
	if(blockSignals)
		return;
	
	DVASSERT(particleEffect != 0);
	float playbackSpeed = ConvertFromSliderValueToPlaybackSpeed(effectPlaybackSpeed->value());	

	CommandUpdateEffect* commandUpdateEffect = new CommandUpdateEffect(particleEffect);
	commandUpdateEffect->Init(playbackSpeed);

	DVASSERT(activeScene != 0);
	activeScene->Exec(commandUpdateEffect);
	activeScene->MarkAsChanged();

	Init(activeScene, particleEffect);
}

void ParticleEffectPropertiesWidget::OnPlay()
{
	DVASSERT(particleEffect != 0);
	particleEffect->Start();
}
void ParticleEffectPropertiesWidget::OnStop()
{
	DVASSERT(particleEffect != 0);
	particleEffect->Stop(false);
}
void ParticleEffectPropertiesWidget::OnStopAndDelete()
{
	DVASSERT(particleEffect != 0);
	particleEffect->Stop(true);
}
void ParticleEffectPropertiesWidget::OnPause()
{
	DVASSERT(particleEffect != 0);
	particleEffect->Pause(!particleEffect->IsPaused());
}
void ParticleEffectPropertiesWidget::OnRestart()
{
	DVASSERT(particleEffect != 0);
	particleEffect->Restart();
}
void ParticleEffectPropertiesWidget::OnStepForward()
{
	DVASSERT(particleEffect != 0);
	float32 step = 1.0f/(float32)stepForwardFPSSpin->value();
	particleEffect->Step(step);
}

void ParticleEffectPropertiesWidget::Init(SceneEditor2* scene, DAVA::ParticleEffectComponent *effect)
{
	DVASSERT(effect != 0);
	this->particleEffect = effect;
	this->emitter = NULL;
	SetActiveScene(scene);

	blockSignals = true;

	// Normalize Playback Speed to the UISlider range.
	float32 playbackSpeed = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeed->setValue(ConvertFromPlaybackSpeedToSliderValue(playbackSpeed));
	UpdatePlaybackSpeedLabel();
	BuildEffectTree();
	UpdateVaribleTables();
	blockSignals = false;
}


ModifiablePropertyLineBase* ParticleEffectPropertiesWidget::GetEmitterLine(ParticleEmitter *emitter, EmitterExternals lineId)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		return dynamic_cast<ModifiablePropertyLineBase*>(emitter->emissionVector.Get());
	case EE_EMISSION_RANGE:
		return dynamic_cast<ModifiablePropertyLineBase*>(emitter->emissionRange.Get());
	case EE_RADUS:
		return dynamic_cast<ModifiablePropertyLineBase*>(emitter->radius.Get());
	case EE_SIZE:
		return dynamic_cast<ModifiablePropertyLineBase*>(emitter->size.Get());
	case EE_COLOR_OVER_LIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(emitter->colorOverLife.Get());
            
    default: break;
	}
	return NULL;
}
ModifiablePropertyLineBase* ParticleEffectPropertiesWidget::GetLayerLine(ParticleLayer *layer, LayerExternals lineId)
{
	switch (lineId)
	{
	case EL_LIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->life.Get());
	case EL_LIFE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->lifeVariation.Get());	
	case EL_NUMBER:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->number.Get());
	case EL_NUMBER_VARIATION:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->numberVariation.Get());
	case EL_SIZE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->size.Get());
	case EL_SIZE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->sizeVariation.Get());
	case EL_SIZE_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->sizeOverLifeXY.Get());
	case EL_VELOCITY:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->velocity.Get());
	case EL_VELOCITY_VARIATON:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->velocityVariation.Get());
	case EL_VELOCITY_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->velocityOverLife.Get());
	case EL_SPIN:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->spin.Get());
	case EL_SPIN_VARIATION:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->spinVariation.Get());
	case EL_SPIN_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->spinOverLife.Get());
	case EL_COLOR:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->colorRandom.Get());
	case EL_ALPHA_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->alphaOverLife.Get());
	case EL_COLOR_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->colorOverLife.Get());
	case EL_ANGLE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->angle.Get());
	case EL_ANGLE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->angleVariation.Get());
	case EL_ANIM_SPEED_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(layer->animSpeedOverLife.Get());

    default: break;
	}
	return NULL;
}
ModifiablePropertyLineBase* ParticleEffectPropertiesWidget::GetForceLine(ParticleForce *force, ForceExternals lineId)
{
	switch(lineId)
	{
	case EF_FORCE:
		return dynamic_cast<ModifiablePropertyLineBase*>(force->force.Get());
	case EF_FORCE_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineBase*>(force->forceOverLife.Get());
        default: break;
	}
	return NULL;
}

void ParticleEffectPropertiesWidget::SetEmitterLineModifiable(ParticleEmitter *emitter, EmitterExternals lineId)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		PropertyLineHelper::MakeModifiable(emitter->emissionVector); break;
	case EE_EMISSION_RANGE:
		PropertyLineHelper::MakeModifiable(emitter->emissionRange); break;
	case EE_RADUS:
		PropertyLineHelper::MakeModifiable(emitter->radius); break;
	case EE_SIZE:
		PropertyLineHelper::MakeModifiable(emitter->size); break;
	case EE_COLOR_OVER_LIFE:
		PropertyLineHelper::MakeModifiable(emitter->colorOverLife); break;

        default: break;
	}
}
void ParticleEffectPropertiesWidget::SetLayerLineModifiable(ParticleLayer *layer, LayerExternals lineId)
{
	switch (lineId)
	{
	case EL_LIFE:
		PropertyLineHelper::MakeModifiable(layer->life); break;
	case EL_LIFE_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->lifeVariation); break;
	case EL_NUMBER:
		PropertyLineHelper::MakeModifiable(layer->number); break;
	case EL_NUMBER_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->numberVariation); break;
	case EL_SIZE:
		PropertyLineHelper::MakeModifiable(layer->size); break;
	case EL_SIZE_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->sizeVariation); break;
	case EL_SIZE_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->sizeOverLifeXY); break;
	case EL_VELOCITY:
		PropertyLineHelper::MakeModifiable(layer->velocity); break;
	case EL_VELOCITY_VARIATON:
		PropertyLineHelper::MakeModifiable(layer->velocityVariation); break;
	case EL_VELOCITY_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->velocityOverLife); break;
	case EL_SPIN:
		PropertyLineHelper::MakeModifiable(layer->spin); break;
	case EL_SPIN_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->spinVariation); break;
	case EL_SPIN_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->spinOverLife); break;
	case EL_COLOR:
		PropertyLineHelper::MakeModifiable(layer->colorRandom); break;
	case EL_ALPHA_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->alphaOverLife); break;
	case EL_COLOR_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->colorOverLife); break;
	case EL_ANGLE:
		PropertyLineHelper::MakeModifiable(layer->angle); break;
	case EL_ANGLE_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->angleVariation); break;
	case EL_ANIM_SPEED_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->animSpeedOverLife); break;

        default: break;
	}
}
void ParticleEffectPropertiesWidget::SetForceLineModifiable(ParticleForce *force, ForceExternals lineId)
{
	switch(lineId)
	{
	case EF_FORCE:
		PropertyLineHelper::MakeModifiable(force->force); break;
	case EF_FORCE_OVERLIFE:
		PropertyLineHelper::MakeModifiable(force->forceOverLife); break;
            
        default: break;
	}
}

void ParticleEffectPropertiesWidget::RemoveEmitterLineModifiable(ParticleEmitter *emitter, EmitterExternals lineId)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		PropertyLineHelper::RemoveModifiable(emitter->emissionVector); break;
	case EE_EMISSION_RANGE:
		PropertyLineHelper::RemoveModifiable(emitter->emissionRange); break;
	case EE_RADUS:
		PropertyLineHelper::RemoveModifiable(emitter->radius); break;
	case EE_SIZE:
		PropertyLineHelper::RemoveModifiable(emitter->size); break;
	case EE_COLOR_OVER_LIFE:
		PropertyLineHelper::RemoveModifiable(emitter->colorOverLife); break;
            
        default: break;
	}
}
void ParticleEffectPropertiesWidget::RemoveLayerLineModifiable(ParticleLayer *layer, LayerExternals lineId)
{
	switch (lineId)
	{
	case EL_LIFE:
		PropertyLineHelper::RemoveModifiable(layer->life); break;
	case EL_LIFE_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->lifeVariation); break;
	case EL_NUMBER:
		PropertyLineHelper::RemoveModifiable(layer->number); break;
	case EL_NUMBER_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->numberVariation); break;
	case EL_SIZE:
		PropertyLineHelper::RemoveModifiable(layer->size); break;
	case EL_SIZE_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->sizeVariation); break;
	case EL_SIZE_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->sizeOverLifeXY); break;
	case EL_VELOCITY:
		PropertyLineHelper::RemoveModifiable(layer->velocity); break;
	case EL_VELOCITY_VARIATON:
		PropertyLineHelper::RemoveModifiable(layer->velocityVariation); break;
	case EL_VELOCITY_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->velocityOverLife); break;
	case EL_SPIN:
		PropertyLineHelper::RemoveModifiable(layer->spin); break;
	case EL_SPIN_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->spinVariation); break;
	case EL_SPIN_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->spinOverLife); break;
	case EL_COLOR:
		PropertyLineHelper::RemoveModifiable(layer->colorRandom); break;
	case EL_ALPHA_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->alphaOverLife); break;
	case EL_COLOR_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->colorOverLife); break;
	case EL_ANGLE:
		PropertyLineHelper::RemoveModifiable(layer->angle); break;
	case EL_ANGLE_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->angleVariation); break;
	case EL_ANIM_SPEED_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->animSpeedOverLife); break;
            
        default: break;
	}
}
void ParticleEffectPropertiesWidget::RemoveForceLineModifiable(ParticleForce *force, ForceExternals lineId)
{
	switch(lineId)
	{
	case EF_FORCE:
		PropertyLineHelper::RemoveModifiable(force->force); break;
	case EF_FORCE_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(force->forceOverLife); break;
            
        default: break;
	}
}


bool ParticleEffectPropertiesWidget::EditEmitterModifiable(ParticleEmitter *emitter, EmitterExternals lineId, bool onAdd)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		return EditModificationLine(emitter->emissionVector, onAdd); break;
	case EE_EMISSION_RANGE:
		return EditModificationLine(emitter->emissionRange, onAdd); break;
	case EE_RADUS:
		return EditModificationLine(emitter->radius, onAdd); break;
	case EE_SIZE:
		return EditModificationLine(emitter->size, onAdd); break;
	case EE_COLOR_OVER_LIFE:
		return EditModificationLine(emitter->colorOverLife, onAdd); break;
    default: break;
	}
	return false;
}
bool ParticleEffectPropertiesWidget::EditLayerModifiable(ParticleLayer *layer, LayerExternals lineId, bool onAdd)
{
	switch (lineId)
	{
	case EL_LIFE:
		return EditModificationLine(layer->life, onAdd); break;
	case EL_LIFE_VARIATION:
		return EditModificationLine(layer->lifeVariation, onAdd); break;
	case EL_NUMBER:
		return EditModificationLine(layer->number, onAdd); break;
	case EL_NUMBER_VARIATION:
		return EditModificationLine(layer->numberVariation, onAdd); break;
	case EL_SIZE:
		return EditModificationLine(layer->size, onAdd); break;
	case EL_SIZE_VARIATION:
		return EditModificationLine(layer->sizeVariation, onAdd); break;
	case EL_SIZE_OVERLIFE:
		return EditModificationLine(layer->sizeOverLifeXY, onAdd); break;
	case EL_VELOCITY:
		return EditModificationLine(layer->velocity, onAdd); break;
	case EL_VELOCITY_VARIATON:
		return EditModificationLine(layer->velocityVariation, onAdd); break;
	case EL_VELOCITY_OVERLIFE:
		return EditModificationLine(layer->velocityOverLife, onAdd); break;
	case EL_SPIN:
		return EditModificationLine(layer->spin, onAdd); break;
	case EL_SPIN_VARIATION:
		return EditModificationLine(layer->spinVariation, onAdd); break;
	case EL_SPIN_OVERLIFE:
		return EditModificationLine(layer->spinOverLife, onAdd); break;
	case EL_COLOR:
		return EditModificationLine(layer->colorRandom, onAdd); break;
	case EL_ALPHA_OVERLIFE:
		return EditModificationLine(layer->alphaOverLife, onAdd); break;
	case EL_COLOR_OVERLIFE:
		return EditModificationLine(layer->colorOverLife, onAdd); break;
	case EL_ANGLE:
		return EditModificationLine(layer->angle, onAdd); break;
	case EL_ANGLE_VARIATION:
		return EditModificationLine(layer->angleVariation, onAdd); break;
	case EL_ANIM_SPEED_OVERLIFE:
		return EditModificationLine(layer->animSpeedOverLife, onAdd); break;
    default: break;
	}
	return false;
}
bool ParticleEffectPropertiesWidget::EditForceModifiable(ParticleForce *force, ForceExternals lineId, bool onAdd)
{
	switch(lineId)
	{
	case EF_FORCE:
		return EditModificationLine(force->force, onAdd); break;
	case EF_FORCE_OVERLIFE:
		return EditModificationLine(force->forceOverLife, onAdd); break;
    default: break;
	}
	return false;
}


void ParticleEffectPropertiesWidget::BuildEffectTree()
{
	currSelectedTreeItem = NULL;
	effectTree->clear();	
	effectTree->setHeaderLabel(QString(particleEffect->GetEntity()->GetName().c_str()));
	QTreeWidgetItem *root = effectTree->invisibleRootItem();
	EffectTreeData data;
	int32 childrenCount = particleEffect->GetEmittersCount();
	for (int32 emitterId = 0; emitterId < childrenCount; emitterId++)
	{
		
		ParticleEmitter * emitter = particleEffect->GetEmitter(emitterId);
		data.emmiter = emitter;
		QTreeWidgetItem *emitterItem = new QTreeWidgetItem(root, TreeItemTypeEmitter);
		emitterItem->setText(0, QString(emitter->name.c_str()));
		emitterItem->setIcon(0, iconEmitter);						
		emitterItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
		//externals
		for (int32 externalId = 0; externalId<EE_TOTAL; ++externalId)
		{
			if (GetEmitterLine(emitter, EmitterExternals(externalId)))
			{
				data.externalParamId = externalId;
				QTreeWidgetItem *externalItem = new QTreeWidgetItem(emitterItem, TreeItemTypeExternal);
				externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[externalId].c_str()));
				externalItem->setIcon(0, iconExternal);
				externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
			}
		}
		data.externalParamId=0;
		//layers
		int32 numLayers = emitter->layers.size();
		for (int32 layerId = 0; layerId<numLayers; ++layerId)
		{
			ParticleLayer* layer = emitter->layers[layerId];
			QTreeWidgetItem *layerItem = new QTreeWidgetItem(emitterItem, TreeItemTypeLayer);
			data.layer = layer;
			layerItem->setText(0, QString(layer->layerName.c_str()));
			layerItem->setIcon(0, iconLayer);
			layerItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
			//externals
			for (int32 externalId = EE_TOTAL; externalId<EL_TOTAL; ++externalId)
			{
				if (GetLayerLine(layer, LayerExternals(externalId)))
				{
					data.externalParamId = externalId;
					QTreeWidgetItem *externalItem = new QTreeWidgetItem(layerItem, TreeItemTypeExternal);
					externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[externalId].c_str()));
					externalItem->setIcon(0, iconExternal);
					externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
				}
			}
			data.externalParamId=0;
			//forces
			int32 numForces = layer->forces.size();
			for (int32 forceId=0; forceId<numForces; ++forceId)
			{
				ParticleForce* force = layer->forces[forceId];
				data.force = force;
				QTreeWidgetItem *forceItem = new QTreeWidgetItem(layerItem, TreeItemTypeForce);
				forceItem->setText(0, QString("force"));
				forceItem->setIcon(0, iconForce);
				forceItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
				//externals
				for (int32 externalId = EL_TOTAL; externalId<EF_TOTAL; ++externalId)
				{
					if (GetForceLine(force, ForceExternals(externalId)))
					{
						data.externalParamId = externalId;
						QTreeWidgetItem *externalItem = new QTreeWidgetItem(forceItem, TreeItemTypeExternal);
						externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[externalId].c_str()));
						externalItem->setIcon(0, iconExternal);
						externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
					}
				}
				data.externalParamId=0;
			}				
			data.force = NULL;
		}			
		data.layer = NULL;		
	}
	particleEffect->RebuildEffectModifiables();
}

void ParticleEffectPropertiesWidget::UpdatePlaybackSpeedLabel()
{
	if (!particleEffect)
	{
		return;
	}
	
	float32 playbackSpeedValue = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeedLabel->setText(QString("playback speed: %1x").arg(playbackSpeedValue));
}

void ParticleEffectPropertiesWidget::StoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to store for now.
}

void ParticleEffectPropertiesWidget::RestoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to restore for now.
}

AddGlobalExternalDialog::AddGlobalExternalDialog(QWidget *parent) : QDialog(parent)
{
	setMinimumWidth(200);
	QVBoxLayout *dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	QHBoxLayout *nameLayot = new QHBoxLayout();	
	nameLayot->addWidget(new QLabel("Name"));
	variableName = new QLineEdit();
	variableName->setText("Variable");
	nameLayot->addWidget(variableName);
	nameLayot->addStretch();
	nameLayot->addWidget(new QLabel("Value"));
	variableValue = new QDoubleSpinBox();	
	variableValue->setMinimum(0.0f);
	variableValue->setMaximum(1.0f);
	variableValue->setDecimals(3);
	variableValue->setSingleStep(0.001f);
	variableValue->setValue(0);
	nameLayot->addWidget(variableValue);
	dialogLayout->addLayout(nameLayot);
	dialogLayout->addStretch();
	variableName->setFocus();
	variableName->selectAll();
	
	QHBoxLayout* btnBox = new QHBoxLayout();
	QPushButton* btnCancel = new QPushButton("Cancel", this);
	QPushButton* btnOk = new QPushButton("Ok", this);
	btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	btnBox->addWidget(btnCancel);
	btnBox->addStretch();
	btnBox->addWidget(btnOk);
	dialogLayout->addLayout(btnBox);	
	connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
	btnOk->setDefault(true);
}	

String AddGlobalExternalDialog::GetVariableName()
{
	return variableName->text().toStdString();
}
float32 AddGlobalExternalDialog::GetVariableValue()
{
	return (float32) variableValue->value();
}

void EditModificationLineDialog::InitName(const String& name, bool onAdd)
{
	QHBoxLayout *nameLayot = new QHBoxLayout();
	QLabel *varNameLabel = new QLabel("External Variable Name");
	nameLayot->addWidget(varNameLabel);
	variableName = new QLineEdit();
	variableName->setText(QString(name.c_str()));
	nameLayot->addWidget(variableName);
	dialogLayout->addLayout(nameLayot);
	if (onAdd)
	{
		variableName->setFocus();
		variableName->selectAll();
	}
}

String EditModificationLineDialog::GetVariableName()
{
	return variableName->text().toStdString();
}

void EditModificationLineDialog::InitButtons()
{
	dialogLayout->addStretch();
	QHBoxLayout* btnBox = new QHBoxLayout();
	QPushButton* btnCancel = new QPushButton("Cancel", this);
	QPushButton* btnOk = new QPushButton("Ok", this);
	btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	btnBox->addWidget(btnCancel);
	btnBox->addStretch();
	btnBox->addWidget(btnOk);
	dialogLayout->addLayout(btnBox);	
	connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
	btnOk->setDefault(true);
}

template <> void EditModificationLineDialog::Init<float32>(ModifiablePropertyLine<float32>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	timeLine = new TimeLineWidget(this);	
	timeLine->Init(0.0f, 1.0f, false);
	timeLine->AddLine(0, PropLineWrapper<float32>(line->GetModificationLine()).GetProps(), Qt::blue, "");	
	dialogLayout->addWidget(timeLine);
	InitButtons();
	
}

template <> void EditModificationLineDialog::Init<Vector2>(ModifiablePropertyLine<Vector2>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	timeLine = new TimeLineWidget(this);	
	timeLine->Init(0.0f, 1.0f, false, true);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::red); vectorColors.push_back(Qt::darkGreen);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("x"); vectorLegends.push_back("y");
	timeLine->AddLines(PropLineWrapper<Vector2>(line->GetModificationLine()).GetProps(), vectorColors, vectorLegends);	
	dialogLayout->addWidget(timeLine);
	InitButtons();	
}

template <> void EditModificationLineDialog::Init<Vector3>(ModifiablePropertyLine<Vector3>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	timeLine = new TimeLineWidget(this);	
	timeLine->Init(0.0f, 1.0f, false, true);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::red); vectorColors.push_back(Qt::darkGreen); vectorColors.push_back(Qt::blue);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("x"); vectorLegends.push_back("y"); vectorLegends.push_back("z");
	timeLine->AddLines(PropLineWrapper<Vector3>(line->GetModificationLine()).GetProps(), vectorColors, vectorLegends);	
	dialogLayout->addWidget(timeLine);
	InitButtons();	
}

template <> void EditModificationLineDialog::Init<Color>(ModifiablePropertyLine<Color>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	gradientLine = new GradientPickerWidget(this);
	gradientLine->Init(0,1);
	gradientLine->SetValues(PropLineWrapper<Color>(line->GetModificationLine()).GetProps());
	dialogLayout->addWidget(gradientLine);
	InitButtons();
	
}


template <> void EditModificationLineDialog::UpdateLine<float32> (ModifiablePropertyLine<float32>* line)
{
	PropLineWrapper<float32> lineWrap;
	if(!timeLine->GetValue(0, lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}

template <> void EditModificationLineDialog::UpdateLine<Vector2> (ModifiablePropertyLine<Vector2>* line)
{
	PropLineWrapper<Vector2> lineWrap;
	if(!timeLine->GetValues(lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}

template <> void EditModificationLineDialog::UpdateLine<Vector3> (ModifiablePropertyLine<Vector3>* line)
{
	PropLineWrapper<Vector3> lineWrap;
	if(!timeLine->GetValues(lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}

template <> void EditModificationLineDialog::UpdateLine<Color> (ModifiablePropertyLine<Color>* line)
{
	PropLineWrapper<Color> lineWrap;
	if(!gradientLine->GetValues(lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}


QWidget* VariableEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
	spinBox->setMinimum(0.0f);
	spinBox->setMaximum(1.0f);
	spinBox->setDecimals(3);
	spinBox->setSingleStep(0.1f);
	return spinBox;
}
void VariableEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	static_cast<QDoubleSpinBox *>(editor)->setValue(editTable->item(index.row(), 1)->text().toDouble());
}
void VariableEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	editTable->item(index.row(), 1)->setText(QString::number(static_cast<QDoubleSpinBox *>(editor)->value()));
}

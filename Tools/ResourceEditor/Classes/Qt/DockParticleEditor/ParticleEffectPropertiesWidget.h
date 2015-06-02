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


#ifndef __PARTICLE_EFFECT_PROPERTIES_WIDGET__H__
#define __PARTICLE_EFFECT_PROPERTIES_WIDGET__H__

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTableWidget>
#include <QStyledItemDelegate>

#include "DockParticleEditor/TimeLineWidget.h"
#include "DockParticleEditor/GradientPickerWidget.h"

struct EffectTreeData
{
	DAVA::ParticleEmitter *emmiter;
	DAVA::ParticleLayer* layer;
	DAVA::ParticleForce* force;
	int32 externalParamId;
};

Q_DECLARE_METATYPE(EffectTreeData);

static const String EXTERNAL_NAMES[]=
{
	"Emission Vector",		//emmiter
	"Emission Range",
	"Radius",
	"Size",
	"Color Over Life",
	"Life",					//layer
	"Life Variation",
	"Number",
	"Number Variation",
	"Size",
	"Size Variation",
	"Size Over Life",
	"Velocity",
	"Velocity Variation",
	"Velocity Over Life",
	"Spin",
	"Spin Variation",
	"Spin Over Life",
	"Color Random",
	"Alpha Over Life",
	"Color Over Life",
	"Angle",
	"Angle Variation",
	"Anim Speed Over Life",
	"Force",					//force
	"Force Variation",
	"Force Over Life"
};

class EditModificationLineDialog: public QDialog
{
	Q_OBJECT
public:
	explicit EditModificationLineDialog(QWidget *parent) : QDialog(parent){setMinimumWidth(400);}
	template <class T> void Init(ModifiablePropertyLine<T>* line, bool onAdd);
	template <class T> void UpdateLine(ModifiablePropertyLine<T>* line); //note! - name would be updated explicitly as it may require re-register in effect
	String GetVariableName();
private:
	void InitName(const String& name, bool onAdd);
	void InitButtons();

	QVBoxLayout *dialogLayout;
	QLineEdit *variableName;
	TimeLineWidget *timeLine;
	GradientPickerWidget *gradientLine;
};

class VariableEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	VariableEditDelegate(QObject *parent, QTableWidget *table) : QStyledItemDelegate(parent), editTable(table){}
	
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
protected:
	QTableWidget *editTable;
};


class ParticleEffectPropertiesWidget: public QWidget, public BaseParticleEditorContentWidget
{
	Q_OBJECT
	
public:
	enum EmitterExternals
	{
		EE_EMISSION_VECTOR=0,
		EE_EMISSION_RANGE,
		EE_RADUS,
		EE_SIZE,
		EE_COLOR_OVER_LIFE,
		EE_TOTAL
	};

	enum LayerExternals
	{
		EL_LIFE = EE_TOTAL,
		EL_LIFE_VARIATION,
		EL_NUMBER,
		EL_NUMBER_VARIATION,
		EL_SIZE,
		EL_SIZE_VARIATION,
		EL_SIZE_OVERLIFE,
		EL_VELOCITY,
		EL_VELOCITY_VARIATON,
		EL_VELOCITY_OVERLIFE,
		EL_SPIN,
		EL_SPIN_VARIATION,
		EL_SPIN_OVERLIFE,
		EL_COLOR,
		EL_ALPHA_OVERLIFE,
		EL_COLOR_OVERLIFE,
		EL_ANGLE,
		EL_ANGLE_VARIATION,
		EL_ANIM_SPEED_OVERLIFE,
		EL_TOTAL
	};	
	enum ForceExternals
	{
		EF_FORCE = EL_TOTAL,		
		EF_FORCE_OVERLIFE,
		EF_TOTAL
	};

	explicit ParticleEffectPropertiesWidget(QWidget* parent = 0);
	~ParticleEffectPropertiesWidget();

	void Init(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect);
	ParticleEffectComponent* GetEffect() {return particleEffect;};

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

public slots:
	void OnValueChanged();
	void OnPlay();
	void OnStop();
	void OnStopAndDelete();
	void OnPause();
	void OnRestart();
	void OnStepForward();

	void ShowContextMenuForEffectTree(const QPoint &pos);
	void OnContextMenuCommand(QAction *action);
	void OnTreeItemDoubleClck(QTreeWidgetItem *treeItem, int column);
	
	void OnVariableValueChanged(int row, int col);
	void OnGlobalVariableValueChanged(int row, int col);
	void OnAddGlobalExternal();
	
protected:
	void InitWidget(QWidget* widget, bool connectWidget = true);
	void BuildEffectTree();
	void UpdatePlaybackSpeedLabel();

	void UpdateVaribleTables();

	ModifiablePropertyLineBase * GetEmitterLine(ParticleEmitter *emitter, EmitterExternals lineId);
	ModifiablePropertyLineBase * GetLayerLine(ParticleLayer *layer, LayerExternals lineId);
	ModifiablePropertyLineBase * GetForceLine(ParticleForce *force, ForceExternals lineId);

	void SetEmitterLineModifiable(ParticleEmitter *emitter, EmitterExternals lineId);
	void SetLayerLineModifiable(ParticleLayer *layer, LayerExternals lineId);
	void SetForceLineModifiable(ParticleForce *force, ForceExternals lineId);

	void RemoveEmitterLineModifiable(ParticleEmitter *emitter, EmitterExternals lineId);
	void RemoveLayerLineModifiable(ParticleLayer *layer, LayerExternals lineId);
	void RemoveForceLineModifiable(ParticleForce *force, ForceExternals lineId);

	bool EditEmitterModifiable(ParticleEmitter *emitter, EmitterExternals lineId, bool onAdd = false);
	bool EditLayerModifiable(ParticleLayer *layer, LayerExternals lineId, bool onAdd = false);
	bool EditForceModifiable(ParticleForce *force, ForceExternals lineId, bool onAdd = false);

	template <class T> bool EditModificationLine(RefPtr<PropertyLine<T> > &line, bool onAdd)
	{
		ModifiablePropertyLine<T> *editLine = dynamic_cast<ModifiablePropertyLine<T>* >(line.Get());
		EditModificationLineDialog dialog(this);
		dialog.Init(editLine, onAdd);
		if (dialog.exec())
		{			
			dialog.UpdateLine(editLine);
			String resName = dialog.GetVariableName();
			if(editLine->GetValueName()!=resName)
			{
				particleEffect->UnRegisterModifiable(editLine);
				editLine->SetValueName(resName);
				particleEffect->RegisterModifiable(editLine);
				UpdateVaribleTables();
			}

			return true;
		}
		return false;
	}

private:
	ParticleEffectComponent* particleEffect;

	QVBoxLayout* mainLayout;

	QLabel* effectPlaybackSpeedLabel;
	QSlider* effectPlaybackSpeed;	

	QPushButton *playBtn;
	QPushButton *stopBtn;
	QPushButton *stopAndDeleteBtn;
	QPushButton *pauseBtn;
	QPushButton *restartBtn;
	QPushButton *stepForwardBtn;
	QSpinBox* stepForwardFPSSpin;	
	
	QIcon iconEmitter, iconLayer, iconForce, iconExternal;

	QTreeWidget *effectTree;
	QTreeWidgetItem *currSelectedTreeItem;
	
	QTableWidget *effectVariables;
	VariableEditDelegate *effectEditDelegate;
	QTableWidget *globalVariables;
	VariableEditDelegate *globalEditDelegate;

	bool blockSignals;
	bool blockTables;
};


class AddGlobalExternalDialog: public QDialog
{
	Q_OBJECT
public:
	explicit AddGlobalExternalDialog(QWidget *parent);
	String GetVariableName();
	float32 GetVariableValue();
private:		
	QLineEdit *variableName;
	QDoubleSpinBox *variableValue;
};



#endif /* defined(__PARTICLE_EFFECT_PROPERTIES_WIDGET__H__) */

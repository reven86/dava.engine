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



#include "LODEditor.h"
#include "ui_LODEditor.h"

#include "EditorLODData.h"
#include "DistanceSlider.h"

#include "Scene/SceneSignals.h"
#include "Classes/Qt/Scene/SceneSignals.h"
#include "Classes/Qt/PlaneLODDialog/PlaneLODDialog.h"
#include "Classes/Qt/Main/mainwindow.h"

#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QInputDialog>

static const int32 MAX_LOD = 0x7FFFFFFF;

struct DistanceWidget
{
    QComboBox* layerCombo;
    QLabel *name;
    QDoubleSpinBox *distance;

    void SetVisible(bool visible)
    {
        name->setVisible(visible);
        distance->setVisible(visible);
        layerCombo->setVisible(visible);
    }
    
    void SelectLod(int32 lod)
    {
        if(layerCombo)
        {
            uint32 currentIndex = 0;
            
            uint32 itemCount = layerCombo->count();
            for(uint32 i = 0; i < itemCount; ++i)
            {
                QVariant itemData = layerCombo->itemData(i);
                if(itemData.toInt() == lod)
                {
                    currentIndex = i;
                    break;
                }
            }
            
            layerCombo->setCurrentIndex(currentIndex);
        }
    }
    
    int32 GetSelectedLod()
    {
        int32 currentLod = MAX_LOD;
        int32 currentIndex = layerCombo->currentIndex();
        
        if(currentIndex >= 0)
        {
            QVariant itemData = layerCombo->itemData(currentIndex);
            currentLod = itemData.toInt();
        }
        
        return currentLod;
    }
    
    void PopulateCombo(const DAVA::Vector<DAVA::int32>& layers)
    {
        layerCombo->blockSignals(true);
        
        layerCombo->clear();
        
        layerCombo->addItem(QString("--"), QVariant(DAVA::LodComponent::INVALID_LOD_LAYER));
        
        size_t layerCount = layers.size();
        for(size_t i = 0; i < layerCount; ++i)
        {
            layerCombo->addItem(QString("%1").arg(layers[i]), QVariant(layers[i]));
        }
        
        layerCombo->blockSignals(false);
    }
};

static const uint32 DISTANCE_WIDGET_COUNT = 4;
static DistanceWidget distanceWidgets[DISTANCE_WIDGET_COUNT];


LODEditor::LODEditor(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::LODEditor)
{
	ui->setupUi(this);
    
    editedLODData = new EditorLODData();
    connect(editedLODData, SIGNAL(DataChanged()), SLOT(LODDataChanged()));

    SetupInternalUI();
    SetupSceneSignals();
    
    ForceDistanceStateChanged(Qt::Unchecked);
    LODDataChanged();
    
    posSaver.Attach(this);

	allSceneModeEnabled = SettingsManager::GetValue(Settings::Internal_LODEditorMode).AsBool();
	ui->checkBoxLodEditorMode->setChecked(allSceneModeEnabled);
	editedLODData->EnableAllSceneMode(allSceneModeEnabled);
}

LODEditor::~LODEditor()
{
    SafeDelete(editedLODData);
    
	delete ui;
}

void LODEditor::SetupInternalUI()
{
    ui->lodEditorSettingsButton->setStyleSheet("Text-align:left");
    ui->viewLODButton->setStyleSheet("Text-align:left");
    ui->editLODButton->setStyleSheet("Text-align:left");
    
    connect(ui->lodEditorSettingsButton, SIGNAL(released()), SLOT(LODEditorSettingsButtonReleased()));
    connect(ui->viewLODButton, SIGNAL(released()), SLOT(ViewLODButtonReleased()));
    connect(ui->editLODButton, SIGNAL(released()), SLOT(EditLODButtonReleased()));
    
    connect(ui->enableForceDistance, SIGNAL(stateChanged(int)), SLOT(ForceDistanceStateChanged(int)));
    connect(ui->forceSlider, SIGNAL(valueChanged(int)), SLOT(ForceDistanceChanged(int)));
    ui->forceSlider->setRange(0, DAVA::LodComponent::MAX_LOD_DISTANCE);
    ui->forceSlider->setValue(0);
    
    connect(ui->distanceSlider, SIGNAL(DistanceChanged(const QVector<int> &, bool)), SLOT(LODDistanceChangedBySlider(const QVector<int> &, bool)));
    
    InitDistanceSpinBox(ui->lod0LayerBox, ui->lod0Name, ui->lod0Distance, 0);
    InitDistanceSpinBox(ui->lod1LayerBox, ui->lod1Name, ui->lod1Distance, 1);
    InitDistanceSpinBox(ui->lod2LayerBox, ui->lod2Name, ui->lod2Distance, 2);
    InitDistanceSpinBox(ui->lod3LayerBox, ui->lod3Name, ui->lod3Distance, 3);
    
    DAVA::Vector<DAVA::int32> dummyIndices;
    dummyIndices.resize(DAVA::LodComponent::MAX_LOD_LAYERS);
    SetForceLayerValues(dummyIndices);
    connect(ui->forceLayer, SIGNAL(activated(int)), SLOT(ForceLayerActivated(int)));

	connect(ui->checkBoxLodEditorMode, SIGNAL(stateChanged(int)), this, SLOT(EditorModeChanged(int)));

    //TODO: remove after lod editing implementation
    connect(ui->lastLodToFrontButton, SIGNAL(clicked()), this, SLOT(CopyLODToLod0Clicked()));
    connect(ui->createPlaneLodButton, SIGNAL(clicked()), this, SLOT(CreatePlaneLODClicked()));
    connect(ui->buttonDeleteFirstLOD, SIGNAL(clicked()), editedLODData, SLOT(DeleteFirstLOD()));
    connect(ui->buttonDeleteLastLOD, SIGNAL(clicked()), editedLODData, SLOT(DeleteLastLOD()));
    
    connect(ui->lodQualityBox, SIGNAL(currentIndexChanged (int)), this, SLOT(QualityNameChanged(int)));
}

void LODEditor::SetupSceneSignals()
{
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));
}

void LODEditor::ForceDistanceStateChanged(int checked)
{
    bool enable = (checked == Qt::Checked);
    editedLODData->EnableForceDistance(enable);
    
    if(!enable)
    {
        int layer = ui->forceLayer->itemData(ui->forceLayer->currentIndex()).toInt();
        editedLODData->SetForceLayer(layer);
    }
    
    ui->forceSlider->setEnabled(enable);
    ui->forceLayer->setEnabled(!enable);
}

void LODEditor::ForceDistanceChanged(int distance)
{
    editedLODData->SetForceDistance(distance);
}


void LODEditor::InitDistanceSpinBox(QComboBox* lodCombo, QLabel *name, QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(0.f, DAVA::LodComponent::MAX_LOD_DISTANCE);  //distance 
    spinbox->setProperty(ResourceEditor::TAG.c_str(), index);
    spinbox->setValue(0.f);
    spinbox->setFocusPolicy(Qt::WheelFocus);
    spinbox->setKeyboardTracking(false);
    
    lodCombo->setProperty(ResourceEditor::TAG.c_str(), index);
    
    connect(spinbox, SIGNAL(valueChanged(double)), SLOT(LODDistanceChangedBySpinbox(double)));
    connect(lodCombo, SIGNAL(currentIndexChanged (int)), this, SLOT(LodIndexChanged(int)));
    
    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    distanceWidgets[index].layerCombo = lodCombo;
    
    distanceWidgets->SetVisible(true);
}


void LODEditor::SceneActivated(SceneEditor2 *scene)
{
    //TODO: set gloabal scene settings
    
    PopulateLODNames();
    editedLODData->SetLODQuality(QualitySettingsSystem::Instance()->GetCurrentLODQuality());
    UpdateLodLayersSelection(editedLODData->GetLODQuality());
}


void LODEditor::SceneDeactivated(SceneEditor2 *scene)
{
    //TODO: clear/save gloabal scene settings

    ForceDistanceStateChanged(Qt::Unchecked);
}


void LODEditor::LODDataChanged()
{
    if(ui->lodQualityBox->currentIndex() >= 0)
    {
        QString selection = ui->lodQualityBox->currentText();
        FastName selectionFastName(selection.toStdString().c_str());
        
        ui->distanceSlider->SetLayersCount(DAVA::LodComponent::MAX_LOD_LAYERS);
        
        UpdateLodLayersSelection(selectionFastName);
        
        SetForceLayerValues(editedLODData->GetLODIndices());
    }
    
    UpdateWidgetVisibility();

    ui->lastLodToFrontButton->setEnabled(editedLODData->CanCreatePlaneLOD());
    ui->createPlaneLodButton->setEnabled(editedLODData->CanCreatePlaneLOD());
    ui->buttonDeleteFirstLOD->setEnabled(editedLODData->CanDeleteLod());
    ui->buttonDeleteLastLOD->setEnabled(editedLODData->CanDeleteLod());
}

void LODEditor::LODDistanceChangedBySlider(const QVector<int> &changedLayers, bool continuous)
{
	ui->distanceSlider->LockDistances(true);

	if(changedLayers.size() != 0)
	{
		DAVA::Map<DAVA::uint32, DAVA::float32> lodDistances;
		for (int i = 0; i < changedLayers.size(); i++)
		{
			int layer = changedLayers[i];

			double value = ui->distanceSlider->GetDistance(layer);
			SetSpinboxValue(distanceWidgets[layer].distance, value);

			if(!continuous)
			{
				lodDistances[layer] = value;
			}
		}

		editedLODData->UpdateDistances(lodDistances);
	}

	ui->distanceSlider->LockDistances(false);
}

void LODEditor::LODDistanceChangedBySpinbox(double value)
{
    QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(sender());
    if(spinBox)
    {
        //TODO set new value to scene
        int lodLevel = spinBox->property(ResourceEditor::TAG.c_str()).toInt();

        {
            const bool wasBlocked = editedLODData->blockSignals(true);
            editedLODData->SetLayerDistance(lodLevel, value);
            editedLODData->blockSignals(wasBlocked);
        }
        {
            const bool wasBlocked = ui->distanceSlider->blockSignals(true);
            ui->distanceSlider->SetDistance(lodLevel, value);
            ui->distanceSlider->blockSignals(wasBlocked);
        }
    }
}

void LODEditor::SetSpinboxValue(QDoubleSpinBox *spinbox, double value)
{
    bool wasBlocked = spinbox->blockSignals(true);
    spinbox->setValue(value);
    spinbox->blockSignals(wasBlocked);
}

void LODEditor::ForceLayerActivated(int index)
{
    int layer = ui->forceLayer->itemData(index).toInt();
    editedLODData->SetForceLayer(layer);
}

void LODEditor::SetForceLayerValues(const DAVA::Vector<DAVA::int32>& lodIndices)
{
    ui->forceLayer->clear();
    
    ui->forceLayer->addItem("Auto", QVariant(DAVA::LodComponent::INVALID_LOD_LAYER));
    
    int32 currentForceIndex = -1;
    uint32 indexCount = lodIndices.size();
    for(uint32 i = 0; i < indexCount; ++i)
    {
        int32 index = lodIndices[i];
        ui->forceLayer->addItem(Format("%d", index).c_str(), QVariant(index));
        
        if(editedLODData->GetForceLayer() == index)
        {
            currentForceIndex = i;
        }

    }
    
    if(currentForceIndex >= 0)
    {
        ui->forceLayer->setCurrentIndex(currentForceIndex);
    }
}

void LODEditor::LODEditorSettingsButtonReleased()
{
    InvertFrameVisibility(ui->frameLodEditorSettings, ui->lodEditorSettingsButton);
}

void LODEditor::ViewLODButtonReleased()
{
    InvertFrameVisibility(ui->frameViewLOD, ui->viewLODButton);
    
    if(ui->frameViewLOD->isVisible() == false)
    {
        editedLODData->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
        editedLODData->SetForceLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
        
        ui->enableForceDistance->setCheckState(Qt::Unchecked);
        ui->forceLayer->setCurrentIndex(0);
    }
}

void LODEditor::EditLODButtonReleased()
{
    InvertFrameVisibility(ui->frameEditLOD, ui->editLODButton);
}

void LODEditor::InvertFrameVisibility(QFrame *frame, QPushButton *frameButton)
{
    bool visible = frame->isVisible();
    frame->setVisible(!visible);

	QIcon icon = (frame->isVisible()) ? QIcon(":/QtIcons/advanced.png") : QIcon(":/QtIcons/play.png");
	frameButton->setIcon(icon);
}


void LODEditor::UpdateWidgetVisibility()
{
    bool visible = (editedLODData->GetLayersCount() != 0);
    
    ui->viewLODButton->setVisible(visible);
    ui->frameViewLOD->setVisible(visible);
    ui->editLODButton->setVisible(visible);
    ui->frameEditLOD->setVisible(visible);
}

void LODEditor::CopyLODToLod0Clicked()
{
    if(editedLODData->CanCreatePlaneLOD())
        editedLODData->CopyLastLodToLod0();
}

void LODEditor::CreatePlaneLODClicked()
{
    if(editedLODData->CanCreatePlaneLOD())
    {
        FilePath defaultTexturePath = editedLODData->GetDefaultTexturePathForPlaneEntity();

        PlaneLODDialog dialog(editedLODData->GetLayersCount(), defaultTexturePath, this);
        if(dialog.exec() == QDialog::Accepted)
        {
            QtMainWindow::Instance()->WaitStart("Creating Plane LOD", "Please wait...");

            editedLODData->CreatePlaneLOD(dialog.GetSelectedLayer(), dialog.GetSelectedTextureSize(), dialog.GetSelectedTexturePath());

            QtMainWindow::Instance()->WaitStop();
        }
    }
}

void LODEditor::EditorModeChanged(int newMode)
{
	allSceneModeEnabled = (newMode == Qt::Checked);
	DAVA::VariantType value(allSceneModeEnabled);
	SettingsManager::SetValue(Settings::Internal_LODEditorMode, value);

	editedLODData->EnableAllSceneMode(allSceneModeEnabled);
}

void LODEditor::PopulateLODNames()
{
    ui->lodQualityBox->blockSignals(true);
    
    DAVA::QualitySettingsSystem* qualitySettingsSystem = DAVA::QualitySettingsSystem::Instance();
    uint32 lodCount = qualitySettingsSystem->GetLODQualityCount();
    
    int32 currentIndex = -1;
    if(ui->lodQualityBox->count() != lodCount)
    {
        ui->lodQualityBox->clear();
        for(uint32 lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            const FastName& lodQualityName = qualitySettingsSystem->GetLODQualityName(lodIndex);
            
            ui->lodQualityBox->addItem(lodQualityName.c_str(), lodQualityName.c_str());
            
            if(lodQualityName == qualitySettingsSystem->GetCurrentLODQuality())
            {
                currentIndex = (int32)lodIndex;
            }
        }
    }
    
    ui->lodQualityBox->blockSignals(false);
    
    if(currentIndex >= 0)
    {
        ui->lodQualityBox->setCurrentIndex(currentIndex);
    }
}

void LODEditor::UpdateLodLayersSelection(const DAVA::FastName& lodQualityName)
{
    if(lodQualityName.IsValid())
    {
        const DAVA::Vector<DAVA::int32>& layers = editedLODData->GetLODIndices();
        
        ui->distanceSlider->SetLayersCount(editedLODData->GetDistanceCount());
        
        for (DAVA::int32 i = 0; i < COUNT_OF(distanceWidgets); ++i)
        {
            distanceWidgets[i].SetVisible(true);
            distanceWidgets[i].PopulateCombo(layers);
            
            DAVA::float32 distance = editedLODData->GetLayerDistance(i);
            DAVA::int32 lodIndex = editedLODData->GetLayerLodIndex(i);
            
            SetSpinboxValue(distanceWidgets[i].distance, distance);
            distanceWidgets[i].name->setText(Format("%d. (%d):", i, editedLODData->GetLayerTriangles(i)).c_str());
            
            distanceWidgets[i].layerCombo->blockSignals(true);
            distanceWidgets[i].SelectLod(lodIndex);
            distanceWidgets[i].layerCombo->blockSignals(false);
            
            if(!editedLODData->IsEmptyLayer(i))
            {
                ui->distanceSlider->SetDistance(i, distance);
            }
        }
    }
}

void LODEditor::QualityNameChanged(int index)
{
    if(index >= 0)
    {
        QComboBox* srcBox = dynamic_cast<QComboBox*>(sender());
        if(NULL != srcBox)
        {
            FastName qualityName(srcBox->itemData(index).toString().toStdString().c_str());
            
            editedLODData->SetLODQuality(qualityName);
        }
    }
}

void LODEditor::LodIndexChanged(int index)
{
    if(index >= 0)
    {
        QComboBox* srcBox = dynamic_cast<QComboBox*>(sender());
        if(NULL != srcBox)
        {
            int layerNum = srcBox->property(ResourceEditor::TAG.c_str()).toInt();
            int lodIndex = srcBox->itemData(index).toInt();
            
            editedLODData->SetLayerLodIndex(layerNum, lodIndex);
        }
    }
}

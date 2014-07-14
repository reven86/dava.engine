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


struct DistanceWidget
{
    QCheckBox* check;
    QLabel *name;
    QDoubleSpinBox *distance;

    void SetVisible(bool visible)
    {
        name->setVisible(visible);
        distance->setVisible(visible);
        check->setVisible(visible);
    }
    
    void SetChecked(bool checked)
    {
        check->setChecked(checked);
    }
};

static const uint32 DISTANCE_WIDGET_COUNT = 8;
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
    
    InitDistanceSpinBox(ui->lod0Check, ui->lod0Name, ui->lod0Distance, 0);
    InitDistanceSpinBox(ui->lod1Check, ui->lod1Name, ui->lod1Distance, 1);
    InitDistanceSpinBox(ui->lod2Check, ui->lod2Name, ui->lod2Distance, 2);
    InitDistanceSpinBox(ui->lod3Check, ui->lod3Name, ui->lod3Distance, 3);
    InitDistanceSpinBox(ui->lod4Check, ui->lod4Name, ui->lod4Distance, 4);
    InitDistanceSpinBox(ui->lod5Check, ui->lod5Name, ui->lod5Distance, 5);
    InitDistanceSpinBox(ui->lod6Check, ui->lod6Name, ui->lod6Distance, 6);
    InitDistanceSpinBox(ui->lod7Check, ui->lod7Name, ui->lod7Distance, 7);
    
    DAVA::Set<DAVA::int32> dummyIndices;
    SetForceLayerValues(dummyIndices);
    connect(ui->forceLayer, SIGNAL(activated(int)), SLOT(ForceLayerActivated(int)));

	connect(ui->checkBoxLodEditorMode, SIGNAL(stateChanged(int)), this, SLOT(EditorModeChanged(int)));

    //TODO: remove after lod editing implementation
    connect(ui->lastLodToFrontButton, SIGNAL(clicked()), this, SLOT(CopyLODToLod0Clicked()));
    connect(ui->createPlaneLodButton, SIGNAL(clicked()), this, SLOT(CreatePlaneLODClicked()));
    connect(ui->buttonDeleteFirstLOD, SIGNAL(clicked()), editedLODData, SLOT(DeleteFirstLOD()));
    connect(ui->buttonDeleteLastLOD, SIGNAL(clicked()), editedLODData, SLOT(DeleteLastLOD()));
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


void LODEditor::InitDistanceSpinBox(QCheckBox* check, QLabel *name, QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(0.f, DAVA::LodComponent::MAX_LOD_DISTANCE);  //distance 
    spinbox->setProperty(ResourceEditor::TAG.c_str(), index);
    spinbox->setValue(0.f);
    spinbox->setFocusPolicy(Qt::WheelFocus);
    spinbox->setKeyboardTracking(false);
    
    connect(spinbox, SIGNAL(valueChanged(double)), SLOT(LODDistanceChangedBySpinbox(double)));
    
    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    distanceWidgets[index].check = check;
    
    distanceWidgets->SetVisible(false);
    distanceWidgets->SetChecked(false);
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
    UpdateLodLayersSelection(FastName(ui->lodQualityBox->currentText().toAscii()));
    
    const DAVA::Set<int32>& activeLodIndices = editedLODData->GetActiveLODIndices();
    
    
    ui->distanceSlider->SetLayersCount(activeLodIndices.size());
    SetForceLayerValues(activeLodIndices);
    
    
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

void LODEditor::SetForceLayerValues(const DAVA::Set<DAVA::int32>& lodIndices)
{
    ui->forceLayer->clear();
    
    ui->forceLayer->addItem("Auto", QVariant(DAVA::LodComponent::INVALID_LOD_LAYER));
    
    DAVA::Set<DAVA::int32>::const_iterator it = lodIndices.begin();
    DAVA::Set<DAVA::int32>::const_iterator end = lodIndices.end();
    while(it != end)
    {
        int32 index = *it;
        ui->forceLayer->addItem(Format("%d", index).c_str(), QVariant(index));
        
        ++it;
    }
    
    int32 currentForceIndex = 0;
    int32 indexToSelect = 0;
    int32 forceLayerIndex = editedLODData->GetForceLayer();
    
    it = lodIndices.begin();
    while(it != end)
    {
        int32 index = *it;
        
        if(editedLODData->GetForceLayer() == index)
        {
            indexToSelect = currentForceIndex;
            break;
        }
        
        currentForceIndex++;
        
        ++it;
    }
    
    ui->forceLayer->setCurrentIndex(indexToSelect);
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
    DAVA::QualitySettingsSystem* qualitySettingsSystem = DAVA::QualitySettingsSystem::Instance();
    uint32 lodCount = qualitySettingsSystem->GetLODQualityCount();
    
    int32 currentIndex = -1;
    if(ui->lodQualityBox->count() != lodCount)
    {
        ui->lodQualityBox->clear();
        for(uint32 lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            const FastName& lodQualityName = qualitySettingsSystem->GetLODQualityName(lodIndex);
            ui->lodQualityBox->addItem(lodQualityName.c_str());
            
            if(lodQualityName == qualitySettingsSystem->GetCurrentLODQuality())
            {
                currentIndex = (int32)lodIndex;
            }
        }
    }
    
    if(currentIndex >= 0)
    {
        ui->lodQualityBox->setCurrentIndex(currentIndex);
    }
}

void LODEditor::UpdateLodLayersSelection(const DAVA::FastName& lodQualityName)
{
    if(lodQualityName.IsValid())
    {
        editedLODData->SetLODQuality(lodQualityName);
        
        const DAVA::Set<int32>& activeLodIndices = editedLODData->GetActiveLODIndices();
        const DAVA::Set<int32>& allLodIndices = editedLODData->GetAllLODIndices();
        
        for (DAVA::int32 i = 0; i < COUNT_OF(distanceWidgets); ++i)
        {
            distanceWidgets[i].SetVisible(false);
            distanceWidgets[i].SetChecked(false);
        }
        
        DAVA::Set<int32>::const_iterator allIndicesIt = allLodIndices.begin();
        DAVA::Set<int32>::const_iterator allIndicesEnd = allLodIndices.end();
        while(allIndicesIt != allIndicesEnd)
        {
            int32 lodIndex = *allIndicesIt;
            
            if(lodIndex >= 0 && lodIndex < COUNT_OF(distanceWidgets))
            {
                distanceWidgets[lodIndex].SetVisible(true);
                
                DAVA::float32 distance = editedLODData->GetLayerDistance(lodIndex);
                
                SetSpinboxValue(distanceWidgets[lodIndex].distance, distance);
                ui->distanceSlider->SetDistance(lodIndex, distance);
                
                distanceWidgets[lodIndex].name->setText(Format("%d. (%d):", lodIndex, editedLODData->GetLayerTriangles(lodIndex)).c_str());
            }
            
            ++allIndicesIt;
        }
        
        DAVA::Set<int32>::const_iterator activeIndicesIt = activeLodIndices.begin();
        DAVA::Set<int32>::const_iterator activeIndicesEnd = activeLodIndices.end();
        while(activeIndicesIt != activeIndicesEnd)
        {
            int32 lodIndex = *activeIndicesIt;
            
            if(lodIndex >= 0 && lodIndex < COUNT_OF(distanceWidgets))
            {
                distanceWidgets[lodIndex].SetChecked(true);
            }
            
            ++activeIndicesIt;
        }
    }
}
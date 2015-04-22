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

#include "Scene/System/EditorLODSystem.h"
#include "DistanceSlider.h"

#include "Scene/SceneSignals.h"
#include "Classes/Qt/Scene/SceneSignals.h"
#include "Classes/Qt/PlaneLODDialog/PlaneLODDialog.h"
#include "Classes/Qt/Main/mainwindow.h"

#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QInputDialog>


LODEditor::LODEditor(QWidget* parent)
:   QWidget(parent),
    ui(new Ui::LODEditor)
{
    ui->setupUi(this);

    bool allSceneModeEnabled = SettingsManager::GetValue(Settings::Internal_LODEditorMode).AsBool();
    ui->checkBoxLodEditorMode->setChecked(allSceneModeEnabled);

    SetupInternalUI();
    SetupSceneSignals();
       
    posSaver.Attach(this);
}

LODEditor::~LODEditor()
{    
    delete ui;
}

void LODEditor::SetupInternalUI()
{
    ui->lodEditorSettingsButton->setStyleSheet("Text-align:left");
    ui->viewLODButton->setStyleSheet("Text-align:left");
    ui->editLODButton->setStyleSheet("Text-align:left");
    
    connect(ui->lodEditorSettingsButton, &QPushButton::clicked, this, &LODEditor::LODEditorSettingsButtonReleased);
    connect(ui->viewLODButton, &QPushButton::clicked, this, &LODEditor::ViewLODButtonReleased);
    connect(ui->editLODButton, &QPushButton::clicked, this, &LODEditor::EditLODButtonReleased);
    
    connect(ui->enableForceDistance, &QCheckBox::toggled, this, &LODEditor::ForceDistanceStateChanged);
    connect(ui->enableForceDistance, &QCheckBox::toggled, ui->forceSlider, &QWidget::setEnabled);
    connect(ui->enableForceDistance, &QCheckBox::toggled, ui->forceLayer, &QWidget::setDisabled);
    connect(ui->forceSlider, &LabeledSlider::valueChanged, this, &LODEditor::ForceDistanceChanged);
    ui->forceSlider->setRange(0, DAVA::LodComponent::MAX_LOD_DISTANCE);
    ui->forceSlider->setValue(0);
    ui->forceSlider->setEnabled(ui->enableForceDistance->isChecked());
    
    connect(ui->distanceSlider, &DistanceSlider::DistanceChanged, this, &LODEditor::LODDistanceChangedBySlider);
    
    InitDistanceSpinBox(ui->lod0Name, ui->lod0Distance, 0);
    InitDistanceSpinBox(ui->lod1Name, ui->lod1Distance, 1);
    InitDistanceSpinBox(ui->lod2Name, ui->lod2Distance, 2);
    InitDistanceSpinBox(ui->lod3Name, ui->lod3Distance, 3);
    
    CreateForceLayerValues(DAVA::LodComponent::MAX_LOD_LAYERS);
    connect(ui->forceLayer, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &LODEditor::ForceLayerActivated);

    connect(ui->checkBoxLodEditorMode, &QCheckBox::stateChanged, this, &LODEditor::EditorModeChanged);

    //TODO: remove after lod editing implementation
    connect(ui->lastLodToFrontButton, &QPushButton::clicked, this, &LODEditor::CopyLODToLod0Clicked);
    connect(ui->createPlaneLodButton, &QPushButton::clicked, this, &LODEditor::CreatePlaneLODClicked);
    connect(ui->buttonDeleteFirstLOD, &QPushButton::clicked, this, &LODEditor::DeleteFirstLOD);
    connect(ui->buttonDeleteLastLOD, &QPushButton::clicked, this, &LODEditor::DeleteLastLOD);
    
    //default state 
    ui->viewLODButton->setVisible(false);
    ui->frameViewLOD->setVisible(false);
    ui->editLODButton->setVisible(false);
    ui->frameEditLOD->setVisible(false);
}

void LODEditor::SetupSceneSignals()
{
    connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &LODEditor::SceneActivated);
    connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &LODEditor::SceneDeactivated);
    connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged, this, &LODEditor::SceneSelectionChanged);
    connect(SceneSignals::Instance(), &SceneSignals::SolidChanged, this, &LODEditor::SolidChanged);
    connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &LODEditor::CommandExecuted);
}

void LODEditor::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
    if (command->GetId() == CMDID_BATCH)
    {
        CommandBatch *batch = (CommandBatch *)command;
        Command2 *firstCommand = batch->GetCommand(0);
        if (nullptr != firstCommand)
        {
            switch (firstCommand->GetId())
            {
            case CMDID_LOD_DISTANCE_CHANGE:
            case CMDID_LOD_COPY_LAST_LOD:
            case CMDID_LOD_DELETE:
            case CMDID_LOD_CREATE_PLANE:
                scene->editorLODSystem->CollectLODDataFromScene();
                LODDataChanged(scene);
                break;
            default:
                break;
            }
        }
    }
}

void LODEditor::ForceDistanceStateChanged(bool checked)
{
    GetCurrentEditorLODSystem()->SetForceDistanceEnabled(checked);
}

void LODEditor::ForceDistanceChanged(int distance)
{
    GetCurrentEditorLODSystem()->SetForceDistance(distance);
}

void LODEditor::InitDistanceSpinBox(QLabel *name, QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(DAVA::LodComponent::MIN_LOD_DISTANCE, DAVA::LodComponent::MAX_LOD_DISTANCE);  //distance 
    spinbox->setProperty(ResourceEditor::TAG.c_str(), index);
    spinbox->setValue(DAVA::LodComponent::MIN_LOD_DISTANCE);
    spinbox->setFocusPolicy(Qt::WheelFocus);
    spinbox->setKeyboardTracking(false);
    
    connect(spinbox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &LODEditor::LODDistanceChangedBySpinbox);
    
    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    
    distanceWidgets[index].SetVisible(false);
}


void LODEditor::SceneActivated(SceneEditor2 *scene)
{
    DVASSERT(scene);
    EditorLODSystem *sceneEditorLodSystem = scene->editorLODSystem;
    ui->checkBoxLodEditorMode->setChecked(sceneEditorLodSystem->GetAllSceneModeEnabled());
    ui->enableForceDistance->setChecked(sceneEditorLodSystem->GetForceDistanceEnabled());
    ui->forceSlider->setValue(sceneEditorLodSystem->GetForceDistance());
    int index = ui->forceLayer->findData(sceneEditorLodSystem->GetForceLayer());
    if (-1 != index)
    {
        ui->forceLayer->setCurrentIndex(index);
    }
    LODDataChanged(scene);
}

void LODEditor::SceneDeactivated(SceneEditor2 *scene)
{
    UpdateWidgetVisibility(nullptr);
}

void LODEditor::LODDataChanged(SceneEditor2 *scene /* = nullptr */)
{
    const EditorLODSystem *currentLODSystem;
    if (nullptr != scene)
    {
        currentLODSystem = scene->editorLODSystem;
    }
    else
    {
        currentLODSystem = GetCurrentEditorLODSystem();
    }

    DAVA::uint32 lodLayersCount = currentLODSystem->GetCurrentLodsLayersCount();
    DVASSERT(lodLayersCount <= DAVA::LodComponent::MAX_LOD_LAYERS);

    ui->distanceSlider->SetLayersCount(lodLayersCount);
    SetForceLayerValues(currentLODSystem, lodLayersCount);
    for (DAVA::uint32 i = 0; i < lodLayersCount; ++i)
    {
        distanceWidgets[i].SetVisible(true);

        DAVA::float32 distance = currentLODSystem->GetLayerDistance(i);

        SetSpinboxValue(distanceWidgets[i].distance, distance);
        ui->distanceSlider->SetDistance(i, distance);

        distanceWidgets[i].name->setText(Format("%d. (%d):", i, currentLODSystem->GetLayerTriangles(i)).c_str());
    }
    for (DAVA::int32 i = lodLayersCount; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        distanceWidgets[i].SetVisible(false);
    }

    UpdateWidgetVisibility(currentLODSystem);

    UpdateLODButtons(currentLODSystem);
}

void LODEditor::LODDistanceChangedBySlider(const QVector<int> &changedLayers, bool continious)
{
    if (changedLayers.empty())
    {
        return;
    }

    ui->distanceSlider->LockDistances(true);

    if (!continious)
    {
        DAVA::Map<DAVA::uint32, DAVA::float32> lodDistances;
        for (auto layer : changedLayers)
        {
            lodDistances[layer] = ui->distanceSlider->GetDistance(layer);
        }
        GetCurrentEditorLODSystem()->UpdateDistances(lodDistances);
    }
    UpdateSpinboxesBorders();
    ui->distanceSlider->LockDistances(false);
}

void LODEditor::LODDistanceChangedBySpinbox(double value)
{
    QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(sender());
    if (nullptr == spinBox)
    {
        return;
    }
        //TODO set new value to scene
    int lodLevel = spinBox->property(ResourceEditor::TAG.c_str()).toInt();

    GetCurrentEditorLODSystem()->SetLayerDistance(lodLevel, value);

    UpdateSpinboxesBorders();

    const bool wasBlocked = ui->distanceSlider->blockSignals(true);
    ui->distanceSlider->SetDistance(lodLevel, value);
    ui->distanceSlider->blockSignals(wasBlocked);
}

void LODEditor::UpdateSpinboxesBorders()
{
    distanceWidgets[1].distance->setMinimum(LodComponent::MIN_LOD_DISTANCE);
    DAVA::uint32 count = ui->distanceSlider->GetLayersCount();
    for (DAVA::uint32  i = 1; i < count; ++i) //we don't work with zero level and zero spinbox
    {
        int val = ui->distanceSlider->GetDistance(i - 1);
        distanceWidgets[i].distance->setMinimum(ui->distanceSlider->GetDistance(i - 1));

        if (i < count - 1)
        {
            int val = ui->distanceSlider->GetDistance(i + 1);
            distanceWidgets[i].distance->setMaximum(ui->distanceSlider->GetDistance(i + 1));
        }
        SetSpinboxValue(distanceWidgets[i].distance, ui->distanceSlider->GetDistance(i));
    }
    distanceWidgets[count - 1].distance->setMaximum(LodComponent::MAX_LOD_DISTANCE);
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
    GetCurrentEditorLODSystem()->SetForceLayer(layer);
}

void LODEditor::CreateForceLayerValues(int layersCount)
{
    ui->forceLayer->clear();

    ui->forceLayer->addItem("Auto", QVariant(DAVA::LodComponent::INVALID_LOD_LAYER));

    for (DAVA::int32 i = 0; i < layersCount; ++i)
    {
        ui->forceLayer->addItem(Format("%d", i).c_str(), QVariant(i));
    }

    ui->forceLayer->setCurrentIndex(0);
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
        GetCurrentEditorLODSystem()->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
        GetCurrentEditorLODSystem()->SetForceLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
        
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

void LODEditor::UpdateWidgetVisibility(const EditorLODSystem *editorLODSystem)
{
    bool visible = nullptr != editorLODSystem && (editorLODSystem->GetCurrentLodsLayersCount() != 0);
    
    ui->viewLODButton->setVisible(visible);
    ui->frameViewLOD->setVisible(visible);
    ui->editLODButton->setVisible(visible);
    ui->frameEditLOD->setVisible(visible);
}

//TODO: refactor this function
void LODEditor::SetForceLayerValues(const EditorLODSystem *editorLODSystem, int layersCount)
{
    int requestedIndex = editorLODSystem->GetForceLayer() + 1;
    CreateForceLayerValues(layersCount);
    ui->forceLayer->setCurrentIndex(requestedIndex);
}

void LODEditor::UpdateLODButtons(const EditorLODSystem *editorLODSystem)
{
    DVASSERT(editorLODSystem);
    bool canDeleteLOD = editorLODSystem->CanDeleteLod();

    ui->buttonDeleteFirstLOD->setEnabled(canDeleteLOD);
    ui->buttonDeleteLastLOD->setEnabled(canDeleteLOD);

    bool canCreatePlaneLOD = editorLODSystem->CanCreatePlaneLOD();
    ui->lastLodToFrontButton->setEnabled(canCreatePlaneLOD);
    ui->createPlaneLodButton->setEnabled(canCreatePlaneLOD);
}

void LODEditor::CopyLODToLod0Clicked()
{
    if (!GetCurrentEditorLODSystem()->CanCreatePlaneLOD())
    {
        return;
    }
    GetCurrentEditorLODSystem()->CopyLastLodToLod0();
}

void LODEditor::CreatePlaneLODClicked()
{
    if (!GetCurrentEditorLODSystem()->CanCreatePlaneLOD())
    {
        return;
    }

    FilePath defaultTexturePath = GetCurrentEditorLODSystem()->GetDefaultTexturePathForPlaneEntity();

    PlaneLODDialog dialog(GetCurrentEditorLODSystem()->GetCurrentLodsLayersCount(), defaultTexturePath, this);
    if(dialog.exec() == QDialog::Accepted)
    {
        QtMainWindow::Instance()->WaitStart("Creating Plane LOD", "Please wait...");

        GetCurrentEditorLODSystem()->CreatePlaneLOD(dialog.GetSelectedLayer(), dialog.GetSelectedTextureSize(), dialog.GetSelectedTexturePath());

        QtMainWindow::Instance()->WaitStop();
    }
}

void LODEditor::EditorModeChanged(int newMode)
{
    bool allSceneModeEnabled = (newMode == Qt::Checked);
    DAVA::VariantType value(allSceneModeEnabled);
    SettingsManager::SetValue(Settings::Internal_LODEditorMode, value);

    GetCurrentEditorLODSystem()->SetAllSceneModeEnabled(allSceneModeEnabled);
    LODDataChanged();
}

EditorLODSystem *LODEditor::GetCurrentEditorLODSystem()
{
    DVASSERT(QtMainWindow::Instance());
    DVASSERT(QtMainWindow::Instance()->GetCurrentScene());
    return QtMainWindow::Instance()->GetCurrentScene()->editorLODSystem;
}

void LODEditor::DeleteFirstLOD()
{
    int requestedIndex = ui->forceLayer->currentIndex();
    if(GetCurrentEditorLODSystem()->DeleteFirstLOD()
        && requestedIndex 
        && requestedIndex == ui->forceLayer->count())
    {
        requestedIndex--;
        ui->forceLayer->setCurrentIndex(requestedIndex);
        ForceLayerActivated(requestedIndex);
    }
}

void LODEditor::DeleteLastLOD()
{
    int requestedIndex = ui->forceLayer->currentIndex();
    if(GetCurrentEditorLODSystem()->DeleteLastLOD()
        && requestedIndex 
        && requestedIndex == ui->forceLayer->count())
    {
        requestedIndex--;
        ui->forceLayer->setCurrentIndex(requestedIndex);
        ForceLayerActivated(requestedIndex);
    }
}

void LODEditor::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    DVASSERT(scene);
    DVASSERT(selected);
    DVASSERT(deselected);
    scene->editorLODSystem->SceneSelectionChanged(selected, deselected);
    LODDataChanged(scene);
}

void LODEditor::SolidChanged(SceneEditor2 *scene, const Entity *entity, bool value)
{
    DVASSERT(scene);
    DVASSERT(entity);
    if (SettingsManager::GetValue(Settings::Scene_RefreshLodForNonSolid).AsBool())
    {
        scene->editorLODSystem->SolidChanged(entity, value);
        LODDataChanged(scene);
    }
}

void LODEditor::DistanceWidget::SetVisible(bool visible)
{
    name->setVisible(visible);
    distance->setVisible(visible);
}

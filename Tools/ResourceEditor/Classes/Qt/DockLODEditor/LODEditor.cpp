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


#include "DockLODEditor/LODEditor.h"
#include "DockLODEditor/DistanceSlider.h"

#include "ui_LODEditor.h"

#include "Commands2/AddComponentCommand.h"
#include "Commands2/RemoveComponentCommand.h"

#include "Main/mainwindow.h"
#include "PlaneLODDialog/PlaneLODDialog.h"

#include "Scene/System/EditorLODSystemV2.h"
#include "Scene/SceneSignals.h"

#include "Tools/LazyUpdater/LazyUpdater.h"


#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QInputDialog>
#include <QFrame>
#include <QPushButton>


using namespace DAVA;

namespace LODEditorInternal
{
    class BlockSignalGuard
    {
    public:

        BlockSignalGuard(QObject *object_)
            : object(object_)
        {
            DVASSERT(object);
            wasBlockedBefore = object->blockSignals(true);
        }

        ~BlockSignalGuard()
        {
            object->blockSignals(wasBlockedBefore);
        }

    private:

        QObject *object = nullptr;
        bool wasBlockedBefore = false;
    };
}

LODEditor::LODEditor(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LODEditor)
{
    ui->setupUi(this);

    Function<void()> fnUpdateUI(this, &LODEditor::UpdateUI);
    uiUpdater = new LazyUpdater(fnUpdateUI, this);

    Function<void()> fnUpdatePanels(this, &LODEditor::UpdatePanelsForCurrentScene);
    panelsUpdater = new LazyUpdater(fnUpdatePanels, this);
    
    SetupSceneSignals();
    SetupInternalUI();
      
    new QtPosSaver( this );
}

void LODEditor::SetupSceneSignals()
{
    connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &LODEditor::SceneActivated);
    connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &LODEditor::SceneDeactivated);
    connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged, this, &LODEditor::SceneSelectionChanged);
    connect(SceneSignals::Instance(), &SceneSignals::SolidChanged, this, &LODEditor::SolidChanged);
}

void LODEditor::SetupInternalUI()
{
    connect(ui->checkBoxLodEditorMode, &QCheckBox::stateChanged, this, &LODEditor::EditorModeChanged);

    SetupPanelsButtonUI();
    SetupForceUI();
    SetupDistancesUI();
    SetupActionsUI();
}

void LODEditor::UpdateUI()
{
    UpdateModeUI();
    UpdateForceUI();
    UpdateDistancesUI();
}


//MODE

void LODEditor::EditorModeChanged(int newMode)
{
    const bool allSceneModeEnabled = (newMode == Qt::Checked);
    VariantType value(allSceneModeEnabled);
    SettingsManager::SetValue(Settings::Internal_LODEditorMode, value);

    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->SetMode(allSceneModeEnabled ? EditorLODSystemV2::MODE_ALL_SCENE : EditorLODSystemV2::MODE_SELECTION);
}

void LODEditor::UpdateModeUI()
{
    LODEditorInternal::BlockSignalGuard guard(ui->checkBoxLodEditorMode);

    const EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    const EditorLODSystemV2::eMode mode = system->GetMode();

    ui->checkBoxLodEditorMode->setChecked(mode == EditorLODSystemV2::MODE_ALL_SCENE);

    panelsUpdater->Update();
}


//MODE

//PANELS
void LODEditor::SetupPanelsButtonUI()
{
    ui->lodEditorSettingsButton->setStyleSheet("Text-align:left");
    ui->viewLODButton->setStyleSheet("Text-align:left");
    ui->editLODButton->setStyleSheet("Text-align:left");

    connect(ui->lodEditorSettingsButton, &QPushButton::clicked, this, &LODEditor::LODEditorSettingsButtonReleased);
    connect(ui->viewLODButton, &QPushButton::clicked, this, &LODEditor::ViewLODButtonReleased);
    connect(ui->editLODButton, &QPushButton::clicked, this, &LODEditor::EditLODButtonReleased);

    //default state 
    frameViewVisible = true;
    frameEditVisible = true;
    ui->viewLODButton->setVisible(frameViewVisible);
    ui->frameViewLOD->setVisible(frameViewVisible);
    ui->editLODButton->setVisible(frameEditVisible);
    ui->frameEditLOD->setVisible(frameEditVisible);
}

void LODEditor::LODEditorSettingsButtonReleased()
{
    InvertFrameVisibility(ui->frameLodEditorSettings, ui->lodEditorSettingsButton);
}

void LODEditor::ViewLODButtonReleased()
{
    InvertFrameVisibility(ui->frameViewLOD, ui->viewLODButton);
    frameViewVisible = ui->frameViewLOD->isVisible();
}

void LODEditor::EditLODButtonReleased()
{
    InvertFrameVisibility(ui->frameEditLOD, ui->editLODButton);
    frameEditVisible = ui->frameEditLOD->isVisible();
}

void LODEditor::InvertFrameVisibility(QFrame *frame, QPushButton *frameButton)
{
    bool visible = !frame->isVisible();
    frame->setVisible(visible);

    QIcon icon = (visible) ? QIcon(":/QtIcons/advanced.png") : QIcon(":/QtIcons/play.png");
    frameButton->setIcon(icon);
}

void LODEditor::UpdatePanelsUI(SceneEditor2 *forScene)
{
    if (forScene != nullptr)
    {
        panelsUpdater->Update();
    }
    else
    {
        ui->viewLODButton->setVisible(false);
        ui->editLODButton->setVisible(false);
        ui->frameViewLOD->setVisible(false);
        ui->frameEditLOD->setVisible(false);
    }
}

void LODEditor::UpdatePanelsForCurrentScene()
{
    bool panelVisible = false;

    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    if (system != nullptr)
    {
        const LODComponentHolder *lodData = system->GetActiveLODData();
        panelVisible = (lodData->GetLODLayersCount() > 0);
    }

    ui->viewLODButton->setVisible(panelVisible);
    ui->editLODButton->setVisible(panelVisible);
    if (!panelVisible)
    {
        ui->frameViewLOD->setVisible(panelVisible);
        ui->frameEditLOD->setVisible(panelVisible);
    }
    else
    {
        QIcon viewIcon = (frameViewVisible) ? QIcon(":/QtIcons/advanced.png") : QIcon(":/QtIcons/play.png");
        ui->viewLODButton->setIcon(viewIcon);
        ui->frameViewLOD->setVisible(frameViewVisible);

        QIcon editIcon = (frameEditVisible) ? QIcon(":/QtIcons/advanced.png") : QIcon(":/QtIcons/play.png");
        ui->editLODButton->setIcon(editIcon);
        ui->frameEditLOD->setVisible(frameEditVisible);
    }
}

//ENDOF PANELS


//FORCE
void LODEditor::SetupForceUI()
{
    connect(ui->enableForceDistance, &QCheckBox::toggled, this, &LODEditor::ForceDistanceStateChanged);
    connect(ui->forceSlider, &LabeledSlider::valueChanged, this, &LODEditor::ForceDistanceChanged);

    ui->forceSlider->setRange(LodComponent::INVALID_DISTANCE, LodComponent::MAX_LOD_DISTANCE);
    ui->forceSlider->setValue(LodComponent::INVALID_DISTANCE);

    connect(ui->forceLayer, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &LODEditor::ForceLayerActivated);
}

void LODEditor::UpdateForceUI()
{
    LODEditorInternal::BlockSignalGuard guard1(ui->enableForceDistance);
    LODEditorInternal::BlockSignalGuard guard2(ui->forceSlider);
    LODEditorInternal::BlockSignalGuard guard3(ui->forceLayer);

    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    const ForceValues & forceValues = system->GetForceValues();

    const bool distanceModeSelected = (forceValues.flag & ForceValues::APPLY_DISTANCE) == ForceValues::APPLY_DISTANCE;
    ui->enableForceDistance->setChecked(distanceModeSelected);
    ui->forceSlider->setEnabled(distanceModeSelected);
    ui->forceLayer->setEnabled(!distanceModeSelected);

    ui->forceSlider->setValue(forceValues.distance);

    const LODComponentHolder *activeLodData = system->GetActiveLODData();
    const uint32 layerItemsCount = activeLodData->GetLODLayersCount() + 1;
    if (ui->forceLayer->count() != layerItemsCount)
    {
        CreateForceLayerValues(layerItemsCount);
    }

    int32 forceIndex = Min(forceValues.layer + 1, ui->forceLayer->count() - 1);
    ui->forceLayer->setCurrentIndex(forceIndex);
}

void LODEditor::ForceDistanceStateChanged(bool checked)
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    
    ForceValues forceValues = system->GetForceValues();
    forceValues.flag = (checked) ? ForceValues::APPLY_DISTANCE : ForceValues::APPLY_LAYER;
    system->SetForceValues(forceValues);
}

void LODEditor::ForceDistanceChanged(int distance)
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();

    ForceValues forceValues = system->GetForceValues();
    forceValues.distance = static_cast<float32> (distance);
    system->SetForceValues(forceValues);
}

void LODEditor::ForceLayerActivated(int index)
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();

    ForceValues forceValues = system->GetForceValues();
    forceValues.layer = index - 1;
    system->SetForceValues(forceValues);
}

void LODEditor::CreateForceLayerValues(uint32 layersCount)
{
    ui->forceLayer->clear();
    ui->forceLayer->addItem("Auto", QVariant(LodComponent::INVALID_LOD_LAYER));
    for (uint32 i = 0; i < layersCount; ++i)
    {
        ui->forceLayer->addItem(Format("%u", i).c_str(), QVariant(i));
    }
    ui->forceLayer->setCurrentIndex(0);
}

//FORCE


//DISTANCES
void LODEditor::SetupDistancesUI()
{
    connect(ui->distanceSlider, &DistanceSlider::DistanceChanged, this, &LODEditor::LODDistanceChangedBySlider);

    InitDistanceSpinBox(ui->lod0Name, ui->lod0Distance, 0);
    InitDistanceSpinBox(ui->lod1Name, ui->lod1Distance, 1);
    InitDistanceSpinBox(ui->lod2Name, ui->lod2Distance, 2);
    InitDistanceSpinBox(ui->lod3Name, ui->lod3Distance, 3);
}

void LODEditor::InitDistanceSpinBox(QLabel *name, QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(LodComponent::MIN_LOD_DISTANCE, LodComponent::MAX_LOD_DISTANCE);  //distance 
    spinbox->setValue(LodComponent::MIN_LOD_DISTANCE);
    spinbox->setFocusPolicy(Qt::WheelFocus);
    spinbox->setKeyboardTracking(false);
    
    connect(spinbox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &LODEditor::LODDistanceChangedBySpinbox);
    
    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    distanceWidgets[index].SetVisible(false);
}


void LODEditor::UpdateDistancesUI()
{
    LODEditorInternal::BlockSignalGuard guard(ui->distanceSlider);

    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    const LODComponentHolder *lodData = system->GetActiveLODData();
    const LodComponent &lc = lodData->GetLODComponent();

    int32 count = static_cast<int32>(lodData->GetLODLayersCount());
    ui->distanceSlider->SetLayersCount(count);

    Array<float32, LodComponent::MAX_LOD_LAYERS> distances;
    distances.fill(0.0f);

    for (int32 i = 0; i < count; ++i)
    {
        distances[i] = lc.GetLodLayerDistance(i);
        ui->distanceSlider->SetDistance(i, distances[i]);

        distanceWidgets[i].SetVisible(true);
    }

    UpdateDistanceSpinboxesUI(distances, count);

    for (int32 i = count; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        distanceWidgets[i].SetVisible(false);
    }
}

void LODEditor::UpdateDistanceSpinboxesUI(const DAVA::Array<DAVA::float32, DAVA::LodComponent::MAX_LOD_LAYERS> &distances, int32 count)
{
    for (int32 i = 0; i < count; ++i)
    {
        LODEditorInternal::BlockSignalGuard guardWidget(distanceWidgets[i].distance);

        float32 minDistance = (i == 0) ? LodComponent::MIN_LOD_DISTANCE : distances[i - 1];
        float32 maxDistance = (i == count - 1) ? LodComponent::MAX_LOD_DISTANCE : distances[i + 1];
        distanceWidgets[i].distance->setRange(minDistance, maxDistance);  //distance 
        distanceWidgets[i].distance->setValue(distances[i]);
    }
}


void LODEditor::LODDistanceChangedBySlider(const QVector<int> &changedLayers, bool continious)
{
    if (changedLayers.empty())
    {
        return;
    }

    Array<float32, LodComponent::MAX_LOD_LAYERS> distances;
    distances.fill(0.0f);
    for (int32 i = 0; i < static_cast<int32>(distances.size()); ++i)
    {
        distances[i] = ui->distanceSlider->GetDistance(i);
    }

    if (continious)
    {   //update only UI
        UpdateDistanceSpinboxesUI(distances, LodComponent::MAX_LOD_LAYERS);
    }
    else
    {
        EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
        system->SetLODDistances(distances);
    }
}

void LODEditor::LODDistanceChangedBySpinbox(double value)
{
    Array<float32, LodComponent::MAX_LOD_LAYERS> distances;
    distances.fill(0.0f);
    for (uint32 i = 0; i < distances.size(); ++i)
    {
        distances[i] = distanceWidgets[i].distance->value();
    }

    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->SetLODDistances(distances);
}

//DISTANCES


//SCENE SIGNALS

void LODEditor::SceneActivated(SceneEditor2 *scene)
{
    DVASSERT(scene);
    UpdateUI();
    UpdatePanelsUI(scene);
}

void LODEditor::SceneDeactivated(SceneEditor2 *scene)
{
    UpdatePanelsUI(nullptr);
}

void LODEditor::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->SelectionChanged(selected, deselected);
}

void LODEditor::SolidChanged(SceneEditor2 *scene, const Entity *entity, bool value)
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->SolidChanged(entity, value);
}

//SCENE SIGNALS

// void LODEditor::LODDataChanged(SceneEditor2 *scene /* = nullptr */)
// {
//     const EditorLODSystem *currentLODSystem;
//     if (nullptr != scene)
//     {
//         currentLODSystem = scene->editorLODSystem;
//     }
//     else
//     {
//         currentLODSystem = GetCurrentEditorLODSystem();
//     }
// 
//     uint32 lodLayersCount = currentLODSystem->GetCurrentLodsLayersCount();
//     DVASSERT(lodLayersCount <= LodComponent::MAX_LOD_LAYERS);
// 
//     ui->distanceSlider->SetLayersCount(lodLayersCount);
//     SetForceLayerValues(currentLODSystem, lodLayersCount);
//     for (uint32 i = 0; i < lodLayersCount; ++i)
//     {
//         distanceWidgets[i].SetVisible(true);
// 
//         float32 distance = currentLODSystem->GetLayerDistance(i);
// 
//         SetSpinboxValue(distanceWidgets[i].distance, distance);
//         ui->distanceSlider->SetDistance(i, distance);
// 
//         distanceWidgets[i].name->setText(Format("%d. (%d):", i, currentLODSystem->GetLayerTriangles(i)).c_str());
//     }
//     for (int32 i = lodLayersCount; i < LodComponent::MAX_LOD_LAYERS; ++i)
//     {
//         distanceWidgets[i].SetVisible(false);
//     }
// 
//     UpdateWidgetVisibility(currentLODSystem);
// 
//     UpdateLODButtons(currentLODSystem);
// 
//     UpdateForceLayer(currentLODSystem);
//     UpdateForceDistance(currentLODSystem);
//}



//TODO: refactor this function
void LODEditor::UpdateLODButtons(const EditorLODSystemV2 *editorLODSystem)
{
//     DVASSERT(editorLODSystem);
//     bool canDeleteLOD = editorLODSystem->CanDeleteLod();
// 
//     ui->buttonDeleteFirstLOD->setEnabled(canDeleteLOD);
//     ui->buttonDeleteLastLOD->setEnabled(canDeleteLOD);
// 
//     bool canCreatePlaneLOD = editorLODSystem->CanCreatePlaneLOD();
//     ui->lastLodToFrontButton->setEnabled(canCreatePlaneLOD);
//     ui->createPlaneLodButton->setEnabled(canCreatePlaneLOD);
}



EditorLODSystemV2 *LODEditor::GetCurrentEditorLODSystem()
{
    DVASSERT(QtMainWindow::Instance());
    DVASSERT(QtMainWindow::Instance()->GetCurrentScene());

    return QtMainWindow::Instance()->GetCurrentScene()->editorLODSystemV2;
}

//ACTIONS

void LODEditor::SetupActionsUI()
{
    connect(ui->lastLodToFrontButton, &QPushButton::clicked, this, &LODEditor::CopyLODToLod0Clicked);
    connect(ui->createPlaneLodButton, &QPushButton::clicked, this, &LODEditor::CreatePlaneLODClicked);
    connect(ui->buttonDeleteFirstLOD, &QPushButton::clicked, this, &LODEditor::DeleteFirstLOD);
    connect(ui->buttonDeleteLastLOD, &QPushButton::clicked, this, &LODEditor::DeleteLastLOD);
}

void LODEditor::UpdateActionsUI()
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();

    const bool canDeleteLod = system->CanDeleteLOD();
    ui->buttonDeleteFirstLOD->setEnabled(canDeleteLod);
    ui->buttonDeleteLastLOD->setEnabled(canDeleteLod);

    bool canCreateLod = system->CanCreateLOD();
    ui->lastLodToFrontButton->setEnabled(canCreateLod);
    ui->createPlaneLodButton->setEnabled(canCreateLod);
}

void LODEditor::CopyLODToLod0Clicked()
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->CopyLastLODToFirst();
}

void LODEditor::CreatePlaneLODClicked()
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();


//    system->CreatePlaneLOD();


    //     if (!GetCurrentEditorLODSystem()->CanCreatePlaneLOD())
    //     {
    //         return;
    //     }
    // 
    //     FilePath defaultTexturePath = GetCurrentEditorLODSystem()->GetDefaultTexturePathForPlaneEntity();
    // 
    //     PlaneLODDialog dialog(GetCurrentEditorLODSystem()->GetCurrentLodsLayersCount(), defaultTexturePath, this);
    //     if(dialog.exec() == QDialog::Accepted)
    //     {
    //         QtMainWindow::Instance()->WaitStart("Creating Plane LOD", "Please wait...");
    // 
    //         GetCurrentEditorLODSystem()->CreatePlaneLOD(dialog.GetSelectedLayer(), dialog.GetSelectedTextureSize(), dialog.GetSelectedTexturePath());
    // 
    //         QtMainWindow::Instance()->WaitStop();
    //     }
}

void LODEditor::DeleteFirstLOD()
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->DeleteFirstLOD();
}

void LODEditor::DeleteLastLOD()
{
    EditorLODSystemV2 *system = GetCurrentEditorLODSystem();
    system->DeleteLastLOD();
}


//ACTIONS



void LODEditor::DistanceWidget::SetVisible(bool visible)
{
    name->setVisible(visible);
    distance->setVisible(visible);
}

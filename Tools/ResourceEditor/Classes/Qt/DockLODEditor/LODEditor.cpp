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

#include "Utils/StringFormat.h"

#include "DockLODEditor/LODEditor.h"
#include "DockLODEditor/DistanceSlider.h"
#include "Commands2/AddComponentCommand.h"
#include "Commands2/RemoveComponentCommand.h"
#include "Main/Guards.h"
#include "Main/mainwindow.h"
#include "PlaneLODDialog/PlaneLODDialog.h"
#include "Scene/System/EditorLODSystem.h"
#include "Scene/System/EditorStatisticsSystem.h"
#include "Scene/SceneSignals.h"
#include "Tools/LazyUpdater/LazyUpdater.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

#include "ui_LODEditor.h"

#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QInputDialog>
#include <QFrame>
#include <QPushButton>
#include <QSignalBlocker>

using namespace DAVA;

LODEditor::LODEditor(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LODEditor)
{
    ui->setupUi(this);

    Function<void()> fnUpdatePanels(this, &LODEditor::UpdatePanelsForCurrentScene);
    panelsUpdater = new LazyUpdater(fnUpdatePanels, this);

    SetupSceneSignals();
    SetupInternalUI();

    new QtPosSaver(this);
}

LODEditor::~LODEditor() = default;

void LODEditor::SetupSceneSignals()
{
    connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &LODEditor::SceneActivated);
    connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &LODEditor::SceneDeactivated);
    connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged, this, &LODEditor::SceneSelectionChanged);
    connect(SceneSignals::Instance(), &SceneSignals::SolidChanged, this, &LODEditor::SolidChanged);
}

void LODEditor::SetupInternalUI()
{
    connect(ui->checkBoxLodEditorMode, &QCheckBox::clicked, this, &LODEditor::SceneOrSelectionModeSelected);

    SetupPanelsButtonUI();
    SetupForceUI();
    SetupDistancesUI();
    SetupActionsUI();

    UpdatePanelsUI(nullptr);
}

//MODE

void LODEditor::SceneOrSelectionModeSelected(bool allSceneModeActivated)
{
    VariantType value(allSceneModeActivated);
    SettingsManager::SetValue(Settings::Internal_LODEditorMode, value);

    EditorLODSystem* system = GetCurrentEditorLODSystem();
    system->SetMode(allSceneModeActivated ? eEditorMode::MODE_ALL_SCENE : eEditorMode::MODE_SELECTION);
}

//MODE

//PANELS
void LODEditor::SetupPanelsButtonUI()
{
    ui->lodEditorSettingsButton->setStyleSheet("Text-align:left");
    ui->viewLODButton->setStyleSheet("Text-align:left");
    ui->editLODButton->setStyleSheet("Text-align:left");

    connect(ui->lodEditorSettingsButton, &QPushButton::clicked, this, &LODEditor::LODEditorSettingsButtonClicked);
    connect(ui->viewLODButton, &QPushButton::clicked, this, &LODEditor::ViewLODButtonClicked);
    connect(ui->editLODButton, &QPushButton::clicked, this, &LODEditor::EditLODButtonClicked);

    //default state
    frameViewVisible = true;
    frameEditVisible = true;
    ui->viewLODButton->setVisible(frameViewVisible);
    ui->frameViewLOD->setVisible(frameViewVisible);
    ui->editLODButton->setVisible(frameEditVisible);
    ui->frameEditLOD->setVisible(frameEditVisible);
}

void LODEditor::LODEditorSettingsButtonClicked()
{
    InvertFrameVisibility(ui->frameLodEditorSettings, ui->lodEditorSettingsButton);
}

void LODEditor::ViewLODButtonClicked()
{
    InvertFrameVisibility(ui->frameViewLOD, ui->viewLODButton);
    frameViewVisible = ui->frameViewLOD->isVisible();
}

void LODEditor::EditLODButtonClicked()
{
    InvertFrameVisibility(ui->frameEditLOD, ui->editLODButton);
    frameEditVisible = ui->frameEditLOD->isVisible();
}

void LODEditor::InvertFrameVisibility(QFrame* frame, QPushButton* frameButton)
{
    bool visible = !frame->isVisible();
    frame->setVisible(visible);

    QIcon icon = (visible) ? QIcon(":/QtIcons/advanced.png") : QIcon(":/QtIcons/play.png");
    frameButton->setIcon(icon);
}

void LODEditor::UpdatePanelsUI(SceneEditor2* forScene)
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

    EditorLODSystem* system = GetCurrentEditorLODSystem();
    if (system != nullptr)
    {
        const LODComponentHolder* lodData = system->GetActiveLODData();
        panelVisible = (lodData->GetLODLayersCount() > 0);
    }

    ui->viewLODButton->setVisible(panelVisible);
    ui->editLODButton->setVisible(panelVisible);
    if (!panelVisible)
    {
        ui->frameViewLOD->setVisible(false);
        ui->frameEditLOD->setVisible(false);
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

    connect(ui->forceLayer, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &LODEditor::ForceLayerActivated);
}

void LODEditor::ForceDistanceStateChanged(bool checked)
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();

    ForceValues forceValues = system->GetForceValues();
    forceValues.flag = (checked) ? ForceValues::APPLY_DISTANCE : ForceValues::APPLY_LAYER;
    system->SetForceValues(forceValues);
}

void LODEditor::ForceDistanceChanged(int distance)
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();

    ForceValues forceValues = system->GetForceValues();
    forceValues.distance = static_cast<float32>(distance);
    system->SetForceValues(forceValues);
}

void LODEditor::ForceLayerActivated(int index)
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();
    const LODComponentHolder* lodData = system->GetActiveLODData();

    ForceValues forceValues = system->GetForceValues();
    forceValues.layer = index - 1;

    if (forceValues.layer == lodData->GetLODLayersCount())
    {
        forceValues.layer = LodComponent::LAST_LOD_LAYER;
    }

    system->SetForceValues(forceValues);
}

void LODEditor::CreateForceLayerValues(uint32 layersCount)
{
    ui->forceLayer->clear();
    ui->forceLayer->addItem("Auto", LodComponent::INVALID_LOD_LAYER);
    for (uint32 i = 0; i < layersCount; ++i)
    {
        ui->forceLayer->addItem(Format("%u", i).c_str(), QVariant(i));
    }
    ui->forceLayer->addItem("Last", LodComponent::LAST_LOD_LAYER);
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

void LODEditor::InitDistanceSpinBox(QLabel* name, QDoubleSpinBox* spinbox, int index)
{
    spinbox->setRange(LodComponent::MIN_LOD_DISTANCE, LodComponent::MAX_LOD_DISTANCE); //distance
    spinbox->setValue(LodComponent::MIN_LOD_DISTANCE);
    spinbox->setFocusPolicy(Qt::WheelFocus);
    spinbox->setKeyboardTracking(false);

    connect(spinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &LODEditor::LODDistanceChangedBySpinbox);

    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    distanceWidgets[index].SetVisible(false);
}

void LODEditor::UpdateDistanceSpinboxesUI(const DAVA::Array<DAVA::float32, DAVA::LodComponent::MAX_LOD_LAYERS>& distances, int32 count)
{
    for (int32 i = 0; i < count; ++i)
    {
        const QSignalBlocker guardWidget(distanceWidgets[i].distance);

        float32 minDistance = (i == 0) ? LodComponent::MIN_LOD_DISTANCE : distances[i - 1];
        float32 maxDistance = (i == count - 1) ? LodComponent::MAX_LOD_DISTANCE : distances[i + 1];
        distanceWidgets[i].distance->setRange(minDistance, maxDistance); //distance
        distanceWidgets[i].distance->setValue(distances[i]);
    }
}

void LODEditor::LODDistanceChangedBySlider(const QVector<int>& changedLayers, bool continious)
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

    EditorLODSystem* system = GetCurrentEditorLODSystem();
    if (continious)
    { //update only UI
        const LODComponentHolder* lodData = system->GetActiveLODData();

        UpdateDistanceSpinboxesUI(distances, lodData->GetLODLayersCount());
    }
    else
    {
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

    EditorLODSystem* system = GetCurrentEditorLODSystem();
    system->SetLODDistances(distances);
}

//DISTANCES

//SCENE SIGNALS

void LODEditor::SceneActivated(SceneEditor2* scene)
{
    DVASSERT(scene);
    scene->editorLODSystem->AddDelegate(this);
    scene->editorStatisticsSystem->AddDelegate(this);
}

void LODEditor::SceneDeactivated(SceneEditor2* scene)
{
    DVASSERT(scene);
    scene->editorLODSystem->RemoveDelegate(this);
    scene->editorStatisticsSystem->RemoveDelegate(this);

    if (GetCurrentEditorLODSystem() == nullptr)
    {
        UpdatePanelsUI(nullptr);
    }
}

void LODEditor::SceneSelectionChanged(SceneEditor2* scene, const EntityGroup* selected, const EntityGroup* deselected)
{
    DVASSERT(scene != nullptr);

    EditorLODSystem* system = scene->editorLODSystem;
    system->SelectionChanged(selected, deselected);
}

void LODEditor::SolidChanged(SceneEditor2* scene, const Entity* entity, bool value)
{
    DVASSERT(scene != nullptr);

    EditorLODSystem* system = scene->editorLODSystem;
    system->SolidChanged(entity, value);
}

//SCENE SIGNALS

//ACTIONS

void LODEditor::SetupActionsUI()
{
    connect(ui->lastLodToFrontButton, &QPushButton::clicked, this, &LODEditor::CopyLastLODToLOD0Clicked);
    connect(ui->createPlaneLodButton, &QPushButton::clicked, this, &LODEditor::CreatePlaneLODClicked);
    connect(ui->buttonDeleteFirstLOD, &QPushButton::clicked, this, &LODEditor::DeleteFirstLOD);
    connect(ui->buttonDeleteLastLOD, &QPushButton::clicked, this, &LODEditor::DeleteLastLOD);
}

void LODEditor::CopyLastLODToLOD0Clicked()
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();
    system->CopyLastLODToFirst();
}

void LODEditor::CreatePlaneLODClicked()
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();
    const LODComponentHolder* lodData = system->GetActiveLODData();

    FilePath defaultTexturePath = system->GetPathForPlaneEntity();
    PlaneLODDialog dialog(lodData->GetLODLayersCount(), defaultTexturePath, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QtMainWindow::Instance()->WaitStart("Creating Plane LOD", "Please wait...");
        system->CreatePlaneLOD(dialog.GetSelectedLayer(), dialog.GetSelectedTextureSize(), dialog.GetSelectedTexturePath());
        QtMainWindow::Instance()->WaitStop();
    }
}

void LODEditor::DeleteFirstLOD()
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();
    system->DeleteFirstLOD();
}

void LODEditor::DeleteLastLOD()
{
    EditorLODSystem* system = GetCurrentEditorLODSystem();
    system->DeleteLastLOD();
}

//ACTIONS

//DELEGATE
void LODEditor::UpdateModeUI(EditorLODSystem* forSystem, const eEditorMode mode)
{
    ui->checkBoxLodEditorMode->setChecked(mode == eEditorMode::MODE_ALL_SCENE);

    panelsUpdater->Update();
}

void LODEditor::UpdateForceUI(EditorLODSystem* forSystem, const ForceValues& forceValues)
{
    const QSignalBlocker guard1(ui->enableForceDistance);
    const QSignalBlocker guard2(ui->forceSlider);
    const QSignalBlocker guard3(ui->forceLayer);

    const bool distanceModeSelected = (forceValues.flag & ForceValues::APPLY_DISTANCE) == ForceValues::APPLY_DISTANCE;
    ui->enableForceDistance->setChecked(distanceModeSelected);
    ui->forceSlider->setEnabled(distanceModeSelected);
    ui->forceLayer->setEnabled(!distanceModeSelected);

    ui->forceSlider->setValue(forceValues.distance);

    const LODComponentHolder* lodData = forSystem->GetActiveLODData();
    const uint32 layerItemsCount = lodData->GetLODLayersCount();
    if (ui->forceLayer->count() != layerItemsCount + 2)
    {
        CreateForceLayerValues(layerItemsCount);
    }

    if (forceValues.layer == LodComponent::LAST_LOD_LAYER)
    {
        ui->forceLayer->setCurrentIndex(ui->forceLayer->count() - 1);
    }
    else
    {
        int32 forceIndex = Min(forceValues.layer + 1, ui->forceLayer->count() - 1);
        ui->forceLayer->setCurrentIndex(forceIndex);
    }
}

void LODEditor::UpdateDistanceUI(EditorLODSystem* forSystem, const LODComponentHolder* lodData)
{
    DVASSERT(lodData != nullptr);

    const QSignalBlocker guard(ui->distanceSlider);

    const LodComponent& lc = lodData->GetLODComponent();
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
    UpdateTrianglesUI(GetCurrentEditorStatisticsSystem());

    for (int32 i = count; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        distanceWidgets[i].SetVisible(false);
    }
}

void LODEditor::UpdateActionUI(EditorLODSystem* forSystem)
{
    const bool canDeleteLod = forSystem->CanDeleteLOD();
    ui->buttonDeleteFirstLOD->setEnabled(canDeleteLod);
    ui->buttonDeleteLastLOD->setEnabled(canDeleteLod);

    bool canCreateLod = forSystem->CanCreateLOD();
    ui->lastLodToFrontButton->setEnabled(canCreateLod);
    ui->createPlaneLodButton->setEnabled(canCreateLod);
}

void LODEditor::UpdateTrianglesUI(EditorStatisticsSystem* forSystem)
{
    EditorLODSystem* lodSystem = GetCurrentEditorLODSystem();
    const auto& triangles = forSystem->GetTriangles(lodSystem->GetMode(), true);

    int32 index = EditorStatisticsSystem::INDEX_OF_FIRST_LOD_TRIANGLES;
    for (auto& dw : distanceWidgets)
    {
        dw.name->setText(Format("%d. (%u)", index, triangles[index]).c_str());
        ++index;
    }
}

//DELEGATE

void LODEditor::DistanceWidget::SetVisible(bool visible)
{
    name->setVisible(visible);
    distance->setVisible(visible);
}

EditorLODSystem* LODEditor::GetCurrentEditorLODSystem() const
{
    DVASSERT(QtMainWindow::Instance());

    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    if (scene != nullptr)
    {
        return scene->editorLODSystem;
    }

    return nullptr;
}

EditorStatisticsSystem* LODEditor::GetCurrentEditorStatisticsSystem() const
{
    DVASSERT(QtMainWindow::Instance());

    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    if (scene != nullptr)
    {
        return scene->editorStatisticsSystem;
    }

    return nullptr;
}

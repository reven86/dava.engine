#ifndef __LOD_EDITOR_H__
#define __LOD_EDITOR_H__

#include "Base/BaseTypes.h"

#include "Scene/System/EditorLODSystem.h"
#include "Scene/System/EditorStatisticsSystem.h"
#include "Tools/QtPosSaver/QtPosSaver.h"

#include <QWidget>

namespace Ui
{
class LODEditor;
}

class QLabel;
class QDoubleSpinBox;
class QLineEdit;
class SceneEditor2;
class SelectableGroup;
class Command2;
class QPushButton;
class QFrame;

class LazyUpdater;
class LODEditor : public QWidget, private EditorLODSystemUIDelegate, EditorStatisticsSystemUIDelegate
{
    Q_OBJECT

public:
    explicit LODEditor(QWidget* parent = nullptr);
    ~LODEditor() override;

private slots:

    //Panels buttons
    void LODEditorSettingsButtonClicked();
    void ViewLODButtonClicked();
    void EditLODButtonClicked();

    //force signals
    void ForceDistanceStateChanged(bool checked);
    void ForceDistanceChanged(int distance);
    void ForceLayerActivated(int index);

    //scene signals
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SolidChanged(SceneEditor2* scene, const DAVA::Entity* entity, bool value);

    //distance signals
    void LODDistanceChangedBySpinbox(double value);
    void LODDistanceIsChangingBySlider();
    void LODDistanceChangedBySlider();

    //mode signal
    void SceneOrSelectionModeSelected(bool allSceneModeActivated);

    //action
    void CopyLastLODToLOD0Clicked();
    void CreatePlaneLODClicked();
    void DeleteFirstLOD();
    void DeleteLastLOD();

private:
    void SetupSceneSignals();
    void SetupInternalUI();

    void SetupForceUI();
    void CreateForceLayerValues(DAVA::uint32 layersCount);

    void SetupPanelsButtonUI();
    void InvertFrameVisibility(QFrame* frame, QPushButton* frameButton);
    void UpdatePanelsUI(SceneEditor2* forScene);
    void UpdatePanelsForCurrentScene();

    void SetupDistancesUI();
    void InitDistanceSpinBox(QLabel* name, QDoubleSpinBox* spinbox, int index);
    void UpdateDistanceSpinboxesUI(const DAVA::Vector<DAVA::float32>& distances, DAVA::int32 count);

    void SetupActionsUI();

    //EditorLODSystemV2UIDelegate
    void UpdateModeUI(EditorLODSystem* forSystem, const eEditorMode mode) override;
    void UpdateForceUI(EditorLODSystem* forSystem, const ForceValues& forceValues) override;
    void UpdateDistanceUI(EditorLODSystem* forSystem, const LODComponentHolder* lodData) override;
    void UpdateActionUI(EditorLODSystem* forSystem) override;
    //end of EditorLODSystemV2UIDelegate

    //EditorStatisticsSystemUIDelegate
    void UpdateTrianglesUI(EditorStatisticsSystem* forSystem) override;
    //end of EditorStatisticsSystemUIDelegate

    EditorLODSystem* GetCurrentEditorLODSystem() const;
    EditorStatisticsSystem* GetCurrentEditorStatisticsSystem() const;

private:
    std::unique_ptr<Ui::LODEditor> ui;

    bool frameViewVisible = true;
    bool frameEditVisible = true;

    struct DistanceWidget
    {
        QLabel* name = nullptr;
        QDoubleSpinBox* distance = nullptr;
        void SetVisible(bool visible);
    };

    DAVA::Vector<DistanceWidget> distanceWidgets;

    LazyUpdater* panelsUpdater = nullptr;
};

#endif //#ifndef __LOD_EDITOR_H__

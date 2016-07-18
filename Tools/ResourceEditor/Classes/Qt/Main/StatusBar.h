#pragma once

#include "Base/BaseTypes.h"
#include "Scene/System/SystemDelegates.h"

#include <QStatusBar>

namespace DAVA
{
class Command;
}
class QLabel;
class SceneEditor2;
class SelectableGroup;
class StatusBar final : public QStatusBar, public SceneSelectionSystemDelegate
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = 0);

    //SceneSelectionSystemDelegate
    void OnSelectionBoxChanged(const DAVA::AABBox3& newBox) override;

public slots:
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);

    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void StructureChanged(SceneEditor2* scene, DAVA::Entity* parent);

    void UpdateByTimer();
    void OnSceneGeometryChaged(int width, int height);

private:
    void UpdateDistanceToCamera();
    void UpdateFPS();
    void SetDistanceToCamera(DAVA::float32 distance);
    void ResetDistanceToCamera();
    void UpdateSelectionBoxSize(SceneEditor2* scene);

    QLabel* distanceToCamera = nullptr;
    QLabel* fpsCounter = nullptr;
    QLabel* sceneGeometry = nullptr;
    QLabel* selectionBoxSize = nullptr;

    DAVA::uint64 lastTimeMS = 0;
};

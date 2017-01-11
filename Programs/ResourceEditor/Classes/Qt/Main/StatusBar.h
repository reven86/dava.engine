#pragma once

#include "Base/BaseTypes.h"
#include "Scene/System/SystemDelegates.h"

#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"

#include <QStatusBar>

class QLabel;
class SceneEditor2;
class SelectableGroup;
class StatusBar final : public QStatusBar, private DAVA::TArc::DataListener, public DAVA::TrackedObject
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = 0);

public slots:
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);

    void UpdateByTimer();
    void OnSceneGeometryChaged(DAVA::uint32 width, DAVA::uint32 height);

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void UpdateSelectionBoxUI(const DAVA::AABBox3& newBox);

    void UpdateDistanceToCamera();
    void UpdateFPS();
    void SetDistanceToCamera(DAVA::float32 distance);
    void ResetDistanceToCamera();
    void UpdateSelectionBoxSize();

    QLabel* distanceToCamera = nullptr;
    QLabel* fpsCounter = nullptr;
    QLabel* sceneGeometry = nullptr;
    QLabel* selectionBoxSize = nullptr;
    SceneEditor2* activeScene = nullptr;

    DAVA::TArc::DataWrapper selectionWrapper;

    DAVA::uint64 lastTimeMS = 0;
};

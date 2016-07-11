#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__

#include "DAVAEngine.h"

#include <QStatusBar>

namespace DAVA
{
class Command;
}
class QLabel;
class SceneEditor2;
class SelectableGroup;

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = 0);
    ~StatusBar();

public slots:
    void SceneActivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void CommandExecuted(SceneEditor2* scene, const DAVA::Command* command, bool redo);
    void StructureChanged(SceneEditor2* scene, DAVA::Entity* parent);

    void UpdateByTimer();

    void OnSceneGeometryChaged(int width, int height);

protected:
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

#endif // __STATUS_BAR_H__

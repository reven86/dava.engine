#pragma once

#include "UI/UI3DView.h"

#include "FileSystem/FilePath.h"

#include <QMap>
#include <QTabBar>
#include <QWidget>
#include <QMetaType>
#include <QMimeData>
#include <QUrl>

#include <memory>

namespace DAVA
{
class UIEvent;
class UIScreen;
class UI3DView;
class RenderWidget;
}

class SceneEditor2;
class MainTabBar;
class ScenePreviewDialog;
class Request;
class SelectableGroup;
class GlobalOperations;

class SceneTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SceneTabWidget(QWidget* parent);
    ~SceneTabWidget();

    bool CloseAllTabs(bool silent);

    void ShowScenePreview(const DAVA::FilePath& scenePath);
    void HideScenePreview();

signals:
    void CloseTabRequest(int index, Request* closeRequest);
    void Escape();

public slots:
    // this slot redirects any UIEvent to the active sceneProxy for processing
    void TabBarDataDropped(const QMimeData* data);

    // scene signals
    void MouseOverSelectedEntities(SceneEditor2* scene, const SelectableGroup* objects);
protected:
    MainTabBar* tabBar;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    ScenePreviewDialog* previewDialog = nullptr;

private:
    bool TestSceneCompatibility(const DAVA::FilePath& scenePath);

    int newSceneCounter = 0;
    SceneEditor2* curScene = nullptr;
    std::shared_ptr<GlobalOperations> globalOperations;
};

// tabBar widged to handle drop actions and emit signal about it
class MainTabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit MainTabBar(QWidget* parent = nullptr);

signals:
    void OnDrop(const QMimeData* mimeData);

protected:
    void dropEvent(QDropEvent* de) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
};

#ifndef __SCENE_TAB_WIDGET_H__
#define __SCENE_TAB_WIDGET_H__

#include <QMap>
#include <QTabBar>
#include <QWidget>
#include <QMetaType>
#include <QMimeData>
#include <QUrl>

#include "UI/UI3DView.h"

#include "FileSystem/FilePath.h"

namespace DAVA
{
class UIEvent;
class UIScreen;
class UI3DView;
}

class SceneEditor2;
class MainTabBar;
class DavaGLWidget;
class ScenePreviewDialog;
class Request;
class SelectableGroup;

class SceneTabWidget
: public QWidget
{
    Q_OBJECT

public:
    explicit SceneTabWidget(QWidget* parent);
    ~SceneTabWidget();

    int OpenTab();
    int OpenTab(const DAVA::FilePath& scenePath);
    bool CloseTab(int index);
    bool CloseAllTabs();

    int GetCurrentTab() const;
    void SetCurrentTab(int index);

    int GetTabCount() const;
    SceneEditor2* GetCurrentScene() const;
    SceneEditor2* GetTabScene(int index) const;

    void ShowScenePreview(const DAVA::FilePath& scenePath);
    void HideScenePreview();

    DavaGLWidget* GetDavaWidget() const;

signals:

    void CloseTabRequest(int index, Request* closeRequest);
    void Escape();

public slots:
    // this slot redirects any UIEvent to the active sceneProxy for processing
    void TabBarCurrentChanged(int index);
    void TabBarCloseRequest(int index);
    void TabBarCloseCurrentRequest();
    void TabBarDataDropped(const QMimeData* data);
    void DAVAWidgetDataDropped(const QMimeData* data);
    void OnDavaGLWidgetResized(int width, int height);

    // scene signals
    void MouseOverSelectedEntities(SceneEditor2* scene, const SelectableGroup* objects);
    void SceneSaved(SceneEditor2* scene);
    void SceneUpdated(SceneEditor2* scene);
    void SceneModifyStatusChanged(SceneEditor2* scene, bool modified);

protected:
    void OpenTabInternal(const DAVA::FilePath scenePathname, int tabIndex);

protected:
    MainTabBar* tabBar;
    DavaGLWidget* davaWidget;
    DAVA::UIScreen* davaUIScreen;
    DAVA::UI3DView* dava3DView;
    const int davaUIScreenID = 0;
    const int dava3DViewMargin = 3;

    void InitDAVAUI();
    void ReleaseDAVAUI();
    void UpdateTabName(int index);

    void SetTabScene(int index, SceneEditor2* scene);

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    ScenePreviewDialog* previewDialog = nullptr;

    int FindTab(const DAVA::FilePath& scenePath);

private:
    bool TestSceneCompatibility(const DAVA::FilePath& scenePath);
    void updateTabBarVisibility();

    int newSceneCounter = 0;
    SceneEditor2* curScene = nullptr;
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

#endif // __SCENE_TAB_WIDGET_H__

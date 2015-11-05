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
class EntityGroup;

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
    void OnDavaGLWidgetResized(int width, int height, int dpr);

    // scene signals
    void MouseOverSelectedEntities(SceneEditor2* scene, const EntityGroup* entities);
    void SceneSaved(SceneEditor2* scene);
    void SceneModifyStatusChanged(SceneEditor2* scene, bool modified);

private slots:
    void InitDAVAUI();
    void ReleaseDAVAUI();

protected:
    void OpenTabInternal(const DAVA::FilePath scenePathname, int tabIndex);

protected:
    MainTabBar* tabBar;
    DavaGLWidget* davaWidget;
    DAVA::UIScreen* davaUIScreen;
    DAVA::UI3DView* dava3DView;
    const int davaUIScreenID;
    const int dava3DViewMargin;

    void UpdateTabName(int index);

    void SetTabScene(int index, SceneEditor2* scene);

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    ScenePreviewDialog* previewDialog;

    int FindTab(const DAVA::FilePath& scenePath);

private:
    bool TestSceneCompatibility(const DAVA::FilePath& scenePath);
    void updateTabBarVisibility();

    int newSceneCounter;
    SceneEditor2* curScene;
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

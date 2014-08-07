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

#include "Qt/Main/davaglwidget.h"
#include "Qt/Scene/EntityGroup.h"
#include "Qt/Scene/SceneSignals.h"
#include "Qt/Scene/SceneTypes.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"

// old ui. should be removed later -->
class SceneEditorScreenMain;
// <--
class SceneEditor2;
class DAVAUI3DView;
class MainTabBar;

class ScenePreviewDialog;

Q_DECLARE_METATYPE(SceneEditor2 *);

class SceneTabWidget : public QWidget
{
	Q_OBJECT

public:
	SceneTabWidget(QWidget *parent);
	~SceneTabWidget();

	int OpenTab();
	int OpenTab(const DAVA::FilePath &scenePath);
	bool CloseTab(int index);
    bool CloseAllTabs();
    
	int GetCurrentTab() const;
	void SetCurrentTab(int index);

	int GetTabCount() const;
	SceneEditor2* GetCurrentScene() const;
	SceneEditor2* GetTabScene(int index) const;

	void ShowScenePreview(const DAVA::FilePath &scenePath);
	void HideScenePreview();
    
    void AddToolWidget(QWidget *widget);

	DavaGLWidget * GetDavaWidget() const;
   
signals:
    
    void CloseTabRequest(int index, Request *closeRequest);
	void Escape();
    
public slots:
	// this slot redirects any UIEvent to the active sceneProxy for processing
	void ProcessDAVAUIEvent(DAVA::UIEvent *event);
	void TabBarCurrentChanged(int index);
	void TabBarCloseRequest(int index);
	void TabBarCloseCurrentRequest();
	void TabBarDataDropped(const QMimeData *data);
	void DAVAWidgetDataDropped(const QMimeData *data);

	// scene signals
	void MouseOverSelectedEntities(SceneEditor2* scene, const EntityGroup *entities);
	void SceneSaved(SceneEditor2 *scene);
	void SceneModifyStatusChanged(SceneEditor2 *scene, bool modified);

protected:
	MainTabBar *tabBar;
	DavaGLWidget *davaWidget;
	DAVA::UIScreen *davaUIScreen;
	DAVA::UI3DView *dava3DView;
	const int davaUIScreenID;
	const int dava3DViewMargin;

	QWidget *toolWidgetContainer;
	QLayout *toolWidgetLayout;

	void InitDAVAUI();
	void ReleaseDAVAUI();
	void UpdateTabName(int index);
	void UpdateToolWidget();

	void SetTabScene(int index, SceneEditor2* scene);

	virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);
	virtual void keyReleaseEvent(QKeyEvent * event);

	ScenePreviewDialog *previewDialog;

	int FindTab(const DAVA::FilePath & scenePath);

private:
    bool TestSceneCompatibility(const DAVA::FilePath &scenePath);

	int newSceneCounter;
	SceneEditor2 *curScene;
};

// this is helper class
// it is only used to grab all UIEvents from 3dView and
// redirect them to the specified SceneTabWidget
class DAVAUI3DView : public DAVA::UI3DView
{
public:
	DAVAUI3DView(SceneTabWidget *tw, const DAVA::Rect &rect) 
		: DAVA::UI3DView(rect)
		, tabWidget(tw)
	{
		SetInputEnabled(true);
	}

	virtual void Input(DAVA::UIEvent *event)
	{
		if(NULL != tabWidget)
		{
			tabWidget->ProcessDAVAUIEvent(event);
		}
	}

protected:
	SceneTabWidget *tabWidget;
};

// tabBar widged to handle drop actions and emit signal about it
class MainTabBar : public QTabBar
{
	Q_OBJECT

public:
	MainTabBar(QWidget* parent = 0);

signals:
	void OnDrop(const QMimeData *mimeData);

protected:
	virtual void dropEvent(QDropEvent *de);
	virtual void dragEnterEvent(QDragEnterEvent *event);
};

#endif // __SCENE_TAB_WIDGET_H__

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


#include "Scene/SceneTabWidget.h"

#include "Main/Request.h"
#include "Main/mainwindow.h"
#include "Scene/SceneEditor2.h"
#include "Tools/QtLabelWithActions/QtLabelWithActions.h"
#include "Tools/MimeData/MimeDataHelper2.h"
#include "Deprecated/ScenePreviewDialog.h"
#include "MaterialEditor/MaterialAssignSystem.h"

#include "Platform/SystemTimer.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcut>
#include <QDebug>


SceneTabWidget::SceneTabWidget(QWidget *parent)
	: QWidget(parent)
	, davaUIScreenID(0)
	, dava3DViewMargin(3)
	, previewDialog(NULL)
	, newSceneCounter(0)
	, curScene(NULL)
{
	this->setMouseTracking(true);

	// create Qt controls and add them into layout
	// 
	// tab bar
	tabBar = new MainTabBar(this);
	tabBar->setTabsClosable(true);
	tabBar->setMovable(true);
	tabBar->setUsesScrollButtons(true);
	tabBar->setExpanding(false);

	// davawidget to display DAVAEngine content
	davaWidget = new DavaGLWidget(this);
    tabBar->setMinimumWidth(davaWidget->minimumWidth());
    setMinimumWidth(davaWidget->minimumWidth());
    setMinimumHeight(davaWidget->minimumHeight() + tabBar->sizeHint().height());
    
	// put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(tabBar);
	layout->addWidget(davaWidget);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
	setAcceptDrops(true);
    
	// create DAVA UI
	InitDAVAUI();

	QObject::connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(TabBarCurrentChanged(int)));
	QObject::connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(TabBarCloseRequest(int)));
	QObject::connect(tabBar, SIGNAL(OnDrop(const QMimeData *)), this, SLOT(TabBarDataDropped(const QMimeData *)));
	QObject::connect(davaWidget, SIGNAL(OnDrop(const QMimeData *)), this, SLOT(DAVAWidgetDataDropped(const QMimeData *)));
    QObject::connect(davaWidget, SIGNAL(Resized(int, int, int)), this, SLOT(OnDavaGLWidgetResized(int, int, int)));

	QObject::connect(SceneSignals::Instance(), SIGNAL(MouseOverSelection(SceneEditor2*, const EntityGroup*)), this, SLOT(MouseOverSelectedEntities(SceneEditor2*, const EntityGroup*)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Saved(SceneEditor2*)), this, SLOT(SceneSaved(SceneEditor2*)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(ModifyStatusChanged(SceneEditor2 *, bool)), this, SLOT(SceneModifyStatusChanged(SceneEditor2 *, bool)));

	SetCurrentTab(0);

    auto mouseWheelHandler = [&]( int ofs )
    {
        if ( curScene == nullptr )
            return;
        const auto moveCamera = SettingsManager::GetValue( Settings::General_Mouse_WheelMoveCamera ).AsBool();
        if ( !moveCamera )
            return;

        const auto reverse = SettingsManager::GetValue( Settings::General_Mouse_InvertWheel ).AsBool() ? -1 : 1;
#ifdef Q_OS_MAC
        ofs *= reverse * -1;
#else
        ofs *= reverse;
#endif

        curScene->cameraSystem->MoveToStep( ofs );
    };
    connect(davaWidget, &DavaGLWidget::mouseScrolled, mouseWheelHandler);

    auto moveToSelectionHandler = [&]
    {
        if ( curScene == nullptr )
            return;
        curScene->cameraSystem->MoveToSelection();
    };
    auto moveToSelectionHandlerHotkey = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_D ), this );
    connect( moveToSelectionHandlerHotkey, &QShortcut::activated, moveToSelectionHandler );
}

SceneTabWidget::~SceneTabWidget()
{
	SafeRelease(previewDialog);

	ReleaseDAVAUI();
}

void SceneTabWidget::InitDAVAUI()
{
    dava3DView = new DAVA::UI3DView(DAVA::Rect(dava3DViewMargin, dava3DViewMargin, 0, 0));
    dava3DView->SetInputEnabled(true, true);

	davaUIScreen = new DAVA::UIScreen();
	davaUIScreen->AddControl(dava3DView);

	UIScreenManager::Instance()->RegisterScreen(davaUIScreenID, davaUIScreen);
	UIScreenManager::Instance()->SetScreen(davaUIScreenID);
}

void SceneTabWidget::ReleaseDAVAUI()
{
	SafeRelease(davaUIScreen);
}

int SceneTabWidget::OpenTab()
{
	QtMainWindow::Instance()->WaitStart("Opening scene...", "Creating new scene.");
    
    DAVA::FilePath scenePath = (QString("newscene") + QString::number(++newSceneCounter)).toStdString();
	scenePath.ReplaceExtension(".sc2");

    int tabIndex = tabBar->addTab(scenePath.GetFilename().c_str());
    tabBar->setTabToolTip(tabIndex, scenePath.GetAbsolutePathname().c_str());

    OpenTabInternal(scenePath, tabIndex);
    
	return tabIndex;
}

int SceneTabWidget::OpenTab(const DAVA::FilePath &scenePath)
{
    HideScenePreview();
    
    int tabIndex = FindTab(scenePath);
    if(tabIndex != -1)
    {
        SetCurrentTab(tabIndex);
        return tabIndex;
    }
    
    if (!TestSceneCompatibility(scenePath))
    {
        return -1;
    }
    
    QtMainWindow::Instance()->WaitStart("Opening scene...", scenePath.GetAbsolutePathname().c_str());

    tabIndex = tabBar->addTab(scenePath.GetFilename().c_str());
    tabBar->setTabToolTip(tabIndex, scenePath.GetAbsolutePathname().c_str());
    
    OpenTabInternal(scenePath, tabIndex);

    return tabIndex;
}

void SceneTabWidget::OpenTabInternal(const DAVA::FilePath scenePathname, int tabIndex)
{
    SceneEditor2 *scene = new SceneEditor2();
    scene->SetScenePath(scenePathname);
    
    if(scenePathname.Exists())
    {
        bool sceneWasLoaded = scene->Load(scenePathname);
        if(!sceneWasLoaded)
        {
            QMessageBox::critical( this, "Open scene error.", "Unexpected opening error. See logs for more info." );
        }
    }
    
    SetTabScene(tabIndex, scene);
    SetCurrentTab(tabIndex);

    QtMainWindow::Instance()->WaitStop();
    updateTabBarVisibility();
}

bool SceneTabWidget::TestSceneCompatibility(const DAVA::FilePath &scenePath)
{
    VersionInfo::SceneVersion sceneVersion = SceneFileV2::LoadSceneVersion(scenePath);

    if (sceneVersion.IsValid())
    {
        VersionInfo::eStatus status = VersionInfo::Instance()->TestVersion(sceneVersion);
        const uint32 curVersion = VersionInfo::Instance()->GetCurrentVersion().version;

        switch (status)
        {
        case VersionInfo::COMPATIBLE:
        {
            const String& branches = VersionInfo::Instance()->UnsupportedTagsMessage(sceneVersion);
            const QString msg = QString("Scene was created with older version or another branch of ResourceEditor. Saving scene will broke compatibility.\nScene version: %1 (required %2)\n\nNext tags will be added:\n%3\n\nContinue opening?").arg(sceneVersion.version).arg(curVersion).arg(branches.c_str());
            const QMessageBox::StandardButton result = QMessageBox::warning(this, "Compatibility warning", msg, QMessageBox::Open | QMessageBox::Cancel, QMessageBox::Open);
            if ( result != QMessageBox::Open )
            {
                return false;
            }
            break;
        }
        case VersionInfo::INVALID:
        {
            const String& branches = VersionInfo::Instance()->NoncompatibleTagsMessage(sceneVersion);
            const QString msg = QString("Scene was created with incompatible version or branch of ResourceEditor.\nScene version: %1 (required %2)\nNext tags aren't implemented in current branch:\n%3").arg(sceneVersion.version).arg(curVersion).arg(branches.c_str());
            QMessageBox::critical(this, "Compatibility error", msg);
            return false;
        }
        default:
            break;
        }
    }

    return true;
}

void SceneTabWidget::updateTabBarVisibility()
{
    const bool visible = (tabBar->count() > 0);
    tabBar->setVisible(visible);
}

bool SceneTabWidget::CloseTab(int index)
{
    Request request;
    
    emit CloseTabRequest(index, &request);
    
    if(!request.IsAccepted())
        return false;

	SceneEditor2 *scene = GetTabScene(index);
    if(index == tabBar->currentIndex())
    {
        curScene = NULL;
        dava3DView->SetScene(NULL);
        SceneSignals::Instance()->EmitDeactivated(scene);
    }
    
    SafeRelease(scene);
    tabBar->removeTab(index);
    updateTabBarVisibility();

    return true;
}


int SceneTabWidget::GetCurrentTab() const
{
	return tabBar->currentIndex();
}

void SceneTabWidget::SetCurrentTab(int index)
{
    davaWidget->setEnabled(false);

    if(index >= 0 && index < tabBar->count())
	{
        SceneEditor2 *oldScene = curScene;
		curScene = GetTabScene(index);

		if(NULL != oldScene)
		{
            oldScene->Deactivate();
		}

		tabBar->blockSignals(true);
		tabBar->setCurrentIndex(index);
		tabBar->blockSignals(false);

		if(NULL != curScene)
		{
			dava3DView->SetScene(curScene);
			curScene->SetViewportRect(dava3DView->GetRect());

            curScene->Activate();

			davaWidget->setEnabled(true);
		}
	}
}

SceneEditor2* SceneTabWidget::GetTabScene(int index) const
{
	SceneEditor2 *ret = NULL;

	if(index >= 0 && index < tabBar->count())
	{
		ret = tabBar->tabData(index).value<SceneEditor2 *>();
	}

	return ret;
}

void SceneTabWidget::SetTabScene(int index, SceneEditor2* scene)
{
	if(index >= 0 && index < tabBar->count())
	{
		tabBar->setTabData(index, qVariantFromValue(scene));
	}
}

int SceneTabWidget::GetTabCount() const
{
	return tabBar->count();
}


void SceneTabWidget::TabBarCurrentChanged(int index)
{
	SetCurrentTab(index);
}

void SceneTabWidget::TabBarCloseRequest(int index)
{
	CloseTab(index);
}

void SceneTabWidget::TabBarCloseCurrentRequest()
{
	int tabIndex = GetCurrentTab();
	if(tabIndex != -1)
	{
		CloseTab(tabIndex);
	}
}

void SceneTabWidget::TabBarDataDropped(const QMimeData *data)
{
	QList<QUrl> urls = data->urls();
	for(int i = 0; i < urls.size(); ++i)
	{
		QString path = urls[i].toLocalFile();
        if(QFileInfo(path).suffix() == "sc2")
        {
            QtMainWindow::Instance()->OpenScene(path);
        }
	}
}

void SceneTabWidget::DAVAWidgetDataDropped(const QMimeData *data)
{
	if(NULL != curScene)
	{
        QList<QUrl> urls = data->urls();
        for(int i = 0; i < urls.size(); ++i)
        {
			QString path = urls[i].toLocalFile();
			if(QFileInfo(path).suffix() == "sc2")
            {
                DAVA::Vector3 pos;
                    
                // check if there is intersection with landscape. ray from camera to mouse pointer
                // if there is - we should move opening scene to that point
                if(!curScene->collisionSystem->LandRayTestFromCamera(pos))
                {
                    DAVA::Landscape *landscape = curScene->collisionSystem->GetLandscape();
                    if( NULL != landscape && NULL != landscape->GetHeightmap() && landscape->GetHeightmap()->Size() > 0)
                    {
                        curScene->collisionSystem->GetLandscape()->PlacePoint(DAVA::Vector3(), pos);
                    }
                }

                QtMainWindow::Instance()->WaitStart("Adding object to scene", path);
                if (TestSceneCompatibility(DAVA::FilePath(path.toStdString())))
                {
                    curScene->structureSystem->Add(path.toStdString(), pos);
                }
                QtMainWindow::Instance()->WaitStop();
            }
        }
	}
	else
	{
		TabBarDataDropped(data);
	}
}

void SceneTabWidget::MouseOverSelectedEntities(SceneEditor2* scene, const EntityGroup *entities)
{
	static QCursor cursorMove(QPixmap(":/QtIcons/curcor_move.png"));
	static QCursor cursorRotate(QPixmap(":/QtIcons/curcor_rotate.png"));
	static QCursor cursorScale(QPixmap(":/QtIcons/curcor_scale.png"));

    auto view = davaWidget->GetGLView();

    if(GetCurrentScene() == scene && nullptr != entities)
	{
		switch(scene->modifSystem->GetModifMode())
		{
		case ST_MODIF_MOVE:
            view->setCursor(cursorMove);
            break;
		case ST_MODIF_ROTATE:
            view->setCursor(cursorRotate);
            break;
		case ST_MODIF_SCALE:
            view->setCursor(cursorScale);
            break;
		case ST_MODIF_OFF:
		default:
            view->unsetCursor();
            break;
		}
	}
	else
	{
        view->unsetCursor();
    }
}

void SceneTabWidget::SceneSaved(SceneEditor2 *scene)
{
	// update scene name on tabBar
	for(int i = 0; i < tabBar->count(); ++i)
	{
		SceneEditor2 *tabScene = GetTabScene(i);
		if(tabScene == scene)
		{
			UpdateTabName(i);
			break;
		}
	}

}

void SceneTabWidget::SceneModifyStatusChanged(SceneEditor2 *scene, bool modified)
{
	// update scene name on tabBar
	for(int i = 0; i < tabBar->count(); ++i)
	{
		SceneEditor2 *tabScene = GetTabScene(i);
		if(tabScene == scene)
		{
			UpdateTabName(i);
			break;
		}
	}
}

void SceneTabWidget::OnDavaGLWidgetResized(int width, int height, int dpr)
{
    int scaledWidth = width * dpr;
    int scaledHeight = height * dpr;
    
    davaUIScreen->SetSize(DAVA::Vector2(scaledWidth, scaledHeight));
    dava3DView->SetSize(DAVA::Vector2(scaledWidth - 2 * dava3DViewMargin, scaledHeight - 2 * dava3DViewMargin));

    SceneEditor2* scene = GetTabScene(tabBar->currentIndex());
    if(NULL != scene)
    {
        scene->SetViewportRect(dava3DView->GetRect());
    }
}


void SceneTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if(MimeDataHelper2<DAVA::NMaterial>::IsValid(event->mimeData()) ||
		event->mimeData()->hasUrls())
	{
		event->acceptProposedAction();
	}
	else
	{
		event->setDropAction(Qt::IgnoreAction);
		event->accept();
	}
}

void SceneTabWidget::dropEvent(QDropEvent *event)
{
	TabBarDataDropped(event->mimeData());
}

void SceneTabWidget::keyReleaseEvent(QKeyEvent * event)
{
	if(event->modifiers() == Qt::NoModifier)
	{
		if(event->key() == Qt::Key_Escape)
		{
			emit Escape();
		}
	}
}

void SceneTabWidget::UpdateTabName(int index)
{
	SceneEditor2 *scene = GetTabScene(index);
	if(NULL != scene)
	{
		DAVA::String tabName = scene->GetScenePath().GetFilename();
		DAVA::String tabTooltip = scene->GetScenePath().GetAbsolutePathname();

		if(scene->IsChanged())
		{
			tabName += "*";
		}

		tabBar->setTabText(index, tabName.c_str());
		tabBar->setTabToolTip(index, tabTooltip.c_str());
	}
}

SceneEditor2* SceneTabWidget::GetCurrentScene() const
{
	return curScene;
}

void SceneTabWidget::ShowScenePreview(const DAVA::FilePath &scenePath)
{
	if(!previewDialog)
    {
        previewDialog = new ScenePreviewDialog();
    }

	if(scenePath.IsEqualToExtension(".sc2"))
	{
		previewDialog->Show(scenePath);
	}
	else
	{
		previewDialog->Close();
	}
}

void SceneTabWidget::HideScenePreview()
{
	if(previewDialog && previewDialog->GetParent())
	{
		previewDialog->Close();
	}
}

DavaGLWidget * SceneTabWidget::GetDavaWidget() const
{
	return davaWidget;
}

int SceneTabWidget::FindTab( const DAVA::FilePath & scenePath )
{
	for(int i = 0; i < tabBar->count(); ++i)
	{
		SceneEditor2 *tabScene = GetTabScene(i);
		if(tabScene && (tabScene->GetScenePath() == scenePath))
		{
			return i;
		}
	}

	return -1;
}

bool SceneTabWidget::CloseAllTabs()
{
    uint32 count = GetTabCount();
    while(count)
    {
        if(!CloseTab(GetCurrentTab()))
        {
            return false;
        }
        count--;
    }
    return true;
}


MainTabBar::MainTabBar(QWidget* parent /* = 0 */)
	: QTabBar(parent)
{
	setAcceptDrops(true);
}

void MainTabBar::dragEnterEvent(QDragEnterEvent *event)
{
	if(event->mimeData()->hasUrls())
	{
		event->acceptProposedAction();
	}
	else
	{
		event->setDropAction(Qt::IgnoreAction);
		event->accept();
	}
}

void MainTabBar::dropEvent(QDropEvent *event)
{
	if(event->mimeData()->hasUrls())
	{
		emit OnDrop(event->mimeData());
	}
}



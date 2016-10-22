#include "Scene/SceneTabWidget.h"

#include "UI/Focus/UIFocusComponent.h"

#include "Main/Request.h"
#include "Main/mainwindow.h"
#include "Scene/SceneEditor2.h"
#include "Classes/Qt/GlobalOperations.h"
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
#include <QTimer>

SceneTabWidget::SceneTabWidget(QWidget* parent)
    : QWidget(parent)
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
    davaWidget = new DavaGLWidget();
    tabBar->setMinimumWidth(davaWidget->minimumWidth());
    setMinimumWidth(davaWidget->minimumWidth());
    setMinimumHeight(davaWidget->minimumHeight() + tabBar->sizeHint().height());

    // put tab bar and davawidget into vertical layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tabBar);
    QTimer::singleShot(1500, [layout, this]
                       {
                           davaWidget->setParent(this);
                           layout->addWidget(davaWidget);
                       });
    layout->setMargin(0);
    layout->setSpacing(1);
    setLayout(layout);
    setAcceptDrops(true);

    // create DAVA UI
    InitDAVAUI();

    QObject::connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(TabBarCurrentChanged(int)));
    QObject::connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(TabBarCloseRequest(int)));
    QObject::connect(tabBar, SIGNAL(OnDrop(const QMimeData*)), this, SLOT(TabBarDataDropped(const QMimeData*)));
    QObject::connect(davaWidget, SIGNAL(OnDrop(const QMimeData*)), this, SLOT(DAVAWidgetDataDropped(const QMimeData*)));
    QObject::connect(davaWidget, SIGNAL(Resized(int, int)), this, SLOT(OnDavaGLWidgetResized(int, int)));

    QObject::connect(SceneSignals::Instance(), SIGNAL(MouseOverSelection(SceneEditor2*, const SelectableGroup*)), this, SLOT(MouseOverSelectedEntities(SceneEditor2*, const SelectableGroup*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Saved(SceneEditor2*)), this, SLOT(SceneSaved(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Updated(SceneEditor2*)), this, SLOT(SceneUpdated(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(ModifyStatusChanged(SceneEditor2*, bool)), this, SLOT(SceneModifyStatusChanged(SceneEditor2*, bool)));

    SetCurrentTab(0);

    auto mouseWheelHandler = [&](int ofs)
    {
        if (curScene == nullptr)
            return;
        const auto moveCamera = SettingsManager::GetValue(Settings::General_Mouse_WheelMoveCamera).AsBool();
        if (!moveCamera)
            return;

        const auto reverse = SettingsManager::GetValue(Settings::General_Mouse_InvertWheel).AsBool() ? -1 : 1;
#ifdef Q_OS_MAC
        ofs *= reverse * -1;
#else
        ofs *= reverse;
#endif

        curScene->cameraSystem->MoveToStep(ofs);
    };
    connect(davaWidget, &DavaGLWidget::mouseScrolled, mouseWheelHandler);

    auto moveToSelectionHandler = [&]
    {
        if (curScene == nullptr)
            return;
        curScene->cameraSystem->MoveToSelection();
    };
    auto moveToSelectionHandlerHotkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
    connect(moveToSelectionHandlerHotkey, &QShortcut::activated, moveToSelectionHandler);
}

SceneTabWidget::~SceneTabWidget()
{
    if (previewDialog != nullptr)
    {
        previewDialog->RemoveFromParent();
    }
    SafeRelease(previewDialog);

    DVASSERT(GetTabCount() == 0);

    ReleaseDAVAUI();
}

void SceneTabWidget::Init(const std::shared_ptr<GlobalOperations>& globalOperations_)
{
    globalOperations = globalOperations_;
}

void SceneTabWidget::InitDAVAUI()
{
    dava3DView = new DAVA::UI3DView(DAVA::Rect(dava3DViewMargin, dava3DViewMargin, 0, 0));
    dava3DView->SetInputEnabled(true, true);
    dava3DView->GetOrCreateComponent<DAVA::UIFocusComponent>();
    dava3DView->SetName(DAVA::FastName("Scene Tab 3D View"));

    davaUIScreen = new DAVA::UIScreen();

    DAVA::UIScreenManager::Instance()->RegisterScreen(davaUIScreenID, davaUIScreen);
    DAVA::UIScreenManager::Instance()->SetScreen(davaUIScreenID);
}

void SceneTabWidget::ReleaseDAVAUI()
{
    SafeRelease(dava3DView);
    SafeRelease(davaUIScreen);
}

int SceneTabWidget::OpenTab()
{
    DVASSERT(globalOperations);
    WaitDialogGuard guard(globalOperations, "Opening scene...", "Creating new scene.");

    DAVA::FilePath scenePath = (QString("newscene") + QString::number(++newSceneCounter)).toStdString();
    scenePath.ReplaceExtension(".sc2");

    int tabIndex = tabBar->addTab(scenePath.GetFilename().c_str());
    tabBar->setTabToolTip(tabIndex, scenePath.GetAbsolutePathname().c_str());

    OpenTabInternal(scenePath, tabIndex);

    return tabIndex;
}

int SceneTabWidget::OpenTab(const DAVA::FilePath& scenePath)
{
    HideScenePreview();

    int tabIndex = FindTab(scenePath);
    if (tabIndex != -1)
    {
        SetCurrentTab(tabIndex);
        return tabIndex;
    }

    if (!TestSceneCompatibility(scenePath))
    {
        return -1;
    }

    WaitDialogGuard guard(globalOperations, "Opening scene...", scenePath.GetAbsolutePathname());

    tabIndex = tabBar->addTab(scenePath.GetFilename().c_str());
    tabBar->setTabToolTip(tabIndex, scenePath.GetAbsolutePathname().c_str());

    OpenTabInternal(scenePath, tabIndex);

    return tabIndex;
}

void SceneTabWidget::OpenTabInternal(const DAVA::FilePath scenePathname, int tabIndex)
{
    SceneEditor2* scene = new SceneEditor2();
    scene->SetScenePath(scenePathname);

    if (DAVA::FileSystem::Instance()->Exists(scenePathname))
    {
        DAVA::SceneFileV2::eError sceneWasLoaded = scene->LoadScene(scenePathname);
        if (sceneWasLoaded != DAVA::SceneFileV2::ERROR_NO_ERROR)
        {
            QMessageBox::critical(this, "Open scene error.", "Unexpected opening error. See logs for more info.");
        }
    }
    scene->EnableEditorSystems();

    SetTabScene(tabIndex, scene);
    SetCurrentTab(tabIndex);

    updateTabBarVisibility();
}

bool SceneTabWidget::TestSceneCompatibility(const DAVA::FilePath& scenePath)
{
    DAVA::VersionInfo::SceneVersion sceneVersion = DAVA::SceneFileV2::LoadSceneVersion(scenePath);

    if (sceneVersion.IsValid())
    {
        DAVA::VersionInfo::eStatus status = DAVA::VersionInfo::Instance()->TestVersion(sceneVersion);
        const DAVA::uint32 curVersion = DAVA::VersionInfo::Instance()->GetCurrentVersion().version;

        switch (status)
        {
        case DAVA::VersionInfo::COMPATIBLE:
        {
            const DAVA::String& branches = DAVA::VersionInfo::Instance()->UnsupportedTagsMessage(sceneVersion);
            const QString msg = QString("Scene was created with older version or another branch of ResourceEditor. Saving scene will broke compatibility.\nScene version: %1 (required %2)\n\nNext tags will be added:\n%3\n\nContinue opening?").arg(sceneVersion.version).arg(curVersion).arg(branches.c_str());
            const QMessageBox::StandardButton result = QMessageBox::warning(this, "Compatibility warning", msg, QMessageBox::Open | QMessageBox::Cancel, QMessageBox::Open);
            if (result != QMessageBox::Open)
            {
                return false;
            }
            break;
        }
        case DAVA::VersionInfo::INVALID:
        {
            const DAVA::String& branches = DAVA::VersionInfo::Instance()->NoncompatibleTagsMessage(sceneVersion);
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
    return CloseTabInternal(index, false);
}

bool SceneTabWidget::CloseTabInternal(int index, bool silent)
{
    if (silent == false)
    {
        Request request;
        emit CloseTabRequest(index, &request);

        if (!request.IsAccepted())
            return false;
    }

    SceneEditor2* scene = GetTabScene(index);
    if (index == tabBar->currentIndex())
    {
        curScene = NULL;
        dava3DView->SetScene(NULL);
        davaUIScreen->RemoveControl(dava3DView);
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

    if (index >= 0 && index < tabBar->count())
    {
        SceneEditor2* oldScene = curScene;
        curScene = GetTabScene(index);

        if (NULL != oldScene)
        {
            oldScene->Deactivate();
        }

        tabBar->blockSignals(true);
        tabBar->setCurrentIndex(index);
        tabBar->blockSignals(false);

        if (NULL != curScene)
        {
            if (dava3DView->GetParent() == nullptr)
            {
                const DAVA::List<DAVA::UIControl*>& children = davaUIScreen->GetChildren();
                if (children.empty())
                {
                    davaUIScreen->AddControl(dava3DView);
                }
                else
                {
                    davaUIScreen->InsertChildBelow(dava3DView, children.front());
                }
            }

            dava3DView->SetScene(curScene);
            curScene->SetViewportRect(dava3DView->GetRect());

            curScene->Activate();

            davaWidget->setEnabled(true);
        }
    }
}

SceneEditor2* SceneTabWidget::GetTabScene(int index) const
{
    SceneEditor2* ret = NULL;

    if (index >= 0 && index < tabBar->count())
    {
        ret = tabBar->tabData(index).value<SceneEditor2*>();
    }

    return ret;
}

void SceneTabWidget::SetTabScene(int index, SceneEditor2* scene)
{
    if (index >= 0 && index < tabBar->count())
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
    if (tabIndex != -1)
    {
        CloseTab(tabIndex);
    }
}

void SceneTabWidget::TabBarDataDropped(const QMimeData* data)
{
    QList<QUrl> urls = data->urls();
    for (int i = 0; i < urls.size(); ++i)
    {
        QString path = urls[i].toLocalFile();
        if (QFileInfo(path).suffix() == "sc2")
        {
            globalOperations->CallAction(GlobalOperations::OpenScene, DAVA::Any(path.toStdString()));
        }
    }
}

void SceneTabWidget::DAVAWidgetDataDropped(const QMimeData* data)
{
    if (NULL != curScene)
    {
        QList<QUrl> urls = data->urls();
        for (int i = 0; i < urls.size(); ++i)
        {
            QString path = urls[i].toLocalFile();
            if (QFileInfo(path).suffix() == "sc2")
            {
                DAVA::Vector3 pos;

                // check if there is intersection with landscape. ray from camera to mouse pointer
                // if there is - we should move opening scene to that point
                if (!curScene->collisionSystem->LandRayTestFromCamera(pos))
                {
                    DAVA::Landscape* landscape = curScene->collisionSystem->GetLandscape();
                    if (NULL != landscape && NULL != landscape->GetHeightmap() && landscape->GetHeightmap()->Size() > 0)
                    {
                        curScene->collisionSystem->GetLandscape()->PlacePoint(DAVA::Vector3(), pos);
                    }
                }

                WaitDialogGuard guard(globalOperations, "Adding object to scene", path.toStdString());
                if (TestSceneCompatibility(DAVA::FilePath(path.toStdString())))
                {
                    curScene->structureSystem->Add(path.toStdString(), pos);
                }
            }
        }
    }
    else
    {
        TabBarDataDropped(data);
    }
}

void SceneTabWidget::MouseOverSelectedEntities(SceneEditor2* scene, const SelectableGroup* objects)
{
    static QCursor cursorMove(QPixmap(":/QtIcons/curcor_move.png"));
    static QCursor cursorRotate(QPixmap(":/QtIcons/curcor_rotate.png"));
    static QCursor cursorScale(QPixmap(":/QtIcons/curcor_scale.png"));

    auto view = davaWidget->GetGLView();

    if ((GetCurrentScene() == scene) && (objects != nullptr))
    {
        switch (scene->modifSystem->GetTransformType())
        {
        case Selectable::TransformType::Translation:
            view->setCursor(cursorMove);
            break;
        case Selectable::TransformType::Rotation:
            view->setCursor(cursorRotate);
            break;
        case Selectable::TransformType::Scale:
            view->setCursor(cursorScale);
            break;
        case Selectable::TransformType::Disabled:
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

void SceneTabWidget::SceneUpdated(SceneEditor2* scene)
{
    // update scene name on tabBar
    for (int i = 0; i < tabBar->count(); ++i)
    {
        SceneEditor2* tabScene = GetTabScene(i);
        if (tabScene == scene)
        {
            UpdateTabName(i);
            break;
        }
    }
}

void SceneTabWidget::SceneSaved(SceneEditor2* scene)
{
    SceneUpdated(scene);
}

void SceneTabWidget::SceneModifyStatusChanged(SceneEditor2* scene, bool modified)
{
    // update scene name on tabBar
    for (int i = 0; i < tabBar->count(); ++i)
    {
        SceneEditor2* tabScene = GetTabScene(i);
        if (tabScene == scene)
        {
            UpdateTabName(i);
            break;
        }
    }
}

void SceneTabWidget::OnDavaGLWidgetResized(int width, int height)
{
    davaUIScreen->SetSize(DAVA::Vector2(width, height));
    dava3DView->SetSize(DAVA::Vector2(width - 2 * dava3DViewMargin, height - 2 * dava3DViewMargin));

    SceneEditor2* scene = GetTabScene(tabBar->currentIndex());
    if (NULL != scene)
    {
        scene->SetViewportRect(dava3DView->GetRect());
    }
}

void SceneTabWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (MimeDataHelper2<DAVA::NMaterial>::IsValid(event->mimeData()) ||
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

void SceneTabWidget::dropEvent(QDropEvent* event)
{
    TabBarDataDropped(event->mimeData());
}

void SceneTabWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->modifiers() == Qt::NoModifier)
    {
        if (event->key() == Qt::Key_Escape)
        {
            emit Escape();
        }
    }
}

void SceneTabWidget::UpdateTabName(int index)
{
    SceneEditor2* scene = GetTabScene(index);
    if (NULL != scene)
    {
        DAVA::String tabName = scene->GetScenePath().GetFilename();
        DAVA::String tabTooltip = scene->GetScenePath().GetAbsolutePathname();

        if (scene->IsChanged())
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

void SceneTabWidget::ShowScenePreview(const DAVA::FilePath& scenePath)
{
    if (!previewDialog)
    {
        previewDialog = new ScenePreviewDialog();
    }

    if (scenePath.IsEqualToExtension(".sc2"))
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
    if (previewDialog && previewDialog->GetParent())
    {
        previewDialog->Close();
    }
}

DavaGLWidget* SceneTabWidget::GetDavaWidget() const
{
    return davaWidget;
}

int SceneTabWidget::FindTab(const DAVA::FilePath& scenePath)
{
    for (int i = 0; i < tabBar->count(); ++i)
    {
        SceneEditor2* tabScene = GetTabScene(i);
        if (tabScene && (tabScene->GetScenePath() == scenePath))
        {
            return i;
        }
    }

    return -1;
}

bool SceneTabWidget::CloseAllTabs(bool silent)
{
    bool areTabBarSignalsBlocked = false;
    if (silent)
    {
        areTabBarSignalsBlocked = tabBar->blockSignals(true);
    }

    bool closed = true;
    DAVA::uint32 count = GetTabCount();
    while (count)
    {
        if (!CloseTabInternal(GetCurrentTab(), silent))
        {
            closed = false;
            break;
        }
        count--;
    }

    if (silent)
    {
        tabBar->blockSignals(areTabBarSignalsBlocked);
    }

    return closed;
}

MainTabBar::MainTabBar(QWidget* parent /* = 0 */)
    : QTabBar(parent)
{
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
}

void MainTabBar::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void MainTabBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        emit OnDrop(event->mimeData());
    }
}

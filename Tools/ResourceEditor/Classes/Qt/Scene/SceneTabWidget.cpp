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
#include "Engine/Qt/RenderWidget.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcut>
#include <QDebug>
#include <QTimer>
#include <QtGlobal>

SceneTabWidget::SceneTabWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);

    // create Qt controls and add them into layout
    //
    // tab bar
    tabBar = new MainTabBar(this);
    tabBar->setTabsClosable(true);
    tabBar->setMovable(true);
    tabBar->setUsesScrollButtons(true);
    tabBar->setExpanding(false);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tabBar);
    layout->setMargin(0);
    layout->setSpacing(1);
    setLayout(layout);

    setAcceptDrops(true);

    QObject::connect(tabBar, SIGNAL(OnDrop(const QMimeData*)), this, SLOT(TabBarDataDropped(const QMimeData*)));

    QObject::connect(SceneSignals::Instance(), SIGNAL(MouseOverSelection(SceneEditor2*, const SelectableGroup*)), this, SLOT(MouseOverSelectedEntities(SceneEditor2*, const SelectableGroup*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Saved(SceneEditor2*)), this, SLOT(SceneSaved(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Updated(SceneEditor2*)), this, SLOT(SceneUpdated(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(ModifyStatusChanged(SceneEditor2*, bool)), this, SLOT(SceneModifyStatusChanged(SceneEditor2*, bool)));

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
}

/*int SceneTabWidget::OpenTab(const DAVA::FilePath& scenePath)
{
    // TODO UVR
    HideScenePreview();


    if (!TestSceneCompatibility(scenePath))
    {
        return -1;
    }

}*/

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

void SceneTabWidget::MouseOverSelectedEntities(SceneEditor2* scene, const SelectableGroup* objects)
{
    static QCursor cursorMove(QPixmap(":/QtIcons/curcor_move.png"));
    static QCursor cursorRotate(QPixmap(":/QtIcons/curcor_rotate.png"));
    static QCursor cursorScale(QPixmap(":/QtIcons/curcor_scale.png"));

    // TODO UVR LATER
    /*DAVA::RenderWidget* renderWidget = GetRenderWidget();

    if ((GetCurrentScene() == scene) && (objects != nullptr))
    {
        switch (scene->modifSystem->GetTransformType())
        {
        case Selectable::TransformType::Translation:
            renderWidget->setCursor(cursorMove);
            break;
        case Selectable::TransformType::Rotation:
            renderWidget->setCursor(cursorRotate);
            break;
        case Selectable::TransformType::Scale:
            renderWidget->setCursor(cursorScale);
            break;
        case Selectable::TransformType::Disabled:
        default:
            renderWidget->unsetCursor();
            break;
        }
    }
    else
    {
        renderWidget->unsetCursor();
    }*/
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
    event->setDropAction(Qt::LinkAction);
    event->accept();

    const QMimeData* data = event->mimeData();

    if (curScene != nullptr)
    {
        foreach (const QUrl& url, data->urls())
        {
            QString path = url.toLocalFile();
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

void SceneTabWidget::dragMoveEvent(QDragMoveEvent* event)
{
    //QObject* object = GetRenderWidget();
    //if (object != nullptr)
    //{ //simulate catching events at RenderWidget. I guess we should inherit SceneTabWidget from RenderWidget::IClientDelegate
    //    object->event(event);
    //}
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

bool SceneTabWidget::CloseAllTabs(bool silent)
{
    // TODO UVR
    //bool areTabBarSignalsBlocked = false;
    //if (silent)
    //{
    //    areTabBarSignalsBlocked = tabBar->blockSignals(true);
    //}

    //bool closed = true;
    //DAVA::uint32 count = GetTabCount();
    //while (count)
    //{
    //    // TODO UVR LATER
    //    //if (!CloseTabInternal(GetCurrentTab(), silent))
    //    {
    //        closed = false;
    //        break;
    //    }
    //    count--;
    //}

    //if (silent)
    //{
    //    tabBar->blockSignals(areTabBarSignalsBlocked);
    //}

    //return closed;
    return false;
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

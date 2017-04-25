#include "mainwindow.h"
#include <Tools/Version.h>
#include "Classes/Qt/BeastDialog/BeastDialog.h"
#include "Classes/Qt/CubemapEditor/CubeMapTextureBrowser.h"
#include "Classes/Qt/CubemapEditor/CubemapUtils.h"
#include "Classes/Qt/DebugTools/VersionInfoWidget/VersionInfoWidget.h"
#include "Classes/Qt/ImageSplitterDialog/ImageSplitterDialog.h"
#include "Classes/Qt/Main/QtUtils.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"
#include "Classes/Qt/QualitySwitcher/QualitySwitcher.h"
#include "Classes/Qt/RunActionEventWidget/RunActionEventWidget.h"
#include "Classes/Qt/Scene/LandscapeThumbnails.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Scene/SceneHelper.h"
#include "Classes/Qt/Scene/System/EditorVegetationSystem.h"
#include "Classes/Qt/Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Classes/Qt/Scene/System/EditorVegetationSystem.h"
#include "Classes/Qt/Scene/Validation/SceneValidationDialog.h"
#include "Classes/Settings/SettingsManager.h"
#include "Classes/Qt/SpritesPacker/SpritesPackerModule.h"
#include "Classes/Qt/TextureBrowser/TextureBrowser.h"
#include "Classes/Qt/TextureBrowser/TextureCache.h"
#include "Classes/Qt/Tools/AddSwitchEntityDialog/AddSwitchEntityDialog.h"
#include "Classes/Qt/Tools/BaseAddEntityDialog/BaseAddEntityDialog.h"
#include "Classes/Qt/Tools/DeveloperTools/DeveloperTools.h"
#include "Classes/Qt/Tools/HangingObjectsHeight/HangingObjectsHeight.h"
#include "Classes/Qt/Tools/HeightDeltaTool/HeightDeltaTool.h"
#include "Classes/Qt/Tools/PathDescriptor/PathDescriptor.h"
#include "Classes/Qt/Tools/QtLabelWithActions/QtLabelWithActions.h"
#include "Classes/Qt/Tools/ToolButtonWithWidget/ToolButtonWithWidget.h"
#include "Classes/Qt/Tools/ExportSceneDialog/ExportSceneDialog.h"
#include "Classes/Qt/Tools/LoggerOutput/ErrorDialogOutput.h"
#include "Classes/Qt/DockLandscapeEditorControls/LandscapeEditorShortcutManager.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Application/REGlobalOperationsData.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Selection/Selection.h"
#include "Classes/Selection/SelectionData.h"

#ifdef __DAVAENGINE_SPEEDTREE__
#include "SpeedTreeImport/SpeedTreeImportDialog.h"
#endif

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"

#include "Utils/SceneSaver/SceneSaver.h"
#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/RECommandNotificationObject.h"
#include "Commands2/AddComponentCommand.h"
#include "Commands2/CustomColorsCommands2.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/HeightmapEditorCommands2.h"
#include "Commands2/RemoveComponentCommand.h"
#include "Commands2/TilemaskEditorCommands.h"
#include "Commands2/LandscapeToolsToggleCommand.h"
#include "Commands2/WayEditCommands.h"

#include "Beast/BeastRunner.h"

#include "SceneProcessing/SceneProcessor.h"

#include "Constants.h"
#include "StringConstants.h"

#include "Render/2D/Sprite.h"

#include <Tools/TextureCompression/TextureConverter.h>

#include <TArc/WindowSubSystem/Private/WaitDialog.h>

#include "QtTools/ConsoleWidget/LogWidget.h"
#include "QtTools/ConsoleWidget/LogModel.h"
#include "QtTools/ConsoleWidget/PointerSerializer.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/FileDialogs/FindFileDialog.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

#include "Engine/Engine.h"
#include "Engine/Qt/RenderWidget.h"
#include "Reflection/ReflectedType.h"

#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene/System/EditorVegetationSystem.h"
#include "Utils/StringFormat.h"

#include <QActionGroup>
#include <QColorDialog>
#include <QDesktopServices>
#include <QKeySequence>
#include <QMessageBox>
#include <QMetaObject>
#include <QMetaType>
#include <QShortcut>
#include <QList>

#define CHECK_GLOBAL_OPERATIONS(retValue) \
    if (globalOperations == nullptr) \
    {\
        DAVA::Logger::Error("GlobalOperationsProxy call after MainWindow was destroyed"); \
        return (retValue);\
    }

namespace MainWindowDetails
{
class GlobalOperationsProxy : public GlobalOperations
{
public:
    GlobalOperationsProxy(GlobalOperations* globalOperations_)
        : globalOperations(globalOperations_)
    {
        globalOperations->waitDialogClosed.Connect(&waitDialogClosed, &DAVA::Signal<>::Emit);
    }

    void Reset()
    {
        globalOperations = nullptr;
    }

    void CallAction(ID id, DAVA::Any&& args) override
    {
        CHECK_GLOBAL_OPERATIONS(void());
        globalOperations->CallAction(id, std::move(args));
    }

    QWidget* GetGlobalParentWidget() const override
    {
        CHECK_GLOBAL_OPERATIONS(nullptr);
        return globalOperations->GetGlobalParentWidget();
    }

    void ShowWaitDialog(const DAVA::String& tittle, const DAVA::String& message, DAVA::uint32 min = 0, DAVA::uint32 max = 100) override
    {
        CHECK_GLOBAL_OPERATIONS(void());
        globalOperations->ShowWaitDialog(tittle, message, min, max);
    }

    bool IsWaitDialogVisible() const override
    {
        CHECK_GLOBAL_OPERATIONS(false);
        return globalOperations->IsWaitDialogVisible();
    }

    void HideWaitDialog() override
    {
        CHECK_GLOBAL_OPERATIONS(void());
        globalOperations->HideWaitDialog();
    }

    void ForEachScene(const DAVA::Function<void(SceneEditor2*)>& functor) override
    {
        CHECK_GLOBAL_OPERATIONS(void());
        globalOperations->ForEachScene(functor);
    }

private:
    GlobalOperations* globalOperations;
};

bool IsSavingAllowed(const QString& warningTitle)
{
    SceneData* data = REGlobal::GetActiveDataNode<SceneData>();
    DVASSERT(data);
    QString message;
    bool result = data->IsSavingAllowed(&message);
    if (result == false)
    {
        using namespace DAVA::TArc;

        ModalMessageParams params;
        params.buttons = ModalMessageParams::Ok;
        params.message = message;
        params.title = warningTitle;
        REGlobal::ShowModalMessage(params);
    }

    return result;
}

DAVA::RefPtr<SceneEditor2> GetCurrentScene()
{
    SceneData* data = REGlobal::GetActiveDataNode<SceneData>();
    DAVA::RefPtr<SceneEditor2> result;
    if (data)
    {
        result = data->GetScene();
    }
    return result;
}
}

QtMainWindow::QtMainWindow(DAVA::TArc::UI* tarcUI_, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , beastWaitDialog(nullptr)
    , globalInvalidate(false)
    , modificationWidget(nullptr)
    , addSwitchEntityDialog(nullptr)
    , hangingObjectsWidget(nullptr)
    , developerTools(new DeveloperTools(this))
#if defined(__DAVAENGINE_MACOS__)
    , shortcutChecker(this)
#endif
    , tarcUI(tarcUI_)
{
    projectDataWrapper = REGlobal::CreateDataWrapper(DAVA::ReflectedTypeDB::Get<ProjectManagerData>());
    projectDataWrapper.SetListener(this);

    ActiveSceneHolder::Init();
    globalOperations.reset(new MainWindowDetails::GlobalOperationsProxy(this));

    DAVA::TArc::DataContext* globalContext = REGlobal::GetGlobalContext();
    std::unique_ptr<REGlobalOperationsData> globalData = std::make_unique<REGlobalOperationsData>();
    globalData->SetGlobalOperations(globalOperations);
    globalContext->CreateData(std::move(globalData));

    errorLoggerOutput = new ErrorDialogOutput(globalOperations);
    DAVA::Logger::AddCustomOutput(errorLoggerOutput);

    tarcUI->lastWaitDialogWasClosed.Connect(&waitDialogClosed, &DAVA::Signal<>::Emit);

    new LandscapeEditorShortcutManager(this);
    PathDescriptor::InitializePathDescriptors();

    ui->setupUi(this);
    setObjectName("ResourceEditor"); //we need to support old names to save settings

    SetupWidget();
    SetupTitle(DAVA::String());

    qApp->installEventFilter(this);

    SetupDocks();
    SetupMainMenu();
    SetupThemeActions();
    SetupToolBars();
    SetupActions();

    // create tool windows
    new TextureBrowser(this);
    new MaterialEditor(this);

    beastWaitDialog = new QtWaitDialog(this);

    connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &QtMainWindow::SceneCommandExecuted);
    connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &QtMainWindow::SceneActivated);
    connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &QtMainWindow::SceneDeactivated);
    connect(SceneSignals::Instance(), &SceneSignals::EditorLightEnabled, this, &QtMainWindow::EditorLightEnabled);

    selectionWrapper = REGlobal::CreateDataWrapper(DAVA::ReflectedTypeDB::Get<SelectionData>());
    selectionWrapper.SetListener(this);

    LoadMaterialLightViewMode();

    EnableGlobalTimeout(globalInvalidate);

    EnableProjectActions(false);
    EnableSceneActions(false);

    SynchronizeStateWithUI();
}

QtMainWindow::~QtMainWindow()
{
    errorLoggerOutput->Disable();
    errorLoggerOutput = nullptr; // will be deleted by DAVA::Logger;

    LogWidget* logWidget = qobject_cast<LogWidget*>(dockConsole->widget());
    QByteArray dataToSave = logWidget->Serialize();

    DAVA::VariantType var(reinterpret_cast<const DAVA::uint8*>(dataToSave.data()), dataToSave.size());
    SettingsManager::Instance()->SetValue(Settings::Internal_LogWidget, var);

    DAVA::SafeDelete(addSwitchEntityDialog);

    TextureBrowser::Instance()->Release();
    MaterialEditor::Instance()->Release();

    LandscapeEditorShortcutManager::Instance()->Release();

    std::static_pointer_cast<MainWindowDetails::GlobalOperationsProxy>(globalOperations)->Reset();
    globalOperations.reset();
    ActiveSceneHolder::Deinit();
}

void QtMainWindow::OnRenderingInitialized()
{
    ui->landscapeEditorControlsPlaceholder->OnOpenGLInitialized();
    DAVA::RenderWidget* renderWidget = DAVA::PlatformApi::Qt::GetRenderWidget();
    renderWidget->resized.Connect(ui->statusBar, &StatusBar::OnSceneGeometryChaged);
}

void QtMainWindow::AfterInjectInit()
{
    SetupStatusBar();
}

QString GetSaveFolderForEmitters()
{
    const DAVA::FilePath defaultPath = SettingsManager::GetValue(Settings::Internal_ParticleLastEmitterDir).AsFilePath();
    QString particlesPath;
    if (defaultPath.IsEmpty())
    {
        ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
        DVASSERT(data != nullptr);
        particlesPath = QString::fromStdString(data->GetParticlesConfigPath().GetAbsolutePathname());
    }
    else
    {
        particlesPath = QString::fromStdString(defaultPath.GetAbsolutePathname());
    }

    return particlesPath;
}

void QtMainWindow::SetupWidget()
{
    ui->sceneTree->Init(globalOperations);
    ui->scrollAreaWidgetContents->Init(globalOperations);
}

void QtMainWindow::WaitStart(const QString& title, const QString& message, int min, int max)
{
    DVASSERT(waitDialog == nullptr);

    DAVA::TArc::WaitDialogParams params;
    params.message = message;
    params.min = min;
    params.max = max;
    params.needProgressBar = min != max;
    waitDialog = tarcUI->ShowWaitDialog(DAVA::TArc::mainWindowKey, params);
}

void QtMainWindow::WaitSetMessage(const QString& messsage)
{
    DVASSERT(waitDialog != nullptr);
    waitDialog->SetMessage(messsage);
}

void QtMainWindow::WaitSetValue(int value)
{
    DVASSERT(waitDialog != nullptr);
    waitDialog->SetProgressValue(value);
}

bool QtMainWindow::IsWaitDialogOnScreen() const
{
    return tarcUI->HasActiveWaitDalogues() || (beastWaitDialog != nullptr && beastWaitDialog->isVisible());
}

void QtMainWindow::WaitStop()
{
    waitDialog.reset();
}

bool QtMainWindow::eventFilter(QObject* obj, QEvent* event)
{
    QEvent::Type eventType = event->type();

#if defined(__DAVAENGINE_MACOS__)
    if (QEvent::ShortcutOverride == eventType && shortcutChecker.TryCallShortcut(static_cast<QKeyEvent*>(event)))
    {
        return true;
    }
#endif
    return QMainWindow::eventFilter(obj, event);
}

void QtMainWindow::SetupTitle(const DAVA::String& projectPath)
{
    DAVA::String title = DAVA::Format("DAVA Framework - ResourceEditor | %s.%s [%u bit]", DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION,
                                      static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));

    if (!projectPath.empty())
    {
        title += " | Project - ";
        title += projectPath;
    }

    setWindowTitle(QString::fromStdString(title));
}

void QtMainWindow::SetupMainMenu()
{
    ui->Dock->addAction(ui->dockSceneInfo->toggleViewAction());
    ui->Dock->addAction(ui->dockProperties->toggleViewAction());
    ui->Dock->addAction(ui->dockParticleEditor->toggleViewAction());
    ui->Dock->addAction(ui->dockParticleEditorTimeLine->toggleViewAction());
    ui->Dock->addAction(ui->dockSceneTree->toggleViewAction());
    ui->Dock->addAction(ui->dockLODEditor->toggleViewAction());
    ui->Dock->addAction(ui->dockLandscapeEditorControls->toggleViewAction());

    ui->Dock->addAction(dockActionEvent->toggleViewAction());
    ui->Dock->addAction(dockConsole->toggleViewAction());
}

void QtMainWindow::SetupThemeActions()
{
    QMenu* themesMenu = ui->menuTheme;
    QActionGroup* actionGroup = new QActionGroup(this);
    for (const QString& theme : Themes::ThemesNames())
    {
        QAction* action = new QAction(theme, themesMenu);
        actionGroup->addAction(action);
        action->setCheckable(true);
        if (theme == Themes::GetCurrentThemeStr())
        {
            action->setChecked(true);
        }
        themesMenu->addAction(action);
    }
    connect(actionGroup, &QActionGroup::triggered, [](QAction* action)
            {
                if (action->isChecked())
                {
                    Themes::SetCurrentTheme(action->text());
                    SceneSignals::Instance()->ThemeChanged();
                }
            });
}

void QtMainWindow::SetupToolBars()
{
    QObject::connect(SceneSignals::Instance(), &SceneSignals::CanUndoStateChanged, ui->actionUndo, &QAction::setEnabled);
    connect(SceneSignals::Instance(), &SceneSignals::CanRedoStateChanged, ui->actionRedo, &QAction::setEnabled);
    connect(SceneSignals::Instance(), &SceneSignals::UndoTextChanged, this, &QtMainWindow::UpdateUndoActionText);
    connect(SceneSignals::Instance(), &SceneSignals::RedoTextChanged, this, &QtMainWindow::UpdateRedoActionText);
    QAction* actionMainToolBar = ui->mainToolBar->toggleViewAction();
    QAction* actionModifToolBar = ui->modificationToolBar->toggleViewAction();
    QAction* actionLandscapeToolbar = ui->landscapeToolBar->toggleViewAction();

    ui->Toolbars->addAction(actionMainToolBar);
    ui->Toolbars->addAction(actionModifToolBar);
    ui->Toolbars->addAction(actionLandscapeToolbar);
    ui->Toolbars->addAction(ui->sceneToolBar->toggleViewAction());
    ui->Toolbars->addAction(ui->testingToolBar->toggleViewAction());
    ui->Toolbars->addAction(ui->cameraToolBar->toggleViewAction());

    // modification widget
    modificationWidget = new ModificationWidget(nullptr);
    ui->modificationToolBar->insertWidget(ui->actionModifyReset, modificationWidget);
    connect(ui->actionModifySnapToLandscape, &QAction::triggered, modificationWidget, &ModificationWidget::OnSnapToLandscapeChanged);

    // adding menu for material light view mode
    {
        QToolButton* setLightViewMode = new QToolButton();
        setLightViewMode->setMenu(ui->menuLightView);
        setLightViewMode->setPopupMode(QToolButton::InstantPopup);
        setLightViewMode->setDefaultAction(ui->actionSetLightViewMode);
        ui->mainToolBar->addWidget(setLightViewMode);
        setLightViewMode->setToolButtonStyle(Qt::ToolButtonIconOnly);
        setLightViewMode->setAutoRaise(false);
    }

    //hanging objects
    {
        HangingObjectsHeight* hangingObjectsWidget = new HangingObjectsHeight(this);
        QObject::connect(hangingObjectsWidget, SIGNAL(HeightChanged(double)), this, SLOT(OnHangingObjectsHeight(double)));

        ToolButtonWithWidget* hangingBtn = new ToolButtonWithWidget();
        hangingBtn->setDefaultAction(ui->actionHangingObjects);
        hangingBtn->SetWidget(hangingObjectsWidget);
        hangingBtn->setMaximumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_ICON);
        hangingBtn->setMinimumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_ICON);
        ui->testingToolBar->addWidget(hangingBtn);
        hangingBtn->setAutoRaise(false);
    }

    // outline by object type
    {
        objectTypesWidget = new QComboBox();
        //objectTypesWidget->setMaximumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);
        //objectTypesWidget->setMinimumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);

        const QList<QAction*> actions = ui->menuObjectTypes->actions();
        QActionGroup* group = new QActionGroup(ui->menuObjectTypes);

        auto endIt = actions.end();
        for (auto it = actions.begin(); it != endIt; ++it)
        {
            if ((*it)->isSeparator())
                continue;

            objectTypesWidget->addItem((*it)->icon(), (*it)->text());
            group->addAction(*it);
        }

        objectTypesWidget->setCurrentIndex(ResourceEditor::ESOT_NONE + 1);
        QObject::connect(objectTypesWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(OnObjectsTypeChanged(int)));

        ui->sceneToolBar->addSeparator();
        ui->sceneToolBar->addWidget(objectTypesWidget);
    }
}

void QtMainWindow::SetupStatusBar()
{
    QObject::connect(SceneSignals::Instance(), &SceneSignals::Activated, ui->statusBar, &StatusBar::SceneActivated);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::Deactivated, ui->statusBar, &StatusBar::SceneDeactivated);

    QObject::connect(this, &QtMainWindow::GlobalInvalidateTimeout, ui->statusBar, &StatusBar::UpdateByTimer);

    DAVA::TArc::InsertionParams insertParams;
    insertParams.method = DAVA::TArc::InsertionParams::eInsertionMethod::BeforeItem;
    DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateStatusbarPoint(true, 0, insertParams));

    tarcUI->AddAction(DAVA::TArc::mainWindowKey, placementInfo, ui->actionShowEditorGizmo);
    tarcUI->AddAction(DAVA::TArc::mainWindowKey, placementInfo, ui->actionLightmapCanvas);
    tarcUI->AddAction(DAVA::TArc::mainWindowKey, placementInfo, ui->actionShowStaticOcclusion);
    tarcUI->AddAction(DAVA::TArc::mainWindowKey, placementInfo, ui->actionEnableVisibilitySystem);
    tarcUI->AddAction(DAVA::TArc::mainWindowKey, placementInfo, ui->actionEnableDisableShadows);
    tarcUI->AddAction(DAVA::TArc::mainWindowKey, placementInfo, ui->actionEnableSounds);
}

void QtMainWindow::SetupDocks()
{
    QObject::connect(ui->sceneTreeFilterClear, SIGNAL(pressed()), ui->sceneTreeFilterEdit, SLOT(clear()));
    QObject::connect(ui->sceneTreeFilterEdit, SIGNAL(textChanged(const QString&)), ui->sceneTree, SLOT(SetFilter(const QString&)));

    QObject::connect(this, SIGNAL(GlobalInvalidateTimeout()), ui->sceneInfo, SLOT(UpdateInfoByTimer()));

    // Run Action Event dock
    {
        dockActionEvent = new QDockWidget("Run Action Event", this);
        dockActionEvent->setWidget(new RunActionEventWidget());
        dockActionEvent->setObjectName(QString("dock_%1").arg(dockActionEvent->widget()->objectName()));
        addDockWidget(Qt::RightDockWidgetArea, dockActionEvent);
    }
    // Console dock
    {
        LogWidget* logWidget = new LogWidget();
        logWidget->SetConvertFunction(&PointerSerializer::CleanUpString);

        LoggerOutputObject* loggerOutput = new LoggerOutputObject(); //will be removed by DAVA::Logger
        connect(loggerOutput, &LoggerOutputObject::OutputReady, logWidget, &LogWidget::AddMessage, Qt::DirectConnection);

        connect(logWidget, &LogWidget::ItemClicked, this, &QtMainWindow::OnConsoleItemClicked);
        const auto var = SettingsManager::Instance()->GetValue(Settings::Internal_LogWidget);

        const QByteArray arr(reinterpret_cast<const char*>(var.AsByteArray()), var.AsByteArraySize());
        logWidget->Deserialize(arr);
        dockConsole = new QDockWidget(logWidget->windowTitle(), this);
        dockConsole->setWidget(logWidget);
        dockConsole->setObjectName(QString("dock_%1").arg(dockConsole->widget()->objectName()));
        addDockWidget(Qt::RightDockWidgetArea, dockConsole);
    }

    ui->dockProperties->Init(ui.get(), globalOperations);
}

void QtMainWindow::SetupActions()
{
// import
#ifdef __DAVAENGINE_SPEEDTREE__
    QObject::connect(ui->actionImportSpeedTreeXML, &QAction::triggered, this, &QtMainWindow::OnImportSpeedTreeXML);
#endif //__DAVAENGINE_SPEEDTREE__

    QObject::connect(ui->actionAlbedo, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));
    QObject::connect(ui->actionAmbient, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));
    QObject::connect(ui->actionDiffuse, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));
    QObject::connect(ui->actionSpecular, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));

    bool gizmoEnabled = SettingsManager::GetValue(Settings::Internal_GizmoEnabled).AsBool();
    OnEditorGizmoToggle(gizmoEnabled);
    QObject::connect(ui->actionShowEditorGizmo, SIGNAL(toggled(bool)), this, SLOT(OnEditorGizmoToggle(bool)));

    QObject::connect(ui->actionLightmapCanvas, SIGNAL(toggled(bool)), this, SLOT(OnViewLightmapCanvas(bool)));
    QObject::connect(ui->actionShowStaticOcclusion, SIGNAL(toggled(bool)), this, SLOT(OnShowStaticOcclusionToggle(bool)));
    QObject::connect(ui->actionEnableVisibilitySystem, SIGNAL(triggered(bool)), this, SLOT(OnEnableVisibilitySystemToggle(bool)));

    QObject::connect(ui->actionRefreshVisibilitySystem, SIGNAL(triggered()), this, SLOT(OnRefreshVisibilitySystem()));
    QObject::connect(ui->actionFixCurrentFrame, SIGNAL(triggered()), this, SLOT(OnFixVisibilityFrame()));
    QObject::connect(ui->actionReleaseCurrentFrame, SIGNAL(triggered()), this, SLOT(OnReleaseVisibilityFrame()));

    QObject::connect(ui->actionEnableDisableShadows, &QAction::toggled, this, &QtMainWindow::OnEnableDisableShadows);

    bool toEnableSounds = SettingsManager::GetValue(Settings::Internal_EnableSounds).AsBool();
    EnableSounds(toEnableSounds);
    QObject::connect(ui->actionEnableSounds, &QAction::toggled, this, &QtMainWindow::EnableSounds);

    // scene undo/redo
    QObject::connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(OnUndo()));
    QObject::connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(OnRedo()));

    // quality
    QObject::connect(ui->actionCustomQuality, SIGNAL(triggered()), this, SLOT(OnCustomQuality()));

    // scene modifications
    QObject::connect(ui->actionModifySelect, SIGNAL(triggered()), this, SLOT(OnSelectMode()));
    QObject::connect(ui->actionModifyMove, SIGNAL(triggered()), this, SLOT(OnMoveMode()));
    QObject::connect(ui->actionModifyRotate, SIGNAL(triggered()), this, SLOT(OnRotateMode()));
    QObject::connect(ui->actionModifyScale, SIGNAL(triggered()), this, SLOT(OnScaleMode()));
    QObject::connect(ui->actionPivotCenter, SIGNAL(triggered()), this, SLOT(OnPivotCenterMode()));
    QObject::connect(ui->actionPivotCommon, SIGNAL(triggered()), this, SLOT(OnPivotCommonMode()));
    QObject::connect(ui->actionManualModifMode, SIGNAL(triggered()), this, SLOT(OnManualModifMode()));
    QObject::connect(ui->actionModifyPlaceOnLandscape, SIGNAL(triggered()), this, SLOT(OnPlaceOnLandscape()));
    QObject::connect(ui->actionModifySnapToLandscape, SIGNAL(triggered()), this, SLOT(OnSnapToLandscape()));
    QObject::connect(ui->actionModifyReset, SIGNAL(triggered()), this, SLOT(OnResetTransform()));
    QObject::connect(ui->actionLockTransform, SIGNAL(triggered()), this, SLOT(OnLockTransform()));
    QObject::connect(ui->actionUnlockTransform, SIGNAL(triggered()), this, SLOT(OnUnlockTransform()));
    QObject::connect(ui->actionCenterPivotPoint, SIGNAL(triggered()), this, SLOT(OnCenterPivotPoint()));
    QObject::connect(ui->actionZeroPivotPoint, SIGNAL(triggered()), this, SLOT(OnZeroPivotPoint()));

    // tools
    QObject::connect(ui->actionMaterialEditor, SIGNAL(triggered()), this, SLOT(OnMaterialEditor()));
    QObject::connect(ui->actionTextureConverter, SIGNAL(triggered()), this, SLOT(OnTextureBrowser()));
    QObject::connect(ui->actionEnableCameraLight, SIGNAL(triggered()), this, SLOT(OnSceneLightMode()));
    QObject::connect(ui->actionCubemapEditor, SIGNAL(triggered()), this, SLOT(OnCubemapEditor()));
    QObject::connect(ui->actionImageSplitter, SIGNAL(triggered()), this, SLOT(OnImageSplitter()));

    QObject::connect(ui->actionForceFirstLODonLandscape, SIGNAL(triggered(bool)), this, SLOT(OnForceFirstLod(bool)));
    QObject::connect(ui->actionShowNotPassableLandscape, SIGNAL(triggered()), this, SLOT(OnNotPassableTerrain()));
    QObject::connect(ui->actionCustomColorsEditor, SIGNAL(triggered()), this, SLOT(OnCustomColorsEditor()));
    QObject::connect(ui->actionHeightMapEditor, SIGNAL(triggered()), this, SLOT(OnHeightmapEditor()));
    QObject::connect(ui->actionTileMapEditor, SIGNAL(triggered()), this, SLOT(OnTilemaskEditor()));
    QObject::connect(ui->actionRulerTool, SIGNAL(triggered()), this, SLOT(OnRulerTool()));
    QObject::connect(ui->actionWayEditor, SIGNAL(triggered()), this, SLOT(OnWayEditor()));

    QObject::connect(ui->actionLight, SIGNAL(triggered()), this, SLOT(OnLightDialog()));
    QObject::connect(ui->actionCamera, SIGNAL(triggered()), this, SLOT(OnCameraDialog()));
    QObject::connect(ui->actionAddEmptyEntity, SIGNAL(triggered()), this, SLOT(OnEmptyEntity()));
    QObject::connect(ui->actionUserNode, SIGNAL(triggered()), this, SLOT(OnUserNodeDialog()));
    QObject::connect(ui->actionSwitchNode, SIGNAL(triggered()), this, SLOT(OnSwitchEntityDialog()));
    QObject::connect(ui->actionParticleEffectNode, SIGNAL(triggered()), this, SLOT(OnParticleEffectDialog()));
    QObject::connect(ui->actionEditor_2D_Camera, SIGNAL(triggered()), this, SLOT(On2DCameraDialog()));
    QObject::connect(ui->actionEditor_Sprite, SIGNAL(triggered()), this, SLOT(On2DSpriteDialog()));
    QObject::connect(ui->actionAddNewEntity, SIGNAL(triggered()), this, SLOT(OnAddEntityFromSceneTree()));
    QObject::connect(ui->actionRemoveEntity, SIGNAL(triggered()), this, SLOT(RemoveSelection()));
    QObject::connect(ui->actionExpandSceneTree, SIGNAL(triggered()), ui->sceneTree, SLOT(expandAll()));
    QObject::connect(ui->actionCollapseSceneTree, SIGNAL(triggered()), ui->sceneTree, SLOT(CollapseAll()));
    QObject::connect(ui->actionAddLandscape, SIGNAL(triggered()), this, SLOT(OnAddLandscape()));
    QObject::connect(ui->actionAddWind, SIGNAL(triggered()), this, SLOT(OnAddWindEntity()));
    QObject::connect(ui->actionAddVegetation, SIGNAL(triggered()), this, SLOT(OnAddVegetation()));
    QObject::connect(ui->actionAddPath, SIGNAL(triggered()), this, SLOT(OnAddPathEntity()));

    QObject::connect(ui->actionSaveHeightmapToPNG, SIGNAL(triggered()), this, SLOT(OnSaveHeightmapToImage()));
    QObject::connect(ui->actionSaveTiledTexture, SIGNAL(triggered()), this, SLOT(OnSaveTiledTexture()));

    QObject::connect(ui->actionConvertModifiedTextures, SIGNAL(triggered()), this, SLOT(OnConvertModifiedTextures()));
    
#if defined(__DAVAENGINE_BEAST__)
    QObject::connect(ui->actionBeastAndSave, SIGNAL(triggered()), this, SLOT(OnBeastAndSave()));
#else
//ui->menuScene->removeAction(ui->menuBeast->menuAction());
#endif //#if defined(__DAVAENGINE_BEAST__)

    QObject::connect(ui->actionBuildStaticOcclusion, SIGNAL(triggered()), this, SLOT(OnBuildStaticOcclusion()));
    QObject::connect(ui->actionInvalidateStaticOcclusion, SIGNAL(triggered()), this, SLOT(OnInavalidateStaticOcclusion()));

    connect(ui->actionHeightmap_Delta_Tool, SIGNAL(triggered()), this, SLOT(OnGenerateHeightDelta()));

    //Help
    QObject::connect(ui->actionHelp, SIGNAL(triggered()), this, SLOT(OnOpenHelp()));

    //Landscape editors toggled
    QObject::connect(SceneSignals::Instance(), SIGNAL(LandscapeEditorToggled(SceneEditor2*)),
                     this, SLOT(OnLandscapeEditorToggled(SceneEditor2*)));

    QObject::connect(SceneSignals::Instance(), SIGNAL(SnapToLandscapeChanged(SceneEditor2*, bool)),
                     this, SLOT(OnSnapToLandscapeChanged(SceneEditor2*, bool)));

    // Debug functions
    QObject::connect(ui->actionGridCopy, SIGNAL(triggered()), developerTools, SLOT(OnDebugFunctionsGridCopy()));
    {
#ifdef USER_VERSIONING_DEBUG_FEATURES
        QAction* act = ui->menuDebug_Functions->addAction("Edit version tags");
        connect(act, SIGNAL(triggered()), SLOT(DebugVersionInfo()));
#endif
    }

    connect(ui->actionImageSplitterForNormals, &QAction::triggered, developerTools, &DeveloperTools::OnImageSplitterNormals);
    connect(ui->actionReplaceTextureMipmap, &QAction::triggered, developerTools, &DeveloperTools::OnReplaceTextureMipmap);
    connect(ui->actionToggleUseInstancing, &QAction::triggered, developerTools, &DeveloperTools::OnToggleLandscapeInstancing);

    connect(ui->actionDumpTextures, &QAction::triggered, [] {
        DAVA::Texture::DumpTextures();
    });
    connect(ui->actionDumpSprites, &QAction::triggered, [] {
        DAVA::Sprite::DumpSprites();
    });

    connect(ui->actionCreateTestSkinnedObject, SIGNAL(triggered()), developerTools, SLOT(OnDebugCreateTestSkinnedObject()));

    ui->actionObjectTypesOff->setData(ResourceEditor::ESOT_NONE);
    ui->actionNoObject->setData(ResourceEditor::ESOT_NO_COLISION);
    ui->actionTree->setData(ResourceEditor::ESOT_TREE);
    ui->actionBush->setData(ResourceEditor::ESOT_BUSH);
    ui->actionFragileProj->setData(ResourceEditor::ESOT_FRAGILE_PROJ);
    ui->actionFragileProjInv->setData(ResourceEditor::ESOT_FRAGILE_PROJ_INV);
    ui->actionFalling->setData(ResourceEditor::ESOT_FALLING);
    ui->actionBuilding->setData(ResourceEditor::ESOT_BUILDING);
    ui->actionInvisibleWall->setData(ResourceEditor::ESOT_INVISIBLE_WALL);
    ui->actionSpeedTree->setData(ResourceEditor::ESOT_SPEED_TREE);

    QObject::connect(ui->menuObjectTypes, SIGNAL(triggered(QAction*)), this, SLOT(OnObjectsTypeChanged(QAction*)));
    QObject::connect(ui->actionHangingObjects, SIGNAL(triggered()), this, SLOT(OnHangingObjects()));
    QObject::connect(ui->actionSwitchesWithDifferentLODs, SIGNAL(triggered(bool)), this, SLOT(OnSwitchWithDifferentLODs(bool)));

    QObject::connect(ui->actionBatchProcess, SIGNAL(triggered(bool)), this, SLOT(OnBatchProcessScene()));

    QObject::connect(ui->actionSnapCameraToLandscape, SIGNAL(triggered(bool)), this, SLOT(OnSnapCameraToLandscape(bool)));

    QObject::connect(ui->actionValidateScene, SIGNAL(triggered()), this, SLOT(OnValidateScene()));
}

// ###################################################################################################
// Scene signals
// ###################################################################################################

void QtMainWindow::SceneActivated(SceneEditor2* scene)
{
    scene->ActivateCommandStack();
    EnableSceneActions(true);

    LoadViewState(scene);
    LoadModificationState(scene);
    LoadEditorLightState(scene);
    LoadLandscapeEditorState(scene);
    LoadObjectTypes(scene);
    LoadHangingObjects(scene);

    OnMaterialLightViewChanged(true);
    OnViewLightmapCanvas(true);

    UpdateModificationActionsState();

    ui->actionSwitchesWithDifferentLODs->setChecked(false);
    ui->actionSnapCameraToLandscape->setChecked(false);
    if (nullptr != scene)
    {
        scene->SetHUDVisible(ui->actionShowEditorGizmo->isChecked());
        if (scene->debugDrawSystem)
            ui->actionSwitchesWithDifferentLODs->setChecked(scene->debugDrawSystem->SwithcesWithDifferentLODsModeEnabled());

        if (scene->cameraSystem)
            ui->actionSnapCameraToLandscape->setChecked(scene->cameraSystem->IsEditorCameraSnappedToLandscape());
    }
    ui->actionUndo->setEnabled(scene->CanUndo());
    ui->actionRedo->setEnabled(scene->CanRedo());

    UpdateUndoActionText(scene->GetUndoText());
    UpdateRedoActionText(scene->GetRedoText());
}

void QtMainWindow::SceneDeactivated(SceneEditor2* scene)
{
    // block some actions, when there is no scene
    EnableSceneActions(false);
}

void QtMainWindow::EnableProjectActions(bool enable)
{
    ui->actionCubemapEditor->setEnabled(enable);
    ui->actionImageSplitter->setEnabled(enable);
}

void QtMainWindow::EnableSceneActions(bool enable)
{
    ui->actionUndo->setEnabled(enable);
    ui->actionRedo->setEnabled(enable);

    ui->dockLODEditor->setEnabled(enable);
    ui->dockProperties->setEnabled(enable);
    ui->dockSceneTree->setEnabled(enable);
    ui->dockSceneInfo->setEnabled(enable);

    ui->actionModifySelect->setEnabled(enable);
    ui->actionModifyMove->setEnabled(enable);
    ui->actionModifyReset->setEnabled(enable);
    ui->actionModifyRotate->setEnabled(enable);
    ui->actionModifyScale->setEnabled(enable);
    ui->actionModifyPlaceOnLandscape->setEnabled(enable);
    ui->actionModifySnapToLandscape->setEnabled(enable);
    ui->actionConvertToShadow->setEnabled(enable);
    ui->actionPivotCenter->setEnabled(enable);
    ui->actionPivotCommon->setEnabled(enable);
    ui->actionCenterPivotPoint->setEnabled(enable);
    ui->actionZeroPivotPoint->setEnabled(enable);
    ui->actionManualModifMode->setEnabled(enable);

    if (modificationWidget)
        modificationWidget->setEnabled(enable);

    ui->actionTextureConverter->setEnabled(enable);
    ui->actionMaterialEditor->setEnabled(enable);
    ui->actionHeightMapEditor->setEnabled(enable);
    ui->actionTileMapEditor->setEnabled(enable);
    ui->actionShowNotPassableLandscape->setEnabled(enable);
    ui->actionRulerTool->setEnabled(enable);
    ui->actionVisibilityCheckTool->setEnabled(enable);
    ui->actionCustomColorsEditor->setEnabled(enable);
    ui->actionWayEditor->setEnabled(enable);
    ui->actionForceFirstLODonLandscape->setEnabled(enable);
    ui->actionEnableVisibilitySystem->setEnabled(enable);

    ui->actionEnableCameraLight->setEnabled(enable);

    ui->actionSetLightViewMode->setEnabled(enable);

    ui->actionSaveHeightmapToPNG->setEnabled(enable);
    ui->actionSaveTiledTexture->setEnabled(enable);

    ui->actionBeastAndSave->setEnabled(enable);

    ui->actionHangingObjects->setEnabled(enable);

    ui->Edit->setEnabled(enable);
    ui->menuCreateNode->setEnabled(enable);
    ui->Scene->setEnabled(enable);
    ui->menuLightView->setEnabled(enable);

    ui->sceneToolBar->setEnabled(enable);
    ui->actionConvertModifiedTextures->setEnabled(enable);

    ui->actionSwitchesWithDifferentLODs->setEnabled(enable);

    ui->actionSnapCameraToLandscape->setEnabled(enable);
    ui->actionHeightmap_Delta_Tool->setEnabled(enable);

    ui->actionValidateScene->setEnabled(enable);

    // Fix for menuBar rendering
    const auto isMenuBarEnabled = ui->menuBar->isEnabled();
    ui->menuBar->setEnabled(false);
    ui->menuBar->setEnabled(isMenuBarEnabled);
}

void QtMainWindow::UpdateModificationActionsState()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    bool isMultiple = (scene.Get() != nullptr) && (scene->modifSystem->GetTransformableSelection().GetSize() > 1);

    // modificationWidget determines inside, if values could be modified and enables/disables itself
    modificationWidget->ReloadValues();
    bool canModify = modificationWidget->isEnabled();

    ui->actionModifyReset->setEnabled(canModify);
    ui->actionCenterPivotPoint->setEnabled(canModify && !isMultiple);
    ui->actionZeroPivotPoint->setEnabled(canModify && !isMultiple);
}

void QtMainWindow::UpdateWayEditor(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandID(CMDID_ENABLE_WAYEDIT))
    {
        DVASSERT(commandNotification.MatchCommandID(CMDID_DISABLE_WAYEDIT) == false);
        SetActionCheckedSilently(ui->actionWayEditor, commandNotification.redo);
    }
    else if (commandNotification.MatchCommandID(CMDID_DISABLE_WAYEDIT))
    {
        DVASSERT(commandNotification.MatchCommandID(CMDID_ENABLE_WAYEDIT) == false);
        SetActionCheckedSilently(ui->actionWayEditor, !commandNotification.redo);
    }
}

void QtMainWindow::SceneCommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification)
{
    if (scene == MainWindowDetails::GetCurrentScene())
    {
        UpdateModificationActionsState();
        UpdateWayEditor(commandNotification);

        auto updateCameraState = [this, scene](const RECommand* command)
        {
            DAVA::Entity* entity = nullptr;
            if (command->GetID() == CMDID_COMPONENT_ADD)
            {
                const AddComponentCommand* addCommand = static_cast<const AddComponentCommand*>(command);
                entity = addCommand->GetEntity();
            }
            else if (command->GetID() == CMDID_COMPONENT_REMOVE)
            {
                const RemoveComponentCommand* removeCommand = static_cast<const RemoveComponentCommand*>(command);
                entity = removeCommand->GetEntity();
            }
            if (entity != nullptr && entity->GetName() == ResourceEditor::EDITOR_DEBUG_CAMERA)
            {
                SetActionCheckedSilently(ui->actionSnapCameraToLandscape, scene->cameraSystem->IsEditorCameraSnappedToLandscape());
                return true;
            }
            return false;
        };

        if (commandNotification.batch != nullptr)
        {
            for (DAVA::uint32 i = 0, count = commandNotification.batch->Size(); i < count; ++i)
            {
                if (updateCameraState(commandNotification.batch->GetCommand(i)))
                {
                    break;
                }
            }
        }
        else
        {
            updateCameraState(commandNotification.command);
        }
    }
}

// ###################################################################################################
// Mainwindow Qt actions
// ###################################################################################################
void QtMainWindow::OnImportSpeedTreeXML()
{
#ifdef __DAVAENGINE_SPEEDTREE__
    SpeedTreeImportDialog importDialog(globalOperations, this);
    importDialog.exec();
#endif //__DAVAENGINE_SPEEDTREE__
}

void QtMainWindow::OnUndo()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->Undo();
    }
}

void QtMainWindow::OnRedo()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->Redo();
    }
}

void QtMainWindow::OnEditorGizmoToggle(bool show)
{
    ui->actionShowEditorGizmo->setChecked(show);
    SettingsManager::Instance()->SetValue(Settings::Internal_GizmoEnabled, DAVA::VariantType(show));
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->SetHUDVisible(show);
    }
}

void QtMainWindow::OnViewLightmapCanvas(bool show)
{
    bool showCanvas = ui->actionLightmapCanvas->isChecked();
    if (showCanvas != SettingsManager::GetValue(Settings::Internal_MaterialsShowLightmapCanvas).AsBool())
    {
        SettingsManager::SetValue(Settings::Internal_MaterialsShowLightmapCanvas, DAVA::VariantType(showCanvas));
    }

    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->materialSystem->SetLightmapCanvasVisible(showCanvas);
    }
}

void QtMainWindow::OnShowStaticOcclusionToggle(bool show)
{
    DAVA::Renderer::GetOptions()->SetOption(DAVA::RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION, show);
}

void QtMainWindow::OnEnableVisibilitySystemToggle(bool enabled)
{
    SetVisibilityToolEnabledIfPossible(enabled);
}

void QtMainWindow::OnRefreshVisibilitySystem()
{
    MainWindowDetails::GetCurrentScene()->visibilityCheckSystem->Recalculate();
}

void QtMainWindow::OnFixVisibilityFrame()
{
    MainWindowDetails::GetCurrentScene()->visibilityCheckSystem->FixCurrentFrame();
}

void QtMainWindow::OnReleaseVisibilityFrame()
{
    MainWindowDetails::GetCurrentScene()->visibilityCheckSystem->ReleaseFixedFrame();
}

void QtMainWindow::OnEnableDisableShadows(bool enable)
{
    DAVA::Renderer::GetOptions()->SetOption(DAVA::RenderOptions::SHADOWVOLUME_DRAW, enable);
}

void QtMainWindow::EnableSounds(bool toEnable)
{
    ui->actionEnableSounds->setChecked(toEnable);

    if (toEnable != SettingsManager::GetValue(Settings::Internal_EnableSounds).AsBool())
    {
        SettingsManager::SetValue(Settings::Internal_EnableSounds, DAVA::VariantType(toEnable));
    }

    DAVA::SoundSystem::Instance()->Mute(!toEnable);
}

void QtMainWindow::OnSelectMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Disabled);
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnMoveMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Translation);
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnRotateMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Rotation);
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnScaleMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Scale);
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnPivotCenterMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->modifSystem->SetPivotPoint(Selectable::TransformPivot::ObjectCenter);
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnPivotCommonMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        scene->modifSystem->SetPivotPoint(Selectable::TransformPivot::CommonCenter);
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnManualModifMode()
{
    if (ui->actionManualModifMode->isChecked())
    {
        modificationWidget->SetPivotMode(ModificationWidget::PivotRelative);
    }
    else
    {
        modificationWidget->SetPivotMode(ModificationWidget::PivotAbsolute);
    }
}

void QtMainWindow::OnPlaceOnLandscape()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        DAVA::Entity* landscapeEntity = FindLandscapeEntity(scene.Get());
        if (landscapeEntity == nullptr || GetLandscape(landscapeEntity) == nullptr)
        {
            DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
            return;
        }

        const SelectableGroup& selection = Selection::GetSelection();
        scene->modifSystem->PlaceOnLandscape(selection);
    }
}

void QtMainWindow::OnSnapToLandscape()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        DAVA::Entity* landscapeEntity = FindLandscapeEntity(scene.Get());
        if (landscapeEntity == nullptr || GetLandscape(landscapeEntity) == nullptr)
        {
            DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
            ui->actionModifySnapToLandscape->setChecked(false);
            return;
        }

        scene->modifSystem->SetLandscapeSnap(ui->actionModifySnapToLandscape->isChecked());
        LoadModificationState(scene.Get());
    }
}

void QtMainWindow::OnResetTransform()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        const SelectableGroup& selection = Selection::GetSelection();
        scene->modifSystem->ResetTransform(selection);
    }
}

void QtMainWindow::OnLockTransform()
{
    LockTransform(MainWindowDetails::GetCurrentScene().Get());
    UpdateModificationActionsState();
}

void QtMainWindow::OnUnlockTransform()
{
    UnlockTransform(MainWindowDetails::GetCurrentScene().Get());
    UpdateModificationActionsState();
}

void QtMainWindow::OnCenterPivotPoint()
{
    DAVA::RefPtr<SceneEditor2> curScene = MainWindowDetails::GetCurrentScene();
    if (curScene.Get() != nullptr)
    {
        const SelectableGroup& selection = Selection::GetSelection();
        curScene->modifSystem->MovePivotCenter(selection);
    }
}

void QtMainWindow::OnZeroPivotPoint()
{
    DAVA::RefPtr<SceneEditor2> curScene = MainWindowDetails::GetCurrentScene();
    if (curScene.Get() != nullptr)
    {
        const SelectableGroup& selection = Selection::GetSelection();
        curScene->modifSystem->MovePivotZero(selection);
    }
}

void QtMainWindow::OnMaterialEditor(DAVA::NMaterial* material)
{
    MaterialEditor* editor = MaterialEditor::Instance();
    editor->show();
    if (material != nullptr)
    {
        editor->SelectMaterial(material);
    }
}

void QtMainWindow::OnTextureBrowser()
{
    TextureBrowser::Instance()->show();

    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    TextureBrowser::Instance()->sceneActivated(sceneEditor.Get());
}

void QtMainWindow::OnSceneLightMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() != nullptr)
    {
        if (ui->actionEnableCameraLight->isChecked())
        {
            scene->editorLightSystem->SetCameraLightEnabled(true);
        }
        else
        {
            scene->editorLightSystem->SetCameraLightEnabled(false);
        }

        LoadEditorLightState(scene.Get());
    }
}

void QtMainWindow::OnCubemapEditor()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();

    CubeMapTextureBrowser dlg(scene.Get(), dynamic_cast<QWidget*>(parent()));
    dlg.exec();
}

void QtMainWindow::OnImageSplitter()
{
    ImageSplitterDialog dlg(this);
    dlg.exec();
}

void QtMainWindow::OnSwitchEntityDialog()
{
    if (nullptr != addSwitchEntityDialog)
    {
        return;
    }
    addSwitchEntityDialog = new AddSwitchEntityDialog(this);
    addSwitchEntityDialog->show();
    connect(addSwitchEntityDialog, SIGNAL(finished(int)), this, SLOT(UnmodalDialogFinished(int)));
}

void QtMainWindow::UnmodalDialogFinished(int)
{
    QObject* sender = QObject::sender();
    disconnect(sender, SIGNAL(finished(int)), this, SLOT(UnmodalDialogFinished(int)));
    if (sender == addSwitchEntityDialog)
    {
        addSwitchEntityDialog = nullptr;
    }
}

void QtMainWindow::OnAddLandscape()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> entityToProcess(new DAVA::Entity());
        entityToProcess->SetName(ResourceEditor::LANDSCAPE_NODE_NAME);
        entityToProcess->SetLocked(true);

        DAVA::ScopedPtr<DAVA::Landscape> newLandscape(new DAVA::Landscape());

        DAVA::RenderComponent* component = new DAVA::RenderComponent();
        component->SetRenderObject(newLandscape);
        entityToProcess->AddComponent(component);

        DAVA::AABBox3 bboxForLandscape;
        DAVA::float32 defaultLandscapeSize = 600.0f;
        DAVA::float32 defaultLandscapeHeight = 50.0f;

        bboxForLandscape.AddPoint(DAVA::Vector3(-defaultLandscapeSize / 2.f, -defaultLandscapeSize / 2.f, 0.f));
        bboxForLandscape.AddPoint(DAVA::Vector3(defaultLandscapeSize / 2.f, defaultLandscapeSize / 2.f, defaultLandscapeHeight));
        newLandscape->BuildLandscapeFromHeightmapImage("", bboxForLandscape);

        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(entityToProcess, sceneEditor.Get())));
    }
}

void QtMainWindow::OnAddVegetation()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::VegetationRenderObject> vro(new DAVA::VegetationRenderObject());
        DAVA::RenderComponent* rc = new DAVA::RenderComponent();
        rc->SetRenderObject(vro);

        DAVA::ScopedPtr<DAVA::Entity> vegetationNode(new DAVA::Entity());
        vegetationNode->AddComponent(rc);
        vegetationNode->SetName(ResourceEditor::VEGETATION_NODE_NAME);
        vegetationNode->SetLocked(true);

        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(vegetationNode, sceneEditor.Get())));
    }
}

void QtMainWindow::OnLightDialog()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        sceneNode->AddComponent(new DAVA::LightComponent(DAVA::ScopedPtr<DAVA::Light>(new DAVA::Light)));
        sceneNode->SetName(ResourceEditor::LIGHT_NODE_NAME);
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
    }
}

void QtMainWindow::OnCameraDialog()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        DAVA::ScopedPtr<DAVA::Camera> camera(new DAVA::Camera());

        camera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
        camera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 0.0f));
        camera->SetTarget(DAVA::Vector3(1.0f, 0.0f, 0.0f));
        camera->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        camera->SetAspect(1.0f);
        camera->RebuildCameraFromValues();

        sceneNode->AddComponent(new DAVA::CameraComponent(camera));
        sceneNode->AddComponent(new DAVA::WASDControllerComponent());
        sceneNode->AddComponent(new DAVA::RotationControllerComponent());

        sceneNode->SetName(ResourceEditor::CAMERA_NODE_NAME);

        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
    }
}

void QtMainWindow::OnUserNodeDialog()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        sceneNode->AddComponent(new DAVA::UserComponent());
        sceneNode->SetName(ResourceEditor::USER_NODE_NAME);
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
    }
}

void QtMainWindow::OnParticleEffectDialog()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        sceneNode->AddComponent(new DAVA::ParticleEffectComponent());
        sceneNode->AddComponent(new DAVA::LodComponent());
        sceneNode->SetName(ResourceEditor::PARTICLE_EFFECT_NODE_NAME);
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
    }
}

void QtMainWindow::On2DCameraDialog()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        DAVA::ScopedPtr<DAVA::Camera> camera(new DAVA::Camera());

        DAVA::float32 w = DAVA::UIControlSystem::Instance()->vcs->GetFullScreenVirtualRect().dx;
        DAVA::float32 h = DAVA::UIControlSystem::Instance()->vcs->GetFullScreenVirtualRect().dy;
        DAVA::float32 aspect = w / h;
        camera->SetupOrtho(w, aspect, 1, 1000);
        camera->SetPosition(DAVA::Vector3(0, 0, -10000));
        camera->SetZFar(10000);
        camera->SetTarget(DAVA::Vector3(0, 0, 0));
        camera->SetUp(DAVA::Vector3(0, -1, 0));
        camera->RebuildCameraFromValues();

        sceneNode->AddComponent(new DAVA::CameraComponent(camera));
        sceneNode->SetName("Camera 2D");
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
    }
}

void QtMainWindow::On2DSpriteDialog()
{
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    DAVA::FilePath projectPath = data->GetParticlesGfxPath();

    QString filePath = FileDialog::getOpenFileName(nullptr, QString("Open sprite"), QString::fromStdString(projectPath.GetAbsolutePathname()), QString("Sprite File (*.psd)"));
    if (filePath.isEmpty())
        return;

    filePath = filePath.replace("/DataSource/", "/Data/");
    filePath.remove(filePath.size() - 4, 4);

    DAVA::ScopedPtr<DAVA::Sprite> sprite(DAVA::Sprite::Create(filePath.toStdString()));
    if (!sprite)
        return;

    DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
    sceneNode->SetName(ResourceEditor::EDITOR_SPRITE);

    DAVA::ScopedPtr<DAVA::SpriteObject> spriteObject(new DAVA::SpriteObject(sprite, 0, DAVA::Vector2(1, 1), DAVA::Vector2(0.5f * sprite->GetWidth(), 0.5f * sprite->GetHeight())));
    spriteObject->AddFlag(DAVA::RenderObject::ALWAYS_CLIPPING_VISIBLE);
    sceneNode->AddComponent(new DAVA::RenderComponent(spriteObject));
    DAVA::Matrix4 m = DAVA::Matrix4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, -1, 0,
                                    0, 0, 0, 1);
    sceneNode->SetLocalTransform(m);
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneEditor)
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
    }
}

void QtMainWindow::OnAddEntityFromSceneTree()
{
    ui->menuAdd->exec(QCursor::pos());
}

void QtMainWindow::OnOpenHelp()
{
    DAVA::FilePath docsPath = ResourceEditor::DOCUMENTATION_PATH + "index.html";
    QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
    QDesktopServices::openUrl(QUrl(docsFile));
}

// ###################################################################################################
// Mainwindow load state functions
// ###################################################################################################

void QtMainWindow::LoadViewState(SceneEditor2* scene)
{
    if (nullptr != scene)
    {
        bool viewLMCanvas = SettingsManager::GetValue(Settings::Internal_MaterialsShowLightmapCanvas).AsBool();
        ui->actionLightmapCanvas->setChecked(viewLMCanvas);

        auto options = DAVA::Renderer::GetOptions();
        ui->actionEnableDisableShadows->setChecked(options->IsOptionEnabled(DAVA::RenderOptions::SHADOWVOLUME_DRAW));
    }
}

void QtMainWindow::LoadModificationState(SceneEditor2* scene)
{
    if (nullptr != scene)
    {
        ui->actionModifySelect->setChecked(false);
        ui->actionModifyMove->setChecked(false);
        ui->actionModifyRotate->setChecked(false);
        ui->actionModifyScale->setChecked(false);

        auto modifMode = scene->modifSystem->GetTransformType();
        modificationWidget->SetTransformType(modifMode);

        switch (modifMode)
        {
        case Selectable::TransformType::Disabled:
            ui->actionModifySelect->setChecked(true);
            break;
        case Selectable::TransformType::Translation:
            ui->actionModifyMove->setChecked(true);
            break;
        case Selectable::TransformType::Rotation:
            ui->actionModifyRotate->setChecked(true);
            break;
        case Selectable::TransformType::Scale:
            ui->actionModifyScale->setChecked(true);
            break;
        default:
            break;
        }

        // pivot point
        if (scene->modifSystem->GetPivotPoint() == Selectable::TransformPivot::ObjectCenter)
        {
            ui->actionPivotCenter->setChecked(true);
            ui->actionPivotCommon->setChecked(false);
        }
        else
        {
            ui->actionPivotCenter->setChecked(false);
            ui->actionPivotCommon->setChecked(true);
        }

        // landscape snap
        ui->actionModifySnapToLandscape->setChecked(scene->modifSystem->GetLandscapeSnap());

        // way editor
        ui->actionWayEditor->setChecked(scene->wayEditSystem->IsWayEditEnabled());

        UpdateModificationActionsState();
    }
}

void QtMainWindow::LoadEditorLightState(SceneEditor2* scene)
{
    if (nullptr != scene)
    {
        ui->actionEnableCameraLight->setChecked(scene->editorLightSystem->GetCameraLightEnabled());
    }
}

void QtMainWindow::LoadMaterialLightViewMode()
{
    int curViewMode = SettingsManager::GetValue(Settings::Internal_MaterialsLightViewMode).AsInt32();

    ui->actionAlbedo->setChecked((bool)(curViewMode & EditorMaterialSystem::LIGHTVIEW_ALBEDO));
    ui->actionAmbient->setChecked((bool)(curViewMode & EditorMaterialSystem::LIGHTVIEW_AMBIENT));
    ui->actionSpecular->setChecked((bool)(curViewMode & EditorMaterialSystem::LIGHTVIEW_SPECULAR));
    ui->actionDiffuse->setChecked((bool)(curViewMode & EditorMaterialSystem::LIGHTVIEW_DIFFUSE));
}

void QtMainWindow::LoadLandscapeEditorState(SceneEditor2* scene)
{
    OnLandscapeEditorToggled(scene);
}

void QtMainWindow::OnSaveHeightmapToImage()
{
    if (MainWindowDetails::IsSavingAllowed("Save heightmap to Image") == false)
    {
        return;
    }

    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();

    DAVA::Landscape* landscape = FindLandscape(scene.Get());
    QString titleString = "Saving is not allowed";

    if (!landscape)
    {
        QMessageBox::warning(this, titleString, "There is no landscape in scene!");
        return;
    }
    if (!landscape->GetHeightmap()->Size())
    {
        QMessageBox::warning(this, titleString, "There is no heightmap in landscape!");
        return;
    }

    DAVA::Heightmap* heightmap = landscape->GetHeightmap();
    DAVA::FilePath heightmapPath = landscape->GetHeightmapPathname();

    QString selectedPath = FileDialog::getSaveFileName(this, "Save heightmap as", heightmapPath.GetAbsolutePathname().c_str(),
                                                       PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);
    if (selectedPath.isEmpty())
        return;

    DAVA::FilePath requestedPngPath = DAVA::FilePath(selectedPath.toStdString());
    heightmap->SaveToImage(requestedPngPath);
}

void QtMainWindow::OnSaveTiledTexture()
{
    if (MainWindowDetails::IsSavingAllowed("Save tiled texture") == false)
    {
        return;
    }

    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    LandscapeEditorDrawSystem::eErrorType varifLandscapeError = scene->landscapeEditorDrawSystem->VerifyLandscape();
    if (varifLandscapeError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        DAVA::Logger::Error(LandscapeEditorDrawSystem::GetDescriptionByError(varifLandscapeError).c_str());
        return;
    }

    DAVA::Landscape* landscape = FindLandscape(scene.Get());
    if (nullptr != landscape)
    {
        LandscapeThumbnails::Create(landscape, MakeFunction(this, &QtMainWindow::OnTiledTextureRetreived));
    }
}

void QtMainWindow::OnTiledTextureRetreived(DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture)
{
    DAVA::FilePath pathToSave = landscape->GetMaterial()->GetEffectiveTexture(DAVA::Landscape::TEXTURE_COLOR)->GetPathname();
    if (pathToSave.IsEmpty())
    {
        ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
        DVASSERT(data != nullptr);
        QString selectedPath = FileDialog::getSaveFileName(this, "Save landscape texture as",
                                                           data->GetDataSource3DPath().GetAbsolutePathname().c_str(),
                                                           PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);

        if (selectedPath.isEmpty())
        {
            return;
        }

        pathToSave = DAVA::FilePath(selectedPath.toStdString());
    }
    else
    {
        pathToSave.ReplaceExtension(".thumbnail.png");
    }

    SaveTextureToFile(landscapeTexture, pathToSave);
}

void QtMainWindow::OnConvertModifiedTextures()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
    {
        return;
    }

    WaitStart("Conversion of modified textures.", "Checking for modified textures.", 0, 0);
    DAVA::Map<DAVA::Texture*, DAVA::Vector<DAVA::eGPUFamily>> textures;
    int filesToUpdate = SceneHelper::EnumerateModifiedTextures(scene.Get(), textures);

    if (filesToUpdate == 0)
    {
        WaitStop();
        return;
    }

    int convretedNumber = 0;
    waitDialog->SetRange(convretedNumber, filesToUpdate);
    WaitSetValue(convretedNumber);
    for (DAVA::Map<DAVA::Texture*, DAVA::Vector<DAVA::eGPUFamily>>::iterator it = textures.begin(); it != textures.end(); ++it)
    {
        DAVA::TextureDescriptor* descriptor = it->first->GetDescriptor();

        if (nullptr == descriptor)
        {
            continue;
        }

        DAVA::VariantType quality = SettingsManager::Instance()->GetValue(Settings::General_CompressionQuality);

        DAVA::Vector<DAVA::eGPUFamily> updatedGPUs = it->second;
        WaitSetMessage(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str());
        foreach (DAVA::eGPUFamily gpu, updatedGPUs)
        {
            DAVA::TextureConverter::ConvertTexture(*descriptor, gpu, true, static_cast<DAVA::TextureConverter::eConvertQuality>(quality.AsInt32()));

            DAVA::TexturesMap texturesMap = DAVA::Texture::GetTextureMap();
            DAVA::TexturesMap::iterator found = texturesMap.find(FILEPATH_MAP_KEY(descriptor->pathname));
            if (found != texturesMap.end())
            {
                DAVA::Texture* tex = found->second;
                tex->Reload();
            }

            WaitSetValue(++convretedNumber);
        }
    }

    WaitStop();
}

void QtMainWindow::OnGlobalInvalidateTimeout()
{
    emit GlobalInvalidateTimeout();
    if (globalInvalidate)
    {
        StartGlobalInvalidateTimer();
    }
}

void QtMainWindow::EnableGlobalTimeout(bool enable)
{
    if (globalInvalidate != enable)
    {
        globalInvalidate = enable;

        if (globalInvalidate)
        {
            StartGlobalInvalidateTimer();
        }
    }
}

void QtMainWindow::StartGlobalInvalidateTimer()
{
    QTimer::singleShot(GLOBAL_INVALIDATE_TIMER_DELTA, this, SLOT(OnGlobalInvalidateTimeout()));
}

void QtMainWindow::EditorLightEnabled(bool enabled)
{
    ui->actionEnableCameraLight->setChecked(enabled);
}

void QtMainWindow::OnBeastAndSave()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    if (scene->GetEnabledTools())
    {
        if (QMessageBox::Yes == QMessageBox::question(this, "Starting Beast", "Disable landscape editor and start beasting?", (QMessageBox::Yes | QMessageBox::No), QMessageBox::No))
        {
            scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);

            bool success = !scene->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
            if (!success)
            {
                DAVA::Logger::Error(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS.c_str());
                return;
            }
        }
        else
        {
            return;
        }
    }

    REGlobal::GetInvoker()->Invoke(REGlobal::SaveCurrentScene.ID);
    if (!scene->IsLoaded() || scene->IsChanged())
    {
        return;
    }

    BeastDialog dlg(this);
    dlg.SetScene(scene.Get());
    const bool run = dlg.Exec();
    if (!run)
        return;

    RunBeast(dlg.GetPath(), dlg.GetMode());
    scene->SetChanged();
    REGlobal::GetInvoker()->Invoke(REGlobal::SaveCurrentScene.ID);
    scene->ClearAllCommands();
}

void QtMainWindow::RunBeast(const QString& outputPath, BeastProxy::eBeastMode mode)
{
#if defined(__DAVAENGINE_BEAST__)

    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    const DAVA::FilePath path = outputPath.toStdString();

    BeastRunner beast(scene.Get(), scene->GetScenePath(), path, mode, beastWaitDialog);
    beast.RunUIMode();

    if (mode == BeastProxy::MODE_LIGHTMAPS)
    {
        // ReloadTextures should be delayed to give Qt some time for closing wait dialog before we will open new one for texture reloading.
        delayedExecutor.DelayedExecute([]() {
            REGlobal::GetInvoker()->Invoke(REGlobal::ReloadTexturesOperation.ID, Settings::GetGPUFormat());
        });
    }

#endif //#if defined (__DAVAENGINE_BEAST__)
}

void QtMainWindow::OnLandscapeEditorToggled(SceneEditor2* scene)
{
    if (scene != MainWindowDetails::GetCurrentScene())
    {
        return;
    }

    ui->actionCustomColorsEditor->setChecked(false);
    ui->actionHeightMapEditor->setChecked(false);
    ui->actionRulerTool->setChecked(false);
    ui->actionTileMapEditor->setChecked(false);
    ui->actionVisibilityCheckTool->setChecked(false);
    ui->actionShowNotPassableLandscape->setChecked(false);

    DAVA::int32 tools = scene->GetEnabledTools();

    bool anyEditorEnabled = false;
    if (tools & SceneEditor2::LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        ui->actionCustomColorsEditor->setChecked(true);
        anyEditorEnabled = true;
    }
    if (tools & SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
    {
        ui->actionHeightMapEditor->setChecked(true);
        anyEditorEnabled = true;
    }
    if (tools & SceneEditor2::LANDSCAPE_TOOL_RULER)
    {
        ui->actionRulerTool->setChecked(true);
        anyEditorEnabled = true;
    }
    if (tools & SceneEditor2::LANDSCAPE_TOOL_TILEMAP_EDITOR)
    {
        ui->actionTileMapEditor->setChecked(true);
        anyEditorEnabled = true;
    }
    if (tools & SceneEditor2::LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
    {
        ui->actionShowNotPassableLandscape->setChecked(true);
        anyEditorEnabled = true;
    }

    if (anyEditorEnabled)
    {
        SetVisibilityToolEnabledIfPossible(false);
    }
    ui->actionForceFirstLODonLandscape->setChecked(anyEditorEnabled);
    OnForceFirstLod(anyEditorEnabled);

    UpdateLandscapeRenderMode();
}

void QtMainWindow::OnCustomColorsEditor()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (!sceneEditor->customColorsSystem->IsLandscapeEditingEnabled())
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            DAVA::Logger::Error("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor.Get());
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EnableCustomColorsCommand(sceneEditor.Get(), true)));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor.Get());
        }
        return;
    }

    sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new DisableCustomColorsCommand(sceneEditor.Get(), true)));
    ui->actionCustomColorsEditor->setChecked(false);
}

void QtMainWindow::OnHeightmapEditor()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->heightmapEditorSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new DisableHeightmapEditorCommand(sceneEditor.Get())));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            DAVA::Logger::Error("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor.Get());
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EnableHeightmapEditorCommand(sceneEditor.Get())));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor.Get());
        }
    }
}

void QtMainWindow::OnRulerTool()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new DisableRulerToolCommand(sceneEditor.Get())));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            DAVA::Logger::Error("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor.Get());
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EnableRulerToolCommand(sceneEditor.Get())));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor.Get());
        }
    }
}

void QtMainWindow::OnTilemaskEditor()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new DisableTilemaskEditorCommand(sceneEditor.Get())));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            DAVA::Logger::Error("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor.Get());
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EnableTilemaskEditorCommand(sceneEditor.Get())));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor.Get());
        }
    }
}

void QtMainWindow::OnForceFirstLod(bool enabled)
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() == nullptr)
    {
        ui->actionForceFirstLODonLandscape->setChecked(false);
        return;
    }

    auto landscape = FindLandscape(scene.Get());
    if (landscape == nullptr)
    {
        ui->actionForceFirstLODonLandscape->setChecked(false);
        return;
    }

    landscape->SetForceMaxSubdiv(enabled);
    scene->visibilityCheckSystem->Recalculate();
}

void QtMainWindow::OnNotPassableTerrain()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled())
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new DisableNotPassableCommand(sceneEditor.Get())));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            DAVA::Logger::Error("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor.Get());
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EnableNotPassableCommand(sceneEditor.Get())));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor.Get());
        }
    }
}

void QtMainWindow::OnWayEditor()
{
    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->pathSystem->IsPathEditEnabled())
    {
        sceneEditor->Exec(std::make_unique<DisableWayEditCommand>(sceneEditor.Get()));
    }
    else
    {
        DAVA::int32 toolsEnabled = sceneEditor->GetEnabledTools();
        if (toolsEnabled)
        {
            DAVA::Logger::Error("Landscape tools should be disabled prior to enabling WayEditor");
            ui->actionWayEditor->setChecked(false);
        }
        else
        {
            sceneEditor->Exec(std::make_unique<EnableWayEditCommand>(sceneEditor.Get()));
        }
    }
}

void QtMainWindow::OnBuildStaticOcclusion()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    QtWaitDialog* waitOcclusionDlg = new QtWaitDialog(this);
    waitOcclusionDlg->Show("Static occlusion", "Please wait while building static occlusion.", true, true);

    bool sceneWasChanged = true;
    scene->staticOcclusionBuildSystem->Build();
    while (scene->staticOcclusionBuildSystem->IsInBuild())
    {
        if (waitOcclusionDlg->WasCanceled())
        {
            scene->staticOcclusionBuildSystem->Cancel();
            sceneWasChanged = false;
        }
        else
        {
            waitOcclusionDlg->SetValue(scene->staticOcclusionBuildSystem->GetBuildStatus());
            waitOcclusionDlg->SetMessage(QString::fromStdString(scene->staticOcclusionBuildSystem->GetBuildStatusInfo()));
        }
    }

    if (sceneWasChanged)
    {
        scene->MarkAsChanged();

        bool needSaveScene = SettingsManager::GetValue(Settings::Scene_SaveStaticOcclusion).AsBool();
        if (needSaveScene)
        {
            REGlobal::GetInvoker()->Invoke(REGlobal::SaveCurrentScene.ID);
        }

        ui->propertyEditor->ResetProperties();
    }

    delete waitOcclusionDlg;
}

void QtMainWindow::OnInavalidateStaticOcclusion()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;
    scene->staticOcclusionSystem->InvalidateOcclusion();
    scene->MarkAsChanged();
}

void QtMainWindow::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    if (projectDataWrapper == wrapper)
    {
        ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();

        // empty fields mean that ProjectManagerData was just added
        // this means that there is a first call
        if (data != nullptr && fields.empty())
        {
            const SpritesPackerModule* spritesPacker = data->GetSpritesModules();
            QObject::connect(spritesPacker, &SpritesPackerModule::SpritesReloaded, ui->sceneInfo, &SceneInfo::SpritesReloaded);
        }

        if (data != nullptr && !data->GetProjectPath().IsEmpty())
        {
            EnableProjectActions(true);
            SetupTitle(data->GetProjectPath().GetAbsolutePathname());
        }
        else
        {
            EnableProjectActions(false);
            SetupTitle(DAVA::String());
        }
    }
    else if (selectionWrapper == wrapper)
    {
        UpdateModificationActionsState();
    }
}

void QtMainWindow::OnObjectsTypeChanged(QAction* action)
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    ResourceEditor::eSceneObjectType objectType = (ResourceEditor::eSceneObjectType)action->data().toInt();
    if (objectType < ResourceEditor::ESOT_COUNT && objectType >= ResourceEditor::ESOT_NONE)
    {
        scene->debugDrawSystem->SetRequestedObjectType(objectType);
    }

    bool wasBlocked = objectTypesWidget->blockSignals(true);
    objectTypesWidget->setCurrentIndex(objectType + 1);
    objectTypesWidget->blockSignals(wasBlocked);
}

void QtMainWindow::OnObjectsTypeChanged(int type)
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    ResourceEditor::eSceneObjectType objectType = (ResourceEditor::eSceneObjectType)(type - 1);
    if (objectType < ResourceEditor::ESOT_COUNT && objectType >= ResourceEditor::ESOT_NONE)
    {
        scene->debugDrawSystem->SetRequestedObjectType(objectType);
    }
}

void QtMainWindow::LoadObjectTypes(SceneEditor2* scene)
{
    if (!scene)
        return;
    ResourceEditor::eSceneObjectType objectType = scene->debugDrawSystem->GetRequestedObjectType();
    objectTypesWidget->setCurrentIndex(objectType + 1);
}

void QtMainWindow::OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape)
{
    if (MainWindowDetails::GetCurrentScene() != scene)
    {
        return;
    }

    ui->actionModifySnapToLandscape->setChecked(isSpanToLandscape);
}

void QtMainWindow::OnHangingObjects()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    scene->debugDrawSystem->EnableHangingObjectsMode(ui->actionHangingObjects->isChecked());
}

void QtMainWindow::LoadHangingObjects(SceneEditor2* scene)
{
    ui->actionHangingObjects->setChecked(scene->debugDrawSystem->HangingObjectsModeEnabled());
    if (hangingObjectsWidget)
    {
        hangingObjectsWidget->SetHeight(DebugDrawSystem::HANGING_OBJECTS_HEIGHT);
    }
}

void QtMainWindow::OnHangingObjectsHeight(double value)
{
    DebugDrawSystem::HANGING_OBJECTS_HEIGHT = (DAVA::float32)value;
}

void QtMainWindow::OnMaterialLightViewChanged(bool)
{
    int newMode = EditorMaterialSystem::LIGHTVIEW_NOTHING;

    if (ui->actionAlbedo->isChecked())
        newMode |= EditorMaterialSystem::LIGHTVIEW_ALBEDO;
    if (ui->actionDiffuse->isChecked())
        newMode |= EditorMaterialSystem::LIGHTVIEW_DIFFUSE;
    if (ui->actionAmbient->isChecked())
        newMode |= EditorMaterialSystem::LIGHTVIEW_AMBIENT;
    if (ui->actionSpecular->isChecked())
        newMode |= EditorMaterialSystem::LIGHTVIEW_SPECULAR;

    if (newMode != SettingsManager::GetValue(Settings::Internal_MaterialsLightViewMode).AsInt32())
    {
        SettingsManager::SetValue(Settings::Internal_MaterialsLightViewMode, DAVA::VariantType(newMode));
    }

    if (MainWindowDetails::GetCurrentScene().Get() != nullptr)
    {
        MainWindowDetails::GetCurrentScene()->materialSystem->SetLightViewMode(newMode);
    }
}

void QtMainWindow::OnCustomQuality()
{
    QualitySwitcher::ShowDialog(globalOperations);
}

void QtMainWindow::SynchronizeStateWithUI()
{
    OnManualModifMode();
}

void QtMainWindow::OnEmptyEntity()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    DAVA::ScopedPtr<DAVA::Entity> newEntity(new DAVA::Entity());
    newEntity->SetName(ResourceEditor::ENTITY_NAME);

    scene->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(newEntity, scene.Get())));
}

void QtMainWindow::OnAddWindEntity()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    DAVA::ScopedPtr<DAVA::Entity> windEntity(new DAVA::Entity());
    windEntity->SetName(ResourceEditor::WIND_NODE_NAME);

    DAVA::Matrix4 ltMx = DAVA::Matrix4::MakeTranslation(DAVA::Vector3(0.f, 0.f, 20.f));
    GetTransformComponent(windEntity)->SetLocalTransform(&ltMx);

    windEntity->AddComponent(new DAVA::WindComponent());

    scene->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(windEntity, scene.Get())));
}

void QtMainWindow::OnAddPathEntity()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (scene.Get() == nullptr)
        return;

    DAVA::ScopedPtr<DAVA::Entity> pathEntity(new DAVA::Entity());
    pathEntity->SetName(ResourceEditor::PATH_NODE_NAME);
    DAVA::PathComponent* pc = new DAVA::PathComponent();

    pathEntity->AddComponent(pc);
    scene->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(pathEntity, scene.Get())));
}

bool QtMainWindow::LoadAppropriateTextureFormat()
{
    DAVA::eGPUFamily gpuFormat = Settings::GetGPUFormat();
    if (gpuFormat != DAVA::GPU_ORIGIN)
    {
        int answer = ShowQuestion("Inappropriate texture format",
                                  "Landscape editing is only allowed in original texture format.\nDo you want to reload textures in original format?",
                                  MB_FLAG_YES | MB_FLAG_NO, MB_FLAG_NO);
        if (answer == MB_FLAG_NO)
        {
            return false;
        }

        REGlobal::GetInvoker()->Invoke(REGlobal::ReloadTexturesOperation.ID, DAVA::eGPUFamily::GPU_ORIGIN);
    }

    return Settings::GetGPUFormat() == DAVA::GPU_ORIGIN;
}

void QtMainWindow::OnSwitchWithDifferentLODs(bool checked)
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    scene->debugDrawSystem->EnableSwithcesWithDifferentLODsMode(checked);

    if (checked)
    {
        DAVA::Set<DAVA::FastName> entitiNames;
        SceneValidator::FindSwitchesWithDifferentLODs(scene.Get(), entitiNames);

        DAVA::Set<DAVA::FastName>::iterator it = entitiNames.begin();
        DAVA::Set<DAVA::FastName>::iterator endIt = entitiNames.end();
        while (it != endIt)
        {
            DAVA::Logger::Info("Entity %s has different lods count.", it->c_str());
            ++it;
        }
    }
}

void QtMainWindow::DebugVersionInfo()
{
    if (!versionInfoWidget)
    {
        versionInfoWidget = new VersionInfoWidget(this);
        versionInfoWidget->setWindowFlags(Qt::Window);
        versionInfoWidget->setAttribute(Qt::WA_DeleteOnClose);
    }

    versionInfoWidget->show();
}

void QtMainWindow::OnConsoleItemClicked(const QString& data)
{
    PointerSerializer conv(data.toStdString());
    if (conv.CanConvert<DAVA::Entity*>())
    {
        DAVA::RefPtr<SceneEditor2> currentScene = MainWindowDetails::GetCurrentScene();
        if (currentScene.Get() != nullptr)
        {
            auto vec = conv.GetPointers<DAVA::Entity*>();
            if (!vec.empty())
            {
                SelectableGroup objects;
                DAVA::Vector<DAVA::Entity*> allEntities;
                currentScene->GetChildNodes(allEntities);
                for (auto entity : vec)
                {
                    if (std::find(allEntities.begin(), allEntities.end(), entity) != allEntities.end())
                    {
                        objects.Add(entity, currentScene->collisionSystem->GetUntransformedBoundingBox(entity));
                    }
                }

                if (!objects.IsEmpty())
                {
                    Selection::SetSelection(objects);
                    currentScene->cameraSystem->LookAt(objects.GetIntegralBoundingBox());
                }
            }
        }
    }
}

void QtMainWindow::OnGenerateHeightDelta()
{
    HeightDeltaTool* w = new HeightDeltaTool(this);
    w->setWindowFlags(Qt::Window);
    w->setAttribute(Qt::WA_DeleteOnClose);

    w->show();
}

void QtMainWindow::OnBatchProcessScene()
{
    SceneProcessor sceneProcessor;

    // For client developers: need to set entity processor derived from EntityProcessorBase
    //DestructibleSoundAdder *entityProcessor = new DestructibleSoundAdder();
    //sceneProcessor.SetEntityProcessor(entityProcessor);
    //SafeRelease(entityProcessor);

    DAVA::RefPtr<SceneEditor2> sceneEditor = MainWindowDetails::GetCurrentScene();
    if (sceneProcessor.Execute(sceneEditor.Get()))
    {
        REGlobal::GetInvoker()->Invoke(REGlobal::SaveCurrentScene.ID);
    }
}

void QtMainWindow::OnSnapCameraToLandscape(bool snap)
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    if (!scene)
        return;

    bool toggleProcessed = false;
    if (scene->cameraSystem)
    {
        toggleProcessed = scene->cameraSystem->SnapEditorCameraToLandscape(snap);
    }

    if (toggleProcessed)
    {
        ui->propertyEditor->ResetProperties();
    }
    else
    {
        ui->actionSnapCameraToLandscape->setChecked(!snap);
    }
}

void QtMainWindow::SetActionCheckedSilently(QAction* action, bool checked)
{
    DVASSERT(action);

    bool b = action->blockSignals(true);
    action->setChecked(checked);
    action->blockSignals(b);
}

void QtMainWindow::RemoveSelection()
{
    ::RemoveSelection(MainWindowDetails::GetCurrentScene().Get());
}

bool QtMainWindow::SetVisibilityToolEnabledIfPossible(bool enabled)
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    DVASSERT(scene.Get() != nullptr, "Switching visibility tool requires an opened scene");

    DAVA::int32 enabledTools = scene->GetEnabledTools();
    if (enabled && (enabledTools != 0))
    {
        DAVA::Logger::Error("Please disable Landscape editing tools before enabling Visibility Check System");
        enabled = false;
    }

    DAVA::Renderer::GetOptions()->SetOption(DAVA::RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM, enabled);

    if (enabled)
    {
        ui->actionForceFirstLODonLandscape->setChecked(true);
        OnForceFirstLod(true);
    }

    ui->actionEnableVisibilitySystem->setChecked(enabled);
    UpdateLandscapeRenderMode();

    return enabled;
}

void QtMainWindow::UpdateLandscapeRenderMode()
{
    DAVA::RefPtr<SceneEditor2> scene = MainWindowDetails::GetCurrentScene();
    DAVA::Landscape* landscape = FindLandscape(scene.Get());
    if (landscape != nullptr)
    {
        bool visibiilityEnabled = DAVA::Renderer::GetOptions()->IsOptionEnabled(DAVA::RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM);
        bool anyToolEnabled = scene->GetEnabledTools() != 0;
        bool enableInstancing = anyToolEnabled || !visibiilityEnabled;

        if (anyToolEnabled)
        {
            DVASSERT(visibiilityEnabled == false);
        }
        if (visibiilityEnabled)
        {
            DVASSERT(anyToolEnabled == false);
        }

        DAVA::Landscape::RenderMode newRenderMode = enableInstancing ?
        DAVA::Landscape::RenderMode::RENDERMODE_INSTANCING_MORPHING :
        DAVA::Landscape::RenderMode::RENDERMODE_NO_INSTANCING;

        landscape->SetRenderMode(newRenderMode);
    }
}

bool QtMainWindow::ParticlesArePacking() const
{
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    return data->GetSpritesModules()->IsRunning();
}

void QtMainWindow::CallAction(ID id, DAVA::Any&& args)
{
    switch (id)
    {
    case GlobalOperations::OpenScene:
    {
        // OpenScene function open WaitDialog and run EventLoop
        // To avoid embedded DAVA::OnFrame calling we will execute OpenScene inside Qt loop.
        DAVA::FilePath scenePath(args.Cast<DAVA::String>());
        delayedExecutor.DelayedExecute([scenePath]()
                                       {
                                           REGlobal::GetInvoker()->Invoke(REGlobal::OpenSceneOperation.ID, scenePath);
                                       });
    }
    break;
    case GlobalOperations::SetNameAsFilter:
        ui->sceneTreeFilterEdit->setText(args.Cast<DAVA::String>().c_str());
        break;
    case GlobalOperations::ShowMaterial:
        OnMaterialEditor(args.Cast<DAVA::NMaterial*>());
        break;
    case GlobalOperations::ReloadTexture:
        REGlobal::GetInvoker()->Invoke(REGlobal::ReloadTexturesOperation.ID, Settings::GetGPUFormat());
        break;
    default:
        DVASSERT(false, DAVA::Format("Not implemented action : %d", static_cast<DAVA::int32>(id)).c_str());
        break;
    }
}

QWidget* QtMainWindow::GetGlobalParentWidget() const
{
    return const_cast<QtMainWindow*>(this);
}

void QtMainWindow::ShowWaitDialog(const DAVA::String& tittle, const DAVA::String& message, DAVA::uint32 min /*= 0*/, DAVA::uint32 max /*= 100*/)
{
    WaitStart(QString::fromStdString(tittle), QString::fromStdString(message), min, max);
}

bool QtMainWindow::IsWaitDialogVisible() const
{
    return IsWaitDialogOnScreen();
}

void QtMainWindow::HideWaitDialog()
{
    WaitStop();
}

void QtMainWindow::ForEachScene(const DAVA::Function<void(SceneEditor2*)>& functor)
{
    REGlobal::GetAccessor()->ForEachContext([&](DAVA::TArc::DataContext& ctx)
                                            {
                                                SceneData* data = ctx.GetData<SceneData>();
                                                functor(data->GetScene().Get());
                                            });
}

void QtMainWindow::UpdateUndoActionText(const DAVA::String& text)
{
    QString actionText = text.empty() ? "Undo" : "Undo: " + QString::fromStdString(text);
    ui->actionUndo->setText(actionText);
    ui->actionUndo->setToolTip(actionText);
}

void QtMainWindow::UpdateRedoActionText(const DAVA::String& text)
{
    QString actionText = text.empty() ? "Redo" : "Redo: " + QString::fromStdString(text);
    ui->actionRedo->setText(actionText);
    ui->actionRedo->setToolTip(actionText);
}

void QtMainWindow::OnValidateScene()
{
    DAVA::RefPtr<SceneEditor2> currentScene = MainWindowDetails::GetCurrentScene();
    DVASSERT(currentScene.Get() != nullptr);

    SceneValidationDialog dlg(currentScene.Get());
    dlg.exec();
}

#include "DAVAEngine.h"

#include "mainwindow.h"
#include "Classes/Qt/BeastDialog/BeastDialog.h"
#include "Classes/Qt/CubemapEditor/CubemapTextureBrowser.h"
#include "Classes/Qt/CubemapEditor/CubemapUtils.h"
#include "Classes/Qt/DebugTools/VersionInfoWidget/VersionInfoWidget.h"
#include "Classes/Qt/DeviceInfo/DeviceList/DeviceListController.h"
#include "Classes/Qt/DeviceInfo/DeviceList/DeviceListWidget.h"
#include "Classes/Qt/ImageSplitterDialog/ImageSplitterDialog.h"
#include "Classes/Qt/Main/QtUtils.h"
#include "Classes/Qt/Main/Request.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"
#include "Classes/Qt/Project/ProjectManager.h"
#include "Classes/Qt/QualitySwitcher/QualitySwitcher.h"
#include "Classes/Qt/RunActionEventWidget/RunActionEventWidget.h"
#include "Classes/Qt/Scene/LandscapeThumbnails.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Scene/SceneHelper.h"
#include "Classes/Qt/Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Classes/Qt/Settings/SettingsDialog.h"
#include "Classes/Qt/Settings/SettingsManager.h"
#include "Classes/Qt/SoundComponentEditor/FMODSoundBrowser.h"
#include "Classes/Qt/SpritesPacker/SpritesPackerModule.h"
#include "Classes/Qt/TextureBrowser/TextureBrowser.h"
#include "Classes/Qt/TextureBrowser/TextureCache.h"
#include "Classes/Qt/NGTPropertyEditor/PropertyPanel.h"
#include "Classes/Qt/Tools/AddSwitchEntityDialog/AddSwitchEntityDialog.h"
#include "Classes/Qt/Tools/BaseAddEntityDialog/BaseAddEntityDialog.h"
#include "Classes/Qt/Tools/ColorPicker/ColorPicker.h"
#include "Classes/Qt/Tools/DeveloperTools/DeveloperTools.h"
#include "Classes/Qt/Tools/HangingObjectsHeight/HangingObjectsHeight.h"
#include "Classes/Qt/Tools/HeightDeltaTool/HeightDeltaTool.h"
#include "Classes/Qt/Tools/PathDescriptor/PathDescriptor.h"
#include "Classes/Qt/Tools/QtLabelWithActions/QtLabelWithActions.h"
#include "Classes/Qt/Tools/QtPosSaver/QtPosSaver.h"
#include "Classes/Qt/Tools/ToolButtonWithWidget/ToolButtonWithWidget.h"
#include "Classes/Qt/Tools/LoggerOutput/LoggerErrorHandler.h"

#ifdef __DAVAENGINE_SPEEDTREE__
#include "Classes/Qt/SpeedTreeImport/SpeedTreeImportDialog.h"
#endif

#include "Classes/Deprecated/EditorConfig.h"
#include "Classes/Deprecated/SceneValidator.h"

#include "Classes/CommandLine/SceneSaver/SceneSaver.h"
#include "Classes/Commands2/Base/CommandStack.h"
#include "Classes/Commands2/Base/CommandBatch.h"
#include "Classes/Commands2/AddComponentCommand.h"
#include "Classes/Commands2/BeastAction.h"
#include "Classes/Commands2/ConvertPathCommands.h"
#include "Classes/Commands2/CustomColorsCommands2.h"
#include "Classes/Commands2/EntityAddCommand.h"
#include "Classes/Commands2/HeightmapEditorCommands2.h"
#include "Classes/Commands2/PaintHeightDeltaAction.h"
#include "Classes/Commands2/RemoveComponentCommand.h"
#include "Classes/Commands2/TilemaskEditorCommands.h"
#include "Classes/Commands2/LandscapeToolsToggleCommand.h"

#include "Classes/SceneProcessing/SceneProcessor.h"

#include "Classes/Constants.h"
#include "Classes/StringConstants.h"

#include "TextureCompression/TextureConverter.h"

#include "QtTools/ConsoleWidget/LogWidget.h"
#include "QtTools/ConsoleWidget/LogModel.h"
#include "QtTools/ConsoleWidget/PointerSerializer.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/FileDialog/FileDialog.h"

#include "Platform/Qt5/QtLayer.h"

#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"

#include <core_generic_plugin/interfaces/i_component_context.hpp>

#include <QActionGroup>
#include <QColorDialog>
#include <QDesktopServices>
#include <QKeySequence>
#include <QMessageBox>
#include <QMetaObject>
#include <QMetaType>
#include <QShortcut>

QtMainWindow::QtMainWindow(wgt::IComponentContext& ngtContext_, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , waitDialog(nullptr)
    , beastWaitDialog(nullptr)
    , globalInvalidate(false)
    , modificationWidget(nullptr)
    , addSwitchEntityDialog(nullptr)
    , hangingObjectsWidget(nullptr)
    , developerTools(new DeveloperTools(this))
    , recentFiles(Settings::General_RecentFilesCount, Settings::Internal_RecentFiles)
    , recentProjects(Settings::General_RecentProjectsCount, Settings::Internal_RecentProjects)
    , ngtContext(ngtContext_)
    , propertyPanel(new PropertyPanel())
    , spritesPacker(new SpritesPackerModule())
{
    PathDescriptor::InitializePathDescriptors();

    new ProjectManager();
    ui->setupUi(this);

    recentFiles.SetMenu(ui->menuFile);
    recentProjects.SetMenu(ui->menuRecentProjects);

    spritesPacker->SetAction(ui->actionReloadSprites);
    ProjectManager::Instance()->SetSpritesPacker(spritesPacker.get());

    centralWidget()->setMinimumSize(ui->sceneTabWidget->minimumSize());

    SetupTitle();

    qApp->installEventFilter(this);

    new QtPosSaver(this);

    SetupDocks();
    SetupMainMenu();
    SetupToolBars();
    SetupStatusBar();
    SetupActions();
    SetupShortCuts();

    // create tool windows
    new TextureBrowser(this);
    new MaterialEditor(this);
    new FMODSoundBrowser(this);

    waitDialog = new QtWaitDialog(this);
    beastWaitDialog = new QtWaitDialog(this);

    connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString&)), this, SLOT(ProjectOpened(const QString&)));
    connect(ProjectManager::Instance(), SIGNAL(ProjectClosed()), this, SLOT(ProjectClosed()));
    connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2*, const Command2*, bool)), this, SLOT(SceneCommandExecuted(SceneEditor2*, const Command2*, bool)));
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
    connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
    connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)), this, SLOT(SceneSelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)));
    connect(SceneSignals::Instance(), SIGNAL(EditorLightEnabled(bool)), this, SLOT(EditorLightEnabled(bool)));
    connect(this, SIGNAL(TexturesReloaded()), TextureCache::Instance(), SLOT(ClearCache()));
    connect(ui->sceneTabWidget->GetDavaWidget(), SIGNAL(Initialized()), ui->landscapeEditorControlsPlaceholder, SLOT(OnOpenGLInitialized()));

    LoadGPUFormat();
    LoadMaterialLightViewMode();

    EnableGlobalTimeout(globalInvalidate);

    EnableProjectActions(false);
    EnableSceneActions(false);

    DiableUIForFutureUsing();
    SynchronizeStateWithUI();

    wgt::IUIApplication* uiApplication = ngtContext.queryInterface<wgt::IUIApplication>();
    wgt::IUIFramework* uiFramework = ngtContext.queryInterface<wgt::IUIFramework>();
    DVASSERT(uiApplication != nullptr);
    DVASSERT(uiFramework != nullptr);
    propertyPanel->Initialize(*uiFramework, *uiApplication);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged, propertyPanel.get(), &PropertyPanel::SceneSelectionChanged);
}

QtMainWindow::~QtMainWindow()
{
    wgt::IUIApplication* uiApplication = ngtContext.queryInterface<wgt::IUIApplication>();
    DVASSERT(uiApplication != nullptr);
    propertyPanel->Finalize(*uiApplication);
    propertyPanel.reset();

    const auto& logWidget = qobject_cast<LogWidget*>(dockConsole->widget());
    const auto dataToSave = logWidget->Serialize();

    DAVA::VariantType var(reinterpret_cast<const DAVA::uint8*>(dataToSave.data()), dataToSave.size());
    SettingsManager::Instance()->SetValue(Settings::Internal_LogWidget, var);

    DAVA::SafeDelete(addSwitchEntityDialog);

    TextureBrowser::Instance()->Release();
    MaterialEditor::Instance()->Release();

    delete ui;
    ui = nullptr;

    ProjectManager::Instance()->Release();
}

Ui::MainWindow* QtMainWindow::GetUI()
{
    return ui;
}

SceneTabWidget* QtMainWindow::GetSceneWidget()
{
    return ui->sceneTabWidget;
}

SceneEditor2* QtMainWindow::GetCurrentScene()
{
    return ui->sceneTabWidget->GetCurrentScene();
}

bool QtMainWindow::SaveScene(SceneEditor2* scene)
{
    DAVA::FilePath scenePath = scene->GetScenePath();
    if (!scene->IsLoaded() || scenePath.IsEmpty())
    {
        return SaveSceneAs(scene);
    }
    else
    {
        if (scene->IsChanged())
        {
            SaveAllSceneEmitters(scene);
            DAVA::SceneFileV2::eError ret = scene->SaveScene(scenePath);
            if (DAVA::SceneFileV2::ERROR_NO_ERROR != ret)
            {
                QMessageBox::warning(this, "Save error", "An error occurred while saving the scene. See log for more info.", QMessageBox::Ok);
            }
            else
            {
                return true;
            }
        }
    }

    return false;
}

bool QtMainWindow::SaveSceneAs(SceneEditor2* scene)
{
    if (nullptr == scene)
    {
        return false;
    }

    DAVA::FilePath saveAsPath = scene->GetScenePath();
    if (!DAVA::FileSystem::Instance()->Exists(saveAsPath))
    {
        DAVA::FilePath dataSourcePath = ProjectManager::Instance()->GetDataSourcePath();
        saveAsPath = dataSourcePath.MakeDirectoryPathname() + scene->GetScenePath().GetFilename();
    }

    QString selectedPath = FileDialog::getSaveFileName(this, "Save scene as", saveAsPath.GetAbsolutePathname().c_str(), "DAVA Scene V2 (*.sc2)");
    if (selectedPath.isEmpty())
    {
        return false;
    }

    DAVA::FilePath scenePath = DAVA::FilePath(selectedPath.toStdString());
    if (scenePath.IsEmpty())
    {
        return false;
    }

    scene->SetScenePath(scenePath);

    SaveAllSceneEmitters(scene);
    DAVA::SceneFileV2::eError ret = scene->SaveScene(scenePath);
    if (DAVA::SceneFileV2::ERROR_NO_ERROR != ret)
    {
        QMessageBox::warning(this, "Save error", "An error occurred while saving the scene. Please, see logs for more info.", QMessageBox::Ok);
    }
    else
    {
        recentFiles.Add(scenePath.GetAbsolutePathname());
        return true;
    }

    return false;
}

QString GetSaveFolderForEmitters()
{
    const DAVA::FilePath defaultPath = SettingsManager::GetValue(Settings::Internal_ParticleLastEmitterDir).AsFilePath();
    QString particlesPath;
    if (defaultPath.IsEmpty())
    {
        particlesPath = QString::fromStdString(ProjectManager::Instance()->GetParticlesConfigPath().GetAbsolutePathname());
    }
    else
    {
        particlesPath = QString::fromStdString(defaultPath.GetAbsolutePathname());
    }

    return particlesPath;
}

void QtMainWindow::CollectEmittersForSave(DAVA::ParticleEmitter* topLevelEmitter, DAVA::List<EmitterDescriptor>& emitters, const DAVA::String& entityName) const
{
    DVASSERT(topLevelEmitter != nullptr);

    for (auto& layer : topLevelEmitter->layers)
    {
        if (nullptr != layer->innerEmitter)
        {
            CollectEmittersForSave(layer->innerEmitter, emitters, entityName);
            emitters.emplace_back(EmitterDescriptor(layer->innerEmitter, layer, layer->innerEmitter->configPath, entityName));
        }
    }

    emitters.emplace_back(EmitterDescriptor(topLevelEmitter, nullptr, topLevelEmitter->configPath, entityName));
}

void QtMainWindow::SaveAllSceneEmitters(SceneEditor2* scene) const
{
    DVASSERT(nullptr != scene);

    if (!SettingsManager::GetValue(Settings::Scene_SaveEmitters).AsBool())
    {
        return;
    }

    DAVA::List<DAVA::Entity*> effectEntities;
    scene->GetChildEntitiesWithComponent(effectEntities, DAVA::Component::PARTICLE_EFFECT_COMPONENT);
    if (effectEntities.empty())
    {
        return;
    }

    DAVA::List<EmitterDescriptor> emittersForSave;
    for (auto& entityWithEffect : effectEntities)
    {
        const DAVA::String entityName = entityWithEffect->GetName().c_str();
        DAVA::ParticleEffectComponent* effect = GetEffectComponent(entityWithEffect);
        for (DAVA::int32 i = 0, sz = effect->GetEmittersCount(); i < sz; ++i)
        {
            CollectEmittersForSave(effect->GetEmitterInstance(i)->GetEmitter(), emittersForSave, entityName);
        }
    }

    for (auto& descriptor : emittersForSave)
    {
        DAVA::ParticleEmitter* emitter = descriptor.emitter;
        const DAVA::String& entityName = descriptor.entityName;

        DAVA::FilePath yamlPathForSaving = descriptor.yamlPath;
        if (yamlPathForSaving.IsEmpty())
        {
            QString particlesPath = GetSaveFolderForEmitters();

            DAVA::FileSystem::Instance()->CreateDirectory(DAVA::FilePath(particlesPath.toStdString()), true); // to ensure that folder is created

            QString emitterPathname = particlesPath + QString("%1_%2.yaml").arg(entityName.c_str()).arg(emitter->name.c_str());
            QString filePath = FileDialog::getSaveFileName(NULL, QString("Save Particle Emitter ") + QString(emitter->name.c_str()), emitterPathname, QString("YAML File (*.yaml)"));

            if (filePath.isEmpty())
            {
                continue;
            }

            yamlPathForSaving = DAVA::FilePath(filePath.toStdString());
            SettingsManager::SetValue(Settings::Internal_ParticleLastEmitterDir, DAVA::VariantType(yamlPathForSaving.GetDirectory()));
        }

        if (nullptr != descriptor.ownerLayer)
        {
            descriptor.ownerLayer->innerEmitterPath = yamlPathForSaving;
        }
        emitter->SaveToYaml(yamlPathForSaving);
    }
}

DAVA::eGPUFamily QtMainWindow::GetGPUFormat()
{
    return static_cast<DAVA::eGPUFamily>(DAVA::GPUFamilyDescriptor::ConvertValueToGPU(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}

void QtMainWindow::SetGPUFormat(DAVA::eGPUFamily gpu)
{
    // before reloading textures we should save tile-mask texture for all opened scenes
    if (SaveTilemask())
    {
        SettingsManager::SetValue(Settings::Internal_TextureViewGPU, DAVA::VariantType(static_cast<DAVA::uint32>(gpu)));
        DAVA::Texture::SetDefaultGPU(gpu);

        SceneHelper::TextureCollector collector;
        DAVA::Set<DAVA::NMaterial*> allSceneMaterials;
        for (int tab = 0; tab < GetSceneWidget()->GetTabCount(); ++tab)
        {
            SceneEditor2* scene = GetSceneWidget()->GetTabScene(tab);
            SceneHelper::EnumerateSceneTextures(scene, collector);
            SceneHelper::EnumerateMaterials(scene, allSceneMaterials);
        }

        DAVA::TexturesMap& allScenesTextures = collector.GetTextures();
        if (!allScenesTextures.empty())
        {
            int progress = 0;
            WaitStart("Reloading textures...", "", 0, allScenesTextures.size());

            DAVA::TexturesMap::const_iterator it = allScenesTextures.begin();
            DAVA::TexturesMap::const_iterator end = allScenesTextures.end();

            for (; it != end; ++it)
            {
                it->second->ReloadAs(gpu);

#if defined(USE_FILEPATH_IN_MAP)
                WaitSetMessage(it->first.GetAbsolutePathname().c_str());
#else //#if defined(USE_FILEPATH_IN_MAP)
                WaitSetMessage(it->first.c_str());
#endif //#if defined(USE_FILEPATH_IN_MAP)
                WaitSetValue(progress++);
            }

            emit TexturesReloaded();

            WaitStop();
        }

        if (!allSceneMaterials.empty())
        {
            for (auto m : allSceneMaterials)
            {
                m->InvalidateTextureBindings();
            }
        }
    }
    LoadGPUFormat();
}

void QtMainWindow::WaitStart(const QString& title, const QString& message, int min /* = 0 */, int max /* = 100 */)
{
    waitDialog->SetRange(min, max);
    waitDialog->Show(title, message, false, false);
}

void QtMainWindow::WaitSetMessage(const QString& messsage)
{
    waitDialog->SetMessage(messsage);
}

void QtMainWindow::WaitSetValue(int value)
{
    waitDialog->SetValue(value);
}

void QtMainWindow::WaitStop()
{
    waitDialog->Reset();
}

bool QtMainWindow::eventFilter(QObject* obj, QEvent* event)
{
    QEvent::Type eventType = event->type();

    if (qApp == obj)
    {
        if (QEvent::ApplicationStateChange == eventType)
        {
            QApplicationStateChangeEvent* stateChangeEvent = static_cast<QApplicationStateChangeEvent*>(event);
            Qt::ApplicationState state = stateChangeEvent->applicationState();
            switch (state)
            {
            case Qt::ApplicationInactive:
            {
                if (DAVA::QtLayer::Instance())
                {
                    DAVA::QtLayer::Instance()->OnSuspend();
                }
                break;
            }
            case Qt::ApplicationActive:
            {
                if (DAVA::QtLayer::Instance())
                {
                    DAVA::QtLayer::Instance()->OnResume();
                    // Fix for menuBar rendering
                    const auto isMenuBarEnabled = ui->menuBar->isEnabled();
                    ui->menuBar->setEnabled(false);
                    ui->menuBar->setEnabled(isMenuBarEnabled);
                }
                break;
            }
            default:
                break;
            }
        }
    }
    else if (obj == this)
    {
        if (eventType == QEvent::Close)
        {
            if (ShouldClose(static_cast<QCloseEvent*>(event)) == false)
            {
                event->ignore();
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void QtMainWindow::SetupTitle()
{
    DAVA::KeyedArchive* options = DAVA::Core::Instance()->GetOptions();
    QString title = options->GetString("title").c_str();

    if (ProjectManager::Instance()->IsOpened())
    {
        title += " | Project - ";
        title += ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname().c_str();
    }

    this->setWindowTitle(title);
}

void QtMainWindow::SetupMainMenu()
{
    ui->menuDockWindows->addAction(ui->dockSceneInfo->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockLibrary->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockProperties->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockParticleEditor->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockParticleEditorTimeLine->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockSceneTree->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockLODEditor->toggleViewAction());
    ui->menuDockWindows->addAction(ui->dockLandscapeEditorControls->toggleViewAction());

    ui->menuDockWindows->addAction(dockActionEvent->toggleViewAction());
    ui->menuDockWindows->addAction(dockConsole->toggleViewAction());

    recentFiles.InitMenuItems();
    recentProjects.InitMenuItems();
}

void QtMainWindow::SetupToolBars()
{
    QAction* actionMainToolBar = ui->mainToolBar->toggleViewAction();
    QAction* actionModifToolBar = ui->modificationToolBar->toggleViewAction();
    QAction* actionLandscapeToolbar = ui->landscapeToolBar->toggleViewAction();

    ui->menuToolbars->addAction(actionMainToolBar);
    ui->menuToolbars->addAction(actionModifToolBar);
    ui->menuToolbars->addAction(actionLandscapeToolbar);
    ui->menuToolbars->addAction(ui->sceneToolBar->toggleViewAction());
    ui->menuToolbars->addAction(ui->testingToolBar->toggleViewAction());
    ui->menuToolbars->addAction(ui->cameraToolBar->toggleViewAction());

    // undo/redo
    QToolButton* undoBtn = (QToolButton*)ui->mainToolBar->widgetForAction(ui->actionUndo);
    QToolButton* redoBtn = (QToolButton*)ui->mainToolBar->widgetForAction(ui->actionRedo);
    undoBtn->setPopupMode(QToolButton::MenuButtonPopup);
    redoBtn->setPopupMode(QToolButton::MenuButtonPopup);

    // modification widget
    modificationWidget = new ModificationWidget(nullptr);
    ui->modificationToolBar->insertWidget(ui->actionModifyReset, modificationWidget);
    connect(ui->actionModifySnapToLandscape, &QAction::triggered, modificationWidget, &ModificationWidget::OnSnapToLandscapeChanged);

    // adding reload textures actions
    {
        QToolButton* reloadTexturesBtn = new QToolButton();
        reloadTexturesBtn->setMenu(ui->menuTexturesForGPU);
        reloadTexturesBtn->setPopupMode(QToolButton::MenuButtonPopup);
        reloadTexturesBtn->setDefaultAction(ui->actionReloadTextures);
        reloadTexturesBtn->setMaximumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);
        reloadTexturesBtn->setMinimumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);
        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addWidget(reloadTexturesBtn);
        reloadTexturesBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        reloadTexturesBtn->setAutoRaise(false);
    }

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
    QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), ui->statusBar, SLOT(SceneActivated(SceneEditor2*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)), ui->statusBar, SLOT(SceneSelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2*, const Command2*, bool)), ui->statusBar, SLOT(CommandExecuted(SceneEditor2*, const Command2*, bool)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(StructureChanged(SceneEditor2*, DAVA::Entity*)), ui->statusBar, SLOT(StructureChanged(SceneEditor2*, DAVA::Entity*)));
    QObject::connect(SceneSignals::Instance(), &SceneSignals::UndoRedoStateChanged, this, &QtMainWindow::SceneUndoRedoStateChanged);
    QObject::connect(this, SIGNAL(GlobalInvalidateTimeout()), ui->statusBar, SLOT(UpdateByTimer()));

    auto CreateStatusBarButton = [](QAction* action, QStatusBar* statusBar)
    {
        auto* statusBtn = new QToolButton();
        statusBtn->setDefaultAction(action);
        statusBtn->setAutoRaise(true);
        statusBtn->setMaximumSize(QSize(16, 16));
        statusBar->insertPermanentWidget(0, statusBtn);
    };

    CreateStatusBarButton(ui->actionShowEditorGizmo, ui->statusBar);
    CreateStatusBarButton(ui->actionLightmapCanvas, ui->statusBar);
    CreateStatusBarButton(ui->actionOnSceneSelection, ui->statusBar);
    CreateStatusBarButton(ui->actionShowStaticOcclusion, ui->statusBar);
    CreateStatusBarButton(ui->actionEnableVisibilitySystem, ui->statusBar);
    CreateStatusBarButton(ui->actionEnableDisableShadows, ui->statusBar);

    QObject::connect(ui->sceneTabWidget->GetDavaWidget(), SIGNAL(Resized(int, int)), ui->statusBar, SLOT(OnSceneGeometryChaged(int, int)));
}

void QtMainWindow::SetupDocks()
{
    QObject::connect(ui->sceneTreeFilterClear, SIGNAL(pressed()), ui->sceneTreeFilterEdit, SLOT(clear()));
    QObject::connect(ui->sceneTreeFilterEdit, SIGNAL(textChanged(const QString&)), ui->sceneTree, SLOT(SetFilter(const QString&)));

    QObject::connect(ui->sceneTabWidget, SIGNAL(CloseTabRequest(int, Request*)), this, SLOT(OnCloseTabRequest(int, Request*)));

    QObject::connect(this, SIGNAL(GlobalInvalidateTimeout()), ui->sceneInfo, SLOT(UpdateInfoByTimer()));
    QObject::connect(this, SIGNAL(TexturesReloaded()), ui->sceneInfo, SLOT(TexturesReloaded()));

    QObject::connect(spritesPacker.get(), &SpritesPackerModule::SpritesReloaded, this, &QtMainWindow::RestartParticleEffects);
    QObject::connect(spritesPacker.get(), &SpritesPackerModule::SpritesReloaded, ui->sceneInfo, &SceneInfo::SpritesReloaded);

    ui->libraryWidget->SetupSignals();
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

    ui->dockProperties->Init();
}

void QtMainWindow::SetupActions()
{
    // scene file actions
    QObject::connect(ui->actionOpenProject, &QAction::triggered, this, &QtMainWindow::OnProjectOpen);
    QObject::connect(ui->actionCloseProject, &QAction::triggered, this, &QtMainWindow::OnProjectClose);
    QObject::connect(ui->actionOpenScene, &QAction::triggered, this, &QtMainWindow::OnSceneOpen);
    QObject::connect(ui->actionNewScene, &QAction::triggered, this, &QtMainWindow::OnSceneNew);
    QObject::connect(ui->actionSaveScene, &QAction::triggered, this, &QtMainWindow::OnSceneSave);
    QObject::connect(ui->actionSaveSceneAs, &QAction::triggered, this, &QtMainWindow::OnSceneSaveAs);
    QObject::connect(ui->actionOnlyOriginalTextures, &QAction::triggered, this, &QtMainWindow::OnSceneSaveToFolder);
    QObject::connect(ui->actionWithCompressedTextures, &QAction::triggered, this, &QtMainWindow::OnSceneSaveToFolderCompressed);

    QObject::connect(ui->menuFile, &QMenu::triggered, this, &QtMainWindow::OnRecentFilesTriggered);
    QObject::connect(ui->menuRecentProjects, &QMenu::triggered, this, &QtMainWindow::OnRecentProjectsTriggered);

    // export
    QObject::connect(ui->menuExport, &QMenu::triggered, this, &QtMainWindow::ExportMenuTriggered);
    ui->actionExportPVRIOS->setData(DAVA::GPU_POWERVR_IOS);
    ui->actionExportPVRAndroid->setData(DAVA::GPU_POWERVR_ANDROID);
    ui->actionExportTegra->setData(DAVA::GPU_TEGRA);
    ui->actionExportMali->setData(DAVA::GPU_MALI);
    ui->actionExportAdreno->setData(DAVA::GPU_ADRENO);
    ui->actionExportDX11->setData(DAVA::GPU_DX11);
    ui->actionExportPNG->setData(DAVA::GPU_ORIGIN);

// import
#ifdef __DAVAENGINE_SPEEDTREE__
    QObject::connect(ui->actionImportSpeedTreeXML, &QAction::triggered, this, &QtMainWindow::OnImportSpeedTreeXML);
#endif //__DAVAENGINE_SPEEDTREE__

    // reload
    ui->actionReloadPoverVRIOS->setData(DAVA::GPU_POWERVR_IOS);
    ui->actionReloadPoverVRAndroid->setData(DAVA::GPU_POWERVR_ANDROID);
    ui->actionReloadTegra->setData(DAVA::GPU_TEGRA);
    ui->actionReloadMali->setData(DAVA::GPU_MALI);
    ui->actionReloadAdreno->setData(DAVA::GPU_ADRENO);
    ui->actionReloadDX11->setData(DAVA::GPU_DX11);
    ui->actionReloadPNG->setData(DAVA::GPU_ORIGIN);

    QActionGroup* reloadGroup = new QActionGroup(this);
    QList<QAction*> reloadActions = ui->menuTexturesForGPU->actions();
    for (int i = 0; i < reloadActions.size(); i++)
    {
        reloadGroup->addAction(reloadActions[i]);
    }

    QObject::connect(ui->menuTexturesForGPU, SIGNAL(triggered(QAction*)), this, SLOT(OnReloadTexturesTriggered(QAction*)));
    QObject::connect(ui->actionReloadTextures, SIGNAL(triggered()), this, SLOT(OnReloadTextures()));

    QObject::connect(ui->actionAlbedo, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));
    QObject::connect(ui->actionAmbient, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));
    QObject::connect(ui->actionDiffuse, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));
    QObject::connect(ui->actionSpecular, SIGNAL(toggled(bool)), this, SLOT(OnMaterialLightViewChanged(bool)));

    QObject::connect(ui->actionShowEditorGizmo, SIGNAL(toggled(bool)), this, SLOT(OnEditorGizmoToggle(bool)));
    QObject::connect(ui->actionLightmapCanvas, SIGNAL(toggled(bool)), this, SLOT(OnViewLightmapCanvas(bool)));
    QObject::connect(ui->actionOnSceneSelection, SIGNAL(toggled(bool)), this, SLOT(OnAllowOnSceneSelectionToggle(bool)));
    QObject::connect(ui->actionShowStaticOcclusion, SIGNAL(toggled(bool)), this, SLOT(OnShowStaticOcclusionToggle(bool)));
    QObject::connect(ui->actionEnableVisibilitySystem, SIGNAL(triggered(bool)), this, SLOT(OnEnableVisibilitySystemToggle(bool)));

    QObject::connect(ui->actionRefreshVisibilitySystem, SIGNAL(triggered()), this, SLOT(OnRefreshVisibilitySystem()));
    QObject::connect(ui->actionFixCurrentFrame, SIGNAL(triggered()), this, SLOT(OnFixVisibilityFrame()));
    QObject::connect(ui->actionReleaseCurrentFrame, SIGNAL(triggered()), this, SLOT(OnReleaseVisibilityFrame()));

    QObject::connect(ui->actionEnableDisableShadows, &QAction::toggled, this, &QtMainWindow::OnEnableDisableShadows);

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

    QObject::connect(ui->actionShowSettings, SIGNAL(triggered()), this, SLOT(OnShowSettings()));

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

    connect(ui->actionDeviceList, &QAction::triggered, this, &QtMainWindow::DebugDeviceList);
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
    QObject::connect(ui->actionReloadShader, SIGNAL(triggered()), this, SLOT(OnReloadShaders()));
    QObject::connect(ui->actionSwitchesWithDifferentLODs, SIGNAL(triggered(bool)), this, SLOT(OnSwitchWithDifferentLODs(bool)));

    QObject::connect(ui->actionBatchProcess, SIGNAL(triggered(bool)), this, SLOT(OnBatchProcessScene()));

    QObject::connect(ui->actionSnapCameraToLandscape, SIGNAL(triggered(bool)), this, SLOT(OnSnapCameraToLandscape(bool)));
}

void QtMainWindow::SetupShortCuts()
{
    // select mode
    connect(ui->sceneTabWidget, SIGNAL(Escape()), this, SLOT(OnSelectMode()));

    // delete
    connect(new QShortcut(QKeySequence(Qt::Key_Delete), ui->sceneTabWidget), SIGNAL(activated()), this, SLOT(RemoveSelection()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace), ui->sceneTabWidget), SIGNAL(activated()), this, SLOT(RemoveSelection()));

    // scene tree collapse/expand
    connect(new QShortcut(QKeySequence(Qt::Key_X), ui->sceneTree), SIGNAL(activated()), ui->sceneTree, SLOT(CollapseSwitch()));

    //tab closing
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), ui->sceneTabWidget), SIGNAL(activated()), ui->sceneTabWidget, SLOT(TabBarCloseCurrentRequest()));
#if defined(__DAVAENGINE_WIN32__)
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4), ui->sceneTabWidget), SIGNAL(activated()), ui->sceneTabWidget, SLOT(TabBarCloseCurrentRequest()));
#endif
}

// ###################################################################################################
// Scene signals
// ###################################################################################################

void QtMainWindow::ProjectOpened(const QString& path)
{
    EditorConfig::Instance()->ParseConfig(ProjectManager::Instance()->GetProjectPath() + "EditorConfig.yaml");

    EnableProjectActions(true);
    SetupTitle();
}

void QtMainWindow::ProjectClosed()
{
    EnableProjectActions(false);
    SetupTitle();
}

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

    DAVA::int32 tools = scene->GetEnabledTools();
    UpdateConflictingActionsState(tools == 0);
    UpdateModificationActionsState();

    ui->actionSwitchesWithDifferentLODs->setChecked(false);
    ui->actionSnapCameraToLandscape->setChecked(false);
    if (nullptr != scene)
    {
        if (scene->debugDrawSystem)
            ui->actionSwitchesWithDifferentLODs->setChecked(scene->debugDrawSystem->SwithcesWithDifferentLODsModeEnabled());

        if (scene->cameraSystem)
            ui->actionSnapCameraToLandscape->setChecked(scene->cameraSystem->IsEditorCameraSnappedToLandscape());

        SceneSelectionChanged(scene, &scene->selectionSystem->GetSelection(), nullptr);
    }

    SceneUndoRedoStateChanged(scene);
}

void QtMainWindow::SceneDeactivated(SceneEditor2* scene)
{
    // block some actions, when there is no scene
    EnableSceneActions(false);
}

void QtMainWindow::SceneSelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)
{
    UpdateModificationActionsState();
}

void QtMainWindow::EnableProjectActions(bool enable)
{
    ui->actionNewScene->setEnabled(enable);
    ui->actionOpenScene->setEnabled(enable);
    ui->actionCubemapEditor->setEnabled(enable);
    ui->actionImageSplitter->setEnabled(enable);
    ui->dockLibrary->setEnabled(enable);
    ui->actionCloseProject->setEnabled(enable);

    recentFiles.EnableMenuItems(enable);
}

void QtMainWindow::EnableSceneActions(bool enable)
{
    ui->actionUndo->setEnabled(enable);
    ui->actionRedo->setEnabled(enable);

    ui->dockLODEditor->setEnabled(enable);
    ui->dockProperties->setEnabled(enable);
    ui->dockSceneTree->setEnabled(enable);
    ui->dockSceneInfo->setEnabled(enable);

    ui->actionSaveScene->setEnabled(enable);
    ui->actionSaveSceneAs->setEnabled(enable);
    ui->actionOnlyOriginalTextures->setEnabled(enable);
    ui->actionWithCompressedTextures->setEnabled(enable);

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

    ui->actionEnableCameraLight->setEnabled(enable);
    ui->actionReloadTextures->setEnabled(enable);

    QAction* actionReloadSprites = spritesPacker->GetReloadAction();
    actionReloadSprites->setEnabled(enable);

    ui->actionSetLightViewMode->setEnabled(enable);

    ui->actionSaveHeightmapToPNG->setEnabled(enable);
    ui->actionSaveTiledTexture->setEnabled(enable);

    ui->actionBeastAndSave->setEnabled(enable);

    ui->actionHangingObjects->setEnabled(enable);

    ui->menuExport->setEnabled(enable);
    ui->menuEdit->setEnabled(enable);
    ui->menuCreateNode->setEnabled(enable);
    ui->menuScene->setEnabled(enable);
    ui->menuLightView->setEnabled(enable);
    ui->menuTexturesForGPU->setEnabled(enable);

    ui->sceneToolBar->setEnabled(enable);
    ui->actionConvertModifiedTextures->setEnabled(enable);

    ui->actionReloadShader->setEnabled(enable);
    ui->actionSwitchesWithDifferentLODs->setEnabled(enable);

    ui->actionSnapCameraToLandscape->setEnabled(enable);

    // Fix for menuBar rendering
    const auto isMenuBarEnabled = ui->menuBar->isEnabled();
    ui->menuBar->setEnabled(false);
    ui->menuBar->setEnabled(isMenuBarEnabled);
}

void QtMainWindow::UpdateModificationActionsState()
{
    SceneEditor2* scene = GetCurrentScene();
    bool isMultiple = (nullptr != scene) && (scene->selectionSystem->GetSelection().GetSize() > 1);

    // modificationWidget determines inside, if values could be modified and enables/disables itself
    modificationWidget->ReloadValues();
    bool canModify = modificationWidget->isEnabled();

    ui->actionModifyReset->setEnabled(canModify);
    ui->actionCenterPivotPoint->setEnabled(canModify && !isMultiple);
    ui->actionZeroPivotPoint->setEnabled(canModify && !isMultiple);
}

void QtMainWindow::UpdateWayEditor(const Command2* command, bool redo)
{
    if (command->MatchCommandID(CMDID_ENABLE_WAYEDIT))
    {
        DVASSERT(command->MatchCommandID(CMDID_DISABLE_WAYEDIT) == false);
        SetActionCheckedSilently(ui->actionWayEditor, redo);
    }
    else if (command->MatchCommandID(CMDID_DISABLE_WAYEDIT))
    {
        DVASSERT(command->MatchCommandID(CMDID_ENABLE_WAYEDIT) == false);
        SetActionCheckedSilently(ui->actionWayEditor, !redo);
    }
}

void QtMainWindow::SceneCommandExecuted(SceneEditor2* scene, const Command2* command, bool redo)
{
    if (scene == GetCurrentScene())
    {
        UpdateModificationActionsState();

        auto UpdateCameraState = [this, scene](const DAVA::Entity* entity)
        {
            if (entity && entity->GetName() == ResourceEditor::EDITOR_DEBUG_CAMERA)
            {
                SetActionCheckedSilently(ui->actionSnapCameraToLandscape, scene->cameraSystem->IsEditorCameraSnappedToLandscape());
                return true;
            }
            return false;
        };

        if (command->GetId() == CMDID_BATCH)
        {
            const CommandBatch* batch = static_cast<const CommandBatch*>(command);
            const DAVA::uint32 count = batch->Size();
            for (DAVA::uint32 i = 0; i < count; ++i)
            {
                const Command2* cmd = batch->GetCommand(i);
                if (UpdateCameraState(cmd->GetEntity()))
                {
                    break;
                }
            }
        }
        else
        {
            UpdateCameraState(command->GetEntity());
        }

        UpdateWayEditor(command, redo);
    }
}

// ###################################################################################################
// Mainwindow Qt actions
// ###################################################################################################

void QtMainWindow::OnProjectOpen()
{
    DAVA::FilePath incomePath = ProjectManager::Instance()->ProjectOpenDialog();
    OpenProject(incomePath);
}

void QtMainWindow::OpenProject(const DAVA::FilePath& projectPath)
{
    if (!projectPath.IsEmpty() &&
        ProjectManager::Instance()->GetProjectPath() != projectPath &&
        ui->sceneTabWidget->CloseAllTabs())
    {
        ProjectManager::Instance()->OpenProject(projectPath);
        recentProjects.Add(projectPath.GetAbsolutePathname());
    }
}

void QtMainWindow::OnProjectClose()
{
    if (ui->sceneTabWidget->CloseAllTabs())
    {
        ProjectManager::Instance()->CloseProject();
    }
}

void QtMainWindow::OnSceneNew()
{
    if (ProjectManager::Instance()->IsOpened())
    {
        ui->sceneTabWidget->OpenTab();
    }
}

void QtMainWindow::OnSceneOpen()
{
    QString path = FileDialog::getOpenFileName(this, "Open scene file", ProjectManager::Instance()->GetDataSourcePath().GetAbsolutePathname().c_str(), "DAVA Scene V2 (*.sc2)");
    OpenScene(path);
}

void QtMainWindow::OnSceneSave()
{
    if (!IsSavingAllowed())
    {
        return;
    }

    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        SaveScene(scene);
    }
}

void QtMainWindow::OnSceneSaveAs()
{
    if (!IsSavingAllowed())
    {
        return;
    }

    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        SaveSceneAs(scene);
    }
}

void QtMainWindow::OnSceneSaveToFolder()
{
    OnSceneSaveAsInternal(false);
}

void QtMainWindow::OnSceneSaveToFolderCompressed()
{
    OnSceneSaveAsInternal(true);
}

void QtMainWindow::OnSceneSaveAsInternal(bool saveWithCompressed)
{
    if (!IsSavingAllowed())
    {
        return;
    }

    SceneEditor2* scene = GetCurrentScene();
    if (nullptr == scene)
    {
        return;
    }

    auto scenePathname = scene->GetScenePath();
    if (scenePathname.IsEmpty() || scenePathname.GetType() == DAVA::FilePath::PATH_IN_MEMORY || !scene->IsLoaded())
    {
        ShowErrorDialog("Can't save not saved scene.");
        return;
    }

    QString path = FileDialog::getExistingDirectory(nullptr, QString("Open Folder"), QString("/"));
    if (path.isEmpty())
    {
        return;
    }

    WaitStart("Save with Children", "Please wait...");

    DAVA::FilePath folder = PathnameToDAVAStyle(path);
    folder.MakeDirectoryPathname();

    SceneSaver sceneSaver;
    sceneSaver.SetInFolder(scene->GetScenePath().GetDirectory());
    sceneSaver.SetOutFolder(folder);
    sceneSaver.EnableCopyConverted(saveWithCompressed);

    LoggerErrorHandler handler;
    DAVA::Logger::AddCustomOutput(&handler);

    SceneEditor2* sceneForSaving = scene->CreateCopyForExport();
    sceneSaver.SaveScene(sceneForSaving, scene->GetScenePath());
    sceneForSaving->Release();
    DAVA::Logger::RemoveCustomOutput(&handler);

    WaitStop();

    if (handler.HasErrors())
    {
        ShowErrorDialog(handler.GetErrors());
    }
}

void QtMainWindow::OnCloseTabRequest(int tabIndex, Request* closeRequest)
{
    SceneEditor2* scene = ui->sceneTabWidget->GetTabScene(tabIndex);
    if (!scene)
    {
        closeRequest->Accept();
        return;
    }

    DAVA::int32 toolsFlags = scene->GetEnabledTools();
    if (!scene->IsChanged())
    {
        if (toolsFlags)
        {
            scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);
        }
        closeRequest->Accept();
        return;
    }

    if (!IsSavingAllowed())
    {
        closeRequest->Cancel();
        return;
    }

    int answer = QMessageBox::warning(nullptr, "Scene was changed", "Do you want to save changes, made to scene?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel)
    {
        closeRequest->Cancel();
        return;
    }

    if (answer == QMessageBox::No)
    {
        if (toolsFlags)
        {
            scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, false);
        }
        closeRequest->Accept();
        return;
    }

    if (toolsFlags)
    {
        DAVA::FilePath colorSystemTexturePath = scene->customColorsSystem->GetCurrentSaveFileName();
        if ((toolsFlags & SceneEditor2::LANDSCAPE_TOOL_CUSTOM_COLOR) &&
            !DAVA::FileSystem::Instance()->Exists(colorSystemTexturePath) && !SelectCustomColorsTexturePath())
        {
            closeRequest->Cancel();
            return;
        }

        scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, true);
    }

    if (!SaveScene(scene))
    {
        closeRequest->Cancel();
        return;
    }

    closeRequest->Accept();
}

void QtMainWindow::ExportMenuTriggered(QAction* exportAsAction)
{
    SceneEditor2* scene = GetCurrentScene();
    if (scene == nullptr || !SaveTilemask(false))
    {
        return;
    }

    WaitStart("Export", "Please wait...");

    DAVA::eGPUFamily gpuFamily = static_cast<DAVA::eGPUFamily>(exportAsAction->data().toInt());
    scene->Export(gpuFamily); // errors will be displayed by logger output

    WaitStop();

    OnReloadTextures(); // need reload textures because they may be re-compressed
}

void QtMainWindow::OnImportSpeedTreeXML()
{
#ifdef __DAVAENGINE_SPEEDTREE__
    SpeedTreeImportDialog importDialog(this);
    importDialog.exec();
#endif //__DAVAENGINE_SPEEDTREE__
}

void QtMainWindow::OnRecentFilesTriggered(QAction* recentAction)
{
    DAVA::String path = recentFiles.GetItem(recentAction);
    if (!path.empty())
    {
        OpenScene(QString::fromStdString(path));
    }
}

void QtMainWindow::OnRecentProjectsTriggered(QAction* recentAction)
{
    DAVA::String path = recentProjects.GetItem(recentAction);
    if (!path.empty())
    {
        OpenProject(path);
    }
}

void QtMainWindow::OnUndo()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->Undo();
    }
}

void QtMainWindow::OnRedo()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->Redo();
    }
}

void QtMainWindow::OnEditorGizmoToggle(bool show)
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
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

    if (nullptr != GetCurrentScene())
    {
        GetCurrentScene()->materialSystem->SetLightmapCanvasVisible(showCanvas);
    }
}

void QtMainWindow::OnAllowOnSceneSelectionToggle(bool allow)
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->selectionSystem->SetSelectionAllowed(allow);
    }
}

void QtMainWindow::OnShowStaticOcclusionToggle(bool show)
{
    DAVA::Renderer::GetOptions()->SetOption(DAVA::RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION, show);
}

void QtMainWindow::OnEnableVisibilitySystemToggle(bool enabled)
{
    if (SetVisibilityToolEnabledIfPossible(enabled))
    {
        SetLandscapeInstancingEnabled(false);
    }
}

void QtMainWindow::OnRefreshVisibilitySystem()
{
    GetCurrentScene()->visibilityCheckSystem->Recalculate();
}

void QtMainWindow::OnFixVisibilityFrame()
{
    GetCurrentScene()->visibilityCheckSystem->FixCurrentFrame();
}

void QtMainWindow::OnReleaseVisibilityFrame()
{
    GetCurrentScene()->visibilityCheckSystem->ReleaseFixedFrame();
}

void QtMainWindow::OnEnableDisableShadows(bool enable)
{
    DAVA::Renderer::GetOptions()->SetOption(DAVA::RenderOptions::SHADOWVOLUME_DRAW, enable);
}

void QtMainWindow::OnReloadTextures()
{
    SetGPUFormat(GetGPUFormat());
}

void QtMainWindow::OnReloadTexturesTriggered(QAction* reloadAction)
{
    DAVA::eGPUFamily gpu = static_cast<DAVA::eGPUFamily>(reloadAction->data().toInt());
    if (gpu >= 0 && gpu < DAVA::GPU_FAMILY_COUNT)
    {
        SetGPUFormat(gpu);
    }
}

void QtMainWindow::OnSelectMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Disabled);
        LoadModificationState(scene);
    }
}

void QtMainWindow::OnMoveMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Translation);
        LoadModificationState(scene);
    }
}

void QtMainWindow::OnRotateMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Rotation);
        LoadModificationState(scene);
    }
}

void QtMainWindow::OnScaleMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->modifSystem->SetTransformType(Selectable::TransformType::Scale);
        LoadModificationState(scene);
    }
}

void QtMainWindow::OnPivotCenterMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->selectionSystem->SetPivotPoint(Selectable::TransformPivot::ObjectCenter);
        LoadModificationState(scene);
    }
}

void QtMainWindow::OnPivotCommonMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->selectionSystem->SetPivotPoint(Selectable::TransformPivot::CommonCenter);
        LoadModificationState(scene);
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
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        DAVA::Entity* landscapeEntity = FindLandscapeEntity(scene);
        if (landscapeEntity == nullptr || GetLandscape(landscapeEntity) == nullptr)
        {
            ShowErrorDialog(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE);
            return;
        }

        scene->modifSystem->PlaceOnLandscape(scene->selectionSystem->GetSelection());
    }
}

void QtMainWindow::OnSnapToLandscape()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        DAVA::Entity* landscapeEntity = FindLandscapeEntity(scene);
        if (landscapeEntity == nullptr || GetLandscape(landscapeEntity) == nullptr)
        {
            ShowErrorDialog(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE);
            ui->actionModifySnapToLandscape->setChecked(false);
            return;
        }

        scene->modifSystem->SetLandscapeSnap(ui->actionModifySnapToLandscape->isChecked());
        LoadModificationState(scene);
    }
}

void QtMainWindow::OnResetTransform()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        scene->modifSystem->ResetTransform(scene->selectionSystem->GetSelection());
    }
}

void QtMainWindow::OnLockTransform()
{
    LockTransform(GetCurrentScene());
    UpdateModificationActionsState();
}

void QtMainWindow::OnUnlockTransform()
{
    UnlockTransform(GetCurrentScene());
    UpdateModificationActionsState();
}

void QtMainWindow::OnCenterPivotPoint()
{
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene)
    {
        curScene->modifSystem->MovePivotCenter(curScene->selectionSystem->GetSelection());
    }
}

void QtMainWindow::OnZeroPivotPoint()
{
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();
    if (nullptr != curScene)
    {
        curScene->modifSystem->MovePivotZero(curScene->selectionSystem->GetSelection());
    }
}

void QtMainWindow::OnMaterialEditor()
{
    MaterialEditor::Instance()->show();
}

void QtMainWindow::OnTextureBrowser()
{
    SceneEditor2* sceneEditor = GetCurrentScene();

    SelectableGroup selectedEntities;
    if (nullptr != sceneEditor)
    {
        selectedEntities.Join(sceneEditor->selectionSystem->GetSelection());
    }

    TextureBrowser::Instance()->show();
    TextureBrowser::Instance()->sceneActivated(sceneEditor);
    TextureBrowser::Instance()->sceneSelectionChanged(sceneEditor, &selectedEntities, nullptr);
}

void QtMainWindow::OnSceneLightMode()
{
    SceneEditor2* scene = GetCurrentScene();
    if (nullptr != scene)
    {
        if (ui->actionEnableCameraLight->isChecked())
        {
            scene->editorLightSystem->SetCameraLightEnabled(true);
        }
        else
        {
            scene->editorLightSystem->SetCameraLightEnabled(false);
        }

        LoadEditorLightState(scene);
    }
}

void QtMainWindow::OnCubemapEditor()
{
    SceneEditor2* scene = GetCurrentScene();

    CubeMapTextureBrowser dlg(scene, dynamic_cast<QWidget*>(parent()));
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
    SceneEditor2* sceneEditor = GetCurrentScene();
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

        sceneEditor->Exec(Command2::Create<EntityAddCommand>(entityToProcess, sceneEditor));
    }
}

void QtMainWindow::OnAddVegetation()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::VegetationRenderObject> vro(new DAVA::VegetationRenderObject());
        DAVA::RenderComponent* rc = new DAVA::RenderComponent();
        rc->SetRenderObject(vro);

        DAVA::ScopedPtr<DAVA::Entity> vegetationNode(new DAVA::Entity());
        vegetationNode->AddComponent(rc);
        vegetationNode->SetName(ResourceEditor::VEGETATION_NODE_NAME);
        vegetationNode->SetLocked(true);

        sceneEditor->Exec(Command2::Create<EntityAddCommand>(vegetationNode, sceneEditor));
    }
}

void QtMainWindow::OnLightDialog()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        sceneNode->AddComponent(new DAVA::LightComponent(DAVA::ScopedPtr<DAVA::Light>(new DAVA::Light)));
        sceneNode->SetName(ResourceEditor::LIGHT_NODE_NAME);
        sceneEditor->Exec(Command2::Create<EntityAddCommand>(sceneNode, sceneEditor));
    }
}

void QtMainWindow::OnCameraDialog()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
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

        sceneEditor->Exec(Command2::Create<EntityAddCommand>(sceneNode, sceneEditor));
    }
}

void QtMainWindow::OnUserNodeDialog()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        sceneNode->AddComponent(new DAVA::UserComponent());
        sceneNode->SetName(ResourceEditor::USER_NODE_NAME);
        sceneEditor->Exec(Command2::Create<EntityAddCommand>(sceneNode, sceneEditor));
    }
}

void QtMainWindow::OnParticleEffectDialog()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        sceneNode->AddComponent(new DAVA::ParticleEffectComponent());
        sceneNode->AddComponent(new DAVA::LodComponent());
        sceneNode->SetName(ResourceEditor::PARTICLE_EFFECT_NODE_NAME);
        sceneEditor->Exec(Command2::Create<EntityAddCommand>(sceneNode, sceneEditor));
    }
}

void QtMainWindow::On2DCameraDialog()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneEditor)
    {
        DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
        DAVA::ScopedPtr<DAVA::Camera> camera(new DAVA::Camera());

        DAVA::float32 w = DAVA::VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dx;
        DAVA::float32 h = DAVA::VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dy;
        DAVA::float32 aspect = w / h;
        camera->SetupOrtho(w, aspect, 1, 1000);
        camera->SetPosition(DAVA::Vector3(0, 0, -10000));
        camera->SetZFar(10000);
        camera->SetTarget(DAVA::Vector3(0, 0, 0));
        camera->SetUp(DAVA::Vector3(0, -1, 0));
        camera->RebuildCameraFromValues();

        sceneNode->AddComponent(new DAVA::CameraComponent(camera));
        sceneNode->SetName("Camera 2D");
        sceneEditor->Exec(Command2::Create<EntityAddCommand>(sceneNode, sceneEditor));
    }
}

void QtMainWindow::On2DSpriteDialog()
{
    DAVA::FilePath projectPath = ProjectManager::Instance()->GetProjectPath();
    projectPath += "Data/Gfx/";

    QString filePath = FileDialog::getOpenFileName(nullptr, QString("Open sprite"), QString::fromStdString(projectPath.GetAbsolutePathname()), QString("Sprite File (*.txt)"));
    if (filePath.isEmpty())
        return;
    filePath.remove(filePath.size() - 4, 4);
    DAVA::Sprite* sprite = DAVA::Sprite::Create(filePath.toStdString());
    if (!sprite)
        return;

    DAVA::Entity* sceneNode = new DAVA::Entity();
    sceneNode->SetName(ResourceEditor::EDITOR_SPRITE);
    DAVA::SpriteObject* spriteObject = new DAVA::SpriteObject(sprite, 0, DAVA::Vector2(1, 1), DAVA::Vector2(0.5f * sprite->GetWidth(), 0.5f * sprite->GetHeight()));
    spriteObject->AddFlag(DAVA::RenderObject::ALWAYS_CLIPPING_VISIBLE);
    sceneNode->AddComponent(new DAVA::RenderComponent(spriteObject));
    DAVA::Matrix4 m = DAVA::Matrix4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, -1, 0,
                                    0, 0, 0, 1);
    sceneNode->SetLocalTransform(m);
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneEditor)
    {
        sceneEditor->Exec(Command2::Create<EntityAddCommand>(sceneNode, sceneEditor));
    }
    SafeRelease(sceneNode);
    SafeRelease(spriteObject);
    SafeRelease(sprite);
}

void QtMainWindow::OnAddEntityFromSceneTree()
{
    ui->menuAdd->exec(QCursor::pos());
}

void QtMainWindow::OnShowSettings()
{
    SettingsDialog t(this);
    t.exec();
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
        ui->actionShowEditorGizmo->setChecked(scene->IsHUDVisible());
        ui->actionOnSceneSelection->setChecked(scene->selectionSystem->IsSelectionAllowed());

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
        if (scene->selectionSystem->GetPivotPoint() == Selectable::TransformPivot::ObjectCenter)
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

void QtMainWindow::SceneUndoRedoStateChanged(SceneEditor2* scene)
{
    if (nullptr != scene)
    {
        ui->actionUndo->setEnabled(scene->CanUndo());
        ui->actionRedo->setEnabled(scene->CanRedo());
    }
}

void QtMainWindow::LoadEditorLightState(SceneEditor2* scene)
{
    if (nullptr != scene)
    {
        ui->actionEnableCameraLight->setChecked(scene->editorLightSystem->GetCameraLightEnabled());
    }
}

void QtMainWindow::LoadGPUFormat()
{
    int curGPU = GetGPUFormat();

    QList<QAction*> allActions = ui->menuTexturesForGPU->actions();
    for (int i = 0; i < allActions.size(); ++i)
    {
        QAction* actionN = allActions[i];

        if (actionN->data().isValid() &&
            actionN->data().toInt() == curGPU)
        {
            actionN->setChecked(true);
            ui->actionReloadTextures->setText(actionN->text());
            break;
        }
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
    if (!IsSavingAllowed())
    {
        return;
    }

    SceneEditor2* scene = GetCurrentScene();

    DAVA::Landscape* landscape = FindLandscape(scene);
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
    SceneEditor2* scene = GetCurrentScene();
    if (!IsSavingAllowed() || (nullptr == scene))
    {
        return;
    }

    LandscapeEditorDrawSystem::eErrorType varifLandscapeError = scene->landscapeEditorDrawSystem->VerifyLandscape();
    if (varifLandscapeError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(varifLandscapeError));
        return;
    }

    DAVA::Landscape* landscape = FindLandscape(scene);
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
        QString selectedPath = FileDialog::getSaveFileName(this, "Save landscape texture as",
                                                           ProjectManager::Instance()->GetDataSourcePath().GetAbsolutePathname().c_str(),
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
    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
    {
        return;
    }

    WaitStart("Conversion of modified textures.", "Checking for modified textures.");
    DAVA::Map<DAVA::Texture*, DAVA::Vector<DAVA::eGPUFamily>> textures;
    int filesToUpdate = SceneHelper::EnumerateModifiedTextures(scene, textures);

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
    SceneEditor2* scene = GetCurrentScene();
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
                ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
                return;
            }
        }
        else
        {
            return;
        }
    }

    if (!scene->IsLoaded() || scene->IsChanged())
    {
        if (!SaveScene(scene))
            return;
    }

    BeastDialog dlg(this);
    dlg.SetScene(scene);
    const bool run = dlg.Exec();
    if (!run)
        return;

    if (!SaveTilemask(false))
    {
        return;
    }

    RunBeast(dlg.GetPath(), dlg.GetMode());
    scene->SetChanged(true);
    SaveScene(scene);

    scene->ClearAllCommands();
}

void QtMainWindow::RunBeast(const QString& outputPath, BeastProxy::eBeastMode mode)
{
#if defined(__DAVAENGINE_BEAST__)

    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
        return;

    const DAVA::FilePath path = outputPath.toStdString();
    scene->Exec(Command2::Create<BeastAction>(scene, path, mode, beastWaitDialog));

    if (mode == BeastProxy::MODE_LIGHTMAPS)
    {
        OnReloadTextures();
    }

#endif //#if defined (__DAVAENGINE_BEAST__)
}

void QtMainWindow::BeastWaitSetMessage(const QString& messsage)
{
    beastWaitDialog->SetMessage(messsage);
}

bool QtMainWindow::BeastWaitCanceled()
{
    return beastWaitDialog->WasCanceled();
}

void QtMainWindow::OnLandscapeEditorToggled(SceneEditor2* scene)
{
    if (scene != GetCurrentScene())
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

    UpdateConflictingActionsState(tools == 0);

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
    SetLandscapeInstancingEnabled(anyEditorEnabled);

    ui->actionForceFirstLODonLandscape->setChecked(anyEditorEnabled);
    OnForceFirstLod(anyEditorEnabled);
}

void QtMainWindow::OnCustomColorsEditor()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (!sceneEditor->customColorsSystem->IsLandscapeEditingEnabled())
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            ShowErrorDialog("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor);
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(Command2::Create<EnableCustomColorsCommand>(sceneEditor, true));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor);
        }
        return;
    }

    if (sceneEditor->customColorsSystem->ChangesPresent())
    {
        DAVA::FilePath currentTexturePath = sceneEditor->customColorsSystem->GetCurrentSaveFileName();

        if (!DAVA::FileSystem::Instance()->Exists(currentTexturePath) && !SelectCustomColorsTexturePath())
        {
            ui->actionCustomColorsEditor->setChecked(true);
            return;
        }
    }

    sceneEditor->Exec(Command2::Create<DisableCustomColorsCommand>(sceneEditor, true));
    ui->actionCustomColorsEditor->setChecked(false);
}

bool QtMainWindow::SelectCustomColorsTexturePath()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return false;
    }
    DAVA::FilePath scenePath = sceneEditor->GetScenePath().GetDirectory();

    QString filePath = FileDialog::getSaveFileName(nullptr,
                                                   QString(ResourceEditor::CUSTOM_COLORS_SAVE_CAPTION.c_str()),
                                                   QString(scenePath.GetAbsolutePathname().c_str()),
                                                   PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);
    DAVA::FilePath selectedPathname = PathnameToDAVAStyle(filePath);
    DAVA::Entity* landscape = FindLandscapeEntity(sceneEditor);
    if (selectedPathname.IsEmpty() || nullptr == landscape)
    {
        return false;
    }

    DAVA::KeyedArchive* customProps = GetOrCreateCustomProperties(landscape)->GetArchive();
    if (nullptr == customProps)
    {
        return false;
    }

    DAVA::String pathToSave = selectedPathname.GetRelativePathname(ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname());
    customProps->SetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, pathToSave);

    return true;
}

void QtMainWindow::OnHeightmapEditor()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->heightmapEditorSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->Exec(Command2::Create<DisableHeightmapEditorCommand>(sceneEditor));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            ShowErrorDialog("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor);
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(Command2::Create<EnableHeightmapEditorCommand>(sceneEditor));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor);
        }
    }
}

void QtMainWindow::OnRulerTool()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->Exec(Command2::Create<DisableRulerToolCommand>(sceneEditor));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            ShowErrorDialog("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor);
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(Command2::Create<EnableRulerToolCommand>(sceneEditor));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor);
        }
    }
}

void QtMainWindow::OnTilemaskEditor()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->Exec(Command2::Create<DisableTilemaskEditorCommand>(sceneEditor));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            ShowErrorDialog("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor);
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(Command2::Create<EnableTilemaskEditorCommand>(sceneEditor));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor);
        }
    }
}

void QtMainWindow::OnForceFirstLod(bool enabled)
{
    auto scene = GetCurrentScene();
    if (scene == nullptr)
    {
        ui->actionForceFirstLODonLandscape->setChecked(false);
        return;
    }

    auto landscape = FindLandscape(scene);
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
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    if (sceneEditor->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled())
    {
        sceneEditor->Exec(Command2::Create<DisableNotPassableCommand>(sceneEditor));
    }
    else
    {
        if (sceneEditor->pathSystem->IsPathEditEnabled())
        {
            ShowErrorDialog("WayEditor should be disabled prior to enabling landscape tools");
            OnLandscapeEditorToggled(sceneEditor);
            return;
        }

        if (LoadAppropriateTextureFormat())
        {
            sceneEditor->Exec(Command2::Create<EnableNotPassableCommand>(sceneEditor));
        }
        else
        {
            OnLandscapeEditorToggled(sceneEditor);
        }
    }
}

void QtMainWindow::OnWayEditor()
{
    SceneEditor2* sceneEditor = GetCurrentScene();
    if (!sceneEditor)
    {
        return;
    }

    bool toEnable = !sceneEditor->pathSystem->IsPathEditEnabled();
    DVASSERT(toEnable == ui->actionWayEditor->isChecked());

    DAVA::int32 toolsEnabled = sceneEditor->GetEnabledTools();
    if (toEnable && toolsEnabled)
    {
        ShowErrorDialog("Landscape tools should be disabled prior to enabling WayEditor");
        ui->actionWayEditor->setChecked(false);
        return;
    }

    auto isLocked = sceneEditor->selectionSystem->IsLocked();
    sceneEditor->selectionSystem->SetLocked(true);
    sceneEditor->pathSystem->EnablePathEdit(toEnable);
    sceneEditor->selectionSystem->SetLocked(isLocked);
}

void QtMainWindow::OnBuildStaticOcclusion()
{
    SceneEditor2* scene = GetCurrentScene();
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
        ui->propertyEditor->ResetProperties();
    }

    delete waitOcclusionDlg;
}

void QtMainWindow::OnInavalidateStaticOcclusion()
{
    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
        return;
    scene->staticOcclusionSystem->InvalidateOcclusion();
    scene->MarkAsChanged();
}

bool QtMainWindow::IsSavingAllowed()
{
    SceneEditor2* scene = GetCurrentScene();

    if (!scene || scene->GetEnabledTools() != 0)
    {
        QMessageBox::warning(this, "Saving is not allowed", "Disable landscape editing before save!");
        return false;
    }

    if (!scene || scene->wayEditSystem->IsWayEditEnabled())
    {
        QMessageBox::warning(this, "Saving is not allowed", "Disable path editing before save!");
        return false;
    }

    return true;
}

void QtMainWindow::OnObjectsTypeChanged(QAction* action)
{
    SceneEditor2* scene = GetCurrentScene();
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
    SceneEditor2* scene = GetCurrentScene();
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

bool QtMainWindow::OpenScene(const QString& path)
{
    bool ret = false;

    if (!path.isEmpty())
    {
        DAVA::FilePath projectPath(ProjectManager::Instance()->GetProjectPath());
        DAVA::FilePath argumentPath(path.toStdString());

        if (!DAVA::FilePath::ContainPath(argumentPath, projectPath))
        {
            QMessageBox::warning(this, "Open scene error.", QString().sprintf("Can't open scene file outside project path.\n\nScene:\n%s\n\nProject:\n%s",
                                                                              projectPath.GetAbsolutePathname().c_str(),
                                                                              argumentPath.GetAbsolutePathname().c_str()));
        }
        else
        {
            int needCloseIndex = -1;
            SceneEditor2* scene = ui->sceneTabWidget->GetCurrentScene();
            if (scene && (ui->sceneTabWidget->GetTabCount() == 1))
            {
                DAVA::FilePath path = scene->GetScenePath();
                if (path.GetFilename() == "newscene1.sc2" && !scene->CanUndo() && !scene->IsLoaded())
                {
                    needCloseIndex = 0;
                }
            }

            DAVA::FilePath scenePath = DAVA::FilePath(path.toStdString());

            int index = ui->sceneTabWidget->OpenTab(scenePath);

            if (index != -1)
            {
                recentFiles.Add(path.toStdString());

                // close empty default scene
                if (-1 != needCloseIndex)
                {
                    ui->sceneTabWidget->CloseTab(needCloseIndex);
                }

                ret = true;
            }
        }
    }

    return ret;
}

void QtMainWindow::OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape)
{
    if (GetCurrentScene() != scene)
    {
        return;
    }

    ui->actionModifySnapToLandscape->setChecked(isSpanToLandscape);
}

bool QtMainWindow::ShouldClose(QCloseEvent* e)
{
    if (IsAnySceneChanged() == false)
        return true;

    int answer = QMessageBox::question(this, "Scene was changed", "Do you want to quit anyway?",
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    return (answer == QMessageBox::Yes);
}

bool QtMainWindow::IsAnySceneChanged()
{
    int count = ui->sceneTabWidget->GetTabCount();
    for (int i = 0; i < count; ++i)
    {
        SceneEditor2* scene = ui->sceneTabWidget->GetTabScene(i);
        if (scene && scene->IsChanged())
        {
            return true;
        }
    }

    return false;
}

void QtMainWindow::OnHangingObjects()
{
    SceneEditor2* scene = GetCurrentScene();
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

    if (nullptr != GetCurrentScene())
    {
        GetCurrentScene()->materialSystem->SetLightViewMode(newMode);
    }
}

void QtMainWindow::OnCustomQuality()
{
    QualitySwitcher::ShowDialog();
}

void QtMainWindow::UpdateConflictingActionsState(bool enable)
{
    ui->menuTexturesForGPU->setEnabled(enable);
    ui->actionReloadTextures->setEnabled(enable);
    ui->menuExport->setEnabled(enable);
    ui->actionOnlyOriginalTextures->setEnabled(enable);
    ui->actionWithCompressedTextures->setEnabled(enable);
}

void QtMainWindow::DiableUIForFutureUsing()
{
    //TODO: temporary disabled
    //-->
    //ui->actionAddNewComponent->setVisible(false);
    //ui->actionRemoveComponent->setVisible(false);
    //<--
}

void QtMainWindow::SynchronizeStateWithUI()
{
    OnManualModifMode();
}

void QtMainWindow::OnEmptyEntity()
{
    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
        return;

    DAVA::ScopedPtr<DAVA::Entity> newEntity(new DAVA::Entity());
    newEntity->SetName(ResourceEditor::ENTITY_NAME);

    scene->Exec(Command2::Create<EntityAddCommand>(newEntity, scene));
}

void QtMainWindow::OnAddWindEntity()
{
    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
        return;

    DAVA::ScopedPtr<DAVA::Entity> windEntity(new DAVA::Entity());
    windEntity->SetName(ResourceEditor::WIND_NODE_NAME);

    DAVA::Matrix4 ltMx = DAVA::Matrix4::MakeTranslation(DAVA::Vector3(0.f, 0.f, 20.f));
    GetTransformComponent(windEntity)->SetLocalTransform(&ltMx);

    windEntity->AddComponent(new DAVA::WindComponent());

    scene->Exec(Command2::Create<EntityAddCommand>(windEntity, scene));
}

void QtMainWindow::OnAddPathEntity()
{
    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
        return;

    DAVA::ScopedPtr<DAVA::Entity> pathEntity(new DAVA::Entity());
    pathEntity->SetName(ResourceEditor::PATH_NODE_NAME);
    DAVA::PathComponent* pc = scene->pathSystem->CreatePathComponent();

    pathEntity->AddComponent(pc);
    scene->Exec(Command2::Create<EntityAddCommand>(pathEntity, scene));
}

bool QtMainWindow::LoadAppropriateTextureFormat()
{
    if (GetGPUFormat() != DAVA::GPU_ORIGIN)
    {
        int answer = ShowQuestion("Inappropriate texture format",
                                  "Landscape editing is only allowed in original texture format.\nDo you want to reload textures in original format?",
                                  MB_FLAG_YES | MB_FLAG_NO, MB_FLAG_NO);
        if (answer == MB_FLAG_NO)
        {
            return false;
        }

        OnReloadTexturesTriggered(ui->actionReloadPNG);
    }

    return (GetGPUFormat() == DAVA::GPU_ORIGIN);
}

bool QtMainWindow::SaveTilemask(bool forAllTabs /* = true */)
{
    SceneTabWidget* sceneWidget = GetSceneWidget();

    int lastSceneTab = sceneWidget->GetCurrentTab();
    int answer = QMessageBox::Cancel;
    bool needQuestion = true;

    // tabs range where tilemask should be saved
    DAVA::int32 firstTab = forAllTabs ? 0 : sceneWidget->GetCurrentTab();
    DAVA::int32 lastTab = forAllTabs ? sceneWidget->GetTabCount() : sceneWidget->GetCurrentTab() + 1;

    for (int i = firstTab; i < lastTab; ++i)
    {
        SceneEditor2* tabEditor = sceneWidget->GetTabScene(i);
        if (nullptr != tabEditor)
        {
            const CommandStack* cmdStack = tabEditor->GetCommandStack();
            if (cmdStack->IsUncleanCommandExists(CMDID_TILEMASK_MODIFY))
            {
                // ask user about saving tilemask changes
                sceneWidget->SetCurrentTab(i);

                if (needQuestion)
                {
                    QString message = tabEditor->GetScenePath().GetFilename().c_str();
                    message += " has unsaved tilemask changes.\nDo you want to save?";

                    // if more than one scene to precess
                    if ((lastTab - firstTab) > 1)
                    {
                        answer = QMessageBox::warning(this, "", message, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel | QMessageBox::YesToAll | QMessageBox::NoToAll, QMessageBox::Cancel);
                    }
                    else
                    {
                        answer = QMessageBox::warning(this, "", message, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
                    }
                }

                switch (answer)
                {
                case QMessageBox::YesAll:
                    needQuestion = false;
                case QMessageBox::Yes:
                {
                    // turn off editor
                    tabEditor->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);

                    // save
                    tabEditor->landscapeEditorDrawSystem->SaveTileMaskTexture();
                }
                break;

                case QMessageBox::NoAll:
                    needQuestion = false;
                case QMessageBox::No:
                {
                    // turn off editor
                    tabEditor->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);
                }
                break;

                case QMessageBox::Cancel:
                default:
                {
                    // cancel save process
                    return false;
                }
                break;
                }
            }

            //reset tilemask
            tabEditor->landscapeEditorDrawSystem->ResetTileMaskTexture();

            // clear all tilemask commands in commandStack because they will be
            // invalid after tilemask reloading
            tabEditor->RemoveCommands(CMDID_TILEMASK_MODIFY);
        }
    }

    sceneWidget->SetCurrentTab(lastSceneTab);

    return true;
}

void QtMainWindow::OnReloadShaders()
{
    DAVA::ShaderDescriptorCache::RelaoadShaders();

    SceneTabWidget* tabWidget = QtMainWindow::Instance()->GetSceneWidget();
    for (int tab = 0, sz = tabWidget->GetTabCount(); tab < sz; ++tab)
    {
        SceneEditor2* sceneEditor = tabWidget->GetTabScene(tab);

        const DAVA::Set<DAVA::NMaterial*>& topParents = sceneEditor->materialSystem->GetTopParents();

        for (auto material : topParents)
        {
            material->InvalidateRenderVariants();
        }
        const DAVA::Map<DAVA::uint64, DAVA::NMaterial*>& particleInstances = sceneEditor->particleEffectSystem->GetMaterialInstances();
        for (auto material : particleInstances)
        {
            material.second->InvalidateRenderVariants();
        }

        DAVA::Set<DAVA::NMaterial*> materialList;
        sceneEditor->foliageSystem->CollectFoliageMaterials(materialList);
        for (auto material : materialList)
        {
            if (material)
                material->InvalidateRenderVariants();
        }

        sceneEditor->renderSystem->GetDebugDrawer()->InvalidateMaterials();
        sceneEditor->renderSystem->SetForceUpdateLights();

        sceneEditor->visibilityCheckSystem->InvalidateMaterials();
    }

#define INVALIDATE_2D_MATERIAL(material) \
    if (DAVA::RenderSystem2D::material) \
    { \
        DAVA::RenderSystem2D::material->InvalidateRenderVariants(); \
        DAVA::RenderSystem2D::material->PreBuildMaterial(DAVA::RenderSystem2D::RENDER_PASS_NAME); \
    }

    INVALIDATE_2D_MATERIAL(DEFAULT_2D_COLOR_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_FILL_ALPHA_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL)

#undef INVALIDATE_2D_MATERIAL
}

void QtMainWindow::OnSwitchWithDifferentLODs(bool checked)
{
    SceneEditor2* scene = GetCurrentScene();
    if (!scene)
        return;

    scene->debugDrawSystem->EnableSwithcesWithDifferentLODsMode(checked);

    if (checked)
    {
        DAVA::Set<DAVA::FastName> entitiNames;
        SceneValidator::FindSwitchesWithDifferentLODs(scene, entitiNames);

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

void QtMainWindow::DebugColorPicker()
{
    ColorPicker* cp = new ColorPicker(this);

    cp->Exec();
}

void QtMainWindow::DebugDeviceList()
{
    // Create controller and window if they are not exist
    // Pointer deviceListController automatically becomes nullptr on window destruction
    if (nullptr == deviceListController)
    {
        DeviceListWidget* w = new DeviceListWidget(this);
        w->setAttribute(Qt::WA_DeleteOnClose);

        deviceListController = new DeviceListController(w);
        deviceListController->SetView(w);
    }
    deviceListController->ShowView();
}

void QtMainWindow::OnConsoleItemClicked(const QString& data)
{
    PointerSerializer conv(data.toStdString());
    if (conv.CanConvert<DAVA::Entity*>())
    {
        auto currentScene = GetCurrentScene();
        if (nullptr != currentScene)
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
                        objects.Add(entity, currentScene->selectionSystem->GetUntransformedBoundingBox(entity));
                    }
                }

                if (!objects.IsEmpty())
                {
                    currentScene->selectionSystem->SetSelection(objects);
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
    w->SetOutputTemplate("h_", QString());

    w->show();
}

void QtMainWindow::OnBatchProcessScene()
{
    SceneProcessor sceneProcessor;

    // For client developers: need to set entity processor derived from EntityProcessorBase
    //DestructibleSoundAdder *entityProcessor = new DestructibleSoundAdder();
    //sceneProcessor.SetEntityProcessor(entityProcessor);
    //SafeRelease(entityProcessor);

    SceneEditor2* sceneEditor = GetCurrentScene();
    if (sceneProcessor.Execute(sceneEditor))
    {
        SaveScene(sceneEditor);
    }
}

void QtMainWindow::OnSnapCameraToLandscape(bool snap)
{
    SceneEditor2* scene = GetCurrentScene();
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

void QtMainWindow::RestartParticleEffects()
{
    const SceneTabWidget* widget = GetSceneWidget();
    for (int tab = 0; tab < widget->GetTabCount(); ++tab)
    {
        SceneEditor2* scene = widget->GetTabScene(tab);
        DVASSERT(scene);
        scene->particlesSystem->RestartParticleEffects();
    }
}

void QtMainWindow::RemoveSelection()
{
    ::RemoveSelection(GetCurrentScene());
}

bool QtMainWindow::SetVisibilityToolEnabledIfPossible(bool enabled)
{
    SceneEditor2* scene = GetCurrentScene();
    DAVA::int32 enabledTools = scene->GetEnabledTools();
    if (enabled && (enabledTools != 0))
    {
        ShowErrorDialog("Please disable Landscape editing tools before enabling Visibility Check System");
        enabled = false;
    }

    DAVA::Renderer::GetOptions()->SetOption(DAVA::RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM, enabled);

    if (enabled)
    {
        ui->actionForceFirstLODonLandscape->setChecked(true);
        OnForceFirstLod(true);
    }

    ui->actionEnableVisibilitySystem->setChecked(enabled);
    return enabled;
}

void QtMainWindow::SetLandscapeInstancingEnabled(bool enabled)
{
    DAVA::Landscape* landscape = FindLandscape(GetCurrentScene());

    if (landscape == nullptr)
        return;

    landscape->SetRenderMode(enabled ?
                             DAVA::Landscape::RenderMode::RENDERMODE_INSTANCING_MORPHING :
                             DAVA::Landscape::RenderMode::RENDERMODE_NO_INSTANCING);
}

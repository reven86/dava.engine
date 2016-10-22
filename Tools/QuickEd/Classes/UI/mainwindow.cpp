#include "mainwindow.h"
#include "Project/Project.h"
#include "Document/Document.h"
#include "Document/DocumentGroup.h"
#include "Render/Texture.h"

#include "Helpers/ResourcesManageHelper.h"

#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "Utils/QtDavaConvertion.h"
#include "QtTools/Utils/Utils.h"

#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "Preferences/PreferencesDialog.h"

#include "DebugTools/DebugTools.h"
#include "QtTools/Utils/Themes/Themes.h"

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(MainWindow,
                              PREF_ARG("isPixelized", false),
                              PREF_ARG("state", String()),
                              PREF_ARG("geometry", String()),
                              PREF_ARG("consoleState", String())
                              )

Q_DECLARE_METATYPE(const InspMember*);

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , loggerOutput(new LoggerOutputObject)
{
    setupUi(this);

    connect(loggerOutput, &LoggerOutputObject::OutputReady, this, &MainWindow::OnLogOutput, Qt::DirectConnection);

    DebugTools::ConnectToUI(this);

    // Reload Sprites
    menuTools->addAction(actionReloadSprites);
    toolBarPlugins->addAction(actionReloadSprites);

    toolBarPlugins->addSeparator();
    InitLanguageBox();
    toolBarPlugins->addSeparator();
    InitGlobalClasses();
    toolBarPlugins->addSeparator();
    InitRtlBox();
    toolBarPlugins->addSeparator();
    InitBiDiSupportBox();
    toolBarPlugins->addSeparator();
    InitEmulationMode();

    tabBar->setElideMode(Qt::ElideNone);
    setWindowTitle(ResourcesManageHelper::GetProjectTitle());

    tabBar->setTabsClosable(true);
    tabBar->setUsesScrollButtons(true);
    setUnifiedTitleAndToolBarOnMac(true);

    connect(fileSystemDockWidget, &FileSystemDockWidget::OpenPackageFile, this, &MainWindow::OpenPackageFile);
    connect(previewWidget, &PreviewWidget::OpenPackageFile, this, &MainWindow::OpenPackageFile);

    InitMenu();

    menuTools->setEnabled(false);
    toolBarPlugins->setEnabled(false);

    OnDocumentChanged(nullptr);

    PreferencesStorage::Instance()->RegisterPreferences(this);
}

MainWindow::~MainWindow()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void MainWindow::AttachDocumentGroup(DocumentGroup* documentGroup)
{
    Q_ASSERT(documentGroup != nullptr);

    documentGroup->ConnectToTabBar(tabBar);

    documentGroup->AttachRedoAction(actionRedo);
    documentGroup->AttachUndoAction(actionUndo);
    actionRedo->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_Y << Qt::CTRL + Qt::SHIFT + Qt::Key_Z); //Qt can not set multishortcut or enum shortcut in Qt designer
    Q_ASSERT(documentGroup != nullptr);
    documentGroup->AttachSaveAction(actionSaveDocument);
    documentGroup->AttachSaveAllAction(actionForceSaveAllDocuments);

    QAction* actionCloseDocument = new QAction("Close current document", this);
    actionCloseDocument->setShortcut(static_cast<int>(Qt::ControlModifier | Qt::Key_W));
    actionCloseDocument->setShortcutContext(Qt::WindowShortcut);
    documentGroup->AttachCloseDocumentAction(actionCloseDocument);
    previewWidget->GetGLWidget()->addAction(actionCloseDocument);

    QAction* actionReloadDocument = new QAction("Reload current document", this);
    QList<QKeySequence> shortcurs;
    shortcurs << static_cast<int>(Qt::ControlModifier | Qt::Key_R)
              << Qt::Key_F5;
    actionReloadDocument->setShortcuts(shortcurs);
    actionReloadDocument->setShortcutContext(Qt::WindowShortcut);
    documentGroup->AttachReloadDocumentAction(actionReloadDocument);
    previewWidget->GetGLWidget()->addAction(actionReloadDocument);
}

void MainWindow::OnDocumentChanged(Document* document)
{
    bool enabled = (document != nullptr);
    packageWidget->setEnabled(enabled);
    propertiesWidget->setEnabled(enabled);
    libraryWidget->setEnabled(enabled);
}

QComboBox* MainWindow::GetComboBoxLanguage()
{
    return comboboxLanguage;
}

bool MainWindow::IsInEmulationMode() const
{
    return emulationBox->isChecked();
}

void MainWindow::ExecDialogReloadSprites(SpritesPacker* packer)
{
    DVASSERT(nullptr != packer);
    auto lastFlags = acceptableLoggerFlags;
    acceptableLoggerFlags = (1 << Logger::LEVEL_ERROR) | (1 << Logger::LEVEL_WARNING);
    DialogReloadSprites dialogReloadSprites(packer, this);
    dialogReloadSprites.exec();
    acceptableLoggerFlags = lastFlags;
}

void MainWindow::OnShowHelp()
{
    FilePath docsPath = ResourcesManageHelper::GetDocumentationPath().toStdString() + "index.html";
    QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
    QDesktopServices::openUrl(QUrl(docsFile));
}

void MainWindow::InitLanguageBox()
{
    comboboxLanguage = new QComboBox();
    comboboxLanguage->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QLabel* label = new QLabel(tr("language"));
    label->setBuddy(comboboxLanguage);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(label);
    layout->addWidget(comboboxLanguage);
    QWidget* wrapper = new QWidget();
    wrapper->setLayout(layout);
    toolBarPlugins->addWidget(wrapper);
}

void MainWindow::FillComboboxLanguages(const Project* project)
{
    QString currentText = project->GetEditorLocalizationSystem()->GetCurrentLocale();
    bool wasBlocked = comboboxLanguage->blockSignals(true); //performance fix
    comboboxLanguage->clear();
    comboboxLanguage->addItems(project->GetEditorLocalizationSystem()->GetAvailableLocaleNames());
    comboboxLanguage->setCurrentText(currentText);
    comboboxLanguage->blockSignals(wasBlocked);
}

void MainWindow::InitRtlBox()
{
    QCheckBox* rtlBox = new QCheckBox(tr("Right-to-left"));
    rtlBox->setLayoutDirection(Qt::RightToLeft);
    toolBarPlugins->addWidget(rtlBox);
    connect(rtlBox, &QCheckBox::stateChanged, this, &MainWindow::OnRtlChanged);
}

void MainWindow::InitBiDiSupportBox()
{
    QCheckBox* bidiSupportBox = new QCheckBox(tr("BiDi Support"));
    bidiSupportBox->setLayoutDirection(Qt::RightToLeft);
    toolBarPlugins->addWidget(bidiSupportBox);
    connect(bidiSupportBox, &QCheckBox::stateChanged, this, &MainWindow::OnBiDiSupportChanged);
}

void MainWindow::InitGlobalClasses()
{
    QLineEdit* classesEdit = new QLineEdit();
    classesEdit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    QLabel* label = new QLabel(tr("global classes"));
    label->setBuddy(classesEdit);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(label);
    layout->addWidget(classesEdit);
    QWidget* wrapper = new QWidget();
    wrapper->setLayout(layout);
    toolBarPlugins->addWidget(wrapper);
    connect(classesEdit, &QLineEdit::textChanged, this, &MainWindow::OnGlobalClassesChanged);
}

void MainWindow::InitEmulationMode()
{
    emulationBox = new QCheckBox("Emulation", this);
    emulationBox->setLayoutDirection(Qt::RightToLeft);
    connect(emulationBox, &QCheckBox::toggled, this, &MainWindow::EmulationModeChanged);
    toolBarPlugins->addWidget(emulationBox);
}

void MainWindow::InitMenu()
{
    SetupViewMenu();

    connect(actionOpen_project, &QAction::triggered, this, &MainWindow::OnOpenProjectAction);
    connect(actionClose_project, &QAction::triggered, this, &MainWindow::CloseProject);

    connect(actionExit, &QAction::triggered, this, &MainWindow::ActionExitTriggered);
    connect(menuRecent, &QMenu::triggered, this, &MainWindow::RecentMenuTriggered);

    connect(actionZoomOut, &QAction::triggered, previewWidget, &PreviewWidget::OnDecrementScale);
    connect(actionZoomIn, &QAction::triggered, previewWidget, &PreviewWidget::OnIncrementScale);
    connect(actionActualZoom, &QAction::triggered, previewWidget, &PreviewWidget::SetActualScale);

// Remap zoom in/out shorcuts for windows platform
#if defined(__DAVAENGINE_WIN32__)
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Equal));
    shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Plus));
    actionZoomIn->setShortcuts(shortcuts);
#endif

    //Help contents dialog
    connect(actionHelp, &QAction::triggered, this, &MainWindow::OnShowHelp);

    // Pixelization.
    connect(actionPixelized, &QAction::triggered, this, &MainWindow::OnPixelizationStateChanged);

    connect(action_preferences, &QAction::triggered, this, &MainWindow::OnEditorPreferencesTriggered);
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    menuView->addAction(propertiesWidget->toggleViewAction());
    menuView->addAction(fileSystemDockWidget->toggleViewAction());
    menuView->addAction(packageWidget->toggleViewAction());
    menuView->addAction(libraryWidget->toggleViewAction());
    menuView->addAction(consoleDockWidget->toggleViewAction());

    menuView->addSeparator();
    menuView->addAction(mainToolbar->toggleViewAction());

    QMenu* appStyleMenu = new QMenu(tr("Application style"), menuView);
    menuView->addMenu(appStyleMenu);
    QActionGroup* actionGroup = new QActionGroup(this);
    for (const QString& theme : Themes::ThemesNames())
    {
        QAction* action = new QAction(theme, menuView);
        actionGroup->addAction(action);
        action->setCheckable(true);
        if (theme == Themes::GetCurrentThemeStr())
        {
            action->setChecked(true);
        }
        appStyleMenu->addAction(action);
    }
    connect(actionGroup, &QActionGroup::triggered, [](QAction* action) {
        if (action->isChecked())
        {
            Themes::SetCurrentTheme(action->text());
        }
    });
    SetupBackgroundMenu();
    // Another actions below the Set Background Color.
    menuView->addSeparator();
    menuView->addAction(actionZoomIn);
    menuView->addAction(actionZoomOut);
}

void MainWindow::SetupBackgroundMenu()
{
    const InspInfo* inspInfo = PreferencesStorage::Instance()->GetInspInfo(FastName("ColorControl"));

    backgroundIndexMember = inspInfo->Member(FastName("backgroundColorIndex"));
    DVASSERT(backgroundIndexMember != nullptr);
    if (backgroundIndexMember == nullptr)
    {
        return;
    }

    uint32 currentIndex = PreferencesStorage::Instance()->GetValue(backgroundIndexMember).AsUInt32();

    PreferencesStorage::Instance()->valueChanged.Connect(this, &MainWindow::OnPreferencesPropertyChanged);

    menuView->addSeparator();
    // Setup the Background Color menu.
    QMenu* backgroundColorMenu = new QMenu("Grid Color", this);
    menuView->addSeparator();
    menuView->addMenu(backgroundColorMenu);

    backgroundActions = new QActionGroup(this);
    for (int i = 0, count = inspInfo->MembersCount(), index = 0; i < count; ++i)
    {
        const InspMember* member = inspInfo->Member(i);
        backgroundColorMembers.insert(member);
        QString str(member->Name().c_str());
        if (str.contains(QRegExp("backgroundColor\\d+")))
        {
            QAction* colorAction = new QAction(QString("Background color %1").arg(index), backgroundColorMenu);
            backgroundActions->addAction(colorAction);
            colorAction->setCheckable(true);
            colorAction->setData(QVariant::fromValue<const InspMember*>(member));
            if (index == currentIndex)
            {
                colorAction->setChecked(true);
            }
            backgroundColorMenu->addAction(colorAction);
            QColor color = ColorToQColor(PreferencesStorage::Instance()->GetValue(member).AsColor());
            colorAction->setIcon(CreateIconFromColor(color));
            connect(colorAction, &QAction::toggled, [this, index](bool toggled)
                    {
                        if (toggled)
                        {
                            VariantType value(static_cast<uint32>(index));
                            PreferencesStorage::Instance()->SetValue(backgroundIndexMember, value);
                        }
                    });
            ++index;
        }
    }
}

void MainWindow::RebuildRecentMenu(const QStringList& lastProjectsPathes)
{
    menuRecent->clear();
    for (auto& projectPath : lastProjectsPathes)
    {
        QAction* recentProject = new QAction(projectPath, this);
        recentProject->setData(projectPath);
        menuRecent->addAction(recentProject);
    }
    menuRecent->setEnabled(!lastProjectsPathes.isEmpty());
}

void MainWindow::OnProjectOpened(const ResultList& resultList, const Project* project)
{
    menuTools->setEnabled(resultList);
    toolBarPlugins->setEnabled(resultList);
    currentProjectPath = project->GetProjectPath() + project->GetProjectName();
    if (resultList)
    {
        UpdateProjectSettings();

        RebuildRecentMenu(project->GetProjectsHistory());
        FillComboboxLanguages(project);
        this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());
    }
    else
    {
        QStringList errors;
        for (const auto& result : resultList.GetResults())
        {
            errors << QString::fromStdString(result.message);
        }
        QMessageBox::warning(qApp->activeWindow(), tr("Error while loading project"), errors.join('\n'));
        this->setWindowTitle("QuickEd");
    }
}

void MainWindow::OnOpenProjectAction()
{
    QString defaultPath = currentProjectPath;
    if (defaultPath.isNull() || defaultPath.isEmpty())
    {
        defaultPath = QDir::currentPath();
    }

    QString projectPath = FileDialog::getOpenFileName(this, tr("Select a project file"),
                                                      defaultPath,
                                                      tr("Project (*.uieditor)"));
    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);

    emit ActionOpenProjectTriggered(projectPath);
}

void MainWindow::UpdateProjectSettings()
{
    // Save to settings default project directory
    QFileInfo fileInfo(currentProjectPath);
    QString projectDir = fileInfo.absoluteDir().absolutePath();

    // Update window title
    this->setWindowTitle(ResourcesManageHelper::GetProjectTitle(currentProjectPath));
}

void MainWindow::OnPreferencesPropertyChanged(const InspMember* member, const VariantType& value)
{
    QList<QAction*> actions = backgroundActions->actions();
    if (member == backgroundIndexMember)
    {
        uint32 index = value.AsUInt32();
        DVASSERT(static_cast<int>(index) < actions.size());
        actions.at(index)->setChecked(true);
        return;
    }
    auto iter = backgroundColorMembers.find(member);
    if (iter != backgroundColorMembers.end())
    {
        for (QAction* action : actions)
        {
            if (action->data().value<const InspMember*>() == member)
            {
                QColor color = ColorToQColor(value.AsColor());
                action->setIcon(CreateIconFromColor(color));
            }
        }
    }
}

void MainWindow::OnPixelizationStateChanged(bool isPixelized)
{
    Texture::SetPixelization(isPixelized);
}

void MainWindow::OnRtlChanged(int arg)
{
    emit RtlChanged(arg == Qt::Checked);
}

void MainWindow::OnBiDiSupportChanged(int arg)
{
    emit BiDiSupportChanged(arg == Qt::Checked);
}

void MainWindow::OnGlobalClassesChanged(const QString& str)
{
    emit GlobalStyleClassesChanged(str);
}

void MainWindow::OnLogOutput(Logger::eLogLevel logLevel, const QByteArray& output)
{
    if (static_cast<int32>(1 << logLevel) & acceptableLoggerFlags)
    {
        logWidget->AddMessage(logLevel, output);
    }
}

void MainWindow::OnEditorPreferencesTriggered()
{
    PreferencesDialog dialog(this);
    dialog.exec();
}

bool MainWindow::IsPixelized() const
{
    return actionPixelized->isChecked();
}

void MainWindow::SetPixelized(bool pixelized)
{
    actionPixelized->setChecked(pixelized);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (CanClose())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

String MainWindow::GetState() const
{
    QByteArray state = saveState().toBase64();
    return state.toStdString();
}

void MainWindow::SetState(const String& array)
{
    QByteArray state = QByteArray::fromStdString(array);
    restoreState(QByteArray::fromBase64(state));
}

String MainWindow::GetGeometry() const
{
    QByteArray geometry = saveGeometry().toBase64();
    return geometry.toStdString();
}

void MainWindow::SetGeometry(const String& array)
{
    QByteArray geometry = QByteArray::fromStdString(array);
    restoreGeometry(QByteArray::fromBase64(geometry));
}

String MainWindow::GetConsoleState() const
{
    QByteArray consoleState = logWidget->Serialize().toBase64();
    return consoleState.toStdString();
}

void MainWindow::SetConsoleState(const String& array)
{
    QByteArray consoleState = QByteArray::fromStdString(array);
    logWidget->Deserialize(QByteArray::fromBase64(consoleState));
}

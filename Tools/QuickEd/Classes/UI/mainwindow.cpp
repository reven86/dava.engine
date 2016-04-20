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


#include "mainwindow.h"
#include "Project/Project.h"
#include "Document.h"
#include "DocumentGroup.h"
#include "Render/Texture.h"

#include "Helpers/ResourcesManageHelper.h"

#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "Utils/QtDavaConvertion.h"

#include "QtTools/FileDialog/FileDialog.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/EditorPreferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "Preferences/PreferencesDialog.h"

#include "DebugTools/DebugTools.h"
#include "QtTools/Utils/Themes/Themes.h"

REGISTER_PREFERENCES_ON_START(MainWindow
                              ,
                              PREF_ARG("isPixelized", false)
                              ,
                              PREF_ARG("state", DAVA::String())
                              ,
                              PREF_ARG("geometry", DAVA::String())
                              ,
                              PREF_ARG("consoleState", DAVA::String())
                              )

using namespace DAVA;

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

    QAction* undoAction = documentGroup->CreateUndoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/Icons/edit_undo.png"));

    QAction* redoAction = documentGroup->CreateRedoAction(this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/Icons/edit_redo.png"));

    mainToolbar->addAction(undoAction);
    mainToolbar->addAction(redoAction);

    Q_ASSERT(documentGroup != nullptr);
    documentGroup->AttachSaveAction(actionSaveDocument);
    documentGroup->AttachSaveAllAction(actionSaveAllDocuments);

    QAction* actionCloseDocument = new QAction("Close current document", this);
    actionCloseDocument->setShortcut(static_cast<int>(Qt::ControlModifier | Qt::Key_W));
    actionCloseDocument->setShortcutContext(Qt::WindowShortcut);
    previewWidget->GetGLWidget()->addAction(actionCloseDocument);
    documentGroup->AttachCloseDocumentAction(actionCloseDocument);
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
    bool wasBlocked = comboboxLanguage->blockSignals(true); //performace fix
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

    connect(actionOpen_project, &QAction::triggered, this, &MainWindow::OnOpenProject);
    connect(actionClose_project, &QAction::triggered, this, &MainWindow::CloseProject);

    connect(actionExit, &QAction::triggered, this, &MainWindow::ActionExitTriggered);
    connect(menuRecent, &QMenu::triggered, this, &MainWindow::RecentMenuTriggered);

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

    connect(action_settings, &QAction::triggered, this, &MainWindow::OnEditorPreferencesTriggered);
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
    menuView->addSeparator();
    // Setup the Background Color menu.
    menuView->addSeparator();
    QAction* backgroundColorAction = PreferencesActionsFactory::CreateActionForPreference(FastName("ColorControl"), FastName("backgroundColor"), this);
    backgroundColorAction->setText(tr("Background color"));
    menuView->addAction(backgroundColorAction);
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

void MainWindow::closeEvent(QCloseEvent* ev)
{
    if (!CloseRequested()) //we cannot access to EditorCore directly by parent
    {
        ev->ignore();
    }
    else
    {
        ev->accept();
    }
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

void MainWindow::OnOpenProject()
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
    PreferencesDialog dialog;
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

DAVA::String MainWindow::GetState() const
{
    QByteArray state = saveState().toBase64();
    return state.toStdString();
}

void MainWindow::SetState(const DAVA::String& array)
{
    QByteArray state = QByteArray::fromStdString(array);
    restoreState(QByteArray::fromBase64(state));
}

DAVA::String MainWindow::GetGeometry() const
{
    QByteArray geometry = saveGeometry().toBase64();
    return geometry.toStdString();
}

void MainWindow::SetGeometry(const DAVA::String& array)
{
    QByteArray geometry = QByteArray::fromStdString(array);
    restoreGeometry(QByteArray::fromBase64(geometry));
}

DAVA::String MainWindow::GetConsoleState() const
{
    QByteArray consoleState = logWidget->Serialize().toBase64();
    return consoleState.toStdString();
}

void MainWindow::SetConsoleState(const DAVA::String& array)
{
    QByteArray consoleState = QByteArray::fromStdString(array);
    logWidget->Deserialize(QByteArray::fromBase64(consoleState));
}

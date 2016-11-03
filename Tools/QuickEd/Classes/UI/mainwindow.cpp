#include "mainwindow.h"
#include "Document/Document.h"
#include "Document/DocumentGroup.h"
#include "Render/Texture.h"

#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "Utils/QtDavaConvertion.h"
#include "QtTools/Utils/Utils.h"

#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "Preferences/PreferencesDialog.h"

#include "DebugTools/DebugTools.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "UI/Package/PackageModel.h"
#include "UI/ProjectView.h"
#include "UI/DocumentGroupView.h"

#include "ui_mainwindow.h"

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
    , ui(new Ui::MainWindow())
    , loggerOutput(new LoggerOutputObject)
{
    ui->setupUi(this);
    DebugTools::ConnectToUI(ui.get());
    SetupShortcuts();
    SetupViewMenu();

    projectView = new ProjectView(this);
    documentGroupView = new DocumentGroupView(this);

    InitEmulationMode();
    ConnectActions();

    ui->tabBar->setElideMode(Qt::ElideNone);
    ui->tabBar->setTabsClosable(true);
    ui->tabBar->setUsesScrollButtons(true);

    PreferencesStorage::Instance()->RegisterPreferences(this);

    connect(this, &MainWindow::EmulationModeChanged, ui->previewWidget, &PreviewWidget::OnEmulationModeChanged);
    connect(ui->previewWidget, &PreviewWidget::DropRequested, ui->packageWidget->GetPackageModel(), &PackageModel::OnDropMimeData, Qt::DirectConnection);
    connect(ui->previewWidget, &PreviewWidget::DeleteRequested, ui->packageWidget, &PackageWidget::OnDelete);
    connect(ui->previewWidget, &PreviewWidget::ImportRequested, ui->packageWidget, &PackageWidget::OnImport);
    connect(ui->previewWidget, &PreviewWidget::CutRequested, ui->packageWidget, &PackageWidget::OnCut);
    connect(ui->previewWidget, &PreviewWidget::CopyRequested, ui->packageWidget, &PackageWidget::OnCopy);
    connect(ui->previewWidget, &PreviewWidget::PasteRequested, ui->packageWidget, &PackageWidget::OnPaste);
    connect(ui->previewWidget, &PreviewWidget::SelectionChanged, ui->packageWidget, &PackageWidget::OnSelectionChanged);

    connect(ui->packageWidget, &PackageWidget::SelectedNodesChanged, ui->previewWidget, &PreviewWidget::OnSelectionChanged);

    connect(ui->packageWidget, &PackageWidget::CurrentIndexChanged, ui->propertiesWidget, &PropertiesWidget::UpdateModel);

    connect(loggerOutput, &LoggerOutputObject::OutputReady, this, &MainWindow::OnLogOutput, Qt::DirectConnection);
}

MainWindow::~MainWindow()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void MainWindow::SetEditorTitle(const QString& aEditorTitle)
{
    editorTitle = aEditorTitle;

    UpdateWindowTitle();
}

void MainWindow::SetProjectPath(const QString& aProjectPath)
{
    projectPath = aProjectPath;

    UpdateWindowTitle();
}

void MainWindow::OnRecentMenu(QAction* action)
{
    QString projectPath = action->data().toString();

    if (projectPath.isEmpty())
    {
        return;
    }

    emit RecentProject(projectPath);
}

QString MainWindow::ConvertLangCodeToString(const QString& langCode)
{
    QLocale locale(langCode);
    switch (locale.script())
    {
    case QLocale::SimplifiedChineseScript:
    {
        return "Chinese simpl.";
    }

    case QLocale::TraditionalChineseScript:
    {
        return "Chinese trad.";
    }

    default:
        return QLocale::languageToString(locale.language());
    }
}

void MainWindow::SetupShortcuts()
{
    //Qt can not set multishortcut or enum shortcut in Qt designer
    ui->actionReloadDocument->setShortcuts(QList<QKeySequence>()
                                           << Qt::CTRL + Qt::Key_R
                                           << Qt::Key_F5);

    ui->actionRedo->setShortcuts(QList<QKeySequence>()
                                 << Qt::CTRL + Qt::Key_Y
                                 << Qt::CTRL + Qt::SHIFT + Qt::Key_Z);

    ui->actionFindFileInProject->setShortcuts(QList<QKeySequence>()
                                              << Qt::CTRL + Qt::SHIFT + Qt::Key_O
                                              << Qt::ALT + Qt::SHIFT + Qt::Key_O);
// Remap zoom in/out shorcuts for windows platform
#if defined(__DAVAENGINE_WIN32__)
    ui->actionZoomIn->setShortcuts(QList<QKeySequence>()
                                   << Qt::CTRL + Qt::Key_Equal
                                   << Qt::CTRL + Qt::Key_Plus);
#endif
}

void MainWindow::ConnectActions()
{
    connect(ui->actionNewProject, &QAction::triggered, this, &MainWindow::NewProject);
    connect(ui->actionOpenProject, &QAction::triggered, this, &MainWindow::OpenProject);
    connect(ui->actionCloseProject, &QAction::triggered, this, &MainWindow::CloseProject);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::Exit);
    connect(ui->menuRecent, &QMenu::triggered, this, &MainWindow::OnRecentMenu);

    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::ShowHelp);

    connect(ui->actionPixelized, &QAction::triggered, this, &MainWindow::OnPixelizationStateChanged);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::OnEditorPreferencesTriggered);

    connect(ui->actionZoomOut, &QAction::triggered, ui->previewWidget, &PreviewWidget::OnDecrementScale);
    connect(ui->actionZoomIn, &QAction::triggered, ui->previewWidget, &PreviewWidget::OnIncrementScale);
    connect(ui->actionActualZoom, &QAction::triggered, ui->previewWidget, &PreviewWidget::SetActualScale);
}

void MainWindow::InitEmulationMode()
{
    emulationBox = new QCheckBox("Emulation", this);
    emulationBox->setLayoutDirection(Qt::RightToLeft);
    connect(emulationBox, &QCheckBox::toggled, this, &MainWindow::EmulationModeChanged);
    ui->toolBarPlugins->addSeparator();
    ui->toolBarPlugins->addWidget(emulationBox);
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    QList<QAction*> dockWidgetToggleActions;
    dockWidgetToggleActions << ui->propertiesWidget->toggleViewAction()
                            << ui->fileSystemDockWidget->toggleViewAction()
                            << ui->packageWidget->toggleViewAction()
                            << ui->libraryWidget->toggleViewAction()
                            << ui->consoleDockWidget->toggleViewAction()
                            << ui->mainToolbar->toggleViewAction()
                            << ui->toolBarPlugins->toggleViewAction();

    QAction* separator = ui->menuView->insertSeparator(ui->menuApplicationStyle->menuAction());
    ui->menuView->insertActions(separator, dockWidgetToggleActions);

    SetupAppStyleMenu();
    SetupBackgroundMenu();
}

void MainWindow::SetupAppStyleMenu()
{
    QActionGroup* actionGroup = new QActionGroup(this);
    for (const QString& theme : Themes::ThemesNames())
    {
        QAction* action = new QAction(theme, ui->menuView);
        actionGroup->addAction(action);
        action->setCheckable(true);
        if (theme == Themes::GetCurrentThemeStr())
        {
            action->setChecked(true);
        }
        ui->menuApplicationStyle->addAction(action);
    }
    connect(actionGroup, &QActionGroup::triggered, [](QAction* action) {
        if (action->isChecked())
        {
            Themes::SetCurrentTheme(action->text());
        }
    });
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

    backgroundActions = new QActionGroup(this);
    for (int i = 0, count = inspInfo->MembersCount(), index = 0; i < count; ++i)
    {
        const InspMember* member = inspInfo->Member(i);
        backgroundColorMembers.insert(member);
        QString str(member->Name().c_str());
        if (str.contains(QRegExp("backgroundColor\\d+")))
        {
            QAction* colorAction = new QAction(QString("Background color %1").arg(index), ui->menuGridColor);
            backgroundActions->addAction(colorAction);
            colorAction->setCheckable(true);
            colorAction->setData(QVariant::fromValue<const InspMember*>(member));
            if (index == currentIndex)
            {
                colorAction->setChecked(true);
            }
            ui->menuGridColor->addAction(colorAction);
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

void MainWindow::SetRecentProjects(const QStringList& lastProjectsPathes)
{
    ui->menuRecent->clear();
    for (auto& projectPath : lastProjectsPathes)
    {
        QAction* recentProject = new QAction(projectPath, this);
        recentProject->setData(projectPath);
        ui->menuRecent->addAction(recentProject);
    }
    ui->menuRecent->setEnabled(!lastProjectsPathes.isEmpty());
}

void MainWindow::InjectRenderWidget(DAVA::RenderWidget* renderWidget)
{
    ui->previewWidget->InjectRenderWidget(renderWidget);
}

void MainWindow::OnWindowCreated()
{
    ui->previewWidget->OnWindowCreated();
}

void MainWindow::ShowResultList(const QString& title, const DAVA::ResultList& resultList)
{
    QStringList errors;
    for (const Result& result : resultList.GetResults())
    {
        if (result.type == Result::RESULT_ERROR ||
            result.type == Result::RESULT_FAILURE)
        {
            errors << QString::fromStdString(result.message);
        }
    }

    if (!errors.empty())
    {
        QMessageBox::warning(qApp->activeWindow(), title, errors.join('\n'));
    }
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

void MainWindow::OnLogOutput(Logger::eLogLevel logLevel, const QByteArray& output)
{
    if (static_cast<int32>(1 << logLevel) & acceptableLoggerFlags)
    {
        ui->logWidget->AddMessage(logLevel, output);
    }
}

void MainWindow::OnEditorPreferencesTriggered()
{
    PreferencesDialog dialog(this);
    dialog.exec();
}

bool MainWindow::IsPixelized() const
{
    return ui->actionPixelized->isChecked();
}

void MainWindow::SetPixelized(bool pixelized)
{
    ui->actionPixelized->setChecked(pixelized);
}

void MainWindow::UpdateWindowTitle()
{
    QString title;
    if (projectPath.isEmpty())
    {
        title = editorTitle;
    }
    else
    {
        title = QString("%1 | Project %2").arg(editorTitle).arg(projectPath);
    }
    setWindowTitle(title);
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
    QByteArray consoleState = ui->logWidget->Serialize().toBase64();
    return consoleState.toStdString();
}

void MainWindow::SetConsoleState(const String& array)
{
    QByteArray consoleState = QByteArray::fromStdString(array);
    ui->logWidget->Deserialize(QByteArray::fromBase64(consoleState));
}

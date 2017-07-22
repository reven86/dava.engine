#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Modules/LegacySupportModule/Private/Project.h"
#include "Render/Texture.h"

#include "Utils/QtDavaConvertion.h"
#include "QtTools/Utils/Utils.h"

#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "Preferences/PreferencesDialog.h"

#include "DebugTools/DebugTools.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "UI/Package/PackageModel.h"
#include "UI/ProjectView.h"

#include <Base/Result.h>

#include <QMessageBox>
#include <QCheckBox>
#include <QKeyEvent>

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(MainWindow,
                              PREF_ARG("isPixelized", false),
                              )

Q_DECLARE_METATYPE(const InspMember*);

MainWindow::MainWindow(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* tarcUi, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
#if defined(__DAVAENGINE_MACOS__)
    , shortcutChecker(this)
#endif //__DAVAENGINE_MACOS__
{
    ui->setupUi(this);
    setObjectName("QuickEd"); //we need to support old names to save window settings

    ui->libraryWidget->SetAccessor(accessor);
    ui->libraryWidget->SetUI(tarcUi);
    ui->propertiesWidget->SetAccessor(accessor);
    ui->propertiesWidget->SetUI(tarcUi);
    ui->packageWidget->SetAccessor(accessor);
    ui->packageWidget->SetUI(tarcUi);

    setWindowIcon(QIcon(":/icon.ico"));
    DebugTools::ConnectToUI(ui.get());
    SetupViewMenu();

    projectView = new ProjectView(this);

    InitEmulationMode();
    ConnectActions();

    PreferencesStorage::Instance()->RegisterPreferences(this);

    connect(projectView, &ProjectView::ProjectChanged, ui->propertiesWidget, &PropertiesWidget::SetProject);

    qApp->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void MainWindow::SetEditorTitle(const QString& editorTitle_)
{
    editorTitle = editorTitle_;

    UpdateWindowTitle();
}

void MainWindow::SetProjectPath(const QString& projectPath_)
{
    projectPath = projectPath_;

    UpdateWindowTitle();
}

void MainWindow::ConnectActions()
{
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actionPixelized, &QAction::triggered, this, &MainWindow::OnPixelizationStateChanged);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::OnEditorPreferencesTriggered);
}

void MainWindow::InitEmulationMode()
{
    emulationBox = new QCheckBox("Emulation", this);
    emulationBox->setLayoutDirection(Qt::RightToLeft);
    connect(emulationBox, &QCheckBox::toggled, this, &MainWindow::EmulationModeChanged);
    ui->toolBarGlobal->addSeparator();
    ui->toolBarGlobal->addWidget(emulationBox);
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    QList<QAction*> dockWidgetToggleActions;
    dockWidgetToggleActions << ui->propertiesWidget->toggleViewAction()
                            << ui->packageWidget->toggleViewAction()
                            << ui->libraryWidget->toggleViewAction()
                            << ui->mainToolbar->toggleViewAction()
                            << ui->toolBarGlobal->toggleViewAction();

    QAction* separator = ui->View->insertSeparator(ui->menuApplicationStyle->menuAction());
    ui->Dock->insertActions(separator, dockWidgetToggleActions);

    SetupAppStyleMenu();
    SetupBackgroundMenu();
}

void MainWindow::SetupAppStyleMenu()
{
    QActionGroup* actionGroup = new QActionGroup(this);
    for (const QString& theme : Themes::ThemesNames())
    {
        QAction* action = new QAction(theme, ui->View);
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

MainWindow::ProjectView* MainWindow::GetProjectView() const
{
    return projectView;
}

PackageWidget* MainWindow::GetPackageWidget() const
{
    return ui->packageWidget;
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

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
#if defined(__DAVAENGINE_MACOS__)
    if (QEvent::ShortcutOverride == event->type() && shortcutChecker.TryCallShortcut(static_cast<QKeyEvent*>(event)))
    {
        return true;
    }
#endif
    return false;
}

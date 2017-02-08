#include "mainwindow.h"
#include "Modules/DocumentsModule/Document.h"
#include "Render/Texture.h"

#include "UI/FileSystemView/FileSystemDockWidget.h"
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
#include "UI/DocumentGroupView.h"

#include <Base/Result.h>

#include <QMessageBox>
#include <QCheckBox>

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(MainWindow,
                              PREF_ARG("isPixelized", false),
                              )

Q_DECLARE_METATYPE(const InspMember*);

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
#if defined(__DAVAENGINE_MACOS__)
    , shortcutChecker(this)
#endif //__DAVAENGINE_MACOS__
{
    setupUi(this);
    setWindowIcon(QIcon(":/icon.ico"));
    DebugTools::ConnectToUI(this);
    SetupShortcuts();
    SetupViewMenu();

    projectView = new ProjectView(this);
    documentGroupView = new DocumentGroupView(this);

    InitEmulationMode();
    ConnectActions();

    PreferencesStorage::Instance()->RegisterPreferences(this);

    connect(packageWidget, &PackageWidget::CurrentIndexChanged, propertiesWidget, &PropertiesWidget::UpdateModel);

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

void MainWindow::SetupShortcuts()
{
//Qt can not set multishortcut or enum shortcut in Qt designer
#if defined(__DAVAENGINE_WIN32__)
    actionZoomIn->setShortcuts(QList<QKeySequence>()
                               << Qt::CTRL + Qt::Key_Equal
                               << Qt::CTRL + Qt::Key_Plus);
#endif
}

void MainWindow::ConnectActions()
{
    connect(actionExit, &QAction::triggered, this, &MainWindow::close);

    connect(actionPixelized, &QAction::triggered, this, &MainWindow::OnPixelizationStateChanged);
    connect(actionPreferences, &QAction::triggered, this, &MainWindow::OnEditorPreferencesTriggered);
}

void MainWindow::InitEmulationMode()
{
    emulationBox = new QCheckBox("Emulation", this);
    emulationBox->setLayoutDirection(Qt::RightToLeft);
    connect(emulationBox, &QCheckBox::toggled, this, &MainWindow::EmulationModeChanged);
    toolBarGlobal->addSeparator();
    toolBarGlobal->addWidget(emulationBox);
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    QList<QAction*> dockWidgetToggleActions;
    dockWidgetToggleActions << propertiesWidget->toggleViewAction()
                            << fileSystemDockWidget->toggleViewAction()
                            << packageWidget->toggleViewAction()
                            << libraryWidget->toggleViewAction()
                            << styleSheetInspectorWidget->toggleViewAction()
                            << findWidget->toggleViewAction()
                            << mainToolbar->toggleViewAction()
                            << toolBarGlobal->toggleViewAction();

    QAction* separator = View->insertSeparator(menuApplicationStyle->menuAction());
    View->insertActions(separator, dockWidgetToggleActions);

    SetupAppStyleMenu();
    SetupBackgroundMenu();
}

void MainWindow::SetupAppStyleMenu()
{
    QActionGroup* actionGroup = new QActionGroup(this);
    for (const QString& theme : Themes::ThemesNames())
    {
        QAction* action = new QAction(theme, View);
        actionGroup->addAction(action);
        action->setCheckable(true);
        if (theme == Themes::GetCurrentThemeStr())
        {
            action->setChecked(true);
        }
        menuApplicationStyle->addAction(action);
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
            QAction* colorAction = new QAction(QString("Background color %1").arg(index), menuGridColor);
            backgroundActions->addAction(colorAction);
            colorAction->setCheckable(true);
            colorAction->setData(QVariant::fromValue<const InspMember*>(member));
            if (index == currentIndex)
            {
                colorAction->setChecked(true);
            }
            menuGridColor->addAction(colorAction);
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

MainWindow::ProjectView* MainWindow::GetProjectView()
{
    return projectView;
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
    return actionPixelized->isChecked();
}

void MainWindow::SetPixelized(bool pixelized)
{
    actionPixelized->setChecked(pixelized);
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

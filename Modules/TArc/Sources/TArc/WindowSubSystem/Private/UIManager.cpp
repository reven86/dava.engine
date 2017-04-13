#include "TArc/WindowSubSystem/Private/UIManager.h"

#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/Private/WaitDialog.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include <Base/BaseTypes.h>
#include <Base/Any.h>
#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>

#include <QPointer>
#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QToolButton>
#include <QUrlQuery>
#include <QLayout>
#include <QFrame>
#include <QEvent>

#include <QFileDialog>
#include <QMessageBox>

namespace DAVA
{
namespace TArc
{
namespace UIManagerDetail
{
String WINDOW_GEOMETRY_KEY("geometry");
String WINDOW_STATE_KEY("state");
String FILE_DIR_KEY("fileDialogDir");

static Vector<std::pair<QMessageBox::StandardButton, ModalMessageParams::Button>> buttonsConvertor =
{
  std::make_pair(QMessageBox::NoButton, ModalMessageParams::NoButton),
  std::make_pair(QMessageBox::Ok, ModalMessageParams::Ok),
  std::make_pair(QMessageBox::Open, ModalMessageParams::Open),
  std::make_pair(QMessageBox::Cancel, ModalMessageParams::Cancel),
  std::make_pair(QMessageBox::Close, ModalMessageParams::Close),
  std::make_pair(QMessageBox::Yes, ModalMessageParams::Yes),
  std::make_pair(QMessageBox::YesToAll, ModalMessageParams::YesToAll),
  std::make_pair(QMessageBox::No, ModalMessageParams::No),
  std::make_pair(QMessageBox::NoToAll, ModalMessageParams::NoToAll),
  std::make_pair(QMessageBox::Discard, ModalMessageParams::Discard),
  std::make_pair(QMessageBox::Apply, ModalMessageParams::Apply),
  std::make_pair(QMessageBox::Save, ModalMessageParams::Save),
  std::make_pair(QMessageBox::SaveAll, ModalMessageParams::SaveAll),
  std::make_pair(QMessageBox::Abort, ModalMessageParams::Abort),
  std::make_pair(QMessageBox::Retry, ModalMessageParams::Retry),
  std::make_pair(QMessageBox::Ignore, ModalMessageParams::Ignore),
  std::make_pair(QMessageBox::Reset, ModalMessageParams::Reset)
};

static Vector<std::pair<QMessageBox::Icon, ModalMessageParams::Icon>> iconsConvertor =
{
  std::make_pair(QMessageBox::NoIcon, ModalMessageParams::NoIcon),
  std::make_pair(QMessageBox::Information, ModalMessageParams::Information),
  std::make_pair(QMessageBox::Warning, ModalMessageParams::Warning),
  std::make_pair(QMessageBox::Critical, ModalMessageParams::Critical),
  std::make_pair(QMessageBox::Question, ModalMessageParams::Question),
};

QMessageBox::StandardButton Convert(const ModalMessageParams::Button& button)
{
    using ButtonNode = std::pair<QMessageBox::StandardButton, ModalMessageParams::Button>;
    QMessageBox::StandardButton ret = QMessageBox::NoButton;
    for (const ButtonNode& node : buttonsConvertor)
    {
        if (button == node.second)
        {
            ret = node.first;
        }
    }

    return ret;
}

QMessageBox::StandardButtons Convert(const ModalMessageParams::Buttons& buttons)
{
    using ButtonNode = std::pair<QMessageBox::StandardButton, ModalMessageParams::Button>;
    QMessageBox::StandardButtons ret;
    for (const ButtonNode& node : buttonsConvertor)
    {
        if (buttons.testFlag(node.second))
        {
            ret |= node.first;
        }
    }

    return ret;
}

ModalMessageParams::Button Convert(const QMessageBox::StandardButton& button)
{
    using ButtonNode = std::pair<QMessageBox::StandardButton, ModalMessageParams::Button>;
    auto iter = std::find_if(buttonsConvertor.begin(), buttonsConvertor.end(), [button](const ButtonNode& node)
                             {
                                 return node.first == button;
                             });

    DVASSERT(iter != buttonsConvertor.end());
    return iter->second;
}

QMessageBox::Icon Convert(const ModalMessageParams::Icon& icon)
{
    using IconNode = std::pair<QMessageBox::Icon, ModalMessageParams::Icon>;
    auto iter = std::find_if(iconsConvertor.begin(), iconsConvertor.end(), [icon](const IconNode& node)
                             {
                                 return node.second == icon;
                             });
    DVASSERT(iter != iconsConvertor.end());
    return iter->first;
}

ModalMessageParams::Icon Convert(const QMessageBox::Icon& icon)
{
    using IconNode = std::pair<QMessageBox::Icon, ModalMessageParams::Icon>;
    auto iter = std::find_if(iconsConvertor.begin(), iconsConvertor.end(), [icon](const IconNode& node)
                             {
                                 return node.first == icon;
                             });
    DVASSERT(iter != iconsConvertor.end());
    return iter->second;
}

struct StatusBarWidget
{
    QWidget* widget = nullptr;
    QAction* action = nullptr;
};

struct MainWindowInfo
{
    QPointer<QMainWindow> window = nullptr;
    QMenuBar* menuBar = nullptr;
    Vector<StatusBarWidget> nonPermanentStatusBarWidgets;
    Vector<StatusBarWidget> permanentStatusBarWidgets;
};

QAction* FindAction(QWidget* w, const QString& actionName)
{
    QList<QAction*> actions = w->actions();
    foreach (QAction* action, actions)
    {
        if (action->objectName() == actionName)
            return action;
    }

    QMenu* menu = w->findChild<QMenu*>(actionName, Qt::FindDirectChildrenOnly);
    if (menu != nullptr)
    {
        return menu->menuAction();
    }

    return nullptr;
}

void InsertActionImpl(QMenu* menu, QAction* before, QAction* action)
{
    menu->insertAction(before, action);
}

void InsertActionImpl(QToolBar* toolbar, QAction* before, QAction* action)
{
    QWidget* w = GetAttachedWidget(action);
    if (w == nullptr)
    {
        toolbar->insertAction(before, action);
    }
    else
    {
        QToolButton* toolButton = qobject_cast<QToolButton*>(w);
        bool autoRise = (toolButton != nullptr) ? toolButton->autoRaise() : false;
        toolbar->insertWidget(before, w);
        if (toolButton != nullptr)
        {
            toolButton->setAutoRaise(autoRise);
        }
    }
}

void InsertActionImpl(QMenuBar* menuBar, QAction* before, QAction* action)
{
    menuBar->insertAction(before, action);
}

template <typename T>
void InsertAction(T* container, QAction* action, const InsertionParams& params)
{
    QAction* beforeAction = nullptr;
    if (params.item.isEmpty())
    {
        QList<QAction*> actions = container->actions();
        if (params.method == InsertionParams::eInsertionMethod::BeforeItem && !actions.isEmpty())
        {
            beforeAction = actions.at(0);
        }
    }
    else
    {
        beforeAction = FindAction(container, params.item);
        if (params.method == InsertionParams::eInsertionMethod::AfterItem)
        {
            QList<QAction*> actions = container->actions();
            int beforeActionIndex = actions.indexOf(beforeAction);
            if (beforeActionIndex + 1 < actions.size())
            {
                beforeAction = actions.at(actions.indexOf(beforeAction) + 1);
            }
        }
    }

    action->setParent(container);
    InsertActionImpl(container, beforeAction, action);
}

void AddMenuPoint(const QUrl& url, QAction* action, MainWindowInfo& windowInfo)
{
    if (windowInfo.menuBar == nullptr)
    {
        windowInfo.menuBar = new QMenuBar();
        windowInfo.menuBar->setNativeMenuBar(true);
        windowInfo.menuBar->setObjectName("menu");
        windowInfo.menuBar->setVisible(true);
        windowInfo.window->setMenuBar(windowInfo.menuBar);
    }

    QStringList path = url.path().split("$/", QString::SkipEmptyParts);
    if (path.isEmpty())
    {
        UIManagerDetail::InsertAction(windowInfo.menuBar, action, InsertionParams::Create(url));
        return;
    }

    QMenu* topLevelMenu = nullptr;
    QString topLevelTitle = path.front();
    topLevelMenu = windowInfo.menuBar->findChild<QMenu*>(topLevelTitle, Qt::FindDirectChildrenOnly);
    if (topLevelMenu == nullptr)
    {
        QAction* action = FindAction(windowInfo.menuBar, topLevelTitle);
        topLevelMenu = new QMenu(topLevelTitle, windowInfo.menuBar);
        topLevelMenu->setObjectName(topLevelTitle);
        if (action != nullptr)
        {
            action->setMenu(topLevelMenu);
        }
        else
        {
            windowInfo.menuBar->addMenu(topLevelMenu);
        }
    }

    QMenu* currentLevelMenu = topLevelMenu;
    for (int i = 1; i < path.size(); ++i)
    {
        QString currentLevelTittle = path[i];
        QMenu* menu = currentLevelMenu->findChild<QMenu*>(currentLevelTittle);
        if (menu == nullptr)
        {
            QAction* action = FindAction(currentLevelMenu, currentLevelTittle);
            menu = new QMenu(currentLevelTittle, currentLevelMenu);
            menu->setObjectName(currentLevelTittle);
            if (action != nullptr)
            {
                action->setMenu(menu);
            }
            else
            {
                currentLevelMenu->addMenu(menu);
            }
        }
        currentLevelMenu = menu;
    }

    UIManagerDetail::InsertAction(currentLevelMenu, action, InsertionParams::Create(url));
}

void AddToolbarPoint(const QUrl& url, QAction* action, MainWindowInfo& windowInfo)
{
    QString toolbarName = url.path();
    QToolBar* toolbar = windowInfo.window->findChild<QToolBar*>(toolbarName);
    if (toolbar == nullptr)
    {
        toolbar = new QToolBar(toolbarName, windowInfo.window);
        toolbar->setObjectName(toolbarName);
        windowInfo.window->addToolBar(toolbar);
    }

    UIManagerDetail::InsertAction(toolbar, action, InsertionParams::Create(url));
}

void AddStatusbarPoint(const QUrl& url, QAction* action, MainWindowInfo& windowInfo)
{
    bool isPermanent = url.path() == permanentStatusbarAction;
    int stretchFactor = url.fragment().toInt();
    QWidget* actionWidget = action->data().value<QWidget*>();
    if (actionWidget == nullptr)
    {
        QToolButton* toolButton = new QToolButton();
        toolButton->setDefaultAction(action);
        toolButton->setAutoRaise(true);
        toolButton->setMaximumSize(QSize(16, 16));
        actionWidget = toolButton;
    }

    InsertionParams insertParams = InsertionParams::Create(url);
    Vector<StatusBarWidget>* widgets = nullptr;
    if (isPermanent)
    {
        widgets = &windowInfo.permanentStatusBarWidgets;
    }
    else
    {
        widgets = &windowInfo.nonPermanentStatusBarWidgets;
    }

    size_t positionIndex = 0;
    if (insertParams.item.isEmpty())
    {
        if (insertParams.method == InsertionParams::eInsertionMethod::AfterItem)
        {
            positionIndex = widgets->size();
        }
    }
    else
    {
        for (size_t i = 0; i < widgets->size(); ++i)
        {
            const StatusBarWidget& w = (*widgets)[i];
            if (w.action->objectName() == insertParams.item)
            {
                positionIndex = i;
                break;
            }
        }
    }

    StatusBarWidget statusBarWidget;
    statusBarWidget.widget = actionWidget;
    statusBarWidget.action = action;
    widgets->insert(widgets->begin() + positionIndex, statusBarWidget);
    QStatusBar* statusBar = windowInfo.window->statusBar();
    if (isPermanent)
    {
        statusBar->insertPermanentWidget(static_cast<int>(positionIndex), actionWidget, stretchFactor);
    }
    else
    {
        statusBar->insertWidget(static_cast<int>(positionIndex), actionWidget, stretchFactor);
    }
}

void AddAction(MainWindowInfo& windowInfo, const ActionPlacementInfo& placement, QAction* action)
{
    if (action->objectName().isEmpty())
    {
        action->setObjectName(action->text());
    }

    for (const QUrl& url : placement.GetUrls())
    {
        QString scheme = url.scheme();
        if (scheme == menuScheme)
        {
            AddMenuPoint(url, action, windowInfo);
        }
        else if (scheme == toolbarScheme)
        {
            AddToolbarPoint(url, action, windowInfo);
        }
        else if (scheme == statusbarScheme)
        {
            AddStatusbarPoint(url, action, windowInfo);
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void RemoveMenuPoint(const QUrl& url, MainWindowInfo& windowInfo)
{
    QStringList path = url.path().split("$/");
    DVASSERT(!path.isEmpty());
    QString topLevelTitle = path.front();
    QMenu* topLevelMenu = windowInfo.menuBar->findChild<QMenu*>(topLevelTitle, Qt::FindDirectChildrenOnly);
    if (topLevelMenu == nullptr)
    {
        return;
    }

    QMenu* currentLevelMenu = topLevelMenu;
    for (int i = 1; i < path.size() - 1; ++i)
    {
        QString currentLevelTittle = path[i];
        QMenu* menu = currentLevelMenu->findChild<QMenu*>(currentLevelTittle);
        if (menu == nullptr)
        {
            break;
        }
        currentLevelMenu = menu;
    }

    if (currentLevelMenu == nullptr)
    {
        return;
    }

    QAction* action = FindAction(currentLevelMenu, path.back());
    currentLevelMenu->removeAction(action);
    action->deleteLater();
}

void RemoveToolbarPoint(const QUrl& url, MainWindowInfo& windowInfo)
{
    // TODO not implemented
    DVASSERT(false);
}

void RemoveStatusbarPoint(const QUrl& url, MainWindowInfo& windowInfo)
{
    // TODO not implemented
    DVASSERT(false);
}

void RemoveAction(MainWindowInfo& windowInfo, const ActionPlacementInfo& placement)
{
    for (const QUrl& url : placement.GetUrls())
    {
        QString scheme = url.scheme();
        if (scheme == menuScheme)
        {
            RemoveMenuPoint(url, windowInfo);
        }
        else if (scheme == toolbarScheme)
        {
            RemoveToolbarPoint(url, windowInfo);
        }
        else if (scheme == statusbarScheme)
        {
            RemoveStatusbarPoint(url, windowInfo);
        }
        else
        {
            DVASSERT(false);
        }
    }
}
} // namespace UIManagerDetail

struct UIManager::Impl : public QObject
{
    UIManager::Delegate* managerDelegate = nullptr;
    Array<Function<void(const PanelKey&, const WindowKey&, QWidget*)>, PanelKey::TypesCount> addFunctions;
    UnorderedMap<WindowKey, UIManagerDetail::MainWindowInfo> windows;
    PropertiesItem propertiesHolder;
    bool initializationFinished = false;
    Set<WaitHandle*> activeWaitDialogues;
    ClientModule* currentModule = nullptr;

    struct ModuleResources
    {
        Vector<QPointer<QAction>> actions;
        Vector<QPointer<QWidget>> widgets;
    };

    Map<ClientModule*, ModuleResources> moduleResourcesMap;

    Impl(UIManager::Delegate* delegate, PropertiesItem&& givenPropertiesHolder)
        : managerDelegate(delegate)
        , propertiesHolder(std::move(givenPropertiesHolder))
    {
        addFunctions[PanelKey::DockPanel] = MakeFunction(this, &UIManager::Impl::AddDockPanel);
        addFunctions[PanelKey::CentralPanel] = MakeFunction(this, &UIManager::Impl::AddCentralPanel);
    }

    ~Impl()
    {
        for (auto& wnd : windows)
        {
            delete wnd.second.window.data();
        }
    }

    UIManagerDetail::MainWindowInfo& FindOrCreateWindow(const WindowKey& windowKey)
    {
        auto iter = windows.find(windowKey);
        if (iter == windows.end())
        {
            QMainWindow* window = new QMainWindow();
            InitNewWindow(windowKey, window);

            UIManagerDetail::MainWindowInfo info;
            info.window = window;
            auto emplacePair = windows.emplace(windowKey, info);
            DVASSERT(emplacePair.second == true);
            iter = emplacePair.first;
        }

        return iter->second;
    }

    UIManagerDetail::MainWindowInfo* FindWindow(const WindowKey& key)
    {
        auto iter = windows.find(key);
        if (iter == windows.end())
        {
            return nullptr;
        }

        return &iter->second;
    }

    void InitNewWindow(const WindowKey& windowKey, QMainWindow* window)
    {
        window->installEventFilter(this);

        FastName appId = windowKey.GetAppID();
        window->setObjectName(appId.c_str());
    }

protected:
    bool eventFilter(QObject* obj, QEvent* e)
    {
        if (e->type() == QEvent::Close)
        {
            QMainWindow* window = qobject_cast<QMainWindow*>(obj);
            DVASSERT(window);

            auto iter = std::find_if(windows.begin(), windows.end(), [window](const std::pair<WindowKey, UIManagerDetail::MainWindowInfo>& w)
                                     {
                                         return window == w.second.window;
                                     });

            // When user close application on MacOS by pressing Cmd+Q, Qt somewhy sends CloseEvent twice.
            // So "iter == windows.end()" means that we have already got one CloseEvent for this window
            if (iter != windows.end())
            {
                const WindowKey& windowKey = iter->first;
                if (managerDelegate->WindowCloseRequested(iter->first))
                {
                    QMainWindow* mainWindow = iter->second.window;

                    PropertiesItem ph = propertiesHolder.CreateSubHolder(windowKey.GetAppID().c_str());
                    ph.Set(UIManagerDetail::WINDOW_STATE_KEY, mainWindow->saveState());
                    ph.Set(UIManagerDetail::WINDOW_GEOMETRY_KEY, mainWindow->saveGeometry());

                    mainWindow->deleteLater();
                    managerDelegate->OnWindowClosed(iter->first);
                    windows.erase(iter);
                }
                else
                {
                    e->ignore();
                }
                return true;
            }
        }

        return false;
    }

    QDockWidget* CreateDockWidget(const DockPanelInfo& dockPanelInfo, UIManagerDetail::MainWindowInfo& mainWindowInfo, QMainWindow* mainWindow)
    {
        DVASSERT(dockPanelInfo.title.isEmpty() == false, "Provide correct value of DockPanelInfo::title");
        const QString& text = dockPanelInfo.title;

        QDockWidget* dockWidget = new QDockWidget(text, mainWindow);
        dockWidget->setObjectName(text);

        QAction* dockWidgetAction = dockWidget->toggleViewAction();

        const ActionPlacementInfo& placement = dockPanelInfo.actionPlacementInfo;

        UIManagerDetail::AddAction(mainWindowInfo, placement, dockWidgetAction);

        return dockWidget;
    }

    void AddDockPanel(const PanelKey& key, const WindowKey& windowKey, QWidget* widget)
    {
        DVASSERT(key.GetType() == PanelKey::DockPanel);
        UIManagerDetail::MainWindowInfo& mainWindowInfo = FindOrCreateWindow(windowKey);
        const DockPanelInfo& info = key.GetInfo().Get<DockPanelInfo>();
        QMainWindow* mainWindow = mainWindowInfo.window;
        DVASSERT(mainWindow != nullptr);
        QDockWidget* newDockWidget = CreateDockWidget(info, mainWindowInfo, mainWindow);
        newDockWidget->layout()->setContentsMargins(0, 0, 0, 0);
        newDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);

        newDockWidget->setVisible(true);
        newDockWidget->setWidget(widget);
        if (!mainWindow->restoreDockWidget(newDockWidget))
        {
            if (info.tabbed == true)
            {
                QList<QDockWidget*> dockWidgets = mainWindow->findChildren<QDockWidget*>();
                QDockWidget* dockToTabbify = nullptr;
                foreach (QDockWidget* dock, dockWidgets)
                {
                    if (mainWindow->dockWidgetArea(dock) == info.area)
                    {
                        dockToTabbify = dock;
                        break;
                    }
                }

                if (dockToTabbify != nullptr)
                {
                    mainWindow->tabifyDockWidget(dockToTabbify, newDockWidget);
                }
                else
                {
                    mainWindow->addDockWidget(info.area, newDockWidget);
                }
            }
            else
            {
                mainWindow->addDockWidget(info.area, newDockWidget);
            }
        }
    }

    void AddCentralPanel(const PanelKey& key, const WindowKey& windowKey, QWidget* widget)
    {
        UIManagerDetail::MainWindowInfo& mainWindowInfo = FindOrCreateWindow(windowKey);
        QMainWindow* mainWindow = mainWindowInfo.window;
        DVASSERT(mainWindow != nullptr);

        QWidget* centralWidget = mainWindow->centralWidget();
        if (centralWidget == nullptr)
        {
            mainWindow->setCentralWidget(widget);
            return;
        }

        QLayout* centralWidgetLayout = centralWidget->layout();
        if (centralWidgetLayout && qobject_cast<QFrame*>(widget) != nullptr)
        {
            centralWidgetLayout->addWidget(widget);
            return;
        }

        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget);
        if (tabWidget == nullptr)
        {
            tabWidget = new QTabWidget(mainWindow);
            tabWidget->addTab(centralWidget, centralWidget->objectName());
            mainWindow->setCentralWidget(tabWidget);
        }

        tabWidget->addTab(widget, widget->objectName());
    }
};

UIManager::UIManager(Delegate* delegate, PropertiesItem&& holder)
    : impl(new Impl(delegate, std::move(holder)))
{
}

UIManager::~UIManager() = default;

void UIManager::InitializationFinished()
{
    impl->initializationFinished = true;
    for (auto& windowIter : impl->windows)
    {
        QMainWindow* mainWindow = windowIter.second.window;
        PropertiesItem ph = impl->propertiesHolder.CreateSubHolder(mainWindow->objectName().toStdString());
        mainWindow->restoreGeometry(ph.Get<QByteArray>(UIManagerDetail::WINDOW_GEOMETRY_KEY));
        mainWindow->restoreState(ph.Get<QByteArray>(UIManagerDetail::WINDOW_STATE_KEY));

        windowIter.second.window->show();
    }
}

void UIManager::DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName)
{
    DVASSERT(impl->currentModule != nullptr);
    UIManagerDetail::MainWindowInfo& mainWindowInfo = impl->FindOrCreateWindow(windowKey);
    QToolBar* toolbar = mainWindowInfo.window->findChild<QToolBar*>(toolbarName);
    if (toolbar == nullptr)
    {
        toolbar = new QToolBar(toolbarName, mainWindowInfo.window);
        toolbar->setObjectName(toolbarName);
        mainWindowInfo.window->addToolBar(toolbar);
    }

    AddAction(windowKey, toogleToolbarVisibility, toolbar->toggleViewAction());
}

void UIManager::AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget)
{
    DVASSERT(impl->currentModule != nullptr);
    impl->moduleResourcesMap[impl->currentModule].widgets.push_back(widget);
    DVASSERT(widget != nullptr);
    widget->setObjectName(panelKey.GetViewName());

    PanelKey::Type type = panelKey.GetType();
    DVASSERT(impl->addFunctions[type] != nullptr);

    impl->addFunctions[type](panelKey, windowKey, widget);

    UIManagerDetail::MainWindowInfo& mainWindowInfo = impl->FindOrCreateWindow(windowKey);
    QMainWindow* window = mainWindowInfo.window;
    DVASSERT(window != nullptr);
    if (!window->isVisible() && impl->initializationFinished)
    {
        window->show();
    }
}

void UIManager::AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action)
{
    DVASSERT(impl->currentModule != nullptr);
    impl->moduleResourcesMap[impl->currentModule].actions.push_back(action);

    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);
    UIManagerDetail::AddAction(windowInfo, placement, action);
}

void UIManager::RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement)
{
    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);
    UIManagerDetail::RemoveAction(windowInfo, placement);
}

void UIManager::ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration)
{
    impl->FindOrCreateWindow(windowKey).window->statusBar()->showMessage(message, duration);
}

void UIManager::ClearMessage(const WindowKey& windowKey)
{
    impl->FindOrCreateWindow(windowKey).window->statusBar()->clearMessage();
}

std::unique_ptr<WaitHandle> UIManager::ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params)
{
    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);
    std::unique_ptr<WaitDialog> dlg = std::make_unique<WaitDialog>(params, windowInfo.window);
    impl->activeWaitDialogues.insert(dlg.get());
    dlg->beforeDestroy.Connect([this](WaitHandle* waitHandle)
                               {
                                   impl->activeWaitDialogues.erase(waitHandle);
                                   if (impl->activeWaitDialogues.empty())
                                   {
                                       lastWaitDialogWasClosed.Emit();
                                   }
                               });
    dlg->Show();
    return std::move(dlg);
}

bool UIManager::HasActiveWaitDalogues() const
{
    return !impl->activeWaitDialogues.empty();
}

QWidget* UIManager::GetWindow(const WindowKey& windowKey)
{
    return impl->FindOrCreateWindow(windowKey).window;
}

QString UIManager::GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);

    QString dir = params.dir;
    if (dir.isEmpty())
    {
        dir = impl->propertiesHolder.Get<QString>(UIManagerDetail::FILE_DIR_KEY, dir);
    }
    QString filePath = QFileDialog::getSaveFileName(windowInfo.window, params.title, dir, params.filters);
    if (!filePath.isEmpty())
    {
        impl->propertiesHolder.Set(UIManagerDetail::FILE_DIR_KEY, QFileInfo(filePath).absoluteFilePath());
    }
    return filePath;
}

QString UIManager::GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    UIManagerDetail::MainWindowInfo* windowInfo = impl->FindWindow(windowKey);
    QWidget* parent = nullptr;
    if (windowInfo != nullptr)
    {
        parent = windowInfo->window;
    }

    QString dir = params.dir;
    if (dir.isEmpty())
    {
        dir = impl->propertiesHolder.Get<QString>(UIManagerDetail::FILE_DIR_KEY, dir);
    }
    QString filePath = QFileDialog::getOpenFileName(parent, params.title, dir, params.filters);
    if (!filePath.isEmpty())
    {
        impl->propertiesHolder.Set(UIManagerDetail::FILE_DIR_KEY, QFileInfo(filePath).absoluteFilePath());
    }
    return filePath;
}

QString UIManager::GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params)
{
    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);

    QString dir = params.dir;
    if (dir.isEmpty())
    {
        dir = impl->propertiesHolder.Get<QString>(UIManagerDetail::FILE_DIR_KEY, dir);
    }

    QString dirPath = QFileDialog::getExistingDirectory(windowInfo.window, params.title, dir, params.options);
    if (!dirPath.isEmpty())
    {
        impl->propertiesHolder.Set(UIManagerDetail::FILE_DIR_KEY, dirPath);
    }
    return dirPath;
}

ModalMessageParams::Button UIManager::ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params)
{
    using namespace UIManagerDetail;
    MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);
    QMessageBox msgBox(windowInfo.window);
    msgBox.setWindowTitle(params.title);
    msgBox.setText(params.message);
    msgBox.setStandardButtons(Convert(params.buttons));
    msgBox.setDefaultButton(Convert(params.defaultButton));
    msgBox.setIcon(Convert(params.icon));

    int ret = msgBox.exec();
    QMessageBox::StandardButton resultButton = static_cast<QMessageBox::StandardButton>(ret);
    return Convert(resultButton);
}

void UIManager::InjectWindow(const WindowKey& windowKey, QMainWindow* window)
{
    UIManagerDetail::MainWindowInfo windowInfo;
    windowInfo.window = window;
    windowInfo.menuBar = window->findChild<QMenuBar*>();
    window->show();
    impl->InitNewWindow(windowKey, window);
    impl->windows.emplace(windowKey, windowInfo);
}

void UIManager::SetCurrentModule(ClientModule* module)
{
    DVASSERT((impl->currentModule == nullptr && module != nullptr) ||
             (impl->currentModule != nullptr && module == nullptr));
    impl->currentModule = module;
}

void UIManager::ModuleDestroyed(ClientModule* module)
{
    DVASSERT(impl->currentModule != module);
    auto iter = impl->moduleResourcesMap.find(module);
    if (iter != impl->moduleResourcesMap.end())
    {
        Impl::ModuleResources& resources = iter->second;
        for (QPointer<QWidget>& w : resources.widgets)
        {
            if (!w.isNull())
            {
                delete w.data();
            }
        }

        for (QPointer<QAction>& a : resources.actions)
        {
            if (!a.isNull())
            {
                QWidget* attachedWidget = GetAttachedWidget(a);
                if (attachedWidget != nullptr)
                {
                    delete attachedWidget;
                }
                delete a.data();
            }
        }

        impl->moduleResourcesMap.erase(iter);
    }
}

} // namespace TArc
} // namespace DAVA

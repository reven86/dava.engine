#include "WindowSubSystem/Private/UIManager.h"

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"

#include "WindowSubSystem/ActionUtils.h"
#include "WindowSubSystem/Private/WaitDialog.h"
#include "DataProcessing/Private/QtReflectionBridge.h"
#include "DataProcessing/PropertiesHolder.h"

#include "Utils/StringFormat.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QToolButton>

#include <QFileDialog>
#include <QMessageBox>

#include <QQuickWidget>

namespace DAVA
{
namespace TArc
{
namespace UIManagerDetail
{
String geometryKey("geometry");
String stateKey("state");

static Vector<std::pair<QMessageBox::StandardButton, ModalMessageParams::Button>> buttonsConvertor =
{
  std::make_pair(QMessageBox::Ok, ModalMessageParams::Ok),
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

struct MainWindowInfo
{
    QMainWindow* window = nullptr;
    QMenuBar* menuBar = nullptr;
};

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

    QStringList path = url.path().split("/");
    DVASSERT(!path.isEmpty());
    QString topLevelTitle = path.front();
    QMenu* topLevelMenu = windowInfo.menuBar->findChild<QMenu*>(topLevelTitle, Qt::FindDirectChildrenOnly);
    if (topLevelMenu == nullptr)
    {
        topLevelMenu = new QMenu(topLevelTitle);
        topLevelMenu->setObjectName(topLevelTitle);
        windowInfo.menuBar->addMenu(topLevelMenu);
    }

    QMenu* currentLevelMenu = topLevelMenu;
    for (int i = 1; i < path.size(); ++i)
    {
        QString currentLevelTittle = path[i];
        QMenu* menu = currentLevelMenu->findChild<QMenu*>(currentLevelTittle);
        if (menu == nullptr)
        {
            menu = new QMenu(currentLevelTittle, currentLevelMenu);
            menu->setObjectName(currentLevelTittle);
            currentLevelMenu->addMenu(menu);
        }
        currentLevelMenu = menu;
    }

    currentLevelMenu->addAction(action);
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

    toolbar->addAction(action);
}

void AddStatusbarPoint(const QUrl& url, QAction* action, MainWindowInfo& windowInfo)
{
    bool isPermanent = url.path() == permanentStatusbarAction;
    int stretchFactor = url.fragment().toInt();
    QWidget* widget = action->data().value<QWidget*>();
    if (widget == nullptr)
    {
        QToolButton* toolButton = new QToolButton();
        toolButton->setDefaultAction(action);
        widget = toolButton;
    }

    //action->setParent(widget);
    QStatusBar* statusBar = windowInfo.window->statusBar();
    if (isPermanent)
    {
        statusBar->addPermanentWidget(widget, stretchFactor);
    }
    else
    {
        statusBar->addWidget(widget, stretchFactor);
    }
}

void AddAction(MainWindowInfo& windowInfo, const ActionPlacementInfo& placement, QAction* action)
{
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

QDockWidget* CreateDockWidget(const DockPanelInfo& dockPanelInfo, MainWindowInfo& mainWindowInfo, QMainWindow* mainWindow)
{
    const QString& text = dockPanelInfo.title;

    QDockWidget* dockWidget = new QDockWidget(text, mainWindow);
    dockWidget->setObjectName(text);

    QAction* dockWidgetAction = dockWidget->toggleViewAction();

    const ActionPlacementInfo& placement = dockPanelInfo.actionPlacementInfo;

    AddAction(mainWindowInfo, placement, dockWidgetAction);

    return dockWidget;
}

void AddDockPanel(const PanelKey& key, MainWindowInfo& mainWindowInfo, QWidget* widget)
{
    DVASSERT(key.GetType() == PanelKey::DockPanel);
    const DockPanelInfo& info = key.GetInfo().Get<DockPanelInfo>();
    QMainWindow* mainWindow = mainWindowInfo.window;
    DVASSERT(mainWindow != nullptr);
    QDockWidget* newDockWidget = CreateDockWidget(info, mainWindowInfo, mainWindow);
    newDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    newDockWidget->setWidget(widget);

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
    mainWindow->restoreDockWidget(newDockWidget);
}

void AddCentralPanel(const PanelKey& key, const MainWindowInfo& mainWindowInfo, QWidget* widget)
{
    QMainWindow* mainWindow = mainWindowInfo.window;
    DVASSERT(mainWindow != nullptr);
    QWidget* centralWidget = mainWindow->centralWidget();
    if (centralWidget == nullptr)
    {
        mainWindow->setCentralWidget(widget);
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
} // namespace UIManagerDetail

struct UIManager::Impl : public QObject
{
    UIManager::Delegate* managerDelegate = nullptr;
    Array<Function<void(const PanelKey&, UIManagerDetail::MainWindowInfo&, QWidget*)>, PanelKey::TypesCount> addFunctions;
    UnorderedMap<WindowKey, UIManagerDetail::MainWindowInfo> windows;
    std::unique_ptr<QQmlEngine> qmlEngine;
    QtReflectionBridge reflectionBridge;
    PropertiesItem propertiesHolder;
    bool initializationFinished = false;

    Impl(UIManager::Delegate* delegate, PropertiesItem&& givenPropertiesHolder)
        : managerDelegate(delegate)
        , propertiesHolder(std::move(givenPropertiesHolder))
    {
    }

    ~Impl()
    {
    }

    UIManagerDetail::MainWindowInfo& FindOrCreateWindow(const WindowKey& windowKey)
    {
        auto iter = windows.find(windowKey);
        if (iter == windows.end())
        {
            QMainWindow* window = new QMainWindow();
            window->installEventFilter(this);

            FastName appId = windowKey.GetAppID();
            window->setWindowTitle(appId.c_str());
            window->setObjectName(appId.c_str());

            PropertiesItem ph = propertiesHolder.CreateSubHolder(appId.c_str());
            window->restoreGeometry(ph.Get<QByteArray>(UIManagerDetail::geometryKey));
            window->restoreState(ph.Get<QByteArray>(UIManagerDetail::stateKey));

            UIManagerDetail::MainWindowInfo info;
            info.window = window;
            auto emplacePair = windows.emplace(windowKey, info);
            DVASSERT(emplacePair.second == true);
            iter = emplacePair.first;
        }

        return iter->second;
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
                    ph.Set(UIManagerDetail::stateKey, mainWindow->saveState());
                    ph.Set(UIManagerDetail::geometryKey, mainWindow->saveGeometry());

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
};

UIManager::UIManager(Delegate* delegate, PropertiesItem&& holder)
    : impl(new Impl(delegate, std::move(holder)))
{
    impl->addFunctions[PanelKey::DockPanel] = MakeFunction(&UIManagerDetail::AddDockPanel);
    impl->addFunctions[PanelKey::CentralPanel] = MakeFunction(&UIManagerDetail::AddCentralPanel);

    impl->qmlEngine.reset(new QQmlEngine());
    impl->qmlEngine->addImportPath("qrc:/");
    impl->qmlEngine->addImportPath(":/");
}

UIManager::~UIManager() = default;

void UIManager::InitializationFinished()
{
    impl->initializationFinished = true;
    for (auto& windowIter : impl->windows)
    {
        windowIter.second.window->show();
    }
}

void UIManager::AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget)
{
    DVASSERT(widget != nullptr);
    UIManagerDetail::MainWindowInfo& mainWindowInfo = impl->FindOrCreateWindow(windowKey);

    widget->setObjectName(panelKey.GetViewName());

    PanelKey::Type type = panelKey.GetType();
    DVASSERT(impl->addFunctions[type] != nullptr);

    impl->addFunctions[type](panelKey, mainWindowInfo, widget);

    QMainWindow* window = mainWindowInfo.window;
    DVASSERT(window != nullptr);
    if (!window->isVisible() && impl->initializationFinished)
    {
        window->show();
    }
}

void UIManager::AddView(const WindowKey& windowKey, const PanelKey& panelKey, const QString& resourceName, DataWrapper&& data)
{
    AddView(windowKey, panelKey, LoadView(panelKey.GetViewName(), resourceName, std::move(data)));
}

void UIManager::AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action)
{
    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);
    UIManagerDetail::AddAction(windowInfo, placement, action);
}

QWidget* UIManager::LoadView(const QString& name, const QString& resourceName, DataWrapper&& data)
{
    QPointer<QQuickWidget> view = new QQuickWidget(impl->qmlEngine.get(), nullptr);
    view->setObjectName(name);
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QPointer<QtReflected> qtReflected = impl->reflectionBridge.CreateQtReflected(std::move(data), view);
    qtReflected->metaObjectCreated.Connect([qtReflected, view, resourceName]()
                                           {
                                               if (qtReflected != nullptr && view != nullptr)
                                               {
                                                   view->rootContext()->setContextProperty("context", qtReflected);
                                                   view->setSource(QUrl(resourceName));

                                                   if (view->status() != QQuickWidget::Ready)
                                                   {
                                                       Logger::Error("!!! QML %s has not been loaded !!!", resourceName.toStdString().c_str());
                                                       foreach (QQmlError error, view->errors())
                                                       {
                                                           Logger::Error("Error : %s", error.toString().toStdString().c_str());
                                                       }
                                                   }
                                               }

                                               qtReflected->metaObjectCreated.DisconnectAll();
                                           });
    qtReflected->Init();

    return view;
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
    dlg->Show();
    return std::move(dlg);
}

QString UIManager::GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    UIManagerDetail::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);

    String dirKey("fileDialogDir");
    QString dir = params.dir;
    if (dir.isEmpty())
    {
        dir = impl->propertiesHolder.Get<QString>(dirKey, dir);
    }
    QString filePath = QFileDialog::getOpenFileName(windowInfo.window, params.title, dir, params.filters);
    if (!filePath.isEmpty())
    {
        impl->propertiesHolder.Set(dirKey, filePath);
    }
    return filePath;
}

ModalMessageParams::Button UIManager::ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params)
{
    using namespace UIManagerDetail;
    MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);

    QMessageBox::StandardButton resultButton = QMessageBox::information(windowInfo.window, params.title, params.message, Convert(params.buttons));
    return Convert(resultButton);
}

} // namespace TArc
} // namespace DAVA

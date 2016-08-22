#include "WindowSubSystem/Private/UIManager.h"
#include "WindowSubSystem/ActionUtils.h"
#include "WindowSubSystem/Private/WaitDialog.h"
#include "DataProcessing/Private/QtReflectionBridge.h"

#include "Base/Any.h"
#include "Debug/DVAssert.h"
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

namespace tarc
{

namespace UIManagerDetail
{

static DAVA::Vector<std::pair<QMessageBox::StandardButton, ModalMessageParams::Button>> buttonsConvertor =
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
    for (const ButtonNode& node: buttonsConvertor)
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

void AddDockPanel(const PanelKey& key, QWidget* widget, QMainWindow* mainWindow)
{
    DVASSERT(key.GetType() == PanelKey::DockPanel);
    const DockPanelInfo& info = key.GetInfo().Get<DockPanelInfo>();
    QDockWidget* newDockWidget = new QDockWidget(info.title, mainWindow);
    newDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    newDockWidget->setWidget(widget);

    if (info.tabbed == true)
    {
        QList<QDockWidget*> dockWidgets = mainWindow->findChildren<QDockWidget*>();
        QDockWidget* dockToTabbify = nullptr;
        foreach(QDockWidget* dock, dockWidgets)
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

void AddCentralPanel(const PanelKey& key, QWidget* widget, QMainWindow* mainWindow)
{
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
        QToolButton* toolButton= new QToolButton();
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

}

struct UIManager::Impl
{
    DAVA::Array<DAVA::Function<void(const PanelKey&, QWidget*, QMainWindow*)>, PanelKey::TypesCount> addFunctions;
    DAVA::UnorderedMap<DAVA::FastName, UIManagerDetail::MainWindowInfo> windows;
    std::unique_ptr<QQmlEngine> qmlEngine;
    QtReflectionBridge reflectionBridge;
    bool initializationFinished = false;

    ~Impl()
    {
        for (auto& window : windows)
        {
            delete window.second.window;
        }
    }

    UIManagerDetail::MainWindowInfo& FindOrCreateWindow(const WindowKey& windowKey)
    {
        const DAVA::FastName& appID = windowKey.GetAppID();
        auto iter = windows.find(appID);
        if (iter == windows.end())
        {
            QMainWindow* window = new QMainWindow();
            window->setWindowTitle(appID.c_str());
            window->setObjectName(appID.c_str());
            UIManagerDetail::MainWindowInfo info;
            info.window = window;
            auto emplacePair = windows.emplace(appID, info);
            DVASSERT(emplacePair.second == true);
            iter = emplacePair.first;
        }

        return iter->second;
    }
};

UIManager::UIManager()
    : impl(new Impl())
{
    impl->addFunctions[PanelKey::DockPanel] = DAVA::MakeFunction(&UIManagerDetail::AddDockPanel);
    impl->addFunctions[PanelKey::CentralPanel] = DAVA::MakeFunction(&UIManagerDetail::AddCentralPanel);

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
    
    widget->setObjectName(panelKey.GetViewName());
    QMainWindow* window = impl->FindOrCreateWindow(windowKey).window;
    DVASSERT(window != nullptr);

    PanelKey::Type type = panelKey.GetType();
    DVASSERT(impl->addFunctions[type] != nullptr);
    
    impl->addFunctions[type](panelKey, widget, window);

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
    for (const QUrl& url : placement.urls)
    {
        QString scheme = url.scheme();
        if (scheme == menuScheme)
        {
            UIManagerDetail::AddMenuPoint(url, action, windowInfo);
        }
        else if (scheme == toolbarScheme)
        {
            UIManagerDetail::AddToolbarPoint(url, action, windowInfo);
        }
        else if (scheme == statusbarScheme)
        {
            UIManagerDetail::AddStatusbarPoint(url, action, windowInfo);
        }
    }
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
                DAVA::Logger::Error("!!! QML %s has not been loaded !!!", resourceName.toStdString().c_str());
                foreach(QQmlError error, view->errors())
                {
                    DAVA::Logger::Error("Error : %s", error.toString().toStdString().c_str());
                }
            }
        }

        qtReflected->metaObjectCreated.DisconnectAll();
    });
    qtReflected->Init();

    return view;
}

void UIManager::ShowMessage(const WindowKey& windowKey, const QString& message, DAVA::uint32 duration)
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

    return QFileDialog::getOpenFileName(windowInfo.window, params.title, params.dir, params.filters);
}

ModalMessageParams::Button UIManager::ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params)
{
    using namespace UIManagerDetail;
    MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);

    QMessageBox::StandardButton resultButton = QMessageBox::information(windowInfo.window, params.title, params.message, Convert(params.buttons));
    return Convert(resultButton);
}

}

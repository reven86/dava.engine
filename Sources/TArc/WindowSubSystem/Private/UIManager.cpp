#include "WindowSubSystem/Private/UIManager.h"
#include "WindowSubSystem/ActionPlacementUtils.h"
#include "DataProcessing/Private/QtReflectionBridge.h"

#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QMenuBar>

#include <QQuickWidget>

namespace tarc
{

namespace UIManagerDetails
{
struct MainWindowInfo
{
    QMainWindow* window = nullptr;
    QMenuBar* menuBar = nullptr;
};

void AddDockPanel(const PanelKey& key, QWidget* widget, QMainWindow* mainWindow)
{
    DVASSERT(key.GetType() == PanelKey::DockPanel);
    const DockPanelInfo& info = key.GetInfo().Get<DockPanelInfo>();
    QDockWidget* newDockWidget = new QDockWidget(QString::fromStdString(info.tittle), mainWindow);
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
        windowInfo.window->setMenuBar(windowInfo.menuBar);
        windowInfo.menuBar->setObjectName("menu");
        windowInfo.menuBar->setNativeMenuBar(true);
        windowInfo.menuBar->setVisible(true);
    }

    QStringList path = url.path().split("/");
    DVASSERT(!path.isEmpty());
    QString topLevelTitle = path.front();
    QMenu* topLevelMenu = windowInfo.menuBar->findChild<QMenu*>(topLevelTitle, Qt::FindDirectChildrenOnly);
    if (topLevelMenu == nullptr)
    {
        topLevelMenu = windowInfo.menuBar->addMenu(topLevelTitle);
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

}

struct UIManager::Impl
{
    DAVA::Array<DAVA::Function<void(const PanelKey&, QWidget*, QMainWindow*)>, PanelKey::TypesCount> addFunctions;
    DAVA::UnorderedMap<DAVA::FastName, UIManagerDetails::MainWindowInfo> windows;
    std::unique_ptr<QQmlEngine> qmlEngine;
    QtReflectionBridge reflectionBridge;

    UIManagerDetails::MainWindowInfo& FindOrCreateWindow(const WindowKey& windowKey)
    {
        const DAVA::FastName& appID = windowKey.GetAppID();
        auto iter = windows.find(appID);
        if (iter == windows.end())
        {
            QMainWindow* window = new QMainWindow();
            window->setObjectName(appID.c_str());
            UIManagerDetails::MainWindowInfo info;
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
    impl->addFunctions[PanelKey::DockPanel] = DAVA::MakeFunction(&UIManagerDetails::AddDockPanel);
    impl->addFunctions[PanelKey::CentralPanel] = DAVA::MakeFunction(&UIManagerDetails::AddCentralPanel);

    impl->qmlEngine.reset(new QQmlEngine());
    impl->qmlEngine->addImportPath("qrc:/");
    impl->qmlEngine->addImportPath(":/");
}

UIManager::~UIManager() = default;

void UIManager::AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget)
{
    DVASSERT(widget != nullptr);
    
    widget->setObjectName(QString::fromStdString(panelKey.GetViewName()));
    QMainWindow* window = impl->FindOrCreateWindow(windowKey).window;
    DVASSERT(window != nullptr);

    PanelKey::Type type = panelKey.GetType();
    DVASSERT(impl->addFunctions[type] != nullptr);
    
    impl->addFunctions[type](panelKey, widget, window);

    if (!window->isVisible())
    {
        window->show();
    }
}

void UIManager::AddView(const WindowKey& windowKey, const PanelKey& panelKey, const DAVA::String& resourceName, DataWrapper data)
{
    AddView(windowKey, panelKey, LoadView(panelKey.GetViewName(), resourceName, data));
}

void UIManager::AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action)
{
    UIManagerDetails::MainWindowInfo& windowInfo = impl->FindOrCreateWindow(windowKey);
    for (const QUrl& url : placement.urls)
    {
        if (url.scheme() == menuScheme)
        {
            UIManagerDetails::AddMenuPoint(url, action, windowInfo);
        }
    }
}

QWidget* UIManager::LoadView(const DAVA::String& name, const DAVA::String& resourceName, DataWrapper data)
{
    QPointer<QQuickWidget> view = new QQuickWidget(impl->qmlEngine.get(), nullptr);
    view->setObjectName(QString::fromStdString(name));
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QPointer<QtReflected> qtReflected = impl->reflectionBridge.CreateQtReflected(data, view);
    qtReflected->metaObjectCreated.Connect([qtReflected, view, resourceName]()
    {
        if (qtReflected != nullptr && view != nullptr)
        {
            view->rootContext()->setContextProperty("context", qtReflected);
            view->setSource(QUrl(resourceName.c_str()));

            if (view->status() != QQuickWidget::Ready)
            {
                DAVA::Logger::Warning("!!! QML %s has not been loaded !!!", resourceName.c_str());
                foreach(QQmlError error, view->errors())
                {
                    DAVA::Logger::Warning("Error : %s", error.toString().toStdString().c_str());
                }
            }
        }

        qtReflected->metaObjectCreated.DisconnectAll();
    });
    qtReflected->Init();

    return view;
}

}

#include "WindowSubSystem/Private/UIManager.h"
#include "DataProcessing/Private/QtReflectionBridge.h"

#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QQmlEngine>
#include <QQmlContext>

#include <QQuickWidget>

namespace tarc
{

namespace UIManagerDetails
{
void AddDockPanel(const WindowKey& key, QWidget* widget, QMainWindow* mainWindow)
{
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

void AddCentralPanel(const WindowKey& key, QWidget* widget, QMainWindow* mainWindow)
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
}

struct UIManager::Impl
{
    DAVA::Array<DAVA::Function<void(const WindowKey&, QWidget*, QMainWindow*)>, WindowKey::TypesCount> addFunctions;
    DAVA::UnorderedMap<DAVA::FastName, QMainWindow*> windows;
    std::unique_ptr<QQmlEngine> qmlEngine;
    QtReflectionBridge reflectionBridge;
};

UIManager::UIManager()
    : impl(new Impl())
{
    impl->addFunctions[WindowKey::DockPanel] = DAVA::MakeFunction(&UIManagerDetails::AddDockPanel);
    impl->addFunctions[WindowKey::CentralPanel] = DAVA::MakeFunction(&UIManagerDetails::AddCentralPanel);

    impl->qmlEngine.reset(new QQmlEngine());
    impl->qmlEngine->addImportPath("qrc:/");
    impl->qmlEngine->addImportPath(":/");
}

UIManager::~UIManager() = default;

void UIManager::AddView(const WindowKey& key, QWidget* widget)
{
    DVASSERT(widget != nullptr);
    
    widget->setObjectName(QString::fromStdString(key.viewName));
    QMainWindow* window = FindOrCreateWindow(key.appID);

    DVASSERT(window != nullptr);
    DVASSERT(impl->addFunctions[key.type] != nullptr);
    
    impl->addFunctions[key.type](key, widget, window);

    if (!window->isVisible())
    {
        window->show();
    }
}

void UIManager::AddView(const WindowKey& key, const DAVA::String& resourceName, DataWrapper data)
{
    AddView(key, LoadView(key.viewName, resourceName, data));
}

QMainWindow* UIManager::FindOrCreateWindow(const DAVA::FastName& appID)
{
    auto iter = impl->windows.find(appID);
    if (iter == impl->windows.end())
    {
        QMainWindow* window = new QMainWindow();
        window->setObjectName(appID.c_str());
        auto emplacePair = impl->windows.emplace(appID, window);
        DVASSERT(emplacePair.second == true);
        iter = emplacePair.first;
    }

    return iter->second;
}

QWidget* UIManager::LoadView(const DAVA::String& name, const DAVA::String& resourceName, DataWrapper data)
{
    QPointer<QQuickWidget> view = new QQuickWidget(impl->qmlEngine.get(), nullptr);
    view->setObjectName(QString::fromStdString(name));
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QPointer<QtReflected> qtReflected = impl->reflectionBridge.createQtReflected(data, view);
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

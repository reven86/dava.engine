#include "UIManager.h"

#include "Base/Any.h"
#include "Debug/DVAssert.h"

#include <QMainWindow>
#include <QDockWidget>

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

UIManager::UIManager()
{
    addFunctions[WindowKey::DockPanel] = DAVA::MakeFunction(&UIManagerDetails::AddDockPanel);
    addFunctions[WindowKey::CentralPanel] = DAVA::MakeFunction(&UIManagerDetails::AddCentralPanel);
}

void UIManager::AddView(const WindowKey& key, QWidget* widget)
{
    QMainWindow* window = FindOrCreateWindow(key.appID);
    DVASSERT(addFunctions[key.type] != nullptr);
    addFunctions[key.type](key, widget, window);

    if (!window->isVisible())
    {
        window->show();
    }
}

QMainWindow* UIManager::FindOrCreateWindow(const DAVA::FastName& appID)
{
    auto iter = windows.find(appID);
    if (iter == windows.end())
    {
        QMainWindow* window = new QMainWindow();
        window->setObjectName(appID.c_str());
        auto emplacePair = windows.emplace(appID, window);
        DVASSERT(emplacePair.second == true);
        iter = emplacePair.first;
    }

    return iter->second;
}

}

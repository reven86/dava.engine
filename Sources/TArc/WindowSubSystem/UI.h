#pragma once

#include "Base/FastName.h"
#include "Base/Any.h"

#include "DataProcessing/DataWrapper.h"

#include <Qt>

class QWidget;
class QUrl;
class QAction;
namespace tarc
{

struct DockPanelInfo
{
    Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
    DAVA::String tittle;
    bool tabbed = true;
};

struct CentralPanelInfo
{
};

class WindowKey
{
public:
    WindowKey(DAVA::FastName appID_, const DAVA::String& viewName, const DockPanelInfo& info_)
        : appID(appID_)
        , type(DockPanel)
        , info(info_)
    {
    }

    WindowKey(DAVA::FastName appID_, const DAVA::String& viewName, const CentralPanelInfo& info_)
        : appID(appID_)
        , type(CentralPanel)
        , info(info_)
    {
    }

    const DAVA::Any& GetInfo() const { return info; }

private:
    friend class UIManager;

    enum Type
    {
        DockPanel,
        CentralPanel,
        TypesCount
    };

    DAVA::FastName appID;
    DAVA::String viewName;
    Type type;
    DAVA::Any info;
};

class UI
{
public:
    UI() = default;
    UI(const UI& other) = delete;
    UI& operator=(const UI& other) = delete;
    virtual ~UI() {}

    virtual void AddView(const WindowKey& key, QWidget* widget) = 0;
    virtual void AddView(const WindowKey& key, const DAVA::String& resourceName, DataWrapper data) = 0;
    virtual void AddAction(const DAVA::FastName& appID, const QUrl& placement, QAction* action) = 0;
};

}

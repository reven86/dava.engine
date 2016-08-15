#pragma once

#include "Base/FastName.h"
#include "Base/Any.h"

#include "DataProcessing/DataWrapper.h"

#include <Qt>
#include <QUrl>

class QWidget;
class QAction;
namespace tarc
{

class WindowKey
{
public:
    WindowKey(DAVA::FastName appID);
    const DAVA::FastName& GetAppID() const;

private:
    DAVA::FastName appID;
};

struct DockPanelInfo
{
    Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
    QString title;
    bool tabbed = true;
};

struct CentralPanelInfo
{
};

class PanelKey
{
public:
    enum Type
    {
        DockPanel,
        CentralPanel,
        TypesCount
    };
    
    PanelKey(const QString& viewName, const DockPanelInfo& info);
    PanelKey(const QString& viewName, const CentralPanelInfo& info);

    const QString& GetViewName() const;
    Type GetType() const;
    const DAVA::Any& GetInfo() const;

private:
    PanelKey(Type t, const QString& viewName, const DAVA::Any& info);

    QString viewName;
    Type type;
    DAVA::Any info;
};

class ActionPlacementInfo
{
public:
    ActionPlacementInfo() = default;
    ActionPlacementInfo(const QUrl& url);

    void AddPlacementPoint(const QUrl& url);

private:
    friend class UIManager;
    DAVA::Vector<QUrl> urls;
};

struct WaitDialogParams
{
    QString message = QStringLiteral("Please wait, while current operation will be finished");
    DAVA::uint32 min = 0; // if min and max value equal 0, than progress bar will be infinite
    DAVA::uint32 max = 0;
    bool needProgressBar = true;
};

class WaitHandle
{
public:
    virtual ~WaitHandle() {}

    virtual void SetMessage(const QString& msg) = 0;
    virtual void SetRange(DAVA::uint32 min, DAVA::uint32 max) = 0;
    virtual void SetProgressValue(DAVA::uint32 progress) = 0;
    virtual void Update() = 0;
};

class UI
{
public:
    UI() = default;
    UI(const UI& other) = delete;
    UI& operator=(const UI& other) = delete;
    virtual ~UI() {}

    virtual void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) = 0;
    virtual void AddView(const WindowKey& windowKey, const PanelKey& panelKey, const QString& resourceName, DataWrapper data) = 0;
    virtual void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) = 0;

    virtual void ShowMessage(const WindowKey& windowKey, const QString& message, DAVA::uint32 duration = 0) = 0;
    virtual void ClearMessage(const WindowKey& windowKey) = 0;

    virtual std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) = 0;
};

}

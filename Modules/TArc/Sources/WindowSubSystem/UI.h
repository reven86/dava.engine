#pragma once

#include "Base/FastName.h"
#include "Base/Any.h"

#include "DataProcessing/DataWrapper.h"

#include <Qt>
#include <QUrl>
#include <QString>
#include <QFlags>

class QWidget;
class QAction;
namespace DAVA
{
namespace TArc
{
class WindowKey
{
public:
    WindowKey(const FastName& appID);
    const FastName& GetAppID() const;

    bool operator==(const WindowKey& other) const;
    bool operator!=(const WindowKey& other) const;

private:
    FastName appID;
};

class ActionPlacementInfo
{
public:
    ActionPlacementInfo() = default;
    ActionPlacementInfo(const QUrl& url);

    void AddPlacementPoint(const QUrl& url);
    const Vector<QUrl>& GetUrls() const;

private:
    Vector<QUrl> urls;
};

struct DockPanelInfo
{
    DockPanelInfo();
    QString title;
    //path where action for change dock visibility will be placed
    ActionPlacementInfo actionPlacementInfo;
    bool tabbed = true;
    Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
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
    const Any& GetInfo() const;

private:
    PanelKey(Type t, const QString& viewName, const Any& info);

    QString viewName;
    Type type;
    Any info;
};

struct WaitDialogParams
{
    QString message = QStringLiteral("Please wait, until current operation will be finished");
    uint32 min = 0; // if min and max value equal 0, than progress bar will be infinite
    uint32 max = 0;
    bool needProgressBar = true;
};

class WaitHandle
{
public:
    virtual ~WaitHandle()
    {
    }

    virtual void SetMessage(const QString& msg) = 0;
    virtual void SetRange(uint32 min, uint32 max) = 0;
    virtual void SetProgressValue(uint32 progress) = 0;
    virtual void Update() = 0;
};

struct FileDialogParams
{
    QString title;
    QString dir;
    QString filters;
};

struct ModalMessageParams
{
    enum Button
    {
        Ok = 0x1,
        Cancel = 0x2,
        Close = 0x4,
        Yes = 0x8,
        YesToAll = 0x10,
        No = 0x20,
        NoToAll = 0x40,
        Discard = 0x80,
        Apply = 0x100,
        Save = 0x200,
        SaveAll = 0x400,
        Abort = 0x800,
        Retry = 0x1000,
        Ignore = 0x2000,
        Reset = 0x4000
    };

    Q_DECLARE_FLAGS(Buttons, Button);

    QString title;
    QString message;
    Buttons buttons = Buttons(Ok | Cancel);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModalMessageParams::Buttons);

class UI
{
public:
    UI() = default;
    UI(const UI& other) = delete;
    UI& operator=(const UI& other) = delete;
    virtual ~UI()
    {
    }

    virtual void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) = 0;
    virtual void AddView(const WindowKey& windowKey, const PanelKey& panelKey, const QString& resourceName, DataWrapper&& data) = 0;
    virtual void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) = 0;

    virtual void ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration = 0) = 0;
    virtual void ClearMessage(const WindowKey& windowKey) = 0;
    virtual ModalMessageParams::Button ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params) = 0;

    virtual QString GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params = FileDialogParams()) = 0;

    virtual std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) = 0;
};
} // namespace TArc
} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::TArc::WindowKey>
{
    std::size_t operator()(const DAVA::TArc::WindowKey& k) const
    {
        std::hash<DAVA::FastName> hasher;
        return hasher(k.GetAppID());
    }
};
}
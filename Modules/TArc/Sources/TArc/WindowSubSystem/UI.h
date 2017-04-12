#pragma once

#include "TArc/DataProcessing/DataWrapper.h"

#include <Functional/Function.h>
#include <Base/Result.h>
#include <Base/FastName.h>
#include <Base/Any.h>

#include <Qt>
#include <QUrl>
#include <QString>
#include <QFlags>
#include <QFileDialog>
#include <QPointer>

class QWidget;
class QAction;
class QMainWindow;

namespace DAVA
{
namespace TArc
{
class ClientModule;
class QtReflectionBridge;
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

/** Token of TArc application's main window */
extern const WindowKey mainWindowKey;

/**
    Most common menu items tokens.
    It's recommended to use them instead of literals hardcoding.
    e.g.
    /code
        CreateMenuPoint(MenuItems::menuEdit); // recommended
        CreateMenuPoint("Edit");              // not recommended
    /endcode
    
*/
namespace MenuItems
{
extern const QString menuFile;
extern const QString menuEdit;
extern const QString menuView;
extern const QString menuHelp;
}

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

struct DirectoryDialogParams
{
    QString title;
    QString dir;
    QFileDialog::Option options = QFileDialog::ShowDirsOnly;
};

struct ModalMessageParams
{
    enum Button
    {
        NoButton = 0,
        Ok = 0x1,
        Open = 0x2,
        Cancel = 0x4,
        Close = 0x8,
        Yes = 0x10,
        YesToAll = 0x20,
        No = 0x40,
        NoToAll = 0x80,
        Discard = 0x100,
        Apply = 0x200,
        Save = 0x400,
        SaveAll = 0x800,
        Abort = 0x1000,
        Retry = 0x2000,
        Ignore = 0x4000,
        Reset = 0x8000
    };

    enum Icon
    {
        NoIcon,
        Information,
        Warning,
        Critical,
        Question
    };

    Q_DECLARE_FLAGS(Buttons, Button);

    QString title;
    QString message;
    Buttons buttons = Buttons(Ok | Cancel);
    Button defaultButton = NoButton;
    Icon icon = NoIcon;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModalMessageParams::Buttons);

struct NotificationParams
{
    DAVA::Result message;
    DAVA::String title;
    DAVA::Function<void()> callback;
};

class UI
{
public:
    UI() = default;
    UI(const UI& other) = delete;
    UI& operator=(const UI& other) = delete;
    virtual ~UI()
    {
    }

    virtual void DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName) = 0;

    virtual void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) = 0;
    virtual void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) = 0;
    virtual void RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement) = 0;

    virtual void ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration = 0) = 0;
    virtual void ClearMessage(const WindowKey& windowKey) = 0;
    virtual ModalMessageParams::Button ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params) = 0;
    virtual void ShowNotification(const WindowKey& windowKey, const NotificationParams& params) = 0;

    virtual QString GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params = FileDialogParams()) = 0;
    virtual QString GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params = FileDialogParams()) = 0;
    virtual QString GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params = DirectoryDialogParams()) = 0;

    virtual std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) = 0;
    virtual bool HasActiveWaitDalogues() const = 0;
    Signal<> lastWaitDialogWasClosed;

    DAVA_DEPRECATED(virtual QWidget* GetWindow(const WindowKey& windowKey) = 0);
    DAVA_DEPRECATED(virtual void InjectWindow(const WindowKey& windowKey, QMainWindow* window) = 0);

protected:
    friend class UIProxy;
    virtual void SetCurrentModule(ClientModule* module) = 0;
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

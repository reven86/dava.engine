#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"

#include <QUrl>
#include <QList>
#include <QString>

namespace DAVA
{
namespace TArc
{
const WindowKey mainWindowKey(DAVA::FastName("MainWindow"));

namespace MenuItems
{
const QString menuFile("File");
const QString menuEdit("Edit");
const QString menuView("View");
const QString menuHelp("Help");
}

WindowKey::WindowKey(const FastName& appID_)
    : appID(appID_)
{
}

const FastName& WindowKey::GetAppID() const
{
    return appID;
}

bool WindowKey::operator==(const WindowKey& other) const
{
    return appID == other.appID;
}

bool WindowKey::operator!=(const WindowKey& other) const
{
    return !(*this == other);
}

DockPanelInfo::DockPanelInfo()
    : actionPlacementInfo(CreateMenuPoint(QList<QString>() << MenuItems::menuView
                                                           << "Dock"))
{
}

PanelKey::PanelKey(const QString& viewName_, const DockPanelInfo& info_)
    : PanelKey(DockPanel, viewName_, info_)
{
}

PanelKey::PanelKey(const QString& viewName_, const CentralPanelInfo& info_)
    : PanelKey(CentralPanel, viewName_, info_)
{
}

PanelKey::PanelKey(Type t, const QString& viewName_, const Any& info_)
    : viewName(viewName_)
    , type(t)
    , info(info_)
{
}

const QString& PanelKey::GetViewName() const
{
    return viewName;
}

PanelKey::Type PanelKey::GetType() const
{
    return type;
}

const Any& PanelKey::GetInfo() const
{
    return info;
}

ActionPlacementInfo::ActionPlacementInfo(const QUrl& url)
{
    AddPlacementPoint(url);
}

void ActionPlacementInfo::AddPlacementPoint(const QUrl& url)
{
    urls.emplace_back(url);
}

const Vector<QUrl>& ActionPlacementInfo::GetUrls() const
{
    return urls;
}
} // namespace TArc
} // namespace DAVA

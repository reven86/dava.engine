#include "WindowSubSystem/UI.h"

#include <QUrl>

namespace DAVA
{
namespace TArc
{
WindowKey::WindowKey(const DAVA::FastName& appID_)
    : appID(appID_)
{
}

const DAVA::FastName& WindowKey::GetAppID() const
{
    return appID;
}

PanelKey::PanelKey(const QString& viewName_, const DockPanelInfo& info_)
    : PanelKey(DockPanel, viewName_, info_)
{
}

PanelKey::PanelKey(const QString& viewName_, const CentralPanelInfo& info_)
    : PanelKey(CentralPanel, viewName_, info_)
{
}

PanelKey::PanelKey(Type t, const QString& viewName_, const DAVA::Any& info_)
    : type(t)
    , viewName(viewName_)
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

const DAVA::Any& PanelKey::GetInfo() const
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
} // namespace TArc
} // namespace DAVA
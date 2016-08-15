#include "WindowSubSystem/ActionPLacementUtils.h"

namespace tarc
{

QUrl tarc::CreateMenuPoint(const QString& path)
{
    QUrl url;
    url.setPath(path);
    url.setScheme(menuScheme);

    return url;
}

}

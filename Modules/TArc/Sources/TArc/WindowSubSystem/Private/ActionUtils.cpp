#include "TArc/WindowSubSystem/ActionUtils.h"

#include <QAction>

namespace DAVA
{
namespace TArc
{
QUrl CreateMenuPoint(const QString& path)
{
    QUrl url;
    url.setPath(path);
    url.setScheme(menuScheme);

    return url;
}

QUrl CreateToolbarPoint(const QString& toolbarName)
{
    QUrl url;
    url.setPath(toolbarName);
    url.setScheme(toolbarScheme);

    return url;
}

QUrl CreateStatusbarPoint(bool isPermanent, uint32 stretchFactor)
{
    QUrl url;
    url.setScheme(statusbarScheme);
    if (isPermanent)
    {
        url.setPath(permanentStatusbarAction);
    }

    url.setFragment(QString::number(stretchFactor));

    return url;
}

void AttachWidgetToAction(QAction* action, QWidget* widget)
{
    action->setData(QVariant::fromValue(widget));
}
} // namespace TArc
} // namespace DAVA

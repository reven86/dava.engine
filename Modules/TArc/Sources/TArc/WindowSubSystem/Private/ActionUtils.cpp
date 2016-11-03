#include "TArc/WindowSubSystem/ActionUtils.h"

#include <QAction>
#include <QList>
#include <QPair>
#include <QUrlQuery>

namespace DAVA
{
namespace TArc
{
QUrl CreateMenuPoint(const QString& path, const MenuInsertionParams& params)
{
    QUrl url;
    url.setPath(path);
    url.setScheme(menuScheme);

    QList<QPair<QString, QString>> items;
    items.push_back(qMakePair(QString("eInsertionMethod"), QString::number(static_cast<DAVA::int32>(params.method))));
    items.push_back(qMakePair(QString("itemName"), params.item));

    QUrlQuery query;
    query.setQueryItems(items);
    url.setQuery(query);

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

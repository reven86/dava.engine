#include "TArc/WindowSubSystem/ActionUtils.h"

#include <QAction>
#include <QList>
#include <QPair>
#include <QUrlQuery>

namespace DAVA
{
namespace TArc
{
namespace ActionUtilsDetail
{
QUrl CreateUrl(const QString scemeName, const QString& path, const InsertionParams& params)
{
    QUrl url;
    url.setPath(path);
    url.setScheme(scemeName);

    QList<QPair<QString, QString>> items;
    items.push_back(qMakePair(QString("eInsertionMethod"), QString::number(static_cast<DAVA::int32>(params.method))));
    items.push_back(qMakePair(QString("itemName"), params.item));

    QUrlQuery query;
    query.setQueryItems(items);
    url.setQuery(query);

    return url;
}
}

QUrl CreateMenuPoint(const QString& path, const InsertionParams& params)
{
    return ActionUtilsDetail::CreateUrl(menuScheme, path, params);
}

QUrl CreateToolbarPoint(const QString& toolbarName, const InsertionParams& params)
{
    return ActionUtilsDetail::CreateUrl(toolbarScheme, toolbarName, params);
}

QUrl CreateStatusbarPoint(bool isPermanent, uint32 stretchFactor, const InsertionParams& params)
{
    QUrl url = ActionUtilsDetail::CreateUrl(statusbarScheme, "", params);
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

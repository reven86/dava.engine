#include "TArc/WindowSubSystem/ActionUtils.h"

#include "Base/BaseTypes.h"

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
DAVA::Vector<std::pair<QString, InsertionParams::eInsertionMethod>> convertionMap = {
    { QString("after"), InsertionParams::eInsertionMethod::AfterItem },
    { QString("before"), InsertionParams::eInsertionMethod::BeforeItem }
};

QUrl CreateUrl(const QString scemeName, const QString& path, const InsertionParams& params)
{
    QUrl url;
    url.setPath(path);
    url.setScheme(scemeName);

    QList<QPair<QString, QString>> items;
    items.push_back(qMakePair(QString("eInsertionMethod"), InsertionParams::Convert(params.method)));
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

InsertionParams::eInsertionMethod InsertionParams::Convert(const QString& v)
{
    auto iter = std::find_if(ActionUtilsDetail::convertionMap.begin(), ActionUtilsDetail::convertionMap.end(), [&v](const std::pair<QString, InsertionParams::eInsertionMethod>& node)
                             {
                                 return node.first == v;
                             });

    if (iter == ActionUtilsDetail::convertionMap.end())
    {
        DVASSERT(false);
        return InsertionParams::eInsertionMethod::AfterItem;
    }

    return iter->second;
}

QString InsertionParams::Convert(eInsertionMethod v)
{
    auto iter = std::find_if(ActionUtilsDetail::convertionMap.begin(), ActionUtilsDetail::convertionMap.end(), [&v](const std::pair<QString, InsertionParams::eInsertionMethod>& node)
                             {
                                 return node.second == v;
                             });

    if (iter == ActionUtilsDetail::convertionMap.end())
    {
        DVASSERT(false);
        return ActionUtilsDetail::convertionMap[0].first;
    }

    return iter->first;
}

DAVA::TArc::InsertionParams InsertionParams::Create(const QUrl& url)
{
    QUrlQuery query(url.query());
    InsertionParams params;
    params.item = query.queryItemValue("itemName");
    params.method = InsertionParams::Convert(query.queryItemValue("eInsertionMethod"));
    return params;
}

} // namespace TArc
} // namespace DAVA

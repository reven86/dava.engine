#pragma once

#include "Base/BaseTypes.h"

#include <QString>
#include <QUrl>

class QAction;
class QWidget;
namespace DAVA
{
namespace TArc
{
static const QString menuScheme = QStringLiteral("menu");
static const QString toolbarScheme = QStringLiteral("toolbar");
static const QString statusbarScheme = QStringLiteral("statusbar");

static const QString permanentStatusbarAction = QStringLiteral("permanent");

struct MenuInsertionParams
{
    enum class eInsertionMethod
    {
        BeforeItem,
        AfterItem
    };

    eInsertionMethod method = eInsertionMethod::AfterItem;
    QString item;
};

QUrl CreateMenuPoint(const QString& path, const MenuInsertionParams& params = MenuInsertionParams());
QUrl CreateToolbarPoint(const QString& toolbarName);
QUrl CreateStatusbarPoint(bool isPermanent, uint32 stretchFactor = 0);

void AttachWidgetToAction(QAction* action, QWidget* widget);
} // namespace TArc
} // namespace DAVA
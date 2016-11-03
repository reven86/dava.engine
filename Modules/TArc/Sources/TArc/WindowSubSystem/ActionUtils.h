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

struct InsertionParams
{
    enum class eInsertionMethod
    {
        BeforeItem,
        AfterItem
    };

    eInsertionMethod method = eInsertionMethod::AfterItem;
    QString item;
};

QUrl CreateMenuPoint(const QString& path, const InsertionParams& params = InsertionParams());
QUrl CreateToolbarPoint(const QString& toolbarName, const InsertionParams& params = InsertionParams());
QUrl CreateStatusbarPoint(bool isPermanent, uint32 stretchFactor = 0, const InsertionParams& params = InsertionParams());

void AttachWidgetToAction(QAction* action, QWidget* widget);
} // namespace TArc
} // namespace DAVA
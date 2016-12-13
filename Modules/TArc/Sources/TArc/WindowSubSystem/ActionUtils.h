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

    static eInsertionMethod Convert(const QString& v);
    static QString Convert(eInsertionMethod v);
    static InsertionParams Create(const QUrl& url);

    InsertionParams() = default;
    InsertionParams(eInsertionMethod method_, const QString& item_ = QString());

    eInsertionMethod method = eInsertionMethod::AfterItem;
    QString item;
};

QUrl CreateMenuPoint(const QString& menuName, const InsertionParams& params = InsertionParams());
QUrl CreateMenuPoint(QList<QString> menusPath, const InsertionParams& params = InsertionParams());
QUrl CreateToolbarPoint(const QString& toolbarName, const InsertionParams& params = InsertionParams());
QUrl CreateStatusbarPoint(bool isPermanent, uint32 stretchFactor = 0, const InsertionParams& params = InsertionParams());

/// You can attach widget to Action. This widget will be used to appear action on toolbar or in status bar
void AttachWidgetToAction(QAction* action, QWidget* widget);
QWidget* GetAttachedWidget(QAction* action);
} // namespace TArc
} // namespace DAVA
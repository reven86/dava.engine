#ifndef QTTOOLS_WIDGETITEM_H
#define QTTOOLS_WIDGETITEM_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QList>
#include <QWidgetList>
POP_QT_WARNING_SUPRESSOR

class QWidget;
class WidgetModel;

class WidgetItem
: public QObject
{
    friend class QSharedPointer<WidgetItem>;
    friend class WidgetModel;

    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

private:
    using ItemList = QList<QSharedPointer<WidgetItem>>;

public:
    ~WidgetItem();

    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    explicit WidgetItem(QWidget* w);
    void rebuildChildren();
    void onChildAdd(QWidget* w);
    void onChildRemove(QWidget* w);

    QPointer<QWidget> widget;
    QSharedPointer<WidgetItem> parentItem;
    QWeakPointer<WidgetItem> self;
    QPointer<WidgetModel> model;
    ItemList children;

public:
    static QSharedPointer<WidgetItem> create(QWidget* w);
};


#endif // QTTOOLS_WIDGETITEM_H

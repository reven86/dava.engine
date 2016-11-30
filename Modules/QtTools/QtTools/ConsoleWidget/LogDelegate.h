#ifndef __LOGDELEGATE_H__
#define __LOGDELEGATE_H__

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QWidget>
#include <QPointer>
#include <QScopedPointer>
#include <QStyledItemDelegate>
POP_QT_WARNING_SUPRESSOR

class QAbstractItemView;

class LogDelegate
: public QStyledItemDelegate
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

signals:
    void copyRequest();
    void clearRequest();

public:
    explicit LogDelegate(QAbstractItemView* view, QObject* parent = nullptr);
    ~LogDelegate();

private:
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    QPointer<QAbstractItemView> view;
};


#endif // __LOGDELEGATE_H__

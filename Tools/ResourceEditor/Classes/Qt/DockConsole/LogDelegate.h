#ifndef __LOGDELEGATE_H__
#define __LOGDELEGATE_H__


#include <QWidget>
#include <QPointer>
#include <QScopedPointer>
#include <QStyledItemDelegate>


class QAbstractItemView;


class LogDelegate
    : public QStyledItemDelegate
{
    Q_OBJECT

    signals:
    void copyRequest();

public:
    explicit LogDelegate(QAbstractItemView* view, QObject* parent = NULL);
    ~LogDelegate();

private:
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

    QPointer<QAbstractItemView> view;
};


#endif // __LOGDELEGATE_H__

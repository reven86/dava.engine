#include "LogDelegate.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QCursor>


LogDelegate::LogDelegate(QAbstractItemView* _view, QObject* parent)
    : QStyledItemDelegate(parent)
      , view(_view)
{
    Q_ASSERT( _view );
    view->setItemDelegate(this);
}

LogDelegate::~LogDelegate()
{
}

bool LogDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, QStyleOptionViewItem const& option, QModelIndex const& index)
{
    switch (event->type())
    {
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent* me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::RightButton)
            {
                QMenu menu;
                QAction* copy = new QAction("Copy", this);
                connect(copy, SIGNAL( triggered() ), SIGNAL(copyRequest()));
                menu.addAction(copy);
                menu.exec(QCursor::pos());
                copy->deleteLater();
            }
        }
        break;

    default:
        break;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
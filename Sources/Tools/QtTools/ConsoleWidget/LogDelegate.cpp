#include "LogDelegate.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>


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
                QAction* copy = new QAction("Copy", &menu);
                connect(copy, &QAction::triggered, this, &LogDelegate::copyRequest);
                menu.addAction(copy);
                QAction* clear = new QAction("Clear", &menu);
                connect(clear, &QAction::triggered, this, &LogDelegate::clearRequest);
                menu.addAction(clear);
                menu.exec(QCursor::pos());
            }
        }
        break;

    default:
        break;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
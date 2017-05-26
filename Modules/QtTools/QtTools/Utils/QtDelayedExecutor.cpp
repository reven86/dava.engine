#include "QtDelayedExecutor.h"
#include "QtEventIds.h"

#include <QEvent>
#include <QApplication>

namespace QtDelayedExecutorDetail
{
class QtDelayedExecuteEvent : public QEvent
{
public:
    QtDelayedExecuteEvent(const DAVA::Function<void()>& functor_)
        : QEvent(QT_EVENT_TYPE(QtToolsEventsTable::DelayedExecute))
        , functor(functor_)
    {
    }

    void Execute()
    {
        functor();
    }

private:
    DAVA::Function<void()> functor;
};
}

QtDelayedExecutor::QtDelayedExecutor(QObject* parent /*= nullptr*/)
    : QObject(parent)
{
}

void QtDelayedExecutor::DelayedExecute(const DAVA::Function<void()>& functor)
{
    qApp->postEvent(this, new QtDelayedExecutorDetail::QtDelayedExecuteEvent(functor));
}

bool QtDelayedExecutor::event(QEvent* e)
{
    if (e->type() == QT_EVENT_TYPE(QtToolsEventsTable::DelayedExecute))
    {
        QtDelayedExecutorDetail::QtDelayedExecuteEvent* event = static_cast<QtDelayedExecutorDetail::QtDelayedExecuteEvent*>(e);
        event->Execute();
        return true;
    }

    return QObject::event(e);
}

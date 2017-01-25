#include "QtDelayedExecutor.h"

#include <QEvent>
#include <QApplication>

namespace QtDelayedExecutorDetail
{
class QtDelayedExecuteEvent : public QEvent
{
public:
    QtDelayedExecuteEvent(const DAVA::Function<void()>& functor_)
        : QEvent(delayedEventType)
        , functor(functor_)
    {
    }

    void Execute()
    {
        functor();
    }

    static QEvent::Type delayedEventType;

private:
    DAVA::Function<void()> functor;
};

QEvent::Type QtDelayedExecuteEvent::delayedEventType = static_cast<QEvent::Type>(QEvent::User + 1);
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
    if (e->type() == QtDelayedExecutorDetail::QtDelayedExecuteEvent::delayedEventType)
    {
        QtDelayedExecutorDetail::QtDelayedExecuteEvent* event = static_cast<QtDelayedExecutorDetail::QtDelayedExecuteEvent*>(e);
        event->Execute();
        return true;
    }

    return QObject::event(e);
}

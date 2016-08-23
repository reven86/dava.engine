#include "AssertGuard.h"

#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QWidget>

namespace DAVA
{
namespace TArc
{
class ToolsAssetGuard::EventFilter final : public QObject
{
public:
    EventFilter()
    {
        QWidgetList lst = qApp->allWidgets();
        for (int i = 0; i < lst.size(); ++i)
        {
            QWidget* widget = lst[i];
            widget->installEventFilter(this);
        }

        QAbstractEventDispatcher* dispatcher = qApp->eventDispatcher();
        qApp->installEventFilter(this);
        if (dispatcher != nullptr)
        {
            dispatcher->installEventFilter(this);
        }
    }

    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (e->spontaneous())
        {
            return true;
        }

        QEvent::Type type = e->type();
        switch (type)
        {
        case QEvent::Timer:
        case QEvent::Expose:
        case QEvent::Paint:
            return true;
        default:
            break;
        }

        return false;
    }
};

void ToolsAssetGuard::Init()
{
    DVAssertMessage::SetShowInnerOverride([](DVAssertMessage::eModalType type, const char8* message)
                                                {
                                                    return ToolsAssetGuard::Instance()->InnerShow(type, message);
                                                });
}

bool ToolsAssetGuard::InnerShow(DVAssertMessage::eModalType modalType, const char8* message)
{
    LockGuard<Mutex> mutexGuard(mutex);

    std::unique_ptr<EventFilter> filter;
    if (Thread::IsMainThread())
    {
        filter.reset(new EventFilter());
    }

    return DVAssertMessage::InnerShow(DVAssertMessage::ALWAYS_MODAL, message);
}
} // namespace TArc
} // namespace DAVA

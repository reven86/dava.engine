#include "TArc/Utils/AssertGuard.h"

#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"

#if defined(__DAVAENGINE_MACOS__)
#include "AssertGuardMacOSHack.h"
#endif

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QWidget>

namespace DAVA
{
namespace TArc
{
class ToolsAssertGuard::EventFilter final : public QObject
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

void ToolsAssertGuard::Init()
{
    DVAssertMessage::SetShowInnerOverride([](DVAssertMessage::eModalType type, const char8* message)
                                          {
                                              return ToolsAssertGuard::Instance()->InnerShow(type, message);
                                          });
}

bool ToolsAssertGuard::InnerShow(DVAssertMessage::eModalType modalType, const char8* message)
{
    LockGuard<Mutex> mutexGuard(mutex);

    std::unique_ptr<EventFilter> filter;
    if (Thread::IsMainThread())
    {
        filter.reset(new EventFilter());
    }

#if defined(__DAVAENGINE_MACOS__)
    MacOSRunLoopGuard macOSGuard;
#endif
    return DVAssertMessage::InnerShow(DVAssertMessage::ALWAYS_MODAL, message);
}
} // namespace TArc
} // namespace DAVA

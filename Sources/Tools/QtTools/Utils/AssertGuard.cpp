#include "AssertGuard.h"

#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QWidget>

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
    DAVA::DVAssertMessage::SetShowInnerOverride([](DAVA::DVAssertMessage::eModalType type, const DAVA::char8* message)
                                                {
                                                    return ToolsAssetGuard::Instance()->InnerShow(type, message);
                                                });
}

bool ToolsAssetGuard::InnerShow(DAVA::DVAssertMessage::eModalType modalType, const DAVA::char8* message)
{
    DAVA::LockGuard<DAVA::Mutex> mutexGuard(mutex);

    std::unique_ptr<EventFilter> filter;
    if (DAVA::Thread::IsMainThread())
    {
        filter.reset(new EventFilter());
    }

    return DAVA::DVAssertMessage::InnerShow(DAVA::DVAssertMessage::ALWAYS_MODAL, message);
}

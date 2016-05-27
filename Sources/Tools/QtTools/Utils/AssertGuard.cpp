#include "AssertGuard.h"

#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>
#include <QMessageBox>

class ToolsAssetGuard::EventFilter final : public QObject, public QAbstractNativeEventFilter
{
public:
#if defined(Q_OS_WIN)
    EventFilter(WId winId_)
        : winId(winId_)
#elif defined(Q_OS_OSX)
    EventFilter()
#endif
    {
#if defined(Q_OS_OSX)
        qApp->installEventFilter(this);
#endif
        dispatcher = qApp->eventDispatcher();
        if (dispatcher != nullptr)
        {
            dispatcher->installEventFilter(this);
            dispatcher->installNativeEventFilter(this);
        }
    }

    ~EventFilter()
    {
        if (dispatcher != nullptr)
        {
            dispatcher->removeNativeEventFilter(this);
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

    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override
    {
#if defined(Q_OS_WIN)
        MSG* msg = reinterpret_cast<MSG*>(message);
        if (msg->hwnd != reinterpret_cast<HWND>(winId))
        {
            switch (msg->message)
            {
            case WM_PAINT:
            case WM_TIMER:
                return true;
            }
        }
#endif

        return false;
    }

private:
#if defined(Q_OS_WIN)
    WId winId;
#endif
    QAbstractEventDispatcher* dispatcher;
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

    bool result = false;
    
#if defined(Q_OS_OSX)
    EventFilter eventGuard;
    result = DAVA::DVAssertMessage::InnerShow(DAVA::DVAssertMessage::ALWAYS_MODAL, message);
#elif defined(Q_OS_WIN)
    if (!DAVA::Thread::IsMainThread())
        result = DAVA::DVAssertMessage::InnerShow(DAVA::DVAssertMessage::ALWAYS_MODAL, message);
    else
    {
        QMessageBox msgBox(QMessageBox::Critical, "Assert", QString(message), QMessageBox::Ok | QMessageBox::Cancel);
        EventFilter eventGuard(msgBox.winId());

        result = msgBox.exec() == QMessageBox::Cancel;
    }
#endif

    return result;
}

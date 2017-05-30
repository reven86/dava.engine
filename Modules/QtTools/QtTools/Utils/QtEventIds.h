#pragma 

#include <Base/BaseTypes.h>

#include <QEvent>

enum class QtToolsEventsTable : DAVA::int32
{
    DelayedExecute = QEvent::User + 100, // 100 events is reserved for PlatformQt implementation
    End
};

#define QT_EVENT_TYPE(x) (static_cast<QEvent::Type>(x))
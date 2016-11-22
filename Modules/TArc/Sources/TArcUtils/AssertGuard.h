#pragma once

PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
POP_QT_WARNING_SUPRESSOR

#include "Concurrency/Mutex.h"
#include "Debug/DVAssertMessage.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{
namespace TArc
{
class ToolsAssertGuard : public QObject, public StaticSingleton<ToolsAssertGuard>
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
public:
    void Init();
    bool InnerShow(DVAssertMessage::eModalType modalType, const char8* message);

private:
    class EventFilter;
    Mutex mutex;
};
} // namespace TArc
} // namespace DAVA
